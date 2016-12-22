#include "VMeshBooleanOp.h"
#include "VKernel/VContext.h"
#include "Boolean/bm/bm_isct_bool_include.h"

VMeshBooleanOp::VMeshBooleanOp()
	:
	_hostObj(nullptr),
	_targetObj(nullptr),
	_resultObj(nullptr),
	_type(VbsDef::BOOLEAN_NONE)
{}

VMeshBooleanOp::~VMeshBooleanOp()
{
}

bool VMeshBooleanOp::poll()
{
	VScene *scene	= VContext::instance()->scene();
	VObject *actObj = scene->activeObject();
	if (actObj){
		VMeshObject *meshobj = dynamic_cast<VMeshObject*>(actObj);
		if (meshobj && meshobj->state() == VMeshObject::STATE_NONE)
			return true;
	}
	return false;
}

VOperator* VMeshBooleanOp::create()
{
	return new VMeshBooleanOp();
}

void VMeshBooleanOp::execute()
{
	VScene  *scene = VContext::instance()->scene();
	
	if (_resultObj){
		/*delete result object from last action*/
		scene->deleteObject(_resultObj);
	}

	/*gather input parameters*/
	if (!_hostObj || !_targetObj){
		QVariant booleanType = param("boolean_type");
		if (!booleanType.isValid())
			return;
		_type = static_cast<VbsDef::BOOLEAN_TYPE>(booleanType.toInt());

		QVariant hostName= param("host");
		if (hostName.isValid() && hostName.type() == QVariant::String){
			VObject * obj = scene->object(hostName.toString().toStdString());
			_hostObj = dynamic_cast<VMeshObject*>(obj);
		}

		QVariant targetName = param("target");
		if (targetName.isValid() && targetName.type() == QVariant::String){
			VObject * obj = scene->object(targetName.toString().toStdString());
			_targetObj = dynamic_cast<VMeshObject*>(obj);
		}
	}

	if (!_hostObj || !_targetObj || _hostObj == _targetObj || _type == VbsDef::BOOLEAN_NONE)
		return;

	BMesh *bool_mesh;
	
	switch (_type)
	{
		case VbsDef::BOOLEAN_UNION:
		{
			bool_mesh = bm_isct::compute_union(_hostObj->getBmesh(), _hostObj->getBmeshBvh(), _targetObj->getBmesh(), _targetObj->getBmeshBvh());
			break;
		}
		case VbsDef::BOOLEAN_DIFFERENCE:
		{
			bool_mesh = bm_isct::compute_difference(_hostObj->getBmesh(), _hostObj->getBmeshBvh(), _targetObj->getBmesh(), _targetObj->getBmeshBvh());
			break;
		}
		case VbsDef::BOOLEAN_INTERSECTION:
		{
			bool_mesh = bm_isct::compute_intersection(_hostObj->getBmesh(), _hostObj->getBmeshBvh(), _targetObj->getBmesh(), _targetObj->getBmeshBvh());
			break;
		}
	}

	if (bool_mesh){
		_resultObj = new VMeshObject(scene, bool_mesh);

		if(!_hostObj->isRemoved())
			scene->removeObject(_hostObj);
		if(!_targetObj->isRemoved())
			scene->removeObject(_targetObj);
		
		scene->addNode(_resultObj);

		scene->markActiveObject(_resultObj);
	}

	VContext::instance()->window()->update();
}

void VMeshBooleanOp::cancel()
{
	VScene  *scene = VContext::instance()->scene();
	if (_hostObj && _targetObj && _resultObj){
		scene->deleteObject(_resultObj);
		scene->readdObject(_hostObj);
		scene->readdObject(_targetObj);
		_resultObj = nullptr;
	}
}
