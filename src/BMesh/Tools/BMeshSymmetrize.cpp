#include "BMesh/Tools/BMeshSymmetrize.h"
#include "BMesh/Tools/BMeshBisectPlane.h"
#include "BMesh/Tools/BMeshTriangulate.h"
#include "BaseLib/MathGeom.h"
#include <unordered_map>

VM_BEGIN_NAMESPACE

BMeshSymmetrize::BMeshSymmetrize(BMesh *bm, Vector3f plane_co, Vector3f plane_no, bool side)
	:
	_plane_co(plane_co), _plane_no(plane_no), _side(side), _bm(bm)
{}

BMeshSymmetrize::~BMeshSymmetrize()
{
}

void BMeshSymmetrize::execute()
{
	MathGeom::plane_from_point_normal_v3(_plane, _plane_co, _plane_no);
	bool clear_outer = _side;
	bool clear_inner = !clear_outer;

	BMehsBisectPlane bisect(_bm, _plane, true, false, ELEM_CENTER, FLT_EPSILON, clear_outer, clear_inner, 0.0001f);
	bisect.run();

	mirror_clone();

	BMeshTriangulate triangulator(_bm, VM::BMesh::MOD_TRIANGULATE_QUAD_SHORTEDGE, 0, false);
	triangulator.run();

	clear_flag(_bm);
}

void BMeshSymmetrize::mirror_clone()
{
	BMFace *f;
	BMVert *v, *symn_v;
	BMIter iter;
	std::unordered_map<BMVert*, BMVert*> v_map;
	
	BM_ITER_MESH(v, &iter, _bm, BM_VERTS_OF_MESH){
		if (BM_elem_app_flag_test(v, ELEM_CENTER)){
			symn_v = v;
		} else{
			symn_v = _bm->BM_vert_create(v->co, v, BM_CREATE_NOP);
		}
		v_map[v] = symn_v;
		MathGeom::plane_mirror_point(_plane, v->co, symn_v->co);
	}

	boost::container::small_vector<BMVert*, 4> verts;
	BMLoop *l_iter, *l_first;

	BM_ITER_MESH(f, &iter, _bm, BM_FACES_OF_MESH){

		/*collect verts in opposite direction*/
		verts.clear();
		l_iter = l_first = BM_FACE_FIRST_LOOP(f);
		do {
			auto it = v_map.find(l_iter->v);
			if (it != v_map.end()){
				verts.push_back(it->second);
			}
			else{
				break;
			}
		} while ((l_iter = l_iter->prev) != l_first);

		BMFace *nf = _bm->BM_face_create_verts(verts.data(), f->len, f, BM_CREATE_NOP, true);
		BM_face_normal_update(nf);
	}
}

void BMeshSymmetrize::clear_flag(BMesh *mesh)
{
	BMIter iter;
	BMVert *v;
	BMEdge *e;
	BMFace *f;
	BM_ITER_MESH(v, &iter, mesh, BM_VERTS_OF_MESH){
		BM_elem_app_flag_clear(v);
		BM_elem_flag_disable(v, BM_ELEM_TAG);
	}
	BM_ITER_MESH(f, &iter, mesh, BM_FACES_OF_MESH){
		BM_elem_app_flag_clear(f);
		BM_elem_flag_disable(f, BM_ELEM_TAG);
	}
	BM_ITER_MESH(e, &iter, mesh, BM_EDGES_OF_MESH){
		BM_elem_flag_disable(e, BM_ELEM_TAG);
		BM_elem_app_flag_clear(e);
	}
}


VM_END_NAMESPACE