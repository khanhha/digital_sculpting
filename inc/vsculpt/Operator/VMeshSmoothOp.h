#ifndef VSCULPT_VSMOOTH_OP_H
#define VSCULPT_VSMOOTH_OP_H
#include "VOperator.h"
#include "VKernel/VMeshObject.h"
class VMeshSmoothOp : public VOperator{
public:
	VMeshSmoothOp();
	~VMeshSmoothOp();

	static bool poll();
	static VOperator* create();
	virtual void execute();
	virtual void cancel();
private:
	vk::VMeshObject *_obj;
	vk::VMeshObject *_smoothobj;
	std::vector<Eigen::Vector3f> _orgvertco;
};
#endif