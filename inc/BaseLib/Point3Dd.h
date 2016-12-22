#ifndef BASE_POINT3DD_H
#define BASE_POINT3DD_H
#pragma once

#include <assert.h>
#include <vector>
#include <math.h>
#include "Point2D.h"
#define isnan(x) _isnan(x)
#define isinf(x) (!_finite(x))
#define fpu_error(x) (isinf(x) || isnan(x))

class Matrix3x3d;

class Point3Dd;

typedef Point3Dd Vector3Dd;

class Point3Dd
{
public:
   /**/
public:
   union{
      double v[3];
      struct {
         double x;
         double y;
         double z;
      };
   };
   Point3Dd();
   Point3Dd(const double *mv);
   Point3Dd(const float *mv);
   Point3Dd(const double& mx, const double& my, const double& mz);
   Point3Dd(const Point3Dd& other) ;
   explicit Point3Dd(const Point3Dd* other) ;
   
   void set(const double& mx, const double& my, const double& mz);
   void set(const double& val);
   
   double unit();
   double abs() const;
   double abs2() const;
   Point3Dd crossProduct(const Point3Dd& v1) const;
   int crossProduct(const Point3Dd* v, const Point3Dd* pv);
   double scalarProduct(const Point3Dd &v1) const;
   
   double dot(const Point3Dd &v1) const; // Dot (scalar) product
   Point3Dd cross(const Point3Dd& v1) const; // Cross product
   Point3Dd & normalize(); // Normalize this vector so that it has unit (1.0) length

   bool isEqualTo(const Point3Dd &v1) const;
   bool isStrongEqual(const Point3Dd &v1) const;
   bool isStrongEqualEps(const Point3Dd &v1, const double epsilon = EPSILON_VAL_) const;
   bool isEqualForNearCheck(const Point3Dd &v1) const;
   Point3Dd& operator = (const Point3Dd& other);
   Point3Dd& operator = (const double other[3]);
   bool operator == (const Point3Dd& v1) const;
   bool operator != (const Point3Dd& v1) const;
   Point3Dd operator +(const Point3Dd& v1) const;
   const Point3Dd& operator +=(const Point3Dd& v1);
   Point3Dd operator +(double d) const;
   const Point3Dd& operator +=(double d);
   Point3Dd operator -() const;
   Point3Dd operator -(const Point3Dd& v1) const;
   const Point3Dd& operator -=(const Point3Dd& v1);
   Point3Dd operator -(double d) const;
   const Point3Dd& operator -=(double d);
   //Point3Dd operator *(int i) const;
   Point3Dd operator *(double d) const;
   friend Point3Dd operator *(double d, Point3Dd const & p);
   const Point3Dd& operator *=(double d);
   Point3Dd operator *(const Point3Dd& v1) const;
   //Point3Dd operator /(int i) const;
   Point3Dd operator /(double d) const;
   const Point3Dd& operator /=(double d);

   void add(const Point3Dd& v1);
   void sub(const Point3Dd& v1);
   double angle(const Point3Dd& v1) const;
   double cosValue(const Point3Dd& v1) const;
   const int getIndexOfMaxAbsCoord() const;
   const int getIndexOfMinAbsCoord() const;
   const double distance(const Point3Dd &vp) const;
   const double xyDistance(const Point3Dd &vp) const;
   const double distance2(const Point3Dd &vp) const;
   void rotate(const Point3Dd& av, const double& theta);
   int rotAroundAVector(const Point3Dd & rotVec, const double & angle);
   void rotAroundX(const double &angle);
   void rotAroundX(const double &c, const double &s);
   void rotAroundY(const double &angle);
   void rotAroundY(const double &c, const double &s);
   void rotAroundZ(const double &angle);
   void rotAroundZ(const double &c, const double &s);
   void translateXY(const double &dx, const double &dy);
   int checkPositionCapsule(const double & r,const double & leng);
   int orthoToPlane(const Point3Dd &pv, const Point3Dd &nv,
	   Point3Dd &orth) const;
   int orthoVectorToPlane(const Point3Dd &pv, const Point3Dd &nv);
   const double calcSquareMinDistanceToSeg(const Point3Dd &sp, const Point3Dd &ep) const;
   void orthorToVector(const Point3Dd& dir);
   void calcProjectionOntoLine(const Point3Dd &pv, const Point3Dd &dir, Point3Dd &proj);
   const double calcSquareDistanceToLine(const Point3Dd &pv, const Point3Dd &nv) const;
   const int isInAABBBox(const Point3Dd& ov, const double & sz) const;
   const bool isInAABBBox(const Point3Dd& minCoord, const Point3Dd& maxCoord) const;

   bool isVectorParallel(const Point3Dd & v);
   int multiWithMatrix(const Matrix3x3d& m);
   // get 2d coord: iCoord = 0 -> Yz, 1 -> Zx, 2 -> Xy
   Point2Dd get2d( const int iCoord ) const;
   const double squareDistanceToPoint( const Point3Dd &point ) const;
   const double squareDistanceToPlane(const Point3Dd &point, const Point3Dd& normal ) const;
   const double squareModule(void) const;
   const double module(void) const;  // Length of vector
   const double length(void) const; // Length of vector
   const double length2(void) const; // Square length of vector
   //void normalize(void);
   int updateMinMaxCoord(double* minCoord, double *maxcoord) const;
   const bool isLieOnSegment(const Point3Dd& sp, const Point3Dd& ep) const;
   bool isCollisionWithAabb2D(const double ov[2], const double& lv);
   bool isInsidePolygon(const std::vector<Point3Dd*>& boundary, const Point3Dd& normal);

   const double * data() const {return v;}
   double * data() {return v;}
   double operator [](int i) const;
   double & operator [](int i);
   bool isValid() const{
       if(fpu_error(x) || fpu_error(y) || fpu_error(z)){
           return false;
       }
       return true;
   }
   friend double distance(const Point3Dd &v1, const Point3Dd &v2); // Distance between two points
   friend double dot(const Point3Dd &v1, const Point3Dd &v2); // Dot (scalar) product
   friend Point3Dd cross(const Point3Dd &v1, const Point3Dd &v2); // Cross product
   friend double length(const Point3Dd & p); // Dot (scalar) product
   friend double length2(const Point3Dd & p); // Dot (scalar) product
   friend Point3Dd & normalize(Point3Dd & p); // Normalize this vector so that it has unit (1.0) length

};

struct Point3DAscendingSort
{
    bool operator()(Point3Dd& fv, Point3Dd& ev)
    {
        if(fv.v[0] < ev.v[0]){
            return true;
        }
        else if(fv.v[0] > ev.v[0]){
            return false;
        }
        else{
            if(fv.v[1] < ev.v[1]){
                return true;
            }
            else if(fv.v[1] > ev.v[1]){
                return false;
            }
            else{
                if(fv.v[2] < ev.v[2]){
                    return true;
                }
                else{
                    return false;
                }
            }
        }
    }
};

inline Point3Dd::Point3Dd()
{
	v[0] = 0.0;
	v[1] = 0.0;
	v[2] = 0.0;
}

inline Point3Dd::Point3Dd(const double *mv)
{
	v[0] = mv[0];
	v[1] = mv[1];
	v[2] = mv[2];
}

inline Point3Dd::Point3Dd(const float *mv)
{
	v[0] = (double)mv[0];
	v[1] = (double)mv[1];
	v[2] = (double)mv[2];
}

inline Point3Dd::Point3Dd(const double& mx, const double& my, const double& mz)
{
	v[0] = mx;
	v[1] = my;
	v[2] = mz;
}

inline void Point3Dd::set(const double& mx, const double& my, const double& mz)
{
	v[0] = mx;
	v[1] = my;
	v[2] = mz;
}

inline void Point3Dd::set(const double& val)
{
    v[0] = val;
    v[1] = val;
    v[2] = val;
}


inline Point3Dd::Point3Dd(const Point3Dd& other)
{
	v[0] = other.v[0];
	v[1] = other.v[1];
	v[2] = other.v[2];
}

inline Point3Dd::Point3Dd(const Point3Dd* other)
{
	v[0] = other->v[0];
	v[1] = other->v[1];
	v[2] = other->v[2];
}

inline double Point3Dd::operator [](int i) const
{
	assert(i >=0 && i <= 2);

	return v[i];
}

inline double & Point3Dd::operator [](int i)
{
	assert(i >=0 && i <= 2);

	return v[i];
}

inline const double Point3Dd::squareModule(void) const {
	return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

inline const double Point3Dd::length2(void) const {
    return (v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
}

inline const double Point3Dd::module(void) const {
	return ((double)sqrt(squareModule()));
}

inline Point3Dd Point3Dd::cross(const Point3Dd& v1) const
{
	Point3Dd temp;
	temp.v[0] = v[1] * v1.v[2] - v[2] * v1.v[1];
	temp.v[1] = v[2] * v1.v[0] - v[0] * v1.v[2];
	temp.v[2] = v[0] * v1.v[1] - v[1] * v1.v[0];
	return temp;
}

inline const double Point3Dd::length(void) const {
	return sqrt(squareModule());
}

inline double Point3Dd::dot(const Point3Dd &v1) const
{
	double d = v[0] * v1.v[0] + v[1] * v1.v[1] + v[2] * v1.v[2];
	return d;
}

inline double dot(const Point3Dd &v1, const Point3Dd &v2) // Dot (scalar) product
{
    return v1.dot(v2);
}

inline double distance(const Point3Dd &v1, const Point3Dd &v2)
{
	return v1.distance(v2);
}

inline Point3Dd cross(const Point3Dd &v1, const Point3Dd &v2) // Cross product
{
    return v1.cross(v2);
}

inline double length(const Point3Dd & p) // Dot (scalar) product
{
    return p.length();
}

inline double length2(const Point3Dd & p) // Dot (scalar) product
{
    return p.length2();
}

#endif