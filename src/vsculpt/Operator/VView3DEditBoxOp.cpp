#include "VView3DEditBoxOp.h"
#include "VKernel/VContext.h"
#include "VKernel/VView3DRegion.h"
#include "VKernel/VView3DEditBox.h"
#include "MathUtil.h"
#include "QtQuickAppViewer.h"
#include <boost/math/constants/constants.hpp>
#include <QTimer>

VView3DEditBoxOp::VView3DEditBoxOp()
{}

VView3DEditBoxOp::~VView3DEditBoxOp()
{}

bool VView3DEditBoxOp::poll()
{
	VContext *context = VContext::instance();
	VView3DRegion *region  = context->region();
	if (region != nullptr){
		Vector2f mpos = context->currentGLMouse();
		if (region->viewEditBox()->rect().contains(mpos.x(), mpos.y())){
			return true;
		}
		else{
			/*TODO: is it right? operator's poll should not change data*/
			region->viewEditBox()->resetSelect();
		}
	}
	return false;
}

VOperator* VView3DEditBoxOp::create()
{
	return new VView3DEditBoxOp();
}

int  VView3DEditBoxOp::invoke(QEvent *ev)
{
	init();

	if (ev->type() == QEvent::MouseMove || ev->type() == QEvent::MouseButtonPress){
		QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
		QPoint winpos = mev->pos();
		_pos = Eigen::Vector2f(winpos.x(), VContext::instance()->window()->height() - winpos.y());

		if (mev->type() == QEvent::MouseButtonPress && mev->button() == Qt::LeftButton){
			init();
			if (_rotangle > 0.0f){
				QTimer *timer = new QTimer();
				int timerstep = 50;
				_timerid  = VContext::instance()->window()->startTimer(timerstep);
				return OP_RET_RUNNING;
			}
		}
		else if (mev->type() == QEvent::MouseMove){
			_region->viewEditBox()->select(_pos);
		}
	}

	return OP_RET_FINISHED;
}


int VView3DEditBoxOp::modal(QEvent *ev)
{
	if (ev->type() == QEvent::Timer){
		QTimerEvent *tev = dynamic_cast<QTimerEvent*>(ev);
		if (tev->timerId() == _timerid){
			apply();
			if (_rotangle <= 0.0f){
				VContext::instance()->window()->killTimer(_timerid);
				return OP_RET_FINISHED;
			}
		}
	}
	return OP_RET_RUNNING;
}

void VView3DEditBoxOp::init()
{
	_region = VContext::instance()->region();

	if (_region->viewEditBox()->select(_pos)){
		
		Vector3f dir;
		_region->viewEditBox()->viewdir(dir); dir.normalize();
		VCamera *cam = _region->camera();

		Vector3f ncampos = cam->viewCenter() - cam->viewVector().norm() * dir;
		Vector3f nviewdir = (cam->viewCenter() - ncampos).normalized();
		Vector3f oviewdir = cam->viewVector().normalized();
		
		_rotangle = MathUtil::angle_normalized_v3v3(oviewdir, nviewdir);
		_rotanglestep = boost::math::constants::pi<float>() / 18;
		_rotaxis_view = oviewdir.cross(nviewdir).normalized();
	}
}

void VView3DEditBoxOp::apply()
{
	Vector3f dir;
	_region->viewEditBox()->viewdir(dir); dir.normalize();
	VCamera *camera = _region->camera();

	float angle = 0.0f;
	if (_rotangle > _rotanglestep){
		angle = _rotanglestep;
		_rotangle -= _rotanglestep;
	}
	else{
		angle = _rotangle;
		_rotangle = 0.0f;
		_done = false;
	}

	Eigen::AngleAxisf rot(angle, _rotaxis_view);
	Vector3f nviewdir = (rot * camera->viewVector()).normalized();
	Vector3f ncampos = camera->viewCenter() - nviewdir * camera->viewVector().norm();

	Vector3f zGlobal(0.0f, 0.0f, 1.0f);
	Vector3f nx = nviewdir.cross(zGlobal); 
	nx.z() = 0.0f; /*ensure right vector is parallel to OXY plane*/
	if (nx.norm() < FLT_EPSILON){
		nx = camera->rightVector();
	}
	Vector3f nup = (nx.cross(nviewdir)).normalized();
	if (nup.dot(camera->upVector()) < 0.0f)
		nup = -nup;

	camera->setFrame(ncampos, nup, camera->viewCenter());

	VContext::instance()->window()->update();
}
