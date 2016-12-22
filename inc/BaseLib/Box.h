#ifndef PARALLELEPIPPED_VERTEX
#define PARALLELEPIPPED_VERTEX

//#include "Point3Dd.h"
//class Vector3Dd;
class Point3Dd;
class BoxVertex;
class BoxPlane;
class BoxEdge;
class Box {
public:
	//Cube();
	Box(BoxVertex* minv, BoxVertex* maxv);
	//Parallelepiped(ParallelepipedVertex* minv, ParallelepipedVertex* maxv);
	~Box();
public:
	Point3Dd* getXVect();
	Point3Dd* getYVect();
	Point3Dd* getZVect();
	BoxPlane** getBoxPlanes();
	int getNumberOfPlanes();
	BoxVertex** getVertexs();
	BoxEdge** getEdges();
private:
	//int buildEdges();
	//int buildPlane();
	int calcBasicParams();
public:
	BoxVertex* _vertexes[8];
	BoxVertex* _minVertex;
	BoxVertex* _maxVertex;
	BoxEdge* _edges[12];
	BoxPlane* _planes[6];

	double _dx, _dy, _dz;
	Point3Dd* _vx, *_vy, *_vz;
};

#endif