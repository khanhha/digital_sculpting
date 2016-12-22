#include "VMeshSelfIsectOp.h"
#include "VKernel/VContext.h"
#include "Boolean/bm/bm_isct_bool_include.h"

VMeshSelfIsectOp::VMeshSelfIsectOp()
	:
	_org_obj(nullptr),
	_isct_obj(nullptr)
{}

VMeshSelfIsectOp::~VMeshSelfIsectOp()
{
}

bool VMeshSelfIsectOp::poll()
{
	VScene *scene = VContext::instance()->scene();
	VObject *actObj = scene->activeObject();
	if (actObj){
		VMeshObject *meshobj = dynamic_cast<VMeshObject*>(actObj);
		if (meshobj && meshobj->state() == VMeshObject::STATE_NONE)
			return true;
	}
	return false;
}

VOperator* VMeshSelfIsectOp::create()
{
	return new VMeshSelfIsectOp();
}

void VMeshSelfIsectOp::execute()
{
	VScene  *scene = VContext::instance()->scene();
	BMesh	*bm = nullptr;
	BMBvh   *bvh = nullptr;
	if (_isct_obj){
		scene->deleteObject(_isct_obj);
	}

	if (!_org_obj){
		VObject *actObj = scene->activeObject();
		_org_obj = dynamic_cast<VMeshObject*>(actObj);
	}

	if (_org_obj){
		BMesh *dcbm = bm_isct::resolve_self_isct(_org_obj->getBmesh());

		_isct_obj = new VMeshObject(scene, dcbm);

		scene->addNode(_isct_obj);
		scene->markActiveObject(_isct_obj);

		if (!_org_obj->isRemoved()){
			scene->removeObject(_org_obj);
		}

		VContext::instance()->window()->update();
	}
}

void VMeshSelfIsectOp::cancel()
{
	VScene  *scene = VContext::instance()->scene();
	if (_isct_obj && _org_obj){
		scene->deleteObject(_isct_obj);
		_org_obj->getBmeshBvh()->bvh_full_redraw();
		scene->readdObject(_org_obj);
		_isct_obj = _org_obj = nullptr;
	}
}
