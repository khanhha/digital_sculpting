#include "Operator/VCommonConfigOp.h"
#include "VKernel/VContext.h"
#include "VKernel/VMeshObject.h"
#include "VKernel/VManipulatorNode.h"
#include "VbsQt/VbsDef.h"
using namespace  vk;

VCommonConfigOp::VCommonConfigOp()
{}

VCommonConfigOp::~VCommonConfigOp()
{}

bool VCommonConfigOp::poll()
{
	return VContext::instance()->commonConfig() != nullptr;
}

VOperator* VCommonConfigOp::create()
{
	return new VCommonConfigOp();
}

void VCommonConfigOp::setParam(const QString &name, const QVariant &value)
{
	if (name == "manipulator"){
		VManipulatorNode *manipulator = VContext::instance()->scene()->manipulatorNode();
		if (manipulator){
			int mode = value.toInt();
			manipulator->setMode(mode);
		}
	}
	else if (name == "render_mode"){
		VbsDef::RENDER_MODE mode = static_cast<VbsDef::RENDER_MODE>(value.toInt());
		VObject *obj = VContext::instance()->scene()->activeObject();
		VMeshObject *mobj = dynamic_cast<VMeshObject*>(obj);
		if (mobj){
			mobj->setRenderMode(mode);
		}
	}
}
