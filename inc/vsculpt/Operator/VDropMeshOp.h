#ifndef VSCULPT_VDROP_MESH_OP_H
#define VSCULPT_VDROP_MESH_OP_H
#include "VOperator.h"
#include "VContext.h"
#include "VKernel/VScene.h"
#include "BMesh/BMesh.h"
#include "VKernel/VMeshObject.h"

class VDropMeshOp : public VOperator
{
public:
	VDropMeshOp();
	virtual ~VDropMeshOp();

	static bool poll();
	static VOperator* create();

	virtual int  invoke(QEvent *ev);
private:
	VM::BMesh*	import_vcg(const QString& filepath);


	QPoint			_oldMouse;
};

#endif