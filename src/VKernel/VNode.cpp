#include "VNode.h"
#include "VView3DRegion.h"

using namespace  gte;
VK_BEGIN_NAMESPACE

VNode::VNode(VNode *parent)
	:
	QObject(parent),
	_oflag(0),
	_worldTransform(new Transform()),
	_localTransform(new Transform())
{
	_localTransform->setIdentity();
	_worldTransform->setIdentity();
}

VNode::~VNode()
{}


void VNode::accept(VNodeVisitor *visitor)
{
	if (visitor){
		visitor->visit(this);

		Q_FOREACH(QObject *cobj, children()){
			VNode *cnode = dynamic_cast<VNode*>(cobj);
			if (cnode){
				cnode->accept(visitor);
			}
		}
	}
}

void VNode::updateWorldTransform()
{
	QObject *par = parent();
	VNode *parnode = dynamic_cast<VNode*>(par);
	if (parnode){
		*_worldTransform = parnode->worldTransform() * localTransform();
	}
	else{
		*_worldTransform = *_localTransform;
	}

	Q_FOREACH(QObject *cobj, children()){
		VNode *cnode = dynamic_cast<VNode*>(cobj);
		if (cnode){
			cnode->updateWorldTransform();
		}
	}
}

Transform& VNode::worldTransform()
{
	return *_worldTransform;
}

const Transform& VNode::worldTransform() const
{
	return *_worldTransform;
}

Transform& VNode::localTransform()
{
	return *_localTransform;
}

const Transform& VNode::localTransform() const
{
	return *_localTransform;
}

void VNode::syncUpdate()
{

}

void VNode::onRender(VView3DRegion *region)
{
}




VK_END_NAMESPACE