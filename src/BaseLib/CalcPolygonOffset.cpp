/**************************************************************************************************
* @file    D:\KS-2013\SRC\kstudio\inc\BaseLib\CalcPolygonOffset.cpp
*
* @brief    Definitions of the CalcPolygonOffset class
* @author: Son
* @copyright (c) 2014 Kevvox company
* @history:
*
*  Date          | Author         | Note
*  ---------------------------------------------------------------------------------------
*      11/2014     Son               File created
*
**************************************************************************************************/

#include "CalcPolygonOffset.h"
#include "util.h"
#include "ClipPolygon.h"
#if 0
#ifdef _DEBUG
#include "Diagnotics/AutoCadAutomator.h"
#endif
#endif

CalcPolygonOffset::CalcPolygonOffset()
{

}

CalcPolygonOffset::CalcPolygonOffset(const std::vector<Point3Dd*> &poly, 
                                     const Vector3Dd &norm,
                                     double d)
{
    _planarContour = new PlanarContour(poly);
    _planarContour->setNormUp(norm);
    _d = d;
    _norm = norm;
    _isInward = d > -EPSILON_VAL_;
}

CalcPolygonOffset::~CalcPolygonOffset()
{
    if (_planarContour)
    {
        delete _planarContour;
    }

    for (unsigned i = 0; i < _removedEvents.size(); ++i)
    {
        delete _removedEvents[i];
    }

    for (unsigned i = 0; i < _vertices.size(); ++i)
    {
        delete _vertices[i];
    }

    for (unsigned i = 0; i < _offEdges.size(); ++i)
    {
        delete _offEdges[i];
    }

    for (unsigned i = 0; i < _biSegments.size(); ++i)
    {
        delete _biSegments[i];
    }

    for (unsigned i = 0; i < _invalidPolygons.size(); ++i)
    {
        for (unsigned k = 0; k < _invalidPolygons[i].size(); ++k)
        {
            delete _invalidPolygons[i][k];
        }
    }
}

/*
** Do offset the polygon and output all polygon offseted
*/
void CalcPolygonOffset::offset(std::vector<std::vector<Point3Dd *>> &offpolys)
{
    _planarContour->filterNonCorner();
    initStructure();
    initEvents();

    std::vector<OffBiSegment *> availableBiSegment;
    processEvents(availableBiSegment);
    unsigned sz = availableBiSegment.size();

    std::vector<Point3Dd *> offPolyline;
#ifdef _DEBUG0
    //Acad::start();
    _planarContour->dumpCad();
#endif
    for (unsigned i = 0; i < sz; ++i)
    {
#ifdef _DEBUG0
        Acad::drawLine(availableBiSegment[i]->_sv, availableBiSegment[i]->_ev, 2);
#endif
        if (availableBiSegment[i]->_isOppositeColl) {
            Point3Dd intp1,intp2;
            int rc1 = nearIntOfSegmentOriginal(availableBiSegment[PRE_INDEX(i, sz)]->_ev, 
                                               availableBiSegment[i]->_ev,
                                               availableBiSegment[i]->_sv, intp1);
            int rc2 = nearIntOfSegmentOriginal(availableBiSegment[NEXT_INDEX(i, sz)]->_ev,
                                               availableBiSegment[i]->_ev,
                                               availableBiSegment[i]->_sv, intp2);

            if (rc1 == 1 && rc2 == 1) {
                if (offPolyline.empty() || intp1 != *offPolyline.back())
                    offPolyline.push_back(new Point3Dd(intp1));
                if (offPolyline.empty() || intp2 != *offPolyline.back())
                    offPolyline.push_back(new Point3Dd(intp2));
            }
            else {
                if (offPolyline.empty() || *offPolyline.back() != availableBiSegment[i]->_ev)
                    offPolyline.push_back(new Point3Dd(availableBiSegment[i]->_ev));
            }
        }
        else {
            if (offPolyline.empty() || *offPolyline.back() != availableBiSegment[i]->_ev)
                offPolyline.push_back(new Point3Dd(availableBiSegment[i]->_ev));
        }
    }
#ifdef _DEBUG0
    for (unsigned i = 0; i < _biSegments.size(); i++)
    {
        Acad::drawLine(_biSegments[i]->_sv, _biSegments[i]->_ev, 2);
    }
#endif

    splitPolygonBySelfCut(offPolyline, offpolys);
    removeInvalidPoly(offpolys);
}

/*
** Process all events
*/
int CalcPolygonOffset::processEvents(std::vector<OffBiSegment *> &availableBiSegment)
{
    int ind = _norm.getIndexOfMaxAbsCoord();
    availableBiSegment.assign(_biSegments.begin(), _biSegments.end());
    auto findbi = [&](OffBiSegment *bi) -> int {
        auto it = std::find_if(availableBiSegment.begin(), availableBiSegment.end(),
            [&](OffBiSegment * bbi) ->bool {
            return bi == bbi;
        });

        if (it == availableBiSegment.end())
            return -1;
        return it - availableBiSegment.begin();
    };

    while (false == _events.empty())
    {
        OffEvent *event = getMinEvent();
        OffEdge *ed1 = event->_bi1->_preEdge;
        OffEdge *ed2 = event->_bi2->_nextEdge;
        Point3Dd ip;
        bool b = BaseType::Util::calcIntPointOf2PlanarLine(
            ed1->_sv->_coord + _d * ed1->_sv->_v, ed1->vector(),
            ed2->_sv->_coord + _d * ed2->_sv->_v, ed2->vector(),
            ind, ip);
        //assert(b);
        if (false == b)
            continue;

        Vector3Dd v = event->_bi1->vector(); v.unit();
        Vector3Dd vv = event->_bi2->vector(); vv.unit();
        v = (v + vv);
        bool isOppositeColl = false;
        if (v.dot(ip - event->_intp) < -EPSILON_VAL_) {
            OffEdge *edge = event->_bi1->_preEdge;
            do {
                if (BaseType::Util::is2SegmentCollision(ip, event->_intp, edge->_sv->_coord, edge->_ev->_coord, ind)) {
                    isOppositeColl = true;
                    break;
                }
                edge = edge->_nextEdge;
            } while (edge != event->_bi2->_nextEdge);
        }
        event->_bi1->_ev = event->_intp;
        event->_bi2->_ev = event->_intp;
        int id = findbi(event->_bi1);
        assert(id != -1);
        removeEventHasBiSegment(event->_bi1);
        availableBiSegment.erase(availableBiSegment.begin() + id);

        int id2 = findbi(event->_bi2);
        assert(id2 != -1);
        removeEventHasBiSegment(event->_bi2);
        availableBiSegment.erase(availableBiSegment.begin() + id2);

        // create new Bisector
        OffBiSegment *nwBi = new OffBiSegment();
        nwBi->_sv = event->_intp;
        nwBi->_ev = ip;
        nwBi->_preEdge = ed1;
        nwBi->_nextEdge = ed2;
        if (isOppositeColl) nwBi->_isOppositeColl = true;
        int sz = availableBiSegment.size();
        if (id >= sz) {
            id = sz;
            availableBiSegment.push_back(nwBi);
        }
        else
            availableBiSegment.insert(availableBiSegment.begin() + id, nwBi);

        _biSegments.push_back(nwBi);
        sz = availableBiSegment.size();

        // create new event
        OffBiSegment *pb = availableBiSegment[PRE_INDEX(id, sz)];
        OffBiSegment *nb = availableBiSegment[NEXT_INDEX(id, sz)];
        b = BaseType::Util::calcIntPointOf2PlanarSegment(nwBi->_sv, nwBi->_ev,
            pb->_sv, pb->_ev, ind, ip);
        OffEvent *event1 = NULL, *event2 = NULL;
        if (b) {
            event1 = new OffEvent();
            event1->_bi1 = pb;
            event1->_bi2 = nwBi;
            event1->_dd = ip.calcSquareDistanceToLine(pb->_nextEdge->_sv->_coord,
                pb->_nextEdge->vector());
            event1->_intp = ip;
        }
        b = BaseType::Util::calcIntPointOf2PlanarSegment(nwBi->_sv, nwBi->_ev,
            nb->_sv, nb->_ev, ind, ip);
        if (b) {
            event2 = new OffEvent();
            event2->_bi1 = nwBi;
            event2->_bi2 = nb;
            event2->_dd = ip.calcSquareDistanceToLine(nb->_preEdge->_sv->_coord,
                nb->_preEdge->vector());
            event2->_intp = ip;
        }

        if (event1 && event2) {
            if (event1->_dd < event2->_dd) {
                addEvent(event1);
                delete event2;
            }
            else {
                addEvent(event2);
                delete event1;
            }
        }
        else if (event1){
            addEvent(event1);
        }
        else if (event2) {
            addEvent(event2);
        }
    }

    return 1;
}

/*
** Initialize vertices, bisegments, edges
*/
void CalcPolygonOffset::initStructure()
{
    int ind = _norm.getIndexOfMaxAbsCoord();
    unsigned sz = _planarContour->getNumPoints();
    std::vector<int> reflexVertices;
    _planarContour->findReflexVertices(reflexVertices);
    std::vector <bool> markReflexVertex(sz, false);
    for (unsigned i = 0; i < reflexVertices.size(); ++i)
    {
        markReflexVertex[reflexVertices[i]] = true;
    }

    _vertices.resize(sz);
    _offEdges.resize(sz);
    _biSegments.resize(sz);
    Vector3Dd v1, v2, bi;
    // init Vertex
    for (unsigned i = 0; i < sz; ++i)
    {
        OffVertex *offv = new OffVertex();
        _vertices[i] = (offv);
        offv->_coord = *(*_planarContour)[i];
        if (markReflexVertex[i]) {
            v1 = offv->_coord - *_planarContour->getPrePointAt(i); v1.unit();
            v2 = offv->_coord - *_planarContour->getNextPointAt(i); v2.unit();
        } else {
            v1 = *_planarContour->getPrePointAt(i) - offv->_coord; v1.unit();
            v2 = *_planarContour->getNextPointAt(i) - offv->_coord; v2.unit();
        }
        bi = v1 + v2; bi.unit();
        double l = 1 / (bi.cross(v1)).length();
        offv->_v = l * bi;
    }

    // init Edges
    for (unsigned i = 0; i < sz; ++i)
    {
        OffEdge *edge = new OffEdge();
        edge->_sv = _vertices[i];
        edge->_ev = _vertices[(i + 1) % sz];
        _offEdges[i] = edge;
        if (i == sz - 1) {
            edge->_nextEdge = _offEdges[0];
            _offEdges[0]->_preEdge = edge;
            edge->_preEdge = _offEdges[i - 1];
            _offEdges[i - 1]->_nextEdge = edge;
        }
        else if (i > 0) {
            edge->_preEdge = _offEdges[i - 1];
            _offEdges[i - 1]->_nextEdge = edge;
        }
    }

    // init BiSegment
    for (unsigned i = 0; i < sz; ++i)
    {
        OffBiSegment *biseg = new OffBiSegment();
        biseg->_sv = _vertices[i]->_coord;
        biseg->_ev = biseg->_sv + _d * _vertices[i]->_v;
        biseg->_preEdge = _offEdges[i == 0 ? (sz - 1) : i - 1];
        biseg->_nextEdge = _offEdges[i];
        _biSegments[i] = biseg;
    }
}

/*
** Find and initialize all events
*/
void CalcPolygonOffset::initEvents()
{
    int ind = _norm.getIndexOfMaxAbsCoord();
    unsigned sz = _biSegments.size();
    for (unsigned i = 0; i < sz; ++i)
    {
        OffBiSegment *nxt = _biSegments[(i + 1) % sz];
        OffBiSegment *cur = _biSegments[i];
        Point3Dd intp;
        bool b = false;
        double d = DBL_MAX;
        OffEvent *nwEvent = NULL;
        b = BaseType::Util::calcIntPointOf2PlanarSegment(nxt->_sv, nxt->_ev, 
                                                         cur->_sv, cur->_ev, 
                                                         ind, intp);
        if (b) {
            double d = intp.calcSquareDistanceToLine(_offEdges[i]->_sv->_coord,
                                                     _offEdges[i]->vector());
            nwEvent = new OffEvent();
            nwEvent->_bi1 = cur;
            nwEvent->_bi2 = nxt;
            nwEvent->_intp = intp;
            nwEvent->_dd = d;
            _events.push_back(nwEvent);
        }
    }

    std::sort(_events.begin(), _events.end(), [&](OffEvent *event1, OffEvent *event2) -> bool {
        return event1->_dd > event2->_dd;
    });
}

/*
** Get the earliest event
*/
OffEvent *CalcPolygonOffset::getMinEvent()
{
    OffEvent *ev = _events.back();
    _events.pop_back();
    _removedEvents.push_back(ev);
    return ev;
}

/*
** Remove all events related to biseg
*/
void CalcPolygonOffset::removeEventHasBiSegment(OffBiSegment *biseg)
{
    unsigned sz = _events.size();
    for (unsigned i = 0; i < sz; ++i)
    {
        if (_events[i]->_bi1 == biseg ||
            _events[i]->_bi2 == biseg)
        {
            _removedEvents.push_back(_events[i]);
            _events.erase(_events.begin() + i);
            i--;
            sz--;
        }
    }
}

/*
** Add event to the descending Event list
*/
int CalcPolygonOffset::addEvent(OffEvent *event)
{
    unsigned sz = _events.size();
    unsigned start = 0;
    unsigned end = sz - 1;
    if (sz == 1)
    {
        if (_events[0]->_dd < event->_dd)
        {
            _events.insert(_events.begin(), event);
            return 0;
        }
        else {
            _events.push_back(event);
            return 1;
        }
    }
    else if (sz == 0)
    {
        _events.push_back(event);
        return 0;
    }

    while (start < end) {
        assert(start != end);
        if (end - start == 1) {
            if (event->_dd > _events[start]->_dd)
            {
                _events.insert(_events.begin() + start, event);
                return start;
            }
            else if (event->_dd < _events[end]->_dd) {
                _events.push_back(event);
                return end + 1;
            }
            else {
                _events.insert(_events.begin() + end, event);
                return end;
            }
        }
        unsigned n = (start + end) / 2;
        if (event->_dd > _events[n]->_dd)
        {
            end = n;
        }
        else {
            start = n;
        }
    }

    return start;
}

/*
** Split to separate polygons, non selft cut, non collision together
*/
void CalcPolygonOffset::splitPolygonBySelfCut(std::vector<Point3Dd*> &polygon, 
                                              std::vector<std::vector<Point3Dd*>> &splitPolygons)
{
    int ind = _norm.getIndexOfMaxAbsCoord();
    splitPolygons.push_back(polygon);
    unsigned index = 0;
#ifdef _DEBUG
    int count = 0;
#endif
    while (index < splitPolygons.size()) {
#ifdef _DEBUG0
        count++;
        if (count == 2) {
            //break;
        }
#endif
        /*
        ** Find intersection points and split to multi polygons that non selft cut
        */
        std::vector< Point3Dd *> &ipoly = splitPolygons[index];
        std::vector<Point3Dd *> nwpoly;
        Point3Dd sp0, ep0, sp1, ep1, intp;
        bool br = false;
        for (unsigned i = 0; i < ipoly.size(); i++)
        {
            sp0 = *ipoly[i];
            ep0 = *ipoly[(i + 1) % ipoly.size()];
            for (unsigned j = i + 2; j < ipoly.size(); j++)
            {
                if (i == 0 && j == ipoly.size() - 1) continue;
                sp1 = *ipoly[j];
                ep1 = *ipoly[(j + 1) % ipoly.size()];
                bool b = BaseType::Util::calcIntPointOf2PlanarSegment(sp0, ep0, sp1, ep1, ind, intp);
                if (b) {
                    nwpoly.assign(ipoly.begin() + i + 1, ipoly.begin() + j + 1);
                    if (intp != sp1 && intp != ep0)
                        nwpoly.push_back(new Point3Dd(intp));

                    ipoly.erase(ipoly.begin() + i + 1, ipoly.begin() + j + 1);
                    if (intp != sp0 && intp != ep1)
                        ipoly.insert(ipoly.begin() + i + 1, new Point3Dd(intp));
                    br = true;
                    break;
                }
            }
            if (br)
                break;
        }
        if (nwpoly.size() > 0) {
            splitPolygons.push_back(nwpoly);
            continue;
        }

        index++;
    }

#ifdef _DEBUG
    //Acad::start(0);
    for (unsigned i = 0; i < splitPolygons.size(); ++i)
    {
        std::vector<Point3Dd> cadpoly;
        cadpoly.assign(splitPolygons[i].begin(), splitPolygons[i].end());
#if 0
		Acad::draw3dPolygon(cadpoly, 125);
#endif
    }
#endif
    /* 
    ** split by clip polygons an get the external one
    */
    for (unsigned i = 0; i < splitPolygons.size() - 1;++i)
    {
        for (unsigned j = i + 1; j < splitPolygons.size(); ++j)
        {

            ClipPolygon clip(splitPolygons[i], splitPolygons[j], _norm);
            std::vector<std::vector < Point3Dd * >> externPolygons;
            if (clip.getAllExtPolygons(externPolygons))
            {
                _invalidPolygons.push_back(splitPolygons[i]);
                _invalidPolygons.push_back(splitPolygons[j]);

                splitPolygons.erase(splitPolygons.begin() + j);
                splitPolygons.erase(splitPolygons.begin() + i);
                splitPolygons.insert(splitPolygons.begin() + i,
                    externPolygons.begin(), externPolygons.end());
                i--;
                break;
            }
        }
    }

#ifdef _DEBUG0
    for (unsigned i = 0; i < splitPolygons.size(); ++i)
    {
        std::vector<Point3Dd> cadpoly;
        cadpoly.assign(splitPolygons[i].begin(), splitPolygons[i].end());
        Acad::draw3dPolygon(cadpoly, 75);
    }
#endif
}

/*
** Remove all invalid polygon
*/
void CalcPolygonOffset::removeInvalidPoly(std::vector<std::vector<Point3Dd *>> &polygons)
{
    /*
    ** remove all invalid polygon that has area == 0
    */
    for (unsigned i = 0; i < polygons.size(); ++i)
    {
        if (false == PlanarPolygon::isValidPolygon(polygons[i], _norm)) {
            polygons.erase(polygons.begin() + i);
            i--;
        }
    }

    /*
    ** remove all polygons that has difference direction with original
    */
    removeWrongDirectionPoly(polygons);
#ifdef _DEBUG
    for (unsigned i = 0; i < polygons.size(); ++i)
    {
        std::vector<Point3Dd> cadpoly;
        cadpoly.assign(polygons[i].begin(), polygons[i].end());
#if 0
		Acad::draw3dPolygon(cadpoly, 75);
#endif
    }
#endif
    /*
    ** Remove all polygons that collision with bisector shapes
    */
    removeIntPolygon(polygons);

    /*
    ** Remove all polygons that outside (inside) fo the inward (outward) offset type
    */
    for (unsigned i = 0; i < polygons.size(); ++i)
    {
        for (unsigned k = 0; k < polygons[i].size(); ++k)
        {
            bool checkSide = !_isInward;
            if (checkSide == _planarContour->isPointInside(*polygons[i][k])) {
                _invalidPolygons.push_back(polygons[i]);
                polygons.erase(polygons.begin() + i);
                i--;
                break;
            }
        }
    }

#ifdef _DEBUG0
    for (unsigned i = 0; i < polygons.size(); ++i)
    {
        std::vector<Point3Dd> cadpoly;
        cadpoly.assign(polygons[i].begin(), polygons[i].end());
        Acad::draw3dPolygon(cadpoly, 75);
    }
#endif
}

void CalcPolygonOffset::removeWrongDirectionPoly(std::vector<std::vector<Point3Dd *>> &polygons)
{
    bool isccw = _planarContour->isCounterClockwise();
    for (unsigned i = 0; i < polygons.size(); ++i)
    {
        std::vector<Point3Dd *> ipoly = polygons[i];
        bool ic = PlanarPolygon::isCCWPolygon(ipoly, _norm);
        if (ic != isccw) {
            _invalidPolygons.push_back(polygons[i]);
            polygons.erase(polygons.begin() + i);
            i--;
        }
    }
}

void CalcPolygonOffset::removeIntPolygon(std::vector<std::vector<Point3Dd *>> &polygons)
{
    int ind = _planarContour->getNormUp().getIndexOfMaxAbsCoord();
    // detect and remove polygon which intersect with bisectors
    Point3Dd sp0, ep0, sp1, ep1;
    Point3Dd sp2, ep2;
    unsigned nbisec = _planarContour->getNumPoints();
    for (unsigned i = 0; i < polygons.size(); ++i)
    {
        bool isColl = false;
        bool needClip = false;
        std::vector<Point3Dd *> &ipoly = polygons[i];
        std::vector<Point3Dd *>quad(4);

        for (unsigned t = 0; t < nbisec; t++)
        {
            int nt = NEXT_INDEX(t, nbisec);
            if (_biSegments[t]->_isOppositeColl ||
                _biSegments[nt]->_isOppositeColl)
                continue;
            sp1 = _biSegments[t]->_sv;
            ep1 = _biSegments[t]->_ev;
            if (ep1 == sp0 || ep1 == ep0) {
                continue;
            }
            sp2 = _biSegments[nt]->_sv;
            ep2 = _biSegments[nt]->_ev;
            if (ep2 == sp0 || ep2 == ep0)
                continue;
            Vector3Dd v0 = ep0 - sp0; v0.unit();
            Vector3Dd v1 = ep2 - ep1; v1.unit();
            double dot = v0.dot(v1);
            if (fabs(dot - 1.0) < EPSILON_VAL_E3)
                continue;

            quad[0] = &sp1;
            quad[1] = &ep1;
            quad[2] = &ep2;
            quad[3] = &sp2;
            if (BaseType::Util::is2SegmentCollision(sp1, ep1, sp2, ep2, ind))
            {
                continue;
            }
            unsigned sz = ipoly.size();
            sp0 = *ipoly[sz - 1];
            bool isAllInside = true;
            for (unsigned k = 0; k < sz; ++k)
            {
                ep0 = *ipoly[k];
                bool bb = BaseType::Util::isQuadCollisionWithSegment(quad, sp0, ep0, true);
                if (bb) {
                    isColl = true;
                    break;
                }
                sp0 = ep0;
            }

            if (isColl)
            {
                _invalidPolygons.push_back(polygons[i]);
                polygons.erase(polygons.begin() + i);
                i--;
                break;
            }
        }

    }
#if 0
    for (unsigned i = 0; i < intPolygons.size(); ++i)
    {
        std::vector<Point3Dd *> &ipoly = intPolygons[i];
        unsigned sz = ipoly.size();
        sp0 = *ipoly[sz - 1];
        bool isInvalidInt = false;
        for (unsigned t = 0; t < nbisec; t++)
        {
            int nt = NEXT_INDEX(t, nbisec);
            sp1 = _biSegments[t]->_sv;
            ep1 = _biSegments[t]->_ev;
            if (ep1 == sp0 || ep1 == ep0) {
                continue;
            }
            sp2 = _biSegments[nt]->_sv;
            ep2 = _biSegments[nt]->_ev;
            if (ep2 == sp0 || ep2 == ep0)
                continue;
            Vector3Dd v0 = ep0 - sp0; v0.unit();
            Vector3Dd v1 = ep2 - ep1; v1.unit();
            double dot = v0.dot(v1);
            if (fabsf(dot - 1.0) < EPSILON_VAL_E3)
                continue;
            std::vector<Point3Dd *>quad(4);
            quad[0] = &sp1;
            quad[1] = &ep1;
            quad[2] = &ep2;
            quad[3] = &sp2;
            if (BaseType::Util::is2SegmentCollision(sp1, ep1, sp2, ep2, ind))
            {
                continue;
            }
            unsigned sz = ipoly.size();
            sp0 = *ipoly[sz - 1];
            bool hasColl = false;
            bool isAllInside = true;
            for (unsigned k = 0; k < sz; ++k)
            {
                ep0 = *ipoly[k];
                bool bb = BaseType::Util::isQuadCollisionWithSegment(quad, sp0, ep0, true);
                if (bb) {
                    hasColl = true;
                    bool b1 = BaseType::Util::isPointInConvexPolygon3D(sp0, quad, _norm, false);
                    bool b2 = BaseType::Util::isPointInConvexPolygon3D(ep0, quad, _norm, false);
                    //assert(b1 || b2);
                    if (false == b1 || false == b2) {
                        isAllInside = false;
                        break;
                    }
                }
                sp0 = ep0;
            }

            if (hasColl == true && isAllInside == true) {
                isInvalidInt = true;
                break;
            }
        }
        if (isInvalidInt) {
            intPolygons.erase(intPolygons.begin() + i);
            i--;
        }
    }

    for (unsigned i = 0; i < intPolygons.size(); ++i)
    {
        bool isColl = false;
        std::vector<Point3Dd *> &ipoly = intPolygons[i];
        unsigned sz = ipoly.size();
        sp0 = *ipoly[sz - 1];

        for (unsigned t = 0; t < nbisec; t++)
        {
            int nt = NEXT_INDEX(t, nbisec);
            sp1 = _biSegments[t]->_sv;
            ep1 = _biSegments[t]->_ev;
            if (ep1 == sp0 || ep1 == ep0) {
                continue;
            }
            sp2 = _biSegments[nt]->_sv;
            ep2 = _biSegments[nt]->_ev;
            if (ep2 == sp0 || ep2 == ep0)
                continue;
            Vector3Dd v0 = ep0 - sp0; v0.unit();
            Vector3Dd v1 = ep2 - ep1; v1.unit();
            double dot = v0.dot(v1);
            if (fabsf(dot - 1.0) < EPSILON_VAL_E3)
                continue;
            std::vector<Point3Dd *>quad(4);
            quad[0] = &sp1;
            quad[1] = &ep1;
            quad[2] = &ep2;
            quad[3] = &sp2;
            if (BaseType::Util::is2SegmentCollision(sp1, ep1, sp2, ep2, ind))
            {
                continue;
            }
            ClipPolygon clip(ipoly, quad, _norm);
            std::vector<std::vector<Point3Dd *>> exsubj;
            if (clip.getExtPolygonsOfSubj(exsubj)) {
                for (unsigned ie = 0; ie < exsubj.size(); ie++)
                {
                    continue;
                    polygons.push_back(exsubj[ie]);
                }
            }
        }
    }
#endif
}

/*
** Find the nearest intersection point to nearPoint
*/
int CalcPolygonOffset::nearIntOfSegmentOriginal(const Point3Dd &sp, const Point3Dd &ep,
                                    const Point3Dd &nearPoint, Point3Dd &intp)
{
    double min = DBL_MAX;
    unsigned nump = _planarContour->getNumPoints();
    VBQuaternion quat;
    Vector3Dd oz(0, 0, 1);
    Point3Dd intp1;

    Point3Dd sv = *_planarContour->getLastPoint();
    Point3Dd ev;
    for (unsigned i = 0; i < nump; ++i) {
        ev = *_planarContour->getPointAt(i);
        if (BaseType::Util::calcIntPointOf2PlanarSegment(
            sp, ep, sv, ev, _norm.getIndexOfMaxAbsCoord(), intp1))
        {
            double d = intp1.squareDistanceToPoint(nearPoint);
            if (d < min) {
                intp = intp1;
                min = d;
            }
        }
        sv = ev;
    }

    return min != DBL_MAX;
}