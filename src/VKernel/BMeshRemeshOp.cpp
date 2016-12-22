#include "VKernel/BMeshRemeshOp.h"
#include "BaseLib/MathUtil.h"
#include "BaseLib/MathGeom.h"
#include <QElapsedTimer>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include "tbb/tick_count.h"
#include "tbb/parallel_sort.h"
#include "BMesh/Tools/BMeshDiffCurvature.h"
#include <boost/algorithm/clamp.hpp>
#include "Vbvh/BMBvhIsect.h"
using namespace VM;
using namespace  Eigen;

#if 0
extern std::vector<Point3Dd> g_vscene_testSegments;
extern std::vector<Point3Dd> g_vscene_testSegments_1;
extern std::vector<Point3Dd> g_vscene_testPoints;
extern std::vector<Point3Dd> g_vscene_testTriangles;
#endif

BMeshRemeshOp::BMeshRemeshOp(BMesh *bm_, BMBvh *bvh_, bool feature)
	:
	_refer_bvh(bvh_),
	_bm(bm_),
	_feature_ensure(feature),
	_iteration(1)
{
}

BMeshRemeshOp::~BMeshRemeshOp()
{
}

void BMeshRemeshOp::run()
{
	//BMeshSimplify simplify(bm, 0.8f);
	//simplify.run();

	begin();

	if (_feature_ensure){
		featureDetect();
	}

	{
		BMeshDiffCurvature dif(_bm, _cd_v_curvature);
		dif.compute();
	}

	for (size_t i = 0; i < _iteration; ++i){
		//longEdgeSplit();
		collapseShortEdges();
		optimizeValence();
		tangentialSmooth();
	}

	projectOriginMesh();

	end();
}

void BMeshRemeshOp::splitLongEdges()
{
	std::vector<BMEdge*> cedges;
	BMIter iter;
	BMEdge *e, *ne;
	BMVert *nv;

	BM_ITER_MESH(e, &iter, _bm, BM_EDGES_OF_MESH){
		if (isTooLong(e->v1, e->v2)){
			cedges.push_back(e);
		}
	}

	tbb::parallel_sort(
		cedges.begin(), cedges.end(),
		[](BMEdge *e0, BMEdge *e1)->bool
	{
		return BM_edge_calc_length_squared(e0) > BM_edge_calc_length_squared(e1);
	});

	for (auto it = cedges.begin(); it != cedges.end(); ++it){
		e = *it;
		float inter_curvature = 0.5f * (BM_ELEM_CD_GET_FLOAT(e->v1, _cd_v_curvature) + BM_ELEM_CD_GET_FLOAT(e->v2, _cd_v_curvature));
		nv = nullptr; ne = nullptr;
		_bm->BM_face_tri_edge_split(e, &nv, &ne);
		if (nv && ne){
			BM_ELEM_CD_SET_FLOAT(nv, _cd_v_curvature, inter_curvature);
			if (_feature_ensure){
				if (BM_elem_app_flag_test(e, REMESH_E_FEATURE)){
					BM_elem_app_flag_enable(nv, REMESH_V_FEATURE);
					BM_elem_app_flag_enable(ne, REMESH_E_FEATURE);
				}
			}
		}
	}
}

void BMeshRemeshOp::collapseShortEdges()
{
	std::vector<std::pair<BMVert*, BMVert*>> cedges;
	_bm->BM_mesh_elem_index_ensure(BM_VERT);

	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	const size_t totvert = verts.size();
	std::vector<bool> vdel(totvert, false);
	std::vector<int>  vvalence(totvert, -1);

	tbb::parallel_for(tbb::blocked_range<size_t>(0, totvert),
		[&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			vvalence[i] = BM_vert_edge_count(verts[i]);
		}
	});


	BMIter iter;
	BMEdge *e, *adj_e;
	BM_ITER_MESH(e, &iter, _bm, BM_EDGES_OF_MESH){
		if (isTooShort(e->v1, e->v2)){
			cedges.push_back(std::make_pair(e->v1, e->v2));
		}
	}

#if 1
	tbb::parallel_sort(
		cedges.begin(), cedges.end(),
		[](const std::pair<BMVert*, BMVert*> &e0, const std::pair<BMVert*, BMVert*> &e1)->bool
	{
		return (e0.first->co - e0.second->co).squaredNorm()  > (e1.first->co - e1.second->co).squaredNorm();
	});
#endif

	Vector3f co[2];

	for (auto it = cedges.begin(); it != cedges.end(); ++it){
		BMVert *v0 = it->first;
		BMVert *v1 = it->second;
		BMEdge *e = nullptr;

		if (vdel[BM_elem_index_get(v0)] || vdel[BM_elem_index_get(v1)])
			continue;

		if (BM_elem_app_flag_test(v0, REMESH_V_BOUNDARY) || BM_elem_app_flag_test(v1, REMESH_V_BOUNDARY))
			continue;

		if (_feature_ensure){
			/*do not collapse if one of vertices is corner feature*/
			if (BM_elem_app_flag_test(v0, REMESH_V_CORNER) || BM_elem_app_flag_test(v1, REMESH_V_CORNER))
				continue;

			/*both vertex are feature or not*/
			if (BM_elem_app_flag_test(v0, REMESH_V_FEATURE) != BM_elem_app_flag_test(v1, REMESH_V_FEATURE))
				continue;
		}

		if (isTooShort(v0, v1) && (e = BM_edge_exists(v0, v1))){

			/*collapse to higher valence vertex for the sake of performance*/
			if (vvalence[BM_elem_index_get(v0)] < vvalence[BM_elem_index_get(v1)]){
				std::swap(v0, v1);
			}

			BM_vert_calc_mean(e->v1, co[0]);
			BM_vert_calc_mean(e->v2, co[1]);
			Vector3f collapse_co = 0.5f * (co[0] + co[1]);
			MathGeom::closest_to_line_segment_v3(collapse_co, collapse_co, e->v1->co, e->v2->co);

			if (!BM_edge_tri_collapse_is_degenerate_topology(e) &&
				!BM_edge_collapse_is_degenerate_flip(e, collapse_co)){

				bool collapse_create_long_edge = false;
				BM_ITER_ELEM(adj_e, &iter, v1, BM_EDGES_OF_VERT){
					BMVert *v = BM_edge_other_vert(adj_e, v1);
					if (isTooLong(v, v0)){
						collapse_create_long_edge = true;
						break;
					}
				}

				if (!collapse_create_long_edge){
					vdel[BM_elem_index_get(v1)] = true;
					if (_bm->BM_edge_tri_collapse(e, v1)){
						v0->co = collapse_co;
					}
				}
			}
		}
	}
}


void BMeshRemeshOp::optimizeValence()
{
	int val0, val1, val2, val3;
	int val_opt0, val_opt1, val_opt2, val_opt3;
	int ve0, ve1, ve2, ve3, ve_before, ve_after;
	bool done = true;

	_bm->BM_mesh_elem_table_ensure(BM_EDGE | BM_VERT, true);
	const std::vector<BMEdge*> &edges = _bm->BM_mesh_edge_table();
	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	const size_t totvert = verts.size();
	std::vector<int> vvalence(totvert, -1);

	tbb::parallel_for(tbb::blocked_range<size_t>(0, totvert),
		[&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BLI_assert(BM_elem_index_get(verts[i]) == i);
			vvalence[i] = BM_vert_edge_count(verts[i]);
		}
	});

	BMFace *tri1, *tri2;
	for (size_t i = 0; i < 5; ++i){

		for (auto it = edges.begin(); it != edges.end(); ++it){

			BMEdge *e = *it;

			if (_feature_ensure && BM_elem_app_flag_test(e, REMESH_E_FEATURE))
				continue;

			if (BM_edge_face_pair(e, &tri1, &tri2)){
				BMVert *v0 = e->v1;
				BMVert *v1 = e->v2;

				BMVert *v2 = e->l->next->next->v;
				BMVert *v3 = e->l->radial_next->next->next->v;

				val0 = vvalence[BM_elem_index_get(v0)];
				val1 = vvalence[BM_elem_index_get(v1)];
				val2 = vvalence[BM_elem_index_get(v2)];
				val3 = vvalence[BM_elem_index_get(v3)];

				val_opt0 = (BM_elem_app_flag_test(v0, REMESH_V_BOUNDARY) ? 4 : 6);
				val_opt1 = (BM_elem_app_flag_test(v1, REMESH_V_BOUNDARY) ? 4 : 6);
				val_opt2 = (BM_elem_app_flag_test(v2, REMESH_V_BOUNDARY) ? 4 : 6);
				val_opt3 = (BM_elem_app_flag_test(v3, REMESH_V_BOUNDARY) ? 4 : 6);

				ve0 = (val0 - val_opt0);
				ve0 *= ve0;
				ve1 = (val1 - val_opt1);
				ve1 *= ve1;
				ve2 = (val2 - val_opt2);
				ve2 *= ve2;
				ve3 = (val3 - val_opt3);
				ve3 *= ve3;

				ve_before = ve0 + ve1 + ve2 + ve3;

				--val0;
				--val1;
				++val2;
				++val3;

				ve0 = (val0 - val_opt0);
				ve0 *= ve0;
				ve1 = (val1 - val_opt1);
				ve1 *= ve1;
				ve2 = (val2 - val_opt2);
				ve2 *= ve2;
				ve3 = (val3 - val_opt3);
				ve3 *= ve3;

				ve_after = ve0 + ve1 + ve2 + ve3;

				if (ve_before > ve_after) {
					if (_bm->BM_edge_tri_flip(e, BMesh::BM_EDGEROT_CHECK_DEGENERATE | BMesh::BM_EDGEROT_CHECK_EXISTS)){

						done = false;

						vvalence[BM_elem_index_get(v0)]--;
						vvalence[BM_elem_index_get(v1)]--;
						vvalence[BM_elem_index_get(v2)]++;
						vvalence[BM_elem_index_get(v3)]++;
					}
				}
			}
		}/*end*/

		if (done){
			/*earlier finish*/
			break;
		}
	}
}


bool BMeshRemeshOp::isTooShort(BMVert *v0, BMVert *v1)
{
#if 0
	float emin = 0.7 * _eavg;
	return ((v0->co - v1->co).squaredNorm() < (emin * emin));
#else

	float c = 0.5 * BM_ELEM_CD_GET_FLOAT(v0, _cd_v_curvature) + BM_ELEM_CD_GET_FLOAT(v1, _cd_v_curvature);

	float e = 2.0 / c*_error - _error*_error;
	e = (e <= 0.0 ? _error : 2.0*sqrt(e));

	e = boost::algorithm::clamp(e, _emin, _emax);
	float emin = 4.0 / 5.0 * e;

	return ((v1->co - v0->co).squaredNorm() < emin*emin);
#endif
}

bool BMeshRemeshOp::isTooLong(BMVert *v0, BMVert *v1)
{
#if 0
	float emax = 1.4 * _eavg;
	return ((v0->co - v1->co).squaredNorm() > (emax * emax));
#else
	float c = 0.5f * (BM_ELEM_CD_GET_FLOAT(v0, _cd_v_curvature)) + (BM_ELEM_CD_GET_FLOAT(v1, _cd_v_curvature));

	float e = 2.0 / c*_error - _error *_error;
	e = (e <= 0.0 ? _error : 2.0*std::sqrtf(e));

	e = boost::algorithm::clamp(e, _emin, _emax);
	float emax = 4.0 / 3.0 * e;

	return ((v0->co - v1->co).squaredNorm() > emax*emax);
#endif
}

void BMeshRemeshOp::begin()
{
	_bm->BM_mesh_elem_table_ensure(BM_VERT | BM_FACE | BM_EDGE, true);

	{
		_eavg = 0.0;
		_emax = FLT_MIN;
		_emin = FLT_MAX;

		BMEdge *e;
		BMIter iter;
		BM_ITER_MESH(e, &iter, _bm, BM_EDGES_OF_MESH){
			float len = BM_edge_calc_length(e);
			_eavg += len;
			_emin = std::min<float>(len, _emin);
			_emax = std::max<float>(len, _emax);
		}

		_eavg /= _bm->BM_mesh_edges_total();
		_error = 0.1 * _eavg;
	}

	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	const std::vector<BMEdge*> &edges = _bm->BM_mesh_edge_table();
	const size_t totvert = verts.size();
	const size_t totedge = edges.size();
	const size_t bflag = REMESH_V_BOUNDARY;

	auto mark_vert_functor =
		[&verts, bflag](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			/*clear*/
			BM_elem_app_flag_clear(verts[i]);
			BM_elem_flag_disable(verts[i], BM_ELEM_TAG); /*collapse edge use BM_ELEM_TAG*/

			if (BM_vert_is_boundary(verts[i])){
				BM_elem_app_flag_enable(verts[i], bflag);
			}
		}
	};

	auto clear_edge_functor =
		[&edges](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BM_elem_app_flag_clear(edges[i]);
			BM_elem_flag_disable(edges[i], BM_ELEM_TAG); /*collapse edge use BM_ELEM_TAG*/
		}
	};

	tbb::blocked_range<size_t> vrange(0, totvert);
	tbb::blocked_range<size_t> erange(0, totedge);
	if (totvert > 5000){
		tbb::parallel_for(vrange, mark_vert_functor);
	}
	else{
		mark_vert_functor(vrange);
	}

	if (totedge > 10000){
		tbb::parallel_for(erange, clear_edge_functor);
	}
	else{
		clear_edge_functor(erange);
	}

//#ifdef _DEBUG
//	{
//		BMIter iter;
//		BMVert *v;
//		BMEdge *e;
//		BMFace *f;
//		BM_ITER_MESH(v, &iter, _bm, BM_VERTS_OF_MESH){
//			BM_CHECK_ELEMENT(v);
//		}
//		BM_ITER_MESH(e, &iter, _bm, BM_EDGES_OF_MESH){
//			//BLI_assert(BM_edge_is_manifold(e));
//			BM_CHECK_ELEMENT(e);
//		}
//
//		BMVert *verts[3];
//		BM_ITER_MESH(f, &iter, _bm, BM_FACES_OF_MESH){
//			BM_face_as_array_vert_tri(f, verts);
//			BLI_assert(f->len == 3);
//			BM_CHECK_ELEMENT(f);
//		}
//	}
//#endif


	{
		_bm->BM_data_layer_add_named(BM_VERT, CD_PROP_FLT, "v_curvature");
		_cd_v_curvature = _bm->BM_data_get_offset(BM_VERT, CD_PROP_FLT);
	}
}

void BMeshRemeshOp::end()
{
	_bm->BM_data_layer_free(BM_VERT, CD_PROP_FLT);

	_bm->BM_mesh_elem_table_ensure(BM_VERT | BM_EDGE | BM_FACE, true);
	_bm->BM_mesh_normals_update_parallel();

	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	const std::vector<BMEdge*> &edges = _bm->BM_mesh_edge_table();
	const size_t totvert = verts.size();
	const size_t totedge = edges.size();

	auto clear_vert_functor =
		[&verts](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BM_elem_app_flag_clear(verts[i]);
			BM_elem_flag_disable(verts[i], BM_ELEM_TAG);
		}
	};

	auto clear_edge_functor =
		[&edges](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BM_elem_app_flag_clear(edges[i]);
		}
	};

	tbb::blocked_range<size_t> vrange(0, totvert);
	tbb::blocked_range<size_t> erange(0, totedge);
	if (totvert > 10000){
		tbb::parallel_for(vrange, clear_vert_functor);
	}
	else{
		clear_vert_functor(vrange);
	}

	if (totedge > 10000){
		tbb::parallel_for(erange, clear_edge_functor);
	}
	else{
		clear_edge_functor(erange);
	}

}

void BMeshRemeshOp::tangentialSmooth()
{
	_bm->BM_mesh_normals_update_area();

	_bm->BM_mesh_elem_table_ensure(BM_VERT);
	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	std::vector<Vector3f> v_co_prop(verts.size());

	for (size_t i = 0; i < 1; ++i){

		tbb::parallel_for(tbb::blocked_range<size_t>(0, verts.size()),
			[&](const tbb::blocked_range<size_t> &range)
		{
			BMIter iter;
			BMVert *o_v;
			BMEdge *e;
			size_t valence = 0;
			Vector3f avg;
			for (size_t i = range.begin(); i != range.end(); ++i){

				valence = 0; avg.setZero();
				BMVert *v = verts[i];

				if ((_feature_ensure && BM_elem_app_flag_test(v, REMESH_V_CORNER | REMESH_V_FEATURE)) ||
					(BM_elem_app_flag_test(v, REMESH_V_BOUNDARY)))
					continue;

				BM_ITER_ELEM(e, &iter, v, BM_EDGES_OF_VERT){
					valence++;
					o_v = BM_edge_other_vert(e, v);
					avg += o_v->co;
				}

				avg /= static_cast<float>(valence);
				v_co_prop[i/*BM_elem_index_get(v)*/] = avg + v->no.dot(v->co - avg) * v->no;
			}
		});

		tbb::parallel_for(tbb::blocked_range<size_t>(0, verts.size()),
			[&](const tbb::blocked_range<size_t> &range)
		{
			for (size_t i = range.begin(); i != range.end(); ++i){
				BMVert *v = verts[i];

				if ((_feature_ensure && BM_elem_app_flag_test(v, REMESH_V_CORNER | REMESH_V_FEATURE)) ||
					(BM_elem_app_flag_test(v, REMESH_V_BOUNDARY)))
					continue;

				//g_vscene_testPoints.push_back(MathUtil::convert(v->co));
				//g_vscene_testSegments_1.push_back(MathUtil::convert(v->co));
				//g_vscene_testSegments_1.push_back(MathUtil::convert(v_co_prop[i]));

				//g_vscene_testSegments.push_back(MathUtil::convert(v->co));
				//g_vscene_testSegments.push_back(MathUtil::convert(v->co + 0.1f * v->no));
				v->co = v_co_prop[i];
			}
		});
	}


}

void BMeshRemeshOp::featureDetect()
{
	/*g_vscene_testSegments_1.clear();
	g_vscene_testPoints.clear();
	*/
	const std::vector<BMEdge*> &edges = _bm->BM_mesh_edge_table();
	const size_t totedge = edges.size();
	const float  eFeatureThreshold = M_PI / 5.0f;
	for (size_t i = 0; i < totedge; ++i){
		BMEdge *e = edges[i];
		float dihedral = BM_edge_calc_face_angle(e);
		if (dihedral > eFeatureThreshold){
			BM_elem_app_flag_enable(e, REMESH_E_FEATURE);
			/*g_vscene_testSegments_1.push_back(MathUtil::convert(e->v1->co));
			g_vscene_testSegments_1.push_back(MathUtil::convert(e->v2->co));*/
		}
	}


	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	const size_t totvert = verts.size();
	BMEdge *e_iter;
	BMVert *v;
	size_t feature_edge = 0;
	for (size_t i = 0; i < totvert; ++i){
		v = verts[i];
		feature_edge = 0;
		e_iter = v->e;
		do {
			if (BM_elem_app_flag_test(e_iter, REMESH_E_FEATURE) != 0){
				feature_edge++;
			}
		} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, v)) != v->e);

		if (feature_edge > 0){
			if (feature_edge <= 2){
				BM_elem_app_flag_enable(v, REMESH_V_FEATURE);
				//g_vscene_testPoints.push_back(MathUtil::convert(v->co));
			}
			else if (feature_edge > 2){
				BM_elem_app_flag_enable(v, REMESH_V_CORNER);
				//g_vscene_testPoints.push_back(MathUtil::convert(v->co));
			}
		}
	}
}

void BMeshRemeshOp::projectOriginMesh()
{
	_bm->BM_mesh_elem_index_ensure(BM_VERT);
	_bm->BM_mesh_elem_table_ensure(BM_VERT);
	const std::vector<BMVert*> &verts = _bm->BM_mesh_vert_table();
	const size_t totvert = verts.size();

	const float extrude = _eavg;
	const float sqrExtruede = extrude * extrude;
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, totvert),
		[&](tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BMVert *v = verts[i];

			if ((BM_elem_app_flag_test(v, REMESH_V_BOUNDARY)) || (_feature_ensure && BM_elem_app_flag_test(v, REMESH_V_CORNER | REMESH_V_FEATURE)))
				continue;

			Vector3f orgray = v->co + _eavg * v->no;
			Vector3f dir = (v->co - orgray).normalized();
			Vector3f hit;
			if (isect_ray_bm_bvh_nearest_hit(_refer_bvh, orgray, dir, false, hit, 1.0e-5)){
				if ((v->co - hit).squaredNorm() < extrude){
					v->co = hit;
				}
			}
		}
	});
}