#include "VManipulatorOp.h"
#include "VKernel/VContext.h"
#include "VKernel/VManipulatorNode.h"
#include "VKernel/VView3DRegion.h"

#include <boost/math/constants/constants.hpp>

VManipulatorOp::VManipulatorOp()
{}

VManipulatorOp::~VManipulatorOp()
{}

bool VManipulatorOp::poll()
{
	VContext *context = VContext::instance();
	VView3DRegion *region = context->region();
	VManipulatorNode *manipulator = context->scene()->manipulatorNode();
	VObject *activeObj = context->scene()->activeObject();
	
	if (activeObj && !activeObj->beingModified() && region && manipulator){
		return 	true;
	}

	return false;
}

VOperator* VManipulatorOp::create()
{
	return new VManipulatorOp();
}

int  VManipulatorOp::invoke(QEvent *ev)
{
	VContext *context = VContext::instance();
	_region = context->region();
	_obj = context->scene()->activeObject();
	_manipulator = context->scene()->manipulatorNode();

	_transAxis = VbsDef::MANIPULATOR_AXIS_NONE;
	_transMode = VbsDef::MANIPULATOR_NONE;

	if (_obj && _manipulator){
		if (ev->type() == QEvent::MouseButtonPress){
			QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
			if (mev->button() == Qt::LeftButton){

				if (_manipulator->selectState(_region, context->currentGLMouse())){
					_transAxis = _manipulator->axis();
					_transMode = _manipulator->mode();
					_scaleSensitivity = 0.1f;
				}
			}
		}
		else if (ev->type() == QEvent::KeyPress){
			QKeyEvent *kev = dynamic_cast<QKeyEvent*>(ev);
			if (kev->key() == Qt::Key_G){
				_transMode = VbsDef::MANIPULATOR_TRANSLATE;
				_transAxis = VbsDef::MANIPULATOR_AXIS_FULL;
			}
			else if (kev->key() == Qt::Key_R){
				_transMode = VbsDef::MANIPULATOR_ROTATE;
				_transAxis = VbsDef::MANIPULATOR_AXIS_FULL;
			}
			else if (kev->key() == Qt::Key_S){
				_transMode = VbsDef::MANIPULATOR_SCALE;
				_transAxis = VbsDef::MANIPULATOR_AXIS_FULL;
				_scaleSensitivity = 0.7f;
			}
		}
	}

	if (_transAxis != VbsDef::MANIPULATOR_AXIS_NONE && _transMode != VbsDef::MANIPULATOR_NONE){
		init();
		return OP_RET_RUNNING;
	}
	else{
		return OP_RET_FINISHED;
	}
}

int VManipulatorOp::modal(QEvent *ev)
{
	if (ev->type() == QEvent::MouseMove){
		QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
		_curGLMouse = Vector2f(mev->pos().x(), VContext::instance()->window()->height() - mev->pos().y());
		apply();
		_lastGLMouse = _curGLMouse;
		VContext::instance()->window()->update();
		return OP_RET_RUNNING;
	}
	else if (ev->type() == QEvent::MouseButtonRelease){
		finish();
		VContext::instance()->window()->update();
		return OP_RET_FINISHED;
	}
	else if (ev->type() == QEvent::KeyPress){
		QKeyEvent *kev = dynamic_cast<QKeyEvent*>(ev);
		if (kev->key() == Qt::Key_X || kev->key() == Qt::Key_Y || kev->key() == Qt::Key_Z){
			
			if (kev->key() == Qt::Key_X) _transAxis = VbsDef::MANIPULATOR_AXIS_0;
			if (kev->key() == Qt::Key_Y) _transAxis = VbsDef::MANIPULATOR_AXIS_1;
			if (kev->key() == Qt::Key_Z) _transAxis = VbsDef::MANIPULATOR_AXIS_2;

			_manipulator->setAxis(_transAxis);
			_obj->localTransform() = _initialTransform;
		}
		else if (kev->key() == Qt::Key_Escape){
			cancel();
			VContext::instance()->window()->update();
			return OP_RET_FINISHED;
		}
	}

	return OP_RET_RUNNING;
}

void VManipulatorOp::init()
{
	VContext *context = VContext::instance();

	_initialGLMouse		= context->currentGLMouse();
	_lastGLMouse		= _initialGLMouse;
	_initialTransform	= _obj->localTransform();
	Vector3f pos		= _manipulator->localTransform().translation();
	Vector3f projPoint	= VContext::project(_region, pos);
	_centerGLMouse		= Vector2f(projPoint.x(), projPoint.y());
	_referenceZDepth	= projPoint.z();

	_obj->beginModify();
	_manipulator->setAxis(_transAxis);
	_manipulator->setMode(_transMode);
	_manipulator->beginManipulator();
}

void VManipulatorOp::apply()
{
	if (_transMode == VbsDef::MANIPULATOR_TRANSLATE){
		Vector3f firstwin(_initialGLMouse[0], _initialGLMouse[1], _referenceZDepth);
		Vector3f firstpoint = VContext::instance()->unproject(_region, firstwin);
		Vector3f curwin(_curGLMouse[0], _curGLMouse[1], _referenceZDepth);
		Vector3f curpoint = VContext::instance()->unproject(_region, curwin);
		
		Vector3f move = curpoint - firstpoint;
		Vector3f trans; trans.setZero();
		_transAxis & VbsDef::MANIPULATOR_AXIS_0 ? trans[0] = move[0] : 0.0f;
		_transAxis & VbsDef::MANIPULATOR_AXIS_1 ? trans[1] = move[1] : 0.0f;
		_transAxis & VbsDef::MANIPULATOR_AXIS_2 ? trans[2] = move[2] : 0.0f;

		_obj->localTransform() = _initialTransform;
		_obj->localTransform().translate(trans);
	}
	else if (_transMode == VbsDef::MANIPULATOR_SCALE){
		float firstDst	= (_initialGLMouse	- _centerGLMouse).norm();
		float curDst	= (_curGLMouse		- _centerGLMouse).norm();
		
		if (curDst < FLT_EPSILON && firstDst < FLT_EPSILON) return;

		float val = curDst / firstDst;

		Vector3f scale;
		_transAxis & VbsDef::MANIPULATOR_AXIS_0 ? scale[0] = val : scale[0] = 1.0f;
		_transAxis & VbsDef::MANIPULATOR_AXIS_1 ? scale[1] = val : scale[1] = 1.0f;
		_transAxis & VbsDef::MANIPULATOR_AXIS_2 ? scale[2] = val : scale[2] = 1.0f;

		/*scale too fast in case of full axis, we reduce scale a little bit*/
		if (_transAxis == VbsDef::MANIPULATOR_AXIS_FULL) scale *= _scaleSensitivity;

		vk::Transform trans; trans.setIdentity();
		trans.translate(_manipulator->localTransform().translation());
		trans.scale(scale);
		trans.translate(-_manipulator->localTransform().translation());
		_obj->localTransform() = _initialTransform * trans;
	}
	else if (_transMode == VbsDef::MANIPULATOR_ROTATE){
		Vector2f lastDir	= _lastGLMouse - _centerGLMouse;
		Vector2f curDir		= _curGLMouse - _centerGLMouse;
		if (lastDir.squaredNorm() > 0.1f && curDir.squaredNorm() > 0.1f){
			lastDir.normalize();
			curDir.normalize();
			float angle = MathUtil::angle_normalized_v2v2(lastDir, curDir);
			if (std::abs(angle) > FLT_EPSILON){
				
				float sign = lastDir[0] * curDir[1] - lastDir[1] * curDir[0];
				angle = sign > 0 ? angle : -angle;

				Vector3f viewdir, vieworg;
				VContext::instance()->viewRay(_region, _centerGLMouse, vieworg, viewdir);
				Vector3f axis;
				if (_transAxis == VbsDef::MANIPULATOR_AXIS_FULL){
					axis = -viewdir;
				}
				else{
					if (_transAxis & VbsDef::MANIPULATOR_AXIS_0) axis = Vector3f(1.0f, 0.0f, 0.0f);
					if (_transAxis & VbsDef::MANIPULATOR_AXIS_1) axis = Vector3f(0.0f, 1.0f, 0.0f);
					if (_transAxis & VbsDef::MANIPULATOR_AXIS_2) axis = Vector3f(0.0f, 0.0f, 1.0f);
				}

				if (viewdir.dot(axis) > 0.0f) angle = -angle;
				vk::Transform trans; trans.setIdentity();
				trans.translate( _manipulator->localTransform().translation());
				trans.rotate(Eigen::AngleAxisf(angle, axis));
				trans.translate(-_manipulator->localTransform().translation());
				_obj->localTransform() = _obj->localTransform() * trans;
			}
		}
	}
}

void VManipulatorOp::finish()
{
	/*apply local rotation to object*/
	_obj->applyLocalTransform();
	_obj->endModify();
	_manipulator->endManipulator();
	_manipulator->localTransform().setIdentity();
	_manipulator->localTransform().translate(_obj->center());
}


void VManipulatorOp::cancel()
{
	_obj->localTransform() = _initialTransform;
	_obj->endModify();
	_manipulator->endManipulator();
}

