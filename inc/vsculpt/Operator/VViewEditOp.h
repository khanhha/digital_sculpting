#ifndef VSCULPT_VIEW_EDIT_OP_H
#define VSCULPT_VIEW_EDIT_OP_H
#include "VOperator.h"
#include "VContext.h"
#include "VKernel/VScene.h"
#include "BaseLib/MathGeom.h"
#include <Eigen/Dense>
#include <Eigen/Geometry> 
#include <boost/math/constants/constants.hpp>
#include <QRectF>
#include <tuple>
#ifdef DEBUG_DRAW 
extern std::vector<std::pair<Vector3f, Vector3f>> g_debug_points;
extern std::vector<std::tuple<Vector3f, Vector3f, Vector3f>> g_debug_segments;
#endif
using namespace vk;

class VViewMoveOp: public VOperator
{
public:
	VViewMoveOp();
	virtual ~VViewMoveOp();

	static bool poll();
	static VOperator* create();

	virtual int invoke(QEvent *ev);
	virtual int modal(QEvent *ev);
private:
	void viewMoveApply(const QPoint &curMouse);
	QPoint			_lastmouse;
	VView3DRegion  *_region;
};

class VViewZoomOp : public VOperator
{
public:
	VViewZoomOp();
	virtual ~VViewZoomOp();

	static bool poll();
	static VOperator* create();

	virtual int  invoke(QEvent *ev);
private:
	QPoint			_oldMouse;
	VView3DRegion  *_region;
};

class VViewRotateOp : public VOperator
{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
public:
	VViewRotateOp();
	virtual ~VViewRotateOp();
	static bool poll();
	static VOperator* create();

	virtual int  invoke(QEvent *ev);
	virtual int  modal(QEvent *ev);
private:
	Eigen::Vector2f _lastMouse;
	VView3DRegion  *_region;
};

#endif