#ifndef BOX_PLANE
#define BOX_PLANE

#include <vector>
class Point3Dd;
class BoxEdge;
class BoxVertex;
class BoxPlane{
public:
	BoxPlane(BoxVertex* v1, BoxVertex* v2, BoxVertex* v3, BoxVertex* v4);
	~BoxPlane();
	//void addBoxEdges(BoxEdge* e);
	void setNormal(Point3Dd* normal);
	void setEdges(BoxEdge* e1, BoxEdge* e2, BoxEdge* e3, BoxEdge* e4);
	bool isContainEdges(BoxEdge* e);
	bool isContainVertex(BoxVertex* v);
	BoxVertex* getCommonVertex(BoxPlane* plane1, BoxPlane* plane2);
	BoxEdge* getCommonEdge(BoxPlane* plane);
	BoxEdge** getBoxEdges();
	Point3Dd* getNormal();
	int getIntersectedEdges(BoxPlane* plane1, BoxPlane* plane2, 
		BoxEdge*& edge1, BoxEdge*& edge2, BoxEdge*& edge3);
	BoxVertex* getVertex(const int& index);
private:
	BoxVertex* _vertexes[4];
	//std::vector<BoxEdge*> _edges;
	BoxEdge* _edges[4];
	Point3Dd* _normal;
};

#endif