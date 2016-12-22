#ifndef BASELIB_MATH_GEOM_H
#define BASELIB_MATH_GEOM_H
#include <Eigen/Dense>
using namespace  Eigen;
namespace MathGeom
{
	/* line-line */
	enum
	{
		ISECT_LINE_LINE_COLINEAR    = -1,
		ISECT_LINE_LINE_NONE        = 0,
		ISECT_LINE_LINE_EXACT       = 1,
		ISECT_LINE_LINE_CROSS       = 2
	};


	float cross_tri_v2(const Vector2f &v1, const Vector2f & v2, const Vector2f & v3);
	float area_tri_signed_v2(const Vector2f & v1, const Vector2f & v2, const Vector2f & v3);
	float area_tri_v2(const Vector2f & v1, const Vector2f & v2, const Vector2f & v3);
	float area_squared_tri_v2(const Vector2f & v1, const Vector2f & v2, const Vector2f & v3);

	float normal_tri_v3(Vector3f & r, const Vector3f & a, const Vector3f & b, const Vector3f & c);
	float normal_quad_v3(Vector3f & r, const Vector3f & a, const Vector3f & b, const Vector3f & c, const Vector3f & d);

	void  cross_poly_v3(Vector3f &n, const Vector3f *verts, unsigned int nr);
	void  cross_tri_v3(Vector3f &n, const Vector3f &v1, const Vector3f &v2, const Vector3f &v3);
	float area_tri_v3(const Vector3f &v1, const Vector3f &v2, const Vector3f& v3);

	float area_poly_v3(const Vector3f* verts, unsigned int nr);

	float closest_to_line_v3(Vector3f &r_close, const Vector3f &p, const Vector3f &l1, const Vector3f &l2);
	void  closest_to_line_segment_v3(Vector3f &r_close, const Vector3f &p, const Vector3f &l1, const Vector3f &l2);
	void  closest_on_tri_to_point_v3(Vector3f &r, const Vector3f &p, const Vector3f &a, const Vector3f &b, const Vector3f &c);
	void  closest_to_plane_v3(Vector3f &r_close, const Vector4f &plane, const Vector3f &pt);


	float dist_signed_squared_to_plane_v3(const Vector3f &p, const Vector4f &plane);
	float        dist_squared_to_plane_v3(const Vector3f &p, const Vector4f &plane);
	float dist_signed_to_plane_v3(const Vector3f &p, const Vector4f &plane);
	float        dist_to_plane_v3(const Vector3f &p, const Vector4f &plane);

	void  plane_to_point_vector_v3(const Vector4f &plane, Vector3f &r_plane_co, Vector3f &r_plane_no);
	void  plane_from_point_normal_v3(Vector4f &r_plane, const Vector3f &plane_co, const Vector3f &plane_no);
	float plane_point_side_v3(const Vector4f &plane, const Vector3f &co);
	void  plane_dir_project_v3(const Vector3f &plane_co, const Vector3f &plane_no, const Vector3f &dir, Vector3f &r_proj);
	void  plane_mirror_point(const Vector4f &plane, const Vector3f &p, Vector3f &r_p);
	void  plane_mirror_vector(const Vector4f &plane, const Vector3f &vec, Vector3f &r_vec);
	bool  isect_tri_plane_test(Vector3f &v1, Vector3f &v2, Vector3f &v3, const Vector4f &plane);



	bool  isect_line_plane_v3(
		Vector3f &r_isect_co, const Vector3f &l1, const Vector3f &l2,
		const Vector3f &plane_co, const Vector3f &plane_no);

	bool isect_ray_tri_v3(
		const Vector3f &ray_origin, const Vector3f &ray_direction,
		const Vector3f &v0, const Vector3f &v1, const Vector3f &v2,
		float *r_lambda, float r_uv[2]);

	bool isect_ray_tri_epsilon_v3(
		const Vector3f &ray_origin, const Vector3f &ray_direction,
		const Vector3f &v0, const Vector3f &v1, const Vector3f &v2,
		float *r_lambda, float r_uv[2], const float epsilon, bool test_cull);

	int  isect_line_sphere_v3(const Vector3f &l1, const Vector3f &l2, const Vector3f &sp, const float r, Vector3f *r_p1, Vector3f *r_p2);

	int  isect_sphere_frustum_v3(const Vector3f &center, const float &radius, const Vector4f(*planes)[4]);
	bool isect_aligned_box_sphere_v3(const Vector3f &lower, const Vector3f &upper, const Vector3f &center, const float &radius);
	bool isect_aligned_box_box_tube_v3(const Vector3f &lower, const Vector3f &upper, const Vector4f (*planes)[4]);
	bool isect_aligned_box_aligned_box_v3(const Vector3f &amin, const Vector3f &amax, const Vector3f &bmin, const Vector3f &bmax);
	bool isect_ray_aligned_box_v3(const Vector3f &p, const Vector3f &d, const Vector3f &bmin, const Vector3f &bmax, Vector3f &q);

	bool isect_seg_seg_v2_simple(const Vector2f &v1, const Vector2f &v2, const Vector2f &v3, const Vector2f &v4);
	int  isect_seg_seg_v2(const Vector2f &a1, const Vector2f &a2, const Vector2f &b1, const Vector2f &b2);

	int	 is_quad_flip_v3(const Vector3f &v1, const Vector3f &v2, const Vector3f &v3, const Vector3f &v4);
	void axis_dominant_v3_to_m3(Matrix3f &r_mat, const Vector3f &normal);
}
#endif