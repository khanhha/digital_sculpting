#ifndef VBVH_GRID_UTIL_H
#define VBVH_GRID_UTIL_H

#include "VBvh/VBvhDefine.h"
#include <boost/math/special_functions/round.hpp>
VBVH_BEGIN_NAMESPACE

	/** BasicGrid

	Basic Class abstracting a gridded structure in a 3d space;
	Usueful for having coherent float to integer conversion in a unique place:
	Some Notes:
	- bbox is the real occupation of the box in the space;
	- siz is the number of cells for each side

	OBJTYPE:      Type of the indexed objects.
	SCALARTYPE:   Scalars type for structure's internal data (may differ from
	object's scalar type).

	*/

	template <class SCALARTYPE>
	class BasicGrid
	{
	public:

		typedef SCALARTYPE ScalarType;
		typedef Eigen::AlignedBox<ScalarType,3>  Box3x;
		typedef Eigen::AlignedBox<int, 3>		 Box3i;
		typedef Eigen::Matrix<ScalarType,3,1>	 CoordType;
		typedef BasicGrid<SCALARTYPE> GridType;

		Box3x bbox;

		CoordType dim;				/// Spatial Dimention (edge legth) of the bounding box
		Eigen::Vector3i siz;		/// Number of cells forming the grid
		CoordType voxel;			/// Dimensions of a single cell

		/*
		Derives the right values of Dim and voxel starting
		from the current values of siz and bbox
		*/
		void ComputeDimAndVoxel()
		{
			this->dim = this->bbox.max() - this->bbox.min();
			this->voxel[0] = this->dim[0] / this->siz[0];
			this->voxel[1] = this->dim[1] / this->siz[1];
			this->voxel[2] = this->dim[2] / this->siz[2];
		}
		/* Given a 3D point, returns the coordinates of the cell where the point is
		* @param p is a 3D point
		* @return integer coordinates of the cell
		*/
		inline Eigen::Vector3i GridP(const CoordType & p) const
		{
			Eigen::Vector3i pi;
			PToIP(p, pi);
			return pi;
		}

		/* Given a 3D point p, returns the index of the corresponding cell
		* @param p is a 3D point in the space
		* @return integer coordinates pi of the cell
		*/
		inline void PToIP(const CoordType & p, Eigen::Vector3i &pi) const
		{
			CoordType t = p - bbox.min();
			pi[0] = int(t[0] / voxel[0]);
			pi[1] = int(t[1] / voxel[1]);
			pi[2] = int(t[2] / voxel[2]);
		}

		/* Given a cell index return the lower corner of the cell
		* @param integer coordinates pi of the cell
		* @return p is a 3D point representing the lower corner of the cell
		*/
		inline void IPiToPf(const Eigen::Vector3i & pi, CoordType &p) const
		{
			p[0] = ((ScalarType)pi[0])*voxel[0];
			p[1] = ((ScalarType)pi[1])*voxel[1];
			p[2] = ((ScalarType)pi[2])*voxel[2];
			p += bbox.min();
		}

		/* Given a cell index return the corresponding box
		* @param integer coordinates pi of the cell
		* @return b is the corresponding box in <ScalarType> coordinates
		*/
		inline void IPiToBox(const Eigen::Vector3i & pi, Box3x & b) const
		{
			CoordType p;
			p[0] = ((ScalarType)pi[0])*voxel[0];
			p[1] = ((ScalarType)pi[1])*voxel[1];
			p[2] = ((ScalarType)pi[2])*voxel[2];
			p += bbox.min();
			b.min() = p;
			b.max() = (p + voxel);
		}

		/* Given a cell index return the center of the cell itself
		* @param integer coordinates pi of the cell
		* @return b is the corresponding box in <ScalarType> coordinates
		*/inline void IPiToBoxCenter(const Eigen::Vector3i & pi, CoordType & c) const
		{
			CoordType p;
			IPiToPf(pi, p);
			c = p + voxel / ScalarType(2.0);
		}

		// Same of IPiToPf but for the case that you just want to transform 
		// from a space to the other.
		inline void IPfToPf(const CoordType & pi, CoordType &p) const
		{
			p[0] = ((ScalarType)pi[0])*voxel[0];
			p[1] = ((ScalarType)pi[1])*voxel[1];
			p[2] = ((ScalarType)pi[2])*voxel[2];
			p += bbox.min();
		}

		/* Given a cell in <ScalarType> coordinates, compute the corresponding cell in integer coordinates
		* @param b is the cell in <ScalarType> coordinates
		* @return ib is the correspondent box in integer coordinates
		*/
		inline void BoxToIBox(const Box3x & b, Box3i & ib) const
		{
			PToIP(b.min(), ib.min());
			PToIP(b.max(), ib.max());
			//assert(ib.max[0]>=0 && ib.max[1]>=0 && ib.max[2]>=0);	
		}

		/* Given a cell in integer coordinates, compute the corresponding cell in <ScalarType> coordinates
		* @param ib is the cell in integer coordinates
		* @return b is the correspondent box in <ScalarType> coordinates
		*/
		void IBoxToBox(const Box3i & ib, Box3x & b) const
		{
			IPiToPf(ib.min(), b.min());
			IPiToPf(ib.max() + Eigen::Vector3i(1, 1, 1), b.max());
		}

		void BestDim(const Box3x &box, const ScalarType voxel_size, Eigen::Vector3i & dim)
		{
			CoordType box_size = box.max() - box.min();
			int64_t elem_num = (int64_t)(box_size[0] / voxel_size + 0.5) *(int64_t)(box_size[1] / voxel_size + 0.5) * (int64_t)(box_size[2] / voxel_size + 0.5);
			BestDim(elem_num, box_size, dim);
		}

		void BestDim(const int64_t elems, const CoordType &size, Eigen::Vector3i &dim)
		{
			const int64_t mincells = 1;
			const SCALARTYPE GFactor = (SCALARTYPE)0.04;	// GridEntry = NumElem*GFactor
			SCALARTYPE diag = size.norm();	
			SCALARTYPE eps = diag* (SCALARTYPE)1.0e-4;

			assert(elems > 0);
			assert(size[0] >= 0.0);
			assert(size[1] >= 0.0);
			assert(size[2] >= 0.0);


			int64_t ncell = (int64_t)(elems*GFactor);	// Calcolo numero di voxel
			if (ncell<mincells)
				ncell = mincells;

			dim[0] = 1;
			dim[1] = 1;
			dim[2] = 1;

			if (size[0]>eps)
			{
				if (size[1] > eps)
				{
					if (size[2] > eps)
					{
						SCALARTYPE k = std::pow((SCALARTYPE)(ncell / (size[0] * size[1] * size[2])), SCALARTYPE(1.0 / 3.f));
						dim[0] = int(size[0] * k);
						dim[1] = int(size[1] * k);
						dim[2] = int(size[2] * k);
					}
					else
					{
						dim[0] = int(::sqrt(ncell*size[0] / size[1]));
						dim[1] = int(::sqrt(ncell*size[1] / size[0]));
					}
				}
				else
				{
					if (size[2] > eps)
					{
						dim[0] = int(::sqrt(ncell*size[0] / size[2]));
						dim[2] = int(::sqrt(ncell*size[2] / size[0]));
					}
					else
						dim[0] = int(ncell);
				}
			}
			else
			{
				if (size[1] > eps)
				{
					if (size[2] > eps)
					{
						dim[1] = int(::sqrt(ncell*size[1] / size[2]));
						dim[2] = int(::sqrt(ncell*size[2] / size[1]));
					}
					else
						dim[1] = int(ncell);
				}
				else if (size[2] > eps)
					dim[2] = int(ncell);
			}
			dim[0] = std::max<int>(dim[0], 1);
			dim[1] = std::max<int>(dim[1], 1);
			dim[2] = std::max<int>(dim[2], 1);
		}
	};

VBVH_END_NAMESPACE
#endif
