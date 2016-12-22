/**************************************************************************************************
* @file    D:\KS-2013\SRC\kstudio\inc\BaseLib\CalcPolygonOffset.h
*
* @brief    Declaration of the CalcPolygonOffset class
* @author: Son
* @copyright (c) 2014 Kevvox company
* @history:
*
*  Date          | Author         | Note
*  ---------------------------------------------------------------------------------------
*      11/2014     Son               File created
*
**************************************************************************************************/

#ifndef BASELIB_CALC_POLYGON_OFFSET
#define BASELIB_CALC_POLYGON_OFFSET
#include "Point3Dd.h"
#include "PlanarContour.h"
#include "set"

struct OffVertex {
    Point3Dd _coord;
    Vector3Dd _v;  // velocity
};

struct OffEdge {
    OffVertex *_sv;
    OffVertex *_ev;
    Vector3Dd vector() {
        return _ev->_coord - _sv->_coord;
    }
    OffEdge *_preEdge;
    OffEdge *_nextEdge;
};

struct OffBiSegment {
    OffBiSegment() {
        _isOppositeColl = false;
    }

    Vector3Dd vector() {
        return Vector3Dd(_ev - _sv);
    }
    Point3Dd _sv;
    Point3Dd _ev;
    OffEdge *_preEdge;
    OffEdge *_nextEdge;
    unsigned char _isOppositeColl : 1;
    unsigned char : 7;
};

struct OffEvent {
    OffBiSegment *_bi1;
    OffBiSegment *_bi2;
    Point3Dd _intp;
    double _dd;
};

#define PRE_INDEX(i, sz) (i == 0 ? sz - 1 : i - 1)
#define NEXT_INDEX(i, sz) (i == sz - 1 ? 0 : i + 1)

class CalcPolygonOffset {
public:
    CalcPolygonOffset();
    CalcPolygonOffset(const std::vector<Point3Dd*> &poly,
                     const Vector3Dd &norm, double d);
    CalcPolygonOffset(const std::vector<Point3Dd> &poly,
                      const Vector3Dd &norm, double d);
    ~CalcPolygonOffset();
    void offset(std::vector<std::vector<Point3Dd *>> &offpolys);
    int processEvents(std::vector<OffBiSegment *> &availableBiSegment);
    void initStructure();
    void initEvents();
    OffEvent *getMinEvent();
    void removeEventHasBiSegment(OffBiSegment *biseg);
    int addEvent(OffEvent *event);
    void splitPolygonBySelfCut(std::vector<Point3Dd *> &polygon,
                               std::vector<std::vector<Point3Dd *>> &splitPolygons);
    void removeInvalidPoly(std::vector<std::vector<Point3Dd *>> &polygons);
    void removeWrongDirectionPoly(std::vector<std::vector<Point3Dd *>> &polygons);
    void removeIntPolygon(std::vector<std::vector<Point3Dd *>> &polygons);
    int nearIntOfSegmentOriginal(const Point3Dd &sp, const Point3Dd &ep, 
                                 const Point3Dd &nearPoint, Point3Dd &intp);

private:
    PlanarContour *_planarContour;
    double _d;
    Vector3Dd _norm;
    std::vector<OffVertex *> _vertices;
    std::vector<OffEdge *> _offEdges;
    std::vector<OffBiSegment *> _biSegments;
    std::vector<OffEvent *> _events;
    std::vector<OffEvent *> _removedEvents;
    std::vector<std::vector<Point3Dd *>> _invalidPolygons;
    bool _isInward;
};
#endif