#include "BoxPlane.h"
#include "BoxVertex.h"
#include "BoxEdge.h"

BoxPlane::BoxPlane(BoxVertex* v1, BoxVertex* v2, BoxVertex* v3, BoxVertex* v4) {
	_vertexes[0] = v1;
	_vertexes[1] = v2;
	_vertexes[2] = v3;
	_vertexes[3] = v4;

	_vertexes[0]->addBoxPlan(this);
	_vertexes[1]->addBoxPlan(this);
	_vertexes[2]->addBoxPlan(this);
	_vertexes[3]->addBoxPlan(this);
	
}

BoxPlane::~BoxPlane() {

}

//
//void BoxPlane::addBoxEdges(BoxEdge* e) {
//	_edges.push_back(e);
//}

void BoxPlane::setEdges(BoxEdge* e1, BoxEdge* e2, BoxEdge* e3, BoxEdge* e4) {
	_edges[0] = e1;
	_edges[1] = e2;
	_edges[2] = e3;
	_edges[3] = e4;
}
void BoxPlane::setNormal(Point3Dd* normal) {
	_normal = normal;
}

bool BoxPlane::isContainEdges(BoxEdge* e) {
	for (int i = 0; i < 4; i++) {
		BoxEdge* ei = _edges[i];
		if (ei == e) {
			return true;
		}
	}
	return false;
}

BoxVertex* BoxPlane::getCommonVertex(BoxPlane* plane1, BoxPlane* plane2) {
	BoxEdge* commonEdge = plane1->getCommonEdge(plane2);
	if (commonEdge != 0) {
		BoxVertex* v1 = commonEdge->getVextex(0);
		if (this->isContainVertex(v1)) {
			return v1;
		}

		BoxVertex* v2 = commonEdge->getVextex(1);
		if (this->isContainVertex(v2)) {
			return v2;
		}
	}
	return 0;
}

BoxEdge* BoxPlane::getCommonEdge(BoxPlane* plane) {
	BoxEdge** edges = plane->getBoxEdges();
	for (int i = 0; i < 4; i++) {
		BoxEdge* thisEdgei = _edges[i];
		for (int j = 0; j < 4; j++) {
			BoxEdge* edgej = edges[j];
			if (edgej == thisEdgei) {
				return edgej;
			}
		}
	}
	return 0;
}

BoxEdge** BoxPlane::getBoxEdges() {
	return _edges;
}

bool BoxPlane::isContainVertex(BoxVertex* v) {
	for (int i = 0; i < 4; i++) {
		BoxVertex* vi = _vertexes[i];
		if (vi == v) {
			return true;
		}
	}
	return false;
}


Point3Dd* BoxPlane::getNormal() {
	return _normal;
}


int BoxPlane::getIntersectedEdges(BoxPlane* plane1, BoxPlane* plane2, 
	BoxEdge*& edge1, BoxEdge*& edge2, BoxEdge*& edge3) {

		edge1 = this->getCommonEdge(plane1);
		edge2 = plane1->getCommonEdge(plane2);
		edge3 = plane2->getCommonEdge(this);

		return 1;
}

BoxVertex* BoxPlane::getVertex(const int& index) {
	return _vertexes[index];
}