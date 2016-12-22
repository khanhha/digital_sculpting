#include "sculpt/SculptCommand.h"
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
VSculptCommand::VSculptCommand(VScene *scene, VMeshObject *obj)
	:
	_scene(scene),
	_obj(obj)
{
}

VSculptCommand::~VSculptCommand()
{
	/*logger::undo must be called earlier*/
	if(_logger) _logger->undo_apply();
}

void VSculptCommand::log_data(const std::vector<BMLeafNode*>& nodes)
{
	_logger = std::shared_ptr<VSculptLogger>(new VSculptLogger(_scene, _obj));
	_logger->log_nodes(nodes);
}

void VSculptCommand::log_data(VSculptLogger &&logger)
{
	_logger = std::shared_ptr<VSculptLogger>(new VSculptLogger(std::move(logger)));
}

#if 0
bool VSculptCommand::doHook()
{
	_state = VUndoEntry::DONE;
	return true;
}

void VSculptCommand::undoHook()
{
	if (_scene->objectExist(_obj)){
		_logger->undo();

		_obj->getBmesh()->BM_mesh_normals_update_parallel();
		_obj->rebuildBVH();
	}
}

void VSculptCommand::redoHook()
{
	if (_scene->objectExist(_obj)){
		_logger->redo();

		_obj->getBmesh()->BM_mesh_normals_update_parallel();
		_obj->rebuildBVH();
	}
}

#endif

