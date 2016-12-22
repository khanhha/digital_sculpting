#include "segment3Dd.h"
#include <math.h>
#include "defined.h"

Segment3Dd::Segment3Dd() {

}
Segment3Dd::Segment3Dd(const Point3Dd& sp, const Point3Dd& ep) {
    _sp = sp;
    _ep = ep;
}
Segment3Dd::Segment3Dd( const Segment3Dd& segment ) {
    _sp = segment._sp;
    _ep = segment._ep;
}
Segment3Dd::~Segment3Dd() {

}

const int Segment3Dd::intersectAxisPlane(Point3Dd& ip, const double & offsetVal,
    const int icoord ) const// icoord: coord index
{
    double t;
    int nt = intersectAxisPlane(t, offsetVal, icoord);
    if ( 1 == nt )
        ip = _sp + (_ep - _sp) * t;
    return nt;
}

const int Segment3Dd::intersectAxisPlane(double& t, const double & offsetVal,
    const int icoord ) const// icoord: coord index
{
    Vector3Dd u =  _ep - _sp;
    Vector3Dd w = _sp;
    w.v[icoord] -= offsetVal;

    double D = u.v[icoord];
    double N = -w.v[icoord];
    if( fabs(D) < EPSILON_VAL_){
        if( fabs(N) < EPSILON_VAL_)// == 0 )
            return 2; // lie in plane
        else
            return 0; // not intersect
    }
    t = N / D;
    if( t < - EPSILON_VAL_ || t > 1 + EPSILON_VAL_)
        return 0;
    return 1;
}

 
const Point3Dd Segment3Dd::getFisrtPoint() const {
	return _sp;
}

const Point3Dd Segment3Dd::getSecondPoint() const{
	return _ep;
}


void Segment3Dd::getArrows(Segment3Dd& lArrow, Segment3Dd& rArrow) {
	//Point3Dd v = _sp - _ep;
	//double length = v.module();
	//double t = 0.8;
	//if (length > 2) {
	//	t = (length - 0.1)/length;
	//}
	//Point3Dd p = _sp + v * t;
	//Point3Dd perVector(-v._vcoords[1], v._vcoords[0]);
	//perVector.normalize();

	//double dis = length/10;
	//if (length > 2) {
	//	dis = 0.1;
	//}
	//Point3Dd p1 = p + perVector * dis;
	//Point3Dd p2 = p - perVector * dis;

	//lArrow = Segment3Dd(p1, _ep);
	//rArrow = Segment3Dd(p2, _ep);

}

bool Segment3Dd::isContainPoint(const Point3Dd& p, const bool& isOnLine) {
	Vector3Dd v1 = p - _sp;
	Vector3Dd v2 = p - _ep;
	if (isOnLine) {
		return v1.scalarProduct(v2) < 0;
	} else {
		double dis = p.calcSquareDistanceToLine(_sp, _ep - _sp);
		if (dis < EPSILON_VAL_) {
			return v1.scalarProduct(v2) < 0;
		}
	}
	return false;
}

void Segment3Dd::get2SupplementSegments(const Segment3Dd& seg, Segment3Dd& seg1, Segment3Dd& seg2) {

	Point3Dd v11 = _sp;
	Point3Dd v12 = _ep;
	Point3Dd pv11, pv12;
	v11.calcProjectionOntoLine(seg.getFisrtPoint(), seg.getVector(), pv11);
	v12.calcProjectionOntoLine(seg.getFisrtPoint(), seg.getVector(), pv12);

	Point3Dd v21 = seg.getFisrtPoint();
	Point3Dd v22 = seg.getSecondPoint();
	Vector3Dd vect = pv12 - pv11;
	if (vect.scalarProduct(seg.getVector()) < 0) {
		Point3Dd tmp = v21;
		v21 = v22;
		v22 = tmp;
	}

	seg1 = Segment3Dd(v11, v21);
	seg2 = Segment3Dd(v12, v22);
}


const Vector3Dd Segment3Dd::getVector() const {
	return _ep - _sp;
}