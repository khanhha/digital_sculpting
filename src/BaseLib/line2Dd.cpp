#include "line2Dd.h"
#include "segment2Dd.h"
#include <vector>

Line2Dd::Line2Dd(const Point2Dd& p, const Point2Dd& v) {
	_p = p;
	_v = v;
}

bool Line2Dd::xLineIntersectXSegment(Point2Dd& ip, Segment2Dd* segment) {
	Point2Dd sp = segment->getFirstPoint();
	Point2Dd ep = segment->getSecondPoint();
	double yCoord = _p._vcoords[1];
	//if (fabs((sp._vcoords[1] - yCoord)) < EPSILON_VAL_) {
	//	ip = sp;
	//	return true;
	//} else if (fabs((ep._vcoords[1] - yCoord)) < EPSILON_VAL_) {
	//	ip = ep;
	//	return true;
	//}


	double dis1 = yCoord - sp._vcoords[1];
	double dis2 = yCoord - ep._vcoords[1];
	if (dis1 * dis2 > EPSILON_VAL_E1) {
		return false;
	}

	//if ((fabs(sp._vcoords[1] - ep._vcoords[1])) < EPSILON_VAL_) {
	if ((fabs(sp._vcoords[1] - ep._vcoords[1])) == 0) {// if this is very small, t will be very big, so it's OK.
		return false;
		//int debug = 1;
	}
	//if ((fabs(sp._vcoords[1] - ep._vcoords[1]) > EPSILON_VAL_)) {
	//	if (fabs(dis1) < EPSILON_VAL_) {
	//		ip = sp;
	//		return true;
	//	}
	//	if (fabs(dis2) < EPSILON_VAL_) {
	//		ip = ep;
	//		return true;
	//	}
	//}

	double t = (yCoord - sp._vcoords[1]) / (ep._vcoords[1] - sp._vcoords[1]);
	//if (t >= -EPSILON_VAL_*100000 && t <= 1 + 100000*EPSILON_VAL_) {
	if (t >= -EPSILON_VAL_ && t <= 1 + EPSILON_VAL_) {
	//if (t >= 0 && t <= 1) {
	//if (t >= EPSILON_VAL_ && t <= 1 - EPSILON_VAL_) {
		ip = sp + (ep - sp) * t;
		return true;
	}
	return false;
}

bool Line2Dd::xLineIntersectSegment( std::vector<ISPoint2D>& ips, 
	Segment2Dd* segment ) {
	Point2Dd ip;
	bool isIntersected = xLineIntersectXSegment(ip, segment);
	if (isIntersected) {
		ISPoint2D ips_;
		ips_.ip = ip;
		ips_.segment = segment;
		ips.push_back(ips_);
	}
	return isIntersected;
}