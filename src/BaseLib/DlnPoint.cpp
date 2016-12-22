#include "DlnPoint.h"
#include "Point2D.h"
#include "DlnEdge.h"
#include "util.h"
#include "assert.h"

namespace BaseType{

DlnPoint::DlnPoint(void):
_isRemoved(0),
_nEdge(0),
_sizeEdge(0),
_id(0),
_coord(0),
_edges(0)
{
}

DlnPoint::DlnPoint(Point2Dd* coord):
_isRemoved(0),
_nEdge(0),
_sizeEdge(0),
_id(0),
_coord(coord),
_edges(0)
{
}

DlnPoint::~DlnPoint(void)
{
#ifndef USING_OFFSET_CONTOUR_TO_GEN_CENTERLINE
   delete _coord;
#endif
}

Point2Dd* DlnPoint::getCoord()
{
   return _coord;
}

int DlnPoint::setCoord(Point2Dd* coord)
{
   _coord = coord;
   return 0;
}

//bool SkePoint::isBigAngle()
//{
//   return (_isBig == 1) ? true:false;
//}
//
//int SkePoint::setBigAngle(bool isbig)
//{
//   _isBig = (isbig == true? 1:0);
//   return 0;
//}


bool DlnPoint::isRemoved()
{
   return (_isRemoved == 1) ? true:false;
}

int DlnPoint::setAsRemoved()
{
   _isRemoved = 1;
   return 0;
}

//double SkePoint::getAngle()
//{
//   return _angle;
//}
//int SkePoint::setAngle(const double& angle)
//{
//   _angle = angle;
//   return 0;
//}
/*remove edge from vertex */
bool DlnPoint::removeEdge(DlnEdge* ed)
{
	for (size_t i = 0; i < _nEdge; ++i)
	{
		if (_edges[i] == ed)
		{
			_edges[i] = _edges[_nEdge - 1];
			_edges[_nEdge - 1] = nullptr;
			_nEdge--;
			return true;
		}
	}
	return false;
}

int DlnPoint::addEdge(DlnEdge* ed)
{
   if(0 == _edges){
      _sizeEdge = 2;
      _edges = (DlnEdge**)BaseType::Util::k_malloc(sizeof(DlnEdge*)*2);
   }
   else if(_sizeEdge <= _nEdge){
      _sizeEdge += 2;
      _edges = (DlnEdge**)BaseType::Util::k_realloc((void*)_edges,
         sizeof(DlnEdge*)*_sizeEdge);
   }
   _edges[_nEdge] = ed;
   _nEdge++;
   return _nEdge;
}

bool DlnPoint::isCollisionWithAabb2D(const double ov[2], const double& lv)
{
   return _coord->isCollisionWithAabb2D(ov,lv);
}

DlnEdge* DlnPoint::findSkeEdge(DlnPoint* otherpoint){
   for(unsigned i = 0; i < _nEdge; ++i){
      DlnEdge* ed = _edges[i];
      if((ed->getPoint(0) == this && ed->getPoint(1) == otherpoint) ||
         (ed->getPoint(1) == this && ed->getPoint(0) == otherpoint)){
            return ed;
      }
   }
   return 0;
}

}// namespace