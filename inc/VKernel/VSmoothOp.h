#ifndef VKERNEL_BM_SMOOTH_H
#define VKERNEL_BM_SMOOTH_H
#include "BMesh/BMesh.h"
#include "VKernel/VScene.h"
#include "VKernel/VMeshObject.h"

using namespace VM;
class VSmoothOp
{
	enum LaplaceWeighting
	{
		UNIFORM_WEIGHT,
		COTAGANT_WEIGHT
	};
	enum VFlag
	{
		V_BOUNDARY = 1 << 0
	};
public:
	VSmoothOp(VM::BMesh *bm);
	~VSmoothOp();
	void	run();
	void setIteration(size_t iter);
	void setLambda(float lambda);
private:
	void	mark_locked_vert();
	void	smooth();
	void	compute_weight();
	float	compute_edge_weight(BMEdge *e);
private:
	BMesh *_bm;
	LaplaceWeighting _wtype;
	size_t _iteration;
	float  _lambda;
	size_t _totedge;
	size_t _totvert;
	std::vector<bool>  _vlocked;
	std::vector<float> _eweight;
	std::vector<float> _vweight;
};
#endif