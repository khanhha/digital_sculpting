#ifndef BMESH_BMESH_TRIANGULATE_H
#define BMESH_BMESH_TRIANGULATE_H
#include "BMesh/BMesh.h"
VM_BEGIN_NAMESPACE
class BMeshTriangulate
{
public:
	BMeshTriangulate(BMesh *bm, const int quad_method, const int ngon_method, const bool tag_only);
	~BMeshTriangulate();
	void run();
private:
	BMesh *_bm;
	const int _quad_method;
	const int _ngon_method;
	const bool _tag_only;
};
VM_END_NAMESPACE
#endif