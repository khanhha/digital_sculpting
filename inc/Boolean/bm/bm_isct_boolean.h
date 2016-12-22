#ifndef BOOLEAN_BM_ISCT_BOOLEAN_H
#define BOOLEAN_BM_ISCT_BOOLEAN_H

#include "BMesh/BMesh.h"
#include "VBvh/BMeshBvh.h"
#include "bm/bm_isct.h"
#include <boost/geometry/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/geometry/geometries/box.hpp>
#include <boost/geometry/index/rtree.hpp>

namespace bm_isct
{
	typedef boost::geometry::model::point<float, 2, boost::geometry::cs::cartesian> BBPoint2f; /*boost point 2D*/
	typedef boost::geometry::model::box<BBPoint2f> BBBox2f;

	class BmIsctFacePairAccelerator
	{
	public:
		struct SweepEvent
		{
			enum
			{
				OBJ_0 = 1 << 0,
				OBJ_1 = 1 << 1,
				LOWER = 1 << 2,
				UPPER = 1 << 3
			};

			SweepEvent()
				:trig(nullptr), z(0.0), flag(0){};

			BBBox2f	rect;
			BMFace *trig;
			float	z;
			char	flag;
		};

	public:
		BmIsctFacePairAccelerator(VM::BMesh *mesh0, VBvh::BMBvh *bvh0, VM::BMesh *mesh1, VBvh::BMBvh *bvh1, std::vector<std::pair<VM::BMFace*, VM::BMFace*>> &candidates);
		~BmIsctFacePairAccelerator();
		void	run();
		void	overlap_leaf_node_callback(BMLeafNode *n0, BMLeafNode *n1);
	private:
		void	match(std::vector<SweepEvent> &sevents, std::vector<std::pair<VM::BMFace*, VM::BMFace*>>& candidates);
		void	gen_sweep_face_events(std::vector<BMLeafNode*> &nset0, std::vector<BMLeafNode*> &nset1, std::vector<SweepEvent> &sevents);
		size_t	node_set_max_dimension(std::vector<BMLeafNode*> &nset0, std::vector<BMLeafNode*> &nset1);
		void	calc_node_sweep_event(BMLeafNode *node, size_t object, std::vector<SweepEvent>& events, size_t offset, size_t maxDim);
		void	calc_face_sweep_event(BMFace *face, size_t object, SweepEvent &lowerEv, SweepEvent &upperEv, size_t maxDim);
		void	filter_real_tri_tri_isect_pair(std::vector<std::pair<VM::BMFace*, VM::BMFace*>> &pairs);
	private:
		VM::BMesh* _obj0;
		VM::BMesh* _obj1;
		BMBvh *_bvh0;
		BMBvh *_bvh1;

		std::vector<BMLeafNode*> _overlapNodes0;
		std::vector<BMLeafNode*> _overlapNodes1;
		std::vector<std::pair<VM::BMFace*, VM::BMFace*>>	&_interPairs;
	};

	class BmBoolean
	{
	public:
		BmBoolean(VM::BMesh *bm0, VBvh::BMBvh *bvh0, VM::BMesh *bm1, VBvh::BMBvh *bvh1);
		~BmBoolean();
		void compute_union();
		void compute_differnece();
		void compute_intersection();
		VM::BMesh *result();
	private:
		void	boolean_begin();
		void	boolean_end();
		void	inside_outside_propagate(VM::BMesh *mesh);
		bool	boolean_no_delete_check(BMFace *f, BoolType btype);
		bool	boolean_flip_check(BMFace* f, BoolType btype);
		BMesh*	mesh_join(VM::BMesh* mesh0, VM::BMesh *mesh1, const std::vector<std::pair<VM::BMFace*, VM::BMFace*>> &isct_pairs, std::vector<VM::BMFace*> &isct_faces);
		void	delete_and_flip(BoolType btype);
		void	clean(VM::BMesh *mesh);
	private:
		VM::BMesh	*_bm0;
		VBvh::BMBvh *_bvh0;
		VM::BMesh	*_bm1;
		VBvh::BMBvh *_bvh1;

		std::vector<std::pair<BMFace*, BMFace*>> _isct_pairs;
		float		_avg_isct_edge_len;
		VM::BMesh	*_ret_mesh;
	};
}

#endif