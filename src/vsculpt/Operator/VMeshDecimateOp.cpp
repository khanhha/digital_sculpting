#include "VMeshDecimateOp.h"
#include "VKernel/VContext.h"
#include "BMesh/Tools/BMeshDecimate.h"

VMeshDecimateOp::VMeshDecimateOp()
	:
	_dcmobj(nullptr),
	_org_obj(nullptr)
{}

VMeshDecimateOp::~VMeshDecimateOp()
{
}

bool VMeshDecimateOp::poll()
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

VOperator* VMeshDecimateOp::create()
{
	return new VMeshDecimateOp();
}

void VMeshDecimateOp::execute()
{
	VScene  *scene = VContext::instance()->scene();
	BMesh	*bm = nullptr;
	BMBvh   *bvh = nullptr;
	if (_dcmobj){
		/*delete decimated object*/
		scene->deleteObject(_dcmobj);
	}
	
	if (!_org_obj){
		VObject *actObj = scene->activeObject();
		_org_obj = dynamic_cast<VMeshObject*>(actObj);
	}

	if (_org_obj){
		BMesh *dcbm = new BMesh(*_org_obj->getBmesh());

		BMeshDecimate op(dcbm);
		auto it = _params.find("ratio");
		if (it != _params.end())
			op.setRatio(it->toFloat());
		op.run();
		
		_dcmobj = new VMeshObject(scene, dcbm);

		scene->addNode(_dcmobj);
		scene->markActiveObject(_dcmobj);

		if (!_org_obj->isRemoved()){
			scene->removeObject(_org_obj);
		}
		
		VContext::instance()->window()->update();
	}
}

void VMeshDecimateOp::cancel()
{
	VScene  *scene = VContext::instance()->scene();
	if (_dcmobj && _org_obj){
		scene->deleteObject(_dcmobj);
		_org_obj->getBmeshBvh()->bvh_full_redraw();
		scene->readdObject(_org_obj);
		_dcmobj = _org_obj = nullptr;
	}
}
