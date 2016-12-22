#include "sculpt/VSculptLogger.h"
#include "tbb/mutex.h"
#include "tbb/parallel_for.h"
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include "sculpt/SUtil.h"
#include "VKernel/VScene.h"
#include "VKernel/VMeshObject.h"
#include "BaseLib/MathUtil.h"

////////////////////////////////////////////////////////////////////////////
VSculptLogger::VSculptLogger(VScene *scene, VMeshObject *obj)
	:
	_scene(scene),
	_obj(obj)
{}

VSculptLogger::VSculptLogger(VSculptLogger &&other)
	:
	_scene(other._scene),
	_obj(other._obj),
	_changed_verts(std::move(other._changed_verts)),
	_created_verts(std::move(other._created_verts)),
	_created_faces(std::move(other._created_faces)),
	_deleted_verts(std::move(other._deleted_verts)),
	_deleted_faces(std::move(other._deleted_faces))
{
	other._scene = nullptr;
	other._obj = nullptr;
	other._changed_verts.clear();
	other._created_faces.clear();
	other._created_verts.clear();
	other._deleted_verts.clear();
	other._deleted_faces.clear();
}

VSculptLogger::~VSculptLogger()
{}

void VSculptLogger::undo(bool bvh_undo /*= false*/)
{
	if (_scene->objectExist(_obj)){
		BMesh *bm = _obj->getBmesh();
		BMBvh *bvh = _obj->getBmeshBvh();

		for (auto it = _deleted_verts.begin(); it != _deleted_verts.end(); ++it){
			BMVert *v = *it;
			/*deleted verts will automatically deleted again when its face were deleted*/
			if (bvh_undo){
				bvh->leaf_node_vert_add(bvh->elem_leaf_node_get(v), v);
			}
		}

		for (auto it = _created_verts.begin(); it != _created_verts.end(); ++it){
			BMVert *v = *it;
			/*created verts will automatically restored again when its face was restored*/
			if (bvh_undo){
				bvh->leaf_node_vert_remove(bvh->elem_leaf_node_get(v), v);
			}
		}

		for (auto it = _changed_verts.begin(); it != _changed_verts.end(); ++it){
			BMVLog &vlog = *it;
			std::swap(vlog.co, vlog.v->co);
		}

		/*kill and log all faces created earlier*/
		for (auto it = _created_faces.begin(); it != _created_faces.end(); ++it){
			BMFaceLog &flog = *it;
			BLI_assert(BM_elem_flag_test(flog.f, BM_ELEM_REMOVED) == 0);
			if (bvh_undo){
				bvh->leaf_node_face_remove(bvh->elem_leaf_node_get(flog.f), flog.f);
			}
			bm->BM_face_kill_loose(flog.f, true);
		}

		for (auto it = _deleted_faces.begin(); it != _deleted_faces.end(); ++it){
			BMFaceLog &flog = *it;

			BLI_assert(BM_elem_flag_test(flog.f, BM_ELEM_REMOVED) != 0);

			if (bvh_undo){
				bvh->leaf_node_face_add(bvh->elem_leaf_node_get(flog.f), flog.f);
			}
			bm->BM_face_logged_restore(flog.f, flog.verts, 3, BM_CREATE_NO_DOUBLE);
		}

		if (bvh_undo && _nodes.size() > 0){
			for (auto it = _nodes.begin(); it != _nodes.end(); ++it){
				bvh->leaf_node_org_data_drop(*it);
			}
		}
	}
}

void VSculptLogger::redo(bool bvh_undo /*= false*/)
{
	if (_scene->objectExist(_obj)){
		BMesh *bm = _obj->getBmesh();
		BMBvh *bvh = _obj->getBmeshBvh();

		for (auto it = _deleted_verts.begin(); it != _deleted_verts.end(); ++it){
			BMVert *v = *it;
			if (bvh_undo){
				bvh->leaf_node_vert_add(bvh->elem_leaf_node_get(v), v);
			}
		}

		for (auto it = _created_verts.begin(); it != _created_verts.end(); ++it){
			BMVert *v = *it;
			if (bvh_undo){
				bvh->leaf_node_vert_remove(bvh->elem_leaf_node_get(v), v);
			}
		}

		for (auto it = _created_faces.begin(); it != _created_faces.end(); ++it){
			BMFaceLog &flog = *it;
			bm->BM_face_logged_restore(flog.f, flog.verts, 3, BM_CREATE_NO_DOUBLE);
			if (bvh_undo){
				bvh->leaf_node_face_add(bvh->elem_leaf_node_get(flog.f), flog.f);
			}
		}

		for (auto it = _deleted_faces.begin(); it != _deleted_faces.end(); ++it){
			BMFaceLog &flog = *it;
			if (bvh_undo){
				bvh->leaf_node_face_remove(bvh->elem_leaf_node_get(flog.f), flog.f);
			}
			bm->BM_face_kill_loose(flog.f, true);
		}

		for (auto it = _changed_verts.begin(); it != _changed_verts.end(); ++it){
			BMVLog &vlog = *it;
			std::swap(vlog.co, vlog.v->co);
		}
	}
}

/*this method just frees all memory of created vertices and faces*/
/*NOTE: undo must be called earlier for ensuring consistency property*/
void VSculptLogger::undo_apply()
{
	if (_scene->objectExist(_obj)){
		BMesh *bm = _obj->getBmesh();
		BMBvh *bvh = _obj->getBmeshBvh();

		/*real kill all created faces*/
		for (auto it = _created_faces.begin(); it != _created_faces.end(); ++it){
			BMFaceLog &flog = *it;
			BLI_assert(BM_elem_flag_test(flog.f, BM_ELEM_REMOVED) != 0);
			bm->BM_face_logged_kill_only(flog.f);
		}

		for (auto it = _created_verts.begin(); it != _created_verts.end(); ++it){
			BMVert *v = *it;
			BLI_assert(BM_elem_flag_test(v, BM_ELEM_REMOVED) != 0);
			bm->BM_vert_logged_kill_only(v);
		}
	}
}

void VSculptLogger::log_nodes(const std::vector<BMLeafNode*>& nodes, bool keep_node/*false*/)
{
	log_faces(nodes);
	log_verts(nodes);
	
	if (keep_node){
		_nodes = nodes;
	}
}

void VSculptLogger::log_faces(const std::vector<BMLeafNode*>& nodes)
{
	for (auto it = nodes.begin(); it != nodes.end(); ++it){
		BMLeafNode *node = *it;
		OriginLeafNodeData *orgnode = node->originData();

		/*loop over original faces for deleted faces*/
		BMFaceBackup *orgfaces = orgnode->_faces;
		const size_t  totorgfaces = orgnode->_totfaces;
		for (size_t i = 0; i < totorgfaces; ++i){
			BMFaceBackup &fbackup = orgfaces[i];
			if (BM_elem_flag_test(fbackup.face, BM_ELEM_REMOVED) != 0){
				_deleted_faces.push_back(BMFaceLog());
				BMFaceLog &log = _deleted_faces.back();
				log.f = fbackup.face;
				log.verts[0] = fbackup.verts[0];
				log.verts[1] = fbackup.verts[1];
				log.verts[2] = fbackup.verts[2];
			}
		}

		/*loop over current faces for created faces*/
		const BMFaceVector &faces = node->faces();
		const size_t totface = faces.size();
		for (auto it = faces.begin(); it != faces.end(); ++it){
			BMFace *f = *it;
			if (BM_elem_app_flag_test(f, F_NEW_SUBDIVISION_TRIANGLE)){
				BM_elem_app_flag_disable(f, F_NEW_SUBDIVISION_TRIANGLE);
				_created_faces.push_back(BMFaceLog());
				BMFaceLog &log = _created_faces.back();
				log.f = f;
				BM_face_as_array_vert_tri(f, log.verts);
			}
		}
	}
}

void VSculptLogger::log_verts(const std::vector<BMLeafNode*>& nodes)
{
	size_t totvert = 0;
	for (auto it = nodes.begin(); it != nodes.end(); ++it){
		BLI_assert((*it)->originData() != nullptr);
		totvert += (*it)->originData()->_totverts;
	}

	_changed_verts.reserve(totvert);

	for (auto it = nodes.begin(); it != nodes.end(); ++it){

		OriginLeafNodeData *orgdata = (*it)->originData();
		BMVertBackup *orgverts = orgdata->_verts;
		const size_t totorgverts = orgdata->_totverts;

		for (size_t i = 0; i < totorgverts; ++i){
			const BMVertBackup &vbackup = orgverts[i];

			/*save vertex's original coordinate*/
			_changed_verts.push_back(BMVLog());
			BMVLog &vlog = _changed_verts.back();
			vlog.v = vbackup.v;
			vlog.co = vbackup.co;

			/*removed faces*/
			if (BM_elem_flag_test(vlog.v, BM_ELEM_REMOVED) != 0){
				_deleted_verts.push_back(vlog.v);
			}
		}

		const BMVertVector &verts = (*it)->verts();
		for (auto it = verts.begin(); it != verts.end(); ++it){
			BMVert *v = *it;
			if (BM_elem_app_flag_test(v, V_NEW_SUBDIVISION_VERTEX)){
				BM_elem_app_flag_disable(v, V_NEW_SUBDIVISION_VERTEX);
				_created_verts.push_back(v);
			}
		}
	}
}

const std::vector<BMLeafNode*>& VSculptLogger::nodes()
{
	return _nodes;
}
