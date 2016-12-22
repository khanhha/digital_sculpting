#ifndef VKERNEL_VBMESH_NODE_H
#define VKERNEL_VBMESH_NODE_H
#include <GTGraphics.h>
#include <VBvh/BMBvhIsect.h>
#include "VObject.h"
#include "BMesh/BMesh.h"
#include "VScene.h"

VK_BEGIN_NAMESPACE
class VView3DRegion;
class VContext;
class VMeshObject;



class VMeshObject : public VObject
{
public:
	enum State
	{
		STATE_NONE,
		STATE_SCULPT,
		STATE_POSE_BEING_SELECT,
		STATE_POSE_DEFORM
	};
public:
	VMeshObject(VScene *scene, VM::BMesh *bm);
	virtual ~VMeshObject();
	
	virtual void syncUpdate();
	virtual void applyLocalTransform();
	virtual Eigen::Vector3f center();

	VM::BMesh*	 getBmesh();
	VBvh::BMBvh* getBmeshBvh();


	void	setState(State s){ _state = s; };
	State	state(){ return _state; };

	virtual bool pick(const Eigen::Vector3f& org, const Eigen::Vector3f& dir, Eigen::Vector3f& hit);
	virtual void onSceneRemoved();
	virtual void onSceneReadded();
	void setRenderMode(VbsDef::RENDER_MODE mode);

private:
	void subcribeRenderData();
	void unscribeRenderData();
private:
	VScene		*_scene;
	VM::BMesh	*_bmesh;
	VBvh::BMBvh *_bvh;
	State		 _state;
};
VK_END_NAMESPACE

#endif