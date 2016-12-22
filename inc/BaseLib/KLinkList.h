#ifndef BASELIB_KLINKLIST_H
#define BASELIB_KLINKLIST_H

#include "Point2D.h"

namespace BaseType{
   
template<class T>
class KLinkList{
public:
   KLinkList():
   _value(0),
   _next(0),
   _last(0)
   {
       
   }
   
   KLinkList(T* value):
   _value(value),
   _next(0),
   _last(0)
   {
       
   }

   virtual ~KLinkList(){}

   T* getValue(){ return _value;};
   void setValue(T* value){ _value = value;};
   KLinkList* getNext(){ return _next;};
   void setNext(KLinkList* next){ _next = next;};
   KLinkList* getLast(){ return _last;};
   void setLast(KLinkList* last){ _last = last;};
   void setRelation(KLinkList* other){
      _next = other;
      other->_last = this;
   };
   void swapRelation(){
       KLinkList* temp = _next;
       _next = _last;
       _last = temp;
   };
   bool isCollisionWithAabb2D(const double ov[2], const double& lv)
   {
      return _value->isCollisionWithAabb2D(ov,lv);
   }
   
  /* void setUnder_dValue(const double& val){ _dValues[0] = val;};
   void setUpper_dValue(const double& val){ _dValues[1] = val;};
   double getUnder_dValue(){ return _dValues[0];}
   double getUpper_dValue(){ return _dValues[1];}*/

protected:
    // bool _isBig;
   T* _value;
   KLinkList* _next;
   KLinkList* _last;
   //double _angle;
};

template<class T>
class KLinkListEx :
    public KLinkList < T > {
    bool _isBig;
    float _angle; // cos of angle
    unsigned _position;
    int _id; // contour id; -1 if point is seen as deleted
public:
    KLinkListEx() :
        KLinkList(),
        _isBig(true),
        _angle(0.0),
        _position(0),
        _id(0)
    {

    }

    KLinkListEx(T* value) :
        KLinkList(value),
        _isBig(true),
        _angle(0.0),
        _position(0),
        _id(0)
    {

    }

    virtual ~KLinkListEx(){}
    void setBigAngle(bool flag = true){ _isBig = flag; }
    bool isBigAngle(){ return _isBig; }
    void setAngle(float angle){ _angle = angle; }
    float getAngle(){ return _angle; }
    void setPosition(unsigned pos){ _position = pos; }
    unsigned getPosition(){ return _position; }
    void setId(int id){ _id = id; };
    int getId(){ return _id; }
    float* getCoord(){
        return _value->data();
    }
    bool isAngleSmaller(KLinkListEx *other){
        if (_isBig){
            if (other->_isBig){
                if (_angle < other->_angle){
                    return true;
                }
                else{
                    return false;
                }
            }
            else{
                return false;
            }
        }
        else{
            if (other->_isBig){
                return true;
            }
            else{
                if (_angle > other->_angle){
                    return true;
                }
                else{
                    return false;
                }
            }
        }
        return false;
    }
    bool isMallerAngleNoBig(KLinkListEx *other){
        if (other->_isBig){
            return true;
        }
        else{
            if (_isBig){
                return false;
            }
            else{
                if (_angle > other->_angle){
                    return true;
                }
                else{
                    return false;
                }
            }
        }
        return false;
    }

    bool isCollisionWithAabb2D(const float ov[2], const float& lv)
    {
        return _value->isCollisionWithAabb2D(ov, lv);
    }
    void recalcAngle(){
        float *next = _next->getValue()->data();
        float *last = _last->getValue()->data();
        float *coord = _value->data();
        Vector2Df dv1(next[0] - coord[0], next[1] - coord[1]);
        Vector2Df dv2(last[0] - coord[0], last[1] - coord[1]);
        dv1.normalize();
        dv2.normalize();
        if (dv1.crossProduct(dv2) > 0.0f){
            _isBig = false;
        }
        else {
            _isBig = true;
        }
        _angle = (float)dv1.scalarProduct(dv2);
    }
    /* void setUnder_dValue(const double& val){ _dValues[0] = val;};
    void setUpper_dValue(const double& val){ _dValues[1] = val;};
    double getUnder_dValue(){ return _dValues[0];}
    double getUpper_dValue(){ return _dValues[1];}*/
    void setIDForAllLinkList(int id){
        _id = id;
        KLinkListEx *elem = dynamic_cast<KLinkListEx*>(_next);
        while (elem && elem != this)
        {
            elem->_id = id;
            elem = dynamic_cast<KLinkListEx*>(elem->_next);
        }
    }
};
} // namespace
#endif
