#include "VKernel/VSceneRenderer.h"

VK_BEGIN_NAMESPACE

VSceneRender* VSceneRender::_instance = nullptr;

VSceneRender* VSceneRender::instance()
{
	if (!_instance){
		_instance = new VSceneRender();
	}
	return _instance;
}

VSceneRender::VSceneRender()
{}

VSceneRender::~VSceneRender()
{}

void VSceneRender::syncUpdate(VContext *context)
{
	std::vector<VObject*> kills;
	for (auto it : _renders){
		ObjectRenderInfo *render = it.second;
		if (!render->active){
			kills.push_back(it.first);
		}
	}

	for (VObject *obj : kills){
		ObjectRenderInfo *subcribe = _renders[obj];
		_renders.erase(obj);
		subcribe->destructor(subcribe->renderer);
		delete subcribe;
	}

	for (auto it : _renders){
		ObjectRenderInfo *render = it.second;
		render->renderer->syncUpdate(context, it.first);
	}
}

void VSceneRender::render(VView3DRegion *region, Eigen::Vector4f(*cullingPlanes)[4])
{
	for (auto it : _renders){
		ObjectRenderInfo *render = it.second;
		render->renderer->onRender(region, cullingPlanes);
	}
}

void VSceneRender::subcribe(VObject *obj, RenderConstructor constructor, RenderDestuctor destructor)
{
	if (_renders.find(obj) == _renders.end()){
		ObjectRenderInfo *sub = new ObjectRenderInfo();
		sub->constructor = constructor;
		sub->destructor = destructor;
		sub->renderer = constructor();
		sub->active = true;
		_renders.insert(std::make_pair(obj,sub));
	}
}

void VSceneRender::unsubcribe(VObject *obj)
{
	auto it = _renders.find(obj);
	if (it != _renders.end()){
		it->second->active = false;
	}
}

VNodeRender * VSceneRender::renderer(VObject *obj)
{
	auto it = _renders.find(obj);
	if (it != _renders.end()){
		return it->second->renderer;
	}
	return nullptr;
}

VK_END_NAMESPACE