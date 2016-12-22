#include <tuple>
#include "VMeshObject.h"
#include "VBvh/BMeshBvhBuilder.h"
#include "VView3DRegion.h"
#include "VContext.h"
#include "VMeshObjectRender.h"
#include "VSceneRenderer.h"
#include <tbb/parallel_for.h>
using namespace  gte;

#if 0
extern std::vector<std::tuple<Vector3f, Vector3f, Vector3f>> g_debug_segments;
#endif

VK_BEGIN_NAMESPACE

////////////////////////////////////////////////////////////////////////////////////////////////////////
VMeshObject::VMeshObject(VScene *scene, VM::BMesh *bm)
	:
	_scene(scene),
	_bmesh(bm),
	_state(STATE_NONE)
{
	_bmesh->BM_mesh_elem_table_ensure(BM_FACE | BM_VERT, true);
	BMeshBvhBuilder builder(_bmesh);
	_bvh = builder.build();

#ifdef _DEBUG
	_bvh->check_valid();
#endif
	gpuChangedBoundsReset();

	subcribeRenderData();
}

VMeshObject::~VMeshObject()
{
	unscribeRenderData();
	delete _bvh;
	delete _bmesh;
}

void VMeshObject::subcribeRenderData()
{
	if (_bvh && _bmesh){
		VSceneRender::instance()->subcribe(this,
			[](){return new VMeshObjectRender(); }, [](VNodeRender* render){ delete render; }
		);
	}
}

void VMeshObject::unscribeRenderData()
{
	VSceneRender::instance()->unsubcribe(this);
}

void VMeshObject::setRenderMode(VbsDef::RENDER_MODE mode)
{
	VNodeRender *renderer = VSceneRender::instance()->renderer(this);
	VMeshObjectRender *myRenderer = dynamic_cast<VMeshObjectRender*>(renderer);
	if (myRenderer){
		myRenderer->setRenderMode(mode);
	}
}

bool VMeshObject::pick(const Eigen::Vector3f& org, const Eigen::Vector3f& dir, Eigen::Vector3f& hit)
{
	if (_bvh){
		return isect_ray_bm_bvh_nearest_hit(_bvh, org, dir, false, hit, 1.0e-6);
	}
	return false;
}

VM::BMesh* VMeshObject::getBmesh()
{
	return _bmesh;
}

VBvh::BMBvh* VMeshObject::getBmeshBvh()
{
	return _bvh;
}

void VMeshObject::syncUpdate()
{
	_gpu_changed_bb.setEmpty();
	if (_state & STATE_SCULPT){
		/*mesh is being sculpted on by GUI thread, we calculate GPU changed bounding box for partial redraw here*/
		_bvh->leaf_node_dirty_draw_bb(_gpu_changed_bb);
	}

}

void VMeshObject::onSceneRemoved()
{
	unscribeRenderData();
}

void VMeshObject::onSceneReadded()
{
	BLI_assert(_bmesh && _bvh);
	_bvh->bvh_full_redraw();
	subcribeRenderData();
}

void VMeshObject::applyLocalTransform()
{
	vk::Transform identity = vk::Transform::Identity();
	if (!localTransform().isApprox(identity) && _bmesh){
		
		_bmesh->BM_mesh_elem_table_ensure(BM_VERT, true);
		auto verts = _bmesh->BM_mesh_vert_table();
		size_t total = verts.size();
		auto &trans = localTransform();

		auto transformRange = [&verts, &trans](const tbb::blocked_range<size_t> & range)
		{
			for (size_t i = range.begin(); i < range.end(); ++i){
				BMVert *v = verts[i];
				v->co = trans * v->co;
			}
		};

		tbb::blocked_range<size_t> range(0, total);
		if (range.size() > 10000)
			tbb::parallel_for(range, transformRange);
		else
			transformRange(range);

		localTransform().setIdentity();

		if (_bvh){
			_bvh->bvh_full_update_bb_redraw();
		}
	}
}
Eigen::Vector3f VMeshObject::center()
{
	return _bvh ? _bvh->bounds().center() : Vector3f(0.0f, 0.0f, 0.0f);
}



VK_END_NAMESPACE
