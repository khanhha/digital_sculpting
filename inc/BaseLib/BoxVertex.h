#ifndef CUBE_VERTEX
#define CUBE_VERTEX

#include "Point3Dd.h"
#include <vector>

class BoxEdge;
class BoxPlane;
class BoxVertex{
public:
	//CubeVertex();
	BoxVertex(const Point3Dd& p);
	BoxVertex(Point3Dd* p);
	~BoxVertex();

	Point3Dd* getPoint();
	//void setEdges(BoxEdge* e1, BoxEdge* e2, BoxEdge* e3);
	void addBoxEdge(BoxEdge* e);
	void addBoxPlan(BoxPlane* e);

	std::vector<BoxPlane*> getPlanes();
	std::vector<BoxEdge*> getEdges();
	BoxEdge* getOtherEdge(BoxEdge* e1, BoxEdge* e2);
	void get3Edges(BoxEdge*& e1, BoxEdge*& e2, BoxEdge*& e3);
private:
	Point3Dd* _p;
	std::vector<BoxEdge*> _edges;
	std::vector<BoxPlane*> _planes;
	
};

#endif