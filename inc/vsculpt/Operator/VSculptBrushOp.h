#ifndef VSCULPT_SCULT_BRUSH_OPERATOR_H
#define VSCULPT_SCULT_BRUSH_OPERATOR_H

#include "VKernel/VScene.h"
#include "VOperator.h"
#include "BezierCurve.h"
#include "StrokeData.h"
#include "brush/SculptStroke.h"
#include "commonDefine.h"
#include "VKernel/VMeshObject.h"
#include <queue>

class VSculptBrushOp : public VOperator
{
public:
	VSculptBrushOp(vk::VScene *scene);
	virtual ~VSculptBrushOp();

	static bool poll();
	static VOperator* create();

	virtual int  modal(QEvent *ev);
	virtual int  invoke(QEvent *ev);
private:
	void init();
	void apply();
	void cancel();

	void add_step(Vector2f cur_mouse, Vector2f last_mouse, Vector2f first_hit_mouse);
	bool update(Vector2f cur_mouse, Vector2f last_mouse, Vector2f first_hit_mouse);
	bool init_stroke(Vector2f curmouse);
	bool is_sample_mouse();
	void sample_mouse(Vector2f start, Vector2f end);
	bool is_one_step_stroke();

	bool pick(Vector2f mouse, Vector3f &hit);
	void interpolate_param();
	void init_undo_redo();
	void queue_mouse_range(Vector2f start, Vector2f end);
private:
	VScene *_scene;
	VMeshObject *_object;
	BVHRenderer *_renderer;
	VView3DRegion	*_region;

	Vector2f _first_hit_mouse;
	
	Vector2f _last_mouse;
	Vector2f _cur_mouse;
	int		_timerid;
	float _detail_size;
	float _pixel_radius;
	bool  _stroke_started;

	StrokeData  _data;

	SculptStroke *_stroke;
	BezierCurve *_curve;

	std::queue<std::pair<Vector2f, Vector2f>> _queuemouse;
};
#endif