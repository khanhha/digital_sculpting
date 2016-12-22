#include "VMeshObjectRender.h"
#include "VView3DRegion.h"
#include "VMeshObject.h"
#include "VKernel/VContext.h"
#include <tbb/parallel_for.h>

using namespace gte;

VK_BEGIN_NAMESPACE

////////////////////////////////////////////////////////////////////////////////////////////////////////
VMeshObjectRender::VMeshObjectRender()
	:
    _renderMode(VbsDef::RENDER_WIREFRAME)
{}

VMeshObjectRender::~VMeshObjectRender()
{}

void VMeshObjectRender::setRenderMode(VbsDef::RENDER_MODE mode)
{
	_renderMode = mode;
}
/*this must be called on render thread while GUI thread is locked*/
void VMeshObjectRender::syncUpdate(VContext *context, VNode *node)
{
	std::vector<BMLeafNode*> update_nodes;
	auto BufferUpdater = [&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			updateNodeBuffer(update_nodes[i]);
		}
	};

	VScene *scene = context->scene();
	VMeshObject *mobj = dynamic_cast<VMeshObject*>(node);
	if (mobj){
		/*save world matrix and lighting. This would be combined with VView3DRegion later for uniform buffer update*/
		_worldmat = mobj->worldTransform().matrix();
		_lightpos = scene->lighting()->lightPosition();

		BMBvh *bvh = mobj->getBmeshBvh();
		if (bvh->leaf_node_dirty_draw(update_nodes)){
			
			int max_id = 0;
			for (BMLeafNode *node : update_nodes) max_id = std::max<int>(max_id, node->idx());
			if (max_id >= _vNodeBuffers.size()) _vNodeBuffers.resize(max_id + 1);
			

			tbb::blocked_range<size_t> range(0, update_nodes.size());
			if (range.size() > 8){
				tbb::parallel_for(range, BufferUpdater);
			}
			else{
				BufferUpdater(range);
			}
		}
	}
}


void VMeshObjectRender::onRender(VView3DRegion *region, Eigen::Vector4f(*cullingPlanes)[4])
{
	ensureEffect();

	updateUniformBuffers(region);

	auto Culler = [&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			NodeBufer &nodeGL = _vNodeBuffers[i];
			nodeGL.render = MathGeom::isect_sphere_frustum_v3(nodeGL.sphereCenter, nodeGL.sphereRadius, cullingPlanes) > 0;
		}
	};

	if (cullingPlanes){
		tbb::blocked_range<size_t> range(0, _vNodeBuffers.size());
		if (range.size() > 200){
			tbb::parallel_for(range, Culler);
		}
		else{
			Culler(range);
		}
	}
	else{
		for (auto it = _vNodeBuffers.begin(); it != _vNodeBuffers.end(); ++it){
			NodeBufer &nodeGL = *it;
			nodeGL.render = true;
		}
	}

	if (_renderMode == VbsDef::RENDER_SHADING)
		GL::engine()->Enable(_effect);
	else
		GL::engine()->Enable(_wireEffect);

	for (auto it = _vNodeBuffers.begin(); it != _vNodeBuffers.end(); ++it){
		NodeBufer &nodeGL = *it;
		if (nodeGL.updateGL){
			GL::engine()->Update(nodeGL.vBuffer);
			nodeGL.vBuffer->DestroyStorage();
			nodeGL.updateGL = false;
		}

		if (nodeGL.render)	
			GL::engine()->DrawVertexBuffer(_effect, nodeGL.vBuffer);
	}

	if (_renderMode == VbsDef::RENDER_SHADING)
		GL::engine()->Disable(_effect);
	else
		GL::engine()->Disable(_wireEffect);
}



void VMeshObjectRender::updateUniformBuffers(VView3DRegion *region)
{
	EMatrix4x4 invWorld = _worldmat.inverse();
	EVector3 camWPos = region->cameraPosition();

	EMatrix4x4 pvw = region->projViewMatrix() * _worldmat;

	//_lightGeomery->lightModelPosition = invWorld * EVector4(_lightpos[0], _lightpos[1], _lightpos[2], 1.0f);
	_lightGeomery->lightModelPosition = invWorld * EVector4(camWPos[0], camWPos[1], camWPos[2], 1.0f);
	_lightGeomery->cameraModelPosition = invWorld * EVector4(camWPos[0], camWPos[1], camWPos[2], 1.0f);

	if (_renderMode == VbsDef::RENDER_SHADING){
		_effect->UpdateGeometryConstant();
		_effect->UpdatePVWMatrix(pvw);
	}
	else{
		_wireEffect->setWireframeEdgeColor(EVector4(0.0f, 0.0f, 0.0f, 1.0f));
		_wireEffect->setWireframeWindow(region->width(), region->height());
		_wireEffect->UpdateWireframeParams();

		_wireEffect->UpdatePVWMatrix(pvw);
		_wireEffect->UpdateGeometryConstant();
	}
}

void VMeshObjectRender::updateNodeBuffer(VBvh::BMLeafNode *node)
{
	NodeBufer &nodeGL = _vNodeBuffers[node->idx()];

	nodeGL.nodeIdx = node->idx();
	nodeGL.updateGL = true;
	
	Eigen::AlignedBox3f bb = node->boundsEigen();
	nodeGL.sphereCenter = bb.center();
	nodeGL.sphereRadius = 0.5f * bb.diagonal().norm();

	updateNodeVertexBufferData(nodeGL, node);
}

void VMeshObjectRender::updateNodeVertexBufferData(NodeBufer &nodeGL, VBvh::BMLeafNode *node)
{
	const VBvh::BMFaceVector &faces = node->faces();
	const size_t totfaces = faces.size();
	const size_t totverts = totfaces * 3;

	std::shared_ptr<VertexBuffer> &vbuffer = nodeGL.vBuffer;
	if (!vbuffer){
		VertexFormat vformat;
		vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
		vformat.Bind(VA_NORMAL, DF_R32G32B32_FLOAT, 0);
		vbuffer = std::make_shared<VertexBuffer>(vformat, totverts);
		vbuffer->SetUsage(Resource::DYNAMIC_UPDATE);
	}
	else{
		vbuffer->ResetStorage(totverts);
	}

	const VertexFormat &vformat = vbuffer->GetFormat();
	const size_t vsize = vformat.GetVertexSize();

	char *vdata = vbuffer->GetChannel(VA_POSITION, 0, std::set<DFType>());
	char *ndata = vbuffer->GetChannel(VA_NORMAL, 0, std::set<DFType>());


	EVector3 *co;
	EVector3 *no;
	BMLoop *l_iter, *l_first;
	size_t cnt = 0;

	for (size_t i = 0; i < totfaces; ++i){
		BMFace *f = faces[i];
		if (f->len == 3){
			l_iter = l_first = BM_FACE_FIRST_LOOP(f);
			do {
				co = reinterpret_cast<EVector3*>(vdata + cnt * vsize);
				no = reinterpret_cast<EVector3*>(ndata + cnt * vsize);
				*co = l_iter->v->co;
				*no = l_iter->f->no;
				cnt++;
			} while ((l_iter = l_iter->next) != l_first);
		}
	}
}

void VMeshObjectRender::ensureEffect()
{
	if (_effect) return;

	// Gold color for the spheres.
	EVector4 sphereAmbient(0.24725f, 0.2245f, 0.0645f, 1.0f);
	EVector4 sphereDiffuse(0.34615f, 0.3143f, 0.0903f, 1.0f);
	EVector4 sphereSpecular(0.797357f, 0.723991f, 0.208006f, 83.2f);

	// Various parameters shared by the lighting constants.  The geometric
	// parameters are dynamic, modified by UpdateConstants() whenever the
	// camera or scene moves.  These include camera model position, light
	// model position, light model direction, and model-to-world matrix.
	EVector4 darkGray(0.1f, 0.1f, 0.1f, 1.0f);
	EVector4 lightGray(0.75f, 0.75f, 0.75f, 1.0f);

	_material = std::make_shared<Material>();
	_material->ambient = sphereAmbient;
	_material->diffuse = sphereDiffuse;
	_material->specular = sphereSpecular;

	_lighting = std::make_shared<Lighting>();
	_lighting->ambient = darkGray;

	_lightGeomery = std::make_shared<LightCameraGeometry>();

	GLSLProgramFactory factory;
	/*first question: where Vnode could find program factory and mUpdater for updating buffer ==> effect manager??*/
	_effect = std::make_shared<PointLightEffect>(factory, GL::engine()->GetBufferUpdater(), 0, _material, _lighting, _lightGeomery);

	_wireEffect = std::make_shared<WireframeEffect>(factory, GL::engine()->GetBufferUpdater(), _material, _lighting, _lightGeomery);
}


VK_END_NAMESPACE
