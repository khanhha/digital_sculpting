#ifndef VSCULPT_MESH_BOOLEAN_OP_H
#define VSCULPT_MESH_BOOLEAN_OP_H
#include "VOperator.h"
#include "VKernel/VMeshObject.h"

class VMeshBooleanOp : public VOperator{
public:
	VMeshBooleanOp();
	~VMeshBooleanOp();

	static bool poll();
	static VOperator* create();
	virtual void execute();
	virtual void cancel();
private:
	VbsDef::BOOLEAN_TYPE _type;
	vk::VMeshObject		*_hostObj;
	vk::VMeshObject		*_targetObj;
	vk::VMeshObject		*_resultObj;
};
#endif