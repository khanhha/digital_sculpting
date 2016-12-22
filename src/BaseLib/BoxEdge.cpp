#include "BoxEdge.h"
#include "BoxVertex.h"
#include "BoxPlane.h"

BoxEdge::BoxEdge(BoxVertex* v1, BoxVertex* v2) {
	_vextexes[0] = v1;
	_vextexes[1] = v2;

	_vextexes[0]->addBoxEdge(this);
	_vextexes[1]->addBoxEdge(this);
}

BoxEdge::~BoxEdge() {

}


//void BoxEdge::setPlanes(BoxPlane* plane1, BoxPlane* plane2) {
//	_planes[0] = plane1;
//	_planes[1] = plane2;
//}

int BoxEdge::calcPlanes() {
	int nplane = 0;
	std::vector<BoxPlane*> planes1 = _vextexes[0]->getPlanes();
	for (unsigned i = 0; i < planes1.size(); i++) {
		BoxPlane* planei = planes1.at(i);
		if (planei->isContainEdges(this)) {
			_planes[nplane++] = planei;
		}
	}
	return nplane;
}

Point3Dd BoxEdge::getCenterPoint() {
	Point3Dd* p1 = _vextexes[0]->getPoint();
	Point3Dd* p2 = _vextexes[1]->getPoint();

	Point3Dd p = ((*p1) + (*p2)) * 0.5;
	return p;
}

BoxVertex* BoxEdge::getVextex(const int& index) {
	return _vextexes[index];
}

BoxVertex* BoxEdge::getOtherVertex(const BoxVertex* vertex) {
	if (_vextexes[0] == vertex) {
		return _vextexes[1];
	} else {
		return _vextexes[0];
	}
}

double BoxEdge::module() {
	double mdl = _vextexes[0]->getPoint()->distance(*_vextexes[1]->getPoint());
	return mdl;
}

BoxVertex* BoxEdge::getCommonVertex(BoxEdge* edge) {
	if (edge->isContainVertex(_vextexes[0])) {
		return _vextexes[0];
	} else if (edge->isContainVertex(_vextexes[1])) {
		return _vextexes[1];
	}
	return 0;
}

BoxVertex* BoxEdge::getCommonVertex(BoxEdge* edge1, BoxEdge* edge2) {
	BoxVertex* commV = this->getCommonVertex(edge1);
	if (commV) {
		BoxVertex* commV2 = this->getCommonVertex(edge2);
		if (commV == commV2) {
			return commV;
		}
	}
	return 0;
}

bool BoxEdge::isContainVertex(BoxVertex* vertex) {
	if (_vextexes[0] == vertex || _vextexes[1] == vertex) {
		return true;
	}
	return false;
}

Point3Dd BoxEdge::getVector() {
	Point3Dd vect = *_vextexes[1]->getPoint() - *_vextexes[0]->getPoint();
	return vect;
}

std::string BoxEdge::getAxisText() {
	Point3Dd ox(1, 0, 0);
	Point3Dd oy(0, 1, 0);
	Point3Dd oz(0, 0, 1);

	Point3Dd vector = this->getVector();
	if (vector.isVectorParallel(ox)) {
		return "X";
	} else if (vector.isVectorParallel(oy)) {
		return "Y";
	} else if (vector.isVectorParallel(oz)) {
		return "Z";
	}
	return "";
}