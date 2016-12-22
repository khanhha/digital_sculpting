#ifndef SUPPORTGEN_SKETRIANGLE_H
#define SUPPORTGEN_SKETRIANGLE_H

#include "Point2D.h"

namespace BaseType{

class DlnEdge;
class DlnPoint;

class DlnTriangle
{
public:
   DlnTriangle(void);
   DlnTriangle(DlnPoint* p0, DlnPoint* p1, DlnPoint* p2);
   ~DlnTriangle(void);

   DlnEdge* getEdge(const unsigned i);
   bool removeEdge(DlnEdge* e);
   bool addEdge(DlnEdge* e);
   void removeFromEdge();
   void removePoints();
   int setEdge(const int i, DlnEdge* ed);
   bool isExistEdge(const int &i);
   Point2Dd* getCoord(int i);
   int calc2DcenterPoint(double* cen);
   int checkType();
   unsigned getType();
   bool isChecked();
   int setChecked(bool flag = true);
   int filter(const double& aveWidth2);
   int getTipVertexCoordOfType1Trig(double* coord);
   DlnEdge* getSkeEdgeOfType1Trig();
   DlnEdge* getUncheckedSkeEdge();
   DlnEdge* getOtherSkeEdge(DlnEdge* ed);
   DlnPoint* getOtherPoint(DlnPoint* v0, DlnPoint* v1);
   void setEdges(DlnEdge* e0, DlnEdge* e1, DlnEdge* e2);
   void setPoints(DlnPoint* p0, DlnPoint* p1, DlnPoint* p2); 
   void recalcType();
   DlnPoint* getPoint(size_t i){ return _points[i]; }
private:
   unsigned short _isChecked:1;
   unsigned short _skeTrigType:4;
   DlnEdge* _edges[3];
   DlnPoint* _points[3];

#ifdef _DEBUG
int SkeTriangle_filter_count;
#endif
};

}

#endif