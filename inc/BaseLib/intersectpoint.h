#ifndef INTERSECSION_POINT_H
#define INTERSECSION_POINT_H
#include "Point2D.h"

class Segment2Dd;

class ISPoint2D {
public:
	ISPoint2D() {
		isDeleted = false;
        segment = nullptr;
	}
	ISPoint2D(const ISPoint2D& p) 
        : ip (p.ip)
        , segment (p.segment)
        , isDeleted (p.isDeleted)
    {
	}
	ISPoint2D(Point2Dd ip_, Segment2Dd* seg) 
        : ip (ip_)
        , segment (seg)
        , isDeleted (false)
    {
	}
	Point2Dd ip;
	Segment2Dd* segment;
	bool isDeleted;

	const bool operator<(  const ISPoint2D& p ) const {
		return ip < p.ip;
	}

	//ISPoint2D& operator=(  const ISPoint2D& p ) {
	//	ip = p.ip;
	//	segment = p.segment;
	//	isDeleted = p.isDeleted;

	//	return *this;
	//}

	ISPoint2D& operator=(const ISPoint2D& p) {
		ip = p.ip;
		segment = p.segment;
		isDeleted = p.isDeleted;

		return *this;
	}

	//const bool operator>(  const ISPoint& p ) const {
	//	return ip > p.ip;
	//}

};

#endif