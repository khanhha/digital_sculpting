#include "contour2d.h"
#include "segment2Dd.h"
#include "Point2D.h"
#include "Ray2Dd.h"
#include <algorithm>
#include "line2Dd.h"
#include "Point3Dd.h"

Contour2Dd::Contour2Dd(double zValue)
    : _zValue(zValue)
    , _minx(0.0)
    , _miny(0.0)
    , _maxx(0.0)
    , _maxy(0.0)
    , _parrent(nullptr)
{
    _isInverted = false;
	_isInside = false;
	_isMarked = false;
	_isOpened = false;
	_isHollow = false;

	//_color[0] = (rand()%100)/100.0f;
	//_color[1] = (rand()%100)/100.0f;
	//_color[2] = (rand()%100)/100.0f;
}

Contour2Dd::~Contour2Dd()
{
	for (int i = 0, size = _segments.size(); i < size; i++) {
		//Segment2Dd* segment = _segments.at(i);
		delete _segments[i];
		//segment = NULL;
	}
//	_segments.clear();
}

void Contour2Dd::addSegment(Segment2Dd* segment)
{
	if (!_segments.empty()) {
		Segment2Dd* lastSeg = getLastSegment();
		lastSeg->setNextSegment(segment);
		segment->setPrevSegment(lastSeg);
	}
	_segments.push_back(segment);
	segment->setContour(this);

	// update bounding box
	Point2Dd const & sp = segment->getFirstPoint();
	Point2Dd const & ep = segment->getSecondPoint();
	if (_segments.size() == 1) {
		minmax2(_minx, _maxx, sp._vcoords[0], ep._vcoords[0]);
		minmax2(_miny, _maxy, sp._vcoords[1], ep._vcoords[1]);
	} else {
		minmax4(_minx, _maxx, sp._vcoords[0], ep._vcoords[0], _minx, _maxx);
		minmax4(_miny, _maxy, sp._vcoords[1], ep._vcoords[1], _miny, _maxy);
	}
}

Segment2Dd* Contour2Dd::getSegment(int index)
{
	return _segments.at(index);
}

unsigned int Contour2Dd::getNumberOfSegment()
{
	return _segments.size();
}

bool Contour2Dd::intersect(Contour2Dd *other)
{
    Point2Dd ip;
    for (int i = 0, size = _segments.size(); i < size; ++i)
    {
        Segment2Dd* seg = _segments[i];
        for (int k = 0, size = other->segments().size(); k < size; ++k)
            if (seg->intersectSegment(ip, other->segments()[k]))
                return true;
    }

    return false;
}

bool Contour2Dd::isInsideContour(Contour2Dd *other)
{
    if (_segments.size() < 1 || !other->isPointInside(_segments[0]->sp()))
        return false;

    return !this->intersect(other);
}

bool Contour2Dd::intersectSegment(std::vector<Point2Dd>& ip,
                                  Segment2Dd* segment )
{
	for (int i = 0, size = _segments.size(); i < size; i++) {
		Segment2Dd* segmenti = _segments[i];
		segment->intersectSegment(ip, segmenti);
	}
	return ip.size() > 0;
}

bool Contour2Dd::intersectSegment( std::vector<ISPoint2D>& isp,
                                   Segment2Dd* segment )
{
	for (int i = 0, size = _segments.size(); i < size; i++) {
		Segment2Dd* segmenti = _segments.at(i);
		Point2Dd ip;
		bool isIntersected = segment->intersectSegment(ip, segmenti);
		if (isIntersected) {
			ISPoint2D isPoint(ip, segmenti);
			isp.push_back(isPoint);
		}
	}
	return isp.size() > 0;
}

bool Contour2Dd::intersectLine( std::vector<double>& ip, std::vector<Segment2Dd*>& is,
                                const Point2Dd& p, const Point2Dd& d)
{
	for (int i = 0, size = _segments.size(); i < size; i++) {
		Segment2Dd* segment = _segments.at(i);
		double t;
		bool isIntersected = segment->intersectLine(t, p, d);
		if (isIntersected) {
			ip.push_back(t);
			is.push_back(segment);
		}
	}
	return ip.size() > 0;
}

bool Contour2Dd::isClosed()
{
	if (_segments.size() >= 2) {
		Segment2Dd* seg0 = _segments.at(0);
		Segment2Dd* segn = _segments.at(_segments.size() - 1);
		if (segn->getSecondPoint().isConcident(seg0->getFirstPoint())) {
			segn->setNextSegment(seg0);
			seg0->setPrevSegment(segn);
			return true;
		}
		//if (seg0->isConnectedTo(segn)) {
		//	return true;
		//}
	}
	return false;
}

bool Contour2Dd::intersectLine( std::vector<Point2Dd>& ip, std::vector<Segment2Dd*>& is,
                                const Point2Dd& p, const Point2Dd& d)
{
	for (int i = 0, size = _segments.size(); i < size; i++) {
		Segment2Dd* segment = _segments.at(i);
		Point2Dd ip_;
		bool isIntersected = segment->intersectLine(ip_, p, d);
		if (isIntersected) {
			ip.push_back(ip_);
			is.push_back(segment);
		}
	}
	return ip.size() > 0;
}

void Contour2Dd::getBoundingBox(double& minx, double& miny, double& maxx, double& maxy)
{
	minx = _minx;
	miny = _miny;
	maxx = _maxx;
	maxy = _maxy;
}

void Contour2Dd::getMaxBoundingBox(double& maxx, double& maxy)
{
	maxx = _maxx;
	maxy = _maxy;
}

void Contour2Dd::getMinBoundingBox(double& minx, double& miny)
{
	minx = _minx;
	miny = _miny;
}

double Contour2Dd::getNearestSegments(const Point2Dd& p, std::vector<Segment2Dd*>& segs)
{
	if (_segments.size() == 0) {
		return -1;
	}
	// Get nearest segment
	Segment2Dd* seg = _segments[0];
	//std::vector<Segment2Dd*> segs;
	segs.push_back(seg);
	double dis = p.squareDistanceToSegment(
	                 seg->getFirstPoint(), seg->getSecondPoint());
	for (int j = 1, size = getNumberOfSegment(); j < size; j++) {
		Segment2Dd* segj = this->getSegment(j);
		double disj = p.squareDistanceToSegment(
		                  segj->getFirstPoint(), segj->getSecondPoint());
		if (disj < dis - EPSILON_VAL_) {
			dis = disj;
			seg = segj;
			segs.clear();
			segs.push_back(seg);
		} else if (fabs(disj - dis) < EPSILON_VAL_) {// coincident
			dis = disj;
			seg = segj;
			segs.push_back(segj);
		}
	}
	return dis;
}

void Contour2Dd::setMarked(bool isMark)
{
	_isMarked = isMark;
}

bool Contour2Dd::isMarked()
{
	return _isMarked;
}

bool Contour2Dd::isOpened()
{
	return _isOpened;
}

void Contour2Dd::setOpen(bool isOpen)
{
	_isOpened = isOpen;
}

bool Contour2Dd::checkOpen()
{
	return false;
}

void Contour2Dd::clear()
{
	for (int i = 0, size = _segments.size(); i < size; i++) {
		Segment2Dd* seg = _segments.at(i);
		seg->setContour(NULL);
	}
	_segments.clear();
}

Segment2Dd* Contour2Dd::getLastSegment()
{
	if (_segments.size() > 0) {
		return _segments[_segments.size()-1];
	}
	return NULL;
}

Segment2Dd* Contour2Dd::getFirstSegment()
{
	if (_segments.size() > 0) {
		return _segments[0];
	}
	return NULL;
}

bool Contour2Dd::connectTo(Contour2Dd* contour)
{
#if 1
	// this contour: 1 --> 2
	// other contour: 3 --> 4
	int this_size = _segments.size();
	int csize = contour->getNumberOfSegment();
	if (this_size > 0 && csize > 0) {
		Point2Dd p1 = getFirstSegment()->getFirstPoint();
		Point2Dd p2 = getLastSegment()->getSecondPoint();
        Point2Dd p3 = contour->getFirstSegment()->getFirstPoint();
        Point2Dd p4 = contour->getLastSegment()->getSecondPoint();

        if (1==csize)
        {
            bool isToFisrtPoint, isToFisrtPoint2;
            double dist1 = contour->getFirstSegment()->getSquareDistanceToPoint(p1, isToFisrtPoint);
            double dist2 = contour->getFirstSegment()->getSquareDistanceToPoint(p2, isToFisrtPoint2);

            if (dist1 > dist2)
            {
                if (!p2.isConcident(isToFisrtPoint2?p3:p4)) {
                    Segment2Dd* newSegment = new Segment2Dd(p2, isToFisrtPoint2?p3:p4);
                    //addSegment(newSegment);
                    _segments.push_back(newSegment);
                }

                if (!isToFisrtPoint2)
                    contour->getSegment(0)->swapEndPoints();
                
                //addSegment(contour->getSegment(0));
                _segments.push_back(contour->getSegment(0));
            }
            else
            {
                if (!p1.isConcident(isToFisrtPoint?p3:p4)) {
                    Segment2Dd* newSegment = new Segment2Dd(isToFisrtPoint?p3:p4, p1);
                    //contour->addSegment(newSegment);
                    _segments.insert(_segments.begin(), newSegment);
                }

                if (isToFisrtPoint)
                    contour->getSegment(0)->swapEndPoints();

                _segments.insert(_segments.begin(), contour->getSegment(0));
                //_segments.insert(_segments.begin(), contour->segments().begin(), contour->segments().end());
            }
        }
        else
        {
		    double dis23 = p2.squareDistanceToPoint(p3);
		    double dis41 = p4.squareDistanceToPoint(p1);
		    if (dis23 < dis41) {// connect 2 to 3
			    if (!p2.isConcident(p3)) {
				    Segment2Dd* newSegment = new Segment2Dd(p2, p3);				    
				    _segments.push_back(newSegment);
                    //addSegment(newSegment);
			    }
			    
                _segments.insert(_segments.end(), contour->segments().begin(), contour->segments().end());
                //for (unsigned i = 0; i < contour->getNumberOfSegment() ; i++) {
				   // Segment2Dd* segment = contour->getSegment(i);
				   // _segments.push_back(segment);
				   // //addSegment(segment);
			    //}
		    } else {
			    if (!p4.isConcident(p1)) {
				    Segment2Dd* newSegment = new Segment2Dd(p4, p1);
				    //contour->addSegment(newSegment);
				    _segments.insert(_segments.begin(), newSegment);
			    }

                _segments.insert(_segments.begin(), contour->segments().begin(), contour->segments().end());			    
                //  std::vector<Segment2Dd*> tmp = _segments;
			    //_segments.clear();
			    //contour->getSegments(_segments);

			    //for (unsigned i = 0; i < tmp.size() ; i++) {
				   // Segment2Dd* segment = tmp.at(i);
				   // //_segments.push_back(segment);
				   // addSegment(segment);
			    //}
		    }
        }
		//return isClosed();
	}
	_isOpened = !isClosed();
	return !_isOpened;
#else
	// this contour: 1 --> 2
	// other contour: 3 --> 4
	int this_size = _segments.size();
	int csize = contour->getNumberOfSegment();
	if (this_size > 0 && csize > 0) {
		Point2Dd p1 = _segments.at(0)->getFirstPoint();
		Point2Dd p2 = _segments.at(this_size - 1)->getSecondPoint();

		Point2Dd p3 = contour->getSegment(0)->getFirstPoint();
		Point2Dd p4 = contour->getSegment(csize - 1)->getSecondPoint();

		double dis13 = p1.squareDistanceToPoint(p3);
		double dis14 = p1.squareDistanceToPoint(p4);
		double dis23 = p2.squareDistanceToPoint(p3);
		double dis24 = p2.squareDistanceToPoint(p4);
		double min1_34 = min2(dis13, dis14);
		double min2_34 = min2(dis23, dis24);
		if (min1_34 < min2_34) {
			if (min1_34 == dis13) {// connect 1 to 3
				std::vector<Segment2Dd*> tmp = _segments;
				_segments.clear();
				for (int i = contour->getNumberOfSegment() - 1; i >= 0 ; i--) {
					Segment2Dd* segment = contour->getSegment(i);
					segment->invert();
					_segments.push_back(segment);
				}
				for (int i = 0; i < tmp.size(); i++) {
					Segment2Dd* segment = tmp.at(i);
					_segments.push_back(segment);
				}
			} else {// connect 1 to 4
				std::vector<Segment2Dd*> tmp = _segments;
				contour->getSegments(_segments);
				for (int i = 0; i < tmp.size(); i++) {
					Segment2Dd* segment = tmp.at(i);
					_segments.push_back(segment);
				}
			}
		} else {
			if (min2_34 == dis23) {// connect 2 to 3
				for (int i = 0; i < contour->getNumberOfSegment() ; i++) {
					Segment2Dd* segment = contour->getSegment(i);
					_segments.push_back(segment);
				}
			} else {// connect 2 to 4
				for (int i = contour->getNumberOfSegment() - 1; i >= 0 ; i--) {
					Segment2Dd* segment = contour->getSegment(i);
					segment->invert();
					_segments.push_back(segment);
				}
			}
		}
	}
	return true;
#endif
}

void Contour2Dd::getSegments(std::vector<Segment2Dd*>& segments)
{
	segments = _segments;
}

double Contour2Dd::squareDistanceToContour(Contour2Dd* contour)
{
#if 1
    int this_size = _segments.size();
    int csize = contour->getNumberOfSegment();
    if (this_size > 0 && csize > 0) {
        Point2Dd p1 = getFirstSegment()->getFirstPoint();
        Point2Dd p2 = getLastSegment()->getSecondPoint();

        Point2Dd p3 = contour->getFirstSegment()->getFirstPoint();
        Point2Dd p4 = contour->getLastSegment()->getSecondPoint();

        double dis23 = p2.squareDistanceToPoint(p3);
        double dis41 = p4.squareDistanceToPoint(p1);

        double minDis;

        if (csize==1)
        {
            double dis13 = p1.squareDistanceToPoint(p3);
            double dis24 = p2.squareDistanceToPoint(p4);
            minDis = min4(dis23, dis41, dis13, dis24);
        }
        else
        {
            minDis = min2(dis23, dis41);
        }

        return minDis;
    }
    return -1;

	/*int this_size = _segments.size();
	int csize = contour->getNumberOfSegment();
	if (this_size > 0 && csize > 0) {
		Point2Dd p1 = getFirstSegment()->getFirstPoint();
		Point2Dd p2 = getLastSegment()->getSecondPoint();

		Point2Dd p3 = contour->getFirstSegment()->getFirstPoint();
		Point2Dd p4 = contour->getLastSegment()->getSecondPoint();

		double dis23 = p2.squareDistanceToPoint(p3);
		double dis41 = p4.squareDistanceToPoint(p1);

		double minDis = min2(dis23, dis41);
		return minDis;
	}
	return -1;*/
#else
	int this_size = _segments.size();
	int csize = contour->getNumberOfSegment();
	if (this_size > 0 && csize > 0) {
		Point2Dd p1 = _segments.at(0)->getFirstPoint();
		Point2Dd p2 = _segments.at(this_size - 1)->getSecondPoint();

		Point2Dd p3 = contour->getSegment(0)->getFirstPoint();
		Point2Dd p4 = contour->getSegment(csize - 1)->getSecondPoint();

		double dis13 = p1.squareDistanceToPoint(p3);
		double dis14 = p1.squareDistanceToPoint(p4);
		double dis23 = p2.squareDistanceToPoint(p3);
		double dis24 = p2.squareDistanceToPoint(p4);

		double minDis = min4(dis13, dis14, dis23, dis24);
		return minDis;
	}
	return -1;
#endif
}

bool Contour2Dd::connectItself(bool needCheck)
{
	auto this_size = _segments.size();
	if (this_size < 2)
        return false;

	Segment2Dd* firstSegment = getFirstSegment();
	Segment2Dd* lastSegment = getLastSegment();

    if (firstSegment->sp().isConcident(lastSegment->ep()))
    {
        firstSegment->setPrevSegment(lastSegment);
        lastSegment->setNextSegment(firstSegment);
    }
    else
    {
        /*if (!firstSegment->sp().isConcident(lastSegment->ep(), 1.0))
        {
            _isOpened = true;
            return false;
        }*/

		Segment2Dd* newSegment = new Segment2Dd(lastSegment->getSecondPoint(), firstSegment->getFirstPoint());        

        if (needCheck)
        {
            double connectSegLength = newSegment->length();
            if (connectSegLength > EPSILON_VAL_E3)
            {
                double contourLength = 0.0;

                for (unsigned i = 0; i < this_size; ++i)
                {
                    contourLength += _segments[i]->length();
                }

                if (connectSegLength > contourLength/2)
                {
                    delete newSegment;
                    return false;
                }
            }            
            
            /*for (unsigned i = 1; i < this_size-1; ++i)
            {
                if (_segments[i]->isIntersectSegment(newSegment))
                {
                    delete newSegment;
                    return false;
                }                    
            }*/
        }

		_segments.push_back(newSegment);
		firstSegment->setPrevSegment(newSegment);
		lastSegment->setNextSegment(newSegment);
		newSegment->setNextSegment(firstSegment);
		newSegment->setPrevSegment(lastSegment);
    }

	_isOpened = false;
	return true;    
}

double Contour2Dd::squareOpenedDistance()
{
	if (_isOpened) {
		int this_size = _segments.size();
		if (this_size > 0) {
			Segment2Dd* firstSegment = _segments.at(0);
			Segment2Dd* lastSegment = _segments.at(this_size - 1);

			double dis = firstSegment->getFirstPoint().squareDistanceToPoint(lastSegment->getSecondPoint());
			return dis;
		}
	}
	return -1;
}

void Contour2Dd::resetContourSegments()
{
	for (unsigned i = 0; i < _segments.size(); i++) {
		_segments.at(i)->setContour(NULL);
	}
}

void Contour2Dd::rebuildOpenContour()
{
	if (_isOpened) {
		// Find open segment
		Segment2Dd* startSegment = NULL;
		for (int i = 0, size = _segments.size(); i < size; i++) {
			Segment2Dd* segmenti = _segments.at(i);
			Point2Dd fp = segmenti->getFirstPoint();
			bool isFoundNeighbor = false;
			for (unsigned j = i + 1; j < _segments.size(); j++) {
				Segment2Dd* segmentj = _segments.at(j);
				if (fp.isConcident(segmentj->getSecondPoint())) {
					isFoundNeighbor = true;
					break;
				}
			}
			if (isFoundNeighbor == false) {
				startSegment = segmenti;
				break;
			}
		}
		unsigned n = 0;
		std::vector<Segment2Dd*> orientedSegments;
		orientedSegments.push_back(startSegment);
		while (n < _segments.size() - 1) {
			for (int i = 0, size = _segments.size(); i < size; i++) {
				Segment2Dd* segmenti = _segments.at(i);
				if (segmenti == startSegment) {
					continue;
				}

				Point2Dd startPoint2 = startSegment->getSecondPoint();
				Point2Dd thisPoint1 = segmenti->getFirstPoint();
				if (startPoint2.isConcident(thisPoint1)) {
					//n++;
					startSegment = segmenti;
					orientedSegments.push_back(startSegment);
					break;
				} else if (startPoint2.isConcident(segmenti->getSecondPoint())) {
					//segmenti->invert();
					//n++;
					//startSegment = segmenti;
					//orientedSegments.push_back(startSegment);
					//break;
				}
			}
			n++;
		}
		_segments = orientedSegments;
	}
}

bool Contour2Dd::isDiffirentDirection(Contour2Dd* contour)
{
	bool isInside = contour->isInside();

	bool isDiff = _isInside && !isInside || !_isInside&&isInside;

	return isDiff;
}

bool Contour2Dd::isLineContour()
{
	if(_segments.size() > 2) {
//		bool isParallel = true;
		for (unsigned i = 0; i < _segments.size() - 1; i++) {
			Segment2Dd* segi = _segments.at(i);
			Segment2Dd* segi1 = _segments.at(i + 1);
			if (!segi->isParallel(segi1)) {
				return false;
			}
		}
	}
	return true;
}

void Contour2Dd::removeConcidentItersectPoint(std::vector<ISPoint2D>& ips,
        std::vector<ISPoint2D>& new_ips)
{
	if (ips.size() > 1) {
		std::sort(ips.begin(), ips.end());
//		int count = 0;

		for (unsigned i = 0; i < ips.size() - 1; i++) {
			ISPoint2D ipi = ips.at(i);
			ISPoint2D ipi1 = ips.at(i + 1);

			Segment2Dd* segi = ipi.segment;
			Segment2Dd* segi1 = ipi1.segment;

			if (ipi.ip.isXCoincident(ipi1.ip)) {
				bool isContinue = false;
				//Point2Dd otherPointi = segi->getOtherPoint(ipi.ip);
				//Point2Dd otherPointi1 = segi1->getOtherPoint(ipi1.ip);
				Point2Dd otherPointi = segi->getFarPoint(ipi.ip);
				Point2Dd otherPointi1 = segi1->getFarPoint(ipi1.ip);

				double producti = ipi.ip._vcoords[1] - otherPointi._vcoords[1];
				double producti1 = ipi1.ip._vcoords[1] - otherPointi1._vcoords[1];
				//if (producti * producti1 < 0 || fabs(producti) < EPSILON_VAL_ || fabs(producti1) < EPSILON_VAL_) {
				if (producti * producti1 < 0) {
					isContinue = true;
				}

				// Ignore if 2 segments are on the same line
				//if (segi->isParallel(segi1) && segi->isSameDirection(segi1)) {
				if (segi->isParallel(segi1)) {
					isContinue = true;
				}

				if (isContinue) {
					if (i == ips.size() - 2) {
						new_ips.push_back(ipi1);
					}
					continue;
				}
			}
			new_ips.push_back(ipi);
			if (i == ips.size() - 2) {
				new_ips.push_back(ipi1);
			}
		}
	}
}
bool Contour2Dd::intersectXLine( std::vector<ISPoint2D>& ips,
                                 Line2Dd* xLine)
{
	for (int i = 0, size = getNumberOfSegment(); i < size; i++) {
		Segment2Dd* segment = getSegment(i);
		xLine->xLineIntersectSegment(ips, segment);
	}

	if(ips.size() > 1) {
		// Remove coincident points
		std::vector<ISPoint2D> new_ips;
		removeConcidentItersectPoint(ips, new_ips);
		std::vector<ISPoint2D> new_ips2 = new_ips;

		// Force 2 ips when ip is the end point
		bool isNeedToAdd = false;
		for (unsigned i = 0; i < new_ips.size() ; i++) {
			ISPoint2D ipi = new_ips.at(i);
			Segment2Dd* segi = ipi.segment;
			if (segi->getFirstPoint().isConcident(ipi.ip)) {
				Segment2Dd* preSegment = segi->prevSegment();
				ISPoint2D newIpi(ipi.ip, preSegment);
				new_ips2.push_back(newIpi);
				isNeedToAdd = true;
			} else if (segi->getSecondPoint().isConcident(ipi.ip)) {
				Segment2Dd* nextSegment = segi->nextSegment();
				ISPoint2D newIpi(ipi.ip, nextSegment);
				new_ips2.push_back(newIpi);
				isNeedToAdd = true;
			}
		}

		if (isNeedToAdd) {
			std::vector<ISPoint2D> new_ips3;
			removeConcidentItersectPoint(new_ips2, new_ips3);
			ips = new_ips3;
		} else {
			ips = new_ips2;
		}
		//return ips.size();
	}
	if (!_isOpened) {
		//return ips.size()%2 == 0;
		return (!ips.empty() && ips.size()%2 == 0);
	} else {
		return !ips.empty();
	}
}

#if 0
void Contour2Dd::checkInsideContour(std::vector<Contour2Dd*>& contours, const double & z)
{
	// Check the contour is inside or not, in the case the shell is hollow, it just has one contour
	//if (contours.size() >= 2) {
	if (contours.size() >= 1) {
		bool isSwapped;

		// Chose an outside point
		for (unsigned i = 0; i < contours.size(); i++) {
			Contour2Dd* contour = contours.at(i);
			double maxx, maxy;
			contour->getMaxBoundingBox(maxx, maxy);
			Point2Dd outsidePoint = Point2Dd(maxx + 1, maxy + 1);

			// Get nearest segment
			Segment2Dd* seg = contour->getSegment(0);
			std::vector<Segment2Dd*> segs;

			double dis = contour->getNearestSegments(outsidePoint, segs);

			// If found more than 2 nearest segments
			// assert(segs.size() > 0);
			seg = segs[0];//

			if (segs.size() >=2) {
				Segment2Dd* seg1 = segs[0];
				Segment2Dd* seg2 = segs[1];

				Point2Dd nearestPoint = seg1->getNearestPoint(outsidePoint);

				Point2Dd otherP1 = seg1->getOtherPoint(nearestPoint);
				Point2Dd otherP2 = seg2->getOtherPoint(nearestPoint);

				Point2Dd rayVect = otherP2 - nearestPoint;

				Ray2Dd ray2(nearestPoint, rayVect);
				Point2Dd ip;
				bool isIntersected = ray2.intersectSegment(ip, outsidePoint, otherP1);
				if (isIntersected) {
					double disp1 = outsidePoint.squareDistanceToPoint(otherP1);
					double disp2 = outsidePoint.squareDistanceToPoint(ip);
					if (disp2 < disp1)
						seg = seg2;
				}
			}

			contour->_outsidePoint = outsidePoint;
			//Point2Dd segVector;
			//seg->getVector(segVector);

			isSwapped = seg->swapped();

			Point2Dd v1 = isSwapped ? seg->getSecondPoint() : seg->getFirstPoint()  - outsidePoint;
			Point2Dd v2 = isSwapped ? seg->getFirstPoint()  : seg->getSecondPoint() - outsidePoint;

			Point3Dd v1_3d(v1._vcoords[0], v1._vcoords[1], z);
			Point3Dd v2_3d(v2._vcoords[0], v2._vcoords[1], z);
			Vector3Dd n = v1_3d.crossProduct(v2_3d);
			Vector3Dd unitZVector(0, 0 , 1);
			//Vector3Dd zVector = n.crossProduct(unitZVector);

			// check direction
			double dirVal = unitZVector.scalarProduct(n);
			if (dirVal > 0)
				contour->setInside(true);
			else
				contour->setInside(false);

			//break;// Just need to check one segment of contour
		}
	}
}

#else

void Contour2Dd::markContourInOut(std::vector<Contour2Dd*>& contours)
{
	size_t contourCount = contours.size();
	if (contourCount < 1)
        return;

	for (unsigned i = 0; i < contourCount; i++)
		contours[i]->checkDirection();

    //bool isInsideOther;
    //Contour2Dd *contour;
    //for (unsigned i = 0; i < contourCount; ++i)
    //{
    //    contour = contours[i];
    //    contours[i]->checkDirection();
    //    
    //    // Check if inside contour is really inside another contour
    //    contour->setInverted(false);
    //    if (!contour->isInside())
    //        continue;

    //    isInsideOther = false;
    //    for (unsigned k=0; k < contourCount; ++k)
    //    {
    //        if (k != i && contour->isInsideContour(contours[k]))
    //        {               
    //            isInsideOther = true;
    //            break;
    //        }
    //    }

    //    if (!isInsideOther)
    //    {
    //        contour->setInverted(true);
    //        //contour->setInside(false);
    //    }
    //}
}

#endif

void Contour2Dd::connectContours(std::vector<Contour2Dd*>& contours)
{
	std::vector<Contour2Dd*> openedContours;
	for (int j = (int)contours.size()-1; j>=0; --j)
    {
		Contour2Dd* contour = contours[j];
		if (contour->isOpened())
        {
			openedContours.push_back(contour);
            contours.erase(contours.begin()+j);
		}        
	}

	std::vector<Contour2Dd*> newClosedContours;
	while (openedContours.size() > 0)
    {
		if (openedContours.size() == 1)
        {
			Contour2Dd* contour = openedContours[0];
			if (!contour->isOpened())
                assert(0);
            
			if (contour->getNumberOfSegment() >= 2)
            {
				contour->connectItself(true);
				newClosedContours.push_back(contour);
			}
            else
            {
                openedContours.clear();
                delete contour;                    
            }				

            break;
		}		
		
		double minDisSelf = (std::numeric_limits<double>::max)();
        Contour2Dd* toCloseContour = NULL;
            
        unsigned toCloseId = 0;
        toCloseContour = openedContours[0];
        for (unsigned k = 1; k < openedContours.size(); ++k) {                
            if (toCloseContour->getNumberOfSegment() < openedContours[k]->getNumberOfSegment()) {
                toCloseContour = openedContours[k];
                toCloseId = k;
            }
        }
        openedContours.erase(openedContours.begin()+toCloseId);

        while (!toCloseContour->isClosed())
        {
            minDisSelf = toCloseContour->squareOpenedDistance();

            unsigned nearestId = 0;
            Contour2Dd* contour1 = nullptr;
            double minDis2 = (std::numeric_limits<double>::max)();
            for (unsigned k = 0; k < openedContours.size(); k++) {
                Contour2Dd* contourk = openedContours.at(k);
                /*if (contourk==minOpenDisContour)
                    continue;*/

                double disjk = toCloseContour->squareDistanceToContour(contourk);
                if (disjk < minDis2) {
                    minDis2 = disjk;
                    contour1 = contourk;
                    nearestId = k;
                }
            }

            //if (minDisSelf < minDis2 && toCloseContour->connectItself(true)) {
			if (minDisSelf <= minDis2)
            {
				toCloseContour->connectItself(true);
				newClosedContours.push_back(toCloseContour);
                break;
			}
            else
            {
				toCloseContour->connectTo(contour1);
				openedContours.erase(openedContours.begin()+nearestId);

				if (!toCloseContour->isOpened())
                {
					newClosedContours.push_back(toCloseContour);
				}
			}
        }
	}

	if (newClosedContours.size() > 0)
    {
		contours.insert(contours.end(), newClosedContours.begin(), newClosedContours.end());
	}
}

void Contour2Dd::checkDirection()
{
	size_t n = _segments.size();
	if (n < 3) {
		// assert("this is not a contour" == "0");
		return;
	}
	Point2Dd v1,v2;
	Segment2Dd* seg = _segments[0];
	Point2Dd p0 = seg->getFirstPoint();
	seg->getFirstCoord(v2);
	v2 -= p0;
	double total = 0.0;
	n--;
	for(size_t i = 1; i < n; ++i) {
		seg = _segments[i];
		v1 = v2;
		seg->getSecondCoord(v2);
		v2 -= p0;
		total += v1.crossProduct(v2);
	}
	if (total > 0) {
		_isInside = false;
	} else {
		_isInside = true;
	}
}

bool Contour2Dd::isHollow()
{
	return _isHollow;
}

void Contour2Dd::setHollow(const bool& isHollow)
{
	_isHollow = isHollow;
}

void Contour2Dd::setParent(void* parrent)
{
	_parrent = parrent;
}

void* Contour2Dd::getParrent()
{
	return _parrent;
}

bool Contour2Dd::hasOpenSegment()
{
	if (_isOpened) {
		return getFirstSegment()->isOpenAtStartPoint();
	}
	return false;
}

double Contour2Dd::getBoundingBoxArea()
{
	double minX, minY, maxX, maxY;
	getMinBoundingBox(minX, minY);
	getMaxBoundingBox(maxX, maxY);

	return ((maxX - minX) * (maxY - minY));
}

Contour2Dd* Contour2Dd::getBigestContour(std::vector<Contour2Dd*> contours)
{
	double bigestArea = 0;
	Contour2Dd* bigestCon = NULL;
	for (unsigned i = 0; i < contours.size(); i++) {
		Contour2Dd* con = contours.at(i);
		if (i == 0) {
			bigestArea = con->getBoundingBoxArea();
			bigestCon = con;
		} else {
			double areai = con->getBoundingBoxArea();
			if (areai > bigestArea) {
				bigestArea = areai;
				bigestCon = con;
			}
		}
	}

	return bigestCon;
}

bool Contour2Dd::isPointInside(const Point2Dd& p)
{
    unsigned n = _segments.size();
    Segment2Dd* seg;
    double x1,x2,y1,y2,y;
    int nint = 0,j;
    for(unsigned i = 0; i < n; ++i)
    {
        seg = _segments[i];
        seg->sp();
        seg->ep();
        x1 = seg->sp().x;
        y1 = seg->sp().y;
        x2 = seg->ep().x;
        y2 = seg->ep().y;

        if (p.y < std::min(y1,y2) - EPSILON_VAL_BIG  ||
            p.y > std::max(y1,y2) + EPSILON_VAL_BIG)
        {
            continue;
        }
        else if(fabs(p.y - y1) < EPSILON_VAL_BIG){
            if(p.x < x1){
                nint++;
            }
        }
        else if(fabs(p.y - y2) < EPSILON_VAL_BIG){
            continue;
        }
        else if (fabs(y1 - y2) < EPSILON_VAL_BIG){
            if (p.x > std::min(x1,x2) && p.x < std::max(x1,x2) ){ // on boundary
                return true;
            }
        }
        else
        {
            double t = (p.y - y1)/(y2-y1);

            if(t < EPSILON_VAL_BIG){
                continue;
            }
            else if(t > (1.0 - EPSILON_VAL_BIG))
            {
                double x = x1 + t*(x2 - x1);
                j = (i+1) % n;
                y = _segments[j]->ep().y;
                
                if((y - y2)* (y1 - y2) > 0){
                }
                else if(x > p.x){
                    nint++;
                }
            }
            else{
                double x = x1 + t*(x2 - x1);
                if(fabs(x - p.x) < EPSILON_VAL_){ // on boundary
                    return true;
                }
                else if (x > (p.x))
                {
                    nint++;
                }
            }
        }
    }
    if(1 == (nint % 2)){
        return true;
    }
    return false;
}

SegmentList::SegmentList(bool deleteItems): _deleteItems(deleteItems)
{
}

SegmentList::~SegmentList()
{
	clear();
}

void SegmentList::clear()
{
	if (_deleteItems)
		for (int i = 0, len = size(); i < len; i++)
			delete (*this)[i];

	Parent::clear();
}

ContourList::ContourList(bool deleteItems): _deleteItems(deleteItems)
{
}

ContourList::~ContourList()
{
	clear();
}

void ContourList::clear()
{
	if (_deleteItems)
		clearData();
    else
	    Parent::clear();
}

void ContourList::clearData()
{
    for (int i = 0, len = size(); i < len; i++)
        delete (*this)[i];

    Parent::clear();
}