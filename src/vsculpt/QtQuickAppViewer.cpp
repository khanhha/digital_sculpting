#include "vsculpt/QtQuickAppViewer.h"
#include "VKernel/VView3DRegion.h"
#include "VKernel/VSceneRenderer.h"
#include <QtCore/QDir>
#include <QtQml/QQmlEngine>
#include <GTGraphics.h>
#include "VContext.h"
#include <QtCore/QCoreApplication>
#include <tbb/mutex.h>
#include <QTimer>
#include <QOpenGLDebugLogger>

using namespace vk;
using namespace gte;

extern tbb::mutex g_render_gui_mutex;

QtQuickAppViewer::QtQuickAppViewer(QWindow *parent)
	:
	QQuickView(parent)
{
	//setAcceptDrops(true);

	connect(engine(), SIGNAL(quit()), SLOT(close()));
	setResizeMode(QQuickView::SizeRootObjectToView);
	
	_event_manager = new VEventHandlerManager();
	
	connect(this, &QQuickWindow::sceneGraphInvalidated, this, &QtQuickAppViewer::onSceneGraphInvalidated, Qt::DirectConnection);
	connect(this, &QQuickWindow::sceneGraphInitialized, this, &QtQuickAppViewer::onSceneGraphInitialized, Qt::DirectConnection);
	connect(this, &QQuickWindow::sceneGraphAboutToStop, this, &QtQuickAppViewer::onSceneGraphAboutToStop, Qt::DirectConnection);

	connect(this, &QQuickWindow::beforeRendering, this, &QtQuickAppViewer::onBeforeRendering, Qt::DirectConnection);
}

QtQuickAppViewer::~QtQuickAppViewer()
{
}

void QtQuickAppViewer::setMainQmlFile(const QString &file)
{
	setSource(QUrl::fromLocalFile(file));
}

void QtQuickAppViewer::addImportPath(const QString &path)
{
	engine()->addImportPath(path);
}

void QtQuickAppViewer::showExpanded()
{
	show();
}

bool QtQuickAppViewer::event(QEvent *ev)
{
	QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
	if (mev){
		_mousepos = Vector2f(mev->pos().x(), mev->pos().y());
	}

	if (_event_manager){
		
		for (auto it = _regions.begin(); it != _regions.end(); ++it){
			VView3DRegion *region = *it;
			if (region->viewport().contains(_mousepos.x(), height() - _mousepos.y())){
				VContext::instance()->setRegion(region);
			}
		}
		_event_manager->handle(ev);
	}
	return QQuickView::event(ev);
}

void QtQuickAppViewer::onSceneGraphInvalidated()
{
	_renderTarget.reset();
}

void QtQuickAppViewer::onSceneGraphInitialized()
{}

void QtQuickAppViewer::onSceneGraphAboutToStop()
{
	_renderTarget.reset();
}

void QtQuickAppViewer::onBeforeRendering()
{
	tbb::mutex::scoped_lock lock(g_render_gui_mutex);

	{
		VContext *context = VContext::instance();

		gte::GL::initialize(openglContext());

		context->scene()->syncUpdate();
		VSceneRender::instance()->syncUpdate(context);

		for (auto it = _regions.begin(); it != _regions.end(); ++it){
			VView3DRegion *region = *it;
			region->initialize();
			region->syncUpdate(VContext::instance());
		}
	}

	beginDrawTarget();

	for (auto it = _regions.begin(); it != _regions.end(); ++it){
		VView3DRegion *region = *it;
		region->onRender(VContext::instance());
	}

	endDrawTarget();
}

void QtQuickAppViewer::on3DRegionCreated(VView3DRegion *region)
{
	auto it = std::find_if(_regions.begin(), _regions.end(), [&](VView3DRegion *r){ return r == region; });
	if (it == std::end(_regions)){
		_regions.push_back(region);
	}
}

void QtQuickAppViewer::on3DRegionDestroyed(VView3DRegion *region)
{
	auto it = std::find_if(_regions.begin(), _regions.end(), [&](VView3DRegion *r){ return r == region; });
	if (it != std::end(_regions)){
		_regions.erase(it);
	}
}

void QtQuickAppViewer::beginDrawTarget()
{
	if (!_renderTarget){
		_renderTarget = std::make_shared<DrawTarget>(
			1, DF_R32G32B32A32_FLOAT, width(), height(),
			false, false, DF_D24_UNORM_S8_UINT, false);

		GL::engine()->Enable(_renderTarget);
		GL::engine()->ClearBuffers();
	}

	GL::engine()->Enable(_renderTarget);
}

void QtQuickAppViewer::endDrawTarget()
{
	GL::engine()->Disable(_renderTarget);

	GL4DrawTarget *glTarget = dynamic_cast<GL4DrawTarget*>(GL::engine()->Get(_renderTarget));
	if (glTarget){
		GLint width = _renderTarget->GetWidth();
		GLint height = _renderTarget->GetHeight();
		GL::function().glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		GL::function().glBindFramebuffer(GL_READ_FRAMEBUFFER, glTarget->GetFrameBuffer());

		GL::function().glReadBuffer(GL_COLOR_ATTACHMENT0);
		GL::function().glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
	}
}

