#ifndef VSCULPT_SCCULPT_CONFIG_H
#define VSCULPT_SCCULPT_CONFIG_H

#include "VKernel/VScene.h"
#include "VOperator.h"
#include "Sculpt/VSculptConfig.h"

class VSculptConfigOp : public VOperator
{
public:
	VSculptConfigOp(vk::VScene *scene);
	virtual ~VSculptConfigOp();
	static bool poll();
	static VOperator* create();
	virtual void setParam(const QString &name, const QVariant &value);
private:
	VScene *_scene;
	VSculptConfig *_config;
};
#endif