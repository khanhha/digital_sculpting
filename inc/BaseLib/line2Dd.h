#ifndef LINE2D_H
#define LINE2D_H
#include "Point2D.h"
#include "intersectpoint.h"
#include <vector>

class Segment2Dd;

class Line2Dd {
public:
	Line2Dd(const Point2Dd& p, const Point2Dd& v);

public:
	bool xLineIntersectXSegment(Point2Dd& ip, Segment2Dd* segment);

	bool xLineIntersectSegment( std::vector<ISPoint2D>& ips, 
		Segment2Dd* segment );
private:
	Point2Dd _p;
	Point2Dd _v;
};
#endif