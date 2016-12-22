#ifndef BASELIB_MATH_UTIL_H
#define BASELIB_MATH_UTIL_H
#include "BaseLib/Point3Dd.h"
#include "BaseLib/UtilMacro.h"
#include <Eigen/Dense>
#include <QRect>

using namespace Eigen;
namespace MathUtil
{
	Vector3f flipPoint3D(const Vector3f &p, const Vector3f &basisOrg, char sym);
	Vector3f flipVector3D(const Vector3f &p, const Vector3f &basisOrg, char sym);
	Point3Dd flipPoint3D(const Point3Dd &p, const Point3Dd &basisOrg, char sym);
	Point3Dd flipVector3D(const Point3Dd &p, const Point3Dd &basisOrg, char sym);
	Point3Dd convert(const Vector3f &p);
	Vector3f convert(const Point3Dd &p);

	float    normalize(Vector3f &v);
	
	void	add_newell_cross_v3_v3v3(Vector3f& n, const Vector3f&v_prev, const Vector3f&v_curr);
	float	angle_normalized_v3v3(const Vector3f &v1, const Vector3f &v2);
	float	angle_normalized_v2v2(const Vector2f &v1, const Vector2f &v2);
	void	minmax_v3v3_v3(Vector3f &min, Vector3f &max, const Vector3f &vec);
	void	ortho_basis_v3v3_v3(Vector3f &r_n1, Vector3f & r_n2, const Vector3f &n);

	void	mul_v2_m3v3(Vector2f &r, const Matrix3f &M, const Vector3f &a);
	int		signum_i_ex(float a, float eps);
	int		signum_i(float a);

	Vector3f unproject(const Matrix4f &proj, const Matrix4f &modelview,  const QRect &viewport, const Vector3f &win);
	Vector3f unproject(const Matrix4f &inv_pvm, const QRect &viewport, const Vector3f &win);
	Vector3f project(const Matrix4f &proj, const Matrix4f &modelview, const QRect &viewport, const Vector3f &p);
	Vector3f project(const Matrix4f &pvm, const QRect &viewport, const Vector3f &p);
	float	 view3D_depth(const Matrix4f &pvm, const QRect &viewport, const Vector3f &p);
	float	 view3D_pixel_size(const Matrix4f &pvm, const QRect &viewport, float pixel_size, float depth);

	/*debug functions*/
	void bb_to_segments(const Vector3f &lower, const Vector3f &upper, std::vector<Vector3f> &segments);
};

#endif