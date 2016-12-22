#include "Segment2Dd.h"
#include "Point2D.h"
#include "triangle3Dd.h"

Segment2Dd::Segment2Dd()
    :_visited(false),
     _contour(false),
     _isInverted(false),
     _isOpenAtStartPoint(false),
     _swapped(false)
{
    reset();
}

Segment2Dd::Segment2Dd(const Point2Dd &sp, const Point2Dd &ep):
    _swapped(false)
{
    reset();

    _sp = sp;
    _ep = ep;
}

Segment2Dd::Segment2Dd( const Segment2Dd &segment )
{
    reset();
    
    _isOpenAtStartPoint = segment._isOpenAtStartPoint;
    _swapped = segment._swapped;
}

Segment2Dd::~Segment2Dd()
{
}

bool Segment2Dd::isConnectedTo(Segment2Dd *segment)
{
    Point2Dd sp = segment->getFirstPoint();
    Point2Dd ep = segment->getSecondPoint();

    if (_ep.isConcident(sp) || _sp.isConcident(sp) || _sp.isConcident(ep)
            || _ep.isConcident(ep)) {
        return true;
    }

    return false;
}

Point2Dd Segment2Dd::getNearestPoint(const Point2Dd &p)
{
    double dis11 = p.squareDistanceToPoint(_sp);
    double dis12 = p.squareDistanceToPoint(_ep);

    if (dis12 < dis11)
        return _ep;
    else
        return _sp;
}

double Segment2Dd::getSquareDistanceToPoint(const Point2Dd &p, bool &isToFisrtPoint)
{
    double dis1 = p.squareDistanceToPoint(_sp);
    double dis2 = p.squareDistanceToPoint(_ep);

    isToFisrtPoint = (dis1 < dis2);
    return isToFisrtPoint ? dis1 : dis2;
}

bool Segment2Dd::intersectSegment( std::vector<Point2Dd> &ip,
                                   Segment2Dd *segment )
{
    // in this case the ray and segment is not parallel
    Point2Dd u = _ep - _sp;
    Point2Dd v, w;
    Point2Dd sp = segment->getFirstPoint();
    Point2Dd ep = segment->getSecondPoint();
    v = ep - sp;
    //if ( true == _sp.isConcident(segment.getFirstPoint()) ) {
    if (_sp.isConcident(sp)) {
        ip.push_back(_sp);
        return true;
    }

    w = _sp - sp;
    double D = u.perp(v);

    if ( fabs(D) < EPSILON_VAL_ )
        return false;

    double sI = v.perp(w) / D;
    double tI = u.perp(w) / D;

    if ( sI < 0 || sI > 1 || tI < 0 || tI > 1 ) {
        return false;
    } else {
        ip.push_back( _sp + u * sI );
        return true;
    }
}

bool Segment2Dd::intersectSegment( std::vector<ISPoint2D> &ips,
                                   Segment2Dd *segment )
{
    Point2Dd ip;
    bool isIntersected = intersectSegment(ip, segment);

    if (isIntersected) {
        ISPoint2D ips_;
        ips_.ip = ip;
        ips_.segment = segment;
        ips.push_back(ips_);
    }

    return isIntersected;
}

bool Segment2Dd::intersectSegment( Point2Dd &ip,
                                   Segment2Dd *segment )
{
    // in this case the ray and segment is not parallel
    Point2Dd u = _ep - _sp;
    Point2Dd v, w;
    Point2Dd sp = segment->getFirstPoint();
    Point2Dd ep = segment->getSecondPoint();
    v = ep - sp;
    //if ( true == _sp.isConcident(segment.getFirstPoint()) ) {
    if (_sp.isConcident(sp)) {
        ip = _sp;
        return true;
    }

    w = _sp - sp;
    double D = u.perp(v);

    if ( fabs(D) < EPSILON_VAL_ )
        //if ( fabs(D) == 0 )
        return false;

    double sI = v.perp(w) / D;
    double tI = u.perp(w) / D;

    if ( sI < 0 || sI > 1 || tI < 0 || tI > 1 ) { /*(1))*/
        //if( sI < -EPSILON_VAL_ || sI > 1 + EPSILON_VAL_ || tI < -EPSILON_VAL_ || tI > 1 + EPSILON_VAL_ ) {
        //if( sI < EPSILON_VAL_ || sI > 1 - EPSILON_VAL_ || tI < EPSILON_VAL_ || tI > 1 - EPSILON_VAL_ ) {/*(2))*/
        return false;
    } else {
        ip = _sp + u * sI;
        return true;
    }

    /*
    Problem: It does not work correctly when use (1), but Hieu am not sure (2) is alway correct
    */
    //}
}

bool Segment2Dd::isIntersectSegment(Segment2Dd *segment)
{
    // in this case the ray and segment is not parallel
    Point2Dd u = _ep - _sp;
    Point2Dd v, w;
    Point2Dd sp = segment->getFirstPoint();
    Point2Dd ep = segment->getSecondPoint();
    v = ep - sp;
    //if ( true == _sp.isConcident(segment.getFirstPoint()) ) {
    if (_sp.isConcident(sp)) {
        return true;
    }

    w = _sp - sp;
    double D = u.perp(v);

    if ( fabs(D) < EPSILON_VAL_ )
        return false;

    double sI = v.perp(w) / D;
    double tI = u.perp(w) / D;

    if ( sI < 0 || sI > 1 || tI < 0 || tI > 1 ) {
        //if( sI < EPSILON_VAL_ || sI > 1 - EPSILON_VAL_ || tI < EPSILON_VAL_ || tI > 1 - EPSILON_VAL_ ) {/*(2))*/
        return false;
    } else {
        return true;
    }
}

bool Segment2Dd::intersectLine(double &t, const Point2Dd &p, const Point2Dd &d)
{
    // in this case the ray and segment is not parallel
    Point2Dd u = _ep - _sp;
    Point2Dd v, w;
    v = d;

    if ( true == _sp.isConcident(p) ) {
        t = 0.0;
        return true;
    }

    w = _sp - p;
    double D = u.perp(v);

    if ( fabs(D) < EPSILON_VAL_ )
        // Pay attention!!!, change to "if ( fabs(D) < 0 )" if you get any problem that relate to tolerance
        return false;

    double t_ = u.perp(w) / D;

    if ( t_ < 0 || t_ > 1 )
        return false;

    t = t_;
    return true;
}

bool Segment2Dd::intersectLine(Point2Dd &ip, const Point2Dd &p, const Point2Dd &d)
{
    double t;

    if (intersectLine(t, p, d)) {
        //ip = _sp +  (_ep - _sp) * t;
        ip = p + d * t;
        return true;
    }

    return false;
}

Point2Dd Segment2Dd::getOtherPoint(const Point2Dd &p)
{
    if (_sp.isConcident(p)) {
        return _ep;
    }

    return _sp;
}

Point2Dd Segment2Dd::getFarPoint(const Point2Dd &p)
{
    double dis1 = p.squareDistanceToPoint(_sp);
    double dis2 = p.squareDistanceToPoint(_ep);

    if (dis1 > dis2) {
        return _sp;
    }

    return _ep;
}

void Segment2Dd::invert()
{
    Point2Dd tmp = _sp;
    _sp = _ep;
    _ep = tmp;
}

bool Segment2Dd::isParallel(Segment2Dd *segment)
{
    Point2Dd a, b;
    getVector(a);
    segment->getVector(b);

    return a.isVectorParallel(b);
}

bool Segment2Dd::isSameLineWith(Segment2Dd *segment)
{
    bool isPara = isParallel(segment);

    if (isPara) {
        Point2Dd vec;
        segment->getVector(vec);
        bool isOnline = _sp.isOnLine(segment->getFirstPoint(), vec);
        return isOnline;
    }

    return false;
}

bool Segment2Dd::isSameDirection(Segment2Dd *segment)
{
    Point2Dd a, b;
    getVector(a);
    segment->getVector(b);

    double dot = a.dot(b);

    return dot > EPSILON_VAL_;
}

bool Segment2Dd::isEqual(Segment2Dd *segment)
{
    return _sp.isConcident(segment->getFirstPoint())
           && _ep.isConcident(segment->getSecondPoint());
}

void Segment2Dd::setSegment(const Point2Dd &sp, const Point2Dd &ep)
{
    _sp = sp;
    _ep = ep;
}

bool Segment2Dd::containsPoint(const Point2Dd &p)
{
    Point2Dd v1 = _sp - p;
    Point2Dd v2 = _ep - p;

    if (v1.dot(v2) < 0) {
        return true;
    }

    return false;
}

/**********************************************************************************************//**
 * Resets this object.
 **************************************************************************************************/

void Segment2Dd::reset(void)
{
    _prevSegment = nullptr;
    _nextSegment = nullptr;
    _contour = nullptr;
    _visited = false;
    _isInverted = false;
    _isOpenAtStartPoint = false;
    _trianlge = nullptr;
    _stlTriangle = nullptr;
    _zValue = 0.0; // convert to 3D if _zValue != 0
}

double const Segment2Dd::length() const
{
    return _sp.distanceToPoint(_ep);
}

double const Segment2Dd::length2() const
{
    return _sp.squareDistanceToPoint(_ep);
}

/**************************************************************************************************
 * Swap end points.
 **************************************************************************************************/

void Segment2Dd::swapEndPoints(void)
{
    std::swap(_sp, _ep);
    _swapped = !_swapped;
}