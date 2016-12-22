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
#include "PlanarContour.h"
#include "matrix3x3d.h"
#include "VBQuaternion.h"
#include "contour2d.h"
#include "point2D.h"
#include "util.h"
#if 0
#ifdef _DEBUG
#include "Diagnotics\AutoCadAutomator.h"
#endif
#endif

PlanarContour::PlanarContour()
{
    _minCoord = Point3Dd(DBL_MAX, DBL_MAX, DBL_MAX);
    _maxCoord = Point3Dd(-DBL_MAX, -DBL_MAX, -DBL_MAX);
}

PlanarContour::PlanarContour(const PlaConPointPtrList &polyline)
{
    _points = polyline;
    checkDirection();
    calcMinMaxCoord();
}

PlanarContour::PlanarContour(const std::vector<Point3Dd *> &polyline)
{
    _points.resize(polyline.size());
    for (unsigned i = 0; i < polyline.size(); ++i)
    {
        _points[i] = new Point3Dd(polyline[i]);
    }

    checkDirection();
    calcMinMaxCoord();
}

PlanarContour::PlanarContour(Point3Dd* p3ds, unsigned numpoints)
{
    _points.resize(numpoints);
    for (unsigned i = 0; i < numpoints; ++i)
    {
        _points[i] = new Point3Dd(p3ds[i]);
    }
    checkDirection();
    calcMinMaxCoord();
}

PlanarContour::~PlanarContour()
{
    unsigned sz = _points.size();
    for (unsigned i = 0; i < sz; i++)
    {
        if (_points[i])
        {
            delete _points[i];
            _points[i] = NULL;
        }
    }
    _points.clear();
    _points.shrink_to_fit();
}

bool PlanarContour::checkDirection()
{
    unsigned sz = _points.size();
    if (_normUp == ZERO_VECTOR_3Dd || sz < 3) {
        //assert(0);
        return _isCounterClockwise;
    }

    _isCounterClockwise = true;

    Vector3Dd v0, v1;
    Point3Dd p0;
    double sum = 0;
    p0 = *_points[0];
    v0 = *_points[1] - p0;
    _normUp.unit();
    for (unsigned i = 2; i < sz; i++)
    {
        v1 = *_points[i] - p0;
        sum += ((v0 * v1).dot(_normUp));
        v0 = v1;
    }
    
    _isCounterClockwise = sum > 0.0;

    return _isCounterClockwise;
}

int PlanarContour::findReflexVertices(std::vector<Point3Dd *> &reflexVs)
{
    unsigned sz = _points.size();
    Vector3Dd pr = *_points[sz - 1];
    Vector3Dd nx;
    for (unsigned i = 0; i < sz; i++)
    {
        pr = *_points[i] - pr; pr.unit();
        nx = *_points[(i + 1) % sz] - *_points[i]; nx.unit();
        double dot = pr.dot(nx);
        Vector3Dd cv = pr.cross(nx); //cv.unit();

        double d = cv.dot(_normUp);
        if (d < -EPSILON_VAL_ && _isCounterClockwise ||
            d > EPSILON_VAL_ && !_isCounterClockwise) {
            reflexVs.push_back(_points[i]);
        }
        pr = *_points[i];
    }

    return reflexVs.size();
}

int PlanarContour::findReflexVertices(std::vector<int> &reflexVind)
{
    unsigned sz = _points.size();
    Vector3Dd pr = *_points[sz - 1];
    Vector3Dd nx;
    for (unsigned i = 0; i < sz; i++)
    {
        pr = *_points[i] - pr; pr.unit();
        nx = *_points[(i + 1) % sz] - *_points[i]; nx.unit();
        double dot = pr.dot(nx);
        Vector3Dd cv = pr.cross(nx); //cv.unit();

        double d = cv.dot(_normUp);
        if (d < -EPSILON_VAL_ && _isCounterClockwise ||
            d > EPSILON_VAL_ && !_isCounterClockwise) {
            reflexVind.push_back(i);
        }
        pr = *_points[i];
    }

    return reflexVind.size();
}

bool PlanarContour::isPointInsideConvexContour(const Point3Dd &p)
{
    unsigned sz = _points.size();

    Vector3Dd v0, v1;
    Point3Dd p0 = *_points[0];
    for (unsigned i = 1; i < sz; i++)
    {
        v0 = *_points[i] - p0;
        v1 = p - p0;
        if (p0.isStrongEqual(p) || (*_points[i]).isStrongEqual(p))
            return true;

        double d = (v0 * v1).dot(_normUp);
        if (_isCounterClockwise) {
            if (d < -EPSILON_VAL_)
                return false;
        } else {
            if (d > EPSILON_VAL_)
                return false;
        }

        p0 = *_points[i];
    }

    return true;
}

bool PlanarContour::isPointInside(const Point3Dd &p)
{
    Vector3Dd normOf2D(0.0, 0.0, 1.0);
    double angle = _normUp.angle(normOf2D);
    std::vector<Point2Dd> cnt2D;
    Point3Dd ip;
    unsigned sz = _points.size();
    Point3Dd rp;

    if (FLOAT_EQUAL(angle, 0.0)) {
        for (unsigned i = 0; i < sz; i++)
        {
            ip = *_points[i];
            cnt2D.push_back(Point2D<double>(ip.x, ip.y));
        }
        rp = p;
    } else {
        Vector3Dd axis = _normUp.cross(normOf2D);
        VBQuaternion quat;
        quat.quatFromAngleAxis(angle, axis.v);

        rp = quat.rotateVector(p);
        for (unsigned i = 0; i < sz; i++)
        {
             ip = quat.rotateVector(*_points[i]);
             cnt2D.push_back(Point2D<double>(ip.x, ip.y));
        }
    }
#ifdef _DEBUG1
    this->dumpCad();
#endif
    return BaseType::Util::isPointInsidePolygon2D(cnt2D, Point2Dd(rp.x, rp.y));
}

void PlanarContour::calcMinMaxCoord()
{
    double mincoor[3];
    double maxcoor[3];
    std::fill(mincoor, mincoor + 3, DBL_MAX);
    std::fill(maxcoor, maxcoor + 3, -DBL_MAX);
    unsigned sz = _points.size();
    for (unsigned i = 0; i < sz; i++)
    {
        Point3Dd p = *_points[i];
        if (mincoor[0] > p.x) {
            mincoor[0] = p.x;
        } else if (maxcoor[0] < p.x) {
            maxcoor[0] = p.x;
        }

        if (mincoor[1] > p.y) {
            mincoor[1] = p.y;
        } else if (maxcoor[1] < p.y) {
            maxcoor[1] = p.y;
        }

        if (mincoor[2] > p.z) {
            mincoor[2] = p.z;
        } else if (maxcoor[2] < p.z) {
            maxcoor[2] = p.z;
        }
    }

    _minCoord = Point3Dd(mincoor);
    _maxCoord = Point3Dd(maxcoor);
}

bool PlanarContour::isANotIntersectContourInside(const PlanarContour &cnt)
{
    Point3Dd mincoor, maxcoor;
    cnt.getMinMaxCoord(mincoor, maxcoor);
    return this->isPointInside(*cnt.getFirstPoint()) &&
           BaseType::Util::isBoxInsideOtherBox(mincoor.v, maxcoor.v, _minCoord.v, _maxCoord.v);
}

int PlanarContour::findAPoint(const Point3Dd &p)
{
    std::deque<Point3Dd *>::iterator it;
    it = std::find_if(_points.begin(), _points.end(), 
                     [&] (Point3Dd *cp) { return cp->isStrongEqual(p);});
    if (it != _points.end())
    {
        return (it - _points.begin());
    }

    return -1;
}

void PlanarContour::rotate(const double angle, const Vector3Dd &axis, const Point3Dd &center)
{
    VBQuaternion quat(angle, axis.v);
    rotate(quat, center);
}

void PlanarContour::rotate(VBQuaternion &quat, const Point3Dd &center)
{
    unsigned sz = _points.size();

    for (unsigned i = 0; i < sz; i++)
    {
        *_points[i] = quat.rotateVector(*_points[i], center);
    }

    _normUp = quat.rotateVector(_normUp);
    calcMinMaxCoord();
}

void PlanarContour::transformToPlane(const Point3Dd &pv, const Vector3Dd &nv)
{
    assert (nv != ZERO_VECTOR_3Dd);
    double angle = _normUp.angle(nv);
    unsigned sz = _points.size();
    Point3Dd center = *_points[0];
    Vector3Dd mov;
    BaseType::Util::calcIntLinePlane(center, nv, pv, nv, mov);
    mov -= center;
    bool isTranslate = mov != ZERO_VECTOR_3Dd;
    bool isRotate = false == FLOAT_EQUAL(angle, 0.0);

    if (!isRotate && !isTranslate) {
        return;
    }
    
    if (isRotate) {
        rotate(-angle, _normUp * nv, center);
    }

    if (isTranslate) {
        for (unsigned i = 0; i < sz; i++)
        {
                *_points[i] += mov;
        }
    }

    if (isTranslate) {
        _maxCoord += mov;
        _minCoord += mov;
    }
}

void PlanarContour::moveAlongNormal(double len)
{
    _normUp.unit();
    Point3Dd moveVector = len * _normUp;
    assert(_normUp != ZERO_VECTOR_3Dd);
    std::transform(_points.begin(), _points.end(),_points.begin(), 
                  [&] (Point3Dd *p3d) -> Point3Dd *{ *(p3d) += moveVector; return &(*p3d);});
}

void PlanarContour::close() {
    if (_points.front()->isStrongEqual(*_points.back())) {
        return;
    }

    _points.push_back(new Point3Dd(*_points.front()));
}

void PlanarContour::getMinMaxLenSegment(double &minlen, double &maxlen) const
{
    unsigned sz = _points.size();
    minlen = DBL_MAX;
    maxlen = -DBL_MAX;
    double d = 0.0;

    for (unsigned i = 0; i < sz - 1; i++)
    {
        d = _points[i]->distance(*_points[i + 1]);
        if (FLOAT_EQUAL(d, 0.0)) continue;
        
        if (d < minlen) {
            minlen = d;
        } else if (d > maxlen) {
            maxlen = d;
        }
    }
}

void PlanarContour::reverse()
{
    std::reverse(_points.begin(), _points.end());
}

bool PlanarContour::samplingContour(double d)
{
    unsigned sz = _points.size();
    Point3Dd p0, p1;
    for (unsigned i = 0; i < sz - 1; i++)
    {
        p0 = *_points[i];
        p1 = *_points[i + 1];
        double dis = p0.distance(p1);
        unsigned r = (unsigned)(dis/d);
        if (r > 0) {
            Vector3Dd v = p1 - p0;
            v.unit();
            Point3Dd *nwp;
            PlaConPointPtrList nwPnts;
            for (unsigned k = 0; k < r; k++)
            {
                nwp = new Point3Dd(p0 + v * (d * (k + 1)));
                nwPnts.push_back(nwp);
            }
            _points.insert(_points.begin() + i + 1, nwPnts.begin(), nwPnts.end());
            i += r;
            sz += r;
        }
    }

    return true;
}

void PlanarContour::filterNonCorner()
{
    unsigned sz = _points.size();
    Vector3Dd pr = *_points[sz - 1];
    Vector3Dd nx;
    for (unsigned i = 0; i < sz; i++)
    {
        pr = *_points[i] - pr; pr.unit();
        nx = *_points[(i + 1) % sz] - *_points[i]; nx.unit();
        double dot = pr.dot(nx);
        if (fabs(dot - 1.0) < EPSILON_VAL_E4) {
            _points.erase(_points.begin() + i);
            sz = _points.size();
            i--;
        }
        else {
            pr = *_points[i];
        }
        
    }
}

bool PlanarContour::isValidPolygon(std::vector<Point3Dd*> &poly, const Vector3Dd &norm)
{
    if (poly.size() < 3) return false;

    unsigned sz = poly.size();
    if (norm == ZERO_VECTOR_3Dd || sz < 3) {
        return true;
    }

    bool ret = true;

    Vector3Dd v0, v1;
    Point3Dd p0;
    double sum = 0;
    p0 = *poly[0];
    v0 = *poly[1] - p0;
    for (unsigned i = 2; i < sz; i++)
    {
        v1 = *poly[i] - p0;
        sum += ((v0 * v1).dot(norm));
        v0 = v1;
    }

    bool isValidArea = !FLOAT_EQUAL(sum, 0.0);
    return isValidArea;
}

bool PlanarContour::isCCWPolygon(std::vector<Point3Dd *> &polygon, const Vector3Dd &norm)
{
    unsigned sz = polygon.size();
    if (norm == ZERO_VECTOR_3Dd || sz < 3) {
        return true;
    }

    bool ret = true;

    Vector3Dd v0, v1;
    Point3Dd p0;
    double sum = 0;
    p0 = *polygon[0];
    v0 = *polygon[1] - p0;
    for (unsigned i = 2; i < sz; i++)
    {
        v1 = *polygon[i] - p0;
        sum += ((v0 * v1).dot(norm));
        v0 = v1;
    }

    ret = sum > 0.0;

    return ret;
}

#ifdef _DEBUG
void PlanarContour::dumpCad()
{
#if 0
	Acad::start();
    std::vector<Point3Dd *> poly;
    poly.assign(_points.begin(), _points.end());
    Acad::draw3dPoly(poly);
#else
    std::vector<Point2Df *> poly;
    for (unsigned i = 0; i < _points.size(); i++)
    {
		poly.push_back(new Point2Df((float)_points[i]->x, (float)_points[i]->y));
    }
	poly.push_back(new Point2Df((float)_points[0]->x, (float)_points[0]->y));
#if 0
	Acad::draw2dPolyline(poly);
#endif
#endif
}
#endif