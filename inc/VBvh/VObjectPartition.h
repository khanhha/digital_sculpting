#ifndef VBVH_OBJECT_PARTITION_H
#define VBVH_OBJECT_PARTITION_H
#include <exception>

#include "VBvh/VBvhDefine.h"
#include "VBvh/VPrimRef.h"
#include "VBvh/VPrimInfor.h"

using namespace VBvh;

VBVH_BEGIN_NAMESPACE

	/*! Performs standard object binning */
	struct ObjectPartition
	{
		struct Split;
		struct SplitInfo;

		/*! finds the best split */
		static const Split find(
			VPrimRef *__restrict__ const prims, const size_t begin, const size_t end,
			const VPrimInfo& pinfo, const size_t logBlockSize);

	private:

		/*! number of bins */
		static const size_t maxBins = 32;

		/*! number of tasks */
		static const size_t maxTasks = 32;

		/*! mapping into bins */
		struct Mapping
		{
		public:
			__forceinline Mapping() {}

			/*! calculates the mapping */
			__forceinline Mapping(const VPrimInfo& pinfo);

			/*! returns number of bins */
			__forceinline size_t size() const { return num; }

			/*! slower but safe binning */
			__forceinline Vec3ia bin(const Vec3fa& p) const;

			/*! faster but unsafe binning */
			__forceinline Vec3ia bin_unsafe(const Vec3fa& p) const;

			/*! returns true if the mapping is invalid in some dimension */
			__forceinline bool invalid(const int dim) const;

			/*! stream output */
			friend std::ostream& operator<<(std::ostream& cout, const Mapping& mapping) {
				return cout << "Mapping { num = " << mapping.num << ", ofs = " << mapping.ofs << ", scale = " << mapping.scale << "}";
			}
		public:
			size_t num;
			ssef ofs, scale;        //!< linear function that maps to bin ID
		};

	public:

		/*! stores all information to perform some split */
		struct Split
		{
			/*! construct an invalid split by default */
			__forceinline Split()
				: sah(inf), dim(-1), pos(0) {}

			/*! constructs specified split */
			__forceinline Split(float sah, int dim, int pos, const Mapping& mapping)
				: sah(sah), dim(dim), pos(pos), mapping(mapping) {}

			/*! tests if this split is valid */
			__forceinline bool valid() const { return dim != -1; }

			/*! calculates surface area heuristic for performing the split */
			__forceinline float splitSAH() const { return sah; }

			/*! array partitioning */
			void partition(
				VPrimRef *__restrict__ const prims, const size_t begin, const size_t end,
				VPrimInfo& left, VPrimInfo& right) const;

			/*! stream output */
			friend std::ostream& operator<<(std::ostream& cout, const Split& split) {
				return cout << "Split { sah = " << split.sah << ", dim = " << split.dim << ", pos = " << split.pos << "}";
			}

		public:
			float sah;       //!< SAH cost of the split
			int dim;         //!< split dimension
			int pos;         //!< bin index for splitting
			Mapping mapping; //!< mapping into bins
		};

		/*! stores extended information about the split */
		struct SplitInfo
		{
			__forceinline SplitInfo() {}

			__forceinline SplitInfo(size_t leftCount, const BBox3fa& leftBounds, size_t rightCount, const BBox3fa& rightBounds)
				: leftCount(leftCount), rightCount(rightCount), leftBounds(leftBounds), rightBounds(rightBounds) {}

		public:
			size_t leftCount, rightCount;
			BBox3fa leftBounds, rightBounds;
		};

	private:

		/*! stores all binning information */
		struct __aligned(64) BinInfo
		{
			BinInfo();

			/*! clears the bin info */
			void clear();

			/*! bins an array of primitives */
			void bin(const VPrimRef* prims, size_t N, const Mapping& mapping);

			/*! finds the best split by scanning binning information */
			Split best(const Mapping& mapping, const size_t logBlockSize);

			/*! calculates number of primitives on the left */
			__forceinline size_t getNumLeft(Split& split)
			{
				size_t N = 0;
				for (size_t i = 0; i < split.pos; i++)
					N += counts[i][split.dim];
				return N;
			}

			/*! calculates extended split information */
			__forceinline void getSplitInfo(const Mapping& mapping, const Split& split, SplitInfo& info) const
			{
				if (split.dim == -1) {
					new (&info) SplitInfo(0, empty, 0, empty);
					return;
				}

				size_t leftCount = 0;
				BBox3fa leftBounds = empty;
				for (size_t i = 0; i < split.pos; i++) {
					leftCount += counts[i][split.dim];
					leftBounds.extend(bounds[i][split.dim]);
				}
				size_t rightCount = 0;
				BBox3fa rightBounds = empty;
				for (size_t i = split.pos; i < mapping.size(); i++) {
					rightCount += counts[i][split.dim];
					rightBounds.extend(bounds[i][split.dim]);
				}
				new (&info) SplitInfo(leftCount, leftBounds, rightCount, rightBounds);
			}

			//private:
		public: // FIXME
			BBox3fa bounds[maxBins][4]; //!< geometry bounds for each bin in each dimension
			ssei    counts[maxBins];    //!< counts number of primitives that map into the bins
		};
	};

VBVH_END_NAMESPACE
#endif