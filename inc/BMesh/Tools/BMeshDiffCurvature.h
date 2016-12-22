#ifndef BMESH_DIFF_CURVATURE_H
#define BMESH_DIFF_CURVATURE_H

#include "BMesh/BMesh.h"
#define FIXED_VORONOI_AREA

VM_BEGIN_NAMESPACE

class BMeshDiffCurvature
{
public:
	BMeshDiffCurvature(BMesh *bm, int ncurvature_off);
	~BMeshDiffCurvature();
	void compute();
private:
	void init();
	void computeVertexArea();
	void computeGaussCurvature();
	void computeMeanCurvature();
	void computeEdgeWeight();
	void smoothMeshBegin();
	void smoothMeshEnd();
	void smoothCurvature();
	void outputCurvature();
	void checkVertexBoundary();
private:
	BMesh *_bmesh;
	int	   _curvature_off;
	std::vector<Vector3f> _vOrgCoord;

	/*real-time data*/
	std::vector<bool> _vBoundary;
	std::vector<float> _vArea;
	std::vector<float> _vGaussCurvature;
	std::vector<float> _vMeanCurvature;

	std::vector<float>	  _eWeight;
	std::vector<float>	  _eLength;
	std::vector<Vector3f> _eVec;

	std::vector<Vector3f> _fInternalAngle;
	std::vector<float>    _fArea;
};

VM_END_NAMESPACE

#endif