#ifndef VKERNEL_VSCENE_H
#define VKERNEL_VSCENE_H

#include "VNode.h"
#include "VObject.h"
#include "VCamera.h"
#include <GTGraphics.h>
#include "VLighting.h"
#include <Eigen/Dense>
#include "Sculpt/VSculptConfig.h"

VK_BEGIN_NAMESPACE

class VView3DRegion;
class VContext;
class VFloorNode;
class VManipulatorNode;

class VScene : public VNode
{
	Q_OBJECT
private:
	class ChangedBoundsCollect : public VNodeVisitor
	{
	public:
		explicit ChangedBoundsCollect(Eigen::AlignedBox3f &bb_);
		void visit(VNode *node);
	private:
		Eigen::AlignedBox3f &bb;
	};
public:
	VScene();
	~VScene();
	void addNode(VNode *node);
	void detachNode(VNode *node);
	bool nodeExisted(VNode *node);

	virtual void	syncUpdate();
	virtual void	onRender(VView3DRegion *region);
	
	VSculptConfig*  sculptConfig();
	VLighting*		lighting();
	Matrix4x4		worldMatrix() const;
	VFloorNode*			floorNode();
	VManipulatorNode*	manipulatorNode();
	
	Eigen::AlignedBox3f gpuChangedBound();
	void				gpuChangedBoundReset();
	
	Eigen::Vector3f		sceneSpacePointConvert(const Eigen::Vector3f &p);

	Q_INVOKABLE	QStringList objectNames(const QString  &except);
	Q_INVOKABLE QString		activeObjectName();
public:
	bool		objectExist(VObject *obj);
	VObject*	activeObject();

	VObject*	object(const std::string &name);

	void		markSelectedObject(VObject *obj);

	bool		removeObject(VObject* remobj);
	bool		readdObject(VObject* remobj);
	bool		deleteObject(VObject* delobj);
	void		markActiveObject(VObject *obj);
	void		unmarkActiveObject(VObject *obj);
Q_SIGNALS:
	void		activeObjectChanged();
private:
	VLighting		*_lighting;
	VSculptConfig	*_sculptconf;
	
	VManipulatorNode *_manipulator;
	VFloorNode		 *_floor;

	Eigen::AlignedBox3f _gpu_cur_changed_bb, _gpu_last_changed_bb;
};
VK_END_NAMESPACE
#endif