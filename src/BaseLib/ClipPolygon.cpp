/**************************************************************************************************
* @file    D:\KS-2013\SRC\kstudio\inc\BaseLib\ClipPolygon.cpp
*
* @brief    Definitions of the ClipPolygon class
* @author: Son
* @copyright (c) 2014 Kevvox company
* @history:
*
*  Date          | Author         | Note
*  ---------------------------------------------------------------------------------------
*      11/2014     Son               File created
*
**************************************************************************************************/

#include "ClipPolygon.h"
#include "util.h"


ClipPolygon::ClipPolygon(std::vector<Point3Dd*> &poly1,
                         std::vector<Point3Dd*> &poly2, Point3Dd &norm)
{
    _subjpoly = new PlanarPolygon(poly1);
    _clipPoly = new PlanarPolygon(poly2);
    _subjpoly->filterNonCorner();
    _clipPoly->filterNonCorner();

    _subjpoly->setNormUp(norm);
    _clipPoly->setNormUp(norm);

    //
    _isReverseClip = false;
    bool isCCW = _subjpoly->isCounterClockwise();
    if (isCCW == _clipPoly->isCounterClockwise())
    {
        _clipPoly->reverse();
        _isReverseClip = true;
    }
    _isStartUp = false;
    startUpClipping();
}

ClipPolygon::~ClipPolygon()
{
    if (_subjpoly)
        delete _subjpoly;
    if (_clipPoly)
        delete _clipPoly;

    ClVertex *curV = _subjPolyCl;
    do {
        ClVertex *tmp = curV->_next;
        if (curV->_intersect) {
            delete curV->_coord;
        }
        delete curV;
        curV = tmp;
    } while (curV != _subjPolyCl);

    curV = _clipPolyCl;
    do {
        ClVertex *tmp = curV->_next;
        if (curV->_intersect) {
            delete curV->_coord;
        }
        delete curV;
        curV = tmp;
    } while (curV != _clipPolyCl);
}

int ClipPolygon::startUpClipping()
{
    initClVertex();
    int ni = checkIntersection();
    if (ni < 2) {
        return 0;
    }
    if (false == checkValidInt()) {
        return 0;
    }
    classifyIntersectionPoints();
    _isStartUp = true;

    return 1;
}

void ClipPolygon::initClVertex()
{
    unsigned sz = _subjpoly->getNumPoints();
    _subjPolyCl = new ClVertex;
    _subjPolyCl->_coord = (*_subjpoly)[0];
    _subjPolyCl->_intersect = false;
    _subjPolyCl->_isEntry = false;
    _subjPolyCl->_neighbor = NULL;
    _subjPolyCl->_next = NULL;
    _subjPolyCl->_pre = NULL;
    _subjPolyCl->_status = 0;
    ClVertex *pre = _subjPolyCl;
    for (unsigned i = 1; i < sz; ++i)
    {
        ClVertex *cv = new ClVertex;
        cv->_coord = (*_subjpoly)[i];
        cv->_intersect = false;
        cv->_isEntry = false;
        cv->_neighbor = NULL;
        cv->_next = NULL;
        cv->_pre = pre;
        cv->_status = 0;
        pre->_next = cv;
        pre = cv;
    }
    pre->_next = _subjPolyCl;
    _subjPolyCl->_pre = pre;

    sz = _clipPoly->getNumPoints();
    _clipPolyCl = new ClVertex;
    _clipPolyCl->_coord = (*_clipPoly)[0];
    _clipPolyCl->_intersect = false;
    _clipPolyCl->_isEntry = false;
    _clipPolyCl->_neighbor = NULL;
    _clipPolyCl->_next = NULL;
    _clipPolyCl->_pre = NULL;
    pre = _clipPolyCl;
    for (unsigned i = 0; i < sz; ++i)
    {
        ClVertex *cv = new ClVertex;
        cv->_coord = (*_clipPoly)[i];
        cv->_intersect = false;
        cv->_isEntry = false;
        cv->_neighbor = NULL;
        cv->_next = NULL;
        cv->_pre = pre;
        cv->_status = 0;
        pre->_next = cv;
        pre = cv;
    }
    pre->_next = _clipPolyCl;
    _clipPolyCl->_pre = pre;
}

/*
** Find all intersection points between subject and clip polygon
*/
int ClipPolygon::checkIntersection()
{
    Point3Dd sp0, ep0, sp1, ep1, intp;
    bool isInt;
    int ind = _subjpoly->getNormUp().getIndexOfMaxAbsCoord();
    ClVertex *scur = _subjPolyCl;
    ClVertex *snxt = NULL;
    while (snxt != _subjPolyCl)
    {
        snxt = scur->_next;
        assert(snxt);
        ClVertex *ccur = _clipPolyCl;
        ClVertex *cnxt = NULL;
        sp0 = *scur->_coord;
        ep0 = *snxt->_coord;
        std::vector < ClVertex * > sIntList;
        while (cnxt != _clipPolyCl)
        {
            cnxt = ccur->_next;
            assert(cnxt);
            sp1 = *ccur->_coord;
            ep1 = *cnxt->_coord;
            isInt = BaseType::Util::calcIntPointOf2PlanarSegment(sp0, ep0, sp1, ep1, ind, intp);
            auto it = std::find(_intPoints.begin(), _intPoints.end(), intp);

            if (isInt) {
                if (it == _intPoints.end())
                {
                    _intPoints.push_back(intp);
                }
                ClVertex *nwver = new ClVertex();
                nwver->_coord = new Point3Dd(intp);
                nwver->_intersect = true;
                nwver->_status = 0;
                nwver->_alpha = float((intp.v[(ind + 1) % 3] - sp0.v[(ind + 1) % 3]) /
                    (ep0 - sp0).v[(ind + 1) % 3]);
                sIntList.push_back(nwver);

                // Add new intersection point to clip polygon
                ClVertex *nwver1 = new ClVertex();
                nwver1->_coord = new Point3Dd(intp);
                nwver1->_intersect = true;
                nwver1->_alpha = (float)((intp.v[(ind + 1) % 3] - sp1.v[(ind + 1) % 3]) /
                    (ep1 - sp1).v[(ind + 1) % 3]);
                nwver1->_pre = ccur;
                ccur->_next = nwver1;
                nwver1->_next = cnxt;
                cnxt->_pre = nwver1;

                nwver->_neighbor = nwver1;
                nwver1->_neighbor = nwver;
                nwver1->_status = 0;
            }
            ccur = cnxt;
        }

        std::sort(sIntList.begin(), sIntList.end(), [&](ClVertex *v1, ClVertex *v2) {
            return v1->_alpha < v2->_alpha;
        });

        // Add intersection points to subject polygon
        unsigned ni = sIntList.size();
        ClVertex *sv = scur;
        for (unsigned i = 0; i < ni; ++i)
        {
            ClVertex *iv = sIntList[i];
            iv->_pre = sv;
            iv->_next = NULL;
            sv->_next = iv;
            sv = iv;
        }
        sv->_next = snxt;
        snxt->_pre = sv;
        scur = snxt;
    }

    return _intPoints.size();
}

/*
** Mark the entry point flag for all intersetion points
*/
void ClipPolygon::classifyIntersectionPoints()
{
    ClVertex *cur = _subjPolyCl;
    bool isFirst = true;
    bool entryStatus = false;
    do
    {
        if (cur->_intersect) {
            if (isFirst) {
                if (*cur->_pre->_coord != *cur->_coord)
                {
                    Point3Dd tp = *cur->_pre->_coord;
                    if (cur->_pre->_intersect == true) {
                        tp = (*cur->_pre->_coord + *cur->_coord) / 2.0;
                    }

                    if (_clipPoly->isPointInside(tp)) {
                        cur->_isEntry = false;
                    }
                    else {
                        cur->_isEntry = true;
                    }
                }
                else if (*cur->_next->_coord != *cur->_coord) {
                    Point3Dd tp = *cur->_next->_coord;
                    if (cur->_next->_intersect == true) {
                        tp = (*cur->_next->_coord + *cur->_coord) / 2.0;
                    }

                    if (_clipPoly->isPointInside(tp)) {
                        cur->_isEntry = true;
                    }
                    else {
                        cur->_isEntry = false;
                    }
                }
                else {
                    assert(0);
                }
                entryStatus = cur->_isEntry;
                isFirst = false;
            }
            else {
                entryStatus = !entryStatus;
                cur->_isEntry = entryStatus;
            }
        }

        cur = cur->_next;
    } while (cur != _subjPolyCl);

    cur = _clipPolyCl;
    isFirst = true;
    do
    {
        if (cur->_intersect) {
            if (isFirst) {
                if (*cur->_pre->_coord != *cur->_coord)
                {
                    Point3Dd tp = *cur->_pre->_coord;
                    if (cur->_pre->_intersect == true) {
                        tp = (*cur->_pre->_coord + *cur->_coord) / 2.0;
                    }

                    if (_subjpoly->isPointInside(tp)) {
                        cur->_isEntry = false;
                    }
                    else {
                        cur->_isEntry = true;
                    }
                }
                else if (*cur->_next->_coord != *cur->_coord) {
                    Point3Dd tp = *cur->_next->_coord;
                    if (cur->_next->_intersect == true) {
                        tp = (*cur->_next->_coord + *cur->_coord) / 2.0;
                    }
                    if (_subjpoly->isPointInside(tp)) {
                        cur->_isEntry = true;
                    }
                    else {
                        cur->_isEntry = false;
                    }
                }
                else {
                    assert(0);
                }
                entryStatus = cur->_isEntry;
                isFirst = false;
            }
            else {
                entryStatus = !entryStatus;
                cur->_isEntry = entryStatus;
            }
        }

        cur = cur->_next;
    } while (cur != _clipPolyCl);
}

/*
** get all external polygons of a polygon (polyCl)
*/
int ClipPolygon::getExtPolygonsOf(ClVertex *polyCl, 
                                  std::vector<std::vector<Point3Dd *>> &exsubj)
{
    ClVertex *cur = polyCl;
    bool needReverse = cur == _clipPolyCl && _isReverseClip;
    do {
        if (cur->_intersect && cur->_isEntry &&cur->_status == 0)
        {
            std::vector < Point3Dd * > epoly;
            ClVertex *tcur = cur;
            do {
                if (epoly.size() == 0 || *tcur->_coord != *epoly.back())
                    epoly.push_back(new Point3Dd(tcur->_coord));
                if (tcur->_intersect && tcur->_isEntry) { // Entry point
                    if (tcur->_status == 1) {
                        break;
                    }
                    tcur->_status = 1;
                    ClVertex *nei = tcur->_neighbor;
                    if (nei->_isEntry) {
                        nei = nei->_next;
                        while (!nei->_intersect)
                        {
                            if (epoly.size() == 0 || *nei->_coord != *epoly.back())
                                epoly.push_back(new Point3Dd(nei->_coord));
                            nei = nei->_next;
                        };
                        tcur = nei->_neighbor;
                    }
                    else {
                        tcur = tcur->_next;
                    }

                }
                else {
                    tcur = tcur->_next;
                }
            } while (tcur != cur);
            if (PlanarPolygon::isValidPolygon(epoly, _subjpoly->getNormUp())) {
                if (needReverse) {
                    std::reverse(epoly.begin(), epoly.end());
                }
                exsubj.push_back(epoly);

            }
            else {
                for (unsigned ip = 0; ip < epoly.size(); ++ip)
                {
                    delete epoly[ip];
                }
            }
        }
        cur = cur->_next;
    } while (cur != polyCl);

    return exsubj.size();
}

/*
** get all external polygons of subject polygon
*/
int ClipPolygon::getExtPolygonsOfSubj(std::vector<std::vector<Point3Dd *>> &exsubj)
{
    if (0 == _isStartUp) {
        return 0;
    }
    return getExtPolygonsOf(_subjPolyCl, exsubj);
}

/*
** get all external polygons
*/
int ClipPolygon::getAllExtPolygons(std::vector<std::vector<Point3Dd *>> &expoly)
{
    if (0 == _isStartUp) {
        return 0;
    }
    getExtPolygonsOf(_subjPolyCl, expoly);
    getExtPolygonsOf(_clipPolyCl, expoly);

    return expoly.size();
}

/*
** Check valid intersection
*/
int ClipPolygon::checkValidInt()
{
    bool hasCollision = false;
    ClVertex *v = _subjPolyCl;
    do {
        if (v->_intersect && v->_alpha > EPSILON_VAL_ &&
            v->_alpha < 1 - EPSILON_VAL_)
        {
            hasCollision = true;
            break;
        }
        v = v->_next;
    } while (v != _subjPolyCl);

    if (hasCollision) return 1;
    return 0;
}