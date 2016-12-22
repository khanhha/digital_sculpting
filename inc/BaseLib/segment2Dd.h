#ifndef SEGMENT2D_H
#define SEGMENT2D_H
#include "Point2D.h"
#include "contour2d.h"
#include "intersectpoint.h"

class Triangle3Dd;
class Segment2Dd;
class StlTriangle;// will move this call to stl lib later

class Segment2Dd {
public:
	Segment2Dd();
	Segment2Dd( const Point2Dd& firstPoint, const Point2Dd& lastPoint );
	Segment2Dd( const Segment2Dd& segment );
	~Segment2Dd();

public:
    void invert();

public:
	bool isConnectedTo(Segment2Dd* segment);

	bool intersectSegment(std::vector<Point2Dd>& ip, Segment2Dd* segment);
	bool intersectSegment(Point2Dd& ip, Segment2Dd* segment);
	bool intersectSegment(std::vector<ISPoint2D>& ip, Segment2Dd* segment);

	bool intersectLine(double& t, const Point2Dd& p, const Point2Dd& d);
	bool intersectLine(Point2Dd& ip, const Point2Dd& p, const Point2Dd& d);

	Point2Dd getOtherPoint(const Point2Dd& p);
	Point2Dd getFarPoint(const Point2Dd& p);
	bool isIntersectSegment(Segment2Dd* segment);
	Point2Dd getNearestPoint(const Point2Dd& p);
    double getSquareDistanceToPoint(const Point2Dd &p, bool &isToFisrtPoint);

	bool isParallel(Segment2Dd* segment);
	bool isSameLineWith(Segment2Dd* segment);
	bool isSameDirection(Segment2Dd* segment);

	bool isEqual(Segment2Dd* segment);

	void setSegment(const Point2Dd& sp, const Point2Dd& ep);

	bool containsPoint(const Point2Dd& p);

public:

    double  getMaxX() { return max2(_sp._vcoords[0], _ep._vcoords[0]); }

    double  getMaxY() { return max2(_sp._vcoords[1], _ep._vcoords[1]); }

    double  getMinX() { return min2(_sp._vcoords[0], _ep._vcoords[0]); }

    double  getMinY() { return min2(_sp._vcoords[1], _ep._vcoords[1]); }

    double  getYLeght() { return fabs(_sp._vcoords[1] - _ep._vcoords[1]); }

    void  setTriangle(Triangle3Dd *triangle){ _trianlge = triangle; }

    Triangle3Dd * getTriangle() { return _trianlge; }

    Point2Dd  getNormal() { return _normal; }

    bool  isInverted() { return _isInverted; }

    void  setInverted(bool isMarked) { _isInverted = isMarked;}

    void  getVector(Point2Dd &vec) { vec = _ep - _sp; }

    void  setNormal(const Point2Dd &normal) { _normal = normal; }

	// Access the start point
	const Point2Dd& sp(void) const	{ return(_sp);	};

	// Access the end point
	const Point2Dd& ep(void) const	{ return(_ep);	};

    Point2Dd const & getFirstPoint()  const { return _sp; }

    Point2Dd const & getSecondPoint()  const {return _ep; }

    Contour2Dd* getContour() { return _contour; }

    StlTriangle *getStlTriangle() { return _stlTriangle; }

    //void setObjectParrent(void *parrent) { _objectParrent = parrent; }

    //void *getObjectParrent() { return _objectParrent; }

    void setStlTriangle(StlTriangle *triangle) { _stlTriangle = triangle; }

    void setOpenAtStartPoint(const bool &isOpenAtSp) { _isOpenAtStartPoint = isOpenAtSp; }

    bool isOpenAtStartPoint() { return _isOpenAtStartPoint; }

    void  setNextSegment(Segment2Dd *nextSegment) { _nextSegment = nextSegment; }

    void  setPrevSegment(Segment2Dd *preSegment) { _prevSegment = preSegment; }

    Segment2Dd * nextSegment() { return _nextSegment; }

    Segment2Dd * prevSegment() { return _prevSegment; }

    void setVisitted(bool isVisit) { _visited = isVisit; }

    bool isVisitted() { return (_visited == true); }

    void setContour(Contour2Dd *contour) { _contour = contour; }

    double getZValue() { return _zValue; }

	// Access the ZValue
	double zValue(void) const	{ return(_zValue);	};

	double const length() const;
    double const length2() const;

    void getFirstCoord(Point2Dd& coord)  { coord = _sp; }
    void getSecondCoord(Point2Dd& coord) { coord = _ep; }

	// Access the Swapped
	bool isSwapped(void) const		{ return(_swapped);		};
	void setSwapped(bool swapped)	{ _swapped = swapped;	};

    void swapEndPoints();

protected:
	void reset();
protected:

protected:
	Point2Dd _sp; // Start point
	Point2Dd _ep; // End point

	Segment2Dd* _prevSegment; // Previous segment in the loop
	Segment2Dd* _nextSegment; // Next segment in the loop

	Contour2Dd* _contour; // Containing contour
	bool _visited; // Has been visited (for traversal)

	double _zValue; // convert to 3D if _zValue != 0
	Point2Dd _normal; // the vector that has outsize direction
	bool _isInverted; // 
	Triangle3Dd* _trianlge; // Containing StlTriangle

	StlTriangle* _stlTriangle;

	bool _isOpenAtStartPoint;
	bool _swapped; // Are the endpoints swapped during joining
};

#endif