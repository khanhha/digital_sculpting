#ifndef SEGMENT_3D_H
#define SEGMENT_3D_H

#include "Point3Dd.h"
#include "segment2Dd.h"
#include "triangle3Dd.h"

class Segment3Dd
{
public:
    Segment3Dd();
    Segment3Dd( const Point3Dd& firstPoint, const Point3Dd& lastPoint );
    Segment3Dd( const Segment3Dd& segment );
    ~Segment3Dd();
public:

    const int intersectAxisPlane(double& ip, const double & offsetVal, 
        const int icoord ) const/* icoord: coord index */;

    const int intersectAxisPlane(Point3Dd& ip, const double &offsetVal, 
        const int icoord ) const/* icoord: coord index */;
	const Point3Dd getFisrtPoint() const;
	const Point3Dd getSecondPoint() const;
	bool isContainPoint(const Point3Dd& p, const bool& isOnLine = false);
	void getArrows(Segment3Dd& lArrow, Segment3Dd& rArrow);

	void get2SupplementSegments(const Segment3Dd& seg, 
		Segment3Dd& seg1, Segment3Dd& seg2);
	const Vector3Dd getVector() const;
private:
    Point3Dd _sp;
    Point3Dd _ep;
	
};

typedef std::vector<Segment3Dd> Segment3DdListVector;
#endif