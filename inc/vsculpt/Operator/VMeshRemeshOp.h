#ifndef VSCULPT_MESH_REMESH_OP_H
#define VSCULPT_MESH_REMESH_OP_H
#include "VOperator.h"
#include "VKernel/VMeshObject.h"

class VMeshRemeshOp : public VOperator{
public:
	VMeshRemeshOp();
	~VMeshRemeshOp();

	static bool poll();
	static VOperator* create();
	virtual void execute();
	virtual void cancel();
private:
	vk::VMeshObject *_org_obj;
	vk::VMeshObject *_remesh_obj;
};
#endif