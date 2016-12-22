#ifndef SUPPORTGEN_SKEPOINT_H
#define SUPPORTGEN_SKEPOINT_H

#include "Baselib/KLinkList.h"
#include "Baselib/Point2D.h"

namespace BaseType{

class DlnEdge;

class DlnPoint
{
public:
   DlnPoint(void);
   DlnPoint(Point2Dd* coord);
   ~DlnPoint(void);

   Point2Dd* getCoord();
   int setCoord(Point2Dd* coord);
   /*bool isBigAngle();
   int setBigAngle(bool isbig);*/
   bool isRemoved();
   int setAsRemoved();
   /*double getAngle();
   int setAngle(const double& angle);*/
   int addEdge(DlnEdge* ed);
   bool removeEdge(DlnEdge* ed);
   bool isCollisionWithAabb2D(const double ov[2], const double& lv);
   DlnEdge* findSkeEdge(DlnPoint* otherpoint);
   
private:
    unsigned short _isRemoved:1;
   unsigned short _nEdge:15;
   unsigned short _sizeEdge:15;
   unsigned short _id;
   Point2Dd* _coord;
   DlnEdge** _edges;
};

class SkePointLink: public BaseType::KLinkList<DlnPoint>{
private:
    unsigned _isBig:1;
    unsigned _id:31;
    double _angle;
public:
    SkePointLink(void):
        BaseType::KLinkList<DlnPoint>(),
        _isBig(0),
        _id(0),
        _angle(0.0)
    {};
    SkePointLink(DlnPoint* skp):
        BaseType::KLinkList<DlnPoint>(skp),
        _isBig(0),
        _id(0),
        _angle(0.0)
    {};
    virtual ~SkePointLink(void){}
    unsigned getId(){return _id;}
    void setId(unsigned id){ _id = id;}
   bool isBigAngle(){ return _isBig; }

   int setBigAngle(bool isbig)
   {
      _isBig = isbig;
      return 0;
   }
   double getAngle(){return _angle;}
   int setAngle(const double& angle)
   {
      _angle = angle;
      return 0;
   }
};
}// namespace

#endif