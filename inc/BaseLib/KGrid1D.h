#ifndef BASELIB_KGRID1D_H
#define BASELIB_KGRID1D_H

#include "util.h"
#include "defined.h"
#include "assert.h"
namespace BaseType{



template<class T>
class Grid1DItem{
   
public:
   Grid1DItem():
      _nElem(0),
      _sizeElem(0),
      _elems(0)
   {}
   ~Grid1DItem(){
      if(_elems){
         Util::k_free((void*)_elems);
         _elems = 0;
      }
   }
   void addElement(T* elem){
      assert(_nElem < 65530);
      if(0 == _elems){
         _sizeElem = 4;
         _elems = (T**)Util::k_malloc(sizeof(T*)*_sizeElem);
      }
      else if(_sizeElem <= _nElem){
         _sizeElem += 4;
         _elems = (T**)Util::k_realloc((void*)_elems,sizeof(T*)*_sizeElem);
      }
      _elems[_nElem] = elem;
      _nElem++;
   }

   T* getElement(const unsigned i){
      if(i >= 0 && i < _nElem){
         return _elems[i];
      }
      return 0;
   }
   T** getElements(){ return _elems;}
   unsigned short getNumberOfElement(){ return _nElem;};

private:
   unsigned short _nElem:16;
   unsigned short _sizeElem:16;
   T** _elems;
};

template<class T>
class KGrid1D{
public:
   KGrid1D():
      _nItem(0),
      _items(0),
      _mincoord(0.0),
      _maxcoord(0.0),
      _resol(0.0)
   {}
   KGrid1D(const double& mincoord, const double& maxcoord):
      _nItem(0),
      _items(0),
      _mincoord(mincoord),
      _maxcoord(maxcoord),
      _resol(0.0)
   {}
   ~KGrid1D(){
      if(_items){
         for(unsigned i = 0; i < _nItem; ++i){
            delete _items[i];
         }
         Util::k_free((void*)_items);
         _items = 0;
      }
   }

   void setNumberOfItem(unsigned int n){_nItem = n;};
   void setMinCoord(const double& mincoord){ _mincoord = mincoord;};
   double getMinCoord(){ return _mincoord;};
   void setMaxCoord(const double& maxcoord){ _maxcoord = maxcoord;};
   double getMaxCoord(){ return _maxcoord;};
   void setResolution(const double& resol){ _resol = resol;};
   void createGrid(){
      if(_resol > EPSILON_VAL_BIG){
         _nItem = static_cast<unsigned short>((_maxcoord - _mincoord)/_resol);
         _resol = (_maxcoord - _mincoord)/static_cast<double>(_nItem);
      }
      else if(_nItem > 0){
         _resol = (_maxcoord - _mincoord)/static_cast<double>(_nItem);
      }
      else{
         return;
      }
      _items = (Grid1DItem<T>**)Util::k_malloc(sizeof(Grid1DItem<T>*)*_nItem);
      for(unsigned i = 0; i < _nItem; ++i){
         _items[i] = new Grid1DItem<T>();
      }
   };
   void addElement(T* elem, const double& mincoord, const double& maxcoord);
   int getElementAtCoord(const double& coord, int& nElem, T*** elems);
   double getResolution(){ return _resol;};
private:
   unsigned short _nItem;
   Grid1DItem<T>** _items;
   double _mincoord;
   double _maxcoord;
   double _resol;

};

template<class T>
void KGrid1D<T>::addElement(T* elem, const double& mincoord, const double& maxcoord)
{
   int imin = static_cast<int>((mincoord - _mincoord)/_resol);
   int imax = static_cast<int>((maxcoord - _mincoord)/_resol);
   assert(imin < _nItem);
   assert(imax < _nItem);
   for(int i = imin; i <= imax; ++i){
      _items[i]->addElement(elem);
   }
}

template<class T>
int KGrid1D<T>::getElementAtCoord(const double& coord, int& nElem, T*** elems)
{
   int ind = static_cast<int>((coord - _mincoord)/_resol);
   if(ind >= _nItem){
      ind = _nItem - 1;
   }
   else if(ind < 0){
      ind = 0;
   }
   *elems = _items[ind]->getElements();
   nElem = _items[ind]->getNumberOfElement();
   return ind;
}

} // namespace
#endif