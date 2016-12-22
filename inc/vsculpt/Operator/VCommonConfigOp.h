#ifndef VSCULPT_UTIL_CONFIG_H
#define VSCULPT_UTIL_CONFIG_H

#include "VKernel/VScene.h"
#include "VOperator.h"

class VCommonConfigOp : public VOperator
{
public:
	VCommonConfigOp();
	virtual ~VCommonConfigOp();
	static bool poll();
	static VOperator* create();
	virtual void setParam(const QString &name, const QVariant &value);
private:
	VScene *_scene;
};
#endif