#ifndef RAY2D_H
#define RAY2D_H
#include "Point2D.h"
#include "contour2d.h"
#include <vector>
#include "segment2Dd.h"

class Segment2Dd;

class Ray2Dd {
public:
	Ray2Dd();
	Ray2Dd(const Point2Dd& sp, const Point2Dd& vector);
	~Ray2Dd();

public:
	bool checkintersectSegment(const Point2Dd& sp, const Point2Dd& ep);
	bool intersectSegment(Point2Dd& ip, const Point2Dd& sp, 
		const Point2Dd& ep);
	bool intersectSegment(ISPoint2D& ip, Segment2Dd* segment);
	bool intersectLine(Point2Dd& ip, const Point2Dd& sp, 
		const Point2Dd& ep);
	Point2Dd getOriginalPoint();
	Point2Dd getVector();
private:
	Point2Dd _sp;
	Point2Dd _vector;
};
#endif