/**************************************************************************************************
* @file    D:\KS-2013\SRC\kstudio\inc\BaseLib\ClipPolygon.h
*
* @brief    Declaration of the ClipPolygon class
* @author: Son
* @copyright (c) 2014 Kevvox company
* @history:
*
*  Date          | Author         | Note
*  ---------------------------------------------------------------------------------------
*      11/2014     Son               File created
*
**************************************************************************************************/

#ifndef BASELIB_CLIP_POLYGON
#define BASELIB_CLIP_POLYGON

#include "Point3Dd.h"
#include "PlanarContour.h"

struct ClVertex{
    Point3Dd *_coord;
    ClVertex *_pre, *_next;
    bool _intersect;
    bool _isEntry;
    ClVertex *_neighbor;
    float _alpha;
    char _status;
};

class ClipPolygon {
public:
    ClipPolygon(std::vector<Point3Dd*> &poly1, std::vector<Point3Dd*> &poly2, Point3Dd &norm);
    ~ClipPolygon();
    int startUpClipping();
    void initClVertex();
    int checkIntersection();
    void classifyIntersectionPoints();
    int getExtPolygonsOf(ClVertex *polyCl, std::vector<std::vector<Point3Dd *>> &exsubj);
    int getExtPolygonsOfSubj(std::vector<std::vector<Point3Dd *>> &exsubj);
    int getAllExtPolygons(std::vector<std::vector<Point3Dd *>> &exsubj);
    int checkValidInt();

private:
    PlanarPolygon *_subjpoly;
    PlanarPolygon *_clipPoly;
    ClVertex* _subjPolyCl;
    ClVertex* _clipPolyCl;
    std::vector<Point3Dd> _intPoints;
    bool _isReverseClip;
    bool _isStartUp;
};
#endif