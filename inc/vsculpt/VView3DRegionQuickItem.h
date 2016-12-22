#ifndef VSCULPT_VRENDER_QUICK_ITEM_H
#define VSCULPT_VRENDER_QUICK_ITEM_H

#include <QtQuick/QQuickItem>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLFunctions>
#include <Graphics/GL4/GteGL4Engine.h>
#include <Graphics/GL4/GteGLSLProgramFactory.h>
#include <GTGraphics.h>
#include <QEvent>
#include "VKernel/VView3DRegion.h"

class VView3DRegionQuickItem : public QQuickItem
{
	Q_OBJECT
		Q_PROPERTY(qreal t READ t WRITE setT NOTIFY tChanged)
public:
	VView3DRegionQuickItem();
	~VView3DRegionQuickItem();
	qreal t() const { return m_t; }
	void setT(qreal t);
Q_SIGNALS:
	void tChanged();

public Q_SLOTS:
	void sync();
	void cleanup();

private Q_SLOTS:
	void handleWindowChanged(QQuickWindow *win);
private:
	qreal m_t;
	vk::VView3DRegion *_region;
};

#endif