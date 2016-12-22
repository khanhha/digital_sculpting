#include "VContext.h"
#include "BaseLib/MathUtil.h"
#include "VKernel/VFloorNode.h"
#include "VKernel/VManipulatorNode.h"
#include "VView3DRegion.h"

VK_BEGIN_NAMESPACE

VContext VContext::_context = VContext();

VContext* VContext::instance()
{
	return &_context;
}

VContext::VContext()
	:
	_region(nullptr)
{
	_scene = new VScene();

#if 0
	/*TODO: why it crash here*/
	const QMetaObject *obj = _scene->metaObject();
	QObject::connect(
		_scene, &VScene::activeObjectChanged,
		_scene->manipulatorNode(), &VManipulatorNode::onActiveObjectChanged);
#endif
	//VMeshNode *testnode = new VMeshNode(_scene);
	//testnode->setParent(_scene);
}

VContext::~VContext()
{
}

VScene* VContext::scene()
{
	return _scene;
}

void VContext::viewRay(VView3DRegion *region, Eigen::Vector2f win, Eigen::Vector3f &org, Eigen::Vector3f &dir)
{
	Vector3f l2;
	viewLine(region, win, org, l2);
	dir = (l2 - org).normalized();
}

void VContext::viewLine(VView3DRegion *region, Eigen::Vector2f win, Eigen::Vector3f &l1, Eigen::Vector3f &l2)
{
	VScene *scene = VContext::instance()->scene();
	const Matrix4x4 &proj = region->projMatrix();
	Matrix4x4 modelview = region->viewMatrix() * scene->worldMatrix();
	QRect viewport = region->viewport();
	l1 = scene->sceneSpacePointConvert(region->cameraPosition());
	l2 = MathUtil::unproject(proj, modelview, viewport, Vector3f(win[0], win[1], 0.5f));
}

/*ref_pos: position in scene space*/
float VContext::screenDistanceToModelDistance(VView3DRegion *region, float pixel_dst, Eigen::Vector3f ref_pos)
{
	VScene *scene = VContext::instance()->scene();
	const Matrix4x4 &proj	= region->projMatrix();
	Matrix4x4 modelview		= region->viewMatrix() * scene->worldMatrix();;
	QRect viewport			= region->viewport();

	/*calculate 3d depth from reference position*/
	Eigen::Vector3f proj_pos = MathUtil::project(proj, modelview, viewport, ref_pos);

	/*calculate 3d position at the calculated depth value*/
	Eigen::Vector3f sc_pos0 = MathUtil::unproject(proj, modelview, viewport, Eigen::Vector3f(0.0f, 0.0f, proj_pos[2]));
	Eigen::Vector3f sc_pos1 = MathUtil::unproject(proj, modelview, viewport, Eigen::Vector3f(pixel_dst, 0.0f, proj_pos[2]));
	
	return (sc_pos0 - sc_pos1).norm();
}

QQuickWindow* VContext::window()
{
	return _window;
}

void VContext::setWindow(QQuickWindow *win)
{
	_window = win;
}


VView3DRegion*	VContext::region()
{
	return _region;
}
void VContext::setRegion(VView3DRegion *region)
{
	_region = region;
}

Vector3 VContext::viewModelDirection(VView3DRegion *region, VScene *scene)
{
	Vector3f vdir = region->camera()->viewVector().normalized();
	Vector4f out = scene->worldTransform().matrix().inverse() * Vector4f(vdir[0], vdir[1], vdir[2], 0.0f);
	return Vector3f(out[0], out[1], out[2]);
}

VCommonConfig* VContext::commonConfig()
{
	return &_config;
}

Eigen::Vector2f VContext::currentGLMouse()
{
	QPoint globalPos = window()->cursor().pos();
	QPoint localPos  = window()->mapFromGlobal(globalPos);
	return Vector2f(localPos.x(), window()->height() - localPos.y());
}

Eigen::Vector3f VContext::project(VView3DRegion *region, Eigen::Vector3f &pos)
{
	Matrix4x4 pvw = region->projViewMatrix() * instance()->scene()->worldMatrix();
	return MathUtil::project(pvw, region->viewport(), pos);
}

Eigen::Vector3f  VContext::unproject(VView3DRegion *region, Eigen::Vector3f &win)
{
	Matrix4x4 proj = region->projMatrix();
	Matrix4x4 modelview = region->viewMatrix() * instance()->scene()->worldMatrix();
	return MathUtil::unproject(proj, modelview, region->viewport(), win);
}

VK_END_NAMESPACE
