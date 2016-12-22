#ifndef VKERNEL_VSCENE_RENDER_H
#define VKERNEL_VSCENE_RENDER_H
#include "VKernel/VKernelCommon.h"
#include "VKernel/VContext.h"
#include "VKernel/VNodeRender.h"
#include <map>

VK_BEGIN_NAMESPACE

class VView3DRegion;
class VObject;

class VSceneRender
{
public:
	typedef std::function<VNodeRender*()>		RenderConstructor;
	typedef std::function<void(VNodeRender*)>	RenderDestuctor;
public:
	static VSceneRender *instance();
	~VSceneRender();

	void syncUpdate(VContext *context);
	void render(VView3DRegion *region, Eigen::Vector4f(*cullingPlanes)[4]);

	void subcribe(VObject *obj, RenderConstructor constructor, RenderDestuctor destructor);
	void unsubcribe(VObject *obj);
	VNodeRender *renderer(VObject *obj);
private:
	VSceneRender();
private:
	static VSceneRender *_instance;

	struct ObjectRenderInfo
	{
		RenderConstructor	 constructor;
		RenderDestuctor		 destructor;
		VNodeRender			*renderer;
		bool				 active;
	};
	std::map<VObject*, ObjectRenderInfo*> _renders;
};

VK_END_NAMESPACE
#endif