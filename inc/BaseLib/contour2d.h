#ifndef CONTOUR2D_H
#define CONTOUR2D_H

#include <vector>

#include "Point2D.h"
#include "intersectpoint.h"

class Segment2Dd;
class Line2Dd;

class Contour2Dd;
class StlShell;

struct SegmentList: public std::vector<Segment2Dd*>
{
	typedef std::vector<Segment2Dd*> Parent;

	SegmentList(bool deleteItems = true);
	~SegmentList();
	void clear();

	const_reference at (size_type n) const	{return operator[](n);}
	reference  & at(size_type n)			{return operator[](n);}

private:
	//SegmentList(SegmentList const & seg);
protected:
	bool _deleteItems; // Whether the items should be deleted in destructor
};

struct ContourList: public std::vector<Contour2Dd*>
{
	typedef std::vector<Contour2Dd*> Parent;

	ContourList(bool deleteItems = true);
	~ContourList();
	void clear();
    void clearData();

	const_reference at (size_type n) const	{return operator[](n);}
	reference  & at(size_type n)			{return operator[](n);}

public:
	// Access the DeleteItems
	bool deleteItems(void) const			{ return(_deleteItems);			};

protected:
	bool _deleteItems; // Whether the items should be deleted in destructor
};

class Contour2Dd
{

public:
	Contour2Dd(double zValue = 0);
	~Contour2Dd();
public:
	void addSegment(Segment2Dd* segment);
	Segment2Dd* getSegment(int index);
	unsigned int getNumberOfSegment();
    bool intersect(Contour2Dd *other);
    bool isInsideContour(Contour2Dd *other);
	bool intersectSegment( std::vector<Point2Dd>& ip, 
		Segment2Dd* segment );
	bool intersectSegment( std::vector<ISPoint2D>& ip, 
		Segment2Dd* segment );

	bool intersectLine( std::vector<double>& ip, std::vector<Segment2Dd*>& is,
		const Point2Dd& p, const Point2Dd& d);
	bool intersectLine( std::vector<Point2Dd>& ip, std::vector<Segment2Dd*>& is,
		const Point2Dd& p, const Point2Dd& d);
	bool isClosed();
	
	void getBoundingBox(double& minx, double& miny, double& maxx, double& maxy);
	void getMaxBoundingBox(double& maxx, double& maxy);
	void getMinBoundingBox(double& minx, double& miny);
	double getNearestSegments(const Point2Dd& p, std::vector<Segment2Dd*>& segs);
	//bool isContainPoint(const Point2Dd& p);
	void setMarked(bool isMark);
	bool isMarked();
	bool isOpened();
	void setOpen(bool isOpen);
	bool checkOpen();
	void clear();
	Segment2Dd* getLastSegment();
	Segment2Dd* getFirstSegment();

	bool connectTo(Contour2Dd* contour);
	void getSegments(std::vector<Segment2Dd*>& segments);
	double squareDistanceToContour(Contour2Dd* contour);
	double squareOpenedDistance();
	bool connectItself(bool needCheck=false);
	void resetContourSegments();
	void rebuildOpenContour();
	bool isDiffirentDirection(Contour2Dd* contour);
	bool isLineContour();
	bool intersectXLine( std::vector<ISPoint2D>& ip, 
		Line2Dd* xLine);
	void removeConcidentItersectPoint(std::vector<ISPoint2D>& ips,
		std::vector<ISPoint2D>& new_ips);
    bool isPointInside(const Point2Dd& p);

	
    void checkDirection();
	bool isHollow();
	void setHollow(const bool& isHollow);

	void setParent(void* parrent);
	void* getParrent();
	bool hasOpenSegment();
	double getBoundingBoxArea();

	double getZValue() { return _zValue; }

	void setInside(bool isInside) { _isInside = isInside; }

	bool isInside() { return _isInside; }

    void setInverted(bool val) { _isInverted = val; }
    bool isInverted() { return _isInverted; }

public:
	static void markContourInOut(std::vector<Contour2Dd*>& contours);

	static void connectContours(std::vector<Contour2Dd*>& contours);

	static Contour2Dd* getBigestContour(std::vector<Contour2Dd*> contours);

public:

	//float _color[3];// test

	// Access the Segments
	const std::vector<Segment2Dd*>& segments(void) const		{ return(_segments);	};

	// Access the ZValue
	double zValue(void) const	{ return(_zValue);	};

private:
	std::vector<Segment2Dd*> _segments;
	Point2Dd _outsidePoint;

	double _zValue;// is 3D contour if z != 0
	bool   _isInside;
    bool   _isInverted;
	double _minx, _miny, _maxx, _maxy;
	bool   _isMarked;
	bool   _isOpened;
	bool   _isHollow;
	void*  _parrent;
};

#endif