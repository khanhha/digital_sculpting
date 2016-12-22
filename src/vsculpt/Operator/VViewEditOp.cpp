#include <boost/math/constants/constants.hpp>
#include "VViewEditOp.h"
#include "MathUtil.h"
#include "VKernel/VView3DRegion.h"

extern std::vector<std::tuple<Vector3f, Vector3f, Vector3f>> g_debug_segments;

VViewMoveOp::VViewMoveOp()
{}

VViewMoveOp::~VViewMoveOp()
{}

bool VViewMoveOp::poll()
{
	return VContext::instance()->region() != nullptr;
}

VOperator* VViewMoveOp::create()
{
	return new VViewMoveOp();
}

int VViewMoveOp::invoke(QEvent *ev)
{
	_region = VContext::instance()->region();
	QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
	if (mev){
		_lastmouse = mev->pos();
		return VOperator::OP_RET_RUNNING;
	}

	return VOperator::OP_RET_FINISHED;
}

int VViewMoveOp::modal(QEvent *ev)
{
	switch (ev->type())
	{
		case QEvent::MouseMove:
		{
			QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
			viewMoveApply(mev->pos());
			return VOperator::OP_RET_RUNNING;
		}

		case QEvent::MouseButtonRelease:
		{
			return VOperator::OP_RET_FINISHED;
		}
	}
	
	return VOperator::OP_RET_RUNNING;
}

void VViewMoveOp::viewMoveApply(const QPoint &curMouse)
{
 	VScene	*scene = VContext::instance()->scene();
	VCamera	*cam   = _region->camera();
	float xoff =  (curMouse.x() - _lastmouse.x());
	float yoff =  (curMouse.y() - _lastmouse.y());
	if (std::abs(xoff) > 5 || std::abs(yoff) > 5){
		float xWorldOff = VContext::instance()->screenDistanceToModelDistance(_region, std::abs(xoff), cam->viewCenter());
		float yWorldOff = VContext::instance()->screenDistanceToModelDistance(_region, std::abs(yoff), cam->viewCenter());

		xWorldOff = xoff > 0 ? xWorldOff : -xWorldOff;
		yWorldOff = yoff > 0 ? yWorldOff : -yWorldOff;

		Vector3f viewCenterMove = cam->upVector().normalized() * yWorldOff + cam->rightVector().normalized() * xWorldOff;
		cam->translateWorld(viewCenterMove);

		_lastmouse = curMouse;

		//qDebug() << QString::number(xWorldOff) << ", " << QString::number(yWorldOff);

		VContext::instance()->window()->update();

	}
}

VViewZoomOp::VViewZoomOp()
{}

VViewZoomOp::~VViewZoomOp()
{}

bool VViewZoomOp::poll()
{
	return VContext::instance()->region() != nullptr;
}

VOperator* VViewZoomOp::create()
{
	return new VViewZoomOp();
}

int VViewZoomOp::invoke(QEvent *ev) 
{
	_region = VContext::instance()->region();

	if (ev->type() == QEvent::Wheel){
		QWheelEvent *wev = dynamic_cast<QWheelEvent*>(ev);
		float delta = wev->delta();

		VCamera *cam = _region->camera();
		Vector3f camToViewCenter = cam->viewVector().normalized();
		float	 distance = cam->viewVector().norm();
		if (delta < 0.0f){
			distance = 1.2f * distance;
		}
		else{
			distance = 1.1f / 1.2f * distance;
		}
		Vector3f camPos = cam->viewCenter() - distance * camToViewCenter;
		cam->setPosition(camPos);

		VContext::instance()->window()->update();
	}

	return VOperator::OP_RET_FINISHED;
}

VViewRotateOp::VViewRotateOp()
{}

VViewRotateOp::~VViewRotateOp()
{}

bool VViewRotateOp::poll()
{
	return VContext::instance()->region() != nullptr;
}

VOperator* VViewRotateOp::create()
{
	return new VViewRotateOp();
}


int VViewRotateOp::invoke(QEvent *ev)
{
	VContext *context = VContext::instance();
	_region = VContext::instance()->region();

	QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
	if (mev){
		Vector2f curMouse(mev->pos().x(), context->window()->height() - mev->pos().y());
		_lastMouse = curMouse;
		return VOperator::OP_RET_RUNNING;
	}

	return VOperator::OP_RET_FINISHED;
}

int VViewRotateOp::modal(QEvent *ev)
{
	VContext *context = VContext::instance();
	QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
	if (mev){
		Vector2f curMouse(mev->pos().x(), context->window()->height() - mev->pos().y());
		VCamera *camera = _region->camera();

		switch (ev->type())
		{
			case QEvent::MouseMove:
			{
				if ((curMouse - _lastMouse).norm() > 3.0f){

					const float sensitivity = 0.007f;
#if 0
					Eigen::Vector3f globalZ = Vector3f(0.0f, 0.0f, 1.0f);
					Eigen::Vector3f cameraZ = camera->upVector();
					Eigen::Vector3f cameraX = camera->rightVector();

					Eigen::Vector3f xaxis;
					if ((globalZ - cameraZ).squaredNorm() > 0.001f){
						xaxis = globalZ.cross(cameraZ);
						if (xaxis.dot(cameraX) < 0.0f){
							xaxis = -xaxis;
						}
						float fac = MathUtil::angle_normalized_v3v3(globalZ, cameraZ) / boost::math::constants::pi<float>();
						fac = std::abs(fac - 0.5f) * 2.0f;
						fac = fac * fac;
						xaxis = (1.0f - fac) * xaxis + fac * cameraX;
					}
					else{
						xaxis = cameraX;
					}

					xaxis = cameraX.normalized();

					float xangle = sensitivity * -(curMouse.y() - _lastMouse.y());
					Eigen::Quaternion<float> xquad(Eigen::AngleAxisf(xangle, xaxis));
					float reverse = 1.0f;
					if (camera->upVector().z() > 0.0f) reverse *= -1.0f;
					float zangle = sensitivity * reverse * (curMouse.x() - _lastMouse.x());
					Eigen::Quaternion<float> zquad(Eigen::AngleAxisf(zangle, Vector3f(0.0f, 0.0f, 1.0f)));

					Eigen::Quaternionf quad = (zquad /** xquad*/).normalized();
#else
					float xangle = sensitivity * -(curMouse.y() - _lastMouse.y());
					
					float reverse = 1.0f;
					if (camera->upVector().z() > 0.0f) reverse *= -1.0f;
					float zangle = sensitivity * reverse * (curMouse.x() - _lastMouse.x());
					Eigen::Vector3f zGlobal = Vector3(0.0f, 0.0f, 1.0f);

					camera->panAboutViewCenter(zangle, zGlobal);
					camera->tiltAboutViewCenter(xangle);
#endif
					//qDebug() <<xangle << ", " << zangle << " . (" << xaxis[0] << ", " << xaxis[1] <<", "<< xaxis[2] << ")";
					//camera->rotateAboutViewCenter(quad);
					//Vector3f up = camera->upVector().normalized();
					//Vector3f view = camera->viewVector().normalized();
					//float viewDotUp = up.dot(view);
					//float viewlen = camera->viewVector().norm();
					//qDebug() << zangle << ", dot: "<< viewDotUp << ", len: " << viewlen;
					///*qDebug() << "	" << camera->upVector().x() << ", " << camera->upVector().y() << ", " << camera->upVector().z();
					//qDebug() << "	" << camera->viewVector().normalized().x() << ", " << camera->viewVector().normalized().y() << ", " << camera->viewVector().normalized().z();
					//qDebug() << "	" << camera->rightVector().x() << ", " << camera->rightVector().y() << ", " << camera->rightVector().z();*/
					//Eigen::Matrix4f vmat = camera->viewMatrix();
					//qDebug() << "	" << vmat(0, 0) << "	" << vmat(0, 1) << "	" << vmat(0, 2) << "	" << vmat(0, 3);
					//qDebug() << "	" << vmat(1, 0) << "	" << vmat(1, 1) << "	" << vmat(1, 2) << "	" << vmat(1, 3);
					//qDebug() << "	" << vmat(2, 0) << "	" << vmat(2, 1) << "	" << vmat(2, 2) << "	" << vmat(2, 3);
					//qDebug() << "	" << vmat(3, 0) << "	" << vmat(3, 1) << "	" << vmat(3, 2) << "	" << vmat(3, 3);

					_lastMouse = curMouse;
				}

				VContext::instance()->window()->update();

				return VOperator::OP_RET_RUNNING;
			}

			case QEvent::MouseButtonRelease:
			{
				return VOperator::OP_RET_FINISHED;
			}
		}
	}

	return VOperator::OP_RET_RUNNING;
}

