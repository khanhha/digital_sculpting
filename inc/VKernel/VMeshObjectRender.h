#ifndef VKERNEL_VMESH_OBJECT_RENDER_H
#define VKERNEL_VMESH_OBJECT_RENDER_H
#include <GTGraphics.h>

#include "VKernel/VKernelCommon.h"
#include "VKernel/VScene.h"
#include "VKernel/VNodeRender.h"
#include "VBvh/BMeshBvh.h"
#include "GteWireframeEffect.h"

VK_BEGIN_NAMESPACE

class VMeshObject;
class VView3DRegion;

class VMeshObjectRender : public VNodeRender
{
	struct NodeBufer
	{
		NodeBufer() : nodeIdx(-1), updateGL(true){};
		NodeBufer(const NodeBufer &o) : nodeIdx(o.nodeIdx), updateGL(o.updateGL), vBuffer(o.vBuffer), sphereCenter(o.sphereCenter), sphereRadius(o.sphereRadius) {};

		int									nodeIdx;
		bool								updateGL;
		bool								render;
		std::shared_ptr<gte::VertexBuffer>	vBuffer;
		Eigen::Vector3f						sphereCenter;
		float								sphereRadius;
	};
public:
	VMeshObjectRender();
	~VMeshObjectRender();
	virtual void onRender(VView3DRegion *region, Eigen::Vector4f(*cullingPlanes)[4]);
	virtual void syncUpdate(VContext *context, VNode *node);
	void setRenderMode(VbsDef::RENDER_MODE mode);
private:
	void ensureEffect();
	void updateUniformBuffers(VView3DRegion *region);
	void markRenderNode(VView3DRegion *region, VMeshObject *obj);
private:
	void updateNodeBuffer(VBvh::BMLeafNode *node);
	void updateNodeVertexBufferData(NodeBufer &nodeGL, VBvh::BMLeafNode *node);
private:
	VbsDef::RENDER_MODE								_renderMode;
	std::vector <NodeBufer>							_vNodeBuffers;
	gte::EMatrix4x4									_worldmat;
	gte::EVector4									_lightpos;
	
	std::shared_ptr<gte::PointLightEffect>			_effect;
	std::shared_ptr<gte::WireframeEffect>			_wireEffect;

	std::shared_ptr<gte::Material>					_material;
	std::shared_ptr<gte::Lighting>					_lighting;
	std::shared_ptr<gte::LightCameraGeometry>		_lightGeomery;
};

VK_END_NAMESPACE

#endif