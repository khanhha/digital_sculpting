#ifndef BASE_MATRIX3X3D_H
#define BASE_MATRIX3X3D_H

#include "Point3Dd.h"
class Matrix3x3d
{
public:
    Matrix3x3d(void);
    Matrix3x3d(const Matrix3x3d &m);
   ~Matrix3x3d(void);
   
   void setIdentity();
   Matrix3x3d operator*( Matrix3x3d& other) ;
   Matrix3x3d calRotateMatrixRevert();
   Matrix3x3d calRotateMatrixRevert2();
   Matrix3x3d calRotateMatrixRevert(Matrix3x3d& other);
   const Point3Dd operator*(const Point3Dd& p) const;
   const Matrix3x3d operator*(double d) const;
   const Matrix3x3d operator/(double d) const;
   void  makeRotationAboutAxis(Point3Dd& axis, double angle);
   Vector3Dd getRow(unsigned i) const;
   Vector3Dd getColumn(unsigned i) const;
   void setColumn(unsigned i,const Vector3Dd &v);
   void setRow(unsigned i, const Vector3Dd &v);
   void unit();
   Matrix3x3d inverse() const;

public:
   double _v[3][3];
};

#endif