//#include "StdAfx.h"
#include "Point3Dd.h"
#include "defined.h"
#include <math.h>
#include "matrix3x3d.h"
#include "Point2D.h"


double Point3Dd::unit()
{
	double dd = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	if(dd < 1e-15){
		v[0] = 0.0;
		v[1] = 0.0;
		v[2] = 0.0;
		return 0.0;
	} else if (fabs(dd - 1.0) < EPSILON_VAL_MICRO) {
		return 1.0;
	}
	double d = sqrt(dd);
	v[0] /= d;
	v[1] /= d;
	v[2] /= d;
	return d;
}

Point3Dd & Point3Dd::normalize()
{
	double dd = squareModule();
	if(dd < 1e-15){
		v[0] = 0.0;
		v[1] = 0.0;
		v[2] = 0.0;
		return *this;
	} 
	
	double d = sqrt(dd);

	v[0] /= d;
	v[1] /= d;
	v[2] /= d;

	return *this;
}

Point3Dd & normalize(Point3Dd & p)
{
	double dd = p.squareModule();
	if(dd < 1e-15){
		p.v[0] = 0.0;
		p.v[1] = 0.0;
		p.v[2] = 0.0;
		return p;
	} 

	double d = sqrt(dd);

	p.v[0] /= d;
	p.v[1] /= d;
	p.v[2] /= d;

	return p;
}

double Point3Dd::abs() const
{
	double dd = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	double d = sqrt(dd);
	return d;
}

double Point3Dd::abs2() const
{
	double dd = v[0] * v[0] + v[1] * v[1] + v[2] * v[2];
	return dd;
}

Point3Dd Point3Dd::crossProduct(const Point3Dd& v1) const
{
	Point3Dd temp;
	temp.v[0] = v[1] * v1.v[2] - v[2] * v1.v[1];
	temp.v[1] = v[2] * v1.v[0] - v[0] * v1.v[2];
	temp.v[2] = v[0] * v1.v[1] - v[1] * v1.v[0];
	return temp;
}

int Point3Dd::crossProduct(const Point3Dd* v0, const Point3Dd* v1)
{
	v[0] = v0->v[1] * v1->v[2] - v0->v[2] * v1->v[1];
	v[1] = v0->v[2] * v1->v[0] - v0->v[0] * v1->v[2];
	v[2] = v0->v[0] * v1->v[1] - v0->v[1] * v1->v[0];
	return 0;
}

double Point3Dd::scalarProduct(const Point3Dd &v1) const
{
	double d = v[0] * v1.v[0] + v[1] * v1.v[1] + v[2] * v1.v[2];
	return d;
}

bool Point3Dd::isEqualTo(const Point3Dd &v1) const
{
	bool result = false;
	double dx = fabs(this->v[0] - v1.v[0]);
	double dy = fabs(this->v[1] - v1.v[1]);
	double dz = fabs(this->v[2] - v1.v[2]);
	if(dx <= EPSILON_VAL_ && dy <= EPSILON_VAL_ && dz <= EPSILON_VAL_){
		//return true;
		double dist = (dx * dx + dy * dy + dz * dz);
		if(dist <= EPSILON_VAL_*EPSILON_VAL_){
			result = true;
		}
	}
	return result;
}

bool Point3Dd::isStrongEqual(const Point3Dd &v1) const
{
	bool result = false;
	double dx = fabs(this->v[0] - v1.v[0]);
	double dy = fabs(this->v[1] - v1.v[1]);
	double dz = fabs(this->v[2] - v1.v[2]);
	if(dx <= EPSILON_MAKE_COIN_POINT && dy <= EPSILON_MAKE_COIN_POINT && dz <= EPSILON_MAKE_COIN_POINT){
		// return true;
		double dist = (dx * dx + dy * dy + dz * dz);
		if(dist <= SQUARE_EPSILON_MAKE_COIN_POINT){
			result = true;
		}
	}
	return result;
}

bool Point3Dd::isStrongEqualEps(const Point3Dd &v1, const double epsilon) const
{
    bool result = false;
    const double squareEps = epsilon * epsilon;
    double dx = fabs(this->v[0] - v1.v[0]);
    double dy = fabs(this->v[1] - v1.v[1]);
    double dz = fabs(this->v[2] - v1.v[2]);
    if(dx <= epsilon && dy <= epsilon && dz <= epsilon){
        double dist = (dx * dx + dy * dy + dz * dz);
        if(dist <= squareEps){
            result = true;
        }
    }
    return result;
}

bool Point3Dd::isEqualForNearCheck(const Point3Dd &v1) const
{
	bool result = false;
	double dx = fabs(this->v[0] - v1.v[0]);
	double dy = fabs(this->v[1] - v1.v[1]);
	double dz = fabs(this->v[2] - v1.v[2]);
	if(dx <= EPSILON_MOVE_VP && dy <= EPSILON_MOVE_VP && dz <= EPSILON_VAL_E4){
		// return true;
		double dist = (dx * dx + dy * dy + dz * dz);
		if(dist <= EPSILON_MOVE_VP*EPSILON_MOVE_VP){
			result = true;
		}
	}
	return result;
}

Point3Dd& Point3Dd::operator = (const Point3Dd& other) {
	if (this != (&other)) {
		v[0] = other.v[0], v[1] = other.v[1], v[2] = other.v[2];
	}
	return *this;
}

Point3Dd& Point3Dd::operator = (const double other[3])
{
    
    v[0] = other[0], v[1] = other[1], v[2] = other[2];
    
    return *this;
}

bool Point3Dd::operator == (const Point3Dd& v1) const
{
	if( (fabs(v[0] - v1.v[0]) <= EPSILON_VAL_) && (fabs(v[1] - v1.v[1]) <= EPSILON_VAL_) && (fabs(v[2] - v1.v[2]) <= EPSILON_VAL_) ) {
		return true;
	} else {
		double l = sqrt(((v[0] - v1.v[0])*(v[0] - v1.v[0])) + ((v[1] - v1.v[1])*(v[1] - v1.v[1])) + ((v[2] - v1.v[2])*(v[2] - v1.v[2])));
		if( l <= EPSILON_VAL_ )
			return true;
	}
	return false;
}

bool Point3Dd::operator != (const Point3Dd& v1) const
{
	if( (*this) == v1)
		return false;
	else
		return true;
}

Point3Dd Point3Dd::operator +(const Point3Dd& v1) const {
	Point3Dd temp;
	temp.v[0] = v[0] + v1.v[0];
	temp.v[1] = v[1] + v1.v[1];
	temp.v[2] = v[2] + v1.v[2];
	return temp;
}

const Point3Dd& Point3Dd::operator +=(const Point3Dd& v1) {
	v[0] += v1.v[0];
	v[1] += v1.v[1];
	v[2] += v1.v[2];
	return *this;
}

Point3Dd Point3Dd::operator +(double d) const {
	Point3Dd temp;
	temp.v[0] = v[0] + d;
	temp.v[1] = v[1] + d;
	temp.v[2] = v[2] + d;
	return temp;
}

const Point3Dd& Point3Dd::operator +=(double d) {
	v[0] += d;
	v[1] += d;
	v[2] += d;
	return *this;
}

Point3Dd Point3Dd::operator -() const {
	Point3Dd temp;
	temp.v[0] = -v[0];
	temp.v[1] = -v[1] ;
	temp.v[2] = -v[2] ;
	return temp;
}

Point3Dd Point3Dd::operator -(const Point3Dd& v1) const {
	Point3Dd temp;
	temp.v[0] = v[0] - v1.v[0];
	temp.v[1] = v[1] - v1.v[1];
	temp.v[2] = v[2] - v1.v[2];
	return temp;
}

const Point3Dd& Point3Dd::operator -=(const Point3Dd& v1){
	v[0] -= v1.v[0];
	v[1] -= v1.v[1];
	v[2] -= v1.v[2];
	return *this;
}

Point3Dd Point3Dd::operator -(double d) const {
	Point3Dd temp;
	temp.v[0] = v[0] - d;
	temp.v[1] = v[1] - d;
	temp.v[2] = v[2] - d;
	return temp;
}

const Point3Dd& Point3Dd::operator -=(double d){
	v[0] -= d;
	v[1] -= d;
	v[2] -= d;
	return *this;
}

// Point3Dd Point3Dd::operator *(int i) const {
//   Point3Dd temp;
//   temp.v[0] = v[0] * i;
//   temp.v[1] = v[1] * i;
//   temp.v[2] = v[2] * i;
//   return temp;
//}
//

Point3Dd Point3Dd::operator *(double d) const {
	Point3Dd temp;
	temp.v[0] = v[0] * d;
	temp.v[1] = v[1] * d;
	temp.v[2] = v[2] * d;
	return temp;
}

Point3Dd operator *(double d, Point3Dd const & p)
{
	Point3Dd temp;
	temp[0] = p[0] * d;
	temp[1] = p[1] * d;
	temp[2] = p[2] * d;
	return temp;
}

const Point3Dd& Point3Dd::operator *=(double d) {
	v[0] *= d;
	v[1] *= d;
	v[2] *= d;
	return *this;
}

Point3Dd Point3Dd::operator *(const Point3Dd& v1) const {
	Point3Dd temp;
	temp.v[0] = v[1] * v1.v[2] - v[2] * v1.v[1];
	temp.v[1] = v[2] * v1.v[0] - v[0] * v1.v[2];
	temp.v[2] = v[0] * v1.v[1] - v[1] * v1.v[0];
	return temp;
}

/* Point3Dd Point3Dd::operator /(int i) const {
Point3Dd temp;
temp.v[0] = v[0] / i;
temp.v[1] = v[1] / i;
temp.v[2] = v[2] / i;
return temp;
}
*/

Point3Dd Point3Dd::operator /(double d) const {
	Point3Dd temp;
	temp.v[0] = v[0] / d;
	temp.v[1] = v[1] / d;
	temp.v[2] = v[2] / d;
	return temp;
}

const Point3Dd& Point3Dd::operator /=(double d){
	v[0] /= d;
	v[1] /= d;
	v[2] /= d;
	return *this;
}

/*double& Point3Dd::operator [](int i)
{
return v[i];
}

double Point3Dd::operator [](int i) const
{
return v[i];
}
*/

void Point3Dd::add(const Point3Dd& v1) {
	v[0] = v[0] + v1.v[0];
	v[1] = v[1] + v1.v[1];
	v[2] = v[2] + v1.v[2];
}

void Point3Dd::sub(const Point3Dd& v1) {
	v[0] = v[0] - v1.v[0];
	v[1] = v[1] - v1.v[1];
	v[2] = v[2] - v1.v[2];
}

double Point3Dd::angle(const Point3Dd& v1) const
{
	double denominator = (v1.abs()) * (this->abs());
	if (denominator > 1.0e-9) {
		double _cos = (v[0]*v1.v[0] + v[1]*v1.v[1] + v[2]*v1.v[2]) / denominator;
		if(_cos > 1.0){
			return 0.0;
		}
		else if(_cos < -1.0){
			return -PI_VAL_;
		}
		return acos(_cos);
	}
	return 0.0;
};
double Point3Dd::cosValue(const Point3Dd& v1) const
{
    double denominator = (v1.abs()) * (this->abs());
    if (denominator > 1.0e-9) {
        double _cos = (v[0] * v1.v[0] + v[1] * v1.v[1] + v[2] * v1.v[2]) / denominator;
        if (_cos > 1.0){
            return 1;
        }
        else if (_cos < -1.0){
            return -1;
        }
        return _cos;
    }
    return 1;
}
const int Point3Dd::getIndexOfMaxAbsCoord() const
{
	if (fabs(v[0]) >= fabs(v[1])){
		if (fabs(v[0]) >= fabs(v[2])){
			return 0;
		}
		else{
			return 2;
		}
	}
	else{
		if (fabs(v[1]) >= fabs(v[2])){
			return 1;
		}
		else{
			return 2;
		}
	}
}

const int Point3Dd::getIndexOfMinAbsCoord() const
{
	if (fabs(v[0]) <= fabs(v[1])){
		if (fabs(v[0]) <= fabs(v[2])){
			return 0;
		}
		else{
			return 2;
		}
	}
	else{
		if (fabs(v[1]) <= fabs(v[2])){
			return 1;
		}
		else{
			return 2;
		}
	}
}

/*!
Calculate distance to a point.
@param $vp the point need to calculate distance
@return the distance
@author Nghi
*/
const double Point3Dd::distance(const Point3Dd &vp) const
{
	double res;
	Point3Dd vec(vp);
	vec -= *this;
	res = (vec.v[0]*vec.v[0] + vec.v[1]*vec.v[1] + vec.v[2]*vec.v[2]);
	return sqrt(res);
}

/*!
Calculate distance to a point.
@param $vp the point need to calculate distance
@return the distance
@author Nghi
*/
const double Point3Dd::xyDistance(const Point3Dd &vp) const
{
	double res = (v[0] - vp.v[0])*(v[0] - vp.v[0]) + (v[1] - vp.v[1])*(v[1] - vp.v[1]);
	return sqrt(res);
}

/*!
Calculate square distance to a point.
@param $vp the point need to calculate distance
@return the square distance
@author Nghi
*/
const double Point3Dd::distance2(const Point3Dd &vp) const
{
	double res;
	Point3Dd vec(vp);
	vec -= *this;
	res = (vec.v[0]*vec.v[0] + vec.v[1]*vec.v[1] + vec.v[2]*vec.v[2]);
	return res;
}

/*!
rotate a point around a line.
@param av - direction vector of line
@param theta - rotation angle (in radian)
*/
void Point3Dd::rotate(const Point3Dd& av, const double& theta)
{
	double costh, sinth, v0_av, dd;
	Point3Dd nv;
	Point3Dd temp_v;
	temp_v.v[0] = v[0];
	temp_v.v[1] = v[1];
	temp_v.v[2] = v[2];
	costh = cos(theta);
	sinth = sin(theta);
	v0_av = temp_v.scalarProduct(av);
	nv = av * temp_v;
	dd = v0_av * ( 1 - costh );
	*this = temp_v * costh + av * dd + nv * sinth;
}

/*!
rotate a vector around an other vector.
@param rotVec - axis vector
@param angle - rotation angle (in radian)
*/
int Point3Dd::rotAroundAVector(const Point3Dd & rotVec, const double & angle)
{
	double c_ = cos(angle);
	double s_ = sin(angle);
	double u0 = rotVec.v[0];
	double v0 = rotVec.v[1];
	double w0 = rotVec.v[2];
	double u2 = u0*u0;
	double v2 = v0*v0;
	double w2 = w0*w0;
	double m[3][3];
	m[0][0] = u2 + (v2 + w2)*c_;
	m[0][1] = u0*v0*(1 - c_) - w0*s_;
	m[0][2] = u0*w0*(1 - c_) + v0*s_;
	m[1][0] = u0*v0*(1 - c_) + w0*s_;
	m[1][1] = v2 + (u2 + w2)*c_;
	m[1][2] = v0*w0*(1 - c_) - u0*s_;
	m[2][0] = u0*w0*(1 - c_) - v0*s_;
	m[2][1] = v0*w0*(1 - c_) + u0*s_;
	m[2][2] = w2 + (u2 + v2)*c_;
	double newv[3];
	for(int i = 0; i < 3; i++){
		newv[i] = m[i][0]*v[0] + m[i][1]*v[1] + m[i][2]*v[2];
	}
	v[0] = newv[0];
	v[1] = newv[1];
	v[2] = newv[2];
	return 0;
}

/*!
rotate a point around ox axis.
@param angle - rotation angle (in radian)
*/
void Point3Dd::rotAroundX(const double &angle)
{
    double c = cos(angle);
    double s = sin(angle);
    double ynew = c*y - s*z;
    double znew = s*y + c*z;
    y = ynew;
    z = znew;
}

/*!
rotate a point around ox axis.
@param c - cos of rotated angle
@param s - sine of rotated angle
*/
void Point3Dd::rotAroundX(const double &c, const double &s)
{
    double ynew = c*y - s*z;
    double znew = s*y + c*z;
    y = ynew;
    z = znew;
}

void Point3Dd::rotAroundY(const double &angle)
{
    double c = cos(angle);
    double s = sin(angle);
    double xnew = c*x + s*z;
    double znew = -s*x + c*z;
    x = xnew;
    z = znew;
}
void Point3Dd::rotAroundY(const double &c, const double &s)
{

    double xnew = c*x + s*z;
    double znew = -s*x + c*z;
    x = xnew;
    z = znew;
}

void Point3Dd::rotAroundZ(const double &angle)
{
    double c = cos(angle);
    double s = sin(angle);
    double xnew = c*x - s*y;
    double ynew = s*x + c*y;
    x = xnew;
    y = ynew;
}

void Point3Dd::rotAroundZ(const double &c, const double &s)
{
    double xnew = c*x - s*y;
    double ynew = s*x + c*y;
    x = xnew;
    y = ynew;
}

void Point3Dd::translateXY(const double &dx, const double &dy)
{
    x += dx;
    y += dy;
}

/*
return: 1 out
0 on wall
-1 in
*/
int Point3Dd::checkPositionCapsule(const double & r, const double & leng)
{
	if((v[2] < -r) || (v[2] > (leng + r))){
		return 1;
	}
	double d2 = v[0]*v[0] + v[1]*v[1];
	double r2 = r*r;
	if ((d2 - r2) > EPSILON_VAL_){
		return 1;
	}
	else if ((d2 - r2) < -EPSILON_VAL_){
		if(v[2] > 0 && v[2] < leng){
			return -1;
		}
		double del = sqrt(r2 - d2);
		if (v[2] < (-del - EPSILON_VAL_)){
			return 1;
		}
		else if (v[2] < (-del + EPSILON_VAL_)){
			return 0;
		}
		else{
			if (v[2] > (del + leng + EPSILON_VAL_)){
				return 1;
			}
			else if (v[2] > (del + leng - EPSILON_VAL_)){
				return 0;
			}
			else
			{
				return -1;
			}
		}
	}
	else{
		if (v[2] > (leng + EPSILON_VAL_) || v[2] < -EPSILON_VAL_){
			return 1;
		}
		else{
			return 0;
		}
	}
	return 1;
}

/* calculate orthogonal projection of point to plane
*  param:
*        pv - a point of plane
*		 nv - normal vector of plane
*		 orth - orthogonal projection of point (output)
* outhor: Nghi
*/
int Point3Dd::orthoToPlane(const Point3Dd &pv, const Point3Dd &nv,
	Point3Dd &orth) const
{
	Point3Dd v = *this;
	v -= pv;
	double dis2 = v.scalarProduct(v);
	if (dis2 < EPSILON_VAL_MINI)
	{
		orth = pv;
		return 0;
	}

	if (fabs(v.scalarProduct(nv)) < EPSILON_VAL_)
	{
		orth = *this;
		return 0;
	}

	double num = v.scalarProduct(nv);
	double den = nv.scalarProduct(nv);
	//num = pv.scalarProduct(nv)-num;  ko can boi ben tren da co v-=pv
	double t = -num/den;
    //assert(fabs(den) > EPSILON_VAL_);
	orth = *this + nv*t;

	return 0;
}
int Point3Dd::orthoVectorToPlane(const Point3Dd &pv, const Point3Dd &nv)
{
	if (fabs(this->scalarProduct(nv)) < EPSILON_VAL_)
	{
		return 0;
	}

	double num = this->scalarProduct(nv);
	double den = nv.scalarProduct(nv);
	double t = -num/den;
	*this += nv*t;

	return 0;
}

/*! Calculate square minimum distance from a point to a segment.
@param sp - start point of segment
@param ep - end point of segment
@return square minimum distance
@author Nghi
*/
const double Point3Dd::calcSquareMinDistanceToSeg(const Point3Dd &sp, const Point3Dd &ep) const
{
	double ret=0.0;
	Point3Dd v(ep);
	v -= sp;
	Point3Dd s(this);
	s -= sp;
	double num = v.scalarProduct(s);
	if (num < 0){
		ret = distance2(sp);
	}
	else{
		double den = v.scalarProduct(v);
		if ( num > den){
			ret = distance2(ep);
		}
		else{
			ret = (den*(s.scalarProduct(s)) - num*num)/den;
		}
	}
	return ret;
}
/*! Orthorgonal projection vector to a direction.
@param dir - direction vector
@author Nghi
*/
void Point3Dd::orthorToVector(const Point3Dd& dir)
{
	double num = v[0]*dir.v[0] + v[1]*dir.v[1] + v[2]*dir.v[2];
	double den = dir.v[0]*dir.v[0] + dir.v[1]*dir.v[1] + dir.v[2]*dir.v[2];
//    assert(fabs(den) > EPSILON_VAL_);
	double t = num/den;
	v[0] = t*dir.v[0];
	v[1] = t*dir.v[1];
	v[2] = t*dir.v[2];
}

/* Calculate projection of point onto a line.
@param pv - a point on line
@param dir - direction vector of line
@param proj - contain projection point
*/
void Point3Dd::calcProjectionOntoLine(const Point3Dd &pv,
	const Point3Dd &dir, Point3Dd &proj)
{
	proj = *this;
	proj -= pv;
	proj.orthorToVector(dir);
	proj += pv;
}
/*! Calculate square minimum distance from a point to a line.
*   @param pv - a point on line
*	 @param nv - direction vector of line
*	 @return square minimum distance
* 	 @author Nghi
*/
const double Point3Dd::calcSquareDistanceToLine(const Point3Dd &pv,
	const Point3Dd &nv) const
{
	Point3Dd u(pv);
	u -= *this;
	double u2 = u.scalarProduct(u);
	double v2 = nv.scalarProduct(nv);
	double uv = u.scalarProduct(nv);
	double dis2 = u2 - uv*uv/v2;
	return dis2;
}

/*! Check whether point collision with a AABB
*   @param ov - origin of AABB
*	 @param sz - size of AABB
*	 @return -1 if not collision
0 if point is on wall of AABB
1 if point is inside AABB
* 	 @author Nghi
*/
const int Point3Dd::isInAABBBox(const Point3Dd &ov, const double & sz) const
{
	if(v[0] < (ov.v[0] - EPSILON_VAL_) ||
		v[1] < (ov.v[1] - EPSILON_VAL_) ||
		v[2] < (ov.v[2] - EPSILON_VAL_) ||
		v[0] > (ov.v[0] + sz + EPSILON_VAL_) ||
		v[1] > (ov.v[1] + sz + EPSILON_VAL_) ||
		v[2] > (ov.v[2] + sz + EPSILON_VAL_)){
			return -1;
	}
	if(v[0] < (ov.v[0] + EPSILON_VAL_) &&
		v[1] < (ov.v[1] + EPSILON_VAL_) &&
		v[2] < (ov.v[2] + EPSILON_VAL_) &&
		v[0] > (ov.v[0] - sz + EPSILON_VAL_) &&
		v[1] > (ov.v[1] - sz + EPSILON_VAL_) &&
		v[2] > (ov.v[2] - sz + EPSILON_VAL_)){
			return 1;
	}
	return 0;
}

/*! Check whether point collision with a AABB
*   @param minCoord - mininum coordinate of AABB
*	 @param maxCoord - maximum coordinate of AABB
*	 @return true if point is inside AABB
* 	 @author Nghi
*/
const bool Point3Dd::isInAABBBox(const Point3Dd& minCoord, const Point3Dd& maxCoord) const
{
	if(v[0] < minCoord.v[0] ||
		v[1] < minCoord.v[1] ||
		v[2] < minCoord.v[2] ||
		v[0] > maxCoord.v[0] ||
		v[1] > maxCoord.v[1] ||
		v[2] > maxCoord.v[2] ){
			return false;
	}
	return true;
}

bool  Point3Dd::isVectorParallel(const Point3Dd & v)
{
	Point3Dd n = *this*v;
	if(fabs(n.v[0]) < EPSILON_VAL_ &&
		fabs(n.v[1]) < EPSILON_VAL_ &&
		fabs(n.v[2]) < EPSILON_VAL_){
			return true;
	}
	return false;
}

int Point3Dd::multiWithMatrix(const Matrix3x3d& m)
{
	double temp[3];
	for(int i = 0; i < 3; ++i){
		temp[i] = m._v[i][0]*v[0] + m._v[i][1]*v[1] + m._v[i][2]*v[2];
	}
	v[0] = temp[0];
	v[1] = temp[1];
	v[2] = temp[2];
	return 0;
}

Point2Dd Point3Dd::get2d( const int iCoord ) const {
	if ( 0 == iCoord )
		return (Point2Dd( v[1], v[2] ) );
	else if (1 == iCoord)
		return (Point2Dd( v[2], v[0] ) );
	else
		return( Point2Dd( v[0], v[1] ) );
}

const double Point3Dd::squareDistanceToPoint( const Point3Dd &point ) const {
	double dx = point.v[0] - v[0];
	double dy = point.v[1] - v[1];
	double dz = point.v[2] - v[2];
	return( dx*dx + dy*dy + dz*dz );
}

int Point3Dd::updateMinMaxCoord(double* mincoord, double *maxcoord) const
{
	if(v[0] < mincoord[0]){
		mincoord[0] = v[0];
	}
	else if(v[0] > maxcoord[0]){
		maxcoord[0] = v[0];
	}

	if(v[1] < mincoord[1]){
		mincoord[1] = v[1];
	}
	else if(v[1] > maxcoord[1]){
		maxcoord[1] = v[1];
	}

	if(v[2] < mincoord[2]){
		mincoord[2] = v[2];
	}
	else if(v[2] > maxcoord[2]){
		maxcoord[2] = v[2];
	}
	return 0;
}


const bool Point3Dd::isLieOnSegment(const Point3Dd& sp, const Point3Dd& ep) const{
	Vector3Dd v1 = *this - sp;
	if (v1.isVectorParallel(ep - sp)) {
		Vector3Dd v2 = *this - ep;
		double scalar = v1.scalarProduct(v2);
		if (scalar < 0) {
			return true;
		}
		return false;
	}
	return false;
}

bool Point3Dd::isCollisionWithAabb2D(const double ov[2], const double& lv)
{
	if(v[0] < (ov[0] - EPSILON_VAL_) ||
		v[1] < (ov[1] - EPSILON_VAL_) ||
		v[0] > (ov[0] + lv + EPSILON_VAL_) ||
		v[1] > (ov[1] + lv +EPSILON_VAL_)){
			return false;
	}
	return true;
}

bool Point3Dd::isInsidePolygon(const std::vector<Point3Dd*>& boundary, const Point3Dd& normal){
	int i0 = normal.getIndexOfMaxAbsCoord();
	int i1 = (i0 + 1) % 3;
	int i2 = (i0 + 2) % 3;

	unsigned n = boundary.size();
	if(0 == n){
		return false;
	}

	int nint = 0; // number intersection point
	double x1,x2,y1,y2;
	Point3Dd* p0 = boundary.back();
	Point3Dd* p1;
	x1 = p0->v[i1];
	y1 = p0->v[i2];
	for(size_t i = 0; i < n; ++i){
		size_t j = (i+1) % n;
		p1 = boundary[i];
		x2 = p1->v[i1];
		y2 = p1->v[i2];

		if (v[i2] < min2<double>(y1,y2) - EPSILON_VAL_BIG  ||
            v[i2] > max2<double>(y1, y2) + EPSILON_VAL_BIG)
		{
		}
		else if (fabs(y1 - y2) < EPSILON_VAL_){
            if (v[i1] > min2<double>(x1, x2) && v[i1] < max2<double>(x1, x2)){ // on boundary
				return true;
			}
		}

		else
		{
			double t = (v[i2] - y1)/(y2-y1);

			if(t < EPSILON_VAL_BIG){
			}
			else if(t > (1.0 - EPSILON_VAL_BIG))
			{
				double x = x1 + t*(x2 - x1);
				double y = boundary[j]->v[i2];
				if((y - y2)* (y1 - y2) > 0){
				}
				else if(x > v[i1]){
					nint++;
				}
			}
			else{
				double x = x1 + t*(x2 - x1);
				if(fabs(x - v[i1]) < EPSILON_VAL_){ // on boundary
					return true;
				}
				else if (x > (v[i1]))
				{
					nint++;
				}
			}
		}
		x1 = x2;
		y1 = y2;
	} // for

	if(1 == (nint % 2)){
		return true;
	}
	else{
		return false;
	}

	return false;
}

const double Point3Dd::squareDistanceToPlane(const Point3Dd &point, const Point3Dd& normal ) const {
    double d = point.x*normal.x + point.y*normal.y + point.z*normal.z;
    double numer = x*normal.x + y*normal.y + z*normal.z - d;
    double den = normal.x*normal.x + normal.y*normal.y + normal.z*normal.z;
    return numer*numer/den;


	Point3Dd p;
	this->orthoToPlane(point, normal, p);
	return this->squareDistanceToPoint(p);
}
