#ifndef BASE_QUADRANGLE_H
#define BASE_QUADRANGLE_H
#include "Point3Dd.h"
#pragma once

namespace BaseType{

class Quadrangle
{
public:
   Quadrangle(void);
   Quadrangle(const Point3Dd& p1, const Point3Dd& p2, const Point3Dd& p3, const Point3Dd& p4);
   ~Quadrangle(void);
private:
   Point3Dd _vp[4];
};

} // namespace
#endif