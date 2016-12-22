#include "Operator/VSculptConfigOp.h"
#include "VKernel/VContext.h"
#include "VbsQt/VbsDef.h"
using namespace  vk;

VSculptConfigOp::VSculptConfigOp(vk::VScene *scene)
	:
	_scene(scene),
	_config(scene->sculptConfig())
{}

VSculptConfigOp::~VSculptConfigOp()
{}

bool VSculptConfigOp::poll()
{
	VScene *scene = VContext::instance()->scene();
	return scene->sculptConfig() != nullptr;
}

VOperator* VSculptConfigOp::create()
{
	return new VSculptConfigOp(VContext::instance()->scene());
}

void VSculptConfigOp::setParam(const QString &name, const QVariant &value)
{
	if (name == "brush_radius"){
		_config->setBrushPixelSize(value.toFloat());
	}
	else if (name == "brush_strength"){
		_config->setBrushStrength(value.toFloat());
	}
	else if (name == "detail_size"){
		_config->setBrushDetailsize(value.toFloat());
	}
	else if (name == "dynamic_topology"){
		_config->setBrushDynamicTopology(value.toBool());
	}
	else if (name == "brush"){
		VbsDef::BRUSH btype = static_cast<VbsDef::BRUSH>(value.toInt());
		_config->setBrush(btype);
	}
	else if (name == "curve"){
		VbsDef::CURVE  ctype = static_cast<VbsDef::CURVE>(value.toInt());
		_config->setFalloffCurve(ctype);
	}
}
