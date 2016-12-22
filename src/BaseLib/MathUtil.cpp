#include "BaseLib/MathUtil.h"
#include "BaseLib/MathBase.h"
#include <cmath>

Vector3f MathUtil::flipPoint3D(const Vector3f &p, const Vector3f &basisOrg, char sym)
{
	Vector3f locaFlip = p - basisOrg;

	locaFlip[0] = sym & 1 ? -locaFlip[0] : locaFlip[0];
	locaFlip[1] = sym & 2 ? -locaFlip[1] : locaFlip[1];
	locaFlip[2] = sym & 4 ? -locaFlip[2] : locaFlip[2];
	
	locaFlip = locaFlip + basisOrg;

	return locaFlip;
}

Vector3f MathUtil::flipVector3D(const Vector3f &dir, const Vector3f &basisOrg, char sym)
{
	Vector3f globalFlipPoint = flipPoint3D(basisOrg + dir, basisOrg, sym);
	return (globalFlipPoint - basisOrg);
}


Point3Dd MathUtil::flipPoint3D(const Point3Dd &p, const Point3Dd &basisOrg, char sym)
{
	Point3Dd locaFlip = p - basisOrg;

	locaFlip[0] = sym & 1 ? -locaFlip[0] : locaFlip[0];
	locaFlip[1] = sym & 2 ? -locaFlip[1] : locaFlip[1];
	locaFlip[2] = sym & 4 ? -locaFlip[2] : locaFlip[2];

	locaFlip = locaFlip + basisOrg;

	return locaFlip;
}

Point3Dd MathUtil::flipVector3D(const Point3Dd &dir, const Point3Dd &basisOrg, char sym)
{
	Point3Dd globalFlipPoint = flipPoint3D(basisOrg + dir, basisOrg, sym);
	return (globalFlipPoint - basisOrg);
}
Point3Dd MathUtil::convert(const Eigen::Vector3f &p)
{
	return Point3Dd(p(0), p(1), p(2));
}

Vector3f MathUtil::convert(const Point3Dd &p)
{
	return Vector3f((float)p.x, (float)p.y, (float)p.z);
}

/*v1, v2 should be normalized*/
float MathUtil::angle_normalized_v3v3(const Vector3f &v1, const Vector3f &v2)
{
	/* this is the same as acos(dot_v3v3(v1, v2)), but more accurate */
	if (v1.dot(v2) >= 0.0f) {
		return 2.0f * MathBase::saasin((v1 - v2).norm() / 2.0f);
	}
	else {
		Vector3f v2_n;
		v2_n = -v2;
		return (float)M_PI - 2.0f * MathBase::saasin((v1 - v2_n).norm() / 2.0f);
	}
}
float MathUtil::angle_normalized_v2v2(const Vector2f &v1, const Vector2f &v2)
{
	/* double check they are normalized */
	//BLI_ASSERT_UNIT_V2(v1);
	//BLI_ASSERT_UNIT_V2(v2);

	/* this is the same as acos(dot_v3v3(v1, v2)), but more accurate */
	if (v1.dot(v2) >= 0.0f) {
		return 2.0f * MathBase::saasin((v1 - v2).norm()/ 2.0f);
	}
	else {
		Vector2f v2_n;
		v2_n = -v2;
		return (float)M_PI - 2.0f * MathBase::saasin((v1 - v2_n).norm() / 2.0f);
	}
}

float MathUtil::normalize(Vector3f &v)
{
	float sqr = v.squaredNorm();
	if (sqr < 1.0e-35f){
		return 0.0f;
	}
	else{
		sqr = sqrtf(sqr);
		v *= 1.0f / sqr;
		return sqr;
	}
}

void MathUtil::add_newell_cross_v3_v3v3(Vector3f& n, const Vector3f&v_prev, const Vector3f&v_curr)
{
	n[0] += (v_prev[1] - v_curr[1]) * (v_prev[2] + v_curr[2]);
	n[1] += (v_prev[2] - v_curr[2]) * (v_prev[0] + v_curr[0]);
	n[2] += (v_prev[0] - v_curr[0]) * (v_prev[1] + v_curr[1]);
}

void MathUtil::minmax_v3v3_v3(Vector3f &min, Vector3f &max, const Vector3f &vec)
{
	min = min.cwiseMin(vec);
	max = max.cwiseMax(vec);
}

/**
* Takes a vector and computes 2 orthogonal directions.
*
* \note if \a n is n unit length, computed values will be too.
*/
void MathUtil::ortho_basis_v3v3_v3(Vector3f &r_n1, Vector3f & r_n2, const Vector3f &n)
{
	const float eps = FLT_EPSILON;
	const float f =  n[0] * n[0] + n[1] * n[1];

	if (f > eps) {
		const float d = 1.0f / sqrtf(f);

		//BLI_assert(std::isfinite(d));

		r_n1[0] = n[1] * d;
		r_n1[1] = -n[0] * d;
		r_n1[2] = 0.0f;
		r_n2[0] = -n[2] * r_n1[1];
		r_n2[1] = n[2] * r_n1[0];
		r_n2[2] = n[0] * r_n1[1] - n[1] * r_n1[0];
	}
	else {
		/* degenerate case */
		r_n1[0] = (n[2] < 0.0f) ? -1.0f : 1.0f;
		r_n1[1] = r_n1[2] = r_n2[0] = r_n2[2] = 0.0f;
		r_n2[1] = 1.0f;
	}
}

void MathUtil::mul_v2_m3v3(Vector2f &r, const Matrix3f &M, const Vector3f &a)
{
	r[0] = M.row(0).dot(a);
	r[1] = M.row(1).dot(a);
}

int MathUtil::signum_i_ex(float a, float eps)
{
	if (a > eps) return  1;
	if (a < -eps) return -1;
	else          return  0;
}

int MathUtil::signum_i(float a)
{
	if (a > 0.0f) return  1;
	if (a < 0.0f) return -1;
	else          return  0;
}


Vector3f MathUtil::unproject(const Matrix4f &proj, const Matrix4f &modelview, const QRect &viewport, const Vector3f &win)
{
	Matrix4f Inverse = (proj * modelview).inverse();

	Vector4f tmp(win[0], win[1], win[2], 1.0f);
	tmp.x() = (tmp.x() - float(viewport.x())) / float(viewport.width());
	tmp.y() = (tmp.y() - float(viewport.y())) / float(viewport.height());
	tmp = tmp * 2.0f - Vector4f(1.0f, 1.0f, 1.0f, 1.0f);

	Vector4f obj = Inverse * tmp;
	obj /= obj.w();

	return Vector3f(obj[0], obj[1], obj[2]);
}

Vector3f MathUtil::unproject(const Matrix4f &inv_pvm, const QRect &viewport, const Vector3f &win)
{
	Vector4f tmp(win[0], win[1], win[2], 1.0f);
	tmp.x() = (tmp.x() - float(viewport.x())) / float(viewport.width());
	tmp.y() = (tmp.y() - float(viewport.y())) / float(viewport.height());
	tmp = tmp * 2.0f - Vector4f(1.0f, 1.0f, 1.0f, 1.0f);

	Vector4f obj = inv_pvm * tmp;
	obj /= obj.w();

	return Vector3f(obj[0], obj[1], obj[2]);
}

Vector3f MathUtil::project(const Matrix4f &proj, const Matrix4f &modelview, const QRect &viewport, const Vector3f &obj)
{
	Vector4f tmp(obj[0], obj[1], obj[2], 1.0f);
	tmp = modelview * tmp;
	tmp = proj * tmp;

	//Perspective division
	tmp /= tmp.w();

	//Window coordinates
	//Map x, y to range 0-1
	tmp = tmp * float(0.5) + Vector4f(0.5f, 0.5f, 0.5f, 0.5f);
	tmp[0] = tmp[0] * float(viewport.width()) + float(viewport.x());
	tmp[1] = tmp[1] * float(viewport.height()) + float(viewport.y());

	return Vector3f(tmp[0], tmp[1], tmp[2]);
}

Vector3f MathUtil::project(const Matrix4f &pvm, const QRect &viewport, const Vector3f &obj)
{
	Vector4f tmp(obj[0], obj[1], obj[2], 1.0f);
	tmp = pvm * tmp;

	tmp /= tmp.w();
	tmp = tmp * float(0.5) + Vector4f(0.5f, 0.5f, 0.5f, 0.5f);
	tmp[0] = tmp[0] * float(viewport.width()) + float(viewport.x());
	tmp[1] = tmp[1] * float(viewport.height()) + float(viewport.y());

	return Vector3f(tmp[0], tmp[1], tmp[2]);
}

float MathUtil::view3D_pixel_size(const Matrix4f &pvm, const QRect &viewport, float pixel_size, float depth)
{
	Matrix4f pvm_inv = pvm.inverse();
	Vector3f p0 = unproject(pvm_inv, viewport, Vector3f(0, 0, depth));
	Vector3f p1 = unproject(pvm_inv, viewport, Vector3f(pixel_size, 0, depth));
	return (p0 - p1).norm();
}

float MathUtil::view3D_depth(const Matrix4f &pvm, const QRect &viewport, const Vector3f &p)
{
	Vector3f proj = project(pvm, viewport, p);
	return proj[2];
}

void MathUtil::bb_to_segments(const Vector3f &lower, const Vector3f &upper, std::vector<Vector3f> &segments)
{
	Vector3f listVertex[8];
	Vector3f bbmin = lower;
	Vector3f bbmax = upper;

	float size[3];
	for (int i = 0; i < 3; i++)
		size[i] = bbmax[i] - bbmin[i];

	listVertex[0] = bbmin;

	listVertex[1][0] = bbmin[0];
	listVertex[1][1] = bbmin[1] + size[1];
	listVertex[1][2] = bbmin[2];

	listVertex[2][0] = bbmin[0] + size[0];
	listVertex[2][1] = bbmin[1] + size[1];
	listVertex[2][2] = bbmin[2];

	listVertex[3][0] = bbmin[0] + size[0];
	listVertex[3][1] = bbmin[1];
	listVertex[3][2] = bbmin[2];

	listVertex[4][0] = bbmin[0];
	listVertex[4][1] = bbmin[1];
	listVertex[4][2] = bbmin[2] + size[2];

	listVertex[5][0] = bbmin[0] + size[0];
	listVertex[5][1] = bbmin[1];
	listVertex[5][2] = bbmin[2] + size[2];

	listVertex[6][0] = bbmin[0] + size[0];;
	listVertex[6][1] = bbmin[1] + size[1];
	listVertex[6][2] = bbmin[2] + size[2];

	listVertex[7][0] = bbmin[0];
	listVertex[7][1] = bbmin[1] + size[1];
	listVertex[7][2] = bbmin[2] + size[2];

	segments.push_back(listVertex[0]);
	segments.push_back(listVertex[1]);
	segments.push_back(listVertex[1]);
	segments.push_back(listVertex[2]);
	segments.push_back(listVertex[2]);
	segments.push_back(listVertex[3]);
	segments.push_back(listVertex[3]);
	segments.push_back(listVertex[0]);

	segments.push_back(listVertex[4]);
	segments.push_back(listVertex[5]);
	segments.push_back(listVertex[5]);
	segments.push_back(listVertex[6]);
	segments.push_back(listVertex[6]);
	segments.push_back(listVertex[7]);
	segments.push_back(listVertex[7]);
	segments.push_back(listVertex[4]);

	segments.push_back(listVertex[0]);
	segments.push_back(listVertex[4]);
	segments.push_back(listVertex[1]);
	segments.push_back(listVertex[7]);
	segments.push_back(listVertex[2]);
	segments.push_back(listVertex[6]);
	segments.push_back(listVertex[3]);
	segments.push_back(listVertex[5]);
}

