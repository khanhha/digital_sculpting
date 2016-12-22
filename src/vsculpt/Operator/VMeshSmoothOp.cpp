#include "VMeshSmoothOp.h"
#include "VKernel/VContext.h"
#include "VKernel/VSmoothOp.h"

using namespace vk;

VMeshSmoothOp::VMeshSmoothOp()
	:
	_smoothobj(nullptr)
{}

VMeshSmoothOp::~VMeshSmoothOp()
{}

bool VMeshSmoothOp::poll()
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

VOperator* VMeshSmoothOp::create()
{
	return new VMeshSmoothOp();
}

void VMeshSmoothOp::execute()
{
	VScene  *scene = VContext::instance()->scene();
	BMesh	*bm = nullptr;
	BMBvh   *bvh = nullptr;
	if (_smoothobj && _smoothobj->isActive()){
		bm = _smoothobj->getBmesh();
		bvh = _smoothobj->getBmeshBvh();
	}
	else{
		VObject *actObj = scene->activeObject();
		VMeshObject *meshobj = dynamic_cast<VMeshObject*>(actObj);
		if (meshobj){
			_smoothobj = meshobj;
			bm = _smoothobj->getBmesh();
			bvh = _smoothobj->getBmeshBvh();
			{
				_orgvertco.resize(bm->BM_mesh_verts_total());
				BMVert *v;
				BMIter iter;
				size_t idx = 0;
				BM_ITER_MESH(v, &iter, bm, BM_VERTS_OF_MESH){
					_orgvertco[idx++] = v->co;
				}
			}
		}
	}

	if (bm && bvh){
		VSmoothOp op(bm);

		auto it = _params.find("lambda");
		if (it != _params.end()){
			op.setLambda(it->toFloat());
		}

		it = _params.find("iteration");
		if (it != _params.end()){
			op.setIteration(it->toInt());
		}

		op.run();
		bvh->bvh_full_update_bb_redraw();
		
		/*TODO: why do have to reset changed bounding box here?*/
		_smoothobj->gpuChangedBoundsReset();
		
		VContext::instance()->window()->update();
	}
}

void VMeshSmoothOp::cancel()
{
	if (_smoothobj){
		BMesh *bm = _smoothobj->getBmesh();
		BMBvh *bvh = _smoothobj->getBmeshBvh();
		if (bm->BM_mesh_verts_total() == _orgvertco.size()){
			BMVert *v;
			BMIter iter;
			size_t idx = 0;
			BM_ITER_MESH(v, &iter, bm, BM_VERTS_OF_MESH){
				v->co = _orgvertco[idx++];	
			}

			bvh->bvh_full_update_bb_redraw();
			/*TODO: why do have to reset changed bounding box here?*/
			_smoothobj->gpuChangedBoundsReset();
			VContext::instance()->window()->update();
		}
	}
}
