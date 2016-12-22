/**************************************************************************************************
 * @file    D:\KS-2013\SRC\kstudio\inc\BaseLib\PlanarContour.h
 *
 * @brief    Declares the PlanarContour class
 * @author: Son
 * @copyright (c) 2014 Kevvox company
 * @history:
 *
 *  Date          | Author         | Note
 *  ---------------------------------------------------------------------------------------
 *  05/08/2014     Son               File created
 *
 **************************************************************************************************/

#ifndef __PlanarContour_H__
#define __PlanarContour_H__

#include "Point3Dd.h"
#include "deque"
#include "vector"
#include "algorithm"
#include "VBQuaternion.h"

//class Quaternion;

typedef std::deque<Point3Dd *> PlaConPointPtrList;

class PlanarContour {
public:
    PlanarContour();
    PlanarContour(const PlaConPointPtrList &polyline);
    PlanarContour(const std::vector<Point3Dd *> &polyline);
    PlanarContour(Point3Dd *p3ds, unsigned numpoints);
    ~PlanarContour();
    Point3Dd * operator[](unsigned idx) const { return _points[idx]; }
    inline unsigned getNumPoints() const { return _points.size(); }
    inline PlaConPointPtrList * getRefPointList() { return &_points; }
    inline const PlaConPointPtrList *getRefPointList() const { return &_points; }
    inline Point3Dd *getPointAt(unsigned i) const { return _points[i]; };
    inline Point3Dd *getPrePointAt(unsigned i) {
        if (i < 0 || i >= _points.size()) return NULL;
        if (i == 0)
            return getLastPoint();
        else return _points[i - 1];
    }

    inline Point3Dd *getNextPointAt(unsigned i) {
        if (i < 0 || i >= _points.size()) return NULL;
        if (i == _points.size() - 1) {
            return getFirstPoint();
        }
        else return _points[i + 1];
    }
    inline Point3Dd *getFirstPoint() const { return _points.front(); }
    inline Point3Dd *getLastPoint() const { return _points.back(); }
    inline void pushBackAPoint(Point3Dd *p3d) { _points.push_back(p3d); }
    inline void pushFrontAPoint(Point3Dd *p3d) { _points.push_front(p3d); }
    inline void removePointAt(unsigned i) { _points.erase(_points.begin() + i); }
    inline void removePoints(unsigned first, unsigned last) { _points.erase(_points.begin() + first, _points.begin() + last); }
    inline void setNormUp(const Vector3Dd &n) {
        _normUp = n; 
        checkDirection();
    }
    inline Vector3Dd getNormUp() const { return _normUp; }
    inline bool isCounterClockwise() { return _isCounterClockwise; }
    inline bool isClosed() { return *_points.front() == *_points.back(); }
    int findReflexVertices(std::vector<Point3Dd *> &reflexVs);
    int findReflexVertices(std::vector<int> &reflexVind);
    bool isPointInside(const Point3Dd &p);
    bool isPointInsideConvexContour(const Point3Dd &p);
    bool isANotIntersectContourInside(const PlanarContour &cnt);
    int findAPoint(const Point3Dd &p);
    void calcMinMaxCoord();
    inline void setMinMaxcoord(const Point3Dd &minc, const Point3Dd &maxc) {
        _minCoord = minc;
        _maxCoord = maxc;
    }
    inline void getMinMaxCoord(Point3Dd &mincoor, Point3Dd &maxcoor) const { 
        mincoor = _minCoord; 
        maxcoor = _maxCoord;
    }

    bool checkDirection();
    void rotate(const double angle, const Vector3Dd &axis, const Point3Dd &center = ZERO_VECTOR_3Dd);
    void rotate(VBQuaternion &quat, const Point3Dd &center = ZERO_VECTOR_3Dd);
    void transformToPlane(const Point3Dd &pv, const Vector3Dd &nv);
    void moveAlongNormal(double len);
    void close();
    void getMinMaxLenSegment(double &minlen, double &maxlen) const;
    void reverse();
    bool samplingContour(double d);
    void filterNonCorner();
#ifdef _DEBUG
    void dumpCad();
#endif
    static bool isValidPolygon(std::vector<Point3Dd*> &poly, const Vector3Dd &norm);
    static bool isCCWPolygon(std::vector<Point3Dd *> &polygon, const Vector3Dd &norm);

private:
    PlaConPointPtrList _points;
    bool _isCounterClockwise;
    Vector3Dd _normUp; /* up normal vector of plane of contour */
    Point3Dd _minCoord;
    Point3Dd _maxCoord;
};

typedef PlanarContour PlanarPolygon;
#endif // !__PlanarContour_H__
