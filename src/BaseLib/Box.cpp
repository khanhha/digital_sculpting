#include "Box.h"
#include "Point3Dd.h"
#include "BoxVertex.h"
#include "BoxEdge.h"
#include "BoxPlane.h"
//Cube::Cube() {
//
//}

Box::Box(BoxVertex* minv, BoxVertex* maxv) {
	_minVertex = minv;
	_maxVertex = maxv;

	calcBasicParams();
}

Box::~Box() {
	//if (_minVertex) {
	//	delete _minVertex;
	//	_minVertex = 0;
	//}

	//if (_maxVertex) {
	//	delete _maxVertex;
	//	_maxVertex = 0;
	//}

	for (int i  = 0; i < 8; i++) {
		BoxVertex* vi = _vertexes[i];
		if (vi) {
			delete vi;
			_vertexes[i] = 0;
		}
	}
			
	if (_vx) {
		delete _vx;
		_vx = 0;
	}

	if (_vy) {
		delete _vy;
		_vy = 0;
	}

	if (_vz) {
		delete _vz;
		_vz = 0;
	}

	//if (_minVertex) {
	//	delete _minVertex;
	//	_minVertex = 0;
	//}

	//if (_maxVertex) {
	//	delete _maxVertex;
	//	_maxVertex = 0;
	//}

	for (int i = 0; i < 12; i++) {
		BoxEdge* edgei = _edges[i];
		if (edgei) {
			delete edgei;
			_edges[i] = 0;
		}
	}

	for (int i = 0; i < 6; i++) {
		BoxPlane* planei = _planes[i];
		if (planei) {
			delete planei;
			_planes[i] = 0;
		}
	}
}

int Box::calcBasicParams() {
	Vector3Dd ox(1, 0, 0);
	Vector3Dd oy(Vector3Dd(0, 1, 0));
	Vector3Dd oz(Vector3Dd(0, 0, 1));

	Point3Dd* minPoint = _minVertex->getPoint();
	Point3Dd* maxPoint = _maxVertex->getPoint();

	_dx = maxPoint->v[0] - minPoint->v[0];
	_dy = maxPoint->v[1] - minPoint->v[1];
	_dz = maxPoint->v[2] - minPoint->v[2];

	_vx = new Vector3Dd(_dx, 0, 0);
	_vy = new Vector3Dd(0, _dy, 0);
	_vz = new Vector3Dd(0, 0, _dz);

	_vertexes[0] = new BoxVertex(minPoint);
	_vertexes[1] = new BoxVertex(*minPoint + *_vx);
	_vertexes[2] = new BoxVertex(*minPoint + *_vy);
	_vertexes[3] = new BoxVertex(*minPoint + (*_vx + *_vy));

	_vertexes[4] = new BoxVertex(*maxPoint - (*_vx + *_vy));
	Point3Dd* upMinPoint = _vertexes[4]->getPoint();
	_vertexes[5] = new BoxVertex(*upMinPoint + *_vx);
	_vertexes[6] = new BoxVertex(*upMinPoint + *_vy);
	_vertexes[7] = new BoxVertex(*upMinPoint + (*_vx + *_vy));
	//_vertexes[5] = new BoxVertex(*maxPoint - *_vx);
	//_vertexes[6] = new BoxVertex(*maxPoint - *_vy);
	//_vertexes[7] = new BoxVertex(*maxPoint);

	// Edge
	_edges[0] = new BoxEdge(_vertexes[0], _vertexes[1]);
	_edges[1] = new BoxEdge(_vertexes[0], _vertexes[2]);
	_edges[2] = new BoxEdge(_vertexes[1], _vertexes[3]);
	_edges[3] = new BoxEdge(_vertexes[2], _vertexes[3]);

	_edges[4] = new BoxEdge(_vertexes[4], _vertexes[5]);
	_edges[5] = new BoxEdge(_vertexes[4], _vertexes[6]);
	_edges[6] = new BoxEdge(_vertexes[5], _vertexes[7]);
	_edges[7] = new BoxEdge(_vertexes[6], _vertexes[7]);

	_edges[8] = new BoxEdge(_vertexes[0], _vertexes[4]);
	_edges[9] = new BoxEdge(_vertexes[1], _vertexes[5]);
	_edges[10] = new BoxEdge(_vertexes[2], _vertexes[6]);
	_edges[11] = new BoxEdge(_vertexes[3], _vertexes[7]);

	// Plane
	_planes[0] = new BoxPlane(_vertexes[0], _vertexes[4], _vertexes[5], _vertexes[1]);
	Vector3Dd* normal0 = new Vector3Dd(-oy);
	_planes[0]->setNormal(normal0);
	_planes[0]->setEdges(_edges[0], _edges[8], _edges[4], _edges[9]);

	_planes[1] = new BoxPlane(_vertexes[0], _vertexes[2], _vertexes[6], _vertexes[4]);
	Vector3Dd* normal1 = new Vector3Dd(-ox);
	_planes[1]->setNormal(normal1);
	_planes[1]->setEdges(_edges[1], _edges[10], _edges[5], _edges[8]);

	_planes[2] = new BoxPlane(_vertexes[0], _vertexes[1], _vertexes[3], _vertexes[2]);
	Vector3Dd* normal2 = new Vector3Dd(-oz);
	_planes[2]->setNormal(normal2);
	_planes[2]->setEdges(_edges[0], _edges[2], _edges[3], _edges[1]);


	_planes[3] = new BoxPlane(_vertexes[7], _vertexes[3], _vertexes[1], _vertexes[5]);
	Vector3Dd* normal3 = new Vector3Dd(ox);
	_planes[3]->setNormal(normal3);
	_planes[3]->setEdges(_edges[11], _edges[2], _edges[9], _edges[6]);

	_planes[4] = new BoxPlane(_vertexes[7], _vertexes[6], _vertexes[2], _vertexes[3]);
	Vector3Dd* normal4 = new Vector3Dd(oy);
	_planes[4]->setNormal(normal4);
	_planes[4]->setEdges(_edges[7], _edges[10], _edges[3], _edges[11]);

	_planes[5] = new BoxPlane(_vertexes[7], _vertexes[5], _vertexes[4], _vertexes[6]);
	Vector3Dd* normal5 = new Vector3Dd(oz);
	_planes[5]->setNormal(normal5);
	_planes[5]->setEdges(_edges[6], _edges[4], _edges[5], _edges[7]);

	// Link edges to planes
	//_edges[0]->setPlanes(_planes[0], _planes[2]);
	//_edges[1]->setPlanes(_planes[1], _planes[0]);
	//_edges[2]->setPlanes(_planes[0], _planes[2]);
	for (int i = 0; i < 12; i++) {
		BoxEdge* edgei = _edges[i];
		edgei->calcPlanes();
	}


	return 1;
}
//
//int Box::buildEdges() {
//	_edges[0] = new BoxEdge(_vertexes[0], _vertexes[1]);
//	_edges[1] = new BoxEdge(_vertexes[0], _vertexes[2]);
//	_edges[2] = new BoxEdge(_vertexes[1], _vertexes[3]);
//	_edges[3] = new BoxEdge(_vertexes[2], _vertexes[3]);
//
//	_edges[4] = new BoxEdge(_vertexes[4], _vertexes[5]);
//	_edges[5] = new BoxEdge(_vertexes[4], _vertexes[6]);
//	_edges[6] = new BoxEdge(_vertexes[5], _vertexes[7]);
//	_edges[7] = new BoxEdge(_vertexes[6], _vertexes[7]);
//
//	_edges[8] = new BoxEdge(_vertexes[0], _vertexes[4]);
//	_edges[9] = new BoxEdge(_vertexes[1], _vertexes[5]);
//	_edges[10] = new BoxEdge(_vertexes[2], _vertexes[6]);
//	_edges[11] = new BoxEdge(_vertexes[3], _vertexes[7]);
//
//	return 1;
//}
//
//int Box::buildPlane() {
//	_planes[0] = new BoxPlane(_vertexes[0], _vertexes[4], _vertexes[5], _vertexes[1]);
//	_planes[1] = new BoxPlane(_vertexes[0], _vertexes[2], _vertexes[6], _vertexes[4]);
//	_planes[3] = new BoxPlane(_vertexes[0], _vertexes[1], _vertexes[3], _vertexes[2]);
//
//	_planes[4] = new BoxPlane(_vertexes[7], _vertexes[3], _vertexes[1], _vertexes[5]);
//	_planes[5] = new BoxPlane(_vertexes[7], _vertexes[6], _vertexes[2], _vertexes[3]);
//	_planes[6] = new BoxPlane(_vertexes[7], _vertexes[5], _vertexes[4], _vertexes[6]);
//
//	return 1;
//}

Point3Dd* Box::getXVect() {
	return _vx;
}

Point3Dd* Box::getYVect() {
	return _vy;
}

Point3Dd* Box::getZVect() {
	return _vz;
}

BoxPlane** Box::getBoxPlanes() {
	return _planes;
}

int Box::getNumberOfPlanes() {
	return 6;
}

BoxVertex** Box::getVertexs() {
	return _vertexes;
}

BoxEdge** Box::getEdges() {
	return _edges;
}