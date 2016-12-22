#ifndef VKERNEL_REMESH_OP
#define VKERNEL_REMESH_OP
#include "VKernel/VMeshObject.h"
#include "BMesh/BMesh.h"

using namespace VM;
class BMeshRemeshOp
{
	static int count;
	enum
	{
		REMESH_V_FEATURE = 1 << 0,
		REMESH_V_CORNER = 1 << 1,
		REMESH_V_BOUNDARY = 1 << 2
	};

	enum
	{
		REMESH_E_FEATURE = 1 << 0
	};

public:
	BMeshRemeshOp(BMesh *bm_, BMBvh *bvh_, bool feature = true);
	~BMeshRemeshOp();
	void run();
	void setFeatureReserve(bool val){ _feature_ensure = val; };
	void setIteration(size_t iter){ _iteration = iter; };
private:
	bool isTooShort(BMVert *v0, BMVert *v1);
	bool isTooLong(BMVert *v0, BMVert *v1);
	void begin();
	void end();
	void splitLongEdges();
	void collapseShortEdges();
	void optimizeValence();
	void tangentialSmooth();
	void projectOriginMesh();
	void featureDetect();
private:
	BMesh *_bm;
	BMBvh *_refer_bvh;
	BMesh *ret_bm;
	float _emax, _emin, _eavg, _emaxsqr, _eminsqr;
	float _error;
	size_t   _cd_v_curvature;
	bool  _feature_ensure;
	size_t _iteration;
};
#endif