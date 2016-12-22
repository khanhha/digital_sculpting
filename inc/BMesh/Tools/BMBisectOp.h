#ifndef BMESH_BM_BISECT_OP_H
#define BMESH_BM_BISECT_OP_H
#include "BMeshOp.h"
VM_BEGIN_NAMESPACE

class BMBisectOp : public BMeshOp
 {
public:
	BMBisectOp();
	~BMBisectOp();
	virtual void execute();
private:
	Vector3f _plane_co;
	Vector3f _plane_no;
};

VM_END_NAMESPACE
#endif