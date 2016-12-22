#include "BMeshBvh.h"
#include "tbb/parallel_for.h"
#include "VBvhUtil.h"
#include "BaseLib/UtilMacro.h"
#include "VBvh/BMeshBvhNodeSplitter.h"
#include <tbb/mutex.h>
//extern std::vector<Point3Dd> g_vscene_testSegments;

VBVH_BEGIN_NAMESPACE

BMBvh::BMBvh(BMesh *bm, BaseNode *root, std::vector<BMLeafNode*> &leafs)
	:
	_root(root),
	_leafs(leafs),
	_dirty(true)
{
	_bmesh.bm = bm;
	_bmesh.cd_fnode = 0; 	
	_bmesh.cd_foff = 1;
	_bmesh.cd_vnode = 0;
	_bmesh.cd_voff = 1;

	leaf_node_indexing(_leafs);
	leaf_node_faces_add_referece(_leafs);
	leaf_node_collect_vert_from_face();
}

BMBvh::~BMBvh()
{
	node_free_recur(_root);
}

void BMBvh::node_free_recur(BaseNode* node)
{
	BaseNode *bnode;
	for (size_t i = 0; i < 4; ++i){
		if (bnode = node->child(i)){
			if (bnode->isInnerNode()){
				node_free_recur(bnode);
			}
			else{
				leaf_node_free(static_cast<BMLeafNode*>(bnode));
			}
		}
	}
	
	base_node_free(node);
}


void BMBvh::leaf_node_collect_vert_from_face()
{
	size_t totvert = _bmesh.bm->BM_mesh_verts_total();
	tbb::atomic<bool> bexample; bexample.store(true);
	std::vector<tbb::atomic<bool>> vfree(totvert, bexample);

	auto range_op = [&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BMLeafNode *lnode = _leafs[i];

			BMLoop *l_iter, *l_first;
			size_t vindex;
			lnode->_verts.reserve(lnode->_faces.size() * 3);
			for (auto it = lnode->_faces.begin(); it != lnode->_faces.end(); ++it){
				BMFace *f = *it;
				l_iter = l_first = BM_FACE_FIRST_LOOP(f);
				do {
					vindex = BM_elem_index_get(l_iter->v);
					if (vfree[vindex].compare_and_swap(false, true)){
						//lnode->_verts.push_back(l_iter->v);
						leaf_node_vert_add(lnode, l_iter->v);
					}
				} while ((l_iter = l_iter->next) != l_first);
			}
		}
	};
	tbb::blocked_range<size_t> total_range(0, _leafs.size());
	
	if (total_range.size() > 200){
		tbb::parallel_for(total_range, range_op);
	}
	else{
		range_op(total_range);
	}
}

void BMBvh::leaf_node_collect_vert_from_face(std::vector<BMLeafNode*> &nodes)
{
	/*disable tag*/
	BMIter iter;
	BMVert *v;
	BM_ITER_MESH(v, &iter, _bmesh.bm, BM_VERTS_OF_MESH){
		BM_elem_flag_disable(v, BM_ELEM_TAG);
	}
	
	/*start collecting vertices*/
	/*mark all vertices on the current leaf nodes (_leafs) to avoid recollect them on new leaf nodes*/
	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, _leafs.size()),
		[&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			/*_leafs could contain null pointer here since we have detach large nodes from it*/
			if (_leafs[i] != nullptr){
				const BMVertVector &verts = _leafs[i]->verts();
				const size_t totverts = verts.size();
				for (size_t iv = 0; iv < totverts; ++iv){
					BLI_assert(!BM_elem_flag_test_bool(verts[iv], BM_ELEM_TAG));
					BM_elem_flag_enable(verts[iv], BM_ELEM_TAG);
				}
			}
		}
	});

	for (auto it = nodes.begin(); it != nodes.end(); ++it){
		BMLeafNode *node = *it;
		
		const BMFaceVector &faces = node->_faces;
		const size_t totface = faces.size();
		node->_verts.reserve(totface * 2);
		for (size_t i = 0; i < totface; ++i){
			BMFace *f = faces[i];
			BMLoop *l_first, *l_iter;
			l_iter = l_first = BM_FACE_FIRST_LOOP(f);
			do{
				if (!BM_elem_flag_test_bool(l_iter->v, BM_ELEM_TAG)){
					BM_elem_flag_enable(l_iter->v, BM_ELEM_TAG);
					leaf_node_vert_add(node, l_iter->v);
				}
			} while ((l_iter = l_iter->next) != l_first);
		}
	}
}

void BMBvh::bvh_marked_leaf_nodes_bb_update()
{
	//update bounding box for leaf node
	std::vector<BMLeafNode*> nodes;
	for (auto it = _leafs.begin(); it != _leafs.end(); ++it){
		BMLeafNode* node = (*it);
		if (node->appFlagBit(LEAF_UPDATE_STEP_BB)){
			node->unsetAppFlagBit(LEAF_UPDATE_STEP_BB);
			nodes.push_back(node);
		}
	}

	if (!nodes.empty()){

		bvh_set_dirty(true);
		
		tbb::parallel_for(static_cast<size_t>(0), nodes.size(), [&](size_t index){
			BMLeafNode* node = nodes[index];
#if 0
			Vector3f nlower, nupper;
			facesBoundsNormalUpdate(node->faces().data(), node->faces().size(), nlower, nupper);
			node->setBB(BBox3fa(Vec3fa(nlower(0), nlower(1), nlower(2)), Vec3fa(nupper(0), nupper(1), nupper(2))));
#else
			leaf_node_bound_face_norm_update(node);

#endif
		});

	}
}

void BMBvh::bvh_full_refit()
{
	if (_leafs.size() > 200){
		const int depthBarrier = 4;
		
		std::vector<BaseNode*> barrierNodes;
		node_depth_level_collect(_root, barrierNodes, depthBarrier, 0);
		for (auto it = barrierNodes.begin(); it != barrierNodes.end(); ++it){
			(*it)->setAppFlagBit(NODE_BARRIER);
		}

		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, barrierNodes.size()),
			[&](const tbb::blocked_range<size_t> &range)
		{
			for (size_t i = range.begin(); i < range.end(); ++i){
				node_refit_recur(barrierNodes[i], false);
			}
		});

		node_refit_recur(_root, true);
	}
	else{
		node_refit_recur(_root, false);
	}
}

BBox3fa BMBvh::node_refit_recur(BaseNode *bnode, bool barrier)
{
	if (UNLIKELY(bnode == nullptr))
		return BBox3fa(empty);

	if (UNLIKELY(barrier && bnode->appFlagBit(NODE_BARRIER))){
		bnode->unsetAppFlagBit(NODE_BARRIER);
		return bnode->bounds();
	}
	else if (UNLIKELY(bnode->isLeafNode())){
		return bnode->bounds();
	}
	else{
		BBox3fa bounds[4] = {empty, empty, empty, empty};
		for (size_t i = 0; i < 4; ++i){
			if (bnode->child(i)){
				bounds[i] = node_refit_recur(bnode->child(i), barrier);
			}
		}

		bnode->setBB(BBox3fa(empty));
		for (size_t i = 0; i < 4; ++i){
			if (bnode->child(i)){
				bnode->extend(bounds[i]);
			}
		}

		return bnode->bounds();
	}
}

void BMBvh::bvh_full_update_bb_redraw()
{
	bvh_set_dirty(true);

	for (auto it = _leafs.begin(); it != _leafs.end(); ++it){
		BMLeafNode *node = *it;
		node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);
	}

	bvh_marked_leaf_nodes_bb_update();
	
	bvh_full_refit();
}

void BMBvh::bvh_full_redraw()
{
	bvh_set_dirty(true);
	for (auto it = _leafs.begin(); it != _leafs.end(); ++it){
		BMLeafNode *node = *it;
		node->setAppFlagBit(LEAF_UPDATE_STEP_DRAW_BUFFER);
	}
}

void BMBvh::node_depth_level_collect(BaseNode *bnode, std::vector<BaseNode*> &nodes, const int barrerDepth, int curDepth)
{
	if (curDepth == barrerDepth){
		nodes.push_back(bnode);
	}

	if (curDepth < barrerDepth){

		if (bnode->isInnerNode()){
			BaseNode *innode = bnode;
			for (size_t i = 0; i < 4; ++i){
				if (innode->child(i)){
					node_depth_level_collect(innode->child(i), nodes, barrerDepth, curDepth + 1);
				}
			}
		}
	}
}

void BMBvh::leaf_node_org_data_save(BMLeafNode *node)
{
	//this node has been saved
	if (node->_orgData)
		return;

	node->setAppFlagBit(LEAF_ORIGIN_DATA_SAVED);

	OriginLeafNodeData *data = new OriginLeafNodeData();

	const auto &bb = node->bounds();
	data->lower = Vector3f(bb.lower.x, bb.lower.y, bb.lower.z);
	data->upper = Vector3f(bb.upper.x, bb.upper.y, bb.upper.z);

	const BMFaceVector &faces = node->faces();
	const BMVertVector &verts = node->verts();
	const size_t totvert = verts.size();
	const size_t totface = faces.size();

	data->_totverts = totvert;
	data->_totfaces = totface;

	size_t fsize = sizeof(BMFaceBackup) * totface;
	size_t vsize = sizeof(BMVertBackup) * totvert;
	data->_faces = reinterpret_cast<BMFaceBackup*>(_originDataAllocator.malloc(fsize));
	data->_verts = reinterpret_cast<BMVertBackup*>(_originDataAllocator.malloc(vsize));

	BMFace *f; BMLoop *l_iter, *l_first;
	for (size_t i = 0; i < totface; ++i){
		f = faces[i];
		BLI_assert(f->len == 3);

		BMFaceBackup &backup = data->_faces[i];
		backup.face = f;
		l_first = l_iter = BM_FACE_FIRST_LOOP(f);
		size_t cnt = 0;
		do {
			backup.verts[cnt]	= l_iter->v;
			backup.vcoods[cnt]  = l_iter->v->co;
			cnt++;
		} while ((l_iter = l_iter->next) != l_first);
	}

	for (size_t i = 0; i < totvert; ++i){
		BMVertBackup &backup = data->_verts[i];
		backup.v	= verts[i];
		backup.co	= verts[i]->co;
	}

	node->_orgData = data;
}

void BMBvh::leaf_node_org_data_drop(BMLeafNode *node)
{
	if (node->originData()){
		BLI_assert(node->appFlagBit(LEAF_ORIGIN_DATA_SAVED));
		node->unsetAppFlagBit(LEAF_ORIGIN_DATA_SAVED);
		delete node->_orgData; /*vertex coord array and face coord array attribute will be recycled by memory pool*/
		node->_orgData = nullptr;
	}
}

void BMBvh::sculpt_stroke_step_update()
{
	bvh_marked_leaf_nodes_bb_update();
	bvh_full_refit();
	bvh_set_dirty(true);

#ifdef _DEBUG
	check_valid();
#endif
}

void BMBvh::sculpt_stroke_finish_update()
{
	_originDataAllocator.recycle();

	std::vector<BMLeafNode*> bignodes; bignodes.reserve(20);
	for (int i = 0; i < _leafs.size(); i++){
		BMLeafNode* node = _leafs[i];
		leaf_node_org_data_drop(node);
		if (node->faces().size() > 900){
			bignodes.push_back(node);
		}
	}

	bvh_marked_leaf_nodes_bb_update();

	bvh_full_refit();

	leaf_node_split_big(bignodes);

	bvh_set_dirty(true);

#ifdef _DEBUG
	check_valid();
#endif
}

void BMBvh::leaf_node_split_big(std::vector<BMLeafNode*> &largenodes)
{
	if (largenodes.size() > 0){
		
		tbb::mutex mutex;

		/*detach large node from our leaf node array*/
		for (auto it = largenodes.begin(); it != largenodes.end(); ++it){
			BMLeafNode *node = *it;
			BLI_assert(_leafs[node->_idx] == node);
			_leafs[node->_idx] = nullptr;
		}

		std::vector<BMLeafNode*> all_new_leafs;
		for (auto it = largenodes.begin(); it != largenodes.end(); ++it){
			BMLeafNode *node = *(it);
			BMeshBvhNodeSplitter splitter(node, _bmesh);
			splitter.run();

			bool insertdone = node->parent()->replaceChild(node, splitter.root());
			BLI_assert(insertdone);

			std::vector<BMLeafNode*> new_leafs = std::move(splitter.leafs());
			for (auto it = new_leafs.begin(); it != new_leafs.end(); ++it){
				(*it)->setAppFlagBit(LEAF_UPDATE_STEP_DRAW_BUFFER);
			}

		{
			tbb::mutex::scoped_lock lock(mutex);
			all_new_leafs.insert(all_new_leafs.end(), new_leafs.begin(), new_leafs.end());
		}
		}

		for (auto it = largenodes.begin(); it != largenodes.end(); ++it){
			leaf_node_free(*it);
		}

		/*fill new leaf nodes into our leaf node array, and indexing them*/
		size_t tot_old_nodes = _leafs.size();
		size_t tot_new_nodes = all_new_leafs.size();
		size_t new_node_cnt = 0;
		for (size_t i = 0; i < tot_old_nodes; ++i){
			if (_leafs[i] == nullptr){
				_leafs[i] = all_new_leafs[new_node_cnt++];
				_leafs[i]->_idx = i; /*indexing new leaf node*/
				if (new_node_cnt >= tot_new_nodes){
					break;
				}
			}
		}

		/*if we have fill all new leaf nodes, new_node_cnt must equal or greater than tot_new_nodes*/
		if (new_node_cnt < tot_new_nodes){
			for (size_t i = new_node_cnt; i < tot_new_nodes; ++i){
				_leafs.push_back(all_new_leafs[i]);
				_leafs.back()->_idx = _leafs.size() - 1;
			}
		}

#if _DEBUG
		for (size_t i = 0; i < _leafs.size(); ++i){
			BLI_assert(_leafs[i] != nullptr);
		}
#endif

		leaf_node_faces_add_referece(all_new_leafs);
		leaf_node_collect_vert_from_face(all_new_leafs);
	}
}



void BMBvh::leaf_node_vert_add(BMLeafNode *node, BMVert *v)
{
#ifdef BVH_NODE_ELEM_ADD_REMOVE_ASSERT
	bool found = false;
	for (auto it = node->_verts.begin(); it != node->_verts.end(); ++it){
		if (*it == v){
			found = true;
		}
	}
	BLI_assert(!found);
#endif

	node->_verts.push_back(v);
	node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);
	elem_leaf_node_set(v, node);
	elem_leaf_node_offset_set(v, node->_verts.size() -1);

#ifdef _DEBUG
	BLI_assert(this->elem_leaf_node_get(v) == node);
	BLI_assert(this->elem_leaf_node_offset_get(v) == (node->verts().size() - 1));
#endif
}

void BMBvh::leaf_node_vert_remove(BMLeafNode *node, BMVert *v, bool reset)
{
#ifdef BVH_NODE_ELEM_ADD_REMOVE_ASSERT
	bool found = false;
	for (auto it = node->_verts.begin(); it != node->_verts.end(); ++it){
		if (*it == v){
			found = true;
		}
	}
	BLI_assert(found);
#endif

	int off = elem_leaf_node_offset_get(v);
	node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);

#ifdef _DEBUG
	BMLeafNode *vnode = elem_leaf_node_get(v);
	BLI_assert(vnode == node && node->_verts[off] == v);
#endif

	elem_leaf_node_offset_set(node->_verts.back(), off);
	elem_leaf_node_offset_set(v, -1);
	std::swap(node->_verts[off], node->_verts.back());
	node->_verts.pop_back();

	/*don't reset for re-adding this vertex to its node later*/
	if (reset){
		elem_leaf_node_set(v, nullptr);
	}
}

void BMBvh::leaf_node_face_add(BMLeafNode *node, BMFace *f)
{
#ifdef BVH_NODE_ELEM_ADD_REMOVE_ASSERT
	bool found = false;
	for (auto it = node->_faces.begin(); it != node->_faces.end(); ++it){
		if (*it == f){
			found = true;
		}
	}
	BLI_assert(!found);
#endif

	node->_faces.push_back(f);
	node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);

	elem_leaf_node_offset_set(f, node->_faces.size() -1);
	elem_leaf_node_set(f, node);
}

void BMBvh::leaf_node_face_remove(BMLeafNode *node, BMFace *f, bool reset)
{
#ifdef BVH_NODE_ELEM_ADD_REMOVE_ASSERT
	bool found = false;
	for (auto it = node->_faces.begin(); it != node->_faces.end(); ++it){
		if (*it == f){
			found = true;
		}
	}
	BLI_assert(found);
#endif

	int off = elem_leaf_node_offset_get(f);//BM_ELEM_CD_GET_INT(f, _bmesh.cd_foff);

#ifdef _DEBUG
	BMLeafNode *fnode = elem_leaf_node_get(f); 
	BLI_assert(fnode == node && node->_faces[off] == f);
#endif

	elem_leaf_node_offset_set(node->_faces.back(), off);
	elem_leaf_node_offset_set(f, -1);
	std::swap(node->_faces[off], node->_faces.back());
	node->_faces.pop_back();
	node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);

	/*don't reset for re-adding this face to its node later*/
	if (reset){
		elem_leaf_node_set(f, nullptr);
	}
}

bool BMBvh::leaf_node_flagged_collect(const int &flag, std::vector<BMLeafNode*> &r_nodes)
{
	for (auto it = _leafs.begin(); it != _leafs.end(); ++it){
		BMLeafNode *node = *it;
		if (node->appFlagBit(flag)){
			r_nodes.push_back(node);
		}
	}

	return !r_nodes.empty();
}

void BMBvh::leaf_node_free_data(BMLeafNode *lnode)
{
	/*original data should not exist here. it only being used during a stroke*/
	BLI_assert(lnode->_orgData == nullptr);


	lnode->_faces.clear();
	lnode->_verts.clear();
}

void BMBvh::leaf_node_free(BMLeafNode *lnode)
{
	leaf_node_free_data(lnode);
	scalable_aligned_free(lnode);
}

void BMBvh::leaf_node_indexing(const std::vector<BMLeafNode*> &nodes)
{
	size_t totnode = nodes.size();
	for (size_t i = 0; i < totnode; ++i){
		nodes[i]->_idx = i;
	}
}

void BMBvh::leaf_node_faces_add_referece(const std::vector<BMLeafNode*> &nodes)
{
	for (auto it = nodes.begin() ; it != nodes.end() ; ++it){
		BMLeafNode *node = *it;
		
		node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);

		/*set node's reference to its faces*/
		const BMFaceVector &faces = node->_faces;
		int totface = faces.size();
		for (size_t j = 0; j < totface; ++j){
			BMFace *f = faces[j];
			elem_leaf_node_set(f, node);
			elem_leaf_node_offset_set(f, j);
#ifdef _DEBUG
			BLI_assert(elem_leaf_node_get(f) == node);
			BLI_assert(elem_leaf_node_offset_get(f) == j);
#endif
		}
	}
}


void BMBvh::base_node_free(BaseNode *bnode)
{
	scalable_aligned_free(bnode);
}

AlignedBox3f BMBvh::bounds()
{
	Vector3f lo(_root->lower().x, _root->lower().y, _root->lower().z);
	Vector3f up(_root->upper().x, _root->upper().y, _root->upper().z);
	return AlignedBox3f(lo, up);
}

void BMBvh::check_valid()
{
	for (size_t i = 0; i < _leafs.size(); ++i){
		BLI_assert(_leafs[i]->idx() == i);
	}

	for (auto it = _leafs.begin(); it != _leafs.end(); ++it){
		BMLeafNode *n = *it;
		const BMFaceVector &faces = n->_faces;
		const size_t totface = faces.size();
		for (size_t i = 0; i < totface; ++i){
			BLI_assert(elem_leaf_node_get(faces[i]) == n);
			int foff = elem_leaf_node_offset_get(faces[i]);
			BLI_assert( foff == i);
		}
		const BMVertVector &verts = n->verts();
		const size_t totvert = verts.size();
		for (size_t i = 0; i < totvert; ++i){
			BMLeafNode *vn = elem_leaf_node_get(verts[i]);
			BLI_assert(vn == n);
			int voff = elem_leaf_node_offset_get(verts[i]);
			BLI_assert(voff == i);
		}
	}

	/*every BMesh's vertices are in bvh tree*/
	{
		BMIter iter;
		BMVert *v;
		BM_ITER_MESH(v, &iter, _bmesh.bm, BM_VERTS_OF_MESH){
			BLI_assert(BM_elem_aux_data_int_get(v, _bmesh.cd_vnode) >= 0);
			BLI_assert(BM_elem_aux_data_int_get(v, _bmesh.cd_voff) >= 0);
			BM_elem_flag_disable(v, BM_ELEM_TAG);
		}

		for (auto it = _leafs.begin(); it != _leafs.end(); ++it){
			BMLeafNode *n = *it;
			const BMVertVector &verts = n->verts();
			const size_t totvert = verts.size();
			for (size_t i = 0; i < totvert; ++i){
				BM_elem_flag_disable(verts[i], BM_ELEM_TAG);
			}
		}

		BM_ITER_MESH(v, &iter, _bmesh.bm, BM_VERTS_OF_MESH){
			BM_elem_flag_enable(v, BM_ELEM_TAG);
		}
		
		for (auto it = _leafs.begin(); it != _leafs.end(); ++it){
			BMLeafNode *n = *it;
			const BMVertVector &verts = n->verts();
			const size_t totvert = verts.size();
			for (size_t i = 0; i < totvert; ++i){
				BLI_assert(BM_elem_flag_test_bool(verts[i], BM_ELEM_TAG));
			}
		}
	}

	/*every BMesh's faces are in bvh tree*/
	{
		BMIter iter;
		BMFace *f;
		BM_ITER_MESH(f, &iter, _bmesh.bm, BM_FACES_OF_MESH){
			BLI_assert(BM_elem_aux_data_int_get(f, _bmesh.cd_fnode) >= 0);
			BLI_assert(BM_elem_aux_data_int_get(f, _bmesh.cd_foff) >= 0);
			BM_elem_flag_disable(f, BM_ELEM_TAG);
		}

		for (auto it = _leafs.begin(); it != _leafs.end(); ++it){
			BMLeafNode *n = *it;
			const BMFaceVector &faces = n->_faces;
			const size_t totface = faces.size();
			for (size_t i = 0; i < totface; ++i){
				BM_elem_flag_disable(faces[i], BM_ELEM_TAG);
			}
		}

		BM_ITER_MESH(f, &iter, _bmesh.bm, BM_FACES_OF_MESH){
			BM_elem_flag_enable(f, BM_ELEM_TAG);
		}

		for (auto it = _leafs.begin(); it != _leafs.end(); ++it){
			BMLeafNode *n = *it;
			const BMFaceVector &faces = n->_faces;
			const size_t totface = faces.size();
			for (size_t i = 0; i < totface; ++i){
				BLI_assert(BM_elem_flag_test_bool(faces[i], BM_ELEM_TAG));
			}
		}
	}

}

void BMBvh::leaf_node_bound_face_norm_update(BMLeafNode *node)
{
	auto face_update = [](BMFace *f, Vector3f &lower, Vector3f &upper)
	{
		switch (f->len)
		{
		case 3:
		{
			BMLoop *l = f->l_first;
			const Vector3f &v0 = l->v->co; l = l->next;
			const Vector3f &v1 = l->v->co; l = l->next;
			const Vector3f &v2 = l->v->co;
			f->no = ((v1 - v0).cross(v2 - v1)).normalized();
			lower = lower.cwiseMin(v0.cwiseMin(v1.cwiseMin(v2)));
			upper = upper.cwiseMax(v0.cwiseMax(v1.cwiseMax(v2)));
			break;
		}
		default:
			break;
		}
	};

	const BMFaceVector &faces = node->faces();
	const size_t totface = faces.size();

	Vector3f lower(FLT_MAX, FLT_MAX, FLT_MAX);
	Vector3f upper(-FLT_MAX, -FLT_MAX, -FLT_MAX);
	for (size_t i = 0; i != totface; ++i){
		face_update(faces[i], lower, upper);
	}

	BBox3fa bb(Vec3fa(lower[0], lower[1], lower[2]), Vec3fa(upper[0], upper[1], upper[2]));
	node->setBB(bb);
}

bool BMBvh::leaf_node_dirty_draw(std::vector<BMLeafNode*> &nodes)
{
	for (BMLeafNode *node : _leafs){
		if (node->appFlagBit(LEAF_UPDATE_STEP_DRAW_BUFFER)){
			node->unsetAppFlagBit(LEAF_UPDATE_STEP_DRAW_BUFFER);
			nodes.push_back(node);
		}
	}
	return !nodes.empty();
}

void BMBvh::leaf_node_dirty_draw_bb(Eigen::AlignedBox3f &bb)
{
	bb.setEmpty();
	for (BMLeafNode *node : _leafs){
		if (node->appFlagBit(LEAF_UPDATE_STEP_DRAW_BUFFER)){
			bb.extend(node->boundsEigen());
		}
	}
}



VBVH_END_NAMESPACE


