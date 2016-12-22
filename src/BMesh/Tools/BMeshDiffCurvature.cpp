#include "Tools/BMeshDiffCurvature.h"
#include "tbb/parallel_for.h"
#include "BaseLib/Point3Dd.h"
#include "BaseLib/MathUtil.h"
#include <boost/math/constants/constants.hpp>
#include <algorithm>
#include <boost/algorithm/clamp.hpp>

VM_BEGIN_NAMESPACE

BMeshDiffCurvature::BMeshDiffCurvature(BMesh *bm, int curvature_off)
	:
	_bmesh(bm),
	_curvature_off(curvature_off)
{}

BMeshDiffCurvature::~BMeshDiffCurvature()
{}

void BMeshDiffCurvature::compute()
{
	//smoothMeshBegin();
	init();
	computeVertexArea();
	computeEdgeWeight();
	computeGaussCurvature();
	computeMeanCurvature();
	//smoothCurvature();
	outputCurvature();
	//smoothMeshEnd();
	//debugCurvatureMesh(_vMeanCurvature, g_test_color_triangles);
}

void BMeshDiffCurvature::init()
{
	checkVertexBoundary();

	{
		/*edges*/
		size_t totedge = _bmesh->BM_mesh_edges_total();

		_eVec.resize(totedge);
		_eLength.resize(totedge);

		const std::vector<BMEdge*> &edges = _bmesh->BM_mesh_edge_table();
		for (size_t i = 0; i < edges.size(); ++i){
			BMEdge *e = edges[i];
			BLI_assert(BM_elem_index_get(e) == i);
			_eVec[i] = e->v1->co - e->v2->co;
			_eLength[i] = MathUtil::normalize(_eVec[i]);
		}
	}

	{
		/*face's angles*/
		size_t totface = _bmesh->BM_mesh_faces_total();
		_fInternalAngle.resize(totface);
		_fArea.resize(totface);

		const std::vector<BMFace*> &faces = _bmesh->BM_mesh_face_table();
		BMLoop *loops[3];
		float angle;

		for (size_t i = 0; i < totface; ++i){
			BMFace *f = faces[i];
			Vector3f &fangle = _fInternalAngle[i];

			BLI_assert(i == BM_elem_index_get(f));
			_fArea[i] = BM_face_calc_area(f);
			BM_face_as_array_loop_tri(f, loops);
			for (size_t d = 0; d < 3; ++d){

				const auto &e0 = _eLength[BM_elem_index_get(loops[d]->e)];
				const auto &e1 = _eLength[BM_elem_index_get(loops[(d + 1) % 3]->e)];
				const auto &e2 = _eLength[BM_elem_index_get(loops[(d + 2) % 3]->e)];

				angle = (e0*e0 + e2*e2 - e1*e1) / (2.0*e0*e2);
				angle = std::min<float>(1.0f, std::max<float>(-1.0f, angle));

				fangle[d] = std::acos(angle);
			}
		}
	}

}


void BMeshDiffCurvature::smoothMeshBegin()
{
	const std::vector<BMVert*> &verts = _bmesh->BM_mesh_vert_table();
	size_t totverts = verts.size();

	_vOrgCoord.resize(totverts);

	for (size_t i = 0; i < totverts; ++i){
		BMVert *v = verts[i];
		_vOrgCoord[i] = v->co;
	}

	std::vector<Vector3f> tmpcoords(totverts);
	for (size_t iter = 0; iter < 4; ++iter){

		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, totverts),
			[&](const tbb::blocked_range<size_t> &range)
		{
			for (size_t i = range.begin(); i < range.end(); ++i){
				BMVert *v = verts[i];
				BM_vert_calc_mean(v, tmpcoords[i]);
			}
		});

		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, totverts),
			[&](const tbb::blocked_range<size_t> &range)
		{
			for (size_t i = range.begin(); i < range.end(); ++i){
				BMVert *v = verts[i];
				v->co = tmpcoords[i];
			}
		});
	}

}


void BMeshDiffCurvature::smoothMeshEnd()
{
	const std::vector<BMVert*> &verts = _bmesh->BM_mesh_vert_table();
	size_t totverts = verts.size();


	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, totverts),
		[&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BMVert *v = verts[i];
			v->co = _vOrgCoord[i];
		}
	});
}



void BMeshDiffCurvature::computeVertexArea()
{
	const float m_half_pi = boost::math::constants::half_pi<float>();
	const std::vector<BMVert*> &verts = _bmesh->BM_mesh_vert_table();
	const size_t totverts = verts.size();

	_vArea.resize(totverts);

	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, totverts),
		[&](const tbb::blocked_range<size_t> &range)
	{
		BMFace *f;
		BMIter iter;
		BMLoop *loops[3];
		float area;
		size_t findex, vpos, eindex0, eindex2;

		for (size_t i = range.begin(); i < range.end(); ++i){

			BMVert *v = verts[i];
			area = 0.0f;

			BM_ITER_ELEM(f, &iter, v, BM_FACES_OF_VERT){
				findex = BM_elem_index_get(f);

#ifdef FIXED_VORONOI_AREA
				const Vector3f &fAngle = _fInternalAngle[findex];
				BM_face_as_array_loop_tri(f, loops);

				for (vpos = 0; vpos < 3; ++vpos){
					if (v == loops[vpos]->v)
						break;
				}

				if (fAngle[0] >= m_half_pi ||
					fAngle[1] >= m_half_pi ||
					fAngle[2] >= m_half_pi){

					if (fAngle[vpos] >= m_half_pi){
						area += 0.5f  * _fArea[findex];
					}
					else{
						area += 0.25f * _fArea[findex];
					}
				}
				else{
					eindex0 = BM_elem_index_get(loops[vpos]->e);
					eindex2 = BM_elem_index_get(loops[(vpos + 2) % 3]->e);
					area +=
						0.125 *
						(_eLength[eindex0] * _eLength[eindex0] / std::tan(fAngle[(vpos + 2) % 3]) +
						_eLength[eindex2] * _eLength[eindex2] / std::tan(fAngle[(vpos + 1) % 3]));
				}
#else
				area += 0.3333333333 * _fArea[findex];
#endif

			}

			_vArea[BM_elem_index_get(v)] = area;
		}
	});

}


void BMeshDiffCurvature::computeEdgeWeight()
{
	const std::vector<BMEdge*> &edges = _bmesh->BM_mesh_edge_table();
	size_t totedge = edges.size();
	_eWeight.resize(totedge, 0.0f);

	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, totedge),
		[&](const tbb::blocked_range<size_t> &range)
	{
		BMLoop *l[2];
		BMVert *verts[3], *v_op; /*opposite vertex*/
		int vpos;

		for (size_t i = range.begin(); i < range.end(); ++i){
			BMEdge *e = edges[i];
			float weight = 0.0;
			if (BM_edge_loop_pair(e, &l[0], &l[1])){

				for (size_t i = 0; i < 2; ++i){
					BMFace *f = l[i]->f;
					v_op = l[i]->next->next->v;

					BM_face_as_array_vert_tri(f, verts);
					for (vpos = 0; vpos < 3; ++vpos){
						if (v_op == verts[vpos])
							break;
					}

					/*retrieve angle*/
					weight += 1.0f / std::tan(_fInternalAngle[BM_elem_index_get(f)][vpos]);
				}
			}

			BLI_assert(BM_elem_index_get(e) == i);
			_eWeight[i] = weight;
		}
	});

}

void BMeshDiffCurvature::computeMeanCurvature()
{
	const std::vector<BMVert*> &vtable = _bmesh->BM_mesh_vert_table();
	const size_t totvert = vtable.size();
	_vMeanCurvature.resize(totvert, 0.0);

	//g_vscene_testSegments.clear();

	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, totvert),
		[&](const tbb::blocked_range<size_t> &range)
	{
		BMEdge *e, *e_iter;
		Vector3f umbrella;
		for (size_t i = range.begin(); i < range.end(); ++i){
			BMVert *v = vtable[i], *o_v;

			BLI_assert(BM_elem_index_get(v) == i);

			if (!_vBoundary[i]){

				umbrella.setZero();

				e = e_iter = v->e;
				do {
					o_v = BM_edge_other_vert(e_iter, v);
					umbrella += (v->co - o_v->co) * _eWeight[BM_elem_index_get(e_iter)];
				} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, v)) != e);

				_vMeanCurvature[i] = umbrella.norm() / (2.0f * _vArea[i]);

				//g_vscene_testSegments.push_back(MathUtil::convert(v->co));
				//g_vscene_testSegments.push_back(MathUtil::convert(v->co + 0.004f *_vMeanCurvature[i] * umbrella.normalized()));
				//g_vscene_testSegments.push_back(MathUtil::convert(v->co + umbrella));
			}
		}
	});


	BMEdge *e, *e_iter;
	Vector3f umbrella;
	float mean = 0.0;
	size_t valence = 0;
	for (size_t i = 0; i < totvert; ++i){
		BMVert *v = vtable[i], *o_v;

		if (_vBoundary[i]){
			e = e_iter = v->e;
			do {
				o_v = BM_edge_other_vert(e, v);
				mean += _vMeanCurvature[BM_elem_index_get(o_v)];
				valence++;
			} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, v)) != e);

			_vMeanCurvature[i] = mean / static_cast<float>(valence);
		}
	}
}

void BMeshDiffCurvature::computeGaussCurvature()
{
	const std::vector<BMVert*> &vtable = _bmesh->BM_mesh_vert_table();
	const size_t totvert = vtable.size();
	_vGaussCurvature.resize(vtable.size());

	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, totvert),
		[&](const tbb::blocked_range<size_t> &range)
	{
		BMIter iter;
		BMFace *f;
		BMVert *verts[3];
		float angle;
		int vpos;
		for (size_t i = range.begin(); i < range.end(); ++i){
			BMVert *v = vtable[i];

			if (!_vBoundary[i]){
				angle = 0;
				BM_ITER_ELEM(f, &iter, v, BM_FACES_OF_VERT){
					BM_face_as_array_vert_tri(f, verts);
					for (vpos = 0; vpos < 3; ++vpos){
						if (v == verts[vpos])
							break;
					}
					angle += _fInternalAngle[BM_elem_index_get(f)][vpos];
				}
				_vGaussCurvature[BM_elem_index_get(v)] = (2 * M_PI - angle) / _vArea[BM_elem_index_get(v)];
			}
		}
	});


	BMVert *o_v;
	BMEdge *e;
	size_t valence;
	BMIter iter;
	float angle;
	for (size_t i = 0; i < vtable.size(); ++i){
		BMVert *v = vtable[i];
		if (_vBoundary[i]){
			angle = 0.0f;
			valence = 0;
			BM_ITER_ELEM(e, &iter, v, BM_EDGES_OF_VERT){
				o_v = BM_edge_other_vert(e, v);
				if (!_vBoundary[BM_elem_index_get(o_v)]){
					angle += _vGaussCurvature[BM_elem_index_get(o_v)];
					valence++;
				}
			}
			if (valence > 0){
				_vGaussCurvature[BM_elem_index_get(v)] = angle / static_cast<float>(valence);
			}
		}
	}
}


void BMeshDiffCurvature::smoothCurvature()
{
	const std::vector<BMVert*> verts = _bmesh->BM_mesh_vert_table();
	const size_t totvert = verts.size();

	std::vector<float> new_gaus(totvert, 0.0f), new_mean(totvert, 0.0f);
	size_t ovindex;
	BMEdge *e_v, *e_iter;
	BMVert *v, *o_v;
	float gc, mc, ww, w;

	for (size_t iter = 0; iter < 5; ++iter){

		for (size_t i = 0; i < totvert; ++i){
			v = verts[i];
			if (!_vBoundary[i]){
				gc = mc = ww = 0.0f;
				e_v = e_iter = v->e;
				do {
					o_v = BM_edge_other_vert(e_iter, v);
					ovindex = BM_elem_index_get(o_v);
					w = _eWeight[BM_elem_index_get(e_iter)];
					gc += w * _vGaussCurvature[ovindex];
					mc += w * _vMeanCurvature[ovindex];
					ww += w;
				} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, v)) != e_v);

				if (ww){
					gc /= ww;
					mc /= ww;
					new_gaus[i] = gc;
					new_mean[i] = mc;
				}
			}
		}

		for (size_t i = 0; i < totvert; ++i){
			_vGaussCurvature[i] = new_gaus[i];
			_vMeanCurvature[i] = new_mean[i];
		}
	}
}


void BMeshDiffCurvature::checkVertexBoundary()
{
	const std::vector<BMVert*> &vtable = _bmesh->BM_mesh_vert_table();
	_vBoundary.resize(vtable.size());
	for (size_t i = 0; i < vtable.size(); ++i){
		BMVert *v = vtable[i];
		_vBoundary[i] = BM_vert_is_boundary(v);
	}
}

void BMeshDiffCurvature::outputCurvature()
{
	const std::vector<BMVert*> &verts = _bmesh->BM_mesh_vert_table();
	const size_t totvert = verts.size();

	float curvature, H, K;
	for (size_t i = 0; i < totvert; ++i){
		H = _vMeanCurvature[i];
		K = _vGaussCurvature[i];
		curvature = H + sqrt(std::max(0.0f, H*H - K));
		BM_ELEM_CD_SET_FLOAT(verts[i], _curvature_off, curvature);
	}
}
VM_END_NAMESPACE

