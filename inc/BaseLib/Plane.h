#ifndef PLANE_H
#define PLANE_H
#include "point3Dd.h"
#include <vector>

class Segment3Dd;

class Plane {
public:
	Plane(const Point3Dd& p, const Point3Dd& normal);
    Plane(){};
public:
	int intersectPlane(Point3Dd& lp, Vector3Dd& ld, const Point3Dd& p, const Vector3Dd& n) const;

public:
	Point3Dd _p;
	Vector3Dd _normal;
};
#endif