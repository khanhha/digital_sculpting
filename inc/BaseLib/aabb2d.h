#ifndef BASELIB_AABB2D_H
#define BASELIB_AABB2D_H
#include "util.h"
#include "defined.h"

namespace BaseType{

template<class T>
class Aabb2D{
public:
   Aabb2D():
   _isLeaf(1),
   _nElem(0),
   _sizeElem(0),
   _elems(0){
      _sub[0][0] = 0;
      _sub[0][1] = 0;
      _sub[1][0] = 0;
      _sub[1][1] = 0;
   };
   virtual ~Aabb2D(){
      if(_elems){
         Util::k_free((void*)_elems);
      }
      _nElem = 0;
      _sizeElem = 0;
      if(_sub[0][0]){
         delete _sub[0][0];
      }
      if(_sub[0][1]){
         delete _sub[0][1];
      }
      if(_sub[1][0]){
         delete _sub[1][0];
      }
      if(_sub[1][1]){
         delete _sub[1][1];
      }
   };

   unsigned getNumberOfElement(){
      return _nElem;
   };

   T* getElement(unsigned short i){
      if(i < _nElem){
         return _elems[i];
      }
      return 0;
   };

   int addElementToLeaf(T* elem);

   virtual int addElement(unsigned resol, const double* ov, const double& lv, 
      T* elem, int& depth);
   virtual int splitNode(const double* ov, const double& lv);
   int getElementOfNodeCollisionWithSegment(const double* ov, const double& lv,
      const double* sp, const double* ep, T*** elems, unsigned& n);
   int getElementOfNodeCollisionWithLine(const double* ov, const double& lv,
      const double* p, const double* dv, T*** elems, unsigned& n);
   int getElementOfNodeCollisionWithPoint(const double* ov, const double& lv,
      const double* p, T*** elems, unsigned& n);
   int getElementOfNodeCollisionWithCircle(const double* ov, const double& lv,
      const double* cen, const double& r, T*** elems, unsigned& n);
protected:
   unsigned short _isLeaf:1;
   unsigned short _nElem:8;
   unsigned short _sizeElem:8;
   Aabb2D* _sub[2][2];
   T** _elems;
};

template<class T>
int Aabb2D<T>::addElementToLeaf(T* elem){
   if(0 == _elems){
      _sizeElem = 5;
      _elems = (T**)Util::k_malloc(sizeof(T*)*_sizeElem);
   }
   else if(_sizeElem <= _nElem){
      _sizeElem += 4;
      _elems = (T**)Util::k_realloc((void*)_elems,sizeof(T*)*_sizeElem);
   }
   _elems[_nElem] = elem;
   _nElem++;

   return 1;
};


template<class T>
int Aabb2D<T>::addElement(unsigned resol, const double* ov, const double& lv, 
   T* elem, int& depth){
      depth++;
   int iret = 0;
   if(_isLeaf){
      iret = addElementToLeaf(elem);
      if(_nElem >  resol && depth < 10){
         splitNode(ov,lv);
      }
      return iret;
   }

   double ov1[2];
   double lv1 = lv/2.0;
   for(int ix = 0; ix < 2; ++ix){
      ov1[0] = ov[0] + ix*lv1;
      for(int iy = 0; iy < 2; ++iy){
         ov1[1] = ov[1] + iy*lv1;
         if(elem->isCollisionWithAabb2D(ov1,lv1)){
            if(0 == _sub[ix][iy]){
               _sub[ix][iy] = new Aabb2D<T>();
            }
            iret += _sub[ix][iy]->addElement(resol,ov1,lv1,elem,depth);
            depth--;
         }
      }
   }
   return iret;
}

template<class T> 
int Aabb2D<T>::splitNode(const double* ov, const double& lv){
   double ov1[2];
   double lv1 = lv/2.0;
   for(unsigned i = 0; i < _nElem; ++i){
      T* elem = _elems[i];
      ov1[0] = ov[0];
      ov1[1] = ov[1];
      if(elem->isCollisionWithAabb2D(ov1,lv1)){
         Aabb2D* sub = _sub[0][0];
         if(0 == sub){
            sub = new Aabb2D<T>();
            _sub[0][0] = sub;
         }
         sub->addElementToLeaf(elem);
      }

      ov1[1] += lv1;
      if(elem->isCollisionWithAabb2D(ov1,lv1)){
         Aabb2D* sub = _sub[0][1];
         if(0 == sub){
            sub = new Aabb2D<T>();
            _sub[0][1] = sub;
         }
         sub->addElementToLeaf(elem);
      }

      ov1[0] += lv1;
      ov1[1] = ov[1];
      if(elem->isCollisionWithAabb2D(ov1,lv1)){
         Aabb2D* sub = _sub[1][0];
         if(0 == sub){
            sub = new Aabb2D<T>();
            _sub[1][0] = sub;
         }
         sub->addElementToLeaf(elem);
      }

      ov1[1] += lv1;
      if(elem->isCollisionWithAabb2D(ov1,lv1)){
         Aabb2D* sub = _sub[1][1];
         if(0 == sub){
            sub = new Aabb2D<T>();
            _sub[1][1] = sub;
         }
         sub->addElementToLeaf(elem);
      }
   } // for i
   Util::k_free((void*)_elems);
   _elems = 0;
   _nElem = 0;
   _sizeElem = 0;
   _isLeaf = 0;
   return 0;
}

template<class T>
int Aabb2D<T>::getElementOfNodeCollisionWithSegment(const double* ov, const double& lv,
      const double* sp, const double* ep, T*** elems, unsigned& n)
{
   if(_isLeaf){
      if(_nElem > 0){
         T** temp;
         if (0 == n){
            temp = (T**)Util::k_malloc(sizeof(T*)*_nElem);
         }
         else{
            temp = *elems;
            temp = (T**)Util::k_realloc((void*)(temp),sizeof(T*)*(n + _nElem));
         }
         
         for(unsigned i = 0; i < _nElem; ++i){
            temp[n] = _elems[i];
            n++;
         }
         *elems = temp;
      }
      return 0;
   }

   double ov1[2];
   double lv1 = lv/2.0;
   for(int ix = 0; ix < 2; ++ix){
      ov1[0] = ov[0] + ix*lv1;
      for(int iy = 0; iy < 2; ++iy){
         ov1[1] = ov[1] + iy*lv1;
         Aabb2D<T>* node = _sub[ix][iy];
         if(node){
            if(Util::is2DSegmentAabbCollision(sp,ep,ov1,lv1)){
               node->getElementOfNodeCollisionWithSegment(ov1,lv1,sp,ep,elems,n);
            }
         }
      }//iy
   }
   return 0;
}

template<class T>
int Aabb2D<T>::getElementOfNodeCollisionWithLine(const double* ov, const double& lv,
      const double* p, const double* dv, T*** elems, unsigned& n)
{
   if(_isLeaf){
      if(_nElem > 0){
         T** temp;
         if (0 == n){
            temp = (T**)Util::k_malloc(sizeof(T*)*_nElem);
         }
         else{
            temp = *elems;
            temp = (T**)Util::k_realloc((void*)(temp),sizeof(T*)*(n + _nElem));
         }
         
         for(unsigned i = 0; i < _nElem; ++i){
            temp[n] = _elems[i];
            n++;
         }
         *elems = temp;
      }
      return 0;
   }

   double ov1[2];
   double lv1 = lv/2.0;
   for(int ix = 0; ix < 2; ++ix){
      ov1[0] = ov[0] + ix*lv1;
      for(int iy = 0; iy < 2; ++iy){
         ov1[1] = ov[1] + iy*lv1;
         Aabb2D<T>* node = _sub[ix][iy];
         if(node){
            if(Util::is2DLineAabbCollision(p,dv,ov1,lv1)){
               node->getElementOfNodeCollisionWithLine(ov1,lv1,p,dv,elems,n);
            }
         }
      }//iy
   }
   return 0;
}

template<class T>
int Aabb2D<T>::getElementOfNodeCollisionWithPoint(const double* ov, const double& lv,
      const double* p, T*** elems, unsigned& n)
{
   if(_isLeaf){
      if(_nElem > 0){
         T** temp;
         if (0 == n){
            temp = (T**)Util::k_malloc(sizeof(T*)*_nElem);
         }
         else{
            temp = *elems;
            temp = (T**)Util::k_realloc((void*)(temp),sizeof(T*)*(n + _nElem));
         }
         
         for(unsigned i = 0; i < _nElem; ++i){
            temp[n] = _elems[i];
            n++;
         }
         *elems = temp;
      }
      return 0;
   }

   double ov1[2];
   double lv1 = lv/2.0;
   for(int ix = 0; ix < 2; ++ix){
      ov1[0] = ov[0] + ix*lv1;
      for(int iy = 0; iy < 2; ++iy){
         ov1[1] = ov[1] + iy*lv1;
         Aabb2D<T>* node = _sub[ix][iy];
         if(node){
            if(p[0] > (ov1[0] - EPSILON_VAL_) &&
               p[1] > (ov1[1] - EPSILON_VAL_) &&
               p[0] < (ov1[0] + lv1 + EPSILON_VAL_) &&
               p[1] < (ov1[1] + lv1 + EPSILON_VAL_)){
               node->getElementOfNodeCollisionWithPoint(ov1,lv1,p,elems,n);
            }
         }
      }//iy
   }
   return 0;
}

/* @param cen - center of cirlce
   @param r - radius of circle
*/
template<class T>
int Aabb2D<T>::getElementOfNodeCollisionWithCircle(const double* ov, const double& lv,
      const double* cen, const double& r, T*** elems, unsigned& n)
{
   if(_isLeaf){
      if(_nElem > 0){
         T** temp;
         if (0 == n){
            temp = (T**)Util::k_malloc(sizeof(T*)*_nElem);
         }
         else{
            temp = *elems;
            temp = (T**)Util::k_realloc((void*)(temp),sizeof(T*)*(n + _nElem));
         }
         
         for(unsigned i = 0; i < _nElem; ++i){
            temp[n] = _elems[i];
            n++;
         }
         *elems = temp;
      }
      return 0;
   }

   double ov1[2];
   double lv1 = lv/2.0;
   for(int ix = 0; ix < 2; ++ix){
      ov1[0] = ov[0] + ix*lv1;
      for(int iy = 0; iy < 2; ++iy){
         ov1[1] = ov[1] + iy*lv1;
         Aabb2D<T>* node = _sub[ix][iy];
         if(node){
            if(cen[0] > (ov1[0] - r - EPSILON_VAL_) &&
               cen[1] > (ov1[1]- r - EPSILON_VAL_) &&
               cen[0] < (ov1[0] + lv1 + r + EPSILON_VAL_) &&
               cen[1] < (ov1[1] + lv1 + r + EPSILON_VAL_)){
               node->getElementOfNodeCollisionWithCircle(ov1,lv1,cen,r,elems,n);
            }
         }
      }//iy
   }
   return 0;
}


}// namespace

#endif