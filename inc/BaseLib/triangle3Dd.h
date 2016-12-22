#ifndef TRIANGLE_3D_H
#define TRIANGLE_3D_H
#include <vector>
#include "Point2D.h"
#include "Point3Dd.h"


class Triangle3Dd
{
public:
	Triangle3Dd(const Point3Dd* vertexs, const Vector3Dd& normal);
	Triangle3Dd(const Point3Dd* vertexs);
	Triangle3Dd(const Point3Dd& p1, const Point3Dd& p2, const Point3Dd& p3);
	Triangle3Dd(const Point3Dd* p1, const Point3Dd* p2, const Point3Dd* p3, const Point3Dd* norm);
	Triangle3Dd();
	~Triangle3Dd();
public:
	void set(const Point3Dd& p1, const Point3Dd& p2, const Point3Dd& p3);
	int intersectAxisPlane( Point2Dd& ip1, Point2Dd& ip2, 
		const double & zValue, const int iCoord ) const;
	bool isInvertedNormal();
	void correctInvertedNormal();
	void getVertexts(Point3Dd vertexs[3]);
	Point3Dd getVertext(const int i);
	Point3Dd* getpVertext(const int i);
	Vector3Dd getNormal();
	Vector3Dd* getpNormal();
	void getZMinMax(double& min, double& max);
	int calcNormal();
	void offset(const double& d);
	void invertNormal();
public:

#ifdef _DEBUG
	int _ired;
#endif

private:
	Point3Dd _vertexs[3];
	Vector3Dd _normal;
};

#endif