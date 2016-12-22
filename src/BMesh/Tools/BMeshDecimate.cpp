#include "BMesh/Tools/BMeshDecimate.h"
#include "BaseLib/MathUtil.h"
#include "BaseLib/MathGeom.h"
#include "tbb/parallel_for.h"

VM_BEGIN_NAMESPACE

void BMeshDecimate::setRatio(float rat)
{
	_ratio = rat;
	if (_ratio < 0.1f) _ratio = 0.1f;
	if (_ratio > 1.0f) _ratio = 1.0f;
}

void BMeshDecimate::run()
{
	_bm->BM_mesh_elem_table_ensure(BM_VERT | BM_EDGE | BM_FACE, true);

	_vquadrics.resize(_bm->BM_mesh_verts_total());
	memset(_vquadrics.data(), 0, _vquadrics.size() * sizeof(Qdr::Quadric));
	_eheaphandles.resize(_bm->BM_mesh_edges_total());

	bm_decim_build_quadrics();
	bm_decim_build_edge_cost();

	const int tot_edge_orig = _bm->BM_mesh_edges_total();
	size_t face_tot_target  = (size_t)std::ceil(_bm->BM_mesh_faces_total() * _ratio);
	
	/* simple non-mirror case */
	while ((_bm->BM_mesh_faces_total() > face_tot_target) && (!_eheap.empty()))
	{
		const EHeapNode enode = _eheap.top(); _eheap.pop();
		if (enode.cost  == COST_INVALID) continue;

		// const float value = BLI_heap_node_value(BLI_heap_top(eheap));
		BMEdge *e = enode.e;
		Vector3f optimize_co;
		BLI_assert(BM_elem_index_get(e) < tot_edge_orig);  /* handy to detect corruptions elsewhere */

		/* under normal conditions wont be accessed again,
		* but NULL just incase so we don't use freed node */
		_eheaphandles[BM_elem_index_get(e)] = EHeapHanle();

		bm_decim_edge_collapse(e, optimize_co, true);
	}
}

void BMeshDecimate::bm_decim_build_quadrics()
{
	BMIter iter;
	BMFace *f;
	BMEdge *e;

	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	const size_t totvert = verts.size();

	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, verts.size()),
		[&](tbb::blocked_range<size_t> &range)
	{
		BMIter iter;
		BMFace *f;
		Qdr::Quadric q;
		Vector4d plane_db;
		Vector3f center;

		for (size_t i = range.begin(); i < range.end(); ++i){
			BMVert *v = verts[i];

			BM_ITER_ELEM(f, &iter, v, BM_FACES_OF_VERT){
				BM_face_calc_center_mean(f, center);
				plane_db[0] = f->no[0];
				plane_db[1] = f->no[1];
				plane_db[2] = f->no[2];
				plane_db[3] = -f->no.dot(center);
				q.quadric_from_plane(plane_db);
				_vquadrics[i] += q;
			}
		}
	});

	/* boundary edges */
	BM_ITER_MESH(e, &iter, _bm, BM_EDGES_OF_MESH) {
		if (UNLIKELY(BM_edge_is_boundary(e))) {
			Vector3f edge_vector;
			Vector3f edge_plane;
			Vector4d edge_plane_db;
			edge_vector = e->v2->co, e->v1->co;
			f = e->l->f;

			edge_plane = (edge_vector.cross(f->no)).normalized();
			edge_plane_db[0] = edge_plane[0];
			edge_plane_db[1] = edge_plane[1];
			edge_plane_db[2] = edge_plane[2];


			if ((edge_plane).squaredNorm() > FLT_EPSILON) {
				Qdr::Quadric q;
				Vector3f center;

				center = 0.5f * (e->v1->co + e->v2->co);

				edge_plane_db[3] = -edge_plane.dot(center);
				
				q.quadric_from_plane(edge_plane_db);
				q *= BOUNDARY_PRESERVE_WEIGHT;
				_vquadrics[BM_elem_index_get(e->v1)] += q;
				_vquadrics[BM_elem_index_get(e->v2)] += q;
			}
		}
	}
}

void BMeshDecimate::bm_decim_build_edge_cost_single_heap(BMEdge *e, bool update /*= false*/)
{
	float cost = bm_decim_build_edge_cost_single(e);

	if (update){
		EHeapHanle &ehanle = _eheaphandles[BM_elem_index_get(e)];
		if (ehanle.node_){
			EHeapNode &node = *ehanle;
			if (node.cost < cost){
				_eheap.increase(ehanle, EHeapNode(node.e, cost));
			}
			else{
				_eheap.decrease(ehanle, EHeapNode(node.e, cost));
			}
		}
		else{
			_eheaphandles[BM_elem_index_get(e)] = _eheap.push(EHeapNode(e, cost));
		}
	}
	else{
		_eheaphandles[BM_elem_index_get(e)] = _eheap.push(EHeapNode(e, cost));
	}
}


float BMeshDecimate::bm_decim_build_edge_cost_single(BMEdge *e)
{
	Vector3f optimize_co;
	float cost;

	/* check we can collapse, some edges we better not touch */
	if (BM_edge_is_boundary(e)) {
		if (e->l->f->len == 3) {
			/* pass */
		}
		else {
			/* only collapse tri's */
			//goto clear;
			return COST_INVALID;
		}
	}
	else if (BM_edge_is_manifold(e)) {
		if ((e->l->f->len == 3) && (e->l->radial_next->f->len == 3)) {
			/* pass */
		}
		else {
			/* only collapse tri's */
			//goto clear;
			return COST_INVALID;
		}
	}
	else {
		//goto clear;
		return COST_INVALID;
	}

	bm_decim_calc_target_co(e, optimize_co);

	const Qdr::Quadric &q1 = _vquadrics[BM_elem_index_get(e->v1)];
	const Qdr::Quadric &q2 = _vquadrics[BM_elem_index_get(e->v2)];

	cost = (static_cast<float>(q1.quadric_evaluate(optimize_co)) + static_cast<float>(q2.quadric_evaluate(optimize_co)));

	/* note, 'cost' shouldn't be negative but happens sometimes with small values.
	*  this can cause faces that make up a flat surface to over-collapse, see [#37121] */
	return fabsf(cost);
}

void BMeshDecimate::bm_decim_calc_target_co(BMEdge *e, Vector3f &optimize_co)
{
	/* compute an edge contraction target for edge 'e'
	* this is computed by summing it's vertices quadrics and
	* optimizing the result. */
	Qdr::Quadric q = _vquadrics[BM_elem_index_get(e->v1)] + _vquadrics[BM_elem_index_get(e->v2)];

	if (q.quadric_optimize(optimize_co, OPTIMIZE_EPS)) {
		return;  /* all is good */
	}
	else {
		optimize_co = 0.5f * (e->v1->co + e->v2->co);
	}
}

void BMeshDecimate::bm_decim_build_edge_cost()
{
	const std::vector<BMEdge*> &edges = _bm->BM_mesh_edge_table();
	const size_t totedge = edges.size();
	std::vector<float> ecost(totedge);

	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, totedge),
		[&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			ecost[i] = bm_decim_build_edge_cost_single(edges[i]);
		}
	});

	for (size_t i = 0; i < totedge; ++i){
		_eheaphandles[i] = _eheap.push(EHeapNode(edges[i], ecost[i]));
	}
}

bool BMeshDecimate::bm_decim_edge_collapse(BMEdge *e, Vector3f &optimize_co, bool optimize_co_calc)
{
	int e_clear_other[2];
	BMVert *v_other = e->v1;
	const int v_other_index = BM_elem_index_get(e->v1);
	const int v_clear_index = BM_elem_index_get(e->v2);  /* the vert is removed so only store the index */

	Vector3f v_clear_no;
	v_clear_no = e->v2->no;

	/* when false, use without degenerate checks */
	if (optimize_co_calc) {
		/* disallow collapsing which results in degenerate cases */
		if (UNLIKELY(BM_edge_tri_collapse_is_degenerate_topology(e))) {
			//bm_decim_invalid_edge_cost_single(e, eheap, eheap_table);  /* add back with a high cost */
			return false;
		}

		bm_decim_calc_target_co(e, optimize_co);

		/* check if this would result in an overlapping face */
		if (UNLIKELY(BM_edge_collapse_is_degenerate_flip(e, optimize_co))) {
			//bm_decim_invalid_edge_cost_single(e, eheap, eheap_table);  /* add back with a high cost */
			return false;
		}
	}

	if (bm_edge_collapse(e, e->v2, e_clear_other)){
		/* update collapse info */
		int i;

		//e = NULL;  /* paranoid safety check */

		v_other->co = optimize_co;

		/* remove eheap */
		for (i = 0; i < 2; i++) {
			/* highly unlikely 'eheap_table[ke_other[i]]' would be NULL, but do for sanity sake */
			if ((e_clear_other[i] != -1) && (_eheaphandles[e_clear_other[i]].node_)) {
				_eheap.erase(_eheaphandles[e_clear_other[i]]);
				_eheaphandles[e_clear_other[i]] = EHeapHanle();
			}
		}

		/* update vertex quadric, add kept vertex from killed vertex */
		_vquadrics[v_other_index] += _vquadrics[v_clear_index];

		/* update connected normals */

		/* in fact face normals are not used for progressive updates, no need to update them */
		// BM_vert_normal_update_all(v);
#ifdef USE_VERT_NORMAL_INTERP
		interp_v3_v3v3(v_other->no, v_other->no, v_clear_no, customdata_fac);
		normalize_v3(v_other->no);
#else
		BM_vert_normal_update(v_other);
#endif


		/* update error costs and the eheap */
		if (LIKELY(v_other->e)) {
			BMEdge *e_iter;
			BMEdge *e_first;
			e_iter = e_first = v_other->e;
			do {
				BLI_assert(BM_edge_find_double(e_iter) == NULL);
				bm_decim_build_edge_cost_single_heap(e_iter, true);
			} while ((e_iter = bmesh_disk_edge_next(e_iter, v_other)) != e_first);
		}

		/* this block used to be disabled,
		* but enable now since surrounding faces may have been
		* set to COST_INVALID because of a face overlap that no longer occurs */
#if 1
		/* optional, update edges around the vertex face fan */
		{
			BMIter liter;
			BMLoop *l;
			BM_ITER_ELEM(l, &liter, v_other, BM_LOOPS_OF_VERT) {
				if (l->f->len == 3) {
					BMEdge *e_outer;
					if (BM_vert_in_edge(l->prev->e, l->v))
						e_outer = l->next->e;
					else
						e_outer = l->prev->e;

					BLI_assert(BM_vert_in_edge(e_outer, l->v) == false);

					bm_decim_build_edge_cost_single_heap(e_outer, true);
				}
			}
		}
		/* end optional update */
		return true;
#endif
	}
	else {
		/* add back with a high cost */
		bm_decim_invalid_edge_cost_single(e);
		return false;
	}
}

bool BMeshDecimate::bm_edge_collapse(BMEdge *e_clear, BMVert *v_clear, int r_e_clear_other[2])
{
	BMVert *v_other;

	v_other = BM_edge_other_vert(e_clear, v_clear);
	BLI_assert(v_other != NULL);

	if (BM_edge_is_manifold(e_clear)) {
		BMLoop *l_a, *l_b;
		BMEdge *e_a_other[2], *e_b_other[2];
		bool ok;

		ok = BM_edge_loop_pair(e_clear, &l_a, &l_b);

		BLI_assert(ok == true);
		BLI_assert(l_a->f->len == 3);
		BLI_assert(l_b->f->len == 3);

		/* keep 'v_clear' 0th */
		if (BM_vert_in_edge(l_a->prev->e, v_clear)) {
			e_a_other[0] = l_a->prev->e;
			e_a_other[1] = l_a->next->e;
		}
		else {
			e_a_other[1] = l_a->prev->e;
			e_a_other[0] = l_a->next->e;
		}

		if (BM_vert_in_edge(l_b->prev->e, v_clear)) {
			e_b_other[0] = l_b->prev->e;
			e_b_other[1] = l_b->next->e;
		}
		else {
			e_b_other[1] = l_b->prev->e;
			e_b_other[0] = l_b->next->e;
		}

		/* we could assert this case, but better just bail out */
#if 0
		BLI_assert(e_a_other[0] != e_b_other[0]);
		BLI_assert(e_a_other[0] != e_b_other[1]);
		BLI_assert(e_b_other[0] != e_a_other[0]);
		BLI_assert(e_b_other[0] != e_a_other[1]);
#endif
		/* not totally common but we want to avoid */
		if (ELEM(e_a_other[0], e_b_other[0], e_b_other[1]) ||
			ELEM(e_a_other[1], e_b_other[0], e_b_other[1]))
		{
			return false;
		}

		BLI_assert(BM_edge_share_vert(e_a_other[0], e_b_other[0]));
		BLI_assert(BM_edge_share_vert(e_a_other[1], e_b_other[1]));

		r_e_clear_other[0] = BM_elem_index_get(e_a_other[0]);
		r_e_clear_other[1] = BM_elem_index_get(e_b_other[0]);

		_bm->BM_edge_kill(e_clear);

		v_other->head.hflag |= v_clear->head.hflag;
		_bm->BM_vert_splice(v_other, v_clear);

		e_a_other[1]->head.hflag |= e_a_other[0]->head.hflag;
		e_b_other[1]->head.hflag |= e_b_other[0]->head.hflag;
		_bm->BM_edge_splice(e_a_other[1], e_a_other[0]);
		_bm->BM_edge_splice(e_b_other[1], e_b_other[0]);

		return true;
	}
	else if (BM_edge_is_boundary(e_clear)) {
		/* same as above but only one triangle */
		BMLoop *l_a;
		BMEdge *e_a_other[2];

		l_a = e_clear->l;

		BLI_assert(l_a->f->len == 3);

		/* keep 'v_clear' 0th */
		if (BM_vert_in_edge(l_a->prev->e, v_clear)) {
			e_a_other[0] = l_a->prev->e;
			e_a_other[1] = l_a->next->e;
		}
		else {
			e_a_other[1] = l_a->prev->e;
			e_a_other[0] = l_a->next->e;
		}

		r_e_clear_other[0] = BM_elem_index_get(e_a_other[0]);
		r_e_clear_other[1] = -1;

		_bm->BM_edge_kill(e_clear);

		v_other->head.hflag |= v_clear->head.hflag;
		_bm->BM_vert_splice(v_other, v_clear);

		e_a_other[1]->head.hflag |= e_a_other[0]->head.hflag;
		_bm->BM_edge_splice(e_a_other[1], e_a_other[0]);

		// BM_mesh_validate(bm);

		return true;
	}
	else {
		return false;
	}
}

/* use this for degenerate cases - add back to the heap with an invalid cost,
* this way it may be calculated again if surrounding geometry changes */
void BMeshDecimate::bm_decim_invalid_edge_cost_single(BMEdge *e)
{
	_eheaphandles[BM_elem_index_get(e)] = _eheap.push(EHeapNode(e, COST_INVALID));
}


VM_END_NAMESPACE