#include "VKernel/VSmoothOp.h"
#include "BaseLib/MathUtil.h"
#include <tbb/parallel_for.h>

extern std::vector<Point3Dd> g_vscene_testPoints;

VSmoothOp::VSmoothOp(BMesh *bm)
	:
	_bm(bm)
{
	_bm->BM_mesh_elem_table_ensure(BM_EDGE | BM_VERT, true);

	_totedge = _bm->BM_mesh_edges_total();
	_totvert = _bm->BM_mesh_verts_total();
	_eweight.resize(_totedge);
	_vweight.resize(_totvert);
	_vlocked.resize(_totvert, false);

	_wtype = COTAGANT_WEIGHT;
	_iteration = 2;
	_lambda = 0.7f;
}

VSmoothOp::~VSmoothOp()
{
}

void VSmoothOp::run()
{
	mark_locked_vert();
	compute_weight();
	smooth();
}


void VSmoothOp::mark_locked_vert()
{
	const std::vector<BMEdge*> &edges = _bm->BM_mesh_edge_table();
	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	std::vector<BMVert*> bverts, bbverts;
	for (size_t i = 0; i < _totedge; ++i){
		if (BM_edge_is_boundary(edges[i])){
			size_t idx_v1 = BM_elem_index_get(edges[i]->v1);
			size_t idx_v2 = BM_elem_index_get(edges[i]->v2);
			if (!_vlocked[idx_v1]) bverts.push_back(edges[i]->v1);
			if (!_vlocked[idx_v2]) bverts.push_back(edges[i]->v2);
			_vlocked[idx_v1] = true;
			_vlocked[idx_v2] = true;
		}
	}

	for (auto it = bverts.begin(); it < bverts.end(); ++it){
		BMVert *v = *it;
		BMEdge *e_iter = v->e;
		do {
			BMVert *ov = BM_edge_other_vert(e_iter, v);
			size_t idx = BM_elem_index_get(ov);
			if (!_vlocked[idx]){
				bbverts.push_back(ov);
				_vlocked[idx] = true;
			}
		} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, v)) != v->e);
	}

	//for (auto it = bbverts.begin(); it < bbverts.end(); ++it){
	//	BMVert *v = *it;
	//	BMEdge *e_iter = v->e;
	//	do {
	//		BMVert *ov = BM_edge_other_vert(e_iter, v);
	//		size_t idx = BM_elem_index_get(ov);
	//		_vlocked[idx] = true;
	//	} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, v)) != v->e);
	//}
}

void VSmoothOp::compute_weight()
{
	const std::vector<BMEdge*> &edges = _bm->BM_mesh_edge_table();
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, _totedge),
		[&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			_eweight[i] = compute_edge_weight(edges[i]);
		}
	});



	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, _totvert),
		[&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BMVert *v = verts[i];
			BMEdge *e_iter = v->e;

			float vw = 0.0f;
			do {
				vw += _eweight[BM_elem_index_get(e_iter)];
			} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, v)) != v->e);
			_vweight[i] = 1.0f / vw;
		}
	});

}

/*manifold edge weight*/
float VSmoothOp::compute_edge_weight(BMEdge *e)
{
	BMLoop *floops[2];
	float eweight = 0.0f;

	if (BM_edge_loop_pair(e, &floops[0], &floops[1])){
		for (size_t i = 0; i < 2; ++i){
			BMLoop *l = floops[i];
			BMVert *opv = l->next->next->v;
			BLI_assert(!BM_vert_in_edge(e, opv));
			Vector3f dir0 = (e->v2->co - opv->co).normalized();
			Vector3f dir1 = (e->v1->co - opv->co).normalized();
			float angle = MathUtil::angle_normalized_v3v3(dir0, dir1);
			eweight += 1.0f / std::tan(angle);
		}
	}

	return eweight;
}

void VSmoothOp::smooth()
{
	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	std::vector<Vector3f> newvcoords(_totvert);
	std::vector<Vector3f> vumbrella(_totvert);
	const float damping = 0.5f;

	for (size_t it = 0; it < _iteration; ++it){

		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, _totvert),
			[&](const tbb::blocked_range<size_t> &range)
		{
			for (size_t i = range.begin(); i < range.end(); ++i){
				BMVert *v = verts[i];

				Vector3f &umbrell = vumbrella[i]; umbrell.setZero();
				BMEdge *e_iter = v->e;
				do {
					BMVert *o_v = BM_edge_other_vert(e_iter, v);
					float weight = _eweight[BM_elem_index_get(e_iter)];
					umbrell -= o_v->co * weight;
				} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, v)) != v->e);

				umbrell *= _vweight[i];
				umbrell += v->co;
			}
		});

		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, _totvert),
			[&](const tbb::blocked_range<size_t> &range)
		{
			for (size_t i = range.begin(); i < range.end(); ++i){
				BMVert *v = verts[i];

				if (_vlocked[i]) continue;

				Vector3f uu = Vector3f::Zero();
				float diag = 0.0f;
				BMEdge *e_iter = v->e;
				do {
					float w = _eweight[BM_elem_index_get(e_iter)];
					BMVert *ov = BM_edge_other_vert(e_iter, v);
					uu -= vumbrella[BM_elem_index_get(ov)];
					diag += (w * _vweight[BM_elem_index_get(ov)] + 1.0f) * w;
				} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, v)) != v->e);

				uu *= _vweight[i];
				uu += vumbrella[i];

				diag *= _vweight[i];
				if (diag > 0.0f){
					uu *= 1.0f / diag;
				}

				uu *= 0.25f;

				Vector3f &newco = newvcoords[i];
				newco = v->co;
				newco -= uu;
			}
		});


		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, _totvert),
			[&](const tbb::blocked_range<size_t> &range)
		{
			for (size_t i = range.begin(); i < range.end(); ++i){
				BLI_assert(i == BM_elem_index_get(verts[i]));
				if (_vlocked[i]){
					continue;
				}

				BMVert *v = verts[i];
				v->co = (1.0f - _lambda) *v->co + _lambda * newvcoords[i];
			}
		});
	}
}

void VSmoothOp::setIteration(size_t iter)
{
	if (iter  > 0 && iter < 100)
		_iteration = iter;
}

void VSmoothOp::setLambda(float lambda)
{
	if (lambda > 0.0 && lambda <= 1.0f){
		_lambda = lambda;
	}
}
