#ifndef VBVH_BMESH_NODE_H
#define VBVH_BMESH_NODE_H

#include "VBvhNode.h"
#include "VBvhDefine.h"
#include "VPrimRef.h"
#include "VMorton.h"
#include "BMesh/BMesh.h"
#include "tbb/scalable_allocator.h"
#include "tbb/atomic.h"
#define TBB_PREVIEW_MEMORY_POOL 1
#include "tbb/memory_pool.h"
#include <Eigen/Geometry>
using namespace VBvh;
using namespace  VM;
using namespace Eigen;

VBVH_BEGIN_NAMESPACE

//#define BVH_NODE_ELEM_ADD_REMOVE_ASSERT

struct BMeshBvhContext
{
	BMesh *bm;
	int cd_vnode; /*at ((int*)aux_data)[cd_vnode]*/
	int cd_voff;  /*at ((int*)aux_data)[cd_voff]*/
	int cd_fnode; /*as above*/
	int cd_foff;  /*as above*/
};

enum
{
	LEAF_UPDATE_STEP_DRAW_BUFFER = 1 << 0,
	LEAF_UPDATE_STEP_BB = 1 << 1,
	LEAF_ORIGIN_DATA_SAVED = 1 << 2,
	NODE_BARRIER = 1 << 3,
	NODE_TAG = 1 << 4
};


/*only support triangle now*/
struct BMFaceBackup
{
	Vector3f	 vcoods[3];
	BMVert		*verts[3];
	BMFace		*face;
};

struct BMVertBackup
{
	Vector3f co;
	BMVert *v;
};

struct OriginLeafNodeData
{
	Vector3f lower; /*original bounding box. This structure needs to be aligned for correctly functioning*/
	Vector3f upper;

	BMFaceBackup *_faces;
	size_t		  _totfaces;
	BMVertBackup *_verts;
	size_t		  _totverts;
};

class BMLeafNode : public BaseNode
{
public:

public:
	BMLeafNode(BaseNode *parent)
		:
		BaseNode(LEAF_NODE),
		_parent_node(parent),
		_orgData(nullptr),
		_idx(-1)
	{}

	~BMLeafNode(){};

	BBox3fa build(VPrimRef *prims, VMortonID32Bit *morton, size_t total, const void *user_data)
	{
		BBox3fa box(empty);
		_faces.resize(total);
		for (size_t i = 0; i < total; ++i){
			const VPrimRef &ref = prims[morton[i].index];
			box.extend(ref.bounds());
			_faces[i] = reinterpret_cast<BMFace*>(ref.ID());
		}
		setBB(box);
		return box;
	}

	BBox3fa build(VPrimRef *prims, size_t start, size_t end, const void *user_data)
	{
		BLI_assert(start  < end);

		BBox3fa box(empty);
		
		const size_t total = end - start;
		_faces.resize(total);

		for (size_t i = 0; i < total; ++i){
			const VPrimRef &ref = prims[start + i];
			box.extend(ref.bounds());
			_faces[i] = reinterpret_cast<BMFace*>(ref.ID());
		}

		setBB(box);
		return box;
	}


	VBVH_INLINE BaseNode* parent(){ return _parent_node; }
	VBVH_INLINE const BMFaceVector& faces() const { return _faces; }
	VBVH_INLINE const BMVertVector& verts() const { return _verts; }

	VBVH_INLINE OriginLeafNodeData *originData() { return _orgData; };
	VBVH_INLINE int idx()	{ return _idx; };
private:
	friend class BMBvh;
	BaseNode *_parent_node;
	std::vector<BMFace*, tbb::scalable_allocator<BMFace*>> _faces;
	std::vector<BMVert*, tbb::scalable_allocator<BMVert*>> _verts;
	OriginLeafNodeData	*_orgData;
	int					 _idx; /*index to bvh node array*/
};


class BMBvh
{
public:
	BMBvh(BMesh *bm, BaseNode *root, std::vector<BMLeafNode*> &leafs);
	~BMBvh();
	const std::vector<BMLeafNode*>& leafNodes(){ return _leafs; };
	BaseNode *rootNode() const { return _root; } 
	bool bvh_dirty(){ return _dirty; }
	void bvh_set_dirty(bool val){ _dirty = val; };
	void bvh_marked_leaf_nodes_bb_update();
	void bvh_full_refit();
	void bvh_full_update_bb_redraw();
	void bvh_full_redraw();
	void leaf_node_org_data_save(BMLeafNode *node);
	void leaf_node_org_data_drop(BMLeafNode *node);

	void leaf_node_vert_add(BMLeafNode *node, BMVert *v);
	void leaf_node_vert_remove(BMLeafNode *node, BMVert *v, bool reset = false);
	void leaf_node_face_add(BMLeafNode *node, BMFace *f);
	void leaf_node_face_remove(BMLeafNode *node, BMFace *f, bool reset = false);

	void sculpt_stroke_step_update();
	void sculpt_stroke_finish_update();


	BLI_MEMBER_INLINE BMLeafNode* elem_leaf_node_get(BMVert *v)
	{
		int node_pos = BM_elem_aux_data_int_get(v, _bmesh.cd_vnode);
		if (node_pos >= 0 && node_pos < _leafs.size())
			return _leafs[node_pos];
		else
			return nullptr;
	}

	BLI_MEMBER_INLINE BMLeafNode* elem_leaf_node_get(BMFace *f)
	{
		int node_pos = BM_elem_aux_data_int_get(f, _bmesh.cd_fnode);
		if (node_pos >= 0 && node_pos < _leafs.size())
			return _leafs[node_pos];
		else
			return nullptr;
	}

	BLI_MEMBER_INLINE void elem_leaf_node_set(BMVert *v, BMLeafNode *node)
	{
		int node_pos = node ? node->_idx : -1;
#ifdef _DEBUG
		if (node) BLI_assert(node->_idx >= 0 && node->_idx < _leafs.size());
#endif
		BM_elem_aux_data_int_set(v, _bmesh.cd_vnode, node_pos);
	}

	BLI_MEMBER_INLINE void elem_leaf_node_set(BMFace *f, BMLeafNode *node)
	{
		int node_pos = node ? node->_idx : -1;
#ifdef _DEBUG
		if(node) BLI_assert(node->_idx >= 0 && node->_idx < _leafs.size());
#endif
		BM_elem_aux_data_int_set(f, _bmesh.cd_fnode, node_pos);
	}

	BLI_MEMBER_INLINE int	elem_leaf_node_offset_get(BMVert *v)
	{
		return BM_elem_aux_data_int_get(v, _bmesh.cd_voff);
	}

	BLI_MEMBER_INLINE int	elem_leaf_node_offset_get(BMFace *f)
	{
		return BM_elem_aux_data_int_get(f, _bmesh.cd_foff);
	}

	BLI_MEMBER_INLINE void elem_leaf_node_offset_set(BMVert *v, int off)
	{
		BM_elem_aux_data_int_set(v, _bmesh.cd_voff, off);
	}

	BLI_MEMBER_INLINE void elem_leaf_node_offset_set(BMFace *f, int off)
	{
		BM_elem_aux_data_int_set(f, _bmesh.cd_foff, off);
	}

	bool leaf_node_flagged_collect(const int &flag, std::vector<BMLeafNode*> &r_nodes);

	AlignedBox3f bounds();
	void check_valid();
	bool leaf_node_dirty_draw(std::vector<BMLeafNode*> &nodes);
	void leaf_node_dirty_draw_bb(Eigen::AlignedBox3f &bb);
private:
	BBox3fa node_refit_recur(BaseNode *bnode, bool barrier);
	void	node_depth_level_collect(BaseNode *bnode, std::vector<BaseNode*> &nodes, const int barrerDepth, int curDepth);
	void	node_free_recur(BaseNode* node);
	void	leaf_node_free_data(BMLeafNode *lnode);
	void	leaf_node_collect_vert_from_face();
	void	leaf_node_collect_vert_from_face(std::vector<BMLeafNode*> &nodes);
	void	leaf_node_split_big(std::vector<BMLeafNode*> &largenodes);
	void	leaf_node_free(BMLeafNode *lnode);
	void	leaf_node_bound_face_norm_update(BMLeafNode *node);
	void	leaf_node_indexing(const std::vector<BMLeafNode*> &nodes);
	void	leaf_node_faces_add_referece(const std::vector<BMLeafNode*> &nodes);
	void    base_node_free(BaseNode *bnode);
private:
	BMeshBvhContext			_bmesh;
	BaseNode				*_root;
	std::vector<BMLeafNode*> _leafs;
	bool					_dirty;
	tbb::memory_pool<tbb::scalable_allocator<char>> _originDataAllocator;
};


VBVH_END_NAMESPACE
#endif