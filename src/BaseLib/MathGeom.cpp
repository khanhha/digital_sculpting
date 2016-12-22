#include "BaseLib/MathGeom.h"
#include "BaseLib/MathUtil.h"
#include <math.h>

namespace MathGeom
{
	float cross_tri_v2(const Vector2f &v1, const Vector2f & v2, const Vector2f & v3)
	{
		return (v1[0] - v2[0]) * (v2[1] - v3[1]) + (v1[1] - v2[1]) * (v3[0] - v2[0]);
	}
	float area_tri_signed_v2(const Vector2f & v1, const Vector2f & v2, const Vector2f & v3)
	{
		return 0.5f * ((v1[0] - v2[0]) * (v2[1] - v3[1]) + (v1[1] - v2[1]) * (v3[0] - v2[0]));
	}
	float area_tri_v2(const Vector2f & v1, const Vector2f & v2, const Vector2f & v3)
	{
		return fabsf(area_tri_signed_v2(v1, v2, v3));
	}
	float area_squared_tri_v2(const Vector2f & v1, const Vector2f & v2, const Vector2f & v3)
	{
		float area = area_tri_signed_v2(v1, v2, v3);
		return area * area;
	}

	float normal_tri_v3(Vector3f &n, const Vector3f &v1, const Vector3f &v2, const Vector3f &v3)
	{
		Vector3f n1 = v1 - v2;
		Vector3f n2 = v2 - v3;
		
		n = n1.cross(n2);
		
		return MathUtil::normalize(n);
	}

	float normal_quad_v3(Vector3f &n, const Vector3f &v1, const Vector3f &v2, const Vector3f &v3, const Vector3f &v4)
	{
		/* real cross! */
		Vector3f n1, n2;

		n1 = v1 - v3;
		n2 = v2 - v4;

		n = n1.cross(n2);

		return MathUtil::normalize(n);
	}

	float area_tri_v3(const Vector3f &v1, const Vector3f &v2, const Vector3f& v3)
	{
		Vector3f n;
		cross_tri_v3(n, v1, v2, v3);
		return n.norm() * 0.5f;
	}

	void cross_tri_v3(Vector3f &n, const Vector3f &v1, const Vector3f &v2, const Vector3f &v3)
	{
		n = (v1-v2).cross(v2-v3);
	}

	float area_poly_v3(const Vector3f* verts, unsigned int nr)
	{
		Vector3f n;
		cross_poly_v3(n, verts, nr);
		return n.norm()* 0.5f;
	}

	void cross_poly_v3(Vector3f &n, const Vector3f *verts, unsigned int nr)
	{
		const Vector3f *v_prev = &verts[nr - 1];
		const Vector3f *v_curr = &verts[0];
		unsigned int i;

		n.setZero();

		/* Newell's Method */
		for (i = 0; i < nr; v_prev = v_curr, v_curr = &verts[++i]) {
			MathUtil::add_newell_cross_v3_v3v3(n, *v_prev, *v_curr);
		}
	}

	/* find closest point to p on line through (l1, l2) and return lambda,
	* where (0 <= lambda <= 1) when cp is in the line segment (l1, l2)
	*/
	float closest_to_line_v3(Vector3f &r_close, const Vector3f &p, const Vector3f &l1, const Vector3f &l2)
	{
		Vector3f u = l2 - l1;
		Vector3f h = p - l1;
		float lambda = u.dot(h) / u.dot(u);
		r_close = l1 + u * lambda;
		return lambda;
	}

	/* point closest to v1 on line v2-v3 in 3D */
	void closest_to_line_segment_v3(Vector3f &r_close, const Vector3f &p, const Vector3f &l1, const Vector3f &l2)
	{
		float lambda; 
		Vector3f cp;

		lambda = closest_to_line_v3(cp, p, l1, l2);

		if (lambda <= 0.0f)
			r_close = l1;
		else if (lambda >= 1.0f)
			r_close = l2;
		else
			r_close = cp;
	}

	/* Adapted from "Real-Time Collision Detection" by Christer Ericson,
	* published by Morgan Kaufmann Publishers, copyright 2005 Elsevier Inc.
	*
	* Set 'r' to the point in triangle (a, b, c) closest to point 'p' */
	void closest_on_tri_to_point_v3(Vector3f &r, const Vector3f &p, const Vector3f &a, const Vector3f &b, const Vector3f &c)
	{
		/* Check if P in vertex region outside A */
		Vector3f ab = b - a;
		Vector3f ac = c - a;
		Vector3f ap = p - a;
		float d1 = ab.dot(ap);
		float d2 = ac.dot(ap);
		if (d1 <= 0.0f && d2 <= 0.0f) {
			/* barycentric coordinates (1,0,0) */
			r = a;
			return;
		}

		/* Check if P in vertex region outside B */
		Vector3f bp = p - b;
		float d3 = ab.dot(bp);
		float d4 = ac.dot(bp);
		if (d3 >= 0.0f && d4 <= d3) {
			/* barycentric coordinates (0,1,0) */
			r = b;
			return;
		}
		/* Check if P in edge region of AB, if so return projection of P onto AB */
		float vc = d1 * d4 - d3 * d2;
		float v;
		if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) {
			v = d1 / (d1 - d3);
			/* barycentric coordinates (1-v,v,0) */
			r = a + ab *v;
			return;
		}
		/* Check if P in vertex region outside C */
		Vector3f cp = p - c;
		float d5 = ab.dot(cp);
		float d6 = ac.dot(cp);
		if (d6 >= 0.0f && d5 <= d6) {
			/* barycentric coordinates (0,0,1) */
			r = c;
			return;
		}
		/* Check if P in edge region of AC, if so return projection of P onto AC */
		float vb = d5 * d2 - d1 * d6;
		float w;
		if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) {
			w = d2 / (d2 - d6);
			/* barycentric coordinates (1-w,0,w) */
			r = a + ac * w;
			return;
		}

		/* Check if P in edge region of BC, if so return projection of P onto BC */
		float va = d3 * d6 - d5 * d4;
		if (va <= 0.0f && (d4 - d3) >= 0.0f && (d5 - d6) >= 0.0f) {
			w = (d4 - d3) / ((d4 - d3) + (d5 - d6));
			/* barycentric coordinates (0,1-w,w) */
			r = (c - b) * w + b;
			return;
		}

		/* P inside face region. Compute Q through its barycentric coordinates (u,v,w) */
		float denom = 1.0f / (va + vb + vc);
		v = vb * denom;
		w = vc * denom;

		/* = u*a + v*b + w*c, u = va * denom = 1.0f - v - w */
		/* ac * w */
		/* a + ab * v */
		/* a + ab * v + ac * w */
		r = a + ab *v + ac *w;
	}

	/**
	* Find the closest point on a plane.
	*
	* \param r_close  Return coordinate
	* \param plane  The plane to test against.
	* \param pt  The point to find the nearest of
	*
	* \note non-unit-length planes are supported.
	*/
	void  closest_to_plane_v3(Vector3f &r_close, const Vector4f &plane, const Vector3f &pt)
	{
		Vector3f plane_norm(plane.x(), plane.y(), plane.z());
		const float len_sq = plane_norm.squaredNorm();
		const float side = plane_point_side_v3(plane, pt);
		r_close = pt + plane_norm * -side / len_sq;
	}

	bool isect_ray_tri_v3(const Vector3f &ray_origin, const Vector3f &ray_direction, const Vector3f &v0, const Vector3f &v1, const Vector3f &v2, float *r_lambda, float r_uv[2])
	{
		/* note: these values were 0.000001 in 2.4x but for projection snapping on
		* a human head (1BU == 1m), subsurf level 2, this gave many errors - campbell */
		const float epsilon = 0.00000001f;
		Vector3f p, s, e1, e2, q;
		float a, f, u, v;

		e1 = v1 - v0;
		e2 = v2 - v0;

		p = ray_direction.cross(e2);
		a = e1.dot(p);
		if ((a > -epsilon) && (a < epsilon)) return false;
		f = 1.0f / a;

		s = ray_origin - v0;

		u = f * s.dot(p);
		if ((u < 0.0f) || (u > 1.0f)) return false;

		q = s.cross(e1);

		v = f * ray_direction.dot(q);
		if ((v < 0.0f) || ((u + v) > 1.0f)) return false;

		*r_lambda = f * e2.dot(q);
		if ((*r_lambda < 0.0f)) return false;

		if (r_uv) {
			r_uv[0] = u;
			r_uv[1] = v;
		}

		return true;
	}

	/*
	* test if the ray starting at p1 going in d direction intersects the triangle v0..v2
	* test_cull: ignore back facing triangle
	* return non zero if it does
	*/
	bool isect_ray_tri_epsilon_v3(const Vector3f &ray_origin, const Vector3f &ray_direction, const Vector3f &v0, const Vector3f &v1, const Vector3f &v2, float *r_lambda, float r_uv[2], const float epsilon, bool test_cull)
	{
		Vector3f e1 = v1 - v0;
		Vector3f e2 = v2 - v0;

		Vector3f p = ray_direction.cross(e2);
		float a = e1.dot(p);
		
		if (test_cull && a < epsilon){
			return false;
		}
		else if (!test_cull && (a > -epsilon) && (a < epsilon)){
			return false;
		}

		float f = 1.0f / a;

		Vector3f s = ray_origin -v0;

		float u = f * s.dot(p);
		if ((u < -epsilon) || (u > 1.0f + epsilon)) return false;

		Vector3f q = s.cross(e1);

		float v = f * ray_direction.dot(q);
		if ((v < -epsilon) || ((u + v) > 1.0f + epsilon)) return false;

		float lambda = f * e2.dot(q);
		if ((lambda < -epsilon)) return false;

		if (r_uv) {
			r_uv[0] = u;
			r_uv[1] = v;
		}

		if (r_lambda){
			*r_lambda = lambda;
		}

		return true;
	}

	/**
	* \param l1, l2: Coordinates (point of line).
	* \param sp, r:  Coordinate and radius (sphere).
	* \return r_p1, r_p2: Intersection coordinates.
	*/
	int isect_line_sphere_v3(const Vector3f &l1, const Vector3f &l2, const Vector3f &sp, const float r, Vector3f *r_p1, Vector3f *r_p2)
	{
		/* adapted for use in blender by Campbell Barton - 2011
		*
		* atelier iebele abel - 2001
		* atelier@iebele.nl
		* http://www.iebele.nl
		*
		* sphere_line_intersection function adapted from:
		* http://astronomy.swin.edu.au/pbourke/geometry/sphereline
		* Paul Bourke pbourke@swin.edu.au
		*/

		const Vector3f ldir = l2 - l1;

		const float a = ldir.squaredNorm();

		const float b = 2.0f * ldir.dot(l1 - sp);

		const float c =
			sp.squaredNorm() +
			l1.squaredNorm() -
			(2.0f * sp.dot(l1)) -
			(r * r);

		const float i = b * b - 4.0f * a * c;

		float mu;

		if (i < 0.0f) {
			/* no intersections */
			return 0;
		}
		else if (i == 0.0f) {
			/* one intersection */
			mu = -b / (2.0f * a);
			if (r_p1) *r_p1 = l1 + mu * ldir;//madd_v3_v3v3fl(r_p1, l1, ldir, mu);
			return 1;
		}
		else if (i > 0.0f) {
			const float i_sqrt = sqrtf(i);  /* avoid calc twice */

			/* first intersection */
			mu = (-b + i_sqrt) / (2.0f * a);
			if (r_p1) *r_p1 = l1 + mu * ldir;//madd_v3_v3v3fl(r_p1, l1, ldir, mu);

			/* second intersection */
			mu = (-b - i_sqrt) / (2.0f * a);
			if (r_p2) *r_p2 = l1 + mu * ldir;//madd_v3_v3v3fl(r_p2, l1, ldir, mu);
			return 2;
		}
		else {
			/* math domain error - nan */
			return -1;
		}
	}

	/*return 0 : outside, 1 : intersection, 2 : inside*/
	int isect_sphere_frustum_v3(const Vector3f &center, const float &radius, const Vector4f(*planes)[4])
	{
		// various distances
		float fDistance;

		// calculate our distances to each of the planes
		for (int i = 0; i < 4; ++i) {

			// find the distance to this plane
			
			const Vector4f &plane = (*planes)[i];
			fDistance = plane[0] * center[0] + plane[1] * center[1] + plane[2] * center[2] + plane[3];

			// if this distance is < -sphere.radius, we are outside
			if (fDistance > radius)
				return 0;

			// else if the distance is between +- radius, then we intersect
			if (fabs(fDistance) <= radius)
				return 1;
		}

		// otherwise we are fully in view
		return 2;
	}

	bool isect_aligned_box_sphere_v3(const Vector3f &lower, const Vector3f &upper, const Vector3f &center, const float &radius)
	{
		Vector3f nearest;

		if (lower[0] > center[0])
			nearest[0] = lower[0];
		else if (upper[0] < center[0])
			nearest[0] = upper[0];
		else
			nearest[0] = center[0];

		if (lower[1] > center[1])
			nearest[1] = lower[1];
		else if (upper[1] < center[1])
			nearest[1] = upper[1];
		else
			nearest[1] = center[1];

		if (lower[2] > center[2])
			nearest[2] = lower[2];
		else if (upper[2] < center[2])
			nearest[2] = upper[2];
		else
			nearest[2] = center[2];

		return ((nearest - center).squaredNorm() <= (radius * radius));
	}

	bool isect_aligned_box_box_tube_v3(const Vector3f &lower, const Vector3f &upper, const Vector4f (*planes)[4])
	{
		float vmin[3], vmax[3];
		int i, axis;
		bool ret = true;
		for (i = 0; i < 4; ++i) {
			for (axis = 0; axis < 3; ++axis) {
				if ((*planes)[i][axis] > 0) {
					vmin[axis] = lower[axis];
					vmax[axis] = upper[axis];
				}
				else {
					vmin[axis] = upper[axis];
					vmax[axis] = lower[axis];
				}
			}

			if (((*planes)[i][0] * vmin[0] + (*planes)[i][1] * vmin[1] + (*planes)[i][2] * vmin[2]) + (*planes)[i][3] > 0) {
				/*outside*/
				ret = false;
				break;
			}
			else if (((*planes)[i][0] * vmax[0] + (*planes)[i][1] * vmax[1] + (*planes)[i][2] * vmax[2]) + (*planes)[i][3] >= 0) {
				/*intersect*/
				ret = true;
			}
		}
		/*ret == true: partial or inside*/
		return ret;
	}


	bool isect_aligned_box_aligned_box_v3(const Vector3f &amin, const Vector3f &amax, const Vector3f &bmin, const Vector3f &bmax)
	{
		// Exit with no intersection if separated along an axis
		if (amax[0] < bmin[0] || amin[0] > bmax[0]) return false;
		if (amax[1] < bmin[1] || amin[1] > bmax[1]) return false;
		if (amax[2] < bmin[2] || amin[2] > bmax[2]) return false;
		// Overlapping on all axes means AABBs are intersecting
		return true;
	}

	/*Intersect ray R(t) = p + t*d against AABB a. When intersecting,
	  return intersection distance tmin and point q of intersection
	*/
	bool  isect_ray_aligned_box_v3(const Vector3f &p, const Vector3f &d, const Vector3f &bmin, const Vector3f &bmax, Vector3f &q)
	{

		float tmin = -FLT_MAX; // set to -FLT_MAX to get first hit on line
		float tmax = FLT_MAX; // set to max distance ray can travel (for segment)

		// For all three slabs
		for (int i = 0; i < 3; i++) {
			if (std::abs(d[i]) < FLT_EPSILON) {
				// Ray is parallel to slab. No hit if origin not within slab
				if (p[i] < bmin[i] || p[i] > bmax[i]) return false;
			}
			else {
				// Compute intersection t value of ray with near and far plane of slab
				float ood = 1.0f / d[i];
				float t1 = (bmin[i] - p[i]) * ood;
				float t2 = (bmax[i] - p[i]) * ood;
				// Make t1 be intersection with near plane, t2 with far plane
				if (t1 > t2) std::swap(t1, t2);
				// Compute the intersection of slab intersection intervals
				if (t1 > tmin) tmin = t1;
				if (t2 < tmax) tmax = t2;
				// Exit with no collision as soon as slab intersection becomes empty
				if (tmin > tmax) return false;
			}
		}
		// Ray intersects all 3 slabs. Return point (q) and intersection t value (tmin)
		q = p + d * tmin;
		return true;
	}
	/**
	* Intersect line/plane.
	*
	* \param r_isect_co The intersection point.
	* \param l1 The first point of the line.
	* \param l2 The second point of the line.
	* \param plane_co A point on the plane to intersect with.
	* \param plane_no The direction of the plane (does not need to be normalized).
	*
	* \note #line_plane_factor_v3() shares logic.
	*/
	bool isect_line_plane_v3(Vector3f &r_isect_co, const Vector3f &l1, const Vector3f &l2, const Vector3f &plane_co, const Vector3f &plane_no)
	{
		Vector3f u = l2 - l1;
		Vector3f h = l1 - plane_co;
		float dot  = plane_no.dot(u);

		if (std::abs(dot) > FLT_EPSILON) {
			float lambda = -plane_no.dot(h) / dot;
			r_isect_co = l1 + lambda * u;
			return true;
		}
		else {
			/* The segment is parallel to plane */
			return false;
		}
	}

	void  plane_to_point_vector_v3(const Vector4f &plane, Vector3f &r_plane_co, Vector3f &r_plane_no)
	{
		r_plane_no = Vector3f(plane[0], plane[1], plane[2]);
		r_plane_co = r_plane_no * (-plane[3] / r_plane_no.squaredNorm());
	}

	/**
	 * Calculate a plane from a point and a direction,
	 * \note \a point_no isn't required to be normalized.
	 */
	void  plane_from_point_normal_v3(Vector4f &r_plane, const Vector3f &plane_co, const Vector3f &plane_no)
	{
		r_plane[0] = plane_no[0];
		r_plane[1] = plane_no[1];
		r_plane[2] = plane_no[2];
		r_plane[3] = -plane_no.dot(plane_co);
	}

	float plane_point_side_v3(const Vector4f &plane, const Vector3f &co)
	{
		Vector3f plane_norm(plane[0], plane[1], plane[2]);
		return plane_norm.dot(co) + plane[3];
	}

	void  plane_dir_project_v3(const Vector3f &plane_co, const Vector3f &plane_no, const Vector3f &dir, Vector3f &r_proj)
	{
		r_proj = dir - dir.dot(plane_no) * plane_no;
	}

	void  plane_mirror_point(const Vector4f &plane, const Vector3f &p, Vector3f &r_p)
	{
		Vector3f plane_no(plane[0], plane[1], plane[2]);
		float dst_side = MathGeom::plane_point_side_v3(plane, p);
		r_p	= p - 2.0f * dst_side * plane_no;
	}
	
	void  plane_mirror_vector(const Vector4f &plane, const Vector3f &vec, Vector3f &r_vec)
	{
		Vector3f plane_co, plane_no;
		plane_to_point_vector_v3(plane, plane_co, plane_no);
		
		Vector3f mirror_p = plane_co + vec;
		plane_mirror_point(plane, mirror_p, mirror_p);

		r_vec = mirror_p - plane_co;
	}
	
	float	dist_signed_squared_to_plane_v3(const Vector3f &pt, const Vector4f &plane)
	{
		const float len_sq = plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2];
		const float side = plane_point_side_v3(plane, pt);
		const float fac = side / len_sq;
		if (side > 0.0f)
			return len_sq * (fac * fac);
		else
			return -(len_sq * (fac * fac));
	}
	float	dist_squared_to_plane_v3(const Vector3f &pt, const Vector4f &plane)
	{
		const float len_sq = plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2];
		const float side = plane_point_side_v3(plane, pt);
		const float fac = side / len_sq;
		/* only difference to code above - no 'copysignf' */
		return len_sq * (fac * fac);
	}

	float	dist_signed_to_plane_v3(const Vector3f &pt, const Vector4f &plane)
	{
		const float len_sq = plane[0] * plane[0] + plane[1] * plane[1] + plane[2] * plane[2];
		const float side = plane_point_side_v3(plane, pt);
		const float fac = side / len_sq;
		return sqrtf(len_sq) * fac;
	}

	float	dist_to_plane_v3(const Vector3f &pt, const Vector4f &plane)
	{
		return fabsf(dist_signed_to_plane_v3(pt, plane));
	}


	bool isect_tri_plane_test(Vector3f &v1, Vector3f &v2, Vector3f &v3, const Vector4f &plane)
	{
		float d0 = plane_point_side_v3(plane, v1);
		float d1 = plane_point_side_v3(plane, v2);
		float d2 = plane_point_side_v3(plane, v3);

		return ((d0*d1) < 0.0f || (d0*d2) < 0.0f || (d1*d2) < 0.0f);
	}

	bool isect_seg_seg_v2_simple(const Vector2f &v1, const Vector2f &v2, const Vector2f &v3, const Vector2f &v4)
	{
#define CCW(A, B, C) \
		((C[1] - A[1]) * (B[0] - A[0]) > (B[1] - A[1]) * (C[0] - A[0]))

		return CCW(v1, v3, v4) != CCW(v2, v3, v4) && CCW(v1, v2, v3) != CCW(v1, v2, v4);

#undef CCW
	}

	int isect_seg_seg_v2(const Vector2f &v1, const Vector2f &v2, const Vector2f &v3, const Vector2f &v4)
	{
		float div = (v2[0] - v1[0]) * (v4[1] - v3[1]) - (v2[1] - v1[1]) * (v4[0] - v3[0]);
		if (-FLT_EPSILON < div && div < FLT_EPSILON) return ISECT_LINE_LINE_COLINEAR;

		float lambda = ((float)(v1[1] - v3[1]) * (v4[0] - v3[0]) - (v1[0] - v3[0]) * (v4[1] - v3[1])) / div;

		float mu = ((float)(v1[1] - v3[1]) * (v2[0] - v1[0]) - (v1[0] - v3[0]) * (v2[1] - v1[1])) / div;

		if (lambda >= 0.0f && lambda <= 1.0f && mu >= 0.0f && mu <= 1.0f) {
			if (lambda == 0.0f || lambda == 1.0f || mu == 0.0f || mu == 1.0f) return ISECT_LINE_LINE_EXACT;
			return ISECT_LINE_LINE_CROSS;
		}
		return ISECT_LINE_LINE_NONE;
	}

	/**
	* Check if either of the diagonals along this quad create flipped triangles
	* (normals pointing away from eachother).
	* - (1 << 0): (v1-v3) is flipped.
	* - (1 << 1): (v2-v4) is flipped.
	*/
	int is_quad_flip_v3(const Vector3f &v1, const Vector3f &v2, const Vector3f &v3, const Vector3f &v4)
	{
		Vector3f d_12, d_23, d_34, d_41;
		Vector3f cross_a, cross_b;
		int ret = 0;

		d_12 = v1 - v2;
		d_23 = v2 - v3;
		d_34 = v3 - v4;
		d_41 = v4 - v1;

		cross_a = d_12.cross(d_23);
		cross_b = d_34.cross(d_41);
		ret |= ((cross_a.dot(cross_b) < 0.0f) << 0);

		cross_a = d_23.cross(d_34);
		cross_b = d_41.cross(d_12);
		ret |= ((cross_a.dot(cross_b) < 0.0f) << 1);

		return ret;
	}

	/**
	* \brief Normal to x,y matrix
	*
	* Creates a 3x3 matrix from a normal.
	* This matrix can be applied to vectors so their 'z' axis runs along \a normal.
	* In practice it means you can use x,y as 2d coords. \see
	*
	* \param r_mat The matrix to return.
	* \param normal A unit length vector.
	*/
	void axis_dominant_v3_to_m3(Matrix3f &r_mat, const Vector3f &normal)
	{
		r_mat.row(2) = normal;
		Vector3f basis0, basis1;
		MathUtil::ortho_basis_v3v3_v3(basis0, basis1, normal);
		r_mat.row(0) = basis0;
		r_mat.row(1) = basis1;

		//BLI_assert(!is_negative_m3(r_mat));
		//BLI_assert((fabsf(dot_m3_v3_row_z(r_mat, normal) - 1.0f) < BLI_ASSERT_UNIT_EPSILON) || is_zero_v3(normal));
	}
}