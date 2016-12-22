#include "matrix3x3d.h"


Matrix3x3d::Matrix3x3d(void)
{
   _v[0][0] = _v[1][1] = _v[2][2] = 1.0;
   _v[0][1] = _v[0][2] = _v[1][0] = _v[1][2] = _v[2][0] = _v[2][1] = 0.0;
}

Matrix3x3d::Matrix3x3d(const Matrix3x3d &m)
{
    _v[0][0] = m._v[0][0];
    _v[0][1] = m._v[0][1];
    _v[0][2] = m._v[0][2];
    _v[1][0] = m._v[1][0];
    _v[1][1] = m._v[1][1];
    _v[1][2] = m._v[1][2];
    _v[2][0] = m._v[2][0];
    _v[2][1] = m._v[2][1];
    _v[2][2] = m._v[2][2];
}

Matrix3x3d::~Matrix3x3d(void)
{
}
void Matrix3x3d::setIdentity()
{
    _v[0][0] = _v[1][1] = _v[2][2] = 1.0;
    _v[0][1] = _v[0][2] = _v[1][0] = _v[1][2] = _v[2][0] = _v[2][1] = 0.0;
}


Matrix3x3d Matrix3x3d::operator*(Matrix3x3d& other) 
{
   Matrix3x3d result;

   result._v[0][0] = _v[0][0]*other._v[0][0] + _v[0][1]*other._v[1][0] + _v[0][2]*other._v[2][0];
   result._v[0][1] = _v[0][0]*other._v[0][1] + _v[0][1]*other._v[1][1] + _v[0][2]*other._v[2][1];
   result._v[0][2] = _v[0][0]*other._v[0][2] + _v[0][1]*other._v[1][2] + _v[0][2]*other._v[2][2];
   result._v[1][0] = _v[1][0]*other._v[0][0] + _v[1][1]*other._v[1][0] + _v[1][2]*other._v[2][0];
   result._v[1][1] = _v[1][0]*other._v[0][1] + _v[1][1]*other._v[1][1] + _v[1][2]*other._v[2][1];
   result._v[1][2] = _v[1][0]*other._v[0][2] + _v[1][1]*other._v[1][2] + _v[1][2]*other._v[2][2];
   result._v[2][0] = _v[2][0]*other._v[0][0] + _v[2][1]*other._v[1][0] + _v[2][2]*other._v[2][0];
   result._v[2][1] = _v[2][0]*other._v[0][1] + _v[2][1]*other._v[1][1] + _v[2][2]*other._v[2][1];
   result._v[2][2] = _v[2][0]*other._v[0][2] + _v[2][1]*other._v[1][2] + _v[2][2]*other._v[2][2];

   return result;
}
Matrix3x3d Matrix3x3d::calRotateMatrixRevert()
{
    Vector3Dd e1 = getColumn(0);
    Vector3Dd e2 = getColumn(1);
    Vector3Dd e3 = getColumn(2);
    Vector3Dd x1 (1,0,0);
    Vector3Dd y2 (0,1,0);
    Vector3Dd z3 (0,0,1);
    Vector3Dd axis = e3*z3;
    axis.unit();
    if (e3 == z3 || e3 == -z3)
        axis = e1;
    Matrix3x3d rot, rot1, rot2;
    double angl = e3.angle(z3);

    rot1.makeRotationAboutAxis(axis, angl);
    e1.multiWithMatrix(rot1);
    e2.multiWithMatrix(rot1);
    e3.multiWithMatrix(rot1);
    e1.unit();
    e2.unit();
    e3.unit();
    angl = e1.angle(x1);
    axis = e1*x1;
    axis.unit();
    if (e1 == x1 || e1 == -x1)
        axis = e3;

    rot2.makeRotationAboutAxis(axis, angl);
    rot = rot2*rot1;
    return rot;
}
Matrix3x3d Matrix3x3d::calRotateMatrixRevert2()
{
    Vector3Dd x1 = getColumn(0);
    Vector3Dd y2 = getColumn(1);
    Vector3Dd z3 = getColumn(2);
    Vector3Dd e1(1, 0, 0);
    Vector3Dd e2(0, 1, 0);
    Vector3Dd e3(0, 0, 1);
    Vector3Dd axis = e3*z3;
    axis.unit();
    if (e3 == z3 || e3 == -z3)
        axis = e1;
    Matrix3x3d rot, rot1, rot2;
    double angl = e3.angle(z3);

    rot1.makeRotationAboutAxis(axis, angl);
    e1.multiWithMatrix(rot1);
    e2.multiWithMatrix(rot1);
    e3.multiWithMatrix(rot1);
    e1.unit();
    e2.unit();
    e3.unit();
    angl = e1.angle(x1);
    axis = e1*x1;
    axis.unit();
    if (e1 == x1 || e1 == -x1)
        axis = e3;

    rot2.makeRotationAboutAxis(axis, angl);
    rot = rot2*rot1;
    return rot;
}
Matrix3x3d Matrix3x3d::calRotateMatrixRevert(Matrix3x3d& other)
{
    Vector3Dd e1 = getColumn(0);
    Vector3Dd e2 = getColumn(1);
    Vector3Dd e3 = getColumn(2);
    Vector3Dd x1 = other.getColumn(0);
    Vector3Dd y2 = other.getColumn(1);
    Vector3Dd z3 = other.getColumn(2);
    Vector3Dd axis = e3*z3;
    axis.unit();
    if (e3 == z3 || e3 == -z3)
        axis = e1;
    Matrix3x3d rot, rot1, rot2;
    double angl = e3.angle(z3);

    rot1.makeRotationAboutAxis(axis, angl);
    e1.multiWithMatrix(rot1);
    e2.multiWithMatrix(rot1);
    e3.multiWithMatrix(rot1);
    e1.unit();
    e2.unit();
    e3.unit();
    angl = e1.angle(x1);
    axis = e1*x1;
    axis.unit();
    if (e1 == x1 || e1 == -x1)
        axis = e3;

    rot2.makeRotationAboutAxis(axis, angl);
    rot = rot2*rot1;
    return rot;
}
const Point3Dd Matrix3x3d::operator*(const Point3Dd& p) const
{
    Point3Dd out;
    out[0] = _v[0][0] * p[0] + _v[0][1] * p[1] + _v[0][2] * p[2];
    out[1] = _v[1][0] * p[0] + _v[1][1] * p[1] + _v[1][2] * p[2];
    out[2] = _v[2][0] * p[0] + _v[2][1] * p[1] + _v[2][2] * p[2];
    return out;
}

const Matrix3x3d Matrix3x3d::operator*(double d) const
{
    Matrix3x3d result;
    for (unsigned i = 0; i < 3; ++i)
        for (unsigned j = 0; j < 3; j++)
        {
            result._v[i][j] = _v[i][j] * d;
        }
    return result;
}

const Matrix3x3d Matrix3x3d::operator/(double d) const
{
    Matrix3x3d result;
    for (unsigned i = 0; i < 3; ++i)
        for (unsigned j = 0; j < 3; j++)
        {
        result._v[i][j] = _v[i][j] / d;
        }
    return result;
}


void Matrix3x3d::makeRotationAboutAxis(Point3Dd& a, double angle)
{
    double c = cos(angle);
    double s = sin(angle);
    double t = 1 - c;

    _v[0][0] = t*a.x * a.x + c;
    _v[0][1] = t *a.x * a.y - s*a.z;
    _v[0][2] = t * a.x * a.z + s * a.y;

    _v[1][0] = t*a.x*a.y + s*a.z;
    _v[1][1] = t*a.y*a.y + c;
    _v[1][2] = t*a.y*a.z - s*a.x;

    _v[2][0] = t*a.x*a.z - s*a.y;
    _v[2][1] = t*a.y*a.z + s*a.x;
    _v[2][2] = t*a.z*a.z + c;

    return;
}

Vector3Dd Matrix3x3d::getRow(unsigned i) const
{
    return Vector3Dd(_v[i][0], _v[i][1], _v[i][2]);
}

Vector3Dd Matrix3x3d::getColumn(unsigned i) const
{
    return Vector3Dd(_v[0][i], _v[1][i], _v[2][i]);
}
    
void Matrix3x3d::unit()
{
    Vector3Dd n = getColumn(0);
    n.unit();
    _v[0][0] = n[0];
    _v[1][0] = n[1];
    _v[2][0] = n[2];

    n = getColumn(1);
    n.unit();
    _v[0][1] = n[0];
    _v[1][1] = n[1];
    _v[2][1] = n[2];

    n = getColumn(2);
    n.unit();
    _v[0][2] = n[0];
    _v[1][2] = n[1];
    _v[2][2] = n[2];
}



void Matrix3x3d::setColumn(unsigned i, const Vector3Dd &v)
{
    _v[0][i] = v.x;
    _v[1][i] = v.y;
    _v[2][i] = v.z;
}

void Matrix3x3d::setRow(unsigned i, const Vector3Dd &v)
{
    _v[i][0] = v.x;
    _v[i][1] = v.y;
    _v[i][2] = v.z;
}

Matrix3x3d Matrix3x3d::inverse() const
{
    Matrix3x3d result;

    Vector3Dd x0(_v[0][0], _v[1][0], _v[2][0]);
    Vector3Dd x1(_v[0][1], _v[1][1], _v[2][1]);
    Vector3Dd x2(_v[0][2], _v[1][2], _v[2][2]);

    double det = x0.dot(x1.cross(x2));
    assert(!FLOAT_EQUAL(det,0.0));
    if (det == 0)
        return result;

    result.setRow(0, x1.cross(x2));
    result.setRow(1, x2.cross(x0));
    result.setRow(2, x0.cross(x1));

    result = result / det;

    return result;
}