#ifndef BASELIB_KSPHEREOCTREE_H
#define BASELIB_KSPHEREOCTREE_H

#include "util.h"
#include "defined.h"

namespace BaseType{
   template<class T1, class T2>
   class KSphereOctree{
   
   public:
      KSphereOctree():
         _isLeaf(1),
         _firstSize(0),
         _firstN(0),
         _secondSize(0),
         _secondN(0),
         _firstData(0),
         _secondData(0)
      {
         _subs[0][0][0] = 0;
         _subs[0][0][1] = 0;
         _subs[0][1][0] = 0;
         _subs[0][1][1] = 0;
         _subs[1][0][0] = 0;
         _subs[1][0][1] = 0;
         _subs[1][1][0] = 0;
         _subs[1][1][1] = 0;
      };
      virtual ~KSphereOctree(){
         if(_firstData){
            Util::k_free((void*)_firstData);
            _firstData = 0;
         }
         if(_secondData){
            Util::k_free((void*)_secondData);
            _secondData = 0;
         }
         for(int ix = 0; ix < 2; ++ix){
            for(int iy = 0; iy < 2; ++iy){
                for(int iz = 0; iz < 2; ++iz){
                   if(_subs[ix][iy][iz]){
                       delete _subs[ix][iy][iz];
                   }
                }
            }
        }
      };
      virtual KSphereOctree<T1,T2>* createNode(){ return new KSphereOctree<T1,T2>();};

      int addFirstElementToLeaf(T1* elem);
      int addFirstElement(unsigned resol, const double* ov, const double& lv, 
      T1* elem, int& depth, double distance = 0.0);
      int addSecondElementToLeaf(T2* elem);
      int addSecondElement(const double* ov, const double& lv, 
      T2* elem, int& depth, double distance = 0.0);
      int addSecondElement(unsigned resol, const double* ov, const double& lv, 
          T2* elem, int& depth, double distance = 0.0);
      int splitNodeSeparateFirstData(const double* ov, const double& lv, double distance = 0.0);
      int splitNodeSeparateSecondData(const double* ov, const double& lv, double distance = 0.0);
      KSphereOctree<T1,T2>* getSub(int ix,int iy,int iz){ return _subs[ix][iy][iz];}
   protected:
       unsigned char _isLeaf;
       unsigned short _firstSize;
       unsigned short _firstN;
       unsigned short _secondSize;
       unsigned short _secondN;
      /*unsigned short _isLeaf:1;
      unsigned short _firstSize:8;
      unsigned short _firstN:8;
      unsigned short _secondSize:8;
      unsigned short _secondN:7;*/
      T1** _firstData;
      T2** _secondData;
      KSphereOctree<T1,T2>* _subs[2][2][2];
   };



template<class T1,class T2>
int KSphereOctree<T1,T2>::addFirstElementToLeaf(T1* elem){   
   if(0 == _firstData){
      _firstSize = 5;
      _firstData = (T1**)Util::k_malloc(sizeof(T1*)*_firstSize);
      assert( _firstData && _firstN < _firstSize );
   }
   else if(_firstSize <= _firstN){      
      _firstSize += 4;
      _firstData = (T1**)Util::k_realloc((void*)_firstData,sizeof(T1*)*_firstSize);
      assert( _firstData && _firstN < _firstSize );
   }
   
   _firstData[_firstN] = elem;
   _firstN++;

   return 1;
};


template<class T1,class T2>
int KSphereOctree<T1,T2>::addFirstElement(unsigned resol, const double* ov, const double& lv, 
   T1* elem, int& depth, double distance){
      depth++;
   int iret = 0;
   if(_isLeaf){
      iret = addFirstElementToLeaf(elem);
      if(_firstN >  resol && depth < 10){
         splitNodeSeparateFirstData(ov,lv);
      }
      return iret;
   }

   double ov1[3];
   double lv1 = lv/2.0;
   for(int ix = 0; ix < 2; ++ix){
      ov1[0] = ov[0] + ix*lv1;
      for(int iy = 0; iy < 2; ++iy){
         ov1[1] = ov[1] + iy*lv1;
         for(int iz = 0; iz < 2; ++iz){
            ov1[2] = ov[2] + iz*lv1;
            if(elem->isCollisionWithAABB(ov1,lv1,distance)){
               if(0 == _subs[ix][iy][iz]){
                  _subs[ix][iy][iz] = createNode();
               }
               iret += _subs[ix][iy][iz]->addFirstElement(resol,ov1,lv1,elem,depth,distance);
               depth--;
            }
         }
      }
   }
   return iret;
}


template<class T1,class T2>
int KSphereOctree<T1,T2>::addSecondElementToLeaf(T2* elem){
   if(0 == _secondData){
      _secondSize = 5;
      _secondData = (T2**)Util::k_malloc(sizeof(T2*)*_secondSize);
   }
   else if(_secondSize <= _secondN){
      _secondSize += 4;
      _secondData = (T2**)Util::k_realloc((void*)_secondData,sizeof(T2*)*_secondSize);
   }
   _secondData[_secondN] = elem;
   _secondN++;

   return 1;
};


template<class T1,class T2>
int KSphereOctree<T1,T2>::addSecondElement(const double* ov, const double& lv, 
   T2* elem, int& depth, double distance){
      depth++;
   int iret = 0;
   if(_isLeaf){
      iret = addSecondElementToLeaf(elem);
      return iret;
   }

   double ov1[3];
   double lv1 = lv/2.0;
   for(int ix = 0; ix < 2; ++ix){
      ov1[0] = ov[0] + ix*lv1;
      for(int iy = 0; iy < 2; ++iy){
         ov1[1] = ov[1] + iy*lv1;
         for(int iz = 0; iz < 2; ++iz){
            ov1[2] = ov[2] + iz*lv1;
            if(_subs[ix][iy][iz] && elem->isCollisionWithAABB(ov1,lv1,distance)){
               iret += _subs[ix][iy][iz]->addSecondElement(ov1,lv1,elem,depth,distance);
               depth--;
            }
         }
      }
   }
   return iret;
}


template<class T1,class T2>
int KSphereOctree<T1,T2>::addSecondElement(unsigned resol, const double* ov, const double& lv, 
    T2* elem, int& depth, double distance){
        depth++;
        int iret = 0;
        if(_isLeaf){
            iret = addSecondElementToLeaf(elem);
            if(_secondN >  resol && depth < 10){
                splitNodeSeparateSecondData(ov,lv);
            }
            return iret;
        }

        double ov1[3];
        double lv1 = lv/2.0;
        for(int ix = 0; ix < 2; ++ix){
            ov1[0] = ov[0] + ix*lv1;
            for(int iy = 0; iy < 2; ++iy){
                ov1[1] = ov[1] + iy*lv1;
                for(int iz = 0; iz < 2; ++iz){
                    ov1[2] = ov[2] + iz*lv1;
                    if(elem->isCollisionWithAABB(ov1,lv1,distance)){
                        if(0 == _subs[ix][iy][iz]){
                            _subs[ix][iy][iz] = createNode();
                        }
                        iret += _subs[ix][iy][iz]->addSecondElement(resol,ov1,lv1,elem,depth,distance);
                        depth--;
                    }
                }
            }
        }
        return iret;
}

template<class T1,class T2> 
int KSphereOctree<T1,T2>::splitNodeSeparateFirstData(const double* ov, const double& lv, double distance){
   double ov1[3];
   double lv1 = lv/2.0;
   for(unsigned i = 0; i < _firstN; ++i){
      T1* elem = _firstData[i];
      ov1[0] = ov[0];
      ov1[1] = ov[1];
      ov1[2] = ov[2];
      if(elem->isCollisionWithAABB(ov1,lv1,distance)){
         KSphereOctree* sub = _subs[0][0][0];
         if(0 == sub){
            sub = createNode();
            _subs[0][0][0] = sub;
         }
         sub->addFirstElementToLeaf(elem);
      }

      ov1[2] += lv1;
      if(elem->isCollisionWithAABB(ov1,lv1,distance)){
         KSphereOctree* sub = _subs[0][0][1];
         if(0 == sub){
            sub = createNode();
            _subs[0][0][1] = sub;
         }
         sub->addFirstElementToLeaf(elem);
      }

      ov1[1] += lv1;
      ov1[2] = ov[2];
      if(elem->isCollisionWithAABB(ov1,lv1,distance)){
         KSphereOctree* sub = _subs[0][1][0];
         if(0 == sub){
            sub = createNode();
            _subs[0][1][0] = sub;
         }
         sub->addFirstElementToLeaf(elem);
      }

      ov1[2] += lv1;
      if(elem->isCollisionWithAABB(ov1,lv1,distance)){
         KSphereOctree* sub = _subs[0][1][1];
         if(0 == sub){
            sub = createNode();
            _subs[0][1][1] = sub;
         }
         sub->addFirstElementToLeaf(elem);
      }

      ov1[0] = ov[0] + lv1;
      ov1[1] = ov[1];
      ov1[2] = ov[2];
      if(elem->isCollisionWithAABB(ov1,lv1,distance)){
         KSphereOctree* sub = _subs[1][0][0];
         if(0 == sub){
            sub = createNode();
            _subs[1][0][0] = sub;
         }
         sub->addFirstElementToLeaf(elem);
      }

      ov1[2] += lv1;
      if(elem->isCollisionWithAABB(ov1,lv1,distance)){
         KSphereOctree* sub = _subs[1][0][1];
         if(0 == sub){
            sub = createNode();
            _subs[1][0][1] = sub;
         }
         sub->addFirstElementToLeaf(elem);
      }

      ov1[1] += lv1;
      ov1[2] = ov[2];
      if(elem->isCollisionWithAABB(ov1,lv1,distance)){
         KSphereOctree* sub = _subs[1][1][0];
         if(0 == sub){
            sub = createNode();
            _subs[1][1][0] = sub;
         }
         sub->addFirstElementToLeaf(elem);
      }

      ov1[2] += lv1;
      if(elem->isCollisionWithAABB(ov1,lv1,distance)){
         KSphereOctree* sub = _subs[1][1][1];
         if(0 == sub){
            sub = createNode();
            _subs[1][1][1] = sub;
         }
         sub->addFirstElementToLeaf(elem);
      }
   } // for i

   

   Util::k_free((void*)_firstData);
   _firstData = 0;
   _firstN = 0;
   _firstSize = 0;
   _isLeaf = 0;
   return 0;
}


template<class T1,class T2> 
int KSphereOctree<T1,T2>::splitNodeSeparateSecondData(const double* ov, const double& lv, double distance){
    double ov1[3];
    double lv1 = lv/2.0;
    for(unsigned i = 0; i < _secondN; ++i){
        T2* elem = _secondData[i];
        ov1[0] = ov[0];
        ov1[1] = ov[1];
        ov1[2] = ov[2];
        if(elem->isCollisionWithAABB(ov1,lv1,distance)){
            KSphereOctree* sub = _subs[0][0][0];
            if(0 == sub){
                sub = createNode();
                _subs[0][0][0] = sub;
            }
            sub->addSecondElementToLeaf(elem);
        }

        ov1[2] += lv1;
        if(elem->isCollisionWithAABB(ov1,lv1,distance)){
            KSphereOctree* sub = _subs[0][0][1];
            if(0 == sub){
                sub = createNode();
                _subs[0][0][1] = sub;
            }
            sub->addSecondElementToLeaf(elem);
        }

        ov1[1] += lv1;
        ov1[2] = ov[2];
        if(elem->isCollisionWithAABB(ov1,lv1,distance)){
            KSphereOctree* sub = _subs[0][1][0];
            if(0 == sub){
                sub = createNode();
                _subs[0][1][0] = sub;
            }
            sub->addSecondElementToLeaf(elem);
        }

        ov1[2] += lv1;
        if(elem->isCollisionWithAABB(ov1,lv1,distance)){
            KSphereOctree* sub = _subs[0][1][1];
            if(0 == sub){
                sub = createNode();
                _subs[0][1][1] = sub;
            }
            sub->addSecondElementToLeaf(elem);
        }

        ov1[0] = ov[0] + lv1;
        ov1[1] = ov[1];
        ov1[2] = ov[2];
        if(elem->isCollisionWithAABB(ov1,lv1,distance)){
            KSphereOctree* sub = _subs[1][0][0];
            if(0 == sub){
                sub = createNode();
                _subs[1][0][0] = sub;
            }
            sub->addSecondElementToLeaf(elem);
        }

        ov1[2] += lv1;
        if(elem->isCollisionWithAABB(ov1,lv1,distance)){
            KSphereOctree* sub = _subs[1][0][1];
            if(0 == sub){
                sub = createNode();
                _subs[1][0][1] = sub;
            }
            sub->addSecondElementToLeaf(elem);
        }

        ov1[1] += lv1;
        ov1[2] = ov[2];
        if(elem->isCollisionWithAABB(ov1,lv1,distance)){
            KSphereOctree* sub = _subs[1][1][0];
            if(0 == sub){
                sub = createNode();
                _subs[1][1][0] = sub;
            }
            sub->addSecondElementToLeaf(elem);
        }

        ov1[2] += lv1;
        if(elem->isCollisionWithAABB(ov1,lv1,distance)){
            KSphereOctree* sub = _subs[1][1][1];
            if(0 == sub){
                sub = createNode();
                _subs[1][1][1] = sub;
            }
            sub->addSecondElementToLeaf(elem);
        }
    } // for i



    Util::k_free((void*)_secondData);
    _secondData = 0;
    _secondN = 0;
    _secondSize = 0;
    _isLeaf = 0;
    return 0;
}

}
#endif