#ifndef BASELIB_MODELMATRIX_H
#define BASELIB_MODELMATRIX_H
#pragma once

#include "Point3Dd.h"
#include "point2D.h"
/*
     0  4   8   12
     1  5   9   13
     2  6   10  14
     3  7   11  15
*/

#define DEGTORAD(degree) ((degree) * (3.141592654f / 180.0f))
#define RADTODEG(radian) ((radian) * (180.0f / 3.141592654f)

#define MODEL_MATRIX_LENGTH (16)

class KModelMatrix
{
public:

    double _v[MODEL_MATRIX_LENGTH];

    KModelMatrix()  { setIdentity(); }

    KModelMatrix(double m);

    void getValue(double m[MODEL_MATRIX_LENGTH]);
    void setValue(const double* v);
    void setValue(double m);

    void setValue(double m0, double m4, double  m8, double m12,
               double m1, double m5, double  m9, double m13,
               double m2, double m6, double m10, double m14,
               double m3, double m7, double m11, double m15 );
    void setValueTranspose( double m0, double m1, double  m2, double m3,
        double m4, double m5, double  m6, double m7,
        double m8, double m9, double m10, double m11,
        double m12, double m13, double m14, double m15 );
    void  setIdentity(void);
    bool isIdentity() const;
    static KModelMatrix calcTranslate(const Point3Dd &trans);
    static KModelMatrix calcTranslate_x(const double &dist);
    static KModelMatrix calcTranslate_y(const double &dist);
    static KModelMatrix calcTranslate_z(const double &dist);
    static KModelMatrix calcRotate(const double &angle, Point3Dd &axis);
    static KModelMatrix calcRotate_x(const double &angle);
    static KModelMatrix calcRotate_y(const double &angle);
    static KModelMatrix calcRotate_z(const double &angle);


    static KModelMatrix calcRotateAroundCenter(const Vector3Dd& angle, bool isInvert);  


    int index(int i, int j) const {return i + 4*j;}
    
    double const & operator()(int i, int j) const;
    double & operator()(int i, int j);

    void swap(const unsigned int& x1,const unsigned int& y1, const unsigned int& x2,const unsigned int& y2);
    double const operator [] (int i) const {return _v[i];};
    double & operator [] (int i) {return _v[i];};

    // Operators...
    KModelMatrix operator + (const KModelMatrix &other);
    KModelMatrix operator - (const KModelMatrix &other);
    KModelMatrix operator * (const KModelMatrix &other);

    Vector3Dd multiply(const Vector3Dd &other) const;
    Point2Df multiply(const Point2Df &other) const;
    Vector3Dd multiplyRotOnly(const Vector3Dd &other) const;
    KModelMatrix operator * (const double scalar);
    KModelMatrix& operator = (const KModelMatrix& other);


    void getTranslateVector(Vector3Dd &v);
    void setTranslateVector(Vector3Dd &v);

    void scale(const Point3Dd &scale);
    void translate(const Vector3Dd& v);
    void inverse();
    void transpose();

    void multiRotMatrixOnly(const KModelMatrix &other);
    void rotate(const Vector3Dd& angle, bool isInvert);

};

inline double const & KModelMatrix::operator()(int i, int j) const 
{
    return _v[index(i, j)];
}

inline double & KModelMatrix::operator()(int i, int j) 
{
    return _v[index(i, j)];
}


inline KModelMatrix::KModelMatrix(double v)  
{ 
    setValue(v);
}

inline void KModelMatrix::setValue(const double* v)
{
    memcpy(_v, v, sizeof(_v));
}

inline void KModelMatrix::setValue( double m0, double m4, double  m8, double m12,
						 double m1, double m5, double  m9, double m13,
						 double m2, double m6, double m10, double m14,
						 double m3, double m7, double m11, double m15 )
{
	_v[0]=m0; _v[4]=m4; _v[8] =m8;  _v[12]=m12;
	_v[1]=m1; _v[5]=m5; _v[9] =m9;  _v[13]=m13;
	_v[2]=m2; _v[6]=m6; _v[10]=m10; _v[14]=m14;
	_v[3]=m3; _v[7]=m7; _v[11]=m11; _v[15]=m15;
}

inline void KModelMatrix::setValueTranspose( double m0, double m1, double  m2, double m3,
    double m4, double m5, double  m6, double m7,
    double m8, double m9, double m10, double m11,
    double m12, double m13, double m14, double m15 )
{
    _v[0]=m0; _v[4]=m4; _v[8] =m8;  _v[12]=m12;
    _v[1]=m1; _v[5]=m5; _v[9] =m9;  _v[13]=m13;
    _v[2]=m2; _v[6]=m6; _v[10]=m10; _v[14]=m14;
    _v[3]=m3; _v[7]=m7; _v[11]=m11; _v[15]=m15;
}

inline void KModelMatrix::getValue(double m[16])
{
    memcpy(m, _v, sizeof(_v));
}

inline void KModelMatrix::setValue(double v)
{
    for (int i = 0; i < 16; i++)
        _v[i] = v;
}

inline void KModelMatrix::setIdentity( void )
{
    _v[0]=1.0f; _v[4]=0.0f; _v[8] =0.0f; _v[12]=0.0f;
    _v[1]=0.0f; _v[5]=1.0f; _v[9] =0.0f; _v[13]=0.0f;
    _v[2]=0.0f; _v[6]=0.0f; _v[10]=1.0f; _v[14]=0.0f;
    _v[3]=0.0f; _v[7]=0.0f; _v[11]=0.0f; _v[15]=1.0f;
}

inline KModelMatrix KModelMatrix::calcTranslate( const Point3Dd &trans )
{
    KModelMatrix ret;

    ret._v[12] = trans.x;
    ret._v[13] = trans.y;
    ret._v[14] = trans.z;

	return ret;
}

inline KModelMatrix KModelMatrix::calcTranslate_x( const double &dist )
{
    KModelMatrix ret;
    ret._v[12] = dist;
	return ret;
}

inline KModelMatrix KModelMatrix::calcTranslate_y( const double &dist )
{
    KModelMatrix ret;
    ret._v[13] = dist;
	return ret;
}

inline KModelMatrix KModelMatrix::calcTranslate_z( const double &dist )
{
    KModelMatrix ret;
    ret._v[14] = dist;
	return ret;
}


inline KModelMatrix KModelMatrix::calcRotate_x( const double &angle )
{
    double s = sin(DEGTORAD(angle));
    double c = cos(DEGTORAD(angle));

    KModelMatrix ret;

    ret._v[5]  =  c;
    ret._v[6]  =  s;
    ret._v[9]  = -s;
    ret._v[10] =  c;

	return ret;
}

inline KModelMatrix KModelMatrix::calcRotate_y( const double &angle )
{
    double s = sin(DEGTORAD(angle));
    double c = cos(DEGTORAD(angle));

    KModelMatrix ret;

    ret._v[0]  =  c;
    ret._v[2]  = -s;
    ret._v[8]  =  s;
    ret._v[10] =  c;

	return ret;
}

inline KModelMatrix KModelMatrix::calcRotate_z( const double &angle )
{
    double s = sin(DEGTORAD(angle));
    double c = cos(DEGTORAD(angle));

    KModelMatrix ret;

    ret._v[0] =  c;
    ret._v[1] =  s;
    ret._v[4] = -s;
    ret._v[5] =  c;

	return ret;
}

inline void KModelMatrix::scale( const Point3Dd &scale )
{

    KModelMatrix result;
    result._v[0]  = scale.x;
    result._v[5]  = scale.y;
    result._v[10] = scale.z;
    *this = (*this)*result;
}

inline void KModelMatrix::translate(const Vector3Dd& v)
{

    KModelMatrix result;
    result._v[12] = v.x;
    result._v[13] = v.y;
    result._v[14] = v.z;
    *this = (*this)*result;
}


inline KModelMatrix KModelMatrix::operator + ( const KModelMatrix &other )
{
    KModelMatrix result;

    result._v[0]  = _v[0]  + other._v[0];
    result._v[1]  = _v[1]  + other._v[1];
    result._v[2]  = _v[2]  + other._v[2];
    result._v[3]  = _v[3]  + other._v[3];

    result._v[4]  = _v[4]  + other._v[4];
    result._v[5]  = _v[5]  + other._v[5];
    result._v[6]  = _v[6]  + other._v[6];
    result._v[7]  = _v[7]  + other._v[7];

    result._v[8]  = _v[8]  + other._v[8];
    result._v[9]  = _v[9]  + other._v[9];
    result._v[10] = _v[10] + other._v[10];
    result._v[11] = _v[11] + other._v[11];

    result._v[12] = _v[12] + other._v[12];
    result._v[13] = _v[13] + other._v[13];
    result._v[14] = _v[14] + other._v[14];
    result._v[15] = _v[15] + other._v[15];

    return result;
}

inline KModelMatrix KModelMatrix::operator - ( const KModelMatrix &other )
{
    KModelMatrix result;

    result._v[0]  = _v[0]  - other._v[0];
    result._v[1]  = _v[1]  - other._v[1];
    result._v[2]  = _v[2]  - other._v[2];
    result._v[3]  = _v[3]  - other._v[3];

    result._v[4]  = _v[4]  - other._v[4];
    result._v[5]  = _v[5]  - other._v[5];
    result._v[6]  = _v[6]  - other._v[6];
    result._v[7]  = _v[7]  - other._v[7];

    result._v[8]  = _v[8]  - other._v[8];
    result._v[9]  = _v[9]  - other._v[9];
    result._v[10] = _v[10] - other._v[10];
    result._v[11] = _v[11] - other._v[11];

    result._v[12] = _v[12] - other._v[12];
    result._v[13] = _v[13] - other._v[13];
    result._v[14] = _v[14] - other._v[14];
    result._v[15] = _v[15] - other._v[15];

    return result;
}

inline KModelMatrix KModelMatrix::operator * ( const KModelMatrix &other )
{
    KModelMatrix result;

    result._v[0]  = (_v[0]*other._v[0])+(_v[4]*other._v[1])+(_v[8]*other._v[2])+(_v[12]*other._v[3]);
    result._v[1]  = (_v[1]*other._v[0])+(_v[5]*other._v[1])+(_v[9]*other._v[2])+(_v[13]*other._v[3]);
    result._v[2]  = (_v[2]*other._v[0])+(_v[6]*other._v[1])+(_v[10]*other._v[2])+(_v[14]*other._v[3]);
    result._v[3]  = (_v[3]*other._v[0])+(_v[7]*other._v[1])+(_v[11]*other._v[2])+(_v[15]*other._v[3]);

    result._v[4]  = (_v[0]*other._v[4])+(_v[4]*other._v[5])+(_v[8]*other._v[6])+(_v[12]*other._v[7]);
    result._v[5]  = (_v[1]*other._v[4])+(_v[5]*other._v[5])+(_v[9]*other._v[6])+(_v[13]*other._v[7]);
    result._v[6]  = (_v[2]*other._v[4])+(_v[6]*other._v[5])+(_v[10]*other._v[6])+(_v[14]*other._v[7]);
    result._v[7]  = (_v[3]*other._v[4])+(_v[7]*other._v[5])+(_v[11]*other._v[6])+(_v[15]*other._v[7]);

    result._v[8]  = (_v[0]*other._v[8])+(_v[4]*other._v[9])+(_v[8]*other._v[10])+(_v[12]*other._v[11]);
    result._v[9]  = (_v[1]*other._v[8])+(_v[5]*other._v[9])+(_v[9]*other._v[10])+(_v[13]*other._v[11]);
    result._v[10] = (_v[2]*other._v[8])+(_v[6]*other._v[9])+(_v[10]*other._v[10])+(_v[14]*other._v[11]);
    result._v[11] = (_v[3]*other._v[8])+(_v[7]*other._v[9])+(_v[11]*other._v[10])+(_v[15]*other._v[11]);

    result._v[12] = (_v[0]*other._v[12])+(_v[4]*other._v[13])+(_v[8]*other._v[14])+(_v[12]*other._v[15]);
    result._v[13] = (_v[1]*other._v[12])+(_v[5]*other._v[13])+(_v[9]*other._v[14])+(_v[13]*other._v[15]);
    result._v[14] = (_v[2]*other._v[12])+(_v[6]*other._v[13])+(_v[10]*other._v[14])+(_v[14]*other._v[15]);
    result._v[15] = (_v[3]*other._v[12])+(_v[7]*other._v[13])+(_v[11]*other._v[14])+(_v[15]*other._v[15]);

    return result;
}

inline KModelMatrix KModelMatrix::operator * ( const double scalar )
{
    KModelMatrix result;

    result._v[0]  = _v[0]  * scalar;
    result._v[1]  = _v[1]  * scalar;
    result._v[2]  = _v[2]  * scalar;
    result._v[3]  = _v[3]  * scalar;

    result._v[4]  = _v[4]  * scalar;
    result._v[5]  = _v[5]  * scalar;
    result._v[6]  = _v[6]  * scalar;
    result._v[7]  = _v[7]  * scalar;

    result._v[8]  = _v[8]  * scalar;
    result._v[9]  = _v[9]  * scalar;
    result._v[10] = _v[10] * scalar;
    result._v[11] = _v[11] * scalar;

    result._v[12] = _v[12] * scalar;
    result._v[13] = _v[13] * scalar;
    result._v[14] = _v[14] * scalar;
    result._v[15] = _v[15] * scalar;

    return result;
}

inline void KModelMatrix::inverse()
{
    double inv[16], det;
    int i;

    inv[0] = _v[5]  * _v[10] * _v[15] - 
        _v[5]  * _v[11] * _v[14] - 
        _v[9]  * _v[6]  * _v[15] + 
        _v[9]  * _v[7]  * _v[14] +
        _v[13] * _v[6]  * _v[11] - 
        _v[13] * _v[7]  * _v[10];

    inv[4] = -_v[4]  * _v[10] * _v[15] + 
        _v[4]  * _v[11] * _v[14] + 
        _v[8]  * _v[6]  * _v[15] - 
        _v[8]  * _v[7]  * _v[14] - 
        _v[12] * _v[6]  * _v[11] + 
        _v[12] * _v[7]  * _v[10];

    inv[8] = _v[4]  * _v[9] * _v[15] - 
        _v[4]  * _v[11] * _v[13] - 
        _v[8]  * _v[5] * _v[15] + 
        _v[8]  * _v[7] * _v[13] + 
        _v[12] * _v[5] * _v[11] - 
        _v[12] * _v[7] * _v[9];

    inv[12] = -_v[4]  * _v[9] * _v[14] + 
        _v[4]  * _v[10] * _v[13] +
        _v[8]  * _v[5] * _v[14] - 
        _v[8]  * _v[6] * _v[13] - 
        _v[12] * _v[5] * _v[10] + 
        _v[12] * _v[6] * _v[9];

    inv[1] = -_v[1]  * _v[10] * _v[15] + 
        _v[1]  * _v[11] * _v[14] + 
        _v[9]  * _v[2] * _v[15] - 
        _v[9]  * _v[3] * _v[14] - 
        _v[13] * _v[2] * _v[11] + 
        _v[13] * _v[3] * _v[10];

    inv[5] = _v[0]  * _v[10] * _v[15] - 
        _v[0]  * _v[11] * _v[14] - 
        _v[8]  * _v[2] * _v[15] + 
        _v[8]  * _v[3] * _v[14] + 
        _v[12] * _v[2] * _v[11] - 
        _v[12] * _v[3] * _v[10];

    inv[9] = -_v[0]  * _v[9] * _v[15] + 
        _v[0]  * _v[11] * _v[13] + 
        _v[8]  * _v[1] * _v[15] - 
        _v[8]  * _v[3] * _v[13] - 
        _v[12] * _v[1] * _v[11] + 
        _v[12] * _v[3] * _v[9];

    inv[13] = _v[0]  * _v[9] * _v[14] - 
        _v[0]  * _v[10] * _v[13] - 
        _v[8]  * _v[1] * _v[14] + 
        _v[8]  * _v[2] * _v[13] + 
        _v[12] * _v[1] * _v[10] - 
        _v[12] * _v[2] * _v[9];

    inv[2] = _v[1]  * _v[6] * _v[15] - 
        _v[1]  * _v[7] * _v[14] - 
        _v[5]  * _v[2] * _v[15] + 
        _v[5]  * _v[3] * _v[14] + 
        _v[13] * _v[2] * _v[7] - 
        _v[13] * _v[3] * _v[6];

    inv[6] = -_v[0]  * _v[6] * _v[15] + 
        _v[0]  * _v[7] * _v[14] + 
        _v[4]  * _v[2] * _v[15] - 
        _v[4]  * _v[3] * _v[14] - 
        _v[12] * _v[2] * _v[7] + 
        _v[12] * _v[3] * _v[6];

    inv[10] = _v[0]  * _v[5] * _v[15] - 
        _v[0]  * _v[7] * _v[13] - 
        _v[4]  * _v[1] * _v[15] + 
        _v[4]  * _v[3] * _v[13] + 
        _v[12] * _v[1] * _v[7] - 
        _v[12] * _v[3] * _v[5];

    inv[14] = -_v[0]  * _v[5] * _v[14] + 
        _v[0]  * _v[6] * _v[13] + 
        _v[4]  * _v[1] * _v[14] - 
        _v[4]  * _v[2] * _v[13] - 
        _v[12] * _v[1] * _v[6] + 
        _v[12] * _v[2] * _v[5];

    inv[3] = -_v[1] * _v[6] * _v[11] + 
        _v[1] * _v[7] * _v[10] + 
        _v[5] * _v[2] * _v[11] - 
        _v[5] * _v[3] * _v[10] - 
        _v[9] * _v[2] * _v[7] + 
        _v[9] * _v[3] * _v[6];

    inv[7] = _v[0] * _v[6] * _v[11] - 
        _v[0] * _v[7] * _v[10] - 
        _v[4] * _v[2] * _v[11] + 
        _v[4] * _v[3] * _v[10] + 
        _v[8] * _v[2] * _v[7] - 
        _v[8] * _v[3] * _v[6];

    inv[11] = -_v[0] * _v[5] * _v[11] + 
        _v[0] * _v[7] * _v[9] + 
        _v[4] * _v[1] * _v[11] - 
        _v[4] * _v[3] * _v[9] - 
        _v[8] * _v[1] * _v[7] + 
        _v[8] * _v[3] * _v[5];

    inv[15] = _v[0] * _v[5] * _v[10] - 
        _v[0] * _v[6] * _v[9] - 
        _v[4] * _v[1] * _v[10] + 
        _v[4] * _v[2] * _v[9] + 
        _v[8] * _v[1] * _v[6] - 
        _v[8] * _v[2] * _v[5];

    det = _v[0] * inv[0] + _v[1] * inv[4] + _v[2] * inv[8] + _v[3] * inv[12];

    if (det == 0)
    {
        setIdentity();
        throw std::runtime_error("Singular matrix.");
    }

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        _v[i] = inv[i] * det;

}

inline void KModelMatrix::transpose()
{
    swap(0,1,1,0);
    swap(0,2,2,0);
    swap(0,3,3,0);
    swap(1,2,2,1);
    swap(1,3,3,1);
    swap(2,3,3,2);

}

inline void KModelMatrix::swap(const unsigned int& x1,const unsigned int& y1,
    const unsigned int& x2,const unsigned int& y2)
{
    double temp  = _v[index(x1,y1)];
    _v[index(x1,y1)] = _v[index(x2,y2)];
    _v[index(x2,y2)] = temp;
}
#endif // _MATRIX4X4F_H_

