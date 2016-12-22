#ifndef LINE3D_H
#define LINE3D_H
#include "point3Dd.h"
#include <vector>

class Segment3Dd;

class Line3Dd {
public:
	Line3Dd(const Point3Dd& p, const Point3Dd& v);

public:
	bool intersectPlanarLine(Point3Dd& ip, const Point3Dd& p, const Vector3Dd& v);
	int Line3Dd::intersectPlane( Point3Dd& ip, const Point3Dd& onPoint, const Vector3Dd& normal) const;
private:
	Point3Dd _p;
	Point3Dd _v;
};
#endif