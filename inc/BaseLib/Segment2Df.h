#ifndef BASE_SEGMENT2DF_H
#define BASE_SEGMENT2DF_H
#pragma once
#include "Point2D.h"

class Segment2Df
{
public:
   Point2Df* _sp;
   Point2Df* _ep;
public:

   Segment2Df(void);
   Segment2Df(Point2Df* sp, Point2Df* ep);
   ~Segment2Df(void);
};

#endif