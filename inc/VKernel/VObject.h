#ifndef VKERNEL_VOBJECT_H
#define VKERNEL_VOBJECT_H

#include "VNode.h"
#include <Eigen/Dense>

VK_BEGIN_NAMESPACE

class VObject : public VNode
{
public:
	typedef enum OB_Flag
	{
		OBJECT_SELECTED = 1 << 0,
		OBJECT_ACTIVE = 1 << 1,
		OBJECT_REMOVED = 1 << 2,
	};

public:
	VObject::VObject()
		:_beingModified(false)
	{
		_name = "object_" + std::to_string(VObject::ObjectInstanceCount);
		VObject::ObjectInstanceCount++;
	}

	~VObject(){};
	
	std::string name() { return _name; }

	virtual bool pick(const Eigen::Vector3f& org, const Eigen::Vector3f& dir, Eigen::Vector3f& hit) = 0;
	virtual	void applyLocalTransform(){};
	virtual Eigen::Vector3f center(){ return Eigen::Vector3f(0.0f, 0.0f, 0.0f); }

	bool isRemoved(){ return (_oflag & OBJECT_REMOVED) != 0; };
	bool isActive() { return (_oflag & OBJECT_ACTIVE) != 0; };


	Eigen::AlignedBox3f gpuChangedBounds(){ return _gpu_changed_bb; }
	void				gpuChangedBoundsReset() { _gpu_changed_bb.setEmpty(); }
	void	beginModify(){ _beingModified = true; }
	void	endModify(){ _beingModified = false; }
	bool	beingModified() { return _beingModified; }
private:
	static int ObjectInstanceCount;
	friend class VScene;
	void markRemoved(){ _oflag |= OBJECT_REMOVED; };
	void unmarkRemoved(){ _oflag &= ~OBJECT_REMOVED; };

	void markActive(){ _oflag |= OBJECT_ACTIVE; };
	void unmarkActive(){ _oflag &= ~OBJECT_ACTIVE; };

	void markSelected(){ _oflag |= OBJECT_SELECTED; };
	void unmarkSelected(){ _oflag &= ~OBJECT_SELECTED; };

	virtual void onSceneAdded(){};
	virtual void onSceneReadded(){};
	virtual void onSceneRemoved(){};
protected:
	std::string			 _name;
	Eigen::AlignedBox3f	 _gpu_changed_bb;
	bool				 _beingModified; /*this object is being modified by an operator*/
};



VK_END_NAMESPACE

#endif