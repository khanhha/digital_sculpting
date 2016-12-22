#include <QMouseEvent>
#include <QPoint>
#include <qnamespace.h>
#include "vsculpt/Operator/VSculptBrushOp.h"
#include "VKernel/VMeshObject.h"
#include "BaseLib/MathGeom.h"
#include "VBvh/BMBvhIsect.h"
#include "VKernel/VContext.h"
#include "VKernel/VView3DRegion.h"
#include "VbsQt/VbsDef.h"

#ifdef DEBUG_DRAW
extern std::vector<std::pair<Vector3f, Vector3f>> g_debug_points;
#endif
using namespace vk;
VSculptBrushOp::VSculptBrushOp(VScene *scene)
	:
	_scene(scene),
	_object(nullptr),
	_stroke(nullptr),
	_curve(nullptr),
	_timerid(-1)
{
	_stroke_started = false;

	VSculptConfig *config = _scene->sculptConfig();

	_data.brush_type = config->brush();
	_curve = new BezierCurve(config->brushFalloffCurve());
	_data.deform_curve = _curve;
	_data.sym_flag = 0;
	
	_data.brush_strength = config->brushStrength();
	_data.pinch_factor = 0.5f;
	_data.scale = 0.2f;
	_data.edge_len_unit_threshold = 0.01f;

	_pixel_radius = config->brushPixelSize();
	_detail_size = config->brushDetailsize();
	
	_data.flag = 0;
	_data.flag |= PICK_ORIGINAL_LOCATION;
	if(config->brushDynamicTopology()){
		_data.flag |= DYNAMIC_TOPOLOGY;
	}

	_stroke = new SculptStroke(&_data);
}

VSculptBrushOp::~VSculptBrushOp()
{
	delete _stroke;
	delete _curve;
}

bool VSculptBrushOp::poll()
{
	VScene *scene = VContext::instance()->scene();
	VObject *obj = scene->activeObject();
	if (obj && !obj->beingModified()  && VContext::instance()->region()){
		VMeshObject *meshobj = dynamic_cast<VMeshObject*>(obj);
		if (meshobj && meshobj->state() == VMeshObject::STATE_NONE)
			return true;
	}
	return false;
}

VOperator* VSculptBrushOp::create()
{
	return new VSculptBrushOp(VContext::instance()->scene());
}

int  VSculptBrushOp::invoke(QEvent *ev)
{
	VContext *context = VContext::instance();

	init();

	QMouseEvent *mouseEv = dynamic_cast<QMouseEvent*>(ev);
	QEvent::Type type = ev->type();

	if (mouseEv){
		if (type == QEvent::MouseButtonPress){
			_object->setState(VMeshObject::STATE_SCULPT);

			_last_mouse = Vector2f(mouseEv->pos().x(), context->window()->height() - mouseEv->pos().y());;
			_cur_mouse	= _last_mouse;
			queue_mouse_range(_last_mouse, _cur_mouse);

			apply();

			//_timerid = VContext::instance()->window()->startTimer(2);

			VContext::instance()->window()->update();
			return VOperator::OP_RET_RUNNING;
		}
	}

	return VOperator::OP_RET_FINISHED;
}

int VSculptBrushOp::modal(QEvent *ev)
{
	VContext *context = VContext::instance();
	QMouseEvent *mev = dynamic_cast<QMouseEvent*>(ev);
	if (mev){
		Vector2f mpoint(mev->pos().x(), context->window()->height() - mev->pos().y());
		if (ev->type() == QEvent::MouseMove){
			
			_last_mouse = _cur_mouse;
			_cur_mouse  = mpoint;
			queue_mouse_range(_last_mouse, _cur_mouse);
            apply();

			VContext::instance()->window()->update();
			return VOperator::OP_RET_RUNNING;
		}
		else if(ev->type() == QEvent::MouseButtonRelease){

			cancel();

			VContext::instance()->window()->update();
			return VOperator::OP_RET_FINISHED;
		}
	}
	//else if (_timerid != -1 && ev->type() == QEvent::Timer){
	//	QTimerEvent *tev = dynamic_cast<QTimerEvent*>(ev);
	//	if (tev && _timerid == tev->timerId()){
	//		apply();
	//	}
	//}

	return VOperator::OP_RET_RUNNING;
}

void VSculptBrushOp::init()
{
	_object = dynamic_cast<VMeshObject*>(_scene->activeObject());
	_object->beginModify();

	_region = VContext::instance()->region();

	BLI_assert(_region && _object);

	_data.scene = _scene;
	_data.object = _object;
	_data.bvh = _object->getBmeshBvh();
	_data.bm = _object->getBmesh();

}

void VSculptBrushOp::apply()
{
	if (!_stroke_started){
		init_stroke(_cur_mouse);
	}

	if (_stroke_started){

		if (is_sample_mouse()){
			if (!_queuemouse.empty()){
				
				std::pair<Vector2f, Vector2f> mrange = _queuemouse.front(); _queuemouse.pop();
				
				if ((mrange.first - mrange.second).norm() < FLT_EPSILON){
					add_step(mrange.first, mrange.second, _first_hit_mouse);
				}
				else{
					sample_mouse(mrange.first, mrange.second); /*automatic assign last mouse during sampling*/
				}
			}
		}
		else{
			//add_step();
		}
	}
}

void VSculptBrushOp::cancel()
{
	_stroke->finish_stroke();

	_object->gpuChangedBoundsReset();
	_object->setState(VMeshObject::STATE_NONE);
	_object->endModify();
}

void VSculptBrushOp::add_step(Vector2f cur_mouse, Vector2f last_mouse, Vector2f first_hit_mouse)
{
	if (update(cur_mouse, last_mouse, first_hit_mouse)){
		_stroke->add_step(false);
	}
}

bool VSculptBrushOp::update(Vector2f cur_mouse, Vector2f last_mouse, Vector2f first_hit_mouse)
{
	int btype = _data.brush_type;
	bool ret = false;
	if (is_one_step_stroke())
	{
		Vector3f npoint, fpoint, intPoint;
		
		VContext::instance()->viewLine(_region, cur_mouse, npoint, fpoint);

		MathGeom::closest_to_line_v3(intPoint, _data.cur_pos, npoint, fpoint);

		if (btype == VbsDef::BRUSH_SNAKE_HOOK){
			_data.cur_pos = _data.cur_pos + _data.grab_delta;

			_data.grab_delta = intPoint - _data.last_pos;
			_data.last_pos = intPoint;
		}
		else{
			/*rotate, grab, thumb brush*/
			_data.cur_pos = _data.first_hit_pos;
			_data.grab_delta = intPoint - _data.first_hit_pos;
		}

		if (btype == VbsDef::BRUSH_ROTATE){
			Vector2f last_mouse_dir = last_mouse - first_hit_mouse;
			Vector2f cur_mouse_dir	= cur_mouse - first_hit_mouse;
			const double threshold = 0.3f * _pixel_radius;
			if (cur_mouse_dir.norm() > threshold && last_mouse_dir.norm() > DBL_EPSILON)
			{
				cur_mouse_dir.normalize();
				last_mouse_dir.normalize();
				const float acos = last_mouse_dir.dot(cur_mouse_dir);
				const float asin = cur_mouse_dir[0] * last_mouse_dir[1] - cur_mouse_dir[1] * last_mouse_dir[0] /*cross*/;
				_data.rotate_angle = asin > 0 ? std::acos(acos) : -std::acos(acos);
			}
			else{
				_data.rotate_angle = 0.0f;
			}
		}
		ret = true;
	}
	else{
		Vector3f npoint, fpoint;

		_data.view_dir = -VContext::instance()->viewModelDirection(_region, _scene);

		ret = pick(cur_mouse, _data.cur_pos);

		_data.grab_delta = _data.cur_pos - _data.last_pos;
		_data.last_pos = _data.cur_pos;
	}

	interpolate_param();

	return ret;
}

bool VSculptBrushOp::pick(Vector2f mouse, Vector3f &hit)
{
	VContext *context = VContext::instance();
	Vector3f org, dir, far_p;

	context->viewLine(_region, mouse, org, far_p);

	dir = far_p - org; dir.normalize();
	bool ret;
	if (_data.flag & PICK_ORIGINAL_LOCATION){
		ret = isect_ray_bm_bvh_nearest_hit(_data.bvh, org, dir, true, hit, 1.0e-6);
	}
	else{
		ret = isect_ray_bm_bvh_nearest_hit(_data.bvh, org, dir, false, hit, 1.0e-6);
	}
	return ret;
}

void VSculptBrushOp::interpolate_param()
{
#if 0
	_data.world_radius = _scene->glInfor()->worldRadius(_pixel_radius, _curMouse.x(), _curMouse.y(), _data.cur_pos);
#else
	_data.world_radius = VContext::instance()->screenDistanceToModelDistance(_region, _pixel_radius, _data.cur_pos);
#endif

	float worldPerPixelUnitLen = (_data.world_radius / _pixel_radius) * 3.0;
	if (worldPerPixelUnitLen < _data.edge_len_unit_threshold)
		worldPerPixelUnitLen = _data.edge_len_unit_threshold;

	_data.max_edge_len = worldPerPixelUnitLen * _detail_size;
	_data.min_edge_len = 0.4f * _data.max_edge_len;
}

/*(start, end]*/
void VSculptBrushOp::sample_mouse(Vector2f start, Vector2f end)
{
	Vector2f cur = start;
	Vector2f last = start;
	Vector2f mouseDiff = end - start;
	float length	= mouseDiff.norm(); mouseDiff.normalize();

	float spacing = _pixel_radius * 0.2f;
	size_t cnt = 0;
	while (length >= spacing){

		last = cur;
		cur = cur  + mouseDiff * spacing;

		//if (cnt > 0){
			/*ignore the first step*/
		add_step(cur, last, _first_hit_mouse);
		//}

		length -= spacing;
		cnt++;
	}

	if (length > 0.4f * spacing ){
		add_step(end, cur, _first_hit_mouse);
	}
}

bool VSculptBrushOp::is_one_step_stroke()
{
	int btype = _data.brush_type;

	if (btype == VbsDef::BRUSH_GRAB ||
		btype == VbsDef::BRUSH_SNAKE_HOOK ||
		btype == VbsDef::BRUSH_THUMB ||
		btype == VbsDef::BRUSH_ROTATE){
		return true;
	}
	else{
		return false;
	}
}

/*should we sample mouse*/
bool VSculptBrushOp::is_sample_mouse()
{
	if (is_one_step_stroke()){
		return false;
	}
	else{
		return true;
	}
}

bool VSculptBrushOp::init_stroke(Vector2f curmouse)
{
	/*rest undo/redo logger for safety*/
	_data.undo_redo_logger = nullptr;
	_data.view_dir = -VContext::viewModelDirection(_region, _scene);
	
	/*stroke hit object for the first time. Mark the stroke as started and initialize undo/redo*/
	if (pick(curmouse, _data.cur_pos)){

		_first_hit_mouse = curmouse;
		_data.first_hit_pos = _data.cur_pos;
		_data.last_pos = _data.cur_pos;
		_stroke_started = true;

		interpolate_param();
		init_undo_redo();

		return true;
	}
	else{
		return false;
	}
}

void VSculptBrushOp::init_undo_redo()
{
#if 0
	_data.undo_redo_logger = new VSculptCommand(_scene, _data.object);
	VUndoManager::getInstance()->execute(_data.undo_redo_logger);
#endif
}

void VSculptBrushOp::queue_mouse_range(Vector2f start, Vector2f end)
{
#if 0
    const float step = 30; /*pixel*/
    Vector2f mouseDiff = end - start;
    float length = mouseDiff.norm(); mouseDiff.normalize();

    Vector2f cur = start;
    Vector2f last = start;
    while (length > step){
        last = cur;
        cur = last + mouseDiff * step;
        _queuemouse.push(std::make_pair(last, cur));
        length -= step;
    }

    if (length >= 0.0f){
        _queuemouse.push(std::make_pair(last, end));
    }
#else
    _queuemouse.push(std::make_pair(start, end));
#endif
}

