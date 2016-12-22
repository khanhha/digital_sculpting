#ifndef VKERNEL_VNODE_H
#define VKERNEL_VNODE_H
#include <QObject>
#include "VKernelCommon.h"
#include <GTGraphics.h>
#include <Eigen/Dense>
VK_BEGIN_NAMESPACE

class VView3DRegion;
class VNodeVisitor
{
public:
	virtual void visit(class VNode *node) = 0;
};

class VNode : public QObject
{
	Q_OBJECT
public:
	VNode(VNode *parent = nullptr);
	virtual ~VNode();
	void accept(VNodeVisitor *visitor);
	virtual void syncUpdate();
	virtual void onRender(VView3DRegion *region);
public:
	virtual void updateWorldTransform();
	Transform& worldTransform();
	const Transform& worldTransform() const;
	Transform& localTransform();
	const Transform& localTransform() const;

protected:
	int	_oflag;
	std::shared_ptr<Transform>		_worldTransform;
	std::shared_ptr<Transform>		_localTransform;
};

VK_END_NAMESPACE

#endif