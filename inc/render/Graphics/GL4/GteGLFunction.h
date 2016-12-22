#pragma once
#include <QtGui/QOpenGLFunctions>
#include <QtGui/QOpenGLExtraFunctions>
namespace gte
{
	class GL4Engine;

	class GL : public QOpenGLExtraFunctions
	{
	public:
		static bool initialized();
		static void initialize(QOpenGLContext *context);

		static __forceinline QOpenGLExtraFunctions& function()
		{
			return *_instance;
		}

		static __forceinline GL4Engine* engine()
		{
			return _engine;
		}

	private:
		static QOpenGLExtraFunctions  *_instance;
		static GL4Engine			  *_engine;
	};
}