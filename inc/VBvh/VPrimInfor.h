#ifndef VBVH_VPRIM_INFOR_H
#define VBVH_VPRIM_INFOR_H
#include "VBvhDefine.h"
#include "common/math/bbox.h"

VBVH_BEGIN_NAMESPACE

class VCentGeomBBox3fa
{
public:
	__forceinline VCentGeomBBox3fa() {}

	__forceinline VCentGeomBBox3fa(EmptyTy)
		: geomBounds(empty), centBounds(empty) {}

	__forceinline VCentGeomBBox3fa(const BBox3fa& geomBounds, const BBox3fa& centBounds)
		: geomBounds(geomBounds), centBounds(centBounds) {}

	__forceinline void extend(const BBox3fa& geomBounds_, const BBox3fa& centBounds_) {
		geomBounds.extend(geomBounds_);
		centBounds.extend(centBounds_);
	}

	__forceinline void reset() {
		geomBounds = empty;
		centBounds = empty;
	}

	__forceinline void extend(const BBox3fa& geomBounds_) {
		geomBounds.extend(geomBounds_);
		centBounds.extend(center2(geomBounds_));
	}

	__forceinline void extend_atomic(const VCentGeomBBox3fa& bounds)
	{
		atomic_min_f32(&geomBounds.lower.x, bounds.geomBounds.lower.x);
		atomic_min_f32(&geomBounds.lower.y, bounds.geomBounds.lower.y);
		atomic_min_f32(&geomBounds.lower.z, bounds.geomBounds.lower.z);
		atomic_max_f32(&geomBounds.upper.x, bounds.geomBounds.upper.x);
		atomic_max_f32(&geomBounds.upper.y, bounds.geomBounds.upper.y);
		atomic_max_f32(&geomBounds.upper.z, bounds.geomBounds.upper.z);

		atomic_min_f32(&centBounds.lower.x, bounds.centBounds.lower.x);
		atomic_min_f32(&centBounds.lower.y, bounds.centBounds.lower.y);
		atomic_min_f32(&centBounds.lower.z, bounds.centBounds.lower.z);
		atomic_max_f32(&centBounds.upper.x, bounds.centBounds.upper.x);
		atomic_max_f32(&centBounds.upper.y, bounds.centBounds.upper.y);
		atomic_max_f32(&centBounds.upper.z, bounds.centBounds.upper.z);
	}

	__forceinline void merge(const VCentGeomBBox3fa& other)
	{
		geomBounds.extend(other.geomBounds);
		centBounds.extend(other.centBounds);
	}

public:
	BBox3fa geomBounds;   //!< geometry bounds of primitives
	BBox3fa centBounds;   //!< centroid bounds of primitives
};

/*! stores bounding information for a set of primitives */
class VPrimInfo : public VCentGeomBBox3fa
{
public:
	__forceinline VPrimInfo() {}

	__forceinline VPrimInfo(EmptyTy)
		: begin(0), end(0), VCentGeomBBox3fa(empty) {}

	__forceinline void reset() {
		VCentGeomBBox3fa::reset();
		begin = end = 0;
	}

	__forceinline VPrimInfo(size_t num, const BBox3fa& geomBounds, const BBox3fa& centBounds)
		: begin(0), end(num), VCentGeomBBox3fa(geomBounds, centBounds) {}

	__forceinline VPrimInfo(size_t begin, size_t end, const BBox3fa& geomBounds, const BBox3fa& centBounds)
		: begin(begin), end(end), VCentGeomBBox3fa(geomBounds, centBounds) {}

	__forceinline void add(const BBox3fa& geomBounds_) {
		VCentGeomBBox3fa::extend(geomBounds_, center2(geomBounds_));
		end++;
	}

	__forceinline void add(const BBox3fa& geomBounds_, const BBox3fa& centBounds_, size_t num_ = 1) {
		VCentGeomBBox3fa::extend(geomBounds_, centBounds_);
		end += num_;
	}

	__forceinline void atomic_extend(const VPrimInfo& pinfo)
	{
		VCentGeomBBox3fa::extend_atomic(pinfo);
		atomic_add(&begin, pinfo.begin);
		atomic_add(&end, pinfo.end);
	}

	__forceinline void merge(const VPrimInfo& other)
	{
		VCentGeomBBox3fa::merge(other);
		assert(begin == 0);
		end += other.end;
	}

	static __forceinline const VPrimInfo merge(const VPrimInfo& a, const VPrimInfo& b) {
		VPrimInfo r = a; r.merge(b); return r;
	}

	/*! returns the number of primitives */
	__forceinline size_t size() const {
		return end - begin;
	}

	__forceinline float leafSAH() const {
		return halfArea(geomBounds)*float(size());
		//return halfArea(geomBounds)*blocks(num); 
	}

	__forceinline float leafSAH(size_t block_shift) const {
		return halfArea(geomBounds)*float((size() + (size_t(1) << block_shift) - 1) >> block_shift);
		//return halfArea(geomBounds)*float((num+3) >> 2);
		//return halfArea(geomBounds)*blocks(num); 
	}

	/*! stream output */
	friend std::ostream& operator<<(std::ostream& cout, const VPrimInfo& pinfo) {
		return cout << "PrimInfo { begin = " << pinfo.begin << ", end = " << pinfo.end << ", geomBounds = " << pinfo.geomBounds << ", centBounds = " << pinfo.centBounds << "}";
	}

public:
	atomic_t begin, end;          //!< number of primitives
};

VBVH_END_NAMESPACE

#endif