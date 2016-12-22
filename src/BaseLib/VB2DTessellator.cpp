#include "VB2DTessellator.h"

#if 0
#ifdef _DEBUG
#include "..\..\inc\Diagnotics\AutoCadAutomator.h"
#define USING_CAD_TO_LOG
#endif
#endif

namespace BaseType{

    VB2DTessellator::VB2DTessellator():
        _numberInitContour(0)
    {
    }


    VB2DTessellator::~VB2DTessellator()
    {
        if (!_contours.empty()){
            Point2DfLinkList *org, *curent, *next;
            for (unsigned i = 0; i < _contours.size(); ++i){
                org = _contours[i];
                curent = dynamic_cast<Point2DfLinkList*>(org->getNext());
                while (curent != org){
                    next = dynamic_cast<Point2DfLinkList*>(curent->getNext());
                    delete curent;
                    next = curent;
                }
                delete org;
            }
        }
    }

    void VB2DTessellator::setOrigin(float origin[2])
    {
		float origin1[2] = { origin[0] - (float)EPSILON_VAL_E3, origin[1] - (float)EPSILON_VAL_E3 };
        _grid.setOrigin(origin1);
    }

    void VB2DTessellator::setSize(float size[2])
    {
		float size1[2] = { size[0] + 2.0f*(float)EPSILON_VAL_E3, size[1] + 2.0f*(float)EPSILON_VAL_E3 };
        float resol = 10.0;
        float step = min2<float>(size1[0], size1[1]);
        step /= resol;
        _grid.setSize(size1);
        _grid.setStepSize(step);
        _grid.init();
    }

#if 0
    void VB2DTessellator::addAContour(const std::vector<Point2Df*> &contour)
    {
        unsigned n = contour.size();
        if (n < 3){
            return;
        }
        unsigned startpos = 0;
        bool flag = contour[0]->isConcident(*contour.back(), EPSILON_VAL_BIG);
        if (flag){
            startpos = 1;
        }
        Point2DfLinkList *org = new Point2DfLinkList(contour[startpos]);
        Point2DfLinkList *curent, *next;
        curent = org;
        unsigned position = _pointCoord.size();
        curent->setPosition(position);
        position++;
        startpos++;
        for (unsigned i = startpos; i < n; ++i){
            _grid.addElement(curent, contour[0]->_vcoords, contour[1]->_vcoords);
            next = new Point2DfLinkList(contour[i]);
            curent->setRelation(next);
            next->setPosition(position);
            position++;
            curent = next;
        }
        curent->setRelation(org);
        _grid.addElement(curent, contour.back()->_vcoords, contour[0]->_vcoords);
        org->setId(_contours.size());
        _contours.push_back(org);
        _pointCoord.insert(_pointCoord.end(), contour.begin(), contour.end());
    }

#else


    void VB2DTessellator::addAContour(const std::vector<Point2Df*> &contour)
    {
        unsigned n = contour.size();
        if (n < 3){
            return;
        }
        unsigned startpos = 0;
		bool flag = contour[0]->isConcident(*contour.back(), (float)EPSILON_VAL_BIG);
        if (flag){
            startpos = 1;
        }
        int id = _contours.size();
        Point2DfLinkList *org = new Point2DfLinkList(contour[startpos]);
        Point2DfLinkList *curent, *next;
        curent = org;
        unsigned position = _pointCoord.size();
        curent->setPosition(position);
        curent->setId(id);
        _grid.addElement(curent, curent->getValue()->_vcoords);
        position++;
        startpos++;
        for (unsigned i = startpos; i < n; ++i){
            next = new Point2DfLinkList(contour[i]);
            next->setPosition(position);
            next->setId(id);
            curent->setRelation(next);
            position++;
            curent = next;
            _grid.addElement(curent,curent->getValue()->_vcoords);
        }
        curent->setRelation(org);
        _contours.push_back(org);
        _pointCoord.insert(_pointCoord.end(), contour.begin()+(startpos-1), contour.end());
    }

#endif

    void VB2DTessellator::dumpToCad(Point2DfLinkList *contour,int color)
    {
#ifdef USING_CAD_TO_LOG
        std::vector<Point2Df*> points;
        points.push_back(contour->getValue());
        Point2DfLinkList *it = dynamic_cast<Point2DfLinkList*>(contour->getNext());
        while (it != contour){
            points.push_back(it->getValue());
            it = dynamic_cast<Point2DfLinkList*>(it->getNext());
        }
        if (points.size() > 1){
            points.push_back(points[0]);
            Acad::start();
            Acad::draw2dPolyline(points, color);
            int temppp = 1;
        }
#endif
    }

    void VB2DTessellator::dumpToCad( int color)
    {
        for (unsigned i = 0; i < _contours.size(); ++i){
            dumpToCad(_contours[i],color+i);
        }
    }

    Point2DfLinkList* VB2DTessellator::findOuterContour()
    {
        Point2DfLinkList *ret = _grid.findLeftMostElement();
        if (ret){
            assert(ret->getId() >= 0);
            int id = ret->getId();
            for (auto it = _contours.begin(); it != _contours.end(); ++it){
                if ((*it)->getId() == id){
                    _contours.erase(it);
                    break;
                }
            }
        }
        return ret;
    }

    void VB2DTessellator::calcAngleOfContour(Point2DfLinkList *contour)
    {
#ifdef _DEBUG
        std::vector<Point2Df*> dumpPoint;
#endif
        Point2DfLinkList *curent = contour, *next = dynamic_cast<Point2DfLinkList*>(contour->getNext());
        Vector2Df v1, v2;
        v1 = *curent->getValue();
        v1 -= *curent->getLast()->getValue();
        v2 = *next->getValue();
        v2 -= *curent->getValue();
        v1.normalize();
        v2.normalize();
		curent->setAngle((float)v2.scalarProduct(-v1));
        curent->setBigAngle(((v1.crossProduct(v2) > 0.0) ? false : true));
#ifdef _DEBUG
        dumpPoint.push_back(curent->getValue());
#endif
        curent = next;
        while (curent != contour){
            v1 = v2;
            next = dynamic_cast<Point2DfLinkList*>(curent->getNext());
            v2 = *next->getValue();
            v2 -= *curent->getValue();
            v2.normalize();
			curent->setAngle((float)v2.scalarProduct(-v1));
#ifdef _DEBUG
            float prod = v1.crossProduct(v2);
            dumpPoint.push_back(curent->getValue());
#endif
            curent->setBigAngle(((v1.crossProduct(v2) > 0.0) ? false : true));
            curent = next;
        }
    }

    void VB2DTessellator::calcAngleOfAllContours()
    {
        for (auto it = _contours.begin(); it != _contours.end(); ++it){
            calcAngleOfContour(*it);
        }
    }


    Point2DfLinkList* VB2DTessellator::findVertexWithSmallestAngle(Point2DfLinkList *contour)
    {
        
        Point2DfLinkList* v0 = contour;
        Point2DfLinkList* v = dynamic_cast<Point2DfLinkList*>(v0->getNext());

        while (v != contour){
            if (v->isMallerAngleNoBig(v0)){
                v0 = v;
            }
            v = dynamic_cast<Point2DfLinkList*>(v->getNext());
        }
        return v0;
    }
    

#ifdef _DEBUG
    int VB2DTessellator_createTriangle_count = 0;
#endif
    Point2DfLinkList*  VB2DTessellator::createTriangle(Point2DfLinkList *vmin)
    {

#ifdef _DEBUG
        VB2DTessellator_createTriangle_count++;
        if (VB2DTessellator_createTriangle_count >= 26){
            g_debug_var = 1;
        }
#endif
        Point2DfLinkList *next = dynamic_cast<Point2DfLinkList*>(vmin->getNext());
        Point2DfLinkList *last = dynamic_cast<Point2DfLinkList*>(vmin->getLast());
        if (next == vmin){
            vmin->setId(-1);
            return nullptr;
        }
        else if (next == last){
            next->setId(-1);
            vmin->setId(-1);
            return nullptr;
        }
        if (next->getNext() == last){
            _indexTrigs.push_back(vmin->getPosition());
            _indexTrigs.push_back(next->getPosition());
            _indexTrigs.push_back(last->getPosition());
            vmin->setId(-1);
            next->setId(-1);
            last->setId(-1);
            return nullptr;
        }
        Point2DfLinkList *leftMost = _grid.getElementCollisionWithTriangle(vmin->getValue()->_vcoords,
            next->getValue()->_vcoords,last->getValue()->_vcoords);
        if (leftMost){
                Point2DfLinkList *newVmin = new Point2DfLinkList(vmin->getValue());
                newVmin->setId(vmin->getId());
                newVmin->setPosition(vmin->getPosition());
                Point2DfLinkList *newLeft = new Point2DfLinkList(leftMost->getValue());
                newLeft->setPosition(leftMost->getPosition());
                Point2DfLinkList *nextLeft = dynamic_cast<Point2DfLinkList*>(leftMost->getNext());
                Point2DfLinkList *lastLeft = dynamic_cast<Point2DfLinkList*>(leftMost->getLast());
                vmin->setRelation(leftMost);
                vmin->recalcAngle();
                leftMost->recalcAngle();

                lastLeft->setRelation(newLeft);
                newLeft->setRelation(newVmin);
                newVmin->setRelation(next);
                newVmin->recalcAngle();
                newLeft->recalcAngle();
            if (vmin->getId() == leftMost->getId()){
                _numberInitContour++;
                setPolygonID(newLeft, _numberInitContour);
                _contours.push_back(newLeft);
#ifdef _DEBUG
             //   dumpToCad(vmin, VB2DTessellator_createTriangle_count % 7);
             //   dumpToCad(newLeft, VB2DTessellator_createTriangle_count  % 5);

#endif
                return vmin;
            }
            else{
                int id = leftMost->getId();
                for (auto it = _contours.begin(); it != _contours.end(); ++it){
                    if ((*it)->getId() == id){
                        _contours.erase(it);
                        break;
                    }
                }
                setPolygonID(vmin, vmin->getId());
#ifdef _DEBUG
              //  dumpToCad(vmin, VB2DTessellator_createTriangle_count % 7);
#endif
                return vmin;
            }
        }

        _indexTrigs.push_back(vmin->getPosition());
        _indexTrigs.push_back(next->getPosition());
        _indexTrigs.push_back(last->getPosition());
        if (next->getNext()->getValue() == last->getValue()){
            Point2DfLinkList *nextnext = dynamic_cast<Point2DfLinkList*>(next->getNext()->getNext());
            last->setRelation(nextnext);
            last->recalcAngle();
            dynamic_cast<Point2DfLinkList*>(next->getNext())->setId(-1);
            next->setId(-1);
            return last;
        }
        else if (last->getLast()->getValue() == next->getValue()){
            Point2DfLinkList *lastlast = dynamic_cast<Point2DfLinkList*>(last->getLast()->getLast());
            lastlast->setRelation(next);
            next->recalcAngle();
            dynamic_cast<Point2DfLinkList*>(last->getLast())->setId(-1);
            last->setId(-1);
            return next;
        }
        last->setRelation(next);
        last->recalcAngle();
        next->recalcAngle();
        vmin->setId(-1);
#ifdef _DEBUG
        //if (VB2DTessellator_createTriangle_count >= 230){
            g_debug_var = 1;
    //    dumpToCad(last, VB2DTessellator_createTriangle_count % 7);
       // }
#endif
        return last;
    }

    void VB2DTessellator::generateTriangles()
    {
        calcAngleOfAllContours();
        _numberInitContour = _contours.size();
        Point2DfLinkList *outContour;
        while (!_contours.empty()){
            outContour = _contours.back();
            if (outContour->getId() < (int)_numberInitContour){
                outContour = findOuterContour();
#ifdef _DEBUG
               // dumpToCad(outContour);
#endif
            }
            else{
                _contours.pop_back();
            }
            while (outContour)
            {
                outContour = findVertexWithSmallestAngle(outContour);
                outContour = createTriangle(outContour);
            }
        }

    }



    void VB2DTessellator::getData(float **coords, unsigned &coordSize, unsigned **index, unsigned &indexSize)
    {
        coordSize = _pointCoord.size();
        if (coordSize > 0){
            float *temp = new float[coordSize*2];
            unsigned j = 0;
            for (unsigned i = 0; i < coordSize; ++i){
                temp[j] = _pointCoord[i]->x; j++;
                temp[j] = _pointCoord[i]->y; j++;
            }
            *coords = temp;
        }
        indexSize = _indexTrigs.size();
        if (indexSize > 0){
            unsigned *temp = new unsigned[indexSize];
            for (unsigned i = 0; i < indexSize; ++i){
                temp[i] = _indexTrigs[i];
            }
            *index = temp;
        }
    }

    void VB2DTessellator::getTrianglesIndex(unsigned **index, unsigned &nts)
    {
        nts = _indexTrigs.size();
        if (nts > 0){
            unsigned *temp = new unsigned[nts];
            for (unsigned i = 0; i < nts; ++i){
                temp[i] = _indexTrigs[i];
            }
            *index = temp;
        }
        assert((nts % 3) == 0);
        nts /= 3;
    }

}// namespace