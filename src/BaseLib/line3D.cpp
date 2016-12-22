#include "line3d.h"
#include "segment2Dd.h"
#include <vector>

Line3Dd::Line3Dd(const Point3Dd& p, const Point3Dd& v)
	: _p(p)
	, _v(v)
{
}
bool Line3Dd::intersectPlanarLine(Point3Dd& ip, const Point3Dd& p, const Vector3Dd& v) {
	Vector3Dd n1 = _v.crossProduct(v);
	Vector3Dd n = n1.crossProduct(v);
	int ret = this->intersectPlane(ip, p, n);
	return (ret == 1);
}


int Line3Dd::intersectPlane( Point3Dd& ip, const Point3Dd& onPoint, const Vector3Dd& normal) const
{
	Point3Dd ep = _p + _v;
	Vector3Dd u =  _p - ep;
	Vector3Dd w = _p - onPoint;
	double D = normal.scalarProduct( u );
	double N = -normal.scalarProduct( w );
	if( fabs(D) < EPSILON_VAL_){
		if( fabs(N) < EPSILON_VAL_)// == 0 )
			return 2; // in plane
		else
			return 0; // not intersect
	}
	double sI = N/D;
	ip.v[0] = _p.v[0] + sI * u.v[0];
	ip.v[1] = _p.v[1] + sI * u.v[1];
	ip.v[2] = _p.v[2] + sI * u.v[2];
	return 1;
}