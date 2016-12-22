#include "BoxVertex.h"
#include "BoxPlane.h"
#include "BoxEdge.h"

BoxVertex::BoxVertex(const Point3Dd& p) {
	_p = new Point3Dd(p);
}

BoxVertex::BoxVertex(Point3Dd* p) {
	_p = p;
}

BoxVertex::~BoxVertex() {
	if (_p) {
		delete _p;
		_p = 0;
	}
}

Point3Dd* BoxVertex::getPoint() {
	return _p;
}

//void BoxVertex::setEdges(BoxEdge* e1, BoxEdge* e2, BoxEdge* e3) {
//	_edges[0] = e1;
//	_edges[1] = e2;
//	_edges[2] = e3;
//}

void BoxVertex::addBoxEdge(BoxEdge* e) {
	_edges.push_back(e);
}

void BoxVertex::addBoxPlan(BoxPlane* plane) {
	_planes.push_back(plane);
}

std::vector<BoxPlane*> BoxVertex::getPlanes() {
	return _planes;
}

std::vector<BoxEdge*> BoxVertex::getEdges() {
	return _edges;
}

BoxEdge* BoxVertex::getOtherEdge(BoxEdge* e1, BoxEdge* e2) {
	for (unsigned i = 0; i < _edges.size(); i++) {
		BoxEdge* ei = _edges.at(i);
		if (ei != e1 && ei != e2) {
			return ei;
		}
	}
	return 0;
}

void BoxVertex::get3Edges(BoxEdge*& e1, BoxEdge*& e2, BoxEdge*& e3) {
	e1 = _edges.at(0);
	e2 = _edges.at(1);
	e3 = _edges.at(2);
}