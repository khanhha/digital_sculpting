#include "VScene.h"
#include "vsculpt/QtQuickAppViewer.h"
#include "VView3DRegion.h"
#include "VManipulatorNode.h"
#include "VFloorNode.h"

using namespace gte;

VK_BEGIN_NAMESPACE
VScene::ChangedBoundsCollect::ChangedBoundsCollect(Eigen::AlignedBox3f &bb_)
	: bb(bb_)
{}

void VScene::ChangedBoundsCollect::visit(VNode *node)
{
	VObject *obj = dynamic_cast<VObject*>(node);
	if (obj){
		bb.extend(obj->gpuChangedBounds());
	}
}

VScene::VScene()
{
	_lighting = new VLighting();
	_lighting->setParent(this);

	_sculptconf = new VSculptConfig();

	_floor			= new VFloorNode(this);
	_manipulator	= new VManipulatorNode(this);

	addNode(_floor);
	addNode(_manipulator);
}

VScene::~VScene()
{
	delete _lighting;
	delete _sculptconf;
}

void VScene::addNode(VNode *node)
{
	if (!nodeExisted(node))
		node->setParent(this);
}

void VScene::detachNode(VNode *node)
{
	if (nodeExisted(node)){
		node->setParent(nullptr);
	}
}

bool VScene::nodeExisted(VNode *node)
{
	Q_FOREACH(QObject *cobj, children()){
		VNode *child = dynamic_cast<VNode*>(cobj);
		if (child == node){
			return true;
		}
	}
	return false;
}

Matrix4x4 VScene::worldMatrix() const
{
	return worldTransform().matrix();
}

Eigen::AlignedBox3f VScene::gpuChangedBound()
{
	Eigen::AlignedBox3f bb = _gpu_last_changed_bb;
	bb.extend(_gpu_cur_changed_bb);
	return bb;
}

VFloorNode*	VScene::floorNode()
{
	return _floor;
}

VManipulatorNode* VScene::manipulatorNode()
{
	return _manipulator;
}

void VScene::gpuChangedBoundReset()
{
	_gpu_cur_changed_bb.setEmpty();
	_gpu_last_changed_bb.setEmpty();
}

VSculptConfig*  VScene::sculptConfig()
{
	return _sculptconf;
}

VLighting* VScene::lighting()
{
	return _lighting;
}

void VScene::syncUpdate()
{
	updateWorldTransform();

	Q_FOREACH(QObject *cobj, children()){
		VObject *child = dynamic_cast<VObject*>(cobj);
		if (child){
			child->syncUpdate();
		}
	}

	/*collect GPU changed bounds*/
	/*calculate the changed bounding box from all mesh objects*/
	_gpu_last_changed_bb = _gpu_cur_changed_bb;
	_gpu_cur_changed_bb.setEmpty();
	ChangedBoundsCollect visitor(_gpu_cur_changed_bb);
	accept(&visitor);
}

void VScene::onRender(VView3DRegion *region)
{
	Q_FOREACH(QObject *child, children()){
		VNode *node = dynamic_cast<VNode*>(child);
		if (node){
			node->onRender(region);
		}
	}
}

bool VScene::objectExist(VObject *obj)
{
	Q_FOREACH(QObject *cnode, children()){
		VObject *cobj = dynamic_cast<VObject*>(cnode);
		if (cobj && cobj == obj){
			return true;
		}
	}

	return false;
}

VObject* VScene::object(const std::string &name)
{
	Q_FOREACH(QObject *cnode, children()){
		VObject *cobj = dynamic_cast<VObject*>(cnode);
		if (cobj && cobj->name() == name){
			return cobj;
		}
	}

	return nullptr;
}

VObject* VScene::activeObject()
{
	Q_FOREACH(QObject *qobj, children()){
		VObject *obj = dynamic_cast<VObject*>(qobj);
		if (obj){
			if (obj->isRemoved()) continue;
			if (obj->isActive()) return obj;
		}
	}

	return nullptr;
}

/*un-mark the current active object and set the object as active*/
void VScene::markActiveObject(VObject *obj)
{
	if (obj){
		VObject *curActObj = activeObject();
		if (curActObj){
			curActObj->unmarkActive();
			curActObj->unmarkSelected();
		}
		obj->markActive();
		obj->markSelected();

#ifdef USE_QT_SIGNAL
		emit activeObjectChanged();
#else
		_manipulator->onActiveObjectChanged(obj);
#endif
	}
}

void VScene::unmarkActiveObject(VObject *obj)
{
	if (obj){
		obj->unmarkActive();
		obj->unmarkSelected();
		
#ifdef USE_QT_SIGNAL
		emit activeObjectChanged();
#else
		_manipulator->onActiveObjectChanged(obj);
#endif
	}
}

bool VScene::removeObject(VObject* remobj)
{
	if (remobj && nodeExisted(remobj)){
		if (remobj->isActive()) unmarkActiveObject(remobj);
		remobj->markRemoved();
		remobj->onSceneRemoved();
		return true;
	}

	return false;
}

bool VScene::readdObject(VObject* remobj)
{
	if (remobj && remobj->isRemoved()){
		remobj->unmarkRemoved();
		markActiveObject(remobj);
		remobj->onSceneReadded();
		return true;
	}

	return false;
}

bool VScene::deleteObject(VObject* delobj)
{
	if (delobj){
		if (!delobj->isRemoved()){
			removeObject(delobj);
			detachNode(delobj);
		}
		delete delobj;
		return true;
	}
	else{
		return false;
	}
}

Eigen::Vector3f VScene::sceneSpacePointConvert(const Eigen::Vector3f &p)
{
	Vector4f out = worldTransform().matrix().inverse() * Vector4f(p[0], p[1], p[2], 1.0f);
	return Vector3f(out[0], out[1], out[2]);
}

QStringList VScene::objectNames(const QString &except)
{
	QStringList names;
	Q_FOREACH(QObject *qobj, children()){
		VObject *obj = dynamic_cast<VObject*>(qobj);
		if (obj){
			if (obj->isRemoved()) continue;
			QString name(obj->name().c_str());
			if (name != except){
				names.push_back(name);
			}
		}
	}

	return names;
}

QString VScene::activeObjectName()
{
	VObject *active = activeObject();
	if (active){
		return QString(active->name().c_str());
	}
	return QString();
}

VK_END_NAMESPACE