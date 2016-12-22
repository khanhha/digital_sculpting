#include "Segment2Df.h"
#include "Point2Df.h"

Segment2Df::Segment2Df(void):
_sp(0),
_ep(0)
{
}


Segment2Df::Segment2Df(Point2Df* sp, Point2Df* ep):
_sp(sp),
_ep(ep)
{}


Segment2Df::~Segment2Df(void)
{
   if(_sp){
      delete _sp;
   }
   if(_ep){
      delete _ep;
   }
}
