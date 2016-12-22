#include "KModelMatrix.h"

void KModelMatrix::getTranslateVector(Vector3Dd &v)
{
    v.x = _v[12];
    v.y = _v[13];
    v.z = _v[14];
}

void KModelMatrix::setTranslateVector(Vector3Dd &v)
{
    _v[12] = v.x;
    _v[13] = v.y;
    _v[14] = v.z;
}

KModelMatrix KModelMatrix::calcRotateAroundCenter(const Vector3Dd& angle, bool isInvert)
{

    KModelMatrix rot;
    double rad_ang[3];
    rad_ang[0] = angle.v[0]*PI_VAL_/180;
    rad_ang[1] = angle.v[1]*PI_VAL_/180;
    rad_ang[2] = angle.v[2]*PI_VAL_/180;

    if (isInvert)
    {
        rad_ang[0] = -rad_ang[0];
        rad_ang[1] = -rad_ang[1];
        rad_ang[2] = -rad_ang[2];
    }

    double c0 = cos(rad_ang[0]);
    double c1 = cos(rad_ang[1]);
    double c2 = cos(rad_ang[2]);
    double s0 = sin(rad_ang[0]);
    double s1 = sin(rad_ang[1]);
    double s2 = sin(rad_ang[2]);
#if 1
    if (isInvert)
    {
        rot._v[0] = c1*c2;
        rot._v[4] = s0*s1*c2 - c0*s2;
        rot._v[8] = s0*s2 + c0*s1*c2;
        rot._v[1] = c1*s2;
        rot._v[5] = c0*c2 + s0*s1*s2;
        rot._v[9] = c0*s1*s2 - s0*c2;
        rot._v[2] = -s1;
        rot._v[6] = s0*c1;
        rot._v[10] = c0*c1;
    }
    else
    {
        rot._v[0] = c1*c2;
        rot._v[4] = -c1*s2;
        rot._v[8] = s1;
        rot._v[1] = s0*s1*c2 + c0*s2;
        rot._v[5] = -s0*s1*s2 + c0*c2;
        rot._v[9] = -s0*c1;
        rot._v[2] = -c0*c2*s1 + s0*s2;
        rot._v[6] = c0*s1*s2 + s0*c2;
        rot._v[10] = c0*c1;
    }
#else
    if (isInvert)
    {
        rot._v[0] = c1*c2;
        rot._v[1] = s0*s1*c2 - c0*s2;
        rot._v[2] = s0*s2 + c0*s1*c2;
        rot._v[4] = c1*s2;
        rot._v[5] = c0*c2 + s0*s1*s2;
        rot._v[6] = c0*s1*s2 - s0*c2;
        rot._v[7] = -s1;
        rot._v[8] = s0*c1;
        rot._v[10] = c0*c1;
    }
    else
    {
        rot._v[0] = c1*c2;
        rot._v[1] = -c1*s2;
        rot._v[2] = s1;
        rot._v[4] = s0*s1*c2 + c0*s2;
        rot._v[5] = -s0*s1*s2 + c0*c2;
        rot._v[6] = -s0*c1;
        rot._v[8] = -c0*c2*s1 + s0*s2;
        rot._v[9] = c0*s1*s2 + s0*c2;
        rot._v[10] = c0*c1;
    }
#endif
    return rot;
}

void KModelMatrix::multiRotMatrixOnly(const KModelMatrix &other)
{
    KModelMatrix result;

    result._v[0]  = (_v[0]*other._v[0])+(_v[4]*other._v[1])+(_v[8]*other._v[2]);
    result._v[1]  = (_v[1]*other._v[0])+(_v[5]*other._v[1])+(_v[9]*other._v[2]);
    result._v[2]  = (_v[2]*other._v[0])+(_v[6]*other._v[1])+(_v[10]*other._v[2]);

    result._v[4]  = (_v[0]*other._v[4])+(_v[4]*other._v[5])+(_v[8]*other._v[6]);
    result._v[5]  = (_v[1]*other._v[4])+(_v[5]*other._v[5])+(_v[9]*other._v[6]);
    result._v[6]  = (_v[2]*other._v[4])+(_v[6]*other._v[5])+(_v[10]*other._v[6]);

    result._v[8]  = (_v[0]*other._v[8])+(_v[4]*other._v[9])+(_v[8]*other._v[10]);
    result._v[9]  = (_v[1]*other._v[8])+(_v[5]*other._v[9])+(_v[9]*other._v[10]);
    result._v[10] = (_v[2]*other._v[8])+(_v[6]*other._v[9])+(_v[10]*other._v[10]);
    
    _v[0] = result._v[0];
    _v[1] = result._v[1];
    _v[2] = result._v[2];
    _v[4] = result._v[4];
    _v[5] = result._v[5];
    _v[6] = result._v[6];
    _v[8] = result._v[8];
    _v[9] = result._v[9];
    _v[10] = result._v[10];

}


Vector3Dd KModelMatrix::multiply( const Vector3Dd &other ) const
{
    Vector3Dd result;

    result.x  = (_v[0]*other.x)+(_v[4]*other.y)+(_v[8]*other.z) + _v[12];
    result.y  = (_v[1]*other.x)+(_v[5]*other.y)+(_v[9]*other.z) + _v[13];
    result.z  = (_v[2]*other.x)+(_v[6]*other.y)+(_v[10]*other.z) + _v[14];
    return result;
}

Point2Df KModelMatrix::multiply(const Point2Df &other) const
{
    Point2Df result;
    result.x = static_cast<float> (_v[0]*other.x + _v[4]*other.y + _v[12]);
    result.y = static_cast<float> (_v[1]*other.x + _v[5]*other.y + _v[13]);
    return result;

}

Vector3Dd KModelMatrix::multiplyRotOnly( const Vector3Dd &other ) const
{
    Vector3Dd result;

    result.x  = (_v[0]*other.x)+(_v[4]*other.y)+(_v[8]*other.z);
    result.y  = (_v[1]*other.x)+(_v[5]*other.y)+(_v[9]*other.z);
    result.z  = (_v[2]*other.x)+(_v[6]*other.y)+(_v[10]*other.z);
    return result;
}

KModelMatrix& KModelMatrix::operator = (const KModelMatrix& other){
    if (this != (&other)) {
        for(int i = 0; i < 16; ++i){
            _v[i] = other._v[i];
        }
    }
    return *this;
}


void KModelMatrix::rotate(const Vector3Dd& angle, bool isInvert)
{

    KModelMatrix rot;
    double rad_ang[3];
    rad_ang[0] = angle.v[0]*PI_VAL_/180;
    rad_ang[1] = angle.v[1]*PI_VAL_/180;
    rad_ang[2] = angle.v[2]*PI_VAL_/180;

    if (isInvert)
    {
        rad_ang[0] = -rad_ang[0];
        rad_ang[1] = -rad_ang[1];
        rad_ang[2] = -rad_ang[2];
    }

    double c0 = cos(rad_ang[0]);
    double c1 = cos(rad_ang[1]);
    double c2 = cos(rad_ang[2]);
    double s0 = sin(rad_ang[0]);
    double s1 = sin(rad_ang[1]);
    double s2 = sin(rad_ang[2]);

    if (isInvert)
    {
        rot._v[0] = c1*c2;
        rot._v[4] = s0*s1*c2 - c0*s2;
        rot._v[8] = s0*s2 + c0*s1*c2;
        rot._v[1] = c1*s2;
        rot._v[5] = c0*c2 + s0*s1*s2;
        rot._v[9] = c0*s1*s2 - s0*c2;
        rot._v[2] = -s1;
        rot._v[6] = s0*c1;
        rot._v[10] = c0*c1;
    }
    else
    {
        rot._v[0] = c1*c2;
        rot._v[4] = -c1*s2;
        rot._v[8] = s1;
        rot._v[1] = s0*s1*c2 + c0*s2;
        rot._v[5] = -s0*s1*s2 + c0*c2;
        rot._v[9] = -s0*c1;
        rot._v[2] = -c0*c2*s1 + s0*s2;
        rot._v[6] = c0*s1*s2 + s0*c2;
        rot._v[10] = c0*c1;
    }

    *this = (*this)*rot;

}

bool KModelMatrix::isIdentity() const
{
    for (int i = 0; i < MODEL_MATRIX_LENGTH; ++i)
    {
        if (0==i || 5==i || 10==i || 15==i)
        {
            if (!FLOAT_EQUAL(_v[i], 1.0))
                return false;
        }
        else if (abs(_v[i]) > EPSILON_VAL_)
            return false;
    }

    return true;
}
