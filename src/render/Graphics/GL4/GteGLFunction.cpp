#include <GTeGLFunction.h>
#include <GteGL4Engine.h>

namespace gte
{
	GL4Engine *GL::_engine = nullptr;
	QOpenGLExtraFunctions *GL::_instance = nullptr;

	bool GL::initialized()
	{
		return (_engine != nullptr && _instance != nullptr);
	}

	void GL::initialize(QOpenGLContext *context)
	{
		if (!initialized()){
			_instance = new QOpenGLExtraFunctions(context);
			_engine = new GL4Engine();
		}
	}

}