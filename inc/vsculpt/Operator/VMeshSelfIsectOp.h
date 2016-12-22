#ifndef VSCULPT_VMESH_SELF_ISECT_H
#define VSCULPT_VMESH_SELF_ISECT_H

#include "VOperator.h"
#include "VKernel/VMeshObject.h"

class VMeshSelfIsectOp : public VOperator{
public:
	VMeshSelfIsectOp();
	~VMeshSelfIsectOp();

	static bool poll();
	static VOperator* create();
	virtual void execute();
	virtual void cancel();
private:
	vk::VMeshObject *_org_obj;
	vk::VMeshObject *_isct_obj;
};
#endif