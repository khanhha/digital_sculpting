#ifndef VBVH_BIN_BUILDER_H
#define VBVH_BIN_BUILDER_H

#include "VBvh/VPrimInfor.h"
#include "VBvh/VPrimRef.h"
#include "VBvh/VBvhDefine.h"
#include "VBvh/VBvhNode.h"
#include "VObjectPartition.h"

VBVH_BEGIN_NAMESPACE



template<class LeafType, size_t ALIGN>
class VBvhBinBuilder
{
private:
	class __aligned(64) VSplitRecord : public VPrimInfo
	{
	public:
		size_t		depth;         //!< depth from the root of the tree
		float		sArea;
		BaseNode*	parent;

		VSplitRecord() {}

		VSplitRecord& operator=(const VSplitRecord &arg) {
			memcpy(this, &arg, sizeof(VSplitRecord));
			return *this;
		}

		__forceinline void init(unsigned int depth)
		{
			this->depth = depth;
			sArea = area(geomBounds);
		}

		__forceinline void init(const VCentGeomBBox3fa& _bounds, const size_t _begin, const size_t _end)
		{
			geomBounds = _bounds.geomBounds;
			centBounds = _bounds.centBounds;
			begin = _begin;
			end = _end;
			sArea = area(geomBounds);
		}

		__forceinline float sceneArea() {
			return sArea;
		}

		__forceinline bool operator<(const VSplitRecord &br) const { return size() < br.size(); }
		__forceinline bool operator>(const VSplitRecord &br) const { return size() > br.size(); }

		struct Greater {
			bool operator()(const VSplitRecord& a, const VSplitRecord& b) {
				return a > b;
			}
		};
	};

public:
	VBvhBinBuilder(VPrimInfo &priminfo, VPrimRef *prims, size_t totprims, size_t minleafsize, const void *user_data)
		:
		_user_data(user_data), _priminfo(priminfo), _prims(prims), _totprims(totprims), 
		_min_leaf_size(minleafsize), _log_block_size(0), _log_SAH_block_size(0)
	{}

	~VBvhBinBuilder(){}
	
	void build()
	{
		VSplitRecord br;
		br.init(_priminfo, 0, _priminfo.size());
		br.depth = 1;
		br.parent = nullptr;
		
		recurse(br);

		_root = br.parent;
	}

	std::vector<LeafType*> leafNodes(){ return _leafs; };
	BaseNode *rootNode(){ return _root; };
private:
	BBox3fa recurse(VSplitRecord& current)
	{
		__aligned(64) VSplitRecord children[4];

		/* create leaf node */
		if (current.depth >= MAX_BUILD_DEPTH || current.size() <= _min_leaf_size) {
			//assert(mode != BUILD_TOP_LEVEL);
			return make_leaf(current);
		}

		/* fill all 4 children by always splitting the one with the largest surface area */
		size_t numChildren = 1;
		children[0] = current;

		do {
			/* find best child with largest bounding box area */
			int bestChild = -1;
			float bestArea = neg_inf;
			for (unsigned int i = 0; i < numChildren; i++){
				/* ignore leaves as they cannot get split */
				if (children[i].size() <= _min_leaf_size)
					continue;

				/* remember child with largest area */
				if (children[i].sceneArea() > bestArea) {
					bestArea = children[i].sceneArea();
					bestChild = i;
				}
			}
			if (bestChild == -1) break;

			/*! split best child into left and right child */
			__aligned(64) VSplitRecord left, right;
			split_sequential(children[bestChild], left, right);

			/* add new children left and right */
			left.init(current.depth + 1);
			right.init(current.depth + 1);
			children[bestChild] = children[numChildren - 1];
			children[numChildren - 1] = left;
			children[numChildren + 0] = right;
			numChildren++;

		} while (numChildren < 4);

		/* create leaf node if no split is possible */
		if (numChildren == 1) {
			return make_leaf(current);
		}

		/* allocate node */
		BaseNode* node = reinterpret_cast<BaseNode*>(scalable_aligned_malloc(sizeof(BaseNode), ALIGN));
		new (node)BaseNode;
		if (current.parent){
			current.parent->addChild(node);
		}
		else{
			current.parent = node;
		}

		/* recurse into each child */
		BBox3fa bounds0(empty);
		for (unsigned int i = 0; i < numChildren; i++)
		{
			children[i].parent = node;
			const BBox3fa bounds = recurse(children[i]);
			bounds0.extend(bounds);
		}
		node->setBB(bounds0);
		return bounds0;
	}

	BBox3fa make_leaf(VSplitRecord& current)
	{
		BLI_assert(current.parent != nullptr);

		/* allocate  leaf node */
		LeafType* node = reinterpret_cast<LeafType*>(scalable_aligned_malloc(sizeof(LeafType), ALIGN));
		new (node)LeafType(current.parent);

		_leafs.push_back(node);
		
		current.parent->addChild(node);

		BBox3fa leafbb = node->build(_prims, current.begin, current.end, _user_data);
		
		return leafbb;
	}

	void split_sequential(VSplitRecord& current, VSplitRecord& left, VSplitRecord& right)
	{
		/* calculate binning function */
		VPrimInfo pinfo(current.size(), current.geomBounds, current.centBounds);
		VBvh::ObjectPartition::Split split = VBvh::ObjectPartition::find(_prims, current.begin, current.end, pinfo, _log_block_size);

		if (UNLIKELY(!split.valid())){
			/* if we cannot find a valid split, enforce an arbitrary split */
			split_fallback(_prims, current, left, right);
		}
		else{
			/* partitioning of items */
			split.partition(_prims, current.begin, current.end, left, right);
		}
	}

	void split_fallback(VPrimRef * __restrict__ const primref, VSplitRecord& current, VSplitRecord& leftChild, VSplitRecord& rightChild)
	{
		const unsigned int center = (current.begin + current.end) / 2;

		VCentGeomBBox3fa left; left.reset();
		for (size_t i = current.begin; i < center; i++)
			left.extend(primref[i].bounds());
		leftChild.init(left, current.begin, center);

		VCentGeomBBox3fa right; right.reset();
		for (size_t i = center; i < current.end; i++)
			right.extend(primref[i].bounds());
		rightChild.init(right, center, current.end);
	}

private:
	VPrimInfo &_priminfo;
	VPrimRef  *_prims;
	const void *_user_data;
	size_t     _totprims;
	size_t	   _min_leaf_size;
	size_t	   _log_block_size;
	size_t	   _log_SAH_block_size;

	/*result*/
	BaseNode			   *_root;
	std::vector<LeafType*>  _leafs;
};
VBVH_END_NAMESPACE

#endif