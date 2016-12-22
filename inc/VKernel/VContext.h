#ifndef VSCULPT_VCONTEXT_H
#define VSCULPT_VCONTEXT_H

#include "VScene.h"
#include <QQuickWindow>
#include <QQuickItem>
#include <QRectF>

VK_BEGIN_NAMESPACE
struct VCommonConfig
{
	VCommonConfig(){}
};

class VContext
{
public:
	static VContext* instance();
public:
	~VContext();

	VScene* scene();

	QQuickWindow* window();
	void	setWindow(QQuickWindow *win);

	VView3DRegion*			region();
	void					setRegion(VView3DRegion *region);

	static void				viewLine(VView3DRegion *region, Eigen::Vector2f win, Eigen::Vector3f &l1, Eigen::Vector3f &l2);
	static void				viewRay(VView3DRegion *region, Eigen::Vector2f win, Eigen::Vector3f &org, Eigen::Vector3f &dir);
	static float			screenDistanceToModelDistance(VView3DRegion *region, float pixel_dst, Eigen::Vector3f ref_pos);
	static Eigen::Vector3f	project(VView3DRegion *region, Eigen::Vector3f &pos);
	static Eigen::Vector3f  unproject(VView3DRegion *region, Eigen::Vector3f &win);
	static Vector3			viewModelDirection(VView3DRegion *region, VScene *scene);
	VCommonConfig*			commonConfig();
	Eigen::Vector2f			currentGLMouse();
private:
	VContext();
private:
	static VContext		_context;
	VScene				*_scene;
	QQuickWindow		*_window;
	VView3DRegion		*_region;  /*current region under mouse position*/
	VCommonConfig	     _config;
};

VK_END_NAMESPACE
#endif