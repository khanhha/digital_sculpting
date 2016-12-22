#ifndef BM_ISCT_ISECT_OUTER_HULL_EXTRACT_H
#define BM_ISCT_ISECT_OUTER_HULL_EXTRACT_H

#include <vector>
#include "BMesh/BMesh.h"
#include "util/iterPool.h"
#include "bm/bm_isct.h"
#include <Eigen/Geometry>
#include "VBvh/hash/SpacialHash.h"
#include <tbb/concurrent_vector.h>
#include <tbb/parallel_for.h>
#include "BaseLib/TriTriIsct.h"
namespace bm_isct
{

	class IsctFaceExtractor
	{
	public:
		typedef Eigen::AlignedBox<float, 3> BBox3f;
		typedef Eigen::AlignedBox<int, 3>	BBox3i;
	private:
		enum
		{
			F_IN_CELL = 1 << 0,
			F_VISITED = 1 << 1
		};

		struct TriPrim
		{
			typedef Eigen::AlignedBox<float, 3> RealBox;
			typedef Eigen::AlignedBox<int, 3>	IndexBox;

			const RealBox& bbox(){ return bb; }

			BMFace *face;
			BBox3f  bb;
		};

		typedef TriPrim* TriPrimPtr;

		struct Cell
		{
			enum{
				FILTER_SINGLE_COMBINATORIAL_DISK = 1 << 0,
				FILTER_POSITIVE_DIRECTION = 1 << 1,
				FILTER_FREE_ISECT_BOUNDARY = 1 << 2
			};

			Cell() :filter(0){}

			std::vector<TriPrimPtr> tris;
			std::vector<BMEdge*> bdr_edges; /*boundary edges. temporary data while filtering cell*/
			Vector3f lower;
			Vector3f size;
			Vector3f norm; /*average normal*/
			Vector3f ref_dir;
			short filter;
		};

		typedef boost::shared_ptr<Cell> CellPtr;
		typedef tbb::concurrent_vector<std::pair<VM::BMFace*, VM::BMFace*>> IsectFacePairs;
	public:
		IsctFaceExtractor(VM::BMesh *bm, std::vector<VM::BMFace*> &isct_faces);
		~IsctFaceExtractor();
		void run();
	private:
		void isct_faces_collect(BMesh *bm, std::vector<BMFace*> &isct_faces);
		void clone_bmesh(BMesh *origin, const std::vector<BMFace*> &org_isct_faces, BMesh **clone_mesh, std::vector<BMFace*> &clone_isct_faces);
		void tri_prims_compute(std::vector<TriPrim> &prims);
		void cell_build(const VBvh::SpacialHash<TriPrim, float> &spacial, std::vector<CellPtr> &cells);
		void cell_filter(std::vector<CellPtr> &cells);
		void cell_filter_single_combinatorial_disk(std::vector<CellPtr> &cells);
		void cell_filter_positive_direction(std::vector<CellPtr> &cells);
		void cell_filter_free_isect_boundary(std::vector<CellPtr> &cells);

		void cell_filter_tri_tri_isect(std::vector<CellPtr> &cells, IsectFacePairs &isct_pairs);
		bool tri_pair_adjacent(const BMFace *tri0, const BMFace *tri1);

		//void test_brute_force();
#ifdef ISCT_TEST
		void debug_output(VBvh::SpacialHash<TriPrim, float> &spacial);
		void debug_output_bb(BBox3f &bb, std::vector<Point3Dd> &segments);
		void debug_output_self_isect_cell();
		void debug_output_isect_tris();
		void debug_output_tri_segments(BMFace *f, std::vector<Point3Dd> &segments);
		void debug_output_all_cell_tris(VBvh::SpacialHash<TriPrim, float> &spacial, std::vector<Point3Dd> &segments);
#endif

	private:
		VM::BMesh *_bm;
		std::vector<VM::BMFace*> &_isct_faces;
	};

	/*
	*extract outer hull from subdivided mesh from IsctProblem.
	*vertices on intersection region must be marked as V_POSITIVE_SIDE or V_NEGATIE side
	*/
	class BmOuterHullExtract
	{
	private:
		typedef Eigen::AlignedBox<float, 3> RealBox;
		typedef Eigen::AlignedBox<int, 3>	IndexBox;

		struct TriPrim
		{
			const RealBox& bbox(){ return bb; }

			BMFace *face;
			RealBox  bb;
		};

		typedef TriPrim* TriPrimPtr;

		struct Cell
		{
			enum{
				FILTER_SINGLE_COMBINATORIAL_DISK = 1 << 0,
				FILTER_POSITIVE_DIRECTION = 1 << 1,
				FILTER_FREE_ISECT_BOUNDARY = 1 << 2
			};

			Cell() :filter(0){}

			std::vector<TriPrimPtr> tris;
			std::vector<BMEdge*> bdr_edges; /*boundary edges. temporary data while filtering cell*/
			Vector3f lower;
			Vector3f size;
			Vector3f norm; /*average normal*/
			Vector3f ref_dir;
			short	 filter;
		};
		
		typedef Cell* CellPtr;
		typedef tbb::concurrent_vector<std::pair<BMFace*, BMFace*>> IsectFacePairs;

	public:
		BmOuterHullExtract(VM::BMesh *mesh);
		~BmOuterHullExtract();
		void extract();
		VM::BMesh *result();
	private:
		void		delete_non_manifol_edges(VM::BMesh *mesh);
		void		clear(VM::BMesh *mesh);
		void		clone_bmesh(BMesh *origin, const std::vector<BMFace*> &org_isct_faces, BMesh **clone_mesh, std::vector<BMFace*> &clone_isct_faces);
	
		void		outer_hull_peel();
		VM::BMFace* outer_hull_seed_face(VM::BMesh *bm);
		void		outer_hull_propagate(VM::BMFace* seed_face);
		VM::BMFace* outer_hull_next_outer_face_around_edge(VM::BMEdge *e, VM::BMFace *cur_face);
	private:
		VM::BMesh *_org_bm;
		VM::BMesh *_bm;
	private:
		std::vector<TriPrim>	_prims;
		std::vector<CellPtr>	_cells;
		std::vector<BMFace*>	_org_isct_faces;
		std::vector<BMFace*>	_isct_faces;

	};
}

#endif