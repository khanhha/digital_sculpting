#include "DlnPoint.h"
#include "DlnEdge.h"
#include "DlnTriangle.h"
#include "Point2D.h"
#include "assert.h"
#include <float.h>
#include <iostream>

namespace BaseType{

DlnEdge::DlnEdge(void):
_isChecked(false)
{
   _vps[0] = _vps[1] = 0;
   _trigs[0] = _trigs[1] = 0;
}

DlnEdge::DlnEdge(DlnPoint* p1, DlnPoint* p2):
_isChecked(false)
{
   _vps[0] = p1;
   _vps[1] = p2;
   _trigs[0] = _trigs[1] = 0;
}

DlnEdge::~DlnEdge(void)
{
}

bool DlnEdge::isChecked()
{
   return _isChecked;
}

int DlnEdge::setChecked(const bool flag)
{
   _isChecked = flag;
   return 0;
}

DlnPoint* DlnEdge::getPoint(int i)
{
   if(0 == i){
      return _vps[0];
   }
   else if(1 == i){
      return _vps[1];
   }
   return 0;
}

int DlnEdge::addTriangle(DlnTriangle* trig)
{
   if(0 == _trigs[0])
   {
      _trigs[0] = trig;
	  return 1;
   }
   else if(0 == _trigs[1])
   {
      _trigs[1] = trig;
	  return 1;
   }
   else
	  return 0;
}

bool DlnEdge::isNotBoundaryEdge()
{
   if(_trigs[1] && _trigs[0]){
      return true;
   }
   return false;
}
double DlnEdge::calc2DLength()
{
	double length2 = (_vps[0]->getCoord()->_vcoords[0] - _vps[1]->getCoord()->_vcoords[0])*
		(_vps[0]->getCoord()->_vcoords[0] - _vps[1]->getCoord()->_vcoords[0]) +
		(_vps[0]->getCoord()->_vcoords[1] - _vps[1]->getCoord()->_vcoords[1])*
		(_vps[0]->getCoord()->_vcoords[1] - _vps[1]->getCoord()->_vcoords[1]);

	if (length2 > DBL_EPSILON)
		return sqrt(length2);
	else
		return 0.0;
}

double DlnEdge::calc2DSquareLength()
{
   double length = (_vps[0]->getCoord()->_vcoords[0] - _vps[1]->getCoord()->_vcoords[0])*
      (_vps[0]->getCoord()->_vcoords[0] - _vps[1]->getCoord()->_vcoords[0]) +
      (_vps[0]->getCoord()->_vcoords[1] - _vps[1]->getCoord()->_vcoords[1])*
      (_vps[0]->getCoord()->_vcoords[1] - _vps[1]->getCoord()->_vcoords[1]);
   return length;
}

int DlnEdge::calc2DMidPoint(double* mid)
{
   mid[0] = _vps[0]->getCoord()->_vcoords[0] + _vps[1]->getCoord()->_vcoords[0];
   mid[1] = _vps[0]->getCoord()->_vcoords[1] + _vps[1]->getCoord()->_vcoords[1];
   mid[0] /= 2.0;
   mid[1] /= 2.0;
   return 0;
}

int DlnEdge::removeTriangle(const DlnTriangle* trig)
{
   if(trig == _trigs[0]){
      _trigs[0] = _trigs[1];
      _trigs[1] = 0;
	  return 1;
   }
   else if(trig == _trigs[1]){
      _trigs[1] = 0;
	  return 1;
   }
   return 0;
}

DlnTriangle* DlnEdge::getOtherTriangle(DlnTriangle* trig)
{
   if(0 == _trigs[1]){
      return 0;
   }
   if(trig == _trigs[0]){
      return _trigs[1];
   }
   else{
      //assert(trig == _trigs[1]);
      return _trigs[0];
   }
}

DlnTriangle* DlnEdge::getTriangle(size_t i)
{
	if (0 == i)
		return _trigs[0];
	else if (1 == i)
		return _trigs[1];
	else
		return nullptr;
}

bool DlnEdge::removePoint(DlnPoint* p)
{
	if (_vps[0] == p)
	{
		_vps[0] = nullptr;
		return true;
	}
	else if (_vps[1] == p)
	{
		_vps[1] = nullptr;
		return true;
	}
	else
	{
		return false;
	}

}

bool DlnEdge::addPoint(DlnPoint* p)
{
	if (_vps[0] == nullptr)
	{
		_vps[0] = p;
		return true;
	}
	else if (_vps[1] == nullptr)
	{
		_vps[1] = p;
		return true;
	}
	else
	{
		return false;
	}
}

void DlnEdge::removeFromPoint()
{
	for (size_t i = 0; i < 2; ++i)
	{
		if (_vps[i] != nullptr)
		{
			if (!_vps[i]->removeEdge(this))
			{
				assert(false);
			}
			_vps[i] = nullptr;
		}
	}
}

} // namespace