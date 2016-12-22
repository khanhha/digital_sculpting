#ifndef SCULPT_COMMAND_H
#define SCULPT_COMMAND_H
#include <vector>
#include "VBvh//BMeshBvh.h"
#include "VKernel/VScene.h"
#include "VKernel/VMeshObject.h"
#include "VSculptLogger.h"

using namespace VBvh;

class VSculptCommand
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
	VSculptCommand(vk::VScene *scene, vk::VMeshObject *obj);
	~VSculptCommand();
protected:
	/*method called from system. mutex must be locked here*/
	bool doHook();
	void undoHook();
	void redoHook();
public:
	void log_data(const std::vector<BMLeafNode*>& nodes);
	void log_data(VSculptLogger &&logger);
private:
	vk::VScene *_scene;
	vk::VMeshObject* _obj;
	std::shared_ptr<VSculptLogger> _logger;
};
#endif