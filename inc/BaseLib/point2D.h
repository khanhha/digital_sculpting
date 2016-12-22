#ifndef POINT_2D_H
#define POINT_2D_H

#include <math.h>
#include <float.h>
#include <assert.h>
#include "defined.h"

template<class T>
class Point2D {
public:
    Point2D(void) {
        _vcoords[0] = (T)0;
        _vcoords[1] = (T)0;
    }

    Point2D(const T & X, const T & Y) {
        _vcoords[0] = X;
        _vcoords[1] = Y;
    }

    Point2D(const Point2D& point) {
        _vcoords[0] = point._vcoords[0];
        _vcoords[1] = point._vcoords[1];
    }

    ~Point2D(void) {}

    Point2D& operator=(const Point2D& point) {
        _vcoords[0] = point._vcoords[0];
        _vcoords[1] = point._vcoords[1];
        return *this;
    }

    Point2D Point2D::operator -() const {
        Point2D temp;
        temp.x = -x;
        temp.y = -y;
        return temp;
    }

    const Point2D operator-(const Point2D& point) const {
        return (Point2D(_vcoords[0] - point._vcoords[0],
            _vcoords[1] - point._vcoords[1]));
    }

    const Point2D& operator -=(const Point2D& v1)
    {
       _vcoords[0] -= v1._vcoords[0];
       _vcoords[1] -= v1._vcoords[1];
       return *this;
    }

    const Point2D& operator -=(const T& val)
    {
        _vcoords[0] -= val;
        _vcoords[1] -= val;
        return *this;
    }
    const Point2D operator+(const Point2D& point) const {
        return (Point2D(_vcoords[0] + point._vcoords[0],
            _vcoords[1] + point._vcoords[1]));
    }

    const Point2D& operator +=(const Point2D& v1)
    {
       _vcoords[0] += v1._vcoords[0];
       _vcoords[1] += v1._vcoords[1];
       return *this;
    }

    const Point2D& operator +=(const T& val)
    {
        _vcoords[0] += val;
        _vcoords[1] += val;
        return *this;
    }

    const Point2D operator*(  const T& val ) const {
        return (Point2D(val*_vcoords[0], val*_vcoords[1]));
    }

    const Point2D& operator *=(const T& val)
    {
        _vcoords[0] *= val;
        _vcoords[1] *= val;
        return *this;
    }

    const bool operator<(  const Point2D& val ) const {
        if ( _vcoords[0] < val._vcoords[0] )
            return true;
        else if ( _vcoords[0] == val._vcoords[0] )
            return _vcoords[1] < val._vcoords[1];
        else return false;
    }

    const Point2D& operator +(const double& val){
        Point2D p;
        p._vcoords[0] = _vcoords[0] + val;
        p._vcoords[1] = _vcoords[1] + val;
        return p;
    }
    const Point2D& operator -(const double& val){
        Point2D p;
        p._vcoords[0] = _vcoords[0] - val;
        p._vcoords[1] = _vcoords[1] - val;
        return p;
    }

    void set(const T & x, const T & y) {
        _vcoords[0] = x; _vcoords[1] = y;
    }

    T* data(){ return _vcoords; }

    T crossProduct(const Point2D& v) const {
        return ( _vcoords[0]*v._vcoords[1] - _vcoords[1]*v._vcoords[0]);
    }

    const double distanceToPoint(const Point2D& point) const {
        return (sqrt(squareDistanceToPoint(point)));  
    }
	
	const double distance(const Point2D& point) const {
		return (sqrt(squareDistanceToPoint(point)));  
	}
    
    const double distance(const double &x, const double &y){
        return (sqrt(squareDistanceToPoint(x,y)));
    }

    const bool isInAABBNodeCircleBox(const Point2D &ov, const double &sz) const {
        return (squareDistanceToPoint(ov) <= sz*sz);
    }

    const double squareDistanceToPoint(const Point2D &point) const {
        double dx = double(point._vcoords[0] - _vcoords[0]);
        double dy = double(point._vcoords[1] - _vcoords[1]);
        return (dx*dx + dy*dy);
    }
	
    const double squareDistanceToPoint(const double &x, const double &y) const {
		double dx = double(x - _vcoords[0]);
		double dy = double(y - _vcoords[1]);
		return (dx*dx + dy*dy);
	}
    const double scalarProduct(const Point2D &point) const {
        return double(_vcoords[0]*point._vcoords[0] + _vcoords[1]*point._vcoords[1]);
    }

    const double dot(const Point2D &point) const {
        return double(_vcoords[0]*point._vcoords[0] + _vcoords[1]*point._vcoords[1]);
    }

    double normalize(void) {
		double module = _vcoords[0] * _vcoords[0] + _vcoords[1] * _vcoords[1];
		if (module > DBL_EPSILON){
			module = (double)sqrt(module);
			_vcoords[0] /= (T)module;
			_vcoords[1] /= (T)module;
		}
        return module;
    }

    void scale(const T & factor) {
        _vcoords[0] *= factor;
        _vcoords[1] *= factor;
    }

    //double scalarProduct(T nvec[3]);

    const double squareModule(void) const {
        return double(_vcoords[0]*_vcoords[0] + _vcoords[1]*_vcoords[1]);
    }

    const double module(void) const {
        return ((double)sqrt(squareModule()));
    }

    void rotateCcw(void) {
        Point2D oldV = *this;
        _vcoords[0] = -oldV._vcoords[1];
        _vcoords[1] = oldV._vcoords[0];
    }

    void negate(void) {
        _vcoords[0] = -_vcoords[0];
        _vcoords[1] = -_vcoords[1];
    }

	const bool isConcident(const Point2D& p, T const & eps = (T)EPSILON_VAL_) const{
		if (fabs(_vcoords[0] - p._vcoords[0]) < eps
			&& fabs(_vcoords[1] - p._vcoords[1]) < eps) {
				return true;
		}
		return false;
	}

	const bool isXCoincident(const Point2D& p) const{
		if (fabs(_vcoords[0] - p._vcoords[0]) < (T)EPSILON_VAL_) {
				return true;
		}
		return false;
	}

	const bool isYCoincident(const Point2D& p) const{
		if (fabs(_vcoords[1] - p._vcoords[1]) < (T)EPSILON_VAL_) {
			return true;
		}
		return false;
	}

	const double perp( const Point2D& vec ) const {
		return double(_vcoords[0]*vec._vcoords[1] - _vcoords[1]*vec._vcoords[0] );
	}
    
	const double squareDistanceToLine( const Point2D &firstPoint, 
		const Point2D &lastPoint ) const {
			Point2D v = lastPoint - firstPoint;
			Point2D w = *this - firstPoint;
			double b = v.scalarProduct(v);
			if(0 != b)
				b = w.scalarProduct(v) / b;

			Point2D Pb;
			Pb._vcoords[0] = firstPoint._vcoords[0] + (T)b * v._vcoords[0];
			Pb._vcoords[1] = firstPoint._vcoords[1] + (T)b * v._vcoords[1];
			return (double)squareDistanceToPoint(Pb);
	}

	const double squareDistanceToSegment(const Point2D &startPoint,
		const Point2D &endPoint) const {
			//Point2D v = endPoint - startPoint;
			//Point2D w = *this - startPoint;
			//double c1 = w.scalarProduct(v);
			//double c2 = v.scalarProduct(v);
			//if(c1 <= 0)
			//	return (double)squareDistanceToPoint(startPoint);
			//if(c2 <= c1)
			//	return (double)squareDistanceToPoint(endPoint);
			//return (double)squareDistanceToLine(startPoint, endPoint);

			// Get projection of this point to the line
			Point2D v = *this - startPoint;
			Point2D n = endPoint  - startPoint;
			n.normalize();
			T l = v.scalarProduct(n);
			Point2D prjP = startPoint + n * l;

			Point2D w1 = prjP - startPoint;
			Point2D w2 = prjP - endPoint;
			if (w1.scalarProduct(w2) < 0) {
				double disLine = (double)squareDistanceToLine(startPoint, endPoint);
				return disLine;
			}
			
			double dis1 = squareDistanceToPoint(startPoint);
			double dis2 = squareDistanceToPoint(endPoint);
			double minDis = min2(dis1, dis2);
			return minDis;
	}
	bool isVectorParallel(const Point2D & v) {
		if (fabs(_vcoords[0]) < EPSILON_VAL_) {// // OY
			return fabs(v._vcoords[0]) < EPSILON_VAL_;
		}
		
		if (fabs(_vcoords[1]) < EPSILON_VAL_) {// // OX
			return fabs(v._vcoords[1]) < EPSILON_VAL_;
		}

		return fabs(_vcoords[0]/v._vcoords[0] - _vcoords[1]/v._vcoords[1]) < EPSILON_VAL_;


	}

	const Point2D getNormal() const {
		return Point2D(-_vcoords[1], _vcoords[0]); 
	}

	const bool isOnLine(const Point2D& p, 
		const Point2D& d ) const {
			Point2D v = p - *this;
			Point2D n_v = v.getNormal();
			return (fabs(d.scalarProduct(n_v)) < EPSILON_VAL_ );
	}

   const bool isAlignmentVector(const Point2D& other) const
   {
      double d = _vcoords[0]*other._vcoords[1] - _vcoords[1]*other._vcoords[0];
      if(fabs(d) < EPSILON_VAL_BIG){
         return true;
      }
      return false;
   }

   bool isCollisionWithAabb2D(const T ov[2], const T& lv)
   {
      if(_vcoords[0] < (ov[0] - EPSILON_VAL_) ||
       _vcoords[1] < (ov[1] - EPSILON_VAL_) ||
       _vcoords[0] > (ov[0] + lv + EPSILON_VAL_) ||
       _vcoords[1] > (ov[1] + lv +EPSILON_VAL_)){
         return false;
    }
    return true;
   }
   
   double operator [](int i) const
   {
	   assert(i >=0 && i <= 1);

	   return _vcoords[i];
   }

   double & operator [](int i)
   {
	   assert(i >=0 && i <= 1);

	   return _vcoords[i];
   }

   bool isTheSame(const Point2D & other) const
   {
       return (x == other.x) && (y == other.y);
   }

   void orthorToVector(const Point2D& dir)
   {
       double num = x * dir.x + y * dir.y;
       double den = dir.x * dir.x + dir.y * dir.y;
       //    assert(fabs(den) > EPSILON_VAL_);
       double t = num / den;
       x = t*dir.x;
       y = t*dir.y;
   }

   Point2D calcProjectionVectorToLine(const Point2D &pv, const Point2D &dir)
   {
       Point2D proj;
       proj = *this;
       proj -= pv;
       proj.orthorToVector(dir);
       return proj;
   }

   Point2D calcProjectionOntoLine(const Point2D &pv,
       const Point2D &dir)
   {
       Point2D proj;
       proj = *this;
       proj -= pv;
       proj.orthorToVector(dir);
       proj += pv;
       return proj;
   }
public:
	union
	{
		T _vcoords[2];
		struct 
		{
			T x;
			T y;
		};
	};
};

typedef Point2D<double> Point2Dd;
typedef Point2D<float> Point2Df;
typedef Point2D<int> Point2Di;
typedef Point2Dd Vector2Dd;
typedef Point2Df Vector2Df;
typedef Point2Di Vector2Di;

#endif