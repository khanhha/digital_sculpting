#include "triangle3Dd.h"
#include "Point2D.h"
#include "Point3Dd.h"
#include "defined.h"
#include "segment3Dd.h"

//********************************************************//
Triangle3Dd::Triangle3Dd()
{
}

Triangle3Dd::Triangle3Dd(const Point3Dd* vertexs, 
    const Vector3Dd& normal )

{
    _vertexs[0] = vertexs[0];
    _vertexs[1] = vertexs[1];
    _vertexs[2] = vertexs[2];
    _normal = normal;
#ifdef _DEBUG
   _ired = 0;
#endif
}
Triangle3Dd::Triangle3Dd(const Point3Dd* vertexs) {
	_vertexs[0] = vertexs[0];
	_vertexs[1] = vertexs[1];
	_vertexs[2] = vertexs[2];

	Point3Dd v1 = _vertexs[1] - _vertexs[0];
	Point3Dd v2 = _vertexs[2] - _vertexs[1];

	_normal = v1.crossProduct(v2);
#ifdef _DEBUG
   _ired = 0;
#endif
}

Triangle3Dd::Triangle3Dd(const Point3Dd& p1, const Point3Dd& p2, const Point3Dd& p3)
{
   _vertexs[0] = p1;
   _vertexs[1] = p2;
   _vertexs[2] = p3;
   Point3Dd v1 = _vertexs[1] - _vertexs[0];
	Point3Dd v2 = _vertexs[2] - _vertexs[1];
	_normal = v1*v2;

#ifdef _DEBUG
   _ired = 0;
#endif
}

Triangle3Dd::Triangle3Dd(const Point3Dd* p1, const Point3Dd* p2,
   const Point3Dd* p3, const Point3Dd* norm)
{
   _vertexs[0] = *p1;
   _vertexs[1] = *p2;
   _vertexs[2] = *p3;
   _normal = *norm;
}

Triangle3Dd::~Triangle3Dd()
{

}

// return number of intersection point
int Triangle3Dd::intersectAxisPlane( Point2Dd& ip1, Point2Dd& ip2, 
    const double & offsetVal, const int iCoord ) const
{
    double min, max;
    minmax3(min, max, 
        _vertexs[0].v[iCoord],
        _vertexs[1].v[iCoord], 
        _vertexs[2].v[iCoord]);

    if ( (offsetVal < min - EPSILON_VAL_) || (offsetVal > max + EPSILON_VAL_))
        return 0;


    //if ( 0 == doubleCompare(min, max) )
	if ((max - min) < EPSILON_VAL_)
        return 3; //2 plane is parallel or coincidence

	//if (fabs(offsetVal - min) < EPSILON_VAL_ || fabs(max - offsetVal) < EPSILON_VAL_) {
	//	//return 1;// intersect at one vertex
	//	int debug = 1;
	//}
    //
	bool isFirstPoint = false;
    int k = 0;
    bool isVertexOn = false;
    int _nip = 0;
    Point2Dd _ip[2];
    for ( int i = 0; i < 3; i++ ) {
        if ( fabs(_vertexs[i].v[iCoord] - offsetVal) < EPSILON_VAL_ 
			||  fabs(offsetVal - _vertexs[i].v[iCoord]) < EPSILON_VAL_ ) {
            _ip[_nip++] = _vertexs[i].get2d(iCoord);		
			k = i;
            if ( 2 == _nip ) {
                ip1 = _ip[0];
                ip2 = _ip[1];
				if (isFirstPoint && k == 2) {
					Point2Dd tmp = ip1;
					ip1 = ip2;
					ip2 = tmp;
				}
                return 4;// segment is on the plane
            } else if(1 == _nip) {
				isFirstPoint = true;
			}
            
            isVertexOn = true;
            ip1 = _vertexs[i].get2d(iCoord);
        }
    }

    if ( isVertexOn ) {
        //one vertex is on plane -> opposite segment must be cut, if not, whole triangle is on one side of plane
        Point3Dd segp[2];
        int i_ = 0;
        for ( int i = 0; i < 3; i++ ) {
            if ( i != k )
                segp[i_++] = _vertexs[i];
        }
        Segment3Dd opSeg(segp[0], segp[1]);
        Point3Dd ip;
        int nip = opSeg.intersectAxisPlane(ip, offsetVal, iCoord);
        //ASSERT(nip == 1);
        if ( nip ) {
            ip2 = ip.get2d(iCoord);
        }
        else {
            ip2 = ip1;// whole triangle is on one side of plane
        }
        return 2;
    }

    //
    Segment3Dd seg[3] = {
		Segment3Dd(_vertexs[0], _vertexs[1])
        , Segment3Dd(_vertexs[1], _vertexs[2])
        , Segment3Dd(_vertexs[2], _vertexs[0])
    };

    Point3Dd ip3d[3];
    int _i = 0;
    for ( int i = 0; i < 3; i++ ) {
		//// Check if the segmeng is on plane		
		//if ((fabs(seg[i].getFisrtPoint().v[iCoord] - offsetVal) < EPSILON_VAL_
		//	|| fabs(offsetVal - seg[i].getFisrtPoint().v[iCoord]) < EPSILON_VAL_)
		//	&& (fabs(seg[i].getSecondPoint().v[iCoord] - offsetVal) < EPSILON_VAL_
		//	|| fabs(offsetVal - seg[i].getSecondPoint().v[iCoord]) < EPSILON_VAL_)) {
		//		ip1 = seg[i].getFisrtPoint().get2d(iCoord);
		//		ip2 = seg[i].getSecondPoint().get2d(iCoord);
		//}
		//// end check

        if ( 1 == seg[i].intersectAxisPlane(ip3d[_i], offsetVal, iCoord) ) {
            _i++;
        }
    }
    if ( _i > 0 ) {
        if ( 1 == _i ) {
            ip1 = ip2 = ip3d[0].get2d(iCoord);
        }
        else if ( 2 == _i ) {
            ip1 = ip3d[0].get2d(iCoord);
            ip2 = ip3d[1].get2d(iCoord);
        }
        else if ( 3 == _i ) {
            double dis0 = ip3d[1].squareDistanceToPoint(ip3d[2]);
            double dis1 = ip3d[0].squareDistanceToPoint(ip3d[2]);
            double dis2 = ip3d[0].squareDistanceToPoint(ip3d[1]);
            double min = min3(dis0, dis1, dis2);
            if ( min == dis0) {
                ip1 = ip3d[0].get2d(iCoord);
                ip2 = ((ip3d[1] + ip3d[2])*0.5).get2d(iCoord);
            }
            else if ( min == dis1 ) {
                ip1 = ip3d[1].get2d(iCoord);
                ip2 = ((ip3d[0] + ip3d[2])*0.5).get2d(iCoord);
            }
            else { // == dis2
                ip1 = ip3d[2].get2d(iCoord);
                ip2 = ((ip3d[0] + ip3d[1])*0.5).get2d(iCoord);
            }
        }
        return 2;
    }
    return 0;
}

bool Triangle3Dd::isInvertedNormal() {
	Point3Dd v1 = _vertexs[1] - _vertexs[0];
	Point3Dd v2 = _vertexs[2] - _vertexs[1];

	Point3Dd v = v1.crossProduct(v2);
	double dot = v.scalarProduct(_normal);

	return dot < 0;
}

void Triangle3Dd::correctInvertedNormal() {
	Point3Dd tmp = _vertexs[1];
	_vertexs[1] = _vertexs[2];
	_vertexs[2] = tmp;
	_normal = _normal * (-1);

}

void Triangle3Dd::getVertexts(Point3Dd vertexs[3]) {
	_vertexs[0] = vertexs[0];
	_vertexs[1] = vertexs[1];
	_vertexs[2] = vertexs[2];
}

Point3Dd Triangle3Dd::getVertext(const int i) {
	return _vertexs[i];
}
Point3Dd* Triangle3Dd::getpVertext(const int i) {
	return &_vertexs[i];
}
Vector3Dd Triangle3Dd::getNormal() {
	return _normal;
}
Vector3Dd* Triangle3Dd::getpNormal() {
	return &_normal;
}



void Triangle3Dd::getZMinMax(double& min, double& max) {
	min = min3(_vertexs[0].v[2], _vertexs[1].v[2], _vertexs[2].v[2]);
	max = max3(_vertexs[0].v[2], _vertexs[1].v[2], _vertexs[2].v[2]);

}
int Triangle3Dd::calcNormal()
{
   Point3Dd dv1(_vertexs[1]);
	dv1 -= _vertexs[0];
	Point3Dd dv2(_vertexs[2]);
	dv2 -= _vertexs[0];
   
	_normal = dv1*dv2;
   _normal.unit();;
   return 0;
}
void Triangle3Dd::offset(const double& d)
{
   _vertexs[0] += _normal*d;
   _vertexs[1] += _normal*d;
   _vertexs[2] += _normal*d;
}

void Triangle3Dd::invertNormal()
{
   _normal.v[0] = -_normal.v[0];
   _normal.v[1] = -_normal.v[1];
   _normal.v[2] = -_normal.v[2];
}

void Triangle3Dd::set(const Point3Dd& p1, const Point3Dd& p2, const Point3Dd& p3)
{
	_vertexs[0] = p1;
	_vertexs[1] = p2;
	_vertexs[2] = p3;
	Point3Dd v1 = _vertexs[1] - _vertexs[0];
	Point3Dd v2 = _vertexs[2] - _vertexs[1];
	_normal = v1*v2;
}