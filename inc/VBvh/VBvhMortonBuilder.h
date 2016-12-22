#ifndef VBVH_BVH_MORTON_BUILDER_H
#define VBVH_BVH_MORTON_BUILDER_H

#include "VBvhDefine.h"
#include "VBvhNode.h"
#include "VPrimRef.h"
#include "VPrimInfor.h"
#include "VMorton.h"
#include "tbb/task_scheduler_init.h"
#include "tbb/scalable_allocator.h"
#include "tbb/concurrent_vector.h"
#include "common/default.h"
#include <vector>

using namespace VBvh;
#define USE_ALIGNED_ALLOC 1
VBVH_BEGIN_NAMESPACE

template<class LeafType>
class VBvhMortonBuilder
{
public:
	enum { 
		RECURSE = 1, 
		CREATE_TOP_LEVEL = 2 
	};


	static const size_t MAX_TOP_LEVEL_BINS = 1024;
	static const size_t NUM_TOP_LEVEL_BINS = 1024 + 4 * MAX_BUILD_DEPTH;

	static const size_t RADIX_BITS = 11;
	static const size_t RADIX_BUCKETS = (1 << RADIX_BITS);
	static const size_t RADIX_BUCKETS_MASK = (RADIX_BUCKETS - 1);

public:
	class BuildRecord
	{
	public:
		unsigned int begin;
		unsigned int end;
		unsigned int depth;
		BaseNode*	 parent;

		VBVH_INLINE unsigned int size() const {
			return end - begin;
		}

		VBVH_INLINE void init(const unsigned int _begin, const unsigned int _end)
		{
			begin = _begin;
			end = _end;
			depth = 1;
			parent = NULL;
		}

		struct Greater {
			__forceinline bool operator()(const BuildRecord& a, const BuildRecord& b) {
				return a.size() > b.size();
			}
		};
	};
public:
	struct MortonBuilderState
	{
	public:
		typedef unsigned int ThreadRadixCountTy[RADIX_BUCKETS];

		MortonBuilderState()
		{
			numThreads  = tbb::task_scheduler_init::default_num_threads();
			radixCount = (ThreadRadixCountTy*)scalable_aligned_malloc(numThreads*sizeof(ThreadRadixCountTy), 64);
		}

		~MortonBuilderState()
		{
			scalable_aligned_free(radixCount);
		}

		size_t numThreads;
		tbb::concurrent_vector<LeafType*> leafs;
		ThreadRadixCountTy* radixCount;

		std::vector<BuildRecord> buildRecords;
	};
	
	enum
	{
		NODE_BARRIER = 1 << 0
	};
public:
	VBvhMortonBuilder(VPrimRef *nprims, size_t ntotal, void *nuser_data, size_t nMinLeafSize)
		:
		prims(nprims),
		total(ntotal),
		user_data(nuser_data),
		morton_0(nullptr),
		morton_1(nullptr),
		minLeafSize(nMinLeafSize)
	{
		numberOfThreads = tbb::task_scheduler_init::default_num_threads();
	}

	~VBvhMortonBuilder()
	{}
	
	void	  build();
	void	  computeBounds();
	void	  computeMortonCodes();
	void	  computeMortonCodes(const size_t &startID, const size_t &endID, const size_t &destID, VMortonID32Bit* const dest);
	void	  recreateMortonCodes(BuildRecord& current) const;
	void	  radixSort();
	BaseNode* recurse(BuildRecord& current, const size_t mode);
	void	  recurseSubMortonTrees();
	void	  split(BuildRecord& current, BuildRecord& left, BuildRecord& right) const;
	void	  splitFallback(BuildRecord& current, BuildRecord& leftChild, BuildRecord& rightChild) const;
	BaseNode* createLeaf(BuildRecord& current);
	LeafType* createSmallLeaf(BuildRecord& current);
	BBox3fa   refitTopLevel(BaseNode *node) const;
	BaseNode *rootNode(){ return root; }
	std::vector<LeafType*>& leafNodes(){ return leafs; };
private:
	VPrimRef *prims;
	size_t    total;
	void	 *user_data;
	/*config*/
	size_t topLevelItemThreshold;
	size_t minLeafSize;

	/*real-time data*/
	VMortonID32Bit*  morton_0;
	VMortonID32Bit*  morton_1;
	VCentGeomBBox3fa global_bounds;
	MortonBuilderState *state;
	size_t bytesMorton;
	size_t numberOfThreads;

	/*out*/
	BaseNode *root;
	std::vector<LeafType*> leafs;
};


template<class LeafType>
void VBvhMortonBuilder<LeafType>::build()
{
	if (morton_0) scalable_aligned_free(morton_0);
	if (morton_1) scalable_aligned_free(morton_1);

	bytesMorton = ((total + 7)&(-8)) * sizeof(VMortonID32Bit);
	morton_0 = (VMortonID32Bit*)scalable_aligned_malloc(bytesMorton, 8); memset(morton_0, 0, bytesMorton);
	morton_1 = (VMortonID32Bit*)scalable_aligned_malloc(bytesMorton, 8); memset(morton_0, 0, bytesMorton);

	state = new MortonBuilderState();
	state->leafs.reserve(std::min<size_t>(1, (2 * total / minLeafSize)));

	computeBounds();
	computeMortonCodes();

	/* padding */
	VMortonID32Bit*  const dest = morton_1;
	for (size_t i = total; i < ((total + 7)&(-8)); i++) {
		dest[i].code = 0xffffffff;
		dest[i].index = 0;
	}

	radixSort();

#if defined(DEBUG)
	for (size_t i = 1; i < total; i++)
		assert(morton_0[i - 1].code <= morton_0[i].code);
#endif	    

	/* build and extract top-level tree */
	state->buildRecords.clear();
	topLevelItemThreshold = (total + numberOfThreads - 1) / (2 * numberOfThreads);

	BuildRecord br;
	br.init(0, total);
	br.parent = nullptr;
	br.depth = 1;

	/* perform first splits in single threaded mode */
	recurse(br, CREATE_TOP_LEVEL);

	/* sort all subtasks by size */
	std::sort(state->buildRecords.begin(), state->buildRecords.end(), BuildRecord::Greater());

	/* build sub-trees */
	recurseSubMortonTrees();

	/* refit toplevel part of tree */
	refitTopLevel(br.parent);

	/*gather leaf node*/
	root = br.parent;

	leafs.reserve(state->leafs.size());
	leafs.insert(leafs.end(), state->leafs.begin(), state->leafs.end());

	delete state;

	if (morton_0) scalable_aligned_free(morton_0);
	if (morton_1) scalable_aligned_free(morton_1);
}

template<class LeafType>
void VBvhMortonBuilder<LeafType>::computeBounds()
{
	size_t _Step = total / numberOfThreads;
	size_t _Remain = total % numberOfThreads;

	global_bounds.reset();

	// Count in parallel and separately save their local results without reducing
	tbb::parallel_for(static_cast<size_t>(0), numberOfThreads, [&](size_t _Index){
		size_t _Beg_index, _End_index;
		VCentGeomBBox3fa bounds; bounds.reset();

		// Calculate the segment position
		if (_Index < _Remain){
			_Beg_index = _Index * (_Step + 1);
			_End_index = _Beg_index + (_Step + 1);
		}
		else{
			_Beg_index = _Remain * (_Step + 1) + (_Index - _Remain) * _Step;
			_End_index = _Beg_index + _Step;
		}

		for (size_t i = _Beg_index; i < _End_index; ++i){
			const BBox3fa b(prims[i].bounds());
			if (!inFloatRange(b)) continue;
			bounds.extend(b);
		}

		global_bounds.extend_atomic(bounds);
	});
}

template<class LeafType>
void VBvhMortonBuilder<LeafType>::computeMortonCodes()
{
	size_t _Step = total / numberOfThreads;
	size_t _Remain = total  % numberOfThreads;

	// Count in parallel and separately save their local results without reducing
	tbb::parallel_for(static_cast<size_t>(0), numberOfThreads, [&](size_t _Index){
		size_t _Beg_index, _End_index;

		// Calculate the segment position
		if (_Index < _Remain){
			_Beg_index = _Index * (_Step + 1);
			_End_index = _Beg_index + (_Step + 1);
		}
		else{
			_Beg_index = _Remain * (_Step + 1) + (_Index - _Remain) * _Step;
			_End_index = _Beg_index + _Step;
		}

		computeMortonCodes(_Beg_index, _End_index, _Beg_index, morton_1);
	});
}

template<class LeafType>
void VBvhMortonBuilder<LeafType>::computeMortonCodes(const size_t &startID, const size_t &endID, const size_t &destID, VMortonID32Bit* const dest)
{
	/* compute mapping from world space into 3D grid */
	const ssef base = (ssef)global_bounds.centBounds.lower;
	const ssef diag = (ssef)global_bounds.centBounds.upper - (ssef)global_bounds.centBounds.lower;
	const ssef scale = select(diag > ssef(1E-19f), rcp(diag) * ssef(LATTICE_SIZE_PER_DIM * 0.99f), ssef(0.0f));

	size_t currentID = destID;

	/* use SSE to calculate morton codes */
	size_t slotsIdx = 0;
	ssei ax = 0, ay = 0, az = 0, ai = 0;

	for (size_t i = startID; i < endID; i++)
	{
		const BBox3fa b(prims[i].bounds());
		const ssef lower = (ssef)b.lower;
		const ssef upper = (ssef)b.upper;
		const ssef centroid = lower + upper;
		const ssei binID = ssei((centroid - base)*scale);
		unsigned int index = i;
		ax[slotsIdx] = extract<0>(binID);
		ay[slotsIdx] = extract<1>(binID);
		az[slotsIdx] = extract<2>(binID);
		ai[slotsIdx] = index;
		slotsIdx++;
		currentID++;

		if (slotsIdx == 4)
		{
			const ssei code = bitInterleave(ax, ay, az);
			storeu4i(&dest[currentID - 4], unpacklo(code, ai));
			storeu4i(&dest[currentID - 2], unpackhi(code, ai));
			slotsIdx = 0;
		}
	}

	if (slotsIdx != 0)
	{
		const ssei code = bitInterleave(ax, ay, az);
		for (size_t i = 0; i < slotsIdx; i++)
		{
			dest[currentID - slotsIdx + i].index = ai[i];
			dest[currentID - slotsIdx + i].code = code[i];
		}
	}
}

template<class LeafType>
void VBvhMortonBuilder<LeafType>::radixSort()
{
	size_t _Step = total / numberOfThreads;
	size_t _Remain = total % numberOfThreads;

	VMortonID32Bit* mortonID[2];
	mortonID[0] = (VMortonID32Bit*)morton_0;
	mortonID[1] = (VMortonID32Bit*)morton_1;
	MortonBuilderState::ThreadRadixCountTy* radixCount = state->radixCount;

	/* we need 3 iterations to process all 32 bits */
	for (size_t b = 0; b < 3; b++){
		const VMortonID32Bit* __restrict src = (VMortonID32Bit*)&mortonID[((b + 1) % 2)][0];
		VMortonID32Bit*       __restrict dst = (VMortonID32Bit*)&mortonID[((b + 0) % 2)][0];

		/* shift and mask to extract some number of bits */
		const unsigned int mask = RADIX_BUCKETS_MASK;
		const unsigned int shift = b * RADIX_BITS;

		// Count in parallel and separately save their local results without reducing
		tbb::parallel_for(static_cast<size_t>(0), numberOfThreads, [&](size_t _Index){
			size_t _Beg_index, _End_index;

			// Calculate the segment position
			if (_Index < _Remain){
				_Beg_index = _Index * (_Step + 1);
				_End_index = _Beg_index + (_Step + 1);
			}
			else{
				_Beg_index = _Remain * (_Step + 1) + (_Index - _Remain) * _Step;
				_End_index = _Beg_index + _Step;
			}

			/* count how many items go into the buckets */
			for (size_t i = 0; i < RADIX_BUCKETS; i++)
				radixCount[_Index][i] = 0;

			for (size_t i = _Beg_index; i < _End_index; i++) {
				const size_t digit = src[i].get(shift, mask);
				radixCount[_Index][digit]++;
			}
		});

		// Count in parallel and separately save their local results without reducing
		tbb::parallel_for(static_cast<size_t>(0), numberOfThreads, [&](size_t _Index){
			size_t _Beg_index, _End_index;

			// Calculate the segment position
			if (_Index < _Remain){
				_Beg_index = _Index * (_Step + 1);
				_End_index = _Beg_index + (_Step + 1);
			}
			else{
				_Beg_index = _Remain * (_Step + 1) + (_Index - _Remain) * _Step;
				_End_index = _Beg_index + _Step;
			}

			/* calculate total number of items for each bucket */
			__declspec(align(64)) size_t total[RADIX_BUCKETS];
			for (size_t i = 0; i < RADIX_BUCKETS; i++)
				total[i] = 0;

			for (size_t i = 0; i < numberOfThreads; i++)
				for (size_t j = 0; j < RADIX_BUCKETS; j++)
					total[j] += radixCount[i][j];

			/* calculate start offset of each bucket */
			__declspec(align(64)) size_t offset[RADIX_BUCKETS];
			offset[0] = 0;
			for (size_t i = 1; i < RADIX_BUCKETS; i++)
				offset[i] = offset[i - 1] + total[i - 1];

			/* calculate start offset of each bucket for this thread */
			for (size_t j = 0; j < RADIX_BUCKETS; j++)
				for (size_t i = 0; i < _Index; i++)
					offset[j] += radixCount[i][j];

			/* copy items into their buckets */
			for (size_t i = _Beg_index; i < _End_index; i++) {
				const size_t digit = src[i].get(shift, mask);
				dst[offset[digit]++] = src[i];
			}

		});
	}
}

template<class LeafType>
BaseNode* VBvhMortonBuilder<LeafType>::recurse(BuildRecord& current, const size_t mode)
{
	/* stop top-level recursion at some number of items */
	if (mode == CREATE_TOP_LEVEL && current.size() <= topLevelItemThreshold && current.depth > 1) {
		state->buildRecords.push_back(current);
		return nullptr;
	}

	__declspec(align(64)) BuildRecord children[4];

	/* create leaf node */
	if (UNLIKELY(current.depth >= MAX_BUILD_DEPTH_LEAF || current.size() <= minLeafSize)) {
		return createLeaf(current);
	}

	/* fill all 4 children by always splitting the one with the largest surface area */
	size_t numChildren = 1;
	children[0] = current;

	do {
		/* find best child with largest bounding box area */
		int bestChild = -1;
		unsigned bestItems = 0;
		for (unsigned int i = 0; i < numChildren; i++){
			/* ignore leaves as they cannot get split */
			if (children[i].size() <= minLeafSize)
				continue;

			/* remember child with largest area */
			if (children[i].size() > bestItems) {
				bestItems = children[i].size();
				bestChild = i;
			}
		}

		if (bestChild == -1) break;

		/*! split best child into left and right child */
		_declspec(align(64)) BuildRecord left, right;

		split(children[bestChild], left, right);

		/* add new children left and right */
		left.depth = right.depth = current.depth + 1;
		children[bestChild] = children[numChildren - 1];
		children[numChildren - 1] = left;
		children[numChildren + 0] = right;
		numChildren++;

	} while (numChildren < 4);

	/* create leaf node if no split is possible */
	if (UNLIKELY(numChildren == 1)) {
		return createSmallLeaf(current);
	}

	/* allocate node */
#ifdef USE_ALIGNED_ALLOC
	BaseNode *node = (BaseNode*)scalable_aligned_malloc(sizeof(BaseNode), 16);
	new (node)BaseNode();
#else
	BaseNode *node = (BaseNode*)scalable_malloc(sizeof(BaseNode));
	new (node)BaseNode();
#endif


	/*parent already created, add itself to its parent*/
	if (current.parent){
		current.parent->addChild(node);
	}
	else{
		current.parent = node;
	}

	/* recurse into each child */
	BBox3fa bounds0 = BBox3fa(empty);
	for (size_t i = 0; i < numChildren; i++){
		children[i].parent = node;

		if (children[i].size() <= minLeafSize) {
			BaseNode *cnode = createLeaf(children[i]);
			bounds0.extend(cnode->bounds());
		}
		else {
			BaseNode *cnode = recurse(children[i], mode);
			if (cnode)
				bounds0.extend(cnode->bounds());
		}
	}

	node->setBB(bounds0);
	
	return node;
}

template<class LeafType>
void VBvhMortonBuilder<LeafType>::recurseSubMortonTrees()
{
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, state->buildRecords.size()),
		[&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BaseNode *node = recurse(state->buildRecords[i], RECURSE);
			BLI_assert(node);
			node->setAppFlagBit(NODE_BARRIER);
		}
	});
}

/*! split a build record into two */
template<class LeafType>
void VBvhMortonBuilder<LeafType>::split(BuildRecord& current, BuildRecord& left, BuildRecord& right) const
{
		const unsigned int code_start = morton_0[current.begin].code;
		const unsigned int code_end = morton_0[current.end - 1].code;
		unsigned int bitpos = clz(code_start^code_end);
	
		/* if all items mapped to same morton code, then create new morton codes for the items */
		if (UNLIKELY(bitpos == 32))
		{
			recreateMortonCodes(current);
			const unsigned int code_start = morton_0[current.begin].code;
			const unsigned int code_end = morton_0[current.end - 1].code;
			bitpos = clz(code_start^code_end);
	
			/* if the morton code is still the same, goto fall back split */
			if (unlikely(bitpos == 32))
			{
				size_t center = (current.begin + current.end) / 2;
				left.init(current.begin, center);
				right.init(center, current.end);
				return;
			}
		}
	
		/* split the items at the topmost different morton code bit */
		const unsigned int bitpos_diff = 31 - bitpos;
		const unsigned int bitmask = 1 << bitpos_diff;
	
		/* find location where bit differs using binary search */
		size_t begin = current.begin;
		size_t end = current.end;
		while (begin + 1 != end) {
			const size_t mid = (begin + end) / 2;
			const unsigned bit = morton_0[mid].code & bitmask;
			if (bit == 0) begin = mid; else end = mid;
		}
		size_t center = end;
	
	#if defined(DEBUG)      
		for (unsigned int i = begin; i < center; i++) assert((morton_0[i].code & bitmask) == 0);
		for (unsigned int i = center; i < end; i++) assert((morton_0[i].code & bitmask) == bitmask);
	#endif
	
		left.init(current.begin, center);
		right.init(center, current.end);
}


template<class LeafType>
void VBvhMortonBuilder<LeafType>::splitFallback(BuildRecord& current, BuildRecord& leftChild, BuildRecord& rightChild) const
{
	const unsigned int center = (current.begin + current.end) / 2;
	leftChild.init(current.begin, center);
	rightChild.init(center, current.end);
}

/*! recreates morton codes when reaching a region where all codes are identical */
template<class LeafType>
void VBvhMortonBuilder<LeafType>::recreateMortonCodes(BuildRecord& current) const
{
	assert(current.size() > 4);
	VCentGeomBBox3fa global_bounds;
	global_bounds.reset();
	
	for (size_t i = current.begin; i<current.end; i++)
	{
		const size_t index = morton_0[i].index;
		global_bounds.extend(prims[index].bounds());
	}
	
	/* compute mapping from world space into 3D grid */
	const ssef base = (ssef)global_bounds.centBounds.lower;
	const ssef diag = (ssef)global_bounds.centBounds.upper - (ssef)global_bounds.centBounds.lower;
	const ssef scale = select(diag > ssef(1E-19f), rcp(diag) * ssef(LATTICE_SIZE_PER_DIM * 0.99f), ssef(0.0f));
	
	for (size_t i = current.begin; i < current.end; i++)
	{
		const size_t index = morton_0[i].index;
		const BBox3fa b = prims[index].bounds();
		const ssef lower = (ssef)b.lower;
		const ssef upper = (ssef)b.upper;
		const ssef centroid = lower + upper;
		const ssei binID = ssei((centroid - base)*scale);
		const unsigned int bx = extract<0>(binID);
		const unsigned int by = extract<1>(binID);
		const unsigned int bz = extract<2>(binID);
		const unsigned int code = bitInterleave(bx, by, bz);
		morton_0[i].code = code;
	}
	std::sort(morton_0 + current.begin, morton_0 + current.end);
	
#ifdef _DEBUG
	for (size_t i = current.begin; i < current.end - 1; i++)
		assert(morton_0[i].code <= morton_0[i + 1].code);
#endif	
}

template<class LeafType>
BaseNode* VBvhMortonBuilder<LeafType>::createLeaf(BuildRecord& current)
{
	#ifdef _DEBUG
		if (current.depth > MAX_BUILD_DEPTH_LEAF)
			assert(false);
			//THROW_RUNTIME_ERROR("ERROR: depth limit reached");
	#endif
	
		/* create leaf for few primitives */
		if (current.size() <= minLeafSize) {
			return createSmallLeaf(current);
		}
	
		/* first split level */
		BuildRecord record0, record1;
		splitFallback(current, record0, record1);
	
		/* second split level */
		BuildRecord children[4];
		splitFallback(record0, children[0], children[1]);
		splitFallback(record1, children[2], children[3]);
	
		/* allocate node */
#ifdef USE_ALIGNED_ALLOC
		BaseNode *node = (BaseNode*)scalable_aligned_malloc(sizeof(BaseNode), 16); node->clear();
		new (node)BaseNode();
#else
		BaseNode *node = (BaseNode*)scalable_malloc(sizeof(BaseNode)); node->clear();
		new (node)BaseNode();
#endif


		if (current.parent)
			current.parent->addChild(node);
		else
			current.parent = node;
	
		/* recurse into each child */
		BBox3fa bounds0 = empty;
		for (size_t i = 0; i < 4; i++) {
			children[i].parent = node;
			children[i].depth = current.depth + 1;
			BaseNode *cnode = createLeaf(children[i]);
			bounds0.extend(cnode->bounds());
		}
		
		node->setBB(bounds0);
		return node;
}


template<class LeafType>
LeafType* VBvhMortonBuilder<LeafType>::createSmallLeaf(BuildRecord& current)
{
	size_t totLeafPrims = current.size();
	size_t start = current.begin;
	BLI_assert(current.parent != nullptr);

	/* allocate  leaf node */
#ifdef USE_ALIGNED_ALLOC
	LeafType *lnode = (LeafType*)scalable_aligned_malloc(sizeof(LeafType), 16);
	new (lnode)LeafType(current.parent);
#else
	LeafType *lnode = (LeafType*)scalable_malloc(sizeof(LeafType));
	new (lnode)LeafType();
#endif	
	BBox3fa leafbb = lnode->build(prims, &morton_0[start], totLeafPrims, user_data);
	current.parent->addChild(lnode);

	state->leafs.push_back(lnode);

	lnode->setBB(leafbb);
	
	return lnode;
}

template<class LeafType>
BBox3fa VBvhMortonBuilder<LeafType>::refitTopLevel(BaseNode *node) const
{
	/* return point bound for empty nodes */
	if (UNLIKELY(node == nullptr))
		return BBox3fa(empty);
	
	/* stop here if we encounter a barrier */
	if (UNLIKELY(node->isLeafNode())) {
		return node->bounds();
	}

	if (UNLIKELY(node->appFlagBit(NODE_BARRIER))){
		BLI_assert(node->isInnerNode());
		node->unsetAppFlagBit(NODE_BARRIER);
		//InnerNode *innode = dynamic_cast<InnerNode*>(node);
		//innode->updateBB();
		return node->bounds();
	}

	BaseNode *innode = dynamic_cast<BaseNode*>(node);
	/* recurse if this is an internal node */
	const BBox3fa bounds0 = refitTopLevel(innode->child(0));
	const BBox3fa bounds1 = refitTopLevel(innode->child(1));
	const BBox3fa bounds2 = refitTopLevel(innode->child(2));
	const BBox3fa bounds3 = refitTopLevel(innode->child(3));

	BBox3fa bounds;
	bounds.lower = smin<Vec3fa>(bounds0.lower, bounds1.lower, bounds2.lower, bounds3.lower);
	bounds.upper = smax<Vec3fa>(bounds0.upper, bounds1.upper, bounds2.upper, bounds3.upper);

	innode->extend(bounds);
	return bounds;
}


VBVH_END_NAMESPACE


#endif