#include "SkeletonGenerator.h"
#include "util.h"
#include <map>

//extern std::vector<Point2Dd> g_sketch_triangles;
//extern std::vector<std::vector<Point2Dd>*> g_ske_segment;
//extern std::vector<Point2Dd> g_ske_point;
//extern std::vector<Point2Dd> g_debug_segment;
//extern std::vector<Point2Dd> g_debug_segment_1;


namespace BaseType{


    SkeletonGenerator::SkeletonGenerator()
    {

    }


    SkeletonGenerator::~SkeletonGenerator()
    {
        for (auto it = _skePoint.begin(); it != _skePoint.end(); ++it){
            delete (*it);
        }
        _skePoint.clear();

        for (auto it = _skeEdges.begin(); it != _skeEdges.end(); ++it){
            delete (*it);
        }
        _skeEdges.clear();

        for (auto it = _skeTriangles.begin(); it != _skeTriangles.end(); ++it){
            delete (*it);
        }
        _skeTriangles.clear();

		for (auto it = _skeNodes.begin(); it != _skeNodes.end(); ++it)
		{
			delete (*it)->_point;
			delete (*it);
			*it = nullptr;
		}
		_skeNodes.clear();

		for (auto it = _skeLinks.begin(); it != _skeLinks.end(); ++it)
		{
			SkeLink* l = *it;
			std::vector<SkePoint*>& points = l->_points;
			size_t tot = points.size();
			if (tot > 2)
			{
				for (size_t i = 1; i < tot - 1; ++i)
					delete points[i];
			}
			points.clear();
			delete l;
			*it = nullptr;
		}
		_skeLinks.clear();
    }

    int SkeletonGenerator::addContour(const std::vector<Point2Dd*> &contour)
    {
		std::vector<Point2Dd*> localContour;
		
		Point2Dd p0 = *contour[0];
		Point2Dd v0 = *contour[1];
		v0 -= p0;
		Point2Dd v1;
		double total = 0.0;
		for (unsigned i = 2; i < contour.size(); ++i)
		{
			v1 = *contour[i] - p0;
			total += v0.crossProduct(v1);
			v0 = v1;
		}

		if (total > 0.0){
			localContour.assign(contour.rbegin(), contour.rend());
		}
		else{
			localContour.assign(contour.begin(), contour.end());
		}

        SkePointLink *skl0, *skl1, *skl;
        DlnPoint *skp;
		size_t n = localContour.size();
        if (n > 2){
			skp = new DlnPoint(localContour[0]);
            skl0 = new SkePointLink(skp);
            skl = skl0;
            _skePoint.push_back(skp);
            for (unsigned i = 1; i < n; ++i){
				skp = new DlnPoint(localContour[i]);
				_skePoint.push_back(skp);
                skl1 = new SkePointLink(skp);
                skl->setRelation(skl1);
                skl = skl1;
            }
            skl->setRelation(skl0);
        }
        _skeContours.push_back(skl0);
        return 0;
    }


    int SkeletonGenerator::createASkeTriangle(DlnPoint* kp0, DlnPoint* kp1, DlnPoint* kp2)
    {
        DlnTriangle* newt = new DlnTriangle(kp0, kp1, kp2);
        DlnEdge* ed = kp0->findSkeEdge(kp1);
        if (0 == ed){
            ed = new DlnEdge(kp0, kp1);
            kp0->addEdge(ed);
            kp1->addEdge(ed);
            _skeEdges.push_back(ed);
        }
        newt->setEdge(0, ed);
        ed->addTriangle(newt);

        ed = kp1->findSkeEdge(kp2);
        if (0 == ed){
            ed = new DlnEdge(kp1, kp2);
            kp1->addEdge(ed);
            kp2->addEdge(ed);
            _skeEdges.push_back(ed);
        }
        newt->setEdge(1, ed);
        ed->addTriangle(newt);

        ed = kp2->findSkeEdge(kp0);
        if (0 == ed){
            ed = new DlnEdge(kp2, kp0);
            kp2->addEdge(ed);
            kp0->addEdge(ed);
            _skeEdges.push_back(ed);
        }
        newt->setEdge(2, ed);
        ed->addTriangle(newt);
        _skeTriangles.push_back(newt);
        return 0;
    }


    int SkeletonGenerator::triangulateSkeContour(SkePointLink* kp0, std::deque<SkePointLink*> &removed)
    {
        if (kp0->getNext() == kp0){
            // It's better to not have this odd case, improve later.
            removed.push_back(kp0);
            return 0;
        }

        if (kp0->getNext()->getNext() == kp0->getLast()){
            createASkeTriangle(kp0->getValue(), kp0->getNext()->getValue(), kp0->getLast()->getValue());

            removed.push_back(dynamic_cast<SkePointLink*>(kp0->getNext()));
            removed.push_back(dynamic_cast<SkePointLink*>(kp0->getLast()));
            removed.push_back(kp0);
            return 0;
        }
        SkePointLink* v0, *next, *last;
        Point2Dd* p0, *p1, *p2, *p;
        v0 = findMinAngleSkePoint(kp0);
#ifdef _DEBUG
        if ((v0->getValue()->getCoord()->x - 24.1899) < 0.001 &&
            fabs(v0->getValue()->getCoord()->y - 24.7393) < 0.001)
        {
            g_debug_var = 1;
        }
#endif
        next = dynamic_cast<SkePointLink*>(v0->getNext());
        last = dynamic_cast<SkePointLink*>(v0->getLast());
        p0 = v0->getValue()->getCoord();
        p1 = next->getValue()->getCoord();
        p2 = last->getValue()->getCoord();
        SkePointLink* leftMost = 0;
        double mindis = -EPSILON_VAL_;
        double max_r2 = p0->squareDistanceToPoint(*p1);
        double dis2 = p0->squareDistanceToPoint(*p2);
        if (dis2 > max_r2){
            max_r2 = dis2;
        }
        max_r2 *= 1.01;

        SkePointLink* v = dynamic_cast<SkePointLink*>(next->getNext());
        bool isCoinWithNext = false;
        while (v != last){

            p = v->getValue()->getCoord();
            dis2 = p->squareDistanceToPoint(*p0);
            if (dis2 < max_r2){
                if (BaseType::Util::isPointInsideTriangle2D(p->_vcoords, p0->_vcoords, p1->_vcoords, p2->_vcoords, -1)){
                    dis2 = BaseType::Util::calc2DSquareMinDistancePointSegment(
                        p->_vcoords, p1->_vcoords, p2->_vcoords);
                    if (dis2 > mindis){
                        mindis = dis2;
                        leftMost = v;
                    }
                }// isPointIn
            }// if dis2

            v = dynamic_cast<SkePointLink*>(v->getNext());
        }

        if (leftMost){

            SkePointLink* new_v0 = new SkePointLink(v0->getValue());
            SkePointLink* new_left = new SkePointLink(leftMost->getValue());

            SkePointLink* v_end = dynamic_cast<SkePointLink*>(leftMost->getLast());
            v0->setRelation(leftMost);
            new_v0->setRelation(next);
            v_end->setRelation(new_left);
            new_left->setRelation(new_v0);

            calcBoundaryAngle(v0);
            calcBoundaryAngle(new_v0);
            calcBoundaryAngle(leftMost);
            calcBoundaryAngle(new_left);
            _skeContours.push_back(v0);
            _skeContours.push_back(new_v0);

            return 0;
        }

        SkePointLink* last1 = dynamic_cast<SkePointLink*>(last->getLast());
        SkePointLink* next1 = dynamic_cast<SkePointLink*>(next->getNext());
        if (next1 == leftMost ||
            last1 == leftMost){
            last1->setRelation(next1);
            createASkeTriangle(v0->getValue(), next->getValue(), last->getValue());
            removed.push_back(v0);
            removed.push_back(next);
            removed.push_back(last);
            _skeContours.push_back(last1);
        }
        else{
            last->setRelation(next);
            assert(v0 != next && v0 != last && next != last);
            calcBoundaryAngle(last);
            calcBoundaryAngle(next);
            createASkeTriangle(v0->getValue(), next->getValue(), last->getValue());
            removed.push_back(v0);
            _skeContours.push_back(next);

        }
        return 0;
    }



    SkePointLink* SkeletonGenerator::findMinAngleSkePoint(
        SkePointLink* kp)
    {
        SkePointLink* kp1 = dynamic_cast<SkePointLink*>(kp->getNext());
        SkePointLink* kpmin = kp;
        double angle = kp->getAngle();
        while (kp1 != kp){
            if (!kp1->isBigAngle()){
                if (kpmin->isBigAngle()){
                    kpmin = kp1;
                }
                else if (kp1->getAngle() > kpmin->getAngle()){
                    kpmin = kp1;
                }
            }
            kp1 = dynamic_cast<SkePointLink*>(kp1->getNext());
        }
        return kpmin;
    }



    int SkeletonGenerator::calcBoundaryAngle(SkePointLink* kp)
    {
        Vector2Dd dv0(*kp->getValue()->getCoord());
        dv0 -= *(kp->getLast()->getValue()->getCoord());
        Vector2Dd dv1(*kp->getNext()->getValue()->getCoord());
        dv1 -= *(kp->getValue()->getCoord());
        dv0.normalize();
        dv1.normalize();
        double cosangle = dv0.scalarProduct(dv1);
        kp->setAngle(-cosangle);
        if (dv0.crossProduct(dv1) > 0){
            kp->setBigAngle(true);
        }
        else{
            kp->setBigAngle(false);
        }
        return 0;
    }

    int SkeletonGenerator::createSkeTriangles()
    {
        std::deque<SkePointLink*> skenodes;
        int id = 1;
        filterShortEdge(_skeContours.back());
        for (size_t i = 0; i < _skeContours.size(); ++i){
            calcBoundaryAngleAll(_skeContours.at(i));
        }
        SkePointLink* kp0;

        while (!_skeContours.empty()){
            kp0 = _skeContours.back();
            _skeContours.pop_back();

            triangulateSkeContour(kp0, skenodes);
        } //while
        for (std::deque<SkePointLink*>::iterator it = skenodes.begin();
            it != skenodes.end(); ++it){
            delete (*it);
        }
        skenodes.clear();
        return 0;
    }


    int SkeletonGenerator::filterShortEdge(SkePointLink* kp0)
    {
        SkePointLink* kp = kp0;
        SkePointLink* kp1 = dynamic_cast<SkePointLink*>(kp->getNext());
        double dis;
        while (kp1 != kp0 && kp != nullptr && kp1 != nullptr){
            dis = kp->getValue()->getCoord()->distanceToPoint(*kp1->getValue()->getCoord());
            if (dis < 1e-3){
                kp->setRelation(kp1->getNext());
                delete kp1;
                kp1 = dynamic_cast<SkePointLink*>(kp->getNext());
            }
            else{
                kp = kp1;
                kp1 = dynamic_cast<SkePointLink*>(kp1->getNext());
            }
        } // while
        return 0;
    }


    int SkeletonGenerator::calcBoundaryAngleAll(SkePointLink* kp0)
    {
        //SkePointLink* kp0 = _skeContours.at(0);
        SkePointLink* kp = dynamic_cast<SkePointLink*>(kp0->getNext());
        Point2Dd* p0 = kp0->getValue()->getCoord();
        Point2Dd* p = kp->getValue()->getCoord();
        Vector2Dd dv0(*p);
        dv0 -= *p0;
        dv0.normalize();
        Vector2Dd dv1(*p0);
        dv1 -= *(kp0->getLast()->getValue()->getCoord());
        dv1.normalize();
        double cosangle = dv0.scalarProduct(dv1);
        kp0->setAngle(-cosangle);
        if (dv1.crossProduct(dv0) < 0){
            dynamic_cast<SkePointLink*>(kp->getLast())->setBigAngle(true);
        }
        else{
            dynamic_cast<SkePointLink*>(kp->getLast())->setBigAngle(false);
        }
		dv0 = dv1;
        while (kp != kp0){
#ifdef _DEBUG
            if ((kp->getValue()->getCoord()->x - 24.1899) < 0.001 &&
                fabs(kp->getValue()->getCoord()->y - 24.7393) < 0.001)
            {
                g_debug_var = 1;
            }
#endif
            kp = dynamic_cast<SkePointLink*>(kp->getNext());
            p0 = p;
            p = kp->getValue()->getCoord();
            dv1 = *p;
            dv1 -= *p0;
            dv1.normalize();

            cosangle = dv0.scalarProduct(dv1);
            dynamic_cast<SkePointLink*>(kp->getLast())->setAngle(-cosangle);

            if (dv0.crossProduct(dv1) > 0){
                dynamic_cast<SkePointLink*>(kp->getLast())->setBigAngle(true);
            }
            else{
                dynamic_cast<SkePointLink*>(kp->getLast())->setBigAngle(false);
            }
            dv0 = dv1;
        }

        return 0;
    }

    int SkeletonGenerator::filterSkeTriangle()
    {
        double aveWidth2;// = _averageWidth*_averageWidth;
        DlnTriangle* trig;
        size_t n = _skeTriangles.size();
        for (size_t i = 0; i < n; ++i){
            trig = _skeTriangles.at(i);
            if (3 == trig->getType()){
                trig->filter(aveWidth2);
            }
        }
        return 0;
    }

    int SkeletonGenerator::generateSkeCurveFromSkeEdges()
    {
        if (_skeTriangles.empty()){
            return 0;
        }

		//filterSkeTriangle();

		DlnTriangle* cTrig = nullptr, *nextTrig = nullptr, *backTrig = nullptr;
        while (true){ // while 0

			/*find a seed triangle*/
            for (size_t i = 0; i < _skeTriangles.size(); ++i){
                cTrig = _skeTriangles.at(i);
                if (!cTrig->isChecked() && cTrig->getType() > 0){
                    break;
                }
                cTrig = 0;
            }
            if (0 == cTrig){
                return 0;
            }
			std::deque<DlnTriangle*> tTrigs; /*list of triangle with 3 inner edges*/
            std::deque<Point2Dd*> curve;
            Point2Dd* p2d;
            double coord[2];

			DlnEdge* curEd; /*edge of seed triangle from where we start*/
            DlnEdge* backEd = 0;
            Point2Dd* lastP = 0;

			if (1 == cTrig->getType()){
				/*triangle with one inner edge*/
				/*find an extreme point on the shorter boundary edges*/
				cTrig->getTipVertexCoordOfType1Trig(coord);
                p2d = new Point2Dd(coord[0], coord[1]);
                curve.push_back(p2d);
                curEd = cTrig->getSkeEdgeOfType1Trig();
                if (curEd)
                    curEd->setChecked();
                lastP = p2d;
            }
            else if (2 == cTrig->getType()){
				/*triangle with two inner edges*/
				/*this triangle must lie in the middle of the skeleton curve, so  there no need for extreme point*/
				curEd = cTrig->getUncheckedSkeEdge();
                if (curEd)
                    curEd->setChecked();
                backEd = cTrig->getUncheckedSkeEdge();
                if (backEd)
                    backEd->setChecked();
                backTrig = cTrig;
            }
            else if (3 == cTrig->getType()){
				/*triangle with three inner edges*/
				/*extreme point is the centroid*/
				cTrig->calc2DcenterPoint(coord);
                p2d = new Point2Dd(coord[0], coord[1]);
                curve.push_back(p2d);
                curEd = cTrig->getUncheckedSkeEdge();
                if (curEd)
                    curEd->setChecked();
                backEd = cTrig->getUncheckedSkeEdge();
                if (backEd)
                    backEd->setChecked();
                tTrigs.push_back(cTrig);
                backTrig = cTrig;
                lastP = p2d;
            }
            else{
                assert(0);
            }

            cTrig->setChecked();
            if (0 == curEd){
                continue;
            }

            double dis;
            while (true){
                bool isback = false;
                while (true){

                    for (;;){
						/*append mid-point of current edge to skeleton curve*/
						/*ignore it if the distance to last point so small*/
                        curEd->calc2DMidPoint(coord);
                        p2d = new Point2Dd(coord[0], coord[1]);
						/*last point in 1- or 3-typed triangle*/
                        if (lastP){
                            dis = lastP->squareDistanceToPoint(*p2d);
                        }
                        else{
                            dis = HUGE;
                        }
                        if (dis > EPSILON_VAL_){
                            if (isback){
                                curve.push_front(p2d);
                            }
                            else{
                                curve.push_back(p2d);
                            }
                            lastP = p2d;
                        }
                        else {
                            delete p2d;
                        }

						/*expand the skeleton curve to the next triangle*/
                        nextTrig = curEd->getOtherTriangle(cTrig);
                        assert(nextTrig);
                        
						/*we meet a processed triangle here*/
						if (nextTrig->isChecked() && nextTrig->getType() != 3){
                            if (nextTrig->getType() == 2){
                                curEd = nextTrig->getOtherSkeEdge(curEd);
                                if (curEd){
                                    curEd->calc2DMidPoint(coord);
                                    p2d = new Point2Dd(coord[0], coord[1]);
                                    if (lastP){
                                        dis = lastP->squareDistanceToPoint(*p2d);
                                    }
                                    else{
                                        dis = HUGE;
                                    }
                                    if (dis > EPSILON_VAL_){
                                        if (isback){
                                            curve.push_front(p2d);
                                        }
                                        else{
                                            curve.push_back(p2d);
                                        }
                                        lastP = p2d;
                                    }
                                    else{
                                        delete p2d;
                                    }
                                }
                            }
                            break;
                        }
                        else{
                            if (2 == nextTrig->getType()){
								/*get the last unprocessed inner edge*/
                                nextTrig->setChecked();
                                cTrig = nextTrig;
                                curEd = nextTrig->getUncheckedSkeEdge();
                                if (0 == curEd){
                                    break;
                                }
                                assert(curEd);
                                curEd->setChecked();
                            }
                            else if (3 == nextTrig->getType()){
								/*append the centroid to the skeleton curve*/
                                nextTrig->calc2DcenterPoint(coord);
                                p2d = new Point2Dd(coord[0], coord[1]);
                                if (lastP){
                                    dis = lastP->squareDistanceToPoint(*p2d);
                                }
                                else{
                                    dis = HUGE;
                                }
                                if (dis > EPSILON_VAL_){
                                    if (isback){
                                        curve.push_front(p2d);
                                    }
                                    else{
                                        curve.push_back(p2d);
                                    }
                                    lastP = p2d;
                                }
                                else{
                                    delete p2d;
                                }

								/*find the other inner edges for further processing*/
                                if (nextTrig->isChecked()){
                                    break;
                                }
                                else{
                                    tTrigs.push_back(nextTrig);
                                    cTrig = nextTrig;
                                    cTrig->setChecked();
                                    curEd = cTrig->getUncheckedSkeEdge();
                                    if (curEd){
                                        curEd->setChecked();
                                    }
                                    else{
                                        break;
                                    }
                                }
                            }
                            else if (1 == nextTrig->getType()){
								/*append the midpoint of shorter boundary edge to the skeleton curve*/
                                nextTrig->getTipVertexCoordOfType1Trig(coord);
                                p2d = new Point2Dd(coord[0], coord[1]);
                                if (lastP){
                                    dis = lastP->squareDistanceToPoint(*p2d);
                                }
                                else{
                                    dis = HUGE;
                                }
                                if (dis > EPSILON_VAL_){
                                    if (isback){
                                        curve.push_front(p2d);
                                    }
                                    else{
                                        curve.push_back(p2d);
                                    }
                                    lastP = p2d;
                                }
                                else{
                                    delete p2d;
                                }
                                nextTrig->setChecked();
                                break;
                            }
                            else {
                                assert(0);
                            }
                        }
                    }// for(;;)

                    if (backEd){
                        curEd = backEd;
                        backEd = 0;
                        cTrig = backTrig;
                        backTrig = 0;
                        isback = true;
                        if (curve.empty()){
                            lastP = 0;
                        }
                        else
                        {
                            lastP = curve.at(0);
                        }
                    }
                    else{
                        int cur_size = (int)curve.size();
                        if (cur_size > 1){
                            std::vector<Point2Dd*>* curve1 = new std::vector<Point2Dd*>(curve.begin(), curve.end());
                            _centerCurves.push_back(curve1);
                        }
                        else{
                            assert(0);
                        }
                        curve.clear();
                        break;
                    }
                } // while 2

				/*process inner triangle (one with 3 inner edges)*/
                if (tTrigs.empty()){
                    break;
                }
                else{
                    backEd = 0;
                    backTrig = 0;
                    curEd = 0;
                    while (!tTrigs.empty()){
                        cTrig = tTrigs.back();
                        tTrigs.pop_back();
                        curEd = cTrig->getUncheckedSkeEdge();
                        if (curEd){
                            break;
                        }
                    }
                    if (curEd){
                        cTrig->calc2DcenterPoint(coord);
                        p2d = new Point2Dd(coord[0], coord[1]);
                        curve.push_back(p2d);
                        curEd->setChecked();
                        lastP = p2d;
                    }
                    else{
                        break;
                    }
                }

            } // while 1
        } // while 0
        return 0;
    }


    void SkeletonGenerator::generateSkeleton(const std::vector<Point2Dd*> &contour,
        std::vector<std::vector<Point2Dd*>*>& centerCurves)
    {
		if (contour.size() > 2)
		{
			addContour(contour);
			createSkeTriangles();
			generateSkeCurveFromSkeEdges();
			centerCurves.assign(_centerCurves.begin(), _centerCurves.end());
		}
    }

	void SkeletonGenerator::generateSkeleton(const std::vector<Point2Dd*> &contour)
	{

//#define DEBUG_OUTPUT

		if (contour.size() < 3) return;

		_contourLength = calculateContourLength(contour);

		addContour(contour);
		createSkeTriangles();
		makeDelaunayTriangulation();

#ifdef DEBUG_OUTPUT
		debugOutputContour(contour);
		debugOutputTriangle();
#endif

		createSkeNodes();
		createSkeLinks();

		correctSkePointChordalAndRadius();
		calcSkeLinkLengthInfor();
		_skeletonLength = calculateSkeletonLength();

		filterKeyPointsLinks();

#ifdef DEBUG_OUTPUT
		debugOutputLinkChordals();
		debugOutputLink();
#endif

		filterBranchNodes();

		mergeShortBranchLink();

		simplifyLinks();

		removeSmallLinkTips();

	}

	void SkeletonGenerator::calcSkeNodePoint(SkeNode* node)
	{
		DlnTriangle* trig = node->_trig;
		assert(isExtremeTriangle(trig));
		/*1 inner edge, two boudnary edges*/
		if (trig->getType() == 1)
		{
#ifdef USING_SHORTER_EDGE
			/*calc node point as the mid point of shorter edge*/
			double minSqrLength = DBL_MAX;
			DlnEdge* minEdge = NULL;
			for (unsigned i = 0; i < 3; ++i)
			{
				DlnEdge* edge = trig->getEdge(i);
				double sqrLen = edge->calc2DSquareLength();
				if (!edge->isNotBoundaryEdge() && sqrLen < minSqrLength)
				{
					minEdge = edge;
					minSqrLength = sqrLen;
				}
			}

			if (minEdge)
			{
				minEdge->calc2DMidPoint(node->_point._coord._vcoords);
			}
#else
			trig->calc2DcenterPoint(node->_point->_coord._vcoords);
#endif
		}
		else
		{
			/*three inner edges*/
			trig->calc2DcenterPoint(node->_point->_coord._vcoords);
		}
	}

	void SkeletonGenerator::createSkeNodes()
	{
		/*count the number of extreme Delaunay triangles*/
		_skeNodes.reserve(_skeTriangles.size() / 3);
		for (auto it = _skeTriangles.begin(); it != _skeTriangles.end(); ++it)
		{
			DlnTriangle* trig = *it;
			if (isExtremeTriangle(trig))
			{
				SkeNode* node = new SkeNode();
				node->_trig = trig;
				node->_point = new SkePoint();
				calcSkeNodePoint(node);
				calcNodeRadius(node);
				_skeNodes.push_back(node);
			}
		}
	}

	void SkeletonGenerator::createSkeLinks()
	{
		unsigned totnodes = _skeNodes.size();
		
		std::map<DlnTriangle*, SkeNode*> mapTrigNode;
		for (auto it = _skeNodes.begin(); it != _skeNodes.end(); ++it)
			mapTrigNode.insert(std::make_pair((*it)->_trig, *it));

		for (auto it = _skeTriangles.begin(); it != _skeTriangles.end(); ++it)
		{
			(*it)->recalcType();
			(*it)->setChecked(false);
		}
		for (auto it = _skeEdges.begin(); it != _skeEdges.end(); ++it)
		{
			(*it)->setChecked(false);
		}
		while (true)
		{
			/*find an unchecked inner edge to start*/
			DlnEdge* startEdge = NULL;
			SkeNode* startNode = NULL;
			for (unsigned i = 0; i < totnodes; ++i)
			{
				SkeNode* node = _skeNodes[i];
				startEdge = node->_trig->getUncheckedSkeEdge();
				if (startEdge)
				{
					startNode = node;
					break;
				}
			}

			if (startEdge)
			{
				assert(!startEdge->isChecked());
				
				/*create a SKE link from startEdge*/
				/*loop until we met another boundary Delaunay triangle*/
				bool isExtremeTrig = false;
				bool isFinish = false;
				DlnTriangle* curTrig = startNode->_trig;
				DlnEdge* curEdge = startEdge;
				
				SkeLink* curLink = new SkeLink;
				curLink->_node0 = startNode;
				curLink->_points.push_back(startNode->_point);
				_skeLinks.push_back(curLink);
				do 
				{
					curEdge->setChecked();

					SkePoint* point = new SkePoint();
					curEdge->calc2DMidPoint(point->_coord._vcoords);

					point->_cordal0 = *curEdge->getPoint(0)->getCoord();
					point->_cordal1 = *curEdge->getPoint(1)->getCoord();

					curLink->_points.push_back(point);

					/*spread to next triangle and next edges*/
					curTrig = curEdge->getOtherTriangle(curTrig);
					assert(curTrig != NULL);
					isExtremeTrig = isExtremeTriangle(curTrig);
					if (isExtremeTrig)
					{
						/*we finish an skeleton edge here*/
						curLink->_node1 = mapTrigNode[curTrig];
						curLink->_points.push_back(curLink->_node1->_point);
					}

					curEdge = curTrig->getUncheckedSkeEdge();
					isFinish = (curEdge == NULL);
					if (curEdge == NULL)
					{
						size_t gotit = 1;
					}
	
				} while (!isExtremeTrig && !isFinish);

				assert(curLink->_node0 && curLink->_node1);
			}
			else
			{
				/*end process here when cannot find another boundary edges*/
				break; 
			}
		}

		///*add adjacent links to node*/
		for (auto it = _skeLinks.begin(); it != _skeLinks.end(); ++it)
		{
			SkeLink* link = *it;
			link->_node0->_adjLinks.push_back(link);
			link->_node1->_adjLinks.push_back(link);
		}
	}

	void SkeletonGenerator::calcSkeLinkLengthInfor()
	{
		for (auto lit = _skeLinks.begin(); lit != _skeLinks.end(); ++lit)
		{
			SkeLink* link = *lit;
			std::vector<SkePoint*>& points = link->_points;

			updateExtremeNodeChordal(link);

			/*calcualte average chordal length*/
			double avgChordalLength = 0.0;
			double linkLength = 0.0;
			for (auto it = points.begin(); it != points.end(); ++it)
			{
				SkePoint* p = *it;
				avgChordalLength += p->_cordal0.distanceToPoint(p->_cordal1);

				auto next = std::next(it);
				if (next != points.end())
					linkLength += p->_coord.distanceToPoint((*next)->_coord);
			}

			avgChordalLength /= points.size();
			
			link->_avgChordalLength = avgChordalLength;
			link->_length = linkLength;
		}
	}

	/*triangle edges often do not represent correctly the cross width along shape*/
	/*so, in the case where the edge is not perpencular to skeleton, we recalculate chordal*/
	void SkeletonGenerator::correctSkePointChordalAndRadius()
	{
		for (auto it = _skeLinks.begin(); it != _skeLinks.end(); ++it)
		{
			SkeLink* l = *it;
			std::vector<SkePoint*>& points = l->_points;
			size_t totpoints = points.size();
			if (totpoints > 2)
			{
				for (size_t i = 1; i < totpoints - 1; ++i)
				{
					SkePoint* curP = points[i];
					SkePoint* prevP = points[i - 1];
					SkePoint* nextP = points[i + 1];

					Point2Dd axis = (prevP->_coord + curP->_coord) * 0.5 - (curP->_coord + nextP->_coord) * 0.5;
					axis.normalize();
					Point2Dd perpDir(-axis.y, axis.x);
					curP->_radius = std::fabs((curP->_cordal0 - curP->_coord).dot(perpDir));
					curP->_cordal0 =  curP->_coord +  perpDir * curP->_radius;
					curP->_cordal1 =  curP->_coord -  perpDir * curP->_radius;

				}
			}
		}

		for (auto it = _skeLinks.begin(); it != _skeLinks.end(); ++it)
		{
			SkeLink* l = *it;
			size_t tot = l->_points.size();
			if (tot > 2)
			{
				/*set leaf node radius equal to its adjacent node's radius*/
				DlnTriangle* trig0 = l->_node0->_trig;
				DlnTriangle* trig1 = l->_node1->_trig;

				bool isBoundaryTrig = trig0->getType() == 1;
				if (isBoundaryTrig)
					l->_points[0]->_radius = l->_points[1]->_radius;

				isBoundaryTrig = trig1->getType() == 1;
				if (isBoundaryTrig)
					l->_points[tot-1]->_radius = l->_points[tot-2]->_radius;
			}
		}
	}


	/*triangle with two or zero boundary edges
	* such triangles will lie at end of skeleton link
	*/
	bool SkeletonGenerator::isExtremeTriangle(DlnTriangle* trig)
	{
		size_t type = trig->getType();
		if (type == 1 || type == 3)
			return true;
		else
			return false;
	}

	float SkeletonGenerator::calcculateCircumcircleRadius(DlnTriangle* trig)
	{
		double a = trig->getCoord(0)->distanceToPoint(*trig->getCoord(1));
		double b = trig->getCoord(2)->distanceToPoint(*trig->getCoord(1));
		double c = trig->getCoord(2)->distanceToPoint(*trig->getCoord(0));
		return (float)((a*b*c) / sqrt((a + b + c)*(-a + b + c)*(a - b + c)*(a + b - c)));
	}

	void SkeletonGenerator::calculateExtremeNodeCordal(SkeNode* node, const Point2Dd& dir, SkePoint* p)
	{
		double radius = calcculateCircumcircleRadius(node->_trig);
		Point2Dd ortherDir(-dir.y, dir.x);  
		p->_cordal0 = p->_coord + ortherDir * radius;
		p->_cordal1 = p->_coord - ortherDir * radius;
	}

	void SkeletonGenerator::calculateTrapezoid(SkePoint* p0, SkePoint* p1, Trapezoid& trape)
	{
		Point2Dd origin = p0->_coord;
		Point2Dd axis = p1->_coord - p0->_coord;
		Point2Dd ortherDir(-axis.y, axis.x); ortherDir.normalize();
		trape._origin = origin;
		trape._axis = axis;
		auto projectOnDir = [&](const Point2Dd& center, const Point2Dd& p) -> Point2Dd
		{
			return center + ortherDir * (p - center).dot(ortherDir);
		};

		if ((p0->_cordal0 - origin).crossProduct(axis) > 0)
		{
			if ((p1->_cordal0 - origin).crossProduct(axis) > 0)
			{
				trape._positiveSegment[0] = projectOnDir(p0->_coord, p0->_cordal0);
				trape._positiveSegment[1] = projectOnDir(p1->_coord, p1->_cordal0);
				trape._negativeSegment[0] = projectOnDir(p0->_coord, p0->_cordal1);
				trape._negativeSegment[1] = projectOnDir(p1->_coord, p1->_cordal1);
			}
			else
			{
				trape._positiveSegment[0] = projectOnDir(p0->_coord, p0->_cordal0);
				trape._positiveSegment[1] = projectOnDir(p1->_coord, p1->_cordal1);
				trape._negativeSegment[0] = projectOnDir(p0->_coord, p0->_cordal1);
				trape._negativeSegment[1] = projectOnDir(p1->_coord, p1->_cordal0);
			}
		}
		else
		{
			if ((p1->_cordal0 - origin).crossProduct(axis) < 0)
			{
				trape._positiveSegment[0] = projectOnDir(p0->_coord, p0->_cordal1);
				trape._positiveSegment[1] = projectOnDir(p1->_coord, p1->_cordal1);
				trape._negativeSegment[0] = projectOnDir(p0->_coord, p0->_cordal0);
				trape._negativeSegment[1] = projectOnDir(p1->_coord, p1->_cordal0);
			}
			else
			{
				trape._positiveSegment[0] = projectOnDir(p0->_coord, p0->_cordal1);
				trape._positiveSegment[1] = projectOnDir(p1->_coord, p1->_cordal0);
				trape._negativeSegment[0] = projectOnDir(p0->_coord, p0->_cordal0);
				trape._negativeSegment[1] = projectOnDir(p1->_coord, p1->_cordal1);
			}
		}

		if (trape._positiveSegment[0].distanceToPoint(trape._positiveSegment[1]) < FLT_EPSILON || trape._negativeSegment[0].distanceToPoint(trape._negativeSegment[1]) < FLT_EPSILON)
		{
			size_t gotit = 1;
		}
	}
	void SkeletonGenerator::findTrapezoidBestSplit(const std::vector<SkePoint*>& points, SubPoly& poly)
	{
		assert((poly._last - poly._first) > 1);

		Trapezoid trape;
		calculateTrapezoid(points[poly._first], points[poly._last], trape);

		double maxSqrDst = DBL_MIN;
		int maxIdx = -1;
		for (size_t i = poly._first + 1; i != poly._last; ++i)
		{
			SkePoint* p = points[i];
			double sqrDst = 0;
			double area0 = (p->_cordal0 - trape._origin).crossProduct(trape._axis);
			double area1 = (p->_cordal1 - trape._origin).crossProduct(trape._axis);
			
			if (area0 * area1 < 0)
			{
				/*two chordal points are on the opposite side of trapezoid axis*/
				if (area0 > 0 && area1 < 0)
				{
					sqrDst += p->_cordal0.squareDistanceToSegment(trape._positiveSegment[0], trape._positiveSegment[1]);
					sqrDst += p->_cordal1.squareDistanceToSegment(trape._negativeSegment[0], trape._negativeSegment[1]);
				}
				else
				{
					sqrDst += p->_cordal0.squareDistanceToSegment(trape._negativeSegment[0], trape._negativeSegment[1]);
					sqrDst += p->_cordal1.squareDistanceToSegment(trape._positiveSegment[0], trape._positiveSegment[1]);
				}
			}
			else
			{
				/*two chordal points are on the same side of trapezoid axis*/
				bool onPositiveSide = (area0 > 0);
				bool onNegativeSide = (area0 < 0);
				if (
					(onPositiveSide  && abs(area0) > abs(area1)) ||
					(onNegativeSide  && abs(area0) < abs(area1))
					)
				{
					sqrDst += p->_cordal0.squareDistanceToSegment(trape._positiveSegment[0], trape._positiveSegment[1]);
					sqrDst += p->_cordal1.squareDistanceToSegment(trape._negativeSegment[0], trape._negativeSegment[1]);
				}
				else
				{
					sqrDst += p->_cordal0.squareDistanceToSegment(trape._negativeSegment[0], trape._negativeSegment[1]);
					sqrDst += p->_cordal1.squareDistanceToSegment(trape._positiveSegment[0], trape._positiveSegment[1]);
				}
			
			}
			if (sqrDst > maxSqrDst)
			{
				maxSqrDst = sqrDst;
				maxIdx = i;
			}
		}

		poly._index = maxIdx;
		poly._maxDst = sqrt(maxSqrDst);
	}

	/*each branch node have a different chordal pair depending which link*/
	void SkeletonGenerator::updateExtremeNodeChordal(SkeLink* link)
	{
		Point2Dd dir;
		size_t tot = link->_points.size();
		if (tot > 2)
		{
			/*just displace the chordal of the adjacent SKE point*/
			dir = (link->_points[1]->_cordal0 - link->_points[1]->_cordal1) * 0.5;
			link->_points[0]->_cordal0 = link->_points[0]->_coord + dir;
			link->_points[0]->_cordal1 = link->_points[0]->_coord - dir;

			dir = (link->_points[tot - 2]->_cordal0 - link->_points[tot - 2]->_cordal1) * 0.5;
			link->_points[tot-1]->_cordal0 = link->_points[tot-1]->_coord + dir;
			link->_points[tot-1]->_cordal1 = link->_points[tot-1]->_coord - dir;

		}
	}

	void SkeletonGenerator::filterKeyPointsLinks()
	{
		/*calculate average chordal length*/
		for (size_t i = 0; i < _skeLinks.size(); i++)
		{
			if (_skeLinks[i]->_points.size() > 2)
			{
				updateExtremeNodeChordal(_skeLinks[i]);
				filterKeyPointsLink(_skeLinks[i]);
			}
		}
	}

	void SkeletonGenerator::filterKeyPointsLink(SkeLink* link)
	{

		Point2Dd dir;
		std::stack<SubPoly> subpolies;
		size_t tot = link->_points.size();
		std::vector<SkePoint*>& points = link->_points;
		
		double tol = 0.1 * link->_avgChordalLength;

		subpolies.push(SubPoly(0, link->_points.size()-1));
		
		for (auto it = points.begin(); it != points.end(); ++it)
			(*it)->deleted = true;

		/*keep two ending point*/
		points.front()->deleted = false;
		points.back()->deleted = false;

		while (!subpolies.empty())
		{
			SubPoly subpoly = subpolies.top();
			subpolies.pop();

			findTrapezoidBestSplit(points, subpoly);

			if (subpoly._maxDst > tol)
			{
				points[subpoly._index]->deleted = false;
				if (subpoly._index - subpoly._first > 1)
				{
					subpolies.push(SubPoly(subpoly._first, subpoly._index));
				}
				if (subpoly._last- subpoly._index > 1)
				{
					subpolies.push(SubPoly(subpoly._index, subpoly._last));
				}
			}
		}

		points.erase(std::remove_if(points.begin(), points.end(),
			[](SkePoint* p) { return p->deleted; }), points.end());
	}

	/*is pd lies inside the incircle(pa, pb, pc)*/
	bool SkeletonGenerator::isInCircle(Point2Dd& pa, Point2Dd& pb, Point2Dd& pc, Point2Dd& pd)
	{
		double adx = pa.x - pd.x;
		double ady = pa.y - pd.y;
		double bdx = pb.x - pd.x;
		double bdy = pb.y - pd.y;

		double adxbdy = adx * bdy;
		double bdxady = bdx * ady;
		double oabd = adxbdy - bdxady;

		if (oabd <= 0)
			return false;

		double cdx = pc.x - pd.x;
		double cdy = pc.y - pd.y;

		double cdxady = cdx * ady;
		double adxcdy = adx * cdy;
		double ocad = cdxady - adxcdy;

		if (ocad <= 0)
			return false;

		double bdxcdy = bdx * cdy;
		double cdxbdy = cdx * bdy;

		double alift = adx * adx + ady * ady;
		double blift = bdx * bdx + bdy * bdy;
		double clift = cdx * cdx + cdy * cdy;

		double det = alift * (bdxcdy - cdxbdy) + blift * ocad + clift * oabd;

		return det > 0;
	}
	bool SkeletonGenerator::isectSegSeg(const Point2Dd& v1, const Point2Dd& v2, const Point2Dd& v3, const Point2Dd& v4)
	{
		#define CCW(A, B, C) ((C[1] - A[1]) * (B[0] - A[0]) > (B[1]-A[1]) * (C[0]-A[0]))
		return CCW(v1, v3, v4) != CCW(v2, v3, v4) && CCW(v1, v2, v3) != CCW(v1, v2, v4);
		#undef CCW
	}

	bool SkeletonGenerator::isConvexQuad(Point2Dd& a, Point2Dd& b, Point2Dd& c, Point2Dd& d)
	{
		return (isectSegSeg(a, c, b, d));
	}

	void SkeletonGenerator::addEdgesToStack(std::stack<DlnEdge*>& edges, DlnTriangle* trig)
	{
		for (size_t i = 0; i < 3; ++i)
		{
			DlnEdge* e = trig->getEdge(i);
			if (e && !e->isChecked() && e->isNotBoundaryEdge())
			{
				e->setChecked(true);
				edges.push(e);
			}
		}
	}

	void SkeletonGenerator::makeDelaunayTriangulation()
	{
		DlnPoint* verts[4];
		size_t flip_cnt = 0;

		for (auto it = _skeEdges.begin(); it != _skeEdges.end(); ++it)
			(*it)->setChecked(false);

		std::stack<DlnEdge*> edgesStack;
		for (auto it = _skeEdges.begin(); it != _skeEdges.end(); ++it)
		{
			DlnEdge* edge = *it;
			if (!edge->isChecked() && edge->isNotBoundaryEdge())
			{
				edgesStack.push(edge);
				edge->setChecked(true);
			}
		}

		size_t limit_flip = _skePoint.size() * _skePoint.size();

		while (!edgesStack.empty())
		{
			DlnEdge* edge = edgesStack.top();
			edgesStack.pop();
			edge->setChecked(false);
			
			if (collectQuadFromEdge(edge, verts))
			{
				if (isConvexQuad(*verts[0]->getCoord(), *verts[1]->getCoord(), *verts[2]->getCoord(), *verts[3]->getCoord()) && 
					isInCircle1(*verts[0]->getCoord(), *verts[1]->getCoord(), *verts[2]->getCoord(), *verts[3]->getCoord()))
				{
					//if (cnt == 3)
					//{
					//	DlnEdge* e = _skePoint[15]->findSkeEdge(_skePoint[16]);
					//	g_debug_segment.push_back(*edge->getPoint(0)->getCoord());
					//	g_debug_segment.push_back(*edge->getPoint(1)->getCoord());

					//}
					
					flipEdge(edge);

					//if (cnt == 3)
					//{
					//	//yes
					//	for (size_t i = 0; i < _skeEdges.size(); ++i)
					//	{
					//		DlnEdge* e = _skeEdges[i];
					//		if ((e->getPoint(0) == _skePoint[16] && e->getPoint(1) == _skePoint[15]) ||
					//			(e->getPoint(1) == _skePoint[16] && e->getPoint(0) == _skePoint[15]))
					//		{
					//			std::cout << "fdfd";
					//		}
					//	}

					//	//no
					//	for (size_t i = 0; i < _skeTriangles.size(); ++i)
					//	{
					//		bool exist15 = false;
					//		bool exist16 = false;
					//		DlnTriangle* t = _skeTriangles[i];
					//		for (size_t iv = 0; iv < 3; ++iv)
					//		{
					//			if (t->getPoint(iv) == _skePoint[15])
					//				exist15 = true;
					//			if (t->getPoint(iv) == _skePoint[16])
					//				exist16 = true;
					//		}
					//		if (exist15 && exist16)
					//		{
					//			std::cout << "fdfd";
					//		}
					//	}

					//	//no
					//	for (size_t i = 0; i < _skeTriangles.size(); ++i)
					//	{
					//		DlnTriangle* t = _skeTriangles[i];
					//		for (size_t iv = 0; iv < 3; ++iv)
					//		{
					//			DlnEdge* e = t->getEdge(iv);
					//			if ((e->getPoint(0) == _skePoint[16] && e->getPoint(1) == _skePoint[15]) ||
					//				(e->getPoint(1) == _skePoint[16] && e->getPoint(0) == _skePoint[15]))
					//			{
					//				std::cout << "fdfd";
					//			}
					//		}
					//	}

					//	DlnEdge* e = _skePoint[15]->findSkeEdge(_skePoint[16]);
					//	if (!e){
					//		std::cout << "fdfd";
					//	}
					//}

					//if (cnt == 3) return;
					//cnt++;

					flip_cnt++;
					if (flip_cnt > limit_flip) 
						return;

					addEdgesToStack(edgesStack, edge->getTriangle(0));
					addEdgesToStack(edgesStack, edge->getTriangle(1));
				}
			}
		}
	}

	bool SkeletonGenerator::collectQuadFromEdge(DlnEdge* e, DlnPoint* verts[4])
	{
		if (e->isNotBoundaryEdge())
		{
			verts[0] = e->getPoint(0);
			verts[2] = e->getPoint(1);
			verts[1] = e->getTriangle(0)->getOtherPoint(verts[0], verts[2]);
			verts[3] = e->getTriangle(1)->getOtherPoint(verts[0], verts[2]);
			return true;
		}
		else
		{
			return false;
		}
	}

	void SkeletonGenerator::flipEdge(DlnEdge* edge)
	{
		DlnTriangle* trig0 = edge->getTriangle(0);
		DlnTriangle* trig1 = edge->getTriangle(1);
		DlnPoint* v0 = edge->getPoint(0);
		DlnPoint* v1 = edge->getPoint(1);
		DlnPoint* vOpp0 = trig0->getOtherPoint(v0, v1);
		DlnPoint* vOpp1 = trig1->getOtherPoint(v0, v1);

		/*seperate edge and its verts*/
		edge->removeFromPoint();

		/*seperate edge and its triangle*/
		trig0->removeFromEdge();
		trig1->removeFromEdge();
		trig0->removePoints();
		trig1->removePoints();
		
		/*flip edge*/
		vOpp0->addEdge(edge);
		edge->addPoint(vOpp0);
		vOpp1->addEdge(edge);
		edge->addPoint(vOpp1);
		
		/*triangle 0*/
		DlnEdge* e0 = v0->findSkeEdge(vOpp0);
		DlnEdge* e1 = v0->findSkeEdge(vOpp1);
		trig0->setEdges(e0, e1, edge);
		trig0->setPoints(vOpp0, vOpp1, v0);

		/*triangle 1*/
		e0 = v1->findSkeEdge(vOpp0);
		e1 = v1->findSkeEdge(vOpp1);
		trig1->setEdges(e0, e1, edge);
		trig1->setPoints(vOpp1, vOpp0, v1);
	}

	bool SkeletonGenerator::isInCircle1(Point2Dd& pa, Point2Dd& pb, Point2Dd& pc, Point2Dd& pd)
	{
		Point2Dd center;
		double radius;
		calcCircumCircleTriangle(pa, pb, pc, center, radius);
		if (radius > 0)
		{
			return (pd - center).squareModule() < (radius*radius);
		}
		else
			return false;
	}


	void SkeletonGenerator::calcCircumCircleTriangle(const Point2Dd& p0, const Point2Dd& p1, const Point2Dd& p2, Point2Dd& pc, double& r)
	{
		double a;
		double b;
		double bot;
		double c;
		double top1;
		double top2;

		//  Circumradius.
		a = p0.distanceToPoint(p1);// sqrt(pow(t[0 + 1 * 2] - t[0 + 0 * 2], 2) + pow(t[1 + 1 * 2] - t[1 + 0 * 2], 2));
		b = p1.distanceToPoint(p2);//sqrt(pow(t[0 + 2 * 2] - t[0 + 1 * 2], 2) + pow(t[1 + 2 * 2] - t[1 + 1 * 2], 2));
		c = p2.distanceToPoint(p0);//sqrt(pow(t[0 + 0 * 2] - t[0 + 2 * 2], 2) + pow(t[1 + 0 * 2] - t[1 + 2 * 2], 2));

		bot = (a + b + c) * (-a + b + c) * (a - b + c) * (a + b - c);

		if (bot <= 0.0)
		{
			r = -1.0;
			pc[0] = 0.0;
			pc[1] = 0.0;
			return;
		}

		r = a * b * c / sqrt(bot);
		
		//  Circumcenter.
		top1 = (p1.y - p0.y) * c * c - (p2.y - p0.y) * a * a; //top1 = (t[1 + 1 * 2] - t[1 + 0 * 2]) * c * c - (t[1 + 2 * 2] - t[1 + 0 * 2]) * a * a;
		top2 = (p1.x - p0.x) * c * c - (p2.x- p0.x) * a * a;//(t[0 + 1 * 2] - t[0 + 0 * 2]) * c * c - (t[0 + 2 * 2] - t[0 + 0 * 2]) * a * a;
		//bot = (t[1 + 1 * 2] - t[1 + 0 * 2]) * (t[0 + 2 * 2] - t[0 + 0 * 2])
		//	- (t[1 + 2 * 2] - t[1 + 0 * 2]) * (t[0 + 1 * 2] - t[0 + 0 * 2]);
		bot = (p1.y- p0.y) * (p2.x- p0.x)
			- (p2.y -p0.y) * (p1.x- p0.x);

		pc[0] = p0.x + 0.5 * top1 / bot;
		pc[1] = p0.y - 0.5 * top2 / bot;

		return;
	}

	/*remove triangle with two small boundary edges*/
	void SkeletonGenerator::removeRedundant2TypeTriangle()
	{
		double threshold = 0.03 * _contourLength;

		while (true)
		{
			bool found = false;
			for (auto it = _skeTriangles.begin(); it != _skeTriangles.end(); ++it)
			{
				DlnTriangle* trig = *it;
				/*two boundary edges*/
				if (trig->getType() == 1)
				{
					double len = 0.0;
					for (size_t ie = 0; ie < 3; ++ie)
					{
						/*is boundary edge*/
						if (!trig->getEdge(ie)->isNotBoundaryEdge())
						{
							len += trig->getEdge(ie)->calc2DLength();
						}
					}

					if (len < threshold)
					{
						trig->removeFromEdge();
						trig->removePoints();
						delete trig;
						*it = nullptr;
						found = true;
					}
				}
			}

			if (found)
			{			
				/*reorder valid triangles to the first part of triangle vector*/
				auto split = std::partition(
					_skeTriangles.begin(), _skeTriangles.end(),
					[&](DlnTriangle* trig){return trig != nullptr; }
				);
			
				size_t totValidTrig = split - _skeTriangles.begin();
				_skeTriangles.resize(totValidTrig);

				/*recalculate since triangles' type changed after removing*/
				recalcTriangleType();
			}
			else
			{
				break;
			}

		}
	}

	double SkeletonGenerator::calculateContourLength(const std::vector<Point2Dd*> &contour)
	{
		double len = 0.0;
		size_t tot = contour.size();
		for (size_t i = 0; i < tot; ++i)
		{
			size_t inext = (i + 1) % tot;
			len += contour[i]->distanceToPoint(*contour[inext]);
		}

		return len;
	}



	void SkeletonGenerator::simplifyLinks()
	{
		for (auto it = _skeLinks.begin(); it != _skeLinks.end(); ++it)
		{
			SkeLink* link = *it;
			if (link->_points.size() > 2)
			{
				simplifyLink(link);
			}
		}
	}
	
	bool SkeletonGenerator::isCircleCenterIntersectCircle(const Point2Dd& c0, double r0, const Point2Dd& c1, double r1)
	{
		double centerDst = c0.distanceToPoint(c1);
		return (centerDst < r0 + r1 );
	}

	bool SkeletonGenerator::isCircleCenterInsideCirclePercent(const Point2Dd& c0, double r0, const Point2Dd& c1, double r1, double percent)
	{
		double centerDst = c0.distanceToPoint(c1);
		return (centerDst < percent*std::max<double>(r0, r1));
	}

	bool SkeletonGenerator::isCircleCenterInsideCircle(const Point2Dd& c0, double r0, const Point2Dd& c1, double r1)
	{
		double centerDst = c0.distanceToPoint(c1);
		if (centerDst <= r0 || centerDst <= r1)
			return true;
		else
			return false;
	}

	/*filter branch node based on their shape*/
	void SkeletonGenerator::filterBranchNodes()
	{
		std::vector<SkeLink*> smallLimbs;
		for (auto it = _skeNodes.begin(); it != _skeNodes.end(); ++it){
			SkeNode *node = *it;

			if (node->_adjLinks.size() == 3){
				
				double nodeRadius = node->_point->_radius;
				Point2Dd nodeCenter = node->_point->_coord;
				/*we consider an adjacent limb as a bad limb when 
				* - it link one branch node to a leaf node, not to another branch node
				* - their ending circle intersect with the branch node's circle */
				for (size_t i = 0; i < node->_adjLinks.size(); ++i){
				
					SkeLink *link = node->_adjLinks[i];
					SkeNode *otherNode = (node == link->_node0) ? link->_node1 : link->_node0;
					
					assert(otherNode != nullptr);

					if (otherNode && otherNode->_trig->getType() == 1){
						if (isCircleCenterIntersectCircle(nodeCenter, nodeRadius, otherNode->_point->_coord, otherNode->_point->_radius)){ 
							double dst = node->_point->_coord.distance(otherNode->_point->_coord);
							node->_point->_radius = std::max<double>(node->_point->_radius, dst); /*expand the radius of branch node to cover will-be deleted leaf node*/
							smallLimbs.push_back(link); /*save this limb for removal*/
						}
					}
				}
			}
		}

		/*remove all the bad limbs*/
		for (auto it = smallLimbs.begin(); it != smallLimbs.end(); ++it){
			SkeLink * limb = *it;
			SkeNode *n0 = limb->_node0;
			SkeNode *n1 = limb->_node1;
			
			n0->_adjLinks.erase(std::remove_if(n0->_adjLinks.begin(), n0->_adjLinks.end(), [&](SkeLink* l){ return l == limb; }));
			n1->_adjLinks.erase(std::remove_if(n1->_adjLinks.begin(), n1->_adjLinks.end(), [&](SkeLink* l){ return l == limb; }));

			/*remove leaf node*/
			SkeNode* removedNode = (n0->_trig->getType() == 1) ? n0 : n1;
			_skeNodes.erase(std::remove_if(_skeNodes.begin(), _skeNodes.end(), [&](SkeNode* n){return n == removedNode; }));
			delete removedNode->_point;
			delete removedNode;

			/*remove middle SKE points*/
			if (limb->_points.size() > 2)
			{
				for (size_t i = 1; i < limb->_points.size() - 1; ++i)
					delete limb->_points[i];
				limb->_points.clear();
			}
			_skeLinks.erase(std::remove_if(_skeLinks.begin(), _skeLinks.end(), [&](SkeLink* l){return l == limb; }));
			delete limb;
			*it = nullptr;
		}
	}

	void SkeletonGenerator::simplifyLink(SkeLink* link)
	{
		std::vector<SkePoint*>& points = link->_points;
		size_t tot = points.size();
		std::vector<bool> mask(tot, false);
		double overlap_percent = 1.0;
		if (tot < 3) return;

		mask.front() = true;
		mask.back() = true;
		
		int startRange = -1;
		int endRange   = -1;
		
		/*more strict rule with end nodes*/
		SkePoint* firstSphere = points[0];
		SkePoint* lastSphere = points[tot-1];

		/*find the first sphere that do not inside with first sphere
		* we need strict rule in case the first point is branch point
		* as adjacent point to branch point should not lie inside its sphere
		*/
		if (link->_node0->_adjLinks.size() == 3){
			overlap_percent = 0.9;
		}
		else{
			overlap_percent = 0.6;
		}

		for (int i = 1; i < (int)tot - 1; ++i)
		{
			SkePoint* curSphere = points[i];
			if (!isCircleCenterInsideCirclePercent(firstSphere->_coord, firstSphere->_radius, curSphere->_coord, curSphere->_radius, overlap_percent))
			{
				startRange = i;
				break;
			}
		}

		if (startRange != -1)
		{
			/*find last sphere from startRange that do not inside with last sphere*/
			if (link->_node1->_adjLinks.size() == 3){
				overlap_percent = 0.9;
			}
			else{
				overlap_percent = 0.7;
			}
			for (int i = startRange; i < (int)tot - 1; ++i)
			{
				SkePoint* curSphere = points[i];
				if (!isCircleCenterInsideCirclePercent(lastSphere->_coord, lastSphere->_radius, curSphere->_coord, curSphere->_radius, overlap_percent))
				{
					endRange = i;
				}
			}

			/*for middle points, we loosen threshold*/
			overlap_percent = 0.3;

			if (endRange != -1)
			{
				mask[startRange] = true;
				mask[endRange] = true;

				if ((endRange - startRange) > 1)
				{
					std::stack<SubPoly> subpolies;
					subpolies.push(SubPoly(startRange, endRange));
					while (!subpolies.empty())
					{
						SubPoly sub = subpolies.top();
						subpolies.pop();
						SkePoint* firstSphere = points[sub._first];
						SkePoint* lastSphere = points[sub._last];

						/*find the first element that stay outside two extreme spheres*/
						int foundIdx = -1;
						for (size_t i = sub._first + 1; i < sub._last; ++i)
						{
							SkePoint* curSphere = points[i];
							if (!isCircleCenterInsideCirclePercent(firstSphere->_coord, firstSphere->_radius, curSphere->_coord, curSphere->_radius, overlap_percent) &&
								!isCircleCenterInsideCirclePercent(curSphere->_coord, curSphere->_radius, lastSphere->_coord, lastSphere->_radius, overlap_percent)
								)
							{
								foundIdx = i;
								break;
							}
						}
						if (foundIdx != -1)
						{
							mask[foundIdx] = true;
							if (sub._last - foundIdx > 1)
							{
								subpolies.push(SubPoly(foundIdx, sub._last));
							}
						}
					}
				}/**/
			}
		}


		for (size_t i = 0; i < tot; ++i)
		{
			if (!mask[i])
			{
				delete points[i];
				points[i] = nullptr;
			}
		}

		auto split = std::stable_partition(points.begin(), points.end(), [](SkePoint* p){return p != nullptr; });
		if (split != points.end())
		{
			points.resize(split - points.begin());
		}
	}

	/*remove limb tip with so small radius*/
	void SkeletonGenerator::removeSmallLinkTips()
	{
		for (auto it = _skeLinks.begin(); it != _skeLinks.end(); ++it){
			SkeLink *link = *it;
			if (link->_points.size() < 3) continue;

			SkeNode *n0 = link->_node0;
			SkeNode *n1 = link->_node1;
			
			/*n0 is a limb tip, lying on boundary*/
			if (n0->_adjLinks.size() == 1){
				SkePoint *p		=    *link->_points.begin();
				SkePoint *nextP =    *std::next(link->_points.begin());
				if (nextP->_radius > 1.2 * p->_radius){

					/*average position*/
					nextP->_coord = (p->_coord + nextP->_coord) * 0.5;
					/*update n0's point*/
					n0->_point = nextP;
				
					link->_points.erase(link->_points.begin());

					delete p;
				}
			}

			/*n1 is a limb tip, lying on boundary*/
			if (n1->_adjLinks.size() == 1){
				SkePoint *p   =  *std::prev(link->_points.end(), 1);
				SkePoint *prevP =  *std::prev(link->_points.end(), 2);
				if (prevP->_radius > 1.2 * p->_radius){

					/*average position*/
					prevP->_coord = (p->_coord + prevP->_coord) * 0.5;
					
					/*update n0's point*/
					n1->_point = prevP;

					link->_points.pop_back();

					delete p;
				}
			}
		}
	}


	double SkeletonGenerator::calculateSkeletonLength()
	{
		double len = 0;
		for (auto it = _skeLinks.begin(); it != _skeLinks.end(); ++it)
		{
			SkeLink* link = *it;
			std::vector<SkePoint*>& points = link->_points;
			size_t tot = points.size();
			for (int i = 0; i < (int)tot - 1; ++i)
			{
				int inext = i + 1;
				len += points[i]->_coord.distanceToPoint(points[inext]->_coord);
			}
		}

		return len;

	}

	void SkeletonGenerator::debugOutputSkeleton()
	{
		//for (auto it = _skeLinks.begin(); it != _skeLinks.end(); ++it)
		//{
		//	SkeLink* link = *it;
		//	std::vector<SkePoint*>& points = link->_points;
		//	std::vector<Point2Dd>* segment1 = new std::vector<Point2Dd>();
		//	g_ske_segment.push_back(segment1);
		//	for (unsigned i = 0; i < points.size(); ++i)
		//	{
		//		SkePoint* p = points[i];
		//		segment1->push_back(p->_coord);
		//		g_ske_point.push_back(p->_coord);
		//	}
		//}
	}

	void SkeletonGenerator::calcNodeRadius(SkeNode* node)
	{
		DlnTriangle* trig = node->_trig;
		SkePoint* p = node->_point;
		if (trig->getType() == 3)
		{
#ifdef MIN_DST_TO_VERTEX
			double minDist = DBL_MAX;
			for (size_t i = 0; i < 3; ++i)
			{
				double dst = p->_coord.distanceToPoint(*trig->getCoord(i));
				if (minDist > dst)
					minDist = dst;
			}
			p->_radius = minDist;
#else
			double max_dst = DBL_MIN, min_dst = DBL_MAX;
			Point2Dd centroid;
			trig->calc2DcenterPoint(centroid._vcoords);
			for (size_t i = 0; i < 3; ++i){
				double dst = centroid.distanceToPoint(*trig->getCoord(i));
				max_dst = std::max<double>(dst, max_dst);
				min_dst = std::min<double>(dst, min_dst);
			}
			if (min_dst < 0.7 * max_dst){
				/*skinny triangle*/
				p->_radius = min_dst;
			}
			else{
				/*nearly straight triangle*/
				p->_radius = calcculateCircumcircleRadius(trig);
			}
			/*max triangle edge length*/
			//double avgLen = 0.0;
			//for (size_t i = 0; i < 3; ++i)
			//{
			//	avgLen += trig->getEdge(i)->calc2DLength();
			//}
			//p->_radius = avgLen / 3;
#endif
		}
		else
		{
			p->_radius = calcculateCircumcircleRadius(trig);
		}
	}



	void SkeletonGenerator::filterSphere()
	{
		for (auto it = _skeLinks.begin(); it != _skeLinks.end(); ++it)
		{
			SkeLink* link = *it;
			if (link->_points.size() > 2)
				filterSphere(link);
		}
	}

	void SkeletonGenerator::filterSphere(SkeLink* link)
	{
		std::vector<SkePoint*>& points = link->_points;
		size_t tot = points.size();
		
		/*which element should not be removed*/
		std::vector<bool> mask(tot, false);
		mask.front() = true; /*keep*/
		mask.back() = true;  /*keep*/

		std::stack<SubPoly> subpolies;
		subpolies.push(SubPoly(0, tot - 1));
		while (!subpolies.empty())
		{
			SubPoly subpoly = subpolies.top();
			subpolies.pop();

			/*find minimum radius along sub-poly*/
			double minRadius = DBL_MAX;
			size_t minIdx;
			for (size_t i = subpoly._first + 1; i != subpoly._last; ++i)
			{
				SkePoint* p = points[i];
				if (minRadius >= p->_radius)
				{
					minRadius = p->_radius;
					minIdx = i;
				}
			}
		}
	}

	void SkeletonGenerator::recalcTriangleType()
	{
		for (auto it = _skeTriangles.begin(); it != _skeTriangles.end(); ++it)
		{
			DlnTriangle* trig = *it;
			trig->recalcType();
		}
	}

	/*merge intersection branch node*/
	void SkeletonGenerator::mergeShortBranchLink()
	{
		for (auto it = _skeLinks.begin(); it != _skeLinks.end(); ++it)
		{
			SkeLink* link = *it;
			bool isBranchLink = link->_node0->_adjLinks.size() == 3 && link->_node1->_adjLinks.size() == 3;
			if (isBranchLink)
			{
				SkePoint* p0 = link->_node0->_point;
				SkePoint* p1 = link->_node1->_point;
				if (isCircleCenterInsideCirclePercent(p0->_coord, p0->_radius, p1->_coord, p1->_radius, 0.7))
				{
					SkeNode* n0 = link->_node0;
					SkeNode* n1 = link->_node1;
					Point2Dd avgCenter = (p0->_coord  + p1->_coord) * 0.5;
					double avgRadius   = (p0->_radius + p1->_radius) * 0.5;
					n0->_point->_coord = avgCenter;
					n0->_point->_radius = avgRadius;
					SkeLink* sharedLink = link;
					for (auto iit = n1->_adjLinks.begin(); iit != n1->_adjLinks.end(); ++iit)
					{
						SkeLink* movedLink = *iit;
						if (movedLink != sharedLink)
						{
							assert(movedLink->_node0 == n1 || movedLink->_node1 == n1);

							/*replace n1's point by n0's point*/
							if (movedLink->_node0 == n1)
							{
								movedLink->_node0 = n0;
								movedLink->_points.front() = n0->_point;
							}
							else
							{
								movedLink->_node1 = n0;
								movedLink->_points.back() = n0->_point;
							}

							n0->_adjLinks.push_back(movedLink);
						}
					}

					/*delete n1*/
					_skeNodes.erase(std::remove_if(_skeNodes.begin(), _skeNodes.end(), [&](SkeNode* n){ return n == n1; }));
					delete n1->_point;
					delete n1;

					/*detach shared link from n0 and delete it*/
					n0->_adjLinks.erase(std::remove_if(n0->_adjLinks.begin(), n0->_adjLinks.end(), [&](SkeLink* l){ return l == sharedLink;}));
					sharedLink->_node0 = nullptr;
					sharedLink->_node1 = nullptr;
					if (sharedLink->_points.size() > 2)
					{
						for (size_t i = 1; i < sharedLink->_points.size() - 1; ++i)
							delete sharedLink->_points[i];
					}
					sharedLink->_points.clear();
					delete sharedLink;
					*it = nullptr;
				}
			}
		}

		/*clean link vector*/
		auto split = std::stable_partition(_skeLinks.begin(), _skeLinks.end(), [](SkeLink* link){return link != nullptr; });
		_skeLinks.resize(std::distance(_skeLinks.begin(), split));
	}

	void SkeletonGenerator::debugOutputLinkChordals()
	{
		//for (size_t i = 0; i < _skeLinks.size(); ++i){
		//	SkeLink *link = _skeLinks[i];
		//	for (size_t j = 0; j < link->_points.size(); j++){
		//		SkePoint *p = link->_points[j];
		//		g_debug_segment.push_back(p->_cordal0);
		//		g_debug_segment.push_back(p->_cordal1);
		//	}
		//	g_debug_segment.push_back(link->_node0->_point->_cordal0);
		//	g_debug_segment.push_back(link->_node0->_point->_cordal1);
		//	g_debug_segment.push_back(link->_node1->_point->_cordal0);
		//	g_debug_segment.push_back(link->_node1->_point->_cordal1);
		//}
	}

	void SkeletonGenerator::debugOutputLink()
	{
		//for (size_t i = 0; i < _skeLinks.size(); ++i){
		//	SkeLink *link = _skeLinks[i];
		//	size_t tot = link->_points.size();
		//	for (size_t j = 0; j < tot - 1; j++){
		//		g_debug_segment_1.push_back(link->_points[j]->_coord);
		//		g_debug_segment_1.push_back(link->_points[(j + 1) % tot]->_coord);
		//	}
		//}
	}

	void SkeletonGenerator::debugOutputContour(const std::vector<Point2Dd*>& contour)
	{
		//size_t tot = contour.size();
		//for (size_t i = 0; i < tot; ++i){
		//	g_debug_segment.push_back(*contour[i]);
		//	g_debug_segment.push_back(*contour[(i + 1) % tot]);
		//}
	}

	void SkeletonGenerator::debugOutputTriangle()
	{
		//for (size_t i = 0; i < _skeTriangles.size(); ++i){
		//	DlnTriangle *trig = _skeTriangles[i];
		//	for (size_t j = 0; j < 3; ++j){
		//		g_debug_segment.push_back(*trig->getCoord(0));
		//		g_debug_segment.push_back(*trig->getCoord(1));
		//		g_debug_segment.push_back(*trig->getCoord(0));
		//		g_debug_segment.push_back(*trig->getCoord(2));
		//		g_debug_segment.push_back(*trig->getCoord(1));
		//		g_debug_segment.push_back(*trig->getCoord(2));
		//	}
		//}
	}

}