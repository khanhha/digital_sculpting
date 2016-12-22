#ifndef V_VBVH_H
#define V_VBVH_H
#include "VBvh/VBvhDefine.h"
#include "common/math/bbox.h"
#include "BaseLib/UtilMacro.h"
#include <vector>
#include <assert.h>
#include "tbb/scalable_allocator.h"
#include <Eigen/Dense>
#include <Eigen/Geometry>
using namespace VBvh;

VBVH_BEGIN_NAMESPACE

class BaseNode
{
public:
	enum
	{
		LEAF_NODE = 1 << 0,
		INNER_NODE = 1 << 1
	};
public:
	BaseNode(int ntype = INNER_NODE)
		:
		bb(BBox3fa(empty)), appFlag(0), bvhFlag(ntype)
	{
		clear();
	};

	/*virtual */~BaseNode(){};
	
	VBVH_INLINE bool isLeafNode()
	{
		return bvhFlag & LEAF_NODE;
	}
	VBVH_INLINE bool isInnerNode()
	{
		return bvhFlag & INNER_NODE;
	}

	VBVH_INLINE void setBB(const BBox3fa& bounds)
	{
		bb = bounds;
	}
	VBVH_INLINE void extend(const BBox3fa& other)
	{
		bb.lower = smin(bb.lower, other.lower);
		bb.upper = smax(bb.upper, other.upper);
	}
	VBVH_INLINE void extend(const Vec3fa& otherLower, const Vec3fa& otherUpper)
	{
		bb.lower = smin(bb.lower, otherLower);
		bb.upper = smax(bb.upper, otherUpper);
	}

	VBVH_INLINE Vec3fa lower() const {
		return bb.lower;
	}
	
	VBVH_INLINE Vec3fa upper() const {
		return bb.upper;
	}

	VBVH_INLINE const BBox3fa& bounds() const {
		return bb;
	}

	VBVH_INLINE Eigen::AlignedBox3f boundsEigen() const {
		return
			Eigen::AlignedBox3f(
			Eigen::Vector3f(bb.lower[0], bb.lower[1], bb.lower[2]),
			Eigen::Vector3f(bb.upper[0], bb.upper[1], bb.upper[2]));
	}
	
	VBVH_INLINE void setAppFlagBit(size_t bit)
	{
		BLI_assert(appFlag >= 0);
		appFlag |= bit;
		BLI_assert(appFlag >= 0);
	}
	VBVH_INLINE bool appFlagBit(size_t bit)
	{
		return ((appFlag & bit) != 0);
	}
	VBVH_INLINE void unsetAppFlagBit(size_t bit)
	{
		appFlag &= ~bit;
	}

	VBVH_INLINE void setBvhFlagBit(size_t bit)
	{
		bvhFlag |= bit;
	}
	VBVH_INLINE bool bvhFlagBit(size_t bit)
	{
		return ((bvhFlag & bit) != 0);
	}
	VBVH_INLINE void unsetBvhFlagBit(size_t bit)
	{
		bvhFlag &= ~bit;
	}

	VBVH_INLINE void clear()
	{
		children[0] = children[1] = children[2] = children[3] = nullptr;
	}

	VBVH_INLINE void updateBB()
	{
		for (size_t i = 0; i < 4; ++i){
			if (children[i]){
				extend(children[i]->bounds());
			}
		}
	}

	VBVH_INLINE bool addChild(BaseNode *node)
	{
		bool found = false;
		for (size_t i = 0; i < 4; ++i){
			if (!children[i] && children[i] != node){
				found = true;
				children[i] = node;
				break;
			}
		}
		BLI_assert(found);
		return found;
	}
	
	VBVH_INLINE bool replaceChild(BaseNode *source, BaseNode *dest)
	{
		for (size_t i = 0; i < 4; ++i){
			if (children[i] == source){
				children[i] = dest;
				return true;
			}
		}
		return false;
	}

	VBVH_INLINE bool removeChild(BaseNode *node)
	{
		for (size_t i = 0; i < 4; ++i){
			if (children[i] == node){
				children[i] = nullptr;
				return true;
			}
		}
		return false;
	}

	VBVH_INLINE BaseNode* child(size_t i)
	{
		return children[i];
	}
protected:
	BBox3fa bb;
	BaseNode *children[4];
	unsigned short   bvhFlag;
	unsigned short   appFlag;
};

VBVH_END_NAMESPACE
#endif