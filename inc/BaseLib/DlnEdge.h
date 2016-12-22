#ifndef SUPPORTGEN_SKEEDGE_H
#define SUPPORTGEN_SKEEDGE_H


namespace BaseType{

class DlnPoint;
class DlnTriangle;

class DlnEdge
{
public:
   DlnEdge(void);
   DlnEdge(DlnPoint* p1, DlnPoint* p2);
   ~DlnEdge(void);
   bool isChecked();
   int setChecked(const bool flag = true);
   DlnPoint* getPoint(int i);
   int addTriangle(DlnTriangle* trig);
   bool isNotBoundaryEdge();
   double calc2DSquareLength();
   double calc2DLength();
   int calc2DMidPoint(double* mid);
   int removeTriangle(const DlnTriangle* trig);
   DlnTriangle* getOtherTriangle(DlnTriangle* trig);
   DlnTriangle* getTriangle(size_t i);
   bool addPoint(DlnPoint* p);
   bool removePoint(DlnPoint* p);
   void removeFromPoint();
private:
   bool _isChecked;
   DlnPoint* _vps[2];
   DlnTriangle* _trigs[2];
};

} // namespace

#endif