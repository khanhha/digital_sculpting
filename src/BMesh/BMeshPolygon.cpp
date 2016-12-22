#include "BMeshPolygon.h"
#include "BaseLib/MathUtil.h"
#include "BaseLib/MathGeom.h"
#include "BMeshStructure.h"
#include "BMeshQueries.h"
#include "BMeshInline.h"
#include "BaseLib/MathBase.h"
#include <boost/container/small_vector.hpp>

VM_BEGIN_NAMESPACE

/**
* Small utility functions for fast access
*
* faster alternative to:
*  BM_iter_as_array(bm, BM_VERTS_OF_FACE, f, (void **)v, 3);
*/
void BM_face_as_array_vert_tri(const BMFace *f, BMVert *r_verts[3])
{
	BMLoop *l = BM_FACE_FIRST_LOOP(f);

	BLI_assert(f->len == 3);

	r_verts[0] = l->v; l = l->next;
	r_verts[1] = l->v; l = l->next;
	r_verts[2] = l->v;
}

/**
* faster alternative to:
*  BM_iter_as_array(bm, BM_VERTS_OF_FACE, f, (void **)v, 4);
*/
void BM_face_as_array_vert_quad(const BMFace *f, BMVert *r_verts[4])
{
	BMLoop *l = BM_FACE_FIRST_LOOP(f);

	BLI_assert(f->len == 4);

	r_verts[0] = l->v; l = l->next;
	r_verts[1] = l->v; l = l->next;
	r_verts[2] = l->v; l = l->next;
	r_verts[3] = l->v;
}


/**
* Small utility functions for fast access
*
* faster alternative to:
*  BM_iter_as_array(bm, BM_LOOPS_OF_FACE, f, (void **)l, 3);
*/
void BM_face_as_array_loop_tri(const BMFace *f, BMLoop *r_loops[3])
{
	BMLoop *l = BM_FACE_FIRST_LOOP(f);

	BLI_assert(f->len == 3);

	r_loops[0] = l; l = l->next;
	r_loops[1] = l; l = l->next;
	r_loops[2] = l;
}

/**
* faster alternative to:
*  BM_iter_as_array(bm, BM_LOOPS_OF_FACE, f, (void **)l, 4);
*/
void BM_face_as_array_loop_quad(const BMFace *f, BMLoop *r_loops[4])
{
	BMLoop *l = BM_FACE_FIRST_LOOP(f);

	BLI_assert(f->len == 4);

	r_loops[0] = l; l = l->next;
	r_loops[1] = l; l = l->next;
	r_loops[2] = l; l = l->next;
	r_loops[3] = l;
}

bool BM_face_closest_point(BMFace *face, const Vector3f &p, Vector3f &closest)
{
	if (face->len == 3){
		BMVert *verts[3];
		BM_face_as_array_vert_tri(face, verts);
		MathGeom::closest_on_tri_to_point_v3(closest, p, verts[0]->co, verts[1]->co, verts[2]->co);
		return true;
	}
	else if (face->len == 4){
		Vector3f c1, c2;
		BMVert *verts[4];
		BM_face_as_array_vert_quad(face, verts);
		MathGeom::closest_on_tri_to_point_v3(c1, p, verts[0]->co, verts[1]->co, verts[2]->co);
		MathGeom::closest_on_tri_to_point_v3(c2, p, verts[2]->co, verts[3]->co, verts[0]->co);
		if ((c1 - p).squaredNorm() < (c2 - p).squaredNorm()){
			closest = c1;
		}
		else{
			closest = c2;
		}
		return true;
	}
	else{
		return false;
	}
}
/**
* \brief COMPUTE POLY NORMAL (BMFace)
*
* Same as #normal_poly_v3 but operates directly on a bmesh face.
*/
static float bm_face_calc_poly_normal(const BMFace *f, Vector3f &n)
{
	BMLoop *l_first = BM_FACE_FIRST_LOOP(f);
	BMLoop *l_iter = l_first;
	Vector3f &v_prev = l_first->prev->v->co;
	Vector3f &v_curr = l_first->v->co;

	n.setZero();

	/* Newell's Method */
	do {
		MathUtil::add_newell_cross_v3_v3v3(n, v_prev, v_curr);

		l_iter = l_iter->next;
		v_prev = v_curr;
		v_curr = l_iter->v->co;

	} while (l_iter != l_first);

	return MathUtil::normalize(n);
}

/**
* \brief BMESH UPDATE FACE NORMAL
*
* Updates the stored normal for the
* given face. Requires that a buffer
* of sufficient length to store projected
* coordinates for all of the face's vertices
* is passed in as well.
*/

float BM_face_calc_normal(const BMFace *f, Vector3f &r_no)
{
	BMLoop *l;

	/* common cases first */
	switch (f->len) {
		case 4:
		{
			const Vector3f &co1 = (l = BM_FACE_FIRST_LOOP(f))->v->co;
			const Vector3f &co2 = (l = l->next)->v->co;
			const Vector3f &co3 = (l = l->next)->v->co;
			const Vector3f &co4 = (l->next)->v->co;

			return MathGeom::normal_quad_v3(r_no, co1, co2, co3, co4);
		}
		case 3:
		{
			const Vector3f &co1 = (l = BM_FACE_FIRST_LOOP(f))->v->co;
			const Vector3f &co2 = (l = l->next)->v->co;
			const Vector3f &co3 = (l->next)->v->co;

			return MathGeom::normal_tri_v3(r_no, co1, co2, co3);
		}
		default:
		{
			return bm_face_calc_poly_normal(f, r_no);
		}
	}
}

void BM_face_normal_update(BMFace *f)
{
	BM_face_calc_normal(f, f->no);
}

/**
* computes center of face in 3d.  uses center of bounding box.
*/
void BM_face_calc_bounds(const BMFace *f, Vector3f &lower, Vector3f &upper)
{
	Vector3f lower_ = Vector3f(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3f upper_ = Vector3f(-FLT_MAX, -FLT_MAX, -FLT_MAX);

	BMLoop *l_iter = BM_FACE_FIRST_LOOP(f);
	const BMLoop *l_first = l_iter;
	do {
		const Vector3f co = l_iter->v->co;
		lower_ = lower_.cwiseMin(co);
		upper_ = upper_.cwiseMax(co);
	} while ((l_iter = l_iter->next) != l_first);

	lower = lower_;
	upper = upper_;
}

float BM_face_calc_area(const BMFace *f)
{
	const BMLoop *l_first;
	float area;
	l_first = BM_FACE_FIRST_LOOP(f);

	if (f->len == 3){
		area = MathGeom::area_tri_v3(l_first->v->co, l_first->next->v->co, l_first->next->next->v->co);
	}
	else{
		boost::container::small_vector<Vector3f, 6> verts;
		BMLoop *l_iter = BM_FACE_FIRST_LOOP(f);
		do {
			verts.push_back(l_iter->v->co);
		} while ((l_iter = l_iter->next) != l_first);

		area =MathGeom::area_poly_v3((verts).data(), f->len);
	}

	return area;
}

/**
* computes the center of a face, using the mean average
*/
void  BM_face_calc_center_mean(const BMFace *f, Vector3f &r_center)
{
	const BMLoop *l_iter, *l_first;
	
	r_center.setZero();

	l_iter = l_first = BM_FACE_FIRST_LOOP(f);
	do {
		r_center += l_iter->v->co;
	} while ((l_iter = l_iter->next) != l_first);
	
	r_center *= 1.0f / (float)f->len;
}


static void bm_loop_normal_accum(const BMLoop *l, Vector3f &no)
{
	Vector3f vec1, vec2;
	float fac;

	/* Same calculation used in BM_mesh_normals_update */
	vec1 = (l->v->co - l->prev->v->co).normalized();
	vec2 = (l->next->v->co - l->v->co).normalized();

	fac = MathBase::saacos(-vec1.dot(vec2));

	no += fac *l->f->no, fac;
}

bool BM_vert_calc_normal_ex(const BMVert *v, const char hflag, Vector3f &r_no)
{
	int len = 0;

	r_no.setZero();

	if (v->e) {
		const BMEdge *e = v->e;
		do {
			if (e->l) {
				const BMLoop *l = e->l;
				do {
					if (l->v == v) {
						if (BM_elem_flag_test(l->f, hflag)) {
							bm_loop_normal_accum(l, r_no);
							len++;
						}
					}
				} while ((l = l->radial_next) != e->l);
			}
		} while ((e = bmesh_disk_edge_next(e, v)) != v->e);
	}

	if (len) {
		r_no.normalize();
		return true;
	}
	else {
		return false;
	}
}

void  BM_vert_calc_mean(const BMVert *v, Vector3f &r_mean)
{
	size_t tot = 0;
	r_mean.setZero();
	BMEdge *e_iter = v->e;
	if (e_iter){
		do {
			tot++;
			r_mean += BM_edge_other_vert(e_iter, v)->co;
		} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, v)) != v->e);
		if (tot){
			r_mean /= (float)(tot);
		}
	}
}


bool BM_vert_calc_normal(const BMVert *v, Vector3f &r_no)
{
	int len = 0;

	r_no.setZero();

	if (v->e) {
		const BMEdge *e = v->e;
		do {
			if (e->l) {
				const BMLoop *l = e->l;
				do {
					if (l->v == v) {
						bm_loop_normal_accum(l, r_no);
						len++;
					}
				} while ((l = l->radial_next) != e->l);
			}
		} while ((e = bmesh_disk_edge_next(e, v)) != v->e);
	}

	if (len) {
		r_no.normalize();
		return true;
	}
	else {
		return false;
	}
}

void BM_vert_normal_update(BMVert *v)
{
	BM_vert_calc_normal(v, v->no);
}

void  BM_vert_normal_update_face(BMVert *v)
{
	int len = 0;

	v->no.setZero();

	if (v->e) {
		const BMEdge *e = v->e;
		do {
			if (e->l) {
				const BMLoop *l = e->l;
				do {
					if (l->v == v) {
						v->no += l->f->no;
						len++;
					}
				} while ((l = l->radial_next) != e->l);
			}
		} while ((e = bmesh_disk_edge_next(e, v)) != v->e);
	}

	if (len) {
		v->no.normalize();
	}
}
void BM_vert_normal_update_all(BMVert *v)
{
	int len = 0;

	v->no.setZero();

	if (v->e) {
		const BMEdge *e = v->e;
		do {
			if (e->l) {
				const BMLoop *l = e->l;
				do {
					if (l->v == v) {
						BM_face_normal_update(l->f);
						bm_loop_normal_accum(l, v->no);
						len++;
					}
				} while ((l = l->radial_next) != e->l);
			}
		} while ((e = bmesh_disk_edge_next(e, v)) != v->e);
	}

	if (len) {
		v->no.normalize();
	}
}

void BM_vert_default_color_set(BMVert *v)
{
	v->color[0] = (unsigned char)(0.7f * 256);
	v->color[1] = (unsigned char)(0.8f * 256);
	v->color[2] = (unsigned char)(0.4f * 256);
	v->color[3] = (unsigned char)(256);
}

VM_END_NAMESPACE