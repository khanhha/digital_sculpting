#include "bm/bm_isct_boolean.h"
#include "bm/bm_isct_isct_problem.h"
#include "VBvh/BMBvhIsect.h"
#include "VBvh/VBvhOverlap.h"
#include <functional>
#include <tbb/parallel_for.h>
#include <tbb/task_group.h>
#include <tbb/parallel_sort.h>
#include <boost/function_output_iterator.hpp>
#include "BaseLib/TriTriIsct.h"
#include <queue>

#ifdef ISCT_TEST
extern std::vector<Point3Dd> g_vscene_testSegments;
extern std::vector<Point3Dd> g_vscene_testPoints;
#endif

namespace bm_isct
{
	using namespace  VM;

	BmIsctFacePairAccelerator::BmIsctFacePairAccelerator(VM::BMesh *mesh0, VBvh::BMBvh *bvh0, VM::BMesh *mesh1, VBvh::BMBvh *bvh1, std::vector<std::pair<VM::BMFace*, VM::BMFace*>> &candidates)
		:
		_obj0(mesh0),
		_obj1(mesh1),
		_bvh0(bvh0),
		_bvh1(bvh1),
		_interPairs(candidates)
	{
	}

	BmIsctFacePairAccelerator::~BmIsctFacePairAccelerator()
	{
	}

	void BmIsctFacePairAccelerator::run()
	{
		/*tag node-node overlap*/
		VBvhOverlap<BMLeafNode> overlapMark(
			_bvh0->rootNode(), _bvh1->rootNode(),
			std::bind(&BmIsctFacePairAccelerator::overlap_leaf_node_callback, this, std::placeholders::_1, std::placeholders::_2));
		overlapMark.run();

		std::vector<SweepEvent>  sevent;
		gen_sweep_face_events(_overlapNodes0, _overlapNodes1, sevent);

		match(sevent, _interPairs);

		filter_real_tri_tri_isect_pair(_interPairs);

		for (auto it = _overlapNodes0.begin(); it != _overlapNodes0.end(); ++it){
			(*it)->unsetAppFlagBit(NODE_TAG);
		}
		for (auto it = _overlapNodes1.begin(); it != _overlapNodes1.end(); ++it){
			(*it)->unsetAppFlagBit(NODE_TAG);
		}
	}

	void BmIsctFacePairAccelerator::overlap_leaf_node_callback(BMLeafNode *n0, BMLeafNode *n1)
	{
		if (!(n0->appFlagBit(NODE_TAG))){
			_overlapNodes0.push_back(n0);
			n0->setAppFlagBit(NODE_TAG);
		}

		if (!(n1->appFlagBit(NODE_TAG))){
			_overlapNodes1.push_back(n1);
			n1->setAppFlagBit(NODE_TAG);
		}
	}

	void BmIsctFacePairAccelerator::gen_sweep_face_events(std::vector<BMLeafNode*> &nset0, std::vector<BMLeafNode*> &nset1, std::vector<SweepEvent> &sevents)
	{
		size_t totNode0 = nset0.size();
		size_t totNode1 = nset1.size();

		std::vector<size_t> offset(totNode0 + totNode1);

		size_t totSEv = 0;
		for (size_t i = 0; i < totNode0; ++i){
			offset[i] = totSEv;
			totSEv += (2 * nset0[i]->faces().size()); /*each face will generate two sweep events*/
		}

		for (size_t i = 0; i < totNode1; ++i){
			offset[i + totNode0] = totSEv;
			totSEv += (2 * nset1[i]->faces().size()); /*each face will generate two sweep events*/
		}

		sevents.resize(totSEv);

		size_t maxDim = node_set_max_dimension(nset0, nset1);

		tbb::task_group group;
		group.run(
			[&]
		{
			tbb::parallel_for(tbb::blocked_range<size_t>(0, totNode0),
				[&](const tbb::blocked_range<size_t>& range)
			{
				for (size_t i = range.begin(); i < range.end(); ++i){
					BMLeafNode *node = nset0[i];
					size_t evOffset = offset[i];
					calc_node_sweep_event(node, 0, sevents, evOffset, maxDim);
				}
			});
		});

		group.run(
			[&]
		{
			tbb::parallel_for(tbb::blocked_range<size_t>(0, totNode1),
				[&](const tbb::blocked_range<size_t>& range)
			{
				for (size_t i = range.begin(); i < range.end(); ++i){
					BMLeafNode *node = nset1[i];
					size_t evOffset = offset[totNode0 + i];
					calc_node_sweep_event(node, 1, sevents, evOffset, maxDim);
				}
			});
		});

		group.wait();

#ifdef _DEBUG
		assert(sevents.size() % 2 == 0);
		for (size_t i = 0; i < sevents.size() - 2; i += 2)
		{
			assert(sevents[i].trig != nullptr && sevents[i + 1].trig != nullptr);
			assert(sevents[i].trig == sevents[i + 1].trig);
			assert(sevents[i].z <= sevents[i + 1].z);
			assert(sevents[i].flag & SweepEvent::LOWER);
			assert(sevents[i + 1].flag & SweepEvent::UPPER);

			assert((sevents[i].flag & SweepEvent::OBJ_0 && sevents[i + 1].flag & SweepEvent::OBJ_0) ||
				(sevents[i].flag & SweepEvent::OBJ_1 && sevents[i + 1].flag & SweepEvent::OBJ_1));
		}

#endif

		/*sort sweep event based on their Z value*/
		tbb::parallel_sort(
			sevents.begin(), sevents.end(),
			[](const SweepEvent &ev0, const SweepEvent &ev1)->bool { return ev0.z < ev1.z; });

#ifdef _DEBUG
		for (size_t i = 0; i < sevents.size() - 1; ++i)
		{
			assert(sevents[i].trig != nullptr);
			assert(sevents[i].z <= sevents[i + 1].z);
		}
#endif

		//debugOutputRectangle(sevents);
	}


	void BmIsctFacePairAccelerator::calc_node_sweep_event(BMLeafNode *node, size_t object, std::vector<SweepEvent>& events, size_t offset, size_t maxDim)
	{
		const BMFaceVector &faces = node->faces();
		const size_t num = faces.size();
		for (size_t i = 0; i < num; ++i){
#ifdef _DEBUG
			/*empty. Not overlapped by other thread*/
			assert(events[offset + 0].trig == nullptr);
			assert(events[offset + 1].trig == nullptr);
#endif
			calc_face_sweep_event(faces[i], object, events[offset], events[offset + 1], maxDim);
			offset += 2;
		}
	}

	void BmIsctFacePairAccelerator::calc_face_sweep_event(BMFace *face, size_t object, SweepEvent &lowerEv, SweepEvent &upperEv, size_t maxDim)
	{
		const float epsilon = 1.0e-4;
		size_t objFlag = (object == 1) ? SweepEvent::OBJ_1 : SweepEvent::OBJ_0;

		Vector3f lower, upper;
		BM_face_calc_bounds(face, lower, upper);

		/*avoid degenerate case*/
		for (size_t d = 0; d < 3; ++d){
			if (std::fabs(upper[d] - lower[d]) < epsilon){
				upper[d] += epsilon;
				lower[d] -= epsilon;
			}
		}

#ifdef _DEBUG
		assert(lower[0] < upper[0]);
		assert(lower[1] < upper[1]);
		assert(lower[2] < upper[2]);
#endif

		BBPoint2f lower2D(lower[(maxDim + 1) % 3], lower[(maxDim + 2) % 3]);
		BBPoint2f upper2D(upper[(maxDim + 1) % 3], upper[(maxDim + 2) % 3]);

		lowerEv.rect = upperEv.rect = BBBox2f(lower2D, upper2D);

		lowerEv.z = lower[maxDim];
		upperEv.z = upper[maxDim];

		lowerEv.flag |= (SweepEvent::LOWER | objFlag);
		upperEv.flag |= (SweepEvent::UPPER | objFlag);

		lowerEv.trig = upperEv.trig = face;
	}

	void BmIsctFacePairAccelerator::match(std::vector<SweepEvent> &sevents, std::vector<std::pair<BMFace*, BMFace*>>& candidates)
	{
		namespace bg = boost::geometry;
		namespace bgi = boost::geometry::index;

		typedef std::pair<BBBox2f, uintptr_t> Value;
		typedef bgi::quadratic<16, 4> parameters;
		typedef bgi::rtree<Value, parameters> Rtree;

		Rtree rtree0;
		Rtree rtree1;

		size_t totEv = sevents.size();

		for (auto i = 0; i < totEv; ++i){

			SweepEvent &ev = sevents[i];

			if (ev.flag & SweepEvent::LOWER){
				if (ev.flag & SweepEvent::OBJ_0){
					/*find overlap rectangle with rtree1*/
					rtree1.query(
						bgi::intersects(ev.rect),
						boost::make_function_output_iterator([&](const Value& val)
					{
						candidates.push_back(std::make_pair(ev.trig, reinterpret_cast<BMFace*>(val.second)));
					}));

					rtree0.insert(Value(ev.rect, reinterpret_cast<uintptr_t>(ev.trig)));
				}
				else{
					assert(ev.flag & SweepEvent::OBJ_1);
					rtree0.query(
						bgi::intersects(ev.rect),
						boost::make_function_output_iterator([&](const Value& val)
					{
						candidates.push_back(std::make_pair(reinterpret_cast<BMFace*>(val.second), ev.trig));
					}));

					rtree1.insert(Value(ev.rect, reinterpret_cast<uintptr_t>(ev.trig)));
				}
			}
			else{
				assert(ev.flag & SweepEvent::UPPER);
				if (ev.flag & SweepEvent::OBJ_0){
					assert(rtree0.remove(Value(ev.rect, reinterpret_cast<uintptr_t>(ev.trig))) == 1);
				}
				else{
					assert(ev.flag & SweepEvent::OBJ_1);
					assert(rtree1.remove(Value(ev.rect, reinterpret_cast<uintptr_t>(ev.trig))) == 1);
				}
			}
		}
	}

	/*
	*fill pairs with intersection point of triangle pair
	*remove non-overlap triangle pairs from pairs
	*/
	void BmIsctFacePairAccelerator::filter_real_tri_tri_isect_pair(std::vector<std::pair<BMFace*, BMFace*>> &pairs)
	{
		std::vector<bool> real_isct_pair(pairs.size(), false);
		const size_t total = pairs.size();

		/*compute intersection points*/
		tbb::parallel_for(tbb::blocked_range<size_t>(0, pairs.size()),
			[&](const tbb::blocked_range<size_t> &range)
		{
			BMVert *tri0_verts[3], *tri1_verts[3];
			for (size_t i = range.begin(); i != range.end(); ++i){
				auto &pair = pairs[i];
				BMFace *t0 = pair.first;
				BMFace *t1 = pair.second;
				BM_face_as_array_vert_tri(t0, tri0_verts);
				BM_face_as_array_vert_tri(t1, tri1_verts);

				int cnt = tri_tri_isct::tri_tri_overlap_test_3d(
					tri0_verts[0]->co.data(), tri0_verts[1]->co.data(), tri0_verts[2]->co.data(),
					tri1_verts[0]->co.data(), tri1_verts[1]->co.data(), tri1_verts[2]->co.data());

				if (cnt == 1){
					real_isct_pair[i] = true;
				}
			}
		});
		
		for (size_t i = 0; i < total; ++i){
			if (!real_isct_pair[i]){
				pairs[i].first = pairs[i].second = nullptr;
			}
		}

		/*remove non-overlap triangle triangle pair*/
		auto split = std::partition(pairs.begin(), pairs.end(),
			[](const std::pair<BMFace*, BMFace*> &it){ return it.first != nullptr && it.second != nullptr; }
		);
		pairs.resize(std::distance(pairs.begin(), split));
	}

	size_t BmIsctFacePairAccelerator::node_set_max_dimension(std::vector<BMLeafNode*> &nset0, std::vector<BMLeafNode*> &nset1)
	{
		Vec3fa lower(std::numeric_limits<float>::infinity());
		Vec3fa upper(-std::numeric_limits<float>::infinity());

		for (auto it = nset0.begin(); it != nset0.end(); ++it){
			lower = smin((*it)->lower(), lower);
			upper = smax((*it)->upper(), upper);
		}

		for (auto it = nset1.begin(); it != nset1.end(); ++it){
			lower = smin((*it)->lower(), lower);
			upper = smax((*it)->upper(), upper);
		}

		Vec3fa diff = upper - lower;
		if (diff[0] > diff[1]){
			if (diff[0] > diff[2])
				return 0;
			else
				return 2;
		}
		else{
			if (diff[1] > diff[2])
				return 1;
			else
				return 2;
		}
	}


	BmBoolean::BmBoolean(
		VM::BMesh *bm0, VBvh::BMBvh *bvh0, VM::BMesh *bm1, VBvh::BMBvh *bvh1)
		:
		_bm0(bm0),
		_bvh0(bvh0),
		_bm1(bm1),
		_bvh1(bvh1),
		_ret_mesh(nullptr)
	{}

	BmBoolean::~BmBoolean()
	{}

	void BmBoolean::compute_union()
	{
		boolean_begin();
		delete_and_flip(BOOLEAN_UNION);
		boolean_end();
	}
	void BmBoolean::compute_differnece()
	{
		boolean_begin();
		delete_and_flip(BOOLEAN_DIFFERENCE);
		boolean_end();
	}
	void BmBoolean::compute_intersection()
	{
		boolean_begin();
		delete_and_flip(BOOLEAN_INTERSECTION);
		boolean_end();
	}

	void BmBoolean::boolean_begin()
	{
		BmIsctFacePairAccelerator isct_pair_finder(_bm0, _bvh0, _bm1, _bvh1, _isct_pairs);
		isct_pair_finder.run();

		if (!_isct_pairs.empty()){
			std::vector<BMFace*> isct_faces;
			_ret_mesh = mesh_join(_bm0, _bm1, _isct_pairs, isct_faces);

			_avg_isct_edge_len = edge_avg_len(isct_faces);

			IsctProblem isct(_ret_mesh, isct_faces);
			isct.resovleIntersections();

			inside_outside_propagate(_ret_mesh);
		}
	}

	void BmBoolean::boolean_end()
	{
		if (_ret_mesh){
			collapse_short_subdivided_isct_edges(_ret_mesh, 0.05f * _avg_isct_edge_len);
			clean(_ret_mesh);
		}
	}

	BMesh* BmBoolean::mesh_join(BMesh* mesh0, BMesh *mesh1, const std::vector<std::pair<VM::BMFace*, VM::BMFace*>> &isct_pairs, std::vector<VM::BMFace*> &isct_faces)
	{
		BMesh *mesh = new BMesh(*mesh0);

		const size_t totfaces0 = mesh0->BM_mesh_faces_total();

		const std::vector<BMVert*> &verts1 = mesh1->BM_mesh_vert_table();
		const size_t			totvert1 = verts1.size();

		const std::vector<BMFace*> &faces1 = mesh1->BM_mesh_face_table();
		const size_t			totfaces1 = faces1.size();

		std::vector<BMVert*> clone_verts(totvert1);
		for (size_t i = 0; i < totvert1; ++i){
			BMVert *v = mesh->BM_vert_create(verts1[i]->co, nullptr, BM_CREATE_NOP);
			clone_verts[i] = v;
			v->no = verts1[i]->no;
		}

		BMVert *tri_verts[3];
		for (size_t i = 0; i < totfaces1; ++i){
			BMLoop *l = BM_FACE_FIRST_LOOP(faces1[i]);
			for (size_t iv = 0; iv < 3; ++iv){
				tri_verts[iv] = clone_verts[BM_elem_index_get(l->v)];
				l = l->next;
			}
			BMFace *f = mesh->BM_face_create_quad_tri(tri_verts[0], tri_verts[1], tri_verts[2], nullptr, nullptr, BM_CREATE_NOP);
			f->no = faces1[i]->no;
		}

		mesh->BM_mesh_elem_table_ensure(BM_VERT | BM_FACE, true);

		const std::vector<BMVert*> &verts = mesh->BM_mesh_vert_table();
		const std::vector<BMFace*> &faces = mesh->BM_mesh_face_table();
		const size_t totfaces = faces.size();

		/*mark face's object on joined mesh*/
		for (size_t i = 0; i < totfaces; ++i){
			if (i < totfaces0){
				BM_elem_app_flag_enable(faces[i], F_OBJECT_0);
			}
			else{
				BM_elem_app_flag_enable(faces[i], F_OBJECT_1);
			}
		}

		/*extract intersection faces on joined mesh*/
		for (auto it = isct_pairs.begin(); it != isct_pairs.end(); ++it){
			BMFace *new_f0 = faces[BM_elem_index_get(it->first)];
			BMFace *new_f1 = faces[BM_elem_index_get(it->second) + totfaces0];
			
			if (!BM_elem_app_flag_test(new_f0, F_VISITED)){
				BM_elem_app_flag_enable(new_f0, F_VISITED);
				isct_faces.push_back(new_f0);
			}

			if (!BM_elem_app_flag_test(new_f1, F_VISITED)){
				BM_elem_app_flag_enable(new_f1, F_VISITED);
				isct_faces.push_back(new_f1);
			}

		}

		for (auto it = isct_faces.begin(); it != isct_faces.end(); ++it){
			BM_elem_app_flag_disable(*it, F_VISITED);
		}

		return mesh;
	}

	VM::BMesh * BmBoolean::result()
	{
		return _ret_mesh;
	}

	void BmBoolean::inside_outside_propagate(VM::BMesh *mesh)
	{
		std::queue<BMFace*> fqueue;

		for (size_t i = 0; i < 2; ++i){

			VBvh::BMBvh *other_bvh	= (i == 0) ? _bvh1 : _bvh0;
			char		object_flag	= (i == 0) ? F_OBJECT_0 : F_OBJECT_1;

			while (true){
				BMFace *seed = nullptr;
				BMFace *f;
				BMIter iter;
				BM_ITER_MESH(f, &iter, mesh, BM_FACES_OF_MESH){
					if (BM_elem_app_flag_test(f, object_flag)){
						if (!BM_elem_app_flag_test(f, F_OUTSIDE | F_INSIDE)){
							seed = f; 
							break;;
						}
					}
				}
				
				if (seed == nullptr) break;


				Vector3f center; BM_face_calc_center_mean(f, center);
				char in_out_side_flag = VBvh::bvh_inside_outside_point(other_bvh, center, 1.0e-5) ? F_INSIDE : F_OUTSIDE;

				fqueue.push(seed);
				BM_elem_app_flag_enable(seed, in_out_side_flag);

				BMLoop *l_iter, *l_first, *l_other;
				while (!fqueue.empty()){
					BMFace *f = fqueue.front();
					fqueue.pop();

					BM_elem_app_flag_enable(f, in_out_side_flag);

					l_iter = l_first = BM_FACE_FIRST_LOOP(f);
					do {
						/*do not go through intersection contour*/
						if(!BM_elem_app_flag_test(l_iter->e, E_SUBDIVIDED_ISECT)){
							
							l_other = (l_iter->radial_next != l_iter) ? l_iter->radial_next : nullptr;
							
							if (l_other && !BM_elem_app_flag_test(l_other->f, in_out_side_flag)){
								BM_elem_app_flag_enable(l_other->f, in_out_side_flag);
								fqueue.push(l_other->f);
							}
						}
					} while ((l_iter = l_iter->next) != l_first);
				}
			}
		}

	}



	bool BmBoolean::boolean_no_delete_check(BMFace *f, BoolType btype)
	{
		switch (btype)
		{
		case BOOLEAN_UNION:
			return BM_elem_app_flag_test(f, F_OUTSIDE);
			break;
		
		case BOOLEAN_DIFFERENCE:
			if (BM_elem_app_flag_test(f, F_OBJECT_0))
				return BM_elem_app_flag_test(f, F_OUTSIDE);
			else
				return BM_elem_app_flag_test(f, F_INSIDE);
			break;

		case BOOLEAN_INTERSECTION:
			return BM_elem_app_flag_test(f, F_INSIDE);
			break;
		
		default:
			break;
		}

		BLI_assert(false);
		return false;
	}

	void BmBoolean::delete_and_flip(BoolType btype)
	{
		VM::BMesh *mesh = _ret_mesh;

		if (mesh){

			std::vector<BMFace*> kill_faces;
			std::vector<BMFace*> flip_faces;

			BMIter iter;
			BMFace *f;
			BM_ITER_MESH(f, &iter, mesh, BM_FACES_OF_MESH){
				if (!boolean_no_delete_check(f, btype)){
					kill_faces.push_back(f);
				}
				else if (boolean_flip_check(f, btype)){
					flip_faces.push_back(f);
				}
			}

			for (auto it = kill_faces.begin(); it != kill_faces.end(); ++it){
				mesh->BM_face_kill_loose(*it, false);
			}

			for (auto it = flip_faces.begin(); it != flip_faces.end(); ++it){
				mesh->BM_face_normal_flip(*it);
			}
		}
	}

	void BmBoolean::clean(VM::BMesh *mesh)
	{
		BMIter iter;
		BMVert *v;
		BMEdge *e;
		BMFace *f;
		BM_ITER_MESH(v, &iter, mesh, BM_VERTS_OF_MESH){
			BM_elem_app_flag_clear(v);
		}
		BM_ITER_MESH(f, &iter, mesh, BM_FACES_OF_MESH){

#ifdef ISCT_TEST
			if (BM_elem_app_flag_test(f, F_OBJECT_0) && BM_elem_app_flag_test(f, F_OUTSIDE)){
				debug_output_tri_segments(f, g_vscene_testSegments);
			}
#endif
			BM_elem_app_flag_clear(f);
		}
		BM_ITER_MESH(e, &iter, mesh, BM_EDGES_OF_MESH){
			BM_elem_app_flag_clear(e);
		}
	}

	bool BmBoolean::boolean_flip_check(BMFace* f, BoolType btype)
	{
		switch (btype)
		{
		case BOOLEAN_UNION:
			return false;
			break;
		case BOOLEAN_DIFFERENCE:
		{
			if (BM_elem_app_flag_test(f, F_OBJECT_0)){
				return false;
			}
			else{
				return true;
			}
			break;
		}
		case BOOLEAN_INTERSECTION:
			return false;
			break;
		}
		assert(false);
		return false;
	}

}
