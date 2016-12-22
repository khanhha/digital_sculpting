#include "sculpt/BMSplitCollapseOp.h"
#include "BaseLib/MathUtil.h"
#include "BaseLib/MathGeom.h"
#include "VBvh/BMBvhIsect.h"
#include <boost/container/static_vector.hpp>
#include <tbb/parallel_for.h>
#include "sculpt/commonDefine.h"

extern std::vector<Point3Dd> g_vscene_testSegments;
extern std::vector<Point3Dd> g_vscene_testPoints;

BMSplitCollapseOp::BMSplitCollapseOp(StrokeData *sdata)
	:
	_sdata(sdata)
{
	_bm = _sdata->bm;
	_bvh = _sdata->bvh;

	_center			= (_sdata->symn_data.cur_pos);
	_radius			= _sdata->world_radius;
	_sqrRadius		= _radius * _radius;
	_maxEdgeLen		= _sdata->max_edge_len;
	_sqrMaxEdgeLen	= _maxEdgeLen * _maxEdgeLen;
	_minEdgeLen		= _sdata->min_edge_len;
	_sqrMinEdgeLen	= _minEdgeLen * _minEdgeLen;
}

BMSplitCollapseOp::~BMSplitCollapseOp()
{}

void BMSplitCollapseOp::run()
{
	isect_sphere_bm_bvh(_sdata->bvh, _center, _radius, _nodes);

	collapse_short_edges();
	
	split_long_edges();
}

void BMSplitCollapseOp::split_long_edges()
{
	EdgeQueue equeue;
	long_edge_queue_create(equeue);
	subdivide(equeue);
}

void BMSplitCollapseOp::long_edge_queue_create(EdgeQueue &equeue)
{
	mark_tri_node_in_sphere_begin();

	for (auto it = _nodes.begin(); it != _nodes.end(); ++it){
		BMLeafNode *node = *it;
		const BMFaceVector &faces = node->faces();
		const size_t totface = faces.size();

		for (size_t i = 0; i < totface; ++i){
			BMFace *f = faces[i];
			if (BM_elem_app_flag_test(f, F_MARK_DIRTY)){
				long_edge_queue_face_add(equeue, f);
			}
		}
	}

	mark_tri_node_in_sphere_end();
}

void BMSplitCollapseOp::subdivide(EdgeQueue &equeue)
{
	while (!equeue.empty()){
		EdgeNode se = equeue.top(); equeue.pop();
		BMEdge *e;
		if ((e = BM_edge_exists(se.v1, se.v2)) == nullptr) 
			continue;
		edge_queue_disable(e);
		edge_split(equeue, e);
	}
}

bool BMSplitCollapseOp::tri_in_sphere(BMFace *f)
{
	Vector3f closest;
	BMLoop *l = BM_FACE_FIRST_LOOP(f);
	MathGeom::closest_on_tri_to_point_v3(closest, _center, l->v->co, l->next->v->co, l->next->next->v->co);
	return ((closest - _center).squaredNorm() <= _sqrRadius);
}

/*face must be stay in sphere*/
void BMSplitCollapseOp::long_edge_queue_face_add(EdgeQueue &equeue, BMFace *f)
{
	/* Check each edge of the face */
	BMLoop *l_first = BM_FACE_FIRST_LOOP(f);
	BMLoop *l_iter = l_first;
	do {
		const float len_sq = BM_edge_calc_length_squared(l_iter->e);
		if (len_sq > _sqrMaxEdgeLen) {
			long_edge_queue_edge_add_recur(
				equeue,
				l_iter->radial_next, l_iter,
				len_sq, _maxEdgeLen);
		}
	} while ((l_iter = l_iter->next) != l_first);
}

void BMSplitCollapseOp::long_edge_queue_edge_add_recur(EdgeQueue &queue, BMLoop *l_edge, BMLoop *l_end, float len_sq, float limit_len)
{
	/* how much longer we need to be to consider for subdividing
	* (avoids subdividing faces which are only *slightly* skinny) */
	static const float EVEN_EDGE_LEN_THRESHOLD = 1.2f;
	/* how much the limit increases per recursion
	* (avoids performing subdvisions too far away) */
	static const float EVENT_GENERATION_SCALE = 1.6f;

	BLI_assert(len_sq > limit_len * limit_len);

	if (edge_queue_test(l_edge->e)){
		queue.push(EdgeNode(l_edge->e, -len_sq));
		edge_queue_enable(l_edge->e);
	}

	if ((l_edge->radial_next != l_edge)) {

		const float len_sq_cmp = len_sq * EVEN_EDGE_LEN_THRESHOLD;

		limit_len *= EVENT_GENERATION_SCALE;
		const float limit_len_sq = limit_len * limit_len;

		BMLoop *l_iter = l_edge;
		do {
			BMLoop *l_adjacent[2] = { l_iter->next, l_iter->prev };
			for (int i = 0; i < 2; i++) {
				float len_sq_other = BM_edge_calc_length_squared(l_adjacent[i]->e);
				if (len_sq_other > std::max<float>(len_sq_cmp, limit_len_sq)) {
					long_edge_queue_edge_add_recur(
						queue, l_adjacent[i]->radial_next, l_adjacent[i],
						len_sq_other, limit_len);
				}
			}
		} while ((l_iter = l_iter->radial_next) != l_end);
	}
}

void BMSplitCollapseOp::long_edge_queue_edge_add(EdgeQueue &equeue, BMEdge *e)
{
	if (edge_queue_test(e) == false){
		const float len_sq = BM_edge_calc_length_squared(e);
		if (len_sq > _sqrMaxEdgeLen) {
			equeue.push(EdgeNode(e, -len_sq));
		}
	}
}

void BMSplitCollapseOp::edge_split(EdgeQueue &equeue, BMEdge *e)
{
	Vector3f co_mid, no_mid;
	boost::container::static_vector<BMLoop*, 2> edge_loops;
	BMLoop *l_iter = e->l;
	do {
		edge_loops.push_back(l_iter);
	} while ((l_iter = l_iter->radial_next) != e->l);


	/* Get all faces adjacent to the edge */

	/* Create a new vertex in current node at the edge's midpoint */
	co_mid =  0.5f * (e->v1->co + e->v2->co);
	no_mid = (0.5f * (e->v1->no + e->v2->no)).normalized();

	//BMLeafNode *vnode = _bvh->node(e->v1);
	//if (vnode && !vnode->appFlagBit(LEAF_UPDATE_DRAW_BUFFER | LEAF_UPDATE_BB)){
	//	vnode->setAppFlagBit(LEAF_UPDATE_DRAW_BUFFER | LEAF_UPDATE_BB);
	//	_bvh->saveOriginNodeData(vnode);
	//}

	BMVert *v_new = bmesh_vert_create(co_mid, no_mid);

	/* For each face, add two new triangles and delete the original */
	for (int i = 0; i < edge_loops.size(); i++) {
		BMLoop *l_adj = edge_loops[i];
		BMFace *f_adj = l_adj->f;
		BMFace *f_new;
		BMVert *v_opp, *v1, *v2;
		BMVert *v_tri[3];
		BMEdge *e_tri[3];


		BLI_assert(f_adj->len == 3);
		BMLeafNode *fnode = _bvh->elem_leaf_node_get(f_adj);
		leaf_node_test_first_touch(fnode);

		if (i == 0){
			_bvh->leaf_node_vert_add(fnode, v_new);
		}


		/* Find the vertex not in the edge */
		v_opp = l_adj->prev->v;

		/* Get e->v1 and e->v2 in the order they appear in the
		* existing face so that the new faces' winding orders
		* match */
		v1 = l_adj->v;
		v2 = l_adj->next->v;

		/**
		* The 2 new faces created and assigned to ``f_new`` have their
		* verts & edges shuffled around.
		*
		* - faces wind anticlockwise in this example.
		* - original edge is ``(v1, v2)``
		* - oroginal face is ``(v1, v2, v3)``
		*
		* <pre>
		*         + v3(v_opp)
		*        /|\
		*       / | \
		*      /  |  \
		*   e4/   |   \ e3
		*    /    |e5  \
		*   /     |     \
		*  /  e1  |  e2  \
		* +-------+-------+
		* v1      v4(v_new) v2
		*  (first) (second)
		* </pre>
		*
		* - f_new (first):  ``v_tri=(v1, v4, v3), e_tri=(e1, e5, e4)``
		* - f_new (second): ``v_tri=(v4, v2, v3), e_tri=(e2, e3, e5)``
		*/

		/* Create two new faces */
		v_tri[0] = v1;
		v_tri[1] = v_new;
		v_tri[2] = v_opp;
		bm_edges_from_verts(v_tri, e_tri);
		f_new = bvh_bmesh_face_create(fnode, v_tri, e_tri);
		if(tri_in_sphere(f_new)) long_edge_queue_face_add(equeue, f_new);

		v_tri[0] = v_new;
		v_tri[1] = v2;
		/* v_tri[2] = v_opp; */ /* unchanged */
		e_tri[0] = _bm->BM_edge_create(v_tri[0], v_tri[1], NULL, BM_CREATE_NO_DOUBLE);
		e_tri[2] = e_tri[1];  /* switched */
		e_tri[1] = _bm->BM_edge_create(v_tri[1], v_tri[2], NULL, BM_CREATE_NO_DOUBLE);
		f_new = bvh_bmesh_face_create(fnode, v_tri, e_tri);
		if(tri_in_sphere(f_new)) long_edge_queue_face_add(equeue, f_new);

		/* Delete original */
		bvh_bmesh_face_remove(fnode, f_adj);

		if (BM_vert_edge_count(v_opp) >= 8) {
			BMIter bm_iter;
			BMEdge *e2;

			BM_ITER_ELEM(e2, &bm_iter, v_opp, BM_EDGES_OF_VERT) {
				long_edge_queue_edge_add(equeue, e2);
			}
		}
	}

	_bm->BM_edge_kill(e);
}


void BMSplitCollapseOp::leaf_node_test_first_touch(BMLeafNode *node)
{
	_bvh->leaf_node_org_data_save(node);
}


BMVert* BMSplitCollapseOp::bmesh_vert_create(const Vector3f &co, const Vector3f &no)
{
	BMVert *v = _bm->BM_vert_create(co, nullptr, BM_CREATE_NOP);
	_bm->BM_data_vert().CustomData_bmesh_set_default(&v->head.data);
	
	BM_elem_app_flag_enable(v, V_NEW_SUBDIVISION_VERTEX);
	v->no = no;
	return v;
}


BMFace* BMSplitCollapseOp::bvh_bmesh_face_create(BMLeafNode *node, BMVert *verts[3], BMEdge *edges[3])
{
	leaf_node_test_first_touch(node);

	BMFace *f = _bm->BM_face_create(verts, edges, 3, nullptr, BM_CREATE_NO_DOUBLE);
	_bm->BM_data_face().CustomData_bmesh_set_default(&f->head.data);

	_bvh->leaf_node_face_add(node, f);

	/*this face is create during a stroke. mark it*/
	BM_elem_app_flag_enable(f, F_NEW_SUBDIVISION_TRIANGLE);
	return f;
}

void BMSplitCollapseOp::bvh_bmesh_face_remove(BMLeafNode *node, BMFace *face)
{
	leaf_node_test_first_touch(node);

	_bvh->leaf_node_face_remove(node, face);

	if (BM_elem_app_flag_test(face, F_NEW_SUBDIVISION_TRIANGLE)){
		/*this face has been created during a stroke, and now deleted ==> it must be deleted forever*/
		_bm->BM_face_kill(face, false);
	}
	else{
		/*this is an original face ==> log it in mesh for undo/redo later*/
		_bm->BM_face_kill(face, true);
	}
}

void BMSplitCollapseOp::bvh_bmesh_vert_remove(BMVert *v)
{
	BMLeafNode *node = _bvh->elem_leaf_node_get(v);
	leaf_node_test_first_touch(node);

#ifdef _DEBUG
	BLI_assert(!BM_elem_app_flag_test(v, BM_ELEM_REMOVED));
#endif

	_bvh->leaf_node_vert_remove(node, v);
	_bm->BM_vert_kill(v, true);
}

void BMSplitCollapseOp::bm_edges_from_verts(BMVert *v_tri[3], BMEdge *e_tri[3])
{
	e_tri[0] = _bm->BM_edge_create(v_tri[0], v_tri[1], NULL, BM_CREATE_NO_DOUBLE);
	e_tri[1] = _bm->BM_edge_create(v_tri[1], v_tri[2], NULL, BM_CREATE_NO_DOUBLE);
	e_tri[2] = _bm->BM_edge_create(v_tri[2], v_tri[0], NULL, BM_CREATE_NO_DOUBLE);
}

void BMSplitCollapseOp::short_edge_queue_create(EdgeQueue &equeue)
{
	mark_tri_node_in_sphere_begin();

	for (auto it = _nodes.begin(); it != _nodes.end(); ++it){
		BMLeafNode *node = *it;
		const BMFaceVector &faces = node->faces();
		const size_t totface = faces.size();

		for (size_t i = 0; i < totface; ++i){
			BMFace *f = faces[i];
			if (BM_elem_app_flag_test(f, F_MARK_DIRTY)){
				short_edge_queue_face_add(equeue, f);
			}
		}
	}

	mark_tri_node_in_sphere_end();
}

void BMSplitCollapseOp::short_edge_queue_face_add(EdgeQueue &equeue, BMFace *f)
{
#ifdef USE_EDGEQUEUE_FRONTFACE
	if (eq_ctx->q->use_view_normal) {
		if (dot_v3v3(f->no, eq_ctx->q->view_normal) < 0.0f) {
			return;
		}
	}
#endif
	BMLoop *l_iter, *l_first;
	/* Check each edge of the face */
	l_iter = l_first = BM_FACE_FIRST_LOOP(f);
	do {
		short_edge_queue_edge_add(equeue, l_iter->e);
	} while ((l_iter = l_iter->next) != l_first);
}

void BMSplitCollapseOp::short_edge_queue_edge_add(EdgeQueue &equeue, BMEdge *e)
{
	if (edge_queue_test(e)){
		const float len_sq = BM_edge_calc_length_squared(e);
		if (len_sq < _sqrMinEdgeLen) {
			edge_queue_enable(e);
			equeue.push(EdgeNode(e, len_sq));
		}
	}
}

void BMSplitCollapseOp::mark_tri_node_in_sphere_begin()
{
	auto mark_in_sphere_face_node = [&](BMLeafNode *node)
	{
		const BMFaceVector &faces = node->faces();
		const size_t totface = faces.size();
		for (size_t j = 0; j < totface; ++j){
			if (tri_in_sphere(faces[j])){
				BM_elem_app_flag_enable(faces[j], F_MARK_DIRTY);
			}
		}
	};

	size_t totfaces = 0;
	for (auto it = _nodes.begin(); it != _nodes.end(); ++it){
		BMLeafNode *node = *it;
		totfaces += node->faces().size();
	}

	if (totfaces > 400 && _nodes.size() >= 2){
		tbb::parallel_for((size_t)0, _nodes.size(), [&](size_t i){
			mark_in_sphere_face_node(_nodes[i]);
		});
	}
	else{
		for (auto it = _nodes.begin(); it != _nodes.end(); ++it){
			mark_in_sphere_face_node(*it);
		}
	}
}

void BMSplitCollapseOp::mark_tri_node_in_sphere_end()
{
	for (auto it = _nodes.begin(); it != _nodes.end(); ++it){
		BMLeafNode *node = *it;
		const BMFaceVector &faces = node->faces();
		const size_t totface = faces.size();
		for (size_t j = 0; j < totface; ++j){
			BM_elem_app_flag_disable(faces[j], F_MARK_DIRTY);
		}
	}
}

bool BMSplitCollapseOp::collapse_equeue(EdgeQueue &equeue
#ifdef OPTIMIZE_COLLAPSE
	, std::vector<Qdr::Quadric> &vquadric
#endif
	)
{
	const float min_len_squared = _sqrMinEdgeLen;
	bool any_collapsed = false;
	Qdr::Quadric quadric;
	Vector3f opt_co;
	FaceSmallBuffer deleted_faces;
	while (!equeue.empty()) {
		const EdgeNode enode = equeue.top(); equeue.pop();

		BMVert *v1 = enode.v1, *v2 = enode.v2;

		/* Check the verts still exist */
		if (BM_elem_app_flag_test(v1, BM_ELEM_REMOVED) ||
			BM_elem_app_flag_test(v2, BM_ELEM_REMOVED))
		{
			continue;
		}

		/* Check that the edge still exists */
		BMEdge *e;
		if (!(e = BM_edge_exists(v1, v2))) {
			continue;
		}

		edge_queue_disable(e);

		if ((v1->co - v2->co).squaredNorm() >= min_len_squared)
			continue;

		/* Check that the edge's vertices are still in the PBVH. It's
		* possible that an edge collapse has deleted adjacent faces
		* and the node has been split, thus leaving wire edges and
		* associated vertices. */
#if 0
		if ((BM_ELEM_CD_GET_INT(e->v1, eq_ctx->cd_vert_node_offset) == DYNTOPO_NODE_NONE) ||
			(BM_ELEM_CD_GET_INT(e->v2, eq_ctx->cd_vert_node_offset) == DYNTOPO_NODE_NONE))
		{
			continue;
		}
#endif
		any_collapsed = true;
#ifdef OPTIMIZE_COLLAPSE
		Qdr::Quadric &vquad1 = vquadric[BM_elem_index_get(e->v1)];
		Qdr::Quadric &vquad2 = vquadric[BM_elem_index_get(e->v2)];
		quadric = vquad1 + vquad2;
		if(!quadric.quadric_optimize(opt_co, 0.01f)){
			opt_co = 0.5f * (e->v1->co + e->v2->co);
		}
#else
		opt_co = 0.5f * (e->v1->co + e->v2->co);
#endif	
		collapse_edge(e, deleted_faces, opt_co);

	}

	return any_collapsed;
}

void BMSplitCollapseOp::collapse_edge(BMEdge *e, FaceSmallBuffer &deleted_faces, const Vector3f &optimize_co)
{
	BMVert *v_del, *v_conn;
	size_t valence_1 = BM_vert_edge_count(e->v1);
	size_t valence_2 = BM_vert_edge_count(e->v2);
	if (valence_1 > valence_2){
		v_del = e->v1; v_conn = e->v2;
	}
	else{
		v_del = e->v2;v_conn = e->v1;
	}


	/*Remove all faces adjacent to the edge */
	BMLoop *l_adj;
	while ((l_adj = e->l)) {
		BMFace *f_adj = l_adj->f;
		bvh_bmesh_face_remove(_bvh->elem_leaf_node_get(f_adj), f_adj);
	}

	/* Kill the edge */
	BLI_assert(BM_edge_is_wire(e));
	_bm->BM_edge_kill(e);

	/* For all remaining faces of v_del, create a new face that is the
	* same except it uses v_conn instead of v_del */
	/* Note: this could be done with BM_vert_splice(), but that
	* requires handling other issues like duplicate edges, so doesn't
	* really buy anything. */
	deleted_faces.clear();

	BMIter bm_iter;
	BMFace *f;
	BMVert *v_tri[3];
	BMEdge *e_tri[3];

	BM_ITER_ELEM(f, &bm_iter, v_del, BM_FACES_OF_VERT) {
		BMFace *existing_face = nullptr;
		/* Get vertices, replace use of v_del with v_conn */
		BM_face_as_array_vert_tri(f, v_tri);
		for (int i = 0; i < 3; i++) {
			if (v_tri[i] == v_del) {
				v_tri[i] = v_conn;
			}
		}

		/* Check if a face using these vertices already exists. If so,
		* skip adding this face and mark the existing one for
		* deletion as well. Prevents extraneous "flaps" from being
		* created. */
		if (BM_face_exists(v_tri, 3, &existing_face)) {
			BLI_assert(existing_face);
			deleted_faces.push_back(existing_face);
		}
		else {
			BMLeafNode *n = _bvh->elem_leaf_node_get(f);
			bm_edges_from_verts(v_tri, e_tri);
			bvh_bmesh_face_create(n, v_tri, e_tri);
		}

		deleted_faces.push_back(f);
	}

	/* Delete the tagged faces */
	for (auto it = deleted_faces.begin(); it != deleted_faces.end(); it++) {
		BMFace *f_del =  *it;

		/* Get vertices and edges of face */
		BLI_assert(f_del->len == 3);
		BMLoop *l_iter = BM_FACE_FIRST_LOOP(f_del);
		BMVert *v_tri[3];
		BMEdge *e_tri[3];
		v_tri[0] = l_iter->v; e_tri[0] = l_iter->e; l_iter = l_iter->next;
		v_tri[1] = l_iter->v; e_tri[1] = l_iter->e; l_iter = l_iter->next;
		v_tri[2] = l_iter->v; e_tri[2] = l_iter->e;

		/* Remove the face */
		bvh_bmesh_face_remove(_bvh->elem_leaf_node_get(f_del), f_del);
		
		/* Check if any of the face's edges are now unused by any
		* face, if so delete them */
		for (int j = 0; j < 3; j++) {
			if (BM_edge_is_wire(e_tri[j]))
				_bm->BM_edge_kill(e_tri[j]);
		}

		/* Check if any of the face's vertices are now unused, if so
		* remove them from the PBVH */
		for (int j = 0; j < 3; j++) {
			if ((v_tri[j] != v_del) && (v_tri[j]->e == NULL)) {
				bvh_bmesh_vert_remove(v_tri[j]);
			}
		}
	}


	/* Move v_conn to the midpoint of v_conn and v_del (if v_conn still exists, it
	* may have been deleted above) */
	if (!BM_elem_flag_test(v_conn, BM_ELEM_REMOVED)){
		v_conn->co = optimize_co;
		v_conn->no = (v_conn->no + v_del->no).normalized();
	}

	/* Delete v_del */
	BLI_assert(!BM_vert_face_check(v_del));
	//BLI_gset_insert(deleted_verts, v_del);
	//BM_log_vert_removed(bvh->bm_log, v_del, eq_ctx->cd_vert_mask_offset);
	//BM_vert_kill(bvh->bm, v_del);

	bvh_bmesh_vert_remove(v_del);
}

void BMSplitCollapseOp::collapse_short_edges()
{
	EdgeQueue equeue;
	std::vector<Qdr::Quadric> vquadric;
	short_edge_queue_create(equeue);
#ifdef OPTIMIZE_COLLAPSE
	vert_quadric_compute(equeue, vquadric);
	collapse_equeue(equeue, vquadric);
#else
	collapse_equeue(equeue);
#endif
}

#ifdef OPTIMIZE_COLLAPSE
void BMSplitCollapseOp::vert_quadric_compute(EdgeQueue &equeue, std::vector<Qdr::Quadric> &vquadric)
{
	std::vector<BMVert*> verts; verts.reserve(equeue.size() * 2);
	for (auto it = equeue.begin(); it != equeue.end(); ++it){
		if (!BM_elem_app_flag_test(it->v1, V_MARK_DIRTY)){
			BM_elem_app_flag_enable(it->v1, V_MARK_DIRTY);
			verts.push_back(it->v1);
		}
		
		if (!BM_elem_app_flag_test(it->v2, V_MARK_DIRTY)){
			BM_elem_app_flag_enable(it->v2, V_MARK_DIRTY);
			verts.push_back(it->v2);
		}
	}
	
	vquadric.resize(verts.size());
	
	auto compute_vquadric = [&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BMVert *v = verts[i];
			BM_elem_index_set(v, i);
			BM_elem_app_flag_disable(v, V_MARK_DIRTY);
			BM_vert_quadric_calc(v, vquadric[i]);
		}
	};

	tbb::blocked_range<size_t> total_range(0, verts.size());
	if (verts.size() > 300){
		tbb::parallel_for(total_range, compute_vquadric);
	}
	else{
		compute_vquadric(total_range);
	}

	_bm->BM_mesh_elem_index_dirty_set(BM_VERT);
}
#endif




