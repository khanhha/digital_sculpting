#include "Quadrangle.h"

namespace BaseType{

Quadrangle::Quadrangle(void)
{
}

Quadrangle::Quadrangle(const Point3Dd& p1, const Point3Dd& p2,
   const Point3Dd& p3, const Point3Dd& p4)
{
   _vp[0] = p1;
   _vp[1] = p2;
   _vp[2] = p3;
   _vp[3] = p4;
}

Quadrangle::~Quadrangle(void)
{
}

}// namespace