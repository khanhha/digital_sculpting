#ifndef BASELIB_VBOFFSETOR_H
#define BASELIB_VBOFFSETOR_H
#include <deque>
#include <vector>
#include "Point3Dd.h"

namespace BaseType{

    class VBOffsetor
    {
    public:
        
        struct OffSegment
        {
            bool _sFlag;
            bool _eFlag;
            bool _sCross;
            bool _eCross;
            Point3Dd *_sp;
            Point3Dd *_ep;
            int _count;
            OffSegment() :
                _sFlag(true),
                _eFlag(true),
                _sCross(false),
                _eCross(false),
                _sp(nullptr),
                _ep(nullptr)
            {}
            OffSegment(Point3Dd *sp, Point3Dd *ep) :
                _sFlag(true),
                _eFlag(true),
                _sCross(false),
                _eCross(false),
                _sp(sp),
                _ep(ep){}
            void caclDirection(Vector3Dd &dir){ dir = *_ep - *_sp; }

        };

    private:
        bool _isTwoSide;
        bool _isClosed;
        Point3Dd *_sepPoint; // used in case of one side offset of opened contour
        unsigned _startPos; // position of segment that has start coordinate is minimum
        double _offsetVal;
        Vector3Dd _fNormal;
        std::vector<Point3Dd*> _baseCurve;
        std::deque<Point3Dd> _offsetCurve;
        std::deque<OffSegment*> _segments;
        std::vector<std::vector<Point3Dd*>*> _contours;
    public:
        VBOffsetor();
        ~VBOffsetor();

        void setOffsetTwoSide(bool val){ _isTwoSide = val; }
        void setOffsetValue(const double &offsetVal){ _offsetVal = offsetVal; }
        void setNormalVector(const Vector3Dd &norm){ _fNormal = norm; }
        void setBaseCurve(const std::vector<Point3Dd*> &curve){
            _baseCurve.assign(curve.begin(), curve.end()); }
        void doOffset();
        void calcOffsetPolygonTwoSide();
        void calcOffsetPolygonOneSide();
        void resolSegmentIntersection();
        void deleteInsideSegmentsClosedCase();
        void connectContours();
        void getContours(std::vector<std::vector<Point3Dd*>*> &contours){ 
            contours.assign(_contours.begin(), _contours.end()); }
    };

}// namespace
#endif