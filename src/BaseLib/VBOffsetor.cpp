#include "..\..\inc\BaseLib\VBOffsetor.h"
#include "util.h"
#ifdef _DEBUG
#define USING_CAD_TO_LOG
#endif

namespace BaseType{

    VBOffsetor::VBOffsetor():
        _isTwoSide(false),
        _isClosed(false),
        _sepPoint(nullptr),
        _startPos(0),
        _offsetVal(1.0),
        _fNormal(0.0,0.0,1.0)
    {
    }


    VBOffsetor::~VBOffsetor()
    {
        for (auto it = _segments.begin(); it != _segments.end(); ++it){
            delete (*it);
        }
        _segments.clear();
        for (auto it = _contours.begin(); it != _contours.end(); ++it){
            delete (*it);
        }
    }

    void VBOffsetor::doOffset()
    {
        if (_isTwoSide){
            calcOffsetPolygonTwoSide();
        }
        else{
            calcOffsetPolygonOneSide();
        }
        resolSegmentIntersection();
        deleteInsideSegmentsClosedCase();
        connectContours();
    }

    void VBOffsetor::calcOffsetPolygonTwoSide()
    {

        unsigned nvs = _baseCurve.size();
        if (nvs < 2)
            return;
        _offsetCurve.resize(nvs * 2);
        
        Vector3Dd v1 = *_baseCurve[1] - *_baseCurve[0];
        Vector3Dd nv = v1*_fNormal;
        nv.unit();
        double offset = _offsetVal*0.5;
        int i0 = nvs - 1;
        int i1 = nvs;
        _offsetCurve[i1] = *_baseCurve[0] + nv*offset;
        _offsetCurve[i0] = *_baseCurve[0] - nv*offset;
        if (!_isTwoSide && !_isClosed){
            _sepPoint = &_offsetCurve[i1];
        }
        i1++;
        i0--;
        Vector3Dd v2;
        Point3Dd p;
        int normInd = _fNormal.getIndexOfMaxAbsCoord();
        for (unsigned i = 1; i < nvs - 1; ++i){
            v2 = *_baseCurve[i + 1] - *_baseCurve[i];
            nv = v2*_fNormal;
            nv.unit();
            p = *_baseCurve[i] + nv*offset;
            if (!BaseType::Util::calcIntPointOf2PlanarLine(_offsetCurve[i1 - 1], v1, p, v2, normInd, _offsetCurve[i1])){
                _offsetCurve[i1] = p;
            }
            _offsetCurve[i0] = *_baseCurve[i] + (*_baseCurve[i] - _offsetCurve[i1]);
            i0--;
            i1++;
            v1 = v2;
        }
        _offsetCurve[i1] = *_baseCurve.back() + nv*offset;
        _offsetCurve[i0] = *_baseCurve.back() - nv*offset;

        nvs *= 2;
        _segments.resize(nvs);
        nvs--;
        for (unsigned i = 0; i < nvs; ++i){
            _segments[i] = new OffSegment(&_offsetCurve[i], &_offsetCurve[i + 1]);
        }
        _segments[nvs] = new OffSegment(&_offsetCurve[nvs], &_offsetCurve[0]);

    }

    void VBOffsetor::calcOffsetPolygonOneSide()
    {
        unsigned nvs = _baseCurve.size();
        if (nvs < 2)
            return;
        if (_baseCurve.back()->isEqualTo(*_baseCurve[0])){
            _isClosed = true;
            _offsetCurve.resize(nvs-1);
        }
        else{
            _isClosed = false;
        _offsetCurve.resize(nvs);
        }


        Vector3Dd v1 = *_baseCurve[1] - *_baseCurve[0];
        
        Vector3Dd nv = v1*_fNormal;
        nv.unit();
        double offset = _offsetVal;
        int i1 = 0;
        _offsetCurve[i1] = *_baseCurve[0] + nv*offset;
        i1++;
        Vector3Dd v2;
        Point3Dd p;
        int normInd = _fNormal.getIndexOfMaxAbsCoord();
        for (unsigned i = 1; i < nvs-1; ++i){
            v2 = *_baseCurve[i + 1] - *_baseCurve[i];
            nv = v2*_fNormal;
            nv.unit();
            p = *_baseCurve[i] + nv*offset;
            if (!BaseType::Util::calcIntPointOf2PlanarLine(_offsetCurve[i1 - 1], v1, p, v2, normInd, _offsetCurve[i1])){
                _offsetCurve[i1] = p;
            }
            i1++;
            v1 = v2;
        }
        if (_isClosed){
            v1 = *_baseCurve[1] - *_baseCurve[0];
            BaseType::Util::calcIntPointOf2PlanarLine(_offsetCurve[i1 - 1], v2, _offsetCurve[1], v1, normInd, _offsetCurve[0]);
        }
        else{
            _offsetCurve[i1] = *_baseCurve.back() + nv*offset;
        }

#ifdef _DEBUG
#if 0
		Acad::start();
		std::vector<Point3Dd> tempp(_offsetCurve.begin(), _offsetCurve.end());
		Acad::draw3dPoly(tempp, 4);
#endif
#endif
        nvs = _offsetCurve.size();
        _segments.resize(nvs);
        nvs--;
        for (unsigned i = 0; i < nvs; ++i){
            _segments[i] = new OffSegment(&_offsetCurve[i], &_offsetCurve[i + 1]);
        }
        if (_isClosed){
            _segments[nvs] = new OffSegment(&_offsetCurve[nvs], &_offsetCurve[0]);
        }
        else{
            _segments.pop_back();
        }
    }




    void VBOffsetor::resolSegmentIntersection()
    {
        unsigned n = _segments.size();
        
        OffSegment *seg, *seg1, *newSeg, *newSeg1;
        int normInd = _fNormal.getIndexOfMaxAbsCoord();
        int minInd = (normInd + 1) % 3;
        Point3Dd intp;
        Vector3Dd dir1, dir2, dv;
        bool isFoundIntersect;
        _startPos = 0;
        double mincoord = _segments[0]->_sp->v[minInd];
        for (unsigned i = 0; i < _segments.size() - 1;){
            seg = _segments[i];
            if (seg->_sp->v[minInd] < mincoord){
                mincoord = seg->_sp->v[minInd];
                _startPos = i;
            }
            isFoundIntersect = false;
            for (unsigned j = i + 2; j < _segments.size(); ++j){
                seg1 = _segments[j];
                if (BaseType::Util::calcIntPointOf2PlanarSegmentIgnoreAllTips(*seg->_sp,
                    *seg->_ep, *seg1->_sp, *seg1->_ep, normInd, intp)){ // @note: need to resolve the case a segment was separated by end of two other
                    seg->caclDirection(dir1);
                    seg1->caclDirection(dir2);
                    _offsetCurve.push_back(intp);
                    newSeg = new OffSegment(seg->_sp, &_offsetCurve.back());
                    newSeg1 = new OffSegment(seg1->_sp, &_offsetCurve.back());
                    newSeg->_sFlag = seg->_sFlag;
                    newSeg->_sCross = seg->_sCross;
                    newSeg->_eCross = true;
                    newSeg1->_sFlag = seg1->_sFlag;
                    newSeg1->_sCross = seg1->_sCross;
                    newSeg1->_eCross = true;
                    _segments.insert(_segments.begin() + j, newSeg1);
                    _segments.insert(_segments.begin() + i, newSeg);
                    seg->_sp = &_offsetCurve.back();
                    seg1->_sp = &_offsetCurve.back();
                    dv = dir1*dir2;
                    if (dv.scalarProduct(_fNormal) > 0){
                        seg->_sFlag = true;
                        newSeg->_eFlag = false;
                        seg1->_sFlag = false;
                    }
                    else{
                        seg1->_sFlag = true;
                        seg->_sFlag = false;
                        newSeg1->_eFlag = false;
                    }
                    isFoundIntersect = true;
                    break;
                }
            }//for j
            if (!isFoundIntersect){
                i++;
            }
        } // for i
    }

#if 0

    void VBOffsetor::deleteInsideSegmentsClosedCase()
    {
#ifdef USING_CAD_TO_LOG_
        Acad::start();
        std::vector<Point3Dd*> samplingCurve;
        for (unsigned i = 0; i < _segments.size(); ++i){
            OffSegment* seg = _segments[i];
            samplingCurve.clear();
            samplingCurve.push_back(seg->_sp);
            samplingCurve.push_back(seg->_ep);
            Acad::draw3dPoly(samplingCurve,2);
        }
#endif
        unsigned n = _segments.size();
        if (_startPos > 0){
            std::deque<OffSegment*> temp(_segments.begin(), _segments.begin() + _startPos);
            _segments.erase(_segments.begin(), _segments.begin() + _startPos);
            _segments.insert(_segments.end(), temp.begin(),temp.end());
        }
        assert(_segments.size() == n);
        unsigned startpos = 0, endpos = 0;
        int count = 0;
        OffSegment *seg;
        for (unsigned i = 0; i < n; ++i)
        {
            seg = _segments[i];
            if (seg->_eCross){
                if (seg->_eFlag){
                    if (0 == count){
                        startpos = i + 1;
                    }
                    count++;
                }
                else {
                    count--;
                    if (0 == count){
                        endpos = i + 1;
                    }
                }
            }
            if (startpos > 0 && 0 == count){
                for (unsigned j = startpos; j < endpos; ++j){
                    delete _segments[j];
                    _segments[j] = nullptr;
                }
                startpos = 0;
                endpos = 0;
            }
        }

        for (int i = int(n-1); i >= 0; --i)
        {
            if (nullptr == _segments[i]){
                _segments.erase(_segments.begin() + i);
            }
        }

#ifdef USING_CAD_TO_LOG_
        Acad::start();
        //std::vector<Point3Dd*> samplingCurve;
        for (unsigned i = 0; i < _segments.size(); ++i){
            OffSegment* seg = _segments[i];
            samplingCurve.clear();
            samplingCurve.push_back(seg->_sp);
            samplingCurve.push_back(seg->_ep);
            Acad::draw3dPoly(samplingCurve, 1);
        }
#endif

    }

#else



    void VBOffsetor::deleteInsideSegmentsClosedCase()
    {
#ifdef USING_CAD_TO_LOG_
        Acad::start();
        std::vector<Point3Dd*> samplingCurve;
        for (unsigned i = 0; i < _segments.size(); ++i){
            OffSegment* seg = _segments[i];
            samplingCurve.clear();
            samplingCurve.push_back(seg->_sp);
            samplingCurve.push_back(seg->_ep);
            Acad::draw3dPoly(samplingCurve, 2);
        }
#endif
        unsigned n = _segments.size();
        int count = 0;
        int mincount = 0;
        OffSegment *seg;
        for (unsigned i = 0; i < n; ++i)
        {
            seg = _segments[i];
            seg->_count = count;
            if (seg->_eCross){
                if (seg->_eFlag){
                    count++;
                }
                else {
                    count--;
                }
                if (count < mincount){
                    mincount = count;
                }
            }
        }

        for (int i = int(n - 1); i >= 0; --i)
        {
            seg = _segments[i];
            if (seg->_count > mincount){
                _segments.erase(_segments.begin() + i);
                delete seg;
            }
        }

#ifdef USING_CAD_TO_LOG_
        Acad::start();
        //std::vector<Point3Dd*> samplingCurve;
        for (unsigned i = 0; i < _segments.size(); ++i){
            OffSegment* seg = _segments[i];
            samplingCurve.clear();
            samplingCurve.push_back(seg->_sp);
            samplingCurve.push_back(seg->_ep);
            Acad::draw3dPoly(samplingCurve, 1);
        }
#endif

    }

#endif

    void VBOffsetor::connectContours()
    {
#if 1
        std::deque<Point3Dd*> curve;
        unsigned n = _segments.size();
        std::vector<Point3Dd*> *contour;
        OffSegment *elem, *sedElem;
        bool isFound;
        Point3Dd *p0, *p1, *p2;
        unsigned startPos = 0;
        while (startPos < n){
            elem = _segments[startPos];
            p0 = elem->_sp;
            p1 = elem->_ep;
            curve.clear();
            curve.push_back(p1);
            for (unsigned i = startPos; i < n;){
                isFound = false;
                for (unsigned j = i + 1; j < n; ++j){
                    sedElem = _segments[j];
                    p2 = sedElem->_sp;
                    if (p1 == p2){
                        p2 = sedElem->_ep;
                        curve.push_back(p2);
                        p1 = p2;
                        if (j > i + 1){
                            std::swap(_segments[i + 1], _segments[j]);
                        }
                        isFound = true;
                        i++;
                        break;
                    }
                }// for j
                if (!isFound){
                    assert(p0->isEqualTo(*p1));
                    
                    contour = new std::vector < Point3Dd* >(curve.begin(), curve.end());
                    _contours.push_back(contour);
#ifdef USING_CAD_TO_LOG_
                    Acad::start();
                    Acad::draw3dPoly(*contour);
#endif

                    startPos = i + 1;
                    break;
                }
            }
        }// while
#endif
    }

}// namespace