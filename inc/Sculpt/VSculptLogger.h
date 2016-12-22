#ifndef SCULPT_LOGGER_H
#define SCULPT_LOGGER_H

#include <vector>
#include "VBvh//BMeshBvh.h"
#include "VKernel/VScene.h"
#include "VKernel/VMeshObject.h"

using namespace vk;

class VSculptLogger
{
private:
	struct BMFaceLog
	{
		BMFace *f;
		BMVert *verts[3];
	};

	struct BMVLog
	{
		BMVert* v;
		Vector3f co;
	};

public:
	VSculptLogger(VScene *scene, VMeshObject *obj);
	VSculptLogger(VSculptLogger &&other);
	~VSculptLogger();
	void log_nodes(const std::vector<BMLeafNode*>& nodes, bool keep_node = false);
	void undo(bool bvh_undo = false);
	void redo(bool bvh_undo = false);
	void undo_apply();
	const std::vector<BMLeafNode*>& nodes();
private:
	void log_faces(const std::vector<BMLeafNode*>& nodes);
	void log_verts(const std::vector<BMLeafNode*>& nodes);
private:
	VScene *_scene;
	VMeshObject* _obj;
	std::vector<BMLeafNode*>	_nodes;
	std::vector<BMVLog>			_changed_verts;
	std::vector<BMFaceLog>		_created_faces;
	std::vector<BMFaceLog>		_deleted_faces;
	std::vector<BMVert*>		_created_verts;
	std::vector<BMVert*>		_deleted_verts;
};
#endif