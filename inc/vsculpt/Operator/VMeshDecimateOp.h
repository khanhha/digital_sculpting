#ifndef VSCULPT_MESH_DECIMATE_OP_H
#define VSCULPT_MESH_DECIMATE_OP_H
#include "VOperator.h"
#include "VKernel/VMeshObject.h"

class VMeshDecimateOp : public VOperator{
public:
	VMeshDecimateOp();
	~VMeshDecimateOp();

	static bool poll();
	static VOperator* create();
	virtual void execute();
	virtual void cancel();
private:
	vk::VMeshObject *_org_obj;
	vk::VMeshObject *_dcmobj;
};
#endif