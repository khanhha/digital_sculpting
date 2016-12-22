#ifndef CUBE_EDGE
#define CUBE_EDGE
#include "Point3Dd.h"
#include <string>

class BoxVertex;
class BoxPlane;
class BoxEdge {
public:
	BoxEdge(BoxVertex* v1, BoxVertex* v2);
	~BoxEdge();
public:
	//void setPlanes(BoxPlane* plane1, BoxPlane* plane2);
	int calcPlanes();
	Point3Dd getCenterPoint();
	BoxVertex* getVextex(const int& index);
	BoxVertex* getOtherVertex(const BoxVertex* vertex);
	double module();
	BoxVertex* getCommonVertex(BoxEdge* edge);
	BoxVertex* getCommonVertex(BoxEdge* edge1, BoxEdge* edge2);
	bool isContainVertex(BoxVertex* vertex);
	Point3Dd getVector();
	std::string getAxisText();
private:
	BoxVertex* _vextexes[2];
	BoxPlane* _planes[2];
};

#endif