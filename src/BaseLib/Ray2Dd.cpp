#include "Ray2Dd.h"
#include "Point2D.h"
#include "segment2Dd.h"

Ray2Dd::Ray2Dd()
{

}
Ray2Dd::Ray2Dd(const Point2Dd& sp, const Point2Dd& vector) {
	_sp = sp;
	_vector = vector;
}

Ray2Dd::~Ray2Dd() {

}

bool Ray2Dd::checkintersectSegment(const Point2Dd& sp, const Point2Dd& ep) {
	// in this case the ray and segment is not parallel
	Point2Dd u = _vector;
	Point2Dd v(ep - sp);
	if ( true == _sp.isConcident(sp)) {
		return true;
	}
	Point2Dd w = _sp - sp;
	double D = u.perp(v);
	if ( fabs(D) < EPSILON_VAL_ ) // parallel or coincident
		return false;
	//ASSERT(D!=0);
	double sI = v.perp(w) / D;// parameter of ray
	double tI = u.perp(w) / D;// parameter of segment
	if( sI < 0 || tI < 0 || tI > 1 ) {
		return false;
	}
	else {
		return true;
	}
}

bool Ray2Dd::intersectSegment(Point2Dd& ip, 
	const Point2Dd& sp, const Point2Dd& ep) {
	// in this case the ray and segment is not parallel
	Point2Dd u = _vector;
	Point2Dd v(ep - sp);
	if ( true == _sp.isConcident(sp)) {
		ip = _sp;
		return true;
	}
	Point2Dd w = _sp - sp;
	double D = u.perp(v);
	if ( fabs(D) < EPSILON_VAL_ ) // parallel or coincident
		return false;
	//ASSERT(D!=0);
	double sI = v.perp(w) / D;// parameter of ray
	double tI = u.perp(w) / D;// parameter of segment
	if( sI < 0 || tI < 0 || tI > 1 ) {
		return false;
	}
	else {
		ip = sp + (ep - sp) * tI;
		return true;
	}
}

bool Ray2Dd::intersectSegment(ISPoint2D& ip, Segment2Dd* segment) {
	Point2Dd sp = segment->getFirstPoint();
	Point2Dd ep = segment->getSecondPoint();

	// in this case the ray and segment is not parallel
	Point2Dd u = _vector;
	Point2Dd v(ep - sp);
	if ( true == _sp.isConcident(sp)) {
		ip.ip = _sp;
		ip.segment = segment;
		return true;
	}
	Point2Dd w = _sp - sp;
	double D = u.perp(v);
	if ( fabs(D) < EPSILON_VAL_ ) // parallel or coincident
		return false;
	//ASSERT(D!=0);
	double sI = v.perp(w) / D;// parameter of ray
	double tI = u.perp(w) / D;// parameter of segment
	if( sI < 0 || tI < 0 || tI > 1 ) {
		return false;
	}
	else {
		Point2Dd ip_ = sp + (ep - sp) * tI;
		ip.ip = ip_;
		ip.segment = segment;
		return true;
	}
}

Point2Dd Ray2Dd::getOriginalPoint() {
	return _sp;
}
Point2Dd Ray2Dd::getVector() {
	return _vector;
}

bool Ray2Dd::intersectLine(Point2Dd& ip, const Point2Dd& sp, 
	const Point2Dd& ep) {

	// in this case the ray and segment is not parallel
	Point2Dd u = _vector;
	Point2Dd v(ep - sp);
	if ( true == _sp.isConcident(sp)) {
		ip = _sp;
		return true;
	}
	Point2Dd w = _sp - sp;
	double D = u.perp(v);
	if ( fabs(D) < EPSILON_VAL_ ) // parallel or coincident
		return false;
	//ASSERT(D!=0);
	double sI = v.perp(w) / D;// parameter of ray
	//double tI = u.perp(w) / D;// parameter of segment
	if( sI < 0) {
		return false;
	}
	else {
		Point2Dd ip_ = _sp + _vector * sI;
		ip = ip_;
		return true;
	}
}