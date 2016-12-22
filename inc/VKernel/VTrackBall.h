#ifndef VKERNEL_VVTrackball_H
#define VKERNEL_VVTrackball_H
#include "VKernelCommon.h"
#include "VNode.h"
#include <GTGraphics.h>
VK_BEGIN_NAMESPACE

class VTrackball
{
public:
	// Construction.  The VTrackball is the largest circle centered in the
	// rectangle of dimensions xSize-by-ySize.  The rectangle is assumed to
	// be defined in right-handed coordinates, so if you use a window client
	// rectangle for the VTrackball and this rectangle is in left-handed
	// coordinates, you must reflect the y-values in SetInitialPoint and
	// SetFinalPoint by (ySize - 1 - y).  A root node is used to represent
	// the VTrackball orientation.  Objects may be attached and detached as
	// desired.
	VTrackball();
	VTrackball(int xSize, int ySize, std::shared_ptr<gte::Camera> const& camera);

	// Member access.  The Set function is for deferred construction after
	// a default construction of a VTrackball.
	void Set(int xSize, int ySize, std::shared_ptr<gte::Camera> const& camera);
	void setNode(std::shared_ptr<VNode> node);
	inline int GetXSize() const;
	inline int GetYSize() const;
	inline std::shared_ptr<gte::Camera> const& GetCamera() const;
	inline gte::EMatrix4x4 const& GetOrientation() const;

	// The root node is the top-level node of a hierarchy whose local
	// transformation is the VTrackball orientation relative to the specified
	// camera.  The camera directions {D,U,R} act as the world coordinate
	// system.
	inline void Update(double applicationTime = 0.0);

	// Set the arc on the sphere along which the VTrackball rotates.  The
	// standard use is to set the initial point via a mouse button click on a
	// window rectangle.  Mark the VTrackball as active and set final points
	// based on the locations of the dragged mouse.  Once the mouse button is
	// released, mark the VTrackball as inactive.
	inline void SetActive(bool active);
	inline bool GetActive() const;

	// Set the initial point of the arc.  The current VTrackball orientation is
	// recorded.  On construction, the initial point is set to the rectangle
	// center.
	void SetInitialPoint(int x, int y);

	// Set the final point of the arc.  The VTrackball orientation is updated
	// by the incremental rotation implied by the arc endpoints.
	void SetFinalPoint(int x, int y);

protected:
	void UpdateOrientation();

	int mXSize, mYSize;
	std::shared_ptr<gte::Camera> mCamera;
	std::shared_ptr<VNode> mRoot; 
	gte::EMatrix4x4 mInitialOrientation;
	float mMultiplier, mX0, mY0, mX1, mY1;
	bool mActive, mValidVTrackball;
};


inline int VTrackball::GetXSize() const
{
	return mXSize;
}

inline int VTrackball::GetYSize() const
{
	return mYSize;
}

inline std::shared_ptr<gte::Camera> const& VTrackball::GetCamera() const
{
	return mCamera;
}

inline gte::EMatrix4x4 const& VTrackball::GetOrientation() const
{
	return mRoot->transform().GetRotation();
}

inline void VTrackball::Update(double applicationTime)
{
	//mRoot->Update(applicationTime);
}

inline void VTrackball::SetActive(bool active)
{
	mActive = active;
}

inline bool VTrackball::GetActive() const
{
	return mActive;
}

inline void VTrackball::setNode(std::shared_ptr<VNode> node)
{
	mRoot = node;
}

VK_END_NAMESPACE
#endif