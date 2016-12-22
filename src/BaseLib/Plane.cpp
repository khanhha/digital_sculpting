#include "plane.h"
#include "segment2Dd.h"
#include <vector>

Plane::Plane(const Point3Dd& p, const Point3Dd& normal)
	: _p(p)
	, _normal(normal)
{

}


int Plane::intersectPlane( Point3Dd& lp, Vector3Dd& ld, const Point3Dd& p, const Vector3Dd& n) const
{
	Vector3Dd u = _normal.crossProduct(n);
	double ax = (u.v[0] >= 0 ? u.v[0] : -u.v[0]);
	double ay = (u.v[1] >= 0 ? u.v[1] : -u.v[1]);
	double az = (u.v[2] >= 0 ? u.v[2] : -u.v[2]);

	// test if the two planes are parallel
	if ((ax + ay + az) < EPSILON_VAL_) {       // Pn1 and Pn2 are near parallel
		// test if disjoint or coincide
		Vector3Dd v = p - _p;
		if (_normal.scalarProduct(v) == 0)         // p lies in Pn1
			return 1;                   // Pn1 and Pn2 coincide
		else 
			return 0;                   // Pn1 and Pn2 are disjoint
	}
	// intersect in a line
	// first determine max abs coordinate of cross product
	int maxc;                      // max coordinate
	if (ax > ay) {
		if (ax > az)
			maxc = 1;
		else maxc = 3;
	}
	else {
		if (ay > az)
			maxc = 2;
		else maxc = 3;
	}

	// next, to get a point on the intersect line
	// zero the max coord, and solve for the other two
	Point3Dd ip;               // intersect point
	double d1, d2;           // the constants in the 2 plane equations
	d1 = -_normal.scalarProduct(_p);  // note: could be pre-stored with plane
	d2 = -n.scalarProduct(p);  // ditto

	switch (maxc) {            // select max coordinate
	case 1:                    // intersect with x=0
		ip.v[0] = 0;
		ip.v[1] = ( d2 * _normal.v[2] - d1 * n.v[2] ) / u.v[0];
		ip.v[2] = ( d1 * n.v[1] - d2 * _normal.v[1] ) / u.v[0];
		break;
	case 2:                    // intersect with y=0
		ip.v[0] = ( d1 * n.v[2] - d2 * _normal.v[2] ) / u.v[1];
		ip.v[1] = 0;
		ip.v[2] = ( d2 * _normal.v[0] - d1 * n.v[0] ) / u.v[1];
		break;
	case 3:                    // intersect with z=0
		ip.v[0] = ( d2 * _normal.v[1] - d1 * n.v[1] ) / u.v[2];
		ip.v[1] = ( d1 * n.v[0] - d2 * _normal.v[0] ) / u.v[2];
		ip.v[2] = 0;
	}
	lp = ip;
	ld = u;
	return 2;
}