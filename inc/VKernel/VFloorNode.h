#ifndef VKERNEL_VGRID_NODE_H
#define VKERNEL_VGRID_NODE_H
#include "VKernel/VKernelCommon.h"
#include "VKernel/VNode.h"
#include "VKernel/VScene.h"
VK_BEGIN_NAMESPACE
class VFloorNode : public VNode
{
public:
	VFloorNode(VScene *scene);
	virtual ~VFloorNode();
	virtual void onRender(VView3DRegion *region);
private:
	void ensureEffect();
private:
	VScene *_scene;
	std::shared_ptr<gte::VertexColorEffect>	_effect;
	std::shared_ptr<gte::VertexBuffer>			_vBuffer;
	std::shared_ptr<gte::IndexBuffer>			_iBuffer;
	gte::EVector4								_gridColor, _axisXColor, _axisYColor;
};
VK_END_NAMESPACE
#endif