#include "VView3DRegionQuickItem.h"
#include "QtQuickAppViewer.h"
#include "VContext.h"
#include <tbb/mutex.h>

using namespace  vk;
extern tbb::mutex g_render_gui_mutex;

//! [7]
VView3DRegionQuickItem::VView3DRegionQuickItem()
	: m_t(0)
{
	setFlag(ItemAcceptsDrops);

	connect(this, &QQuickItem::windowChanged, this, &VView3DRegionQuickItem::handleWindowChanged);
	connect(this, &VView3DRegionQuickItem::tChanged, this, &VView3DRegionQuickItem::sync);

	_region = new VView3DRegion();
	_region->setScene(VContext::instance()->scene());
	_region->camera()->setAspectRatio(width() / height());
}

VView3DRegionQuickItem::~VView3DRegionQuickItem()
{
	QtQuickAppViewer *win = dynamic_cast<QtQuickAppViewer*>(window());
	if (win){
		win->on3DRegionDestroyed(_region);
	}
}

void VView3DRegionQuickItem::setT(qreal t)
{
	if (t == m_t)
		return;
	m_t = t;
	Q_EMIT tChanged();
	if (window())
		window()->update();
}

void VView3DRegionQuickItem::handleWindowChanged(QQuickWindow *win)
{
	if (win) {

		QtQuickAppViewer *win = dynamic_cast<QtQuickAppViewer*>(window());
		win->on3DRegionCreated(_region);

		connect(win, &QQuickWindow::beforeSynchronizing, this, &VView3DRegionQuickItem::sync, Qt::DirectConnection);
		connect(win, &QQuickWindow::sceneGraphInvalidated, this, &VView3DRegionQuickItem::cleanup, Qt::DirectConnection);
		//! [1]
		// If we allow QML to do the clearing, they would clear what we paint
		// and nothing would show.
		//! [3]
		win->setClearBeforeRendering(false);
	}
}

void VView3DRegionQuickItem::cleanup()
{
	assert(false);
}


void VView3DRegionQuickItem::sync()
{
	QtQuickAppViewer *win = dynamic_cast<QtQuickAppViewer*>(window());
	float glx = x();
	float gly = win->height() - (y() + height());

	QRect rect(glx, gly, width() * win->devicePixelRatio(), height() * win->devicePixelRatio());
	
	if (!_region->initialized()) {
		float aspect = width() / height();
		_region->initialize();
		_region->camera()->setAspectRatio(aspect);
		_region->setViewport(rect);

	}

	_region->setViewport(rect);

}

