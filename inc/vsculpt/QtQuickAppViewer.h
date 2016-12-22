#ifndef QTQUICK2APPLICATIONVIEWER_H
#define QTQUICK2APPLICATIONVIEWER_H

#include <QtQuick/QQuickView>
#include "VEventHandler.h"
#include <Eigen/Dense>
#include <QOpenGLDebugLogger>

class VView3DRegionQuickItem;

class QtQuickAppViewer : public QQuickView
{
	Q_OBJECT
public:
	explicit QtQuickAppViewer(QWindow *parent = 0);
	virtual ~QtQuickAppViewer();

	void	setMainQmlFile(const QString &file);
	void	addImportPath(const QString &path);
	void	showExpanded();
	virtual bool event(QEvent *ev);

	void	onSceneGraphInvalidated();
	void	onSceneGraphInitialized();
	void	onBeforeRendering();
	void	onSceneGraphAboutToStop();

	void	on3DRegionCreated(VView3DRegion *region);
	void	on3DRegionDestroyed(VView3DRegion *region);
	void	beginDrawTarget();
	void	endDrawTarget();
private:
	VEventHandlerManager				*_event_manager;
	std::shared_ptr<gte::DrawTarget>	_renderTarget;/*FrameBuffer on which all region are rendered on*/
	std::vector<VView3DRegion*>			_regions;
	Eigen::Vector2f						_mousepos;
};

#endif // QTQUICK2APPLICATIONVIEWER_H
