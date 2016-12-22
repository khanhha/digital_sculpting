#include "VMeshRemeshOp.h"
#include "VKernel/VContext.h"
#include "VKernel/BMeshRemeshOp.h"

VMeshRemeshOp::VMeshRemeshOp()
	:
	_remesh_obj(nullptr),
	_org_obj(nullptr)
{}

VMeshRemeshOp::~VMeshRemeshOp()
{
}

bool VMeshRemeshOp::poll()
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

VOperator* VMeshRemeshOp::create()
{
	return new VMeshRemeshOp();
}

void VMeshRemeshOp::execute()
{
	VScene  *scene = VContext::instance()->scene();
	BMesh	*bm = nullptr;
	BMBvh   *bvh = nullptr;
	if (_remesh_obj){
		/*delete decimated object*/
		scene->deleteObject(_remesh_obj);
	}

	if (!_org_obj){
		VObject *actObj = scene->activeObject();
		_org_obj = dynamic_cast<VMeshObject*>(actObj);
	}

	if (_org_obj){
		BMesh *dcbm = new BMesh(*_org_obj->getBmesh());

		BMeshRemeshOp op(dcbm, _org_obj->getBmeshBvh());

		auto it = _params.find("feature_reserve");
		if (it != _params.end())
			op.setFeatureReserve(it->toBool());

		it = _params.find("iteration");
		if (it != _params.end())
			op.setIteration(it->toUInt());

		op.run();

		_remesh_obj = new VMeshObject(scene, dcbm);

		scene->addNode(_remesh_obj);
		scene->markActiveObject(_remesh_obj);

		if (!_org_obj->isRemoved()){
			scene->removeObject(_org_obj);
		}

		VContext::instance()->window()->update();
	}
}

void VMeshRemeshOp::cancel()
{
	VScene  *scene = VContext::instance()->scene();
	if (_remesh_obj && _org_obj){
		scene->deleteObject(_remesh_obj);
		_org_obj->getBmeshBvh()->bvh_full_redraw();
		scene->readdObject(_org_obj);
		_remesh_obj = _org_obj = nullptr;
	}
}
