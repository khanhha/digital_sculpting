#include "VKernel/VManipulatorNode.h"
#include "VKernel/VScene.h"
#include "VKernel/VView3DRegion.h"
#include "VKernel/VContext.h"
#include <qglobal.h>
#include "BaseLib/util.h"
#include "BaseLib/MathUtil.h"
#include "BaseLib/MathGeom.h"

using namespace gte;

VK_BEGIN_NAMESPACE

VManipulatorNode::VManipulatorNode(VScene *scene)
	:
	_pixel_size(80.0f),
	_transMode(VbsDef::MANIPULATOR_NONE)
{
	_isManipulating = false;
	_scene = scene;

	_radius = 1.0f;
	_centerSize = 0.3f;
	_selectSize = 0.1 * _radius;

	_basis[0] = Eigen::Vector3f(1.0f, 0.0f, 0.0f);
	_basis[1] = Eigen::Vector3f(0.0f, 1.0f, 0.0f);
	_basis[2] = Eigen::Vector3f(0.0f, 0.0f, 1.0f);

	createGPUBuffer();

	_disableDepthState = std::make_shared<DepthStencilState>();
	_disableDepthState->depthEnable = false;
}

VManipulatorNode::~VManipulatorNode()
{
}

void VManipulatorNode::onRender(VView3DRegion *region)
{
	ensureEffect();
	EMatrix4x4 pvw = region->projViewMatrix() * worldTransform().matrix();

	float zoom = zoomFactor(region, localTransform().translation());
	vk::Transform trans; trans.setIdentity();
	trans.scale(zoom);
	pvw = pvw * trans.matrix();

	_effect->SetPVWMatrix(pvw);
	GL::engine()->Update(_effect->GetPVWMatrixConstant());

	_vcolorEffect->SetPVWMatrix(pvw);
	GL::engine()->Update(_vcolorEffect->GetPVWMatrixConstant());

	if (_isManipulating){
		if (!(_transAxis & VbsDef::MANIPULATOR_AXIS_0 && _transAxis & VbsDef::MANIPULATOR_AXIS_1 && _transAxis & VbsDef::MANIPULATOR_AXIS_2)){

			size_t axis = (_transAxis & VbsDef::MANIPULATOR_AXIS_0) ? 0 : (_transAxis & VbsDef::MANIPULATOR_AXIS_1) ? 1 : 2;
			_effect->SetColor(axisColor(axis));
			GL::engine()->Update(_effect->GetColorConstant());
			GL::engine()->DrawPrimitive(_axisGPUBuffer[axis].first, _axisGPUBuffer[axis].second, _effect);
		}
	}
	else{
		GL::engine()->SetDepthStencilState(_disableDepthState);


		if (_transMode == VbsDef::MANIPULATOR_TRANSLATE){
			for (size_t d = 0; d < 3; ++d){
				_effect->SetColor(axisColor(d));
				GL::engine()->Update(_effect->GetColorConstant());
				GL::engine()->DrawPrimitive(_transAxisGPUBuffer[d].first, _transAxisGPUBuffer[d].second, _effect);
			}
		}
		else if (_transMode == VbsDef::MANIPULATOR_SCALE){
			for (size_t d = 0; d < 3; ++d){
				_effect->SetColor(axisColor(d));
				GL::engine()->Update(_effect->GetColorConstant());
				GL::engine()->DrawPrimitive(_scaleAxisGPUBuffer[d].first, _scaleAxisGPUBuffer[d].second, _effect);
			}
		
		}
		else if (_transMode == VbsDef::MANIPULATOR_ROTATE){
			updateRotateCircleColor(region->camera()->position());
			for (size_t d = 0; d < 3; ++d){
				GL::engine()->DrawPrimitive(_rotateGPUBuffer[d].first, _rotateGPUBuffer[d].second, _vcolorEffect);
			}
		}

		_effect->SetColor(EVector4(.8f, 0.8f, 0.8f, 0.4f));
		GL::engine()->Update(_effect->GetColorConstant());
		GL::engine()->DrawPrimitive(_transCenterBuffer.first, _transCenterBuffer.second, _effect);

		GL::engine()->SetDefaultDepthStencilState();
	}
}

bool VManipulatorNode::poll(VView3DRegion *region, Vector2f glMouse) const
{
	Vector3f org, dir;
	VContext::instance()->viewRay(region, glMouse, org, dir);
	float zoom = zoomFactor(region, localTransform().translation());
	if (isectManipulatorBB(zoom, org, dir)){
		if (isectTransCenter(zoom, org, dir))
			return true;
		if (_transMode == VbsDef::MANIPULATOR_SCALE)
			return 	isectScaleAxis(zoom, org, dir) >= 0;
		else if (_transMode == VbsDef::MANIPULATOR_TRANSLATE)
			return isectTransAxis(zoom, org, dir) >= 0;
		else if (_transMode == VbsDef::MANIPULATOR_ROTATE)
			return isectRotateAxis(zoom, org, dir) >= 0;
	}
	return false;
}

bool VManipulatorNode::selectState(VView3DRegion *region, Vector2f glMouse)
{
	int axis;
	_transAxis = 0; 
	Vector3f org, dir;
	VContext::instance()->viewRay(region, glMouse, org, dir);
	float zoom = zoomFactor(region, localTransform().translation());
	if (isectManipulatorBB(zoom, org, dir)){
		
		if (isectTransCenter(zoom, org, dir)){
			_transAxis |= VbsDef::MANIPULATOR_AXIS_0;
			_transAxis |= VbsDef::MANIPULATOR_AXIS_1;
			_transAxis |= VbsDef::MANIPULATOR_AXIS_2;
			return true;
		}

		if (_transMode == VbsDef::MANIPULATOR_TRANSLATE){
			axis = isectTransAxis(zoom, org, dir);
			if (axis >= 0){
				_transAxis |= 1 << axis;
				return true;
			}
		}
		else if (_transMode == VbsDef::MANIPULATOR_SCALE){
			axis = isectScaleAxis(zoom, org, dir);
			if (axis >= 0){
				_transAxis |= 1 << axis;
				return true;
			}
		}
		else if (_transMode == VbsDef::MANIPULATOR_ROTATE){
			axis = isectRotateAxis(zoom, org, dir);
			if (axis >= 0){
				_transAxis |= 1 << axis;
				return true;
			}
		}
	}

	return false;
}

char  VManipulatorNode::axis()
{
	return _transAxis;
}


char VManipulatorNode::mode()
{
	return _transMode;
}

void VManipulatorNode::setAxis(char axis)
{
	_transAxis = axis;
}

void VManipulatorNode::setMode(char mode)
{
	_transMode = mode;
}

void VManipulatorNode::ensureEffect()
{
	if (!_effect){
		GLSLProgramFactory  factory;

		EVector4 defaultColor(0.5f, 0.5f, 0.5f, 1.0f);
		_effect = std::make_shared<gte::ConstantColorEffect>(factory, defaultColor);

		_vcolorEffect = std::make_shared<gte::VertexColorEffect>(factory);
	}
}


void VManipulatorNode::createGPUBuffer()
{
	{
		MeshFactory mfac;
		VertexFormat vformat;
		vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
		mfac.SetVertexFormat(vformat);
		float extent = 10000000.0f;

		for (size_t i = 0; i < 3; ++i){
			gte::EVector3 p1, p2; p1.setZero(); p2.setZero();
			p1[i] = -extent;
			p2[i] = +extent;
			auto buffer = mfac.CreateLine(p1, p2);
			_axisGPUBuffer[i].first  = buffer.first;
			_axisGPUBuffer[i].second = buffer.second;
		}

		_transCenterBuffer = mfac.CreateSphere(10, 10, _selectSize);
	}

	createTransGPUBuffer();
	createScaleGPUBuffer();
	createRotateGPUBuffer();
}

void VManipulatorNode::createTransGPUBuffer()
{
	MeshFactory mfac;
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	mfac.SetVertexFormat(vformat);

	for (size_t dim = 0; dim < 3; ++dim){
		_transAxisGPUBuffer[dim] = mfac.CreateBox(_selectSize, _selectSize, _selectSize);
		auto vbuffer = _transAxisGPUBuffer[dim].first;
		EVector3 trans; trans.setZero(); trans[dim] = _radius;
		EVector3 *vpos = reinterpret_cast<EVector3*>(vbuffer->GetData());
		for (size_t idx = 0; idx < vbuffer->GetNumElements(); ++idx){
			vpos[idx] = vpos[idx] + trans;
		}
	}
}

void VManipulatorNode::createScaleGPUBuffer()
{
	MeshFactory mfac;
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT, 0);
	mfac.SetVertexFormat(vformat);

	for (size_t dim = 0; dim < 3; ++dim){
		_scaleAxisGPUBuffer[dim] = mfac.CreateSphere(10, 10, _selectSize);
		auto vbuffer = _scaleAxisGPUBuffer[dim].first;
		EVector3 trans; trans.setZero(); trans[dim] = _radius;
		EVector3 *vpos = reinterpret_cast<EVector3*>(vbuffer->GetData());
		for (size_t idx = 0; idx < vbuffer->GetNumElements(); ++idx){
			vpos[idx] = vpos[idx] + trans;
		}
	}
}

void VManipulatorNode::createRotateGPUBuffer()
{
	MeshFactory mfac;
	VertexFormat vformat;
	vformat.Bind(VA_POSITION, DF_R32G32B32_FLOAT,	0);
	vformat.Bind(VA_COLOR,    DF_R32G32B32A32_FLOAT, 0);

	mfac.SetVertexFormat(vformat);

	for (size_t dim = 0; dim < 3; ++dim){
		Vector3f axis; axis.setZero(); axis[dim] = 1.0f;
		_rotateGPUBuffer[dim] = mfac.CreateCircle(axis, 1.0f, 40);
		_rotateGPUBuffer[dim].first->SetUsage(Resource::DYNAMIC_UPDATE);
	}
}

EVector4 VManipulatorNode::axisColor(int i) const
{
	switch (i)
	{
	case 0:
		return EVector4(1.0f, 0.0f, 0.0f, 1.0f);
	case 1:
		return EVector4(0.0f, 1.0f, 0.0f, 1.0f);
	case 2:
		return EVector4(0.0f, 0.0f, 1.0f, 1.0f);
	default:
		return EVector4(1.0f, 1.0f, 1.0f, 1.0f);
	}
}

bool VManipulatorNode::isectManipulatorBB(float scale, const Eigen::Vector3f &org, const Eigen::Vector3f &dir) const
{
	float rad = scale * (_radius + _selectSize);
	Vector3f bbmin = localTransform().translation() - Vector3f(rad, rad, rad);
	Vector3f bbmax = localTransform().translation() + Vector3f(rad, rad, rad);
	Vector3f isctp;
	return MathGeom::isect_ray_aligned_box_v3(org, dir, bbmin, bbmax, isctp);
}

bool VManipulatorNode::isectTransCenter(float scale, const Vector3f &org, const Vector3f &dir) const
{
	float select_size = scale * _centerSize;
	Vector3f l2 = org + 1000 * dir;
	Vector3f pos = localTransform().translation();
	if (MathGeom::isect_line_sphere_v3(org, l2, pos, select_size, nullptr, nullptr)){
		return true;
	}
	return false;
}

int VManipulatorNode::isectTransAxis(float scale, const Vector3f &org, const Vector3f &dir) const
{
	Vector3f bmin, bmax, p;
	float select_size = scale * _selectSize;
	for (size_t i = 0; i < 3; ++i){
		Vector3f trans; trans.setZero(); trans[i] = scale * 1.0f;
		bmax = localTransform().translation() + (trans + Vector3f(select_size, select_size, select_size));
		bmin = localTransform().translation() + (trans - Vector3f(select_size, select_size, select_size));
		if (MathGeom::isect_ray_aligned_box_v3(org, dir, bmin, bmax, p)){
			return i;
		}
	}
	return -1;
}

int VManipulatorNode::isectScaleAxis(float scale, const Vector3f &org, const Vector3f &dir) const
{
	Vector3f l2 = org + 1000 * dir;
	for (size_t i = 0; i < 3; ++i){
		Vector3f pos; pos.setZero(); pos[i] = scale;
		pos = localTransform().translation() + pos;
		if (MathGeom::isect_line_sphere_v3(org, l2, pos, scale * _selectSize, nullptr, nullptr)){
			return i;
		}
	}
	return -1;
}

int  VManipulatorNode::isectRotateAxis(float scale, const Eigen::Vector3f &org, const Eigen::Vector3f &dir) const
{
	Vector3f l2 = org + 1000 * dir;
	Vector3f pos = localTransform().translation();
	Vector3f isct;
	for (size_t i = 0; i < 3; ++i){
		Vector3f axis; axis.setZero(); axis[i] = 1.0f;
		if (MathGeom::isect_line_plane_v3(isct, org, l2, pos, axis)){
			float isctRadius = (isct - pos).norm();
			if (std::abs(isctRadius - scale) < (_selectSize * scale)){
				return i;
			}
		}
	}

	return -1;
}


void VManipulatorNode::beginManipulator()
{
	_isManipulating = true;
}

void VManipulatorNode::endManipulator()
{
	_isManipulating = false;
}

void VManipulatorNode::onActiveObjectChanged(VObject *obj)
{
	if (obj->isActive()){
		localTransform().setIdentity();
		localTransform().translate(obj->center());
	}
}

float VManipulatorNode::zoomFactor(VView3DRegion *region, Vector3f p) const
{
	EMatrix4x4 pvw = region->projViewMatrix();
	float depth = MathUtil::view3D_depth(pvw, region->viewport(), localTransform().translation());
	float scale = MathUtil::view3D_pixel_size(pvw, region->viewport(), _pixel_size, depth);
	return scale;
}

void VManipulatorNode::updateRotateCircleColor(const Vector3f &cameraPosition)
{
	for (size_t i = 0; i < 3; ++i){
		std::shared_ptr<VertexBuffer> vbuffer = _rotateGPUBuffer[i].first;
		VertexData *vdata = reinterpret_cast<VertexData*>(vbuffer->GetData());
		size_t totalVerts = vbuffer->GetNumElements();
		gte::EVector4 color = axisColor(i);

		Vector3f axis; axis.setZero(); axis[i] = 1.0f;
		Vector3f cameraOnPlane = cameraPosition; cameraOnPlane[i] = 0.0f;
		Vector3f centerToCamera = -cameraOnPlane;

		for (size_t j = 0; j < totalVerts; ++j){
			if ((-vdata[j].co).dot(centerToCamera) > 0.0f){
				vdata[j].color = color;
			}
			else{
				vdata[j].color.setZero();
				vdata[j].color[3] = 0.5f;
			}
		}

		GL::engine()->Update(vbuffer);
	}
}


VK_END_NAMESPACE