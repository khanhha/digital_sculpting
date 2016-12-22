#ifndef SUTIL_H
#define SUTIL_H
#include "BaseLib/Point3Dd.h"
#include "VBvh/common/simd/simd.h"
#include "sculpt/commonDefine.h"
#include "sculpt/StrokeData.h"

using namespace VBvh;

class SUtil
{
public:
    static void renderBoundingBox(const Point3Dd& bbmin, const  Point3Dd& bbmax, double* color, bool useColor = true);
    static size_t random();

    static double __forceinline linePlaneIntersect(const Point3Dd& org, const Point3Dd& normal, const Point3Dd& lineStart, const Point3Dd& lineEnd)
    {
        Point3Dd u, h;
        float dot;
        u = lineEnd - lineStart;
        h = lineStart - org;
        dot = u.dot(normal);
        return (dot != 0.0f) ? -normal.dot(h) / dot : 0.0f;
    }

    static __forceinline size_t grainSize(size_t size)
    {
        size_t grain = size / sculpt::thread_number;
        if (!grain || size < 100) 
            return size;
        else return 
            grain;
    }

    static __forceinline void projectPointPlane(
        const Point3Dd& __restrict__ planePoint, const Point3Dd& __restrict__ planeNormal,
        const Point3Dd& __restrict__ point, Point3Dd& __restrict__ out)
    {
        Point3Dd disp = planePoint - point;
        disp = disp.dot(planeNormal) * planeNormal;
        out = point + disp;
    }

	static __forceinline void projectPointPlane(
		const Vector3f&  planePoint, const Vector3f&  planeNormal,
		const Vector3f&  point, Vector3f&  out)
	{
		Vector3f disp = planePoint - point;
		disp = disp.dot(planeNormal) * planeNormal;
		out = point + disp;
	}

	static bool planePointSide(const Point3Dd& coord, const Point3Dd& planePoint, const Point3Dd& planeNorm);
	static bool planePointSide(const Vector3f& coord, const Vector3f& planePoint, const Vector3f& planeNorm);

	static bool planePointSideFlip(const Point3Dd& coord, const Point3Dd& planePoint, const Point3Dd& planeNorm, int flip);
	static bool planePointSideFlip(const Vector3f& coord, const Vector3f& planePoint, const Vector3f& planeNorm, int flip);
	static bool isVertexInsideBrush(const Point3Dd& center, const double& sqrRad, const  Point3Dd& coord);
	static bool isVertexInsideBrush(const Vector3f& center, const float& sqrRad, const  Vector3f& coord);

    static void copy_m4_m4(float m1[4][4], float m2[4][4]);
    static void mul_m4_m4m4(float m1[4][4], float m3_[4][4], float m2_[4][4]);
    static void scale_m4_fl(float R[4][4], float scale);
    static void mul_v3_m4v3(float r[3], const float M[4][4], const float v[3]);
    static bool invert_m4_m4(float R[4][4], float A[4][4]);

    static void axis_angle_normalized_to_mat3_ex(float mat[3][3], const float axis[3], const float angle_sin, const float angle_cos);
    static void mul_v3_m3v3(float r[3], float M[3][3], const float a[3]);

	static void closestToLine(Point3Dd& out, const Point3Dd& in, const Point3Dd& start, const Point3Dd& end);
private:
};
#endif