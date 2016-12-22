#include "BMesh/BMeshUtility.h"
#include "BMesh/BMeshIterators.h"
#include "BMesh/BMeshPolygon.h"
#include "Baselib/MathGeom.h"
#include "BaseLib/MathUtil.h"
VM_BEGIN_NAMESPACE

/**
* Assuming we have 2 triangles sharing an edge (2 - 4),
* check if the edge running from (1 - 3) gives better results.
*
* \return (negative number means the edge can be rotated, lager == better).
*/
static float beautify_quad_rotate_calc(const Vector2f &v1, const Vector2f &v2, const Vector2f &v3, const Vector2f &v4)
{
	/* not a loop (only to be able to break out) */
	do {
		bool is_zero_a, is_zero_b;

		const float area_2x_234 = MathGeom::cross_tri_v2(v2, v3, v4);
		const float area_2x_241 = MathGeom::cross_tri_v2(v2, v4, v1);

		const float area_2x_123 = MathGeom::cross_tri_v2(v1, v2, v3);
		const float area_2x_134 = MathGeom::cross_tri_v2(v1, v3, v4);

		{
			BLI_assert((ELEM(v1, v2, v3, v4) == false) &&
				(ELEM(v2, v1, v3, v4) == false) &&
				(ELEM(v3, v1, v2, v4) == false) &&
				(ELEM(v4, v1, v2, v3) == false));

			is_zero_a = (fabsf(area_2x_234) <= FLT_EPSILON);
			is_zero_b = (fabsf(area_2x_241) <= FLT_EPSILON);

			if (is_zero_a && is_zero_b) {
				break;
			}
		}

		/* one of the tri's was degenerate, check we're not rotating
		* into a different degenerate shape or flipping the face */
		if ((fabsf(area_2x_123) <= FLT_EPSILON) || (fabsf(area_2x_134) <= FLT_EPSILON)) {
			/* one of the new rotations is degenerate */
			break;
		}

		if ((area_2x_123 >= 0.0f) != (area_2x_134 >= 0.0f)) {
			/* rotation would cause flipping */
			break;
		}

		{
			/* testing rule: the area divided by the perimeter,
			* check if (1-3) beats the existing (2-4) edge rotation */
			float area_a, area_b;
			float prim_a, prim_b;
			float fac_24, fac_13;

			float len_12, len_23, len_34, len_41, len_24, len_13;

			/* edges around the quad */
			len_12 = (v1 - v2).norm();
			len_23 = (v2 - v3).norm();
			len_34 = (v3 - v4).norm();
			len_41 = (v4 - v1).norm();
			/* edges crossing the quad interior */
			len_13 = (v1 - v3).norm();
			len_24 = (v2 - v4).norm();

			/* note, area is in fact (area * 2),
			* but in this case its OK, since we're comparing ratios */

			/* edge (2-4), current state */
			area_a = fabsf(area_2x_234);
			area_b = fabsf(area_2x_241);
			prim_a = len_23 + len_34 + len_24;
			prim_b = len_41 + len_12 + len_24;
			fac_24 = (area_a / prim_a) + (area_b / prim_b);

			/* edge (1-3), new state */
			area_a = fabsf(area_2x_123);
			area_b = fabsf(area_2x_134);
			prim_a = len_12 + len_23 + len_13;
			prim_b = len_34 + len_41 + len_13;
			fac_13 = (area_a / prim_a) + (area_b / prim_b);

			/* negative number if (1-3) is an improved state */
			return fac_24 - fac_13;
		}
	} while (false);

	return FLT_MAX;
}

/* -------------------------------------------------------------------- */
/* Calculate the improvement of rotating the edge */

static float bm_edge_calc_rotate_beauty__area(
	const Vector3f &v1, const Vector3f & v2, const Vector3f & v3, const Vector3f & v4)
{
	/* not a loop (only to be able to break out) */
	do {
		Vector2f v1_xy, v2_xy, v3_xy, v4_xy;

		/* first get the 2d values */
		{
			const float eps = 1e-5f;
			Vector3f no_a, no_b;
			Vector3f no;
			Matrix3f axis_mat;
			float no_scale;
			MathGeom::cross_tri_v3(no_a, v2, v3, v4);
			MathGeom::cross_tri_v3(no_b, v2, v4, v1);

			// printf("%p %p %p %p - %p %p\n", v1, v2, v3, v4, e->l->f, e->l->radial_next->f);
			BLI_assert((ELEM(v1, v2, v3, v4) == false) &&
				(ELEM(v2, v1, v3, v4) == false) &&
				(ELEM(v3, v1, v2, v4) == false) &&
				(ELEM(v4, v1, v2, v3) == false));

			no = no_a + no_b;
			if (UNLIKELY((no_scale = MathUtil::normalize(no)) <= FLT_EPSILON)) {
				break;
			}

			MathGeom::axis_dominant_v3_to_m3(axis_mat, no);
			MathUtil::mul_v2_m3v3(v1_xy, axis_mat, v1);
			MathUtil::mul_v2_m3v3(v2_xy, axis_mat, v2);
			MathUtil::mul_v2_m3v3(v3_xy, axis_mat, v3);
			MathUtil::mul_v2_m3v3(v4_xy, axis_mat, v4);

			/**
			* Check if input faces are already flipped.
			* Logic for 'signum_i' addition is:
			*
			* Accept:
			* - (1, 1) or (-1, -1): same side (common case).
			* - (-1/1, 0): one degenerate, OK since we may rotate into a valid state.
			*
			* Ignore:
			* - (-1, 1): opposite winding, ignore.
			* - ( 0, 0): both degenerate, ignore.
			*
			* \note The cross product is divided by 'no_scale'
			* so the rotation calculation is scale independent.
			*/
			if (!(MathUtil::signum_i_ex(MathGeom::cross_tri_v2(v2_xy, v3_xy, v4_xy) / no_scale, eps) +
				  MathUtil::signum_i_ex(MathGeom::cross_tri_v2(v2_xy, v4_xy, v1_xy) / no_scale, eps)))
			{
				break;
			}
		}

		return beautify_quad_rotate_calc(v1_xy, v2_xy, v3_xy, v4_xy);

	} while (false);

	return FLT_MAX;
}

static float bm_edge_calc_rotate_beauty__angle(
	const Vector3f &v1, const Vector3f & v2, const Vector3f & v3, const Vector3f & v4)
{
	/* not a loop (only to be able to break out) */
	do {
		Vector3f no_a, no_b;
		float angle_24, angle_13;

		/* edge (2-4), current state */
		MathGeom::normal_tri_v3(no_a, v2, v3, v4);
		MathGeom::normal_tri_v3(no_b, v2, v4, v1);
		angle_24 = MathUtil::angle_normalized_v3v3(no_a, no_b);

		/* edge (1-3), new state */
		/* only check new state for degenerate outcome */
		if ((MathGeom::normal_tri_v3(no_a, v1, v2, v3) == 0.0f) ||
			(MathGeom::normal_tri_v3(no_b, v1, v3, v4) == 0.0f))
		{
			break;
		}
		angle_13 = MathUtil::angle_normalized_v3v3(no_a, no_b);

		return angle_13 - angle_24;

	} while (false);

	return FLT_MAX;
}

void BM_edge_collapse_optimize_co(BMEdge *e, Vector3f &co)
{
	BMFace *f;
	BMIter iter;
	Qdr::Quadric q, qv0, qv1;
	Vector4d plane_db;
	Vector3f center;
	BM_ITER_ELEM(f, &iter, e->v1, BM_FACES_OF_VERT){
		BM_face_calc_center_mean(f, center);
		plane_db[0] = f->no[0];
		plane_db[1] = f->no[1];
		plane_db[2] = f->no[2];
		plane_db[3] = -f->no.dot(center);
		q.quadric_from_plane(plane_db);
		qv0 += q;
	}

	q.quadric_clear();
	BM_ITER_ELEM(f, &iter, e->v2, BM_FACES_OF_VERT){
		BM_face_calc_center_mean(f, center);
		plane_db[0] = f->no[0];
		plane_db[1] = f->no[1];
		plane_db[2] = f->no[2];
		plane_db[3] = -f->no.dot(center);
		q.quadric_from_plane(plane_db);
		qv1 += q;
	}

	/* compute an edge contraction target for edge 'e'
	* this is computed by summing it's vertices quadrics and
	* optimizing the result. */
	q = qv0 + qv1;

	if (q.quadric_optimize(co, 0.01f)) {
		return;  /* all is good */
	}
	else {
		co = 0.5f * (e->v1->co + e->v2->co);
	}
}

void BM_vert_quadric_calc(BMVert *v, Qdr::Quadric &vquadric)
{
	BMIter iter;
	BMFace *f;
	Vector4d plane_db;
	Vector3f center;
	vquadric.quadric_clear();
	Qdr::Quadric q;
	BM_ITER_ELEM(f, &iter, v, BM_FACES_OF_VERT){
		BM_face_calc_center_mean(f, center);
		plane_db[0] = f->no[0];
		plane_db[1] = f->no[1];
		plane_db[2] = f->no[2];
		plane_db[3] = -f->no.dot(center);
		q.quadric_from_plane(plane_db);
		vquadric += q;
	}
}

/**
* Assuming we have 2 triangles sharing an edge (2 - 4),
* check if the edge running from (1 - 3) gives better results.
*
* \return (negative number means the edge can be rotated, lager == better).
*/
float BM_verts_calc_rotate_beauty(const BMVert *v1, const BMVert *v2, const BMVert *v3, const BMVert *v4, const short method)
{
	/* not a loop (only to be able to break out) */
	do {
		if (UNLIKELY(v1 == v3)) {
			// printf("This should never happen, but does sometimes!\n");
			break;
		}

		switch (method) {
		case 0:
			return bm_edge_calc_rotate_beauty__area(v1->co, v2->co, v3->co, v4->co);
		default:
			return bm_edge_calc_rotate_beauty__angle(v1->co, v2->co, v3->co, v4->co);
		}
	} while (false);

	return FLT_MAX;
}

VM_END_NAMESPACE