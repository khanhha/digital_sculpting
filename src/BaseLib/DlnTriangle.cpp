#include "DlnPoint.h"
#include "DlnEdge.h"
#include "DlnTriangle.h"
#include "Point2D.h"
#include "assert.h"
#include <iostream>

namespace BaseType{

DlnTriangle::DlnTriangle(void):
_isChecked(0),
_skeTrigType(0)
{
   _edges[0] = _edges[1] = _edges[2] = 0;
   _points[0] = _points[1] = _points[2] = 0;
#ifdef _DEBUG
 SkeTriangle_filter_count = 0;
#endif
}

DlnTriangle::DlnTriangle(DlnPoint* p0, DlnPoint* p1, DlnPoint* p2):
_isChecked(0),
_skeTrigType(0)
{
   _edges[0] = _edges[1] = _edges[2] = 0;
   _points[0] = p0;
   _points[1] = p1;
   _points[2] = p2;
#ifdef _DEBUG
 SkeTriangle_filter_count = 0;
#endif
}

DlnTriangle::~DlnTriangle(void)
{
}

DlnEdge* DlnTriangle::getEdge(const unsigned i)
{
	assert(i >= 0 && i < 3);
	return _edges[i];
}

int DlnTriangle::setEdge(const int i, DlnEdge* ed)
{
   _edges[i] = ed;
   return 0;
}

bool DlnTriangle::removeEdge(DlnEdge* e)
{
	for (size_t i = 0; i < 3;++i)
	{
		if (_edges[i] == e)
		{
			_edges[i] = nullptr;
			return true;
		}
	}
	return false;
}

void DlnTriangle::removeFromEdge()
{
	for (size_t i = 0; i < 3; ++i)
	{
		if (_edges[i] != nullptr)
		{
			if (!_edges[i]->removeTriangle(this))
			{
				std::cout << "error";
			}
			_edges[i] = nullptr;
		}
	}
}

bool DlnTriangle::addEdge(DlnEdge* e)
{
	for (size_t i = 0; i < 3; ++i)
	{
		if (_edges[i] == nullptr)
		{
			_edges[i] = e;
			return true;
		}
	}
	return false;
}

bool DlnTriangle::isExistEdge(const int &i)
{
    return ((1 == int(_edges[i])) ? true : false);
}


Point2Dd* DlnTriangle::getCoord(int i)
{
   return _points[i]->getCoord();
}

int DlnTriangle::calc2DcenterPoint(double* cen)
{
   cen[0] = _points[0]->getCoord()->_vcoords[0] + _points[1]->getCoord()->_vcoords[0] + _points[2]->getCoord()->_vcoords[0];
   cen[1] = _points[0]->getCoord()->_vcoords[1] + _points[1]->getCoord()->_vcoords[1] + _points[2]->getCoord()->_vcoords[1];
   cen[0] /= 3.0;
   cen[1] /= 3.0;
   return 0;
}

void DlnTriangle::recalcType()
{
	_skeTrigType = 0;
	checkType();
}

int DlnTriangle::checkType()
{
   if(_edges[0]->isNotBoundaryEdge()){
      _skeTrigType++;
   }
   if(_edges[1]->isNotBoundaryEdge()){
      _skeTrigType++;
   }
   if(_edges[2]->isNotBoundaryEdge()){
      _skeTrigType++;
   }
   return 0;
}

unsigned DlnTriangle::getType()
{
   if(0 == _skeTrigType){
      checkType();
   }
   return _skeTrigType;
}

bool DlnTriangle::isChecked()
{
   return (_isChecked == 1)?true:false;
}

int DlnTriangle::setChecked(bool flag)
{
   _isChecked = (flag == true)? 1:0;
   return 0;
}


int DlnTriangle::filter(const double& aveWidth2)
{
   DlnTriangle* other;
#ifdef _DEBUG
   SkeTriangle_filter_count++;
   for(int i = 0; i <3; ++i){
      other = _edges[i]->getOtherTriangle(this);
      if(0 == other){
         assert(other);
         continue;
      }
   }
#endif
   for(int i = 0; i <3; ++i){
      other = _edges[i]->getOtherTriangle(this);
      if(0 == other){
//         assert(other);
         continue;
      }
      if(1 == other->getType()){
         double coord[2];
         other->getTipVertexCoordOfType1Trig(coord);
         double cen[2];
         calc2DcenterPoint(cen);
         double dis2 = (coord[0] - cen[0])*(coord[0] - cen[0]) +
            (coord[1] - cen[1])*(coord[1] - cen[1]);
         if(dis2 < aveWidth2){
            other->setChecked(1);
            _edges[i]->removeTriangle(other);
            _skeTrigType--;
         }
      }
   }
   return 0;
}

int DlnTriangle::getTipVertexCoordOfType1Trig(double* coord)
{
# if 0 // using tip vertex
   for(int i = 0; i < 3; ++i){
      if(_edges[i]->isNotBoundaryEdge()){
         int j = (i + 2) % 3;
         coord[0] = _points[j]->getCoord()->v[0];
         coord[1] = _points[j]->getCoord()->v[1];
      }
   }
#else // using mid point of short edge
   for(int i = 0; i < 3; ++i){
      if(_edges[i]->isNotBoundaryEdge()){
         int j = (i + 1) % 3;
         double l1 = _edges[j]->calc2DSquareLength();
         int j1 = (i+2) % 3;
         double l2 = _edges[j1]->calc2DSquareLength();
         if(l1 < l2){
            _edges[j]->calc2DMidPoint(coord);
         }
         else{
            _edges[j1]->calc2DMidPoint(coord);
         }
      }
   }
#endif
   return 0;
}

DlnEdge* DlnTriangle::getSkeEdgeOfType1Trig()
{
   if(_skeTrigType != 1){
      return 0;
   }
   for(int i = 0;i < 3; ++i){
      if(_edges[i]->isNotBoundaryEdge()){
         return _edges[i];
      }
   }
   return 0;
}

DlnEdge* DlnTriangle::getUncheckedSkeEdge()
{
   for(int i = 0;i < 3; ++i){
      if(_edges[i]->isNotBoundaryEdge() &&!_edges[i]->isChecked()){
         return _edges[i];
      }
   }
   return 0;
}

DlnEdge* DlnTriangle::getOtherSkeEdge(DlnEdge* ed)
{
   for(int i = 0;i < 3; ++i){
      if(_edges[i]->isNotBoundaryEdge() && _edges[i] != ed){
         return _edges[i];
      }
   }
   return 0;
}

DlnPoint* DlnTriangle::getOtherPoint(DlnPoint* v0, DlnPoint* v1)
{
	for (size_t i = 0; i < 3; ++i)
	{
		if (_points[i] != v0 && _points[i] != v1)
		{
			return _points[i];
		}
	}
	return nullptr;
}

void DlnTriangle::removePoints()
{
	for (size_t i = 0; i < 3; ++i)
	{
		_points[i] = nullptr;
	}
}

void DlnTriangle::setEdges(DlnEdge* e0, DlnEdge* e1, DlnEdge* e2)
{
	_edges[0] = e0;
	_edges[1] = e1;
	_edges[2] = e2;

	if (!e0->addTriangle(this))
	{
		std::cout << "error";
		assert(false);
	}
	if (!e1->addTriangle(this))
	{
		std::cout << "error";
		assert(false);
	}
	if (!e2->addTriangle(this))
	{
		std::cout << "error";
		assert(false);
	}
}

void DlnTriangle::setPoints(DlnPoint* p0, DlnPoint* p1, DlnPoint* p2)
{
	_points[0] = p0;
	_points[1] = p1;
	_points[2] = p2;
}



}// namespace