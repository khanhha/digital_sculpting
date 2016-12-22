#ifndef VSCULPT_VMANIPULATOR_OP_H
#define VSCULPT_VMANIPULATOR_OP_H

#include "VOperator.h"
#include "VKernel/VObject.h"
#include "VbsQt/VbsDef.h"
#include <Eigen/Dense>

class VManipulatorOp : public VOperator
{
public:
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
public:
	VManipulatorOp();
	virtual ~VManipulatorOp();
	static bool poll();
	static VOperator* create();

	virtual int  invoke(QEvent *ev);
	virtual int  modal(QEvent *ev);
private:
	void init();
	void apply();
	void finish();
	void cancel();
private:
	VObject *_obj;
	VManipulatorNode *_manipulator;
	VView3DRegion	 *_region;
	char _transAxis;
	char _transMode;
	float _scaleSensitivity;
	vk::Transform   _initialTransform;
	Eigen::Vector2f	_centerGLMouse;
	Eigen::Vector2f _initialGLMouse;
	Eigen::Vector2f	_curGLMouse;
	Eigen::Vector2f	_lastGLMouse;
	float			_referenceZDepth;

};

#endif