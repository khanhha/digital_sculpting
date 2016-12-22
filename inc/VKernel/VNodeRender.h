#ifndef VKERNEL_VOBJET_RENDER_H
#define VKERNEL_VOBJET_RENDER_H
#include "VKernel/VKernelCommon.h"

VK_BEGIN_NAMESPACE
class VContext;
class VView3DRegion;
class VNode;
class VNodeRender
{
public:
	VNodeRender(){}
	virtual ~VNodeRender(){}
	virtual void onRender(VView3DRegion *region, Eigen::Vector4f(*cullingPlanes)[4]) = 0;
	virtual void syncUpdate(VContext *context, VNode *object) = 0;
};
VK_END_NAMESPACE
#endif