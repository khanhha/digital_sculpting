#include "brush/SculptStroke.h"
#include "BMSplitCollapseOp.h"
#include "NormalUpdateOp.h"
#include "SculptCommand.h"
#include "SUtil.h"
#include "tbb/parallel_for.h"
#include "brush/ClayStripBrushOp.h"
#include "brush/ClayBrushOp.h"
#include "brush/DrawBrushOp.h"
#include "brush/SmoothBrushOp.h"
#include "brush/FillBrushOp.h"
#include "brush/FlattenBrushOp.h"
#include "brush/PinchBrushOp.h"
#include "brush/InflateBrushOp.h"
#include "brush/NudgetBrushOp.h"
#include "brush/CreaseBrushOp.h"
#include "brush/RotateBrushOp.h"
#include "brush/GrabBrushOp.h"
#include "brush/SnakeHookBrushOp.h"
#include "brush/ThumbBrushOp.h"
#include "commonDefine.h"
#include "BaseLib/MathUtil.h"
#include "VBvh/BMBvhIsect.h"
#include "BaseLib/Point3Dd.h"
#include "VbsQt/VbsDef.h"

SculptStroke::SculptStroke(StrokeData *data)
	:
	_data(data),
	_step_logger(nullptr)
{}

SculptStroke::~SculptStroke()
{
	BLI_assert(_step_logger == nullptr);
	delete _step_logger;
	_step_logger = nullptr;
}

void SculptStroke::add_step(bool log_step)
{
	if (log_step){
		step_logger_begin();
	}

	size_t symm = _data->sym_flag;
	std::vector<BMLeafNode*> nodes;
	//_symFlag is a bit combination of XYZ - 1 is mirror X; 2 is Y; 3 is XY; 4 is Z; 5 is XZ; 6 is YZ; 7 is XYZ
	for (size_t i = 0; i <= symm; ++i){
		if (i == 0 || (symm & i && (symm != 5 || i != 3) && (symm != 6 || (i != 3 && i != 5)))){
			calc_symm_data(i);
			nodes.clear();
			Vector3f center = _data->symn_data.cur_pos;
			if (isect_sphere_bm_bvh(_data->bvh, center, _data->world_radius, nodes)){
				save_origin_node_data(nodes);
				update_topology();
				do_brush();
			}
		}
	}
	_data->bvh->sculpt_stroke_step_update();
	
	if (log_step){
		step_logger_end();
	}
}

void SculptStroke::step_logger_begin()
{
	if (_step_logger){
		_step_logger->undo(true);
		_step_logger->undo_apply();
		delete _step_logger; _step_logger = nullptr;
	}
}

void SculptStroke::step_logger_end()
{
	BLI_assert(_step_logger == nullptr);

	std::vector<BMLeafNode*> nodes;
	if (_data->bvh->leaf_node_flagged_collect(LEAF_ORIGIN_DATA_SAVED, nodes)){
		_step_logger = new VSculptLogger(_data->scene, _data->object);
		_step_logger->log_nodes(nodes, true/*log node for clear node's origin data later*/);
	}
}

void SculptStroke::finish_stroke()
{
	push_undo_redo();
	_data->bvh->sculpt_stroke_finish_update();
}

void SculptStroke::do_brush()
{
	int btype = _data->brush_type;
	switch (btype)
	{
	case VbsDef::BRUSH_INFLATE:
	{
		InflateBrushOp op(_data);
		op.run();
		break;
	}
	case VbsDef::BRUSH_SNAKE_HOOK:
	{
		SnakeHookBrushOp op(_data);
		op.run();
		break;
	}
	case VbsDef::BRUSH_CREASE:
	{
		CreaseBrushOp op(_data);
		op.run();
		break;
	}
	case VbsDef::BRUSH_FLATTEN:
	{
		FlattenBrushOp op(_data);
		op.run();
		break;
	}
	case VbsDef::BRUSH_NUDGET:
	{
		NudgetBrushOp op(_data);
		op.run();
		break;
	}
	case VbsDef::BRUSH_CLAY_STRIP:
	{
		ClayStripBrushOp op(_data);
		op.run();
		break;
	}
	case VbsDef::BRUSH_CLAY:
	{
		ClayBrushOp op(_data);
		op.run();
		break;
	}
	case VbsDef::BRUSH_FILL:
	{
		FillBrushOp op(_data);
		op.run();
		break;
	}
	case VbsDef::BRUSH_ROTATE:
	{
		RotateBrushOp op(_data);
		op.run(false);
		break;
	}
	case VbsDef::BRUSH_DRAW:
	{
		DrawBrushOp op(_data);
		op.run();
		break;
	}
	case VbsDef::BRUSH_SMOOTH:
	{
		SmoothBrushOp op(_data);
		op.run();
		break;
	}
	case VbsDef::BRUSH_GRAB:
	{
		GrabBrushOp op(_data);
		op.run(false);
		break;
	}
	case VbsDef::BRUSH_THUMB:
	{
		ThumbBrushOp op(_data);
		op.run(false);
		break;
	}
	case VbsDef::BRUSH_PINCH:
	{
		PinchBrushOp op(_data);
		op.run();
		break;
	}
	}
}

void SculptStroke::calc_symm_data(size_t symm)
{
#if 0
	Vector3f basis_org = _data->object->center();
#else
	Vector3f basis_org =  Vector3f(0.0f, 0.0f, 0.0f);
#endif
	_data->symn_data.cur_pos = MathUtil::flipPoint3D(_data->cur_pos, basis_org, symm);
	_data->symn_data.first_hit_pos = MathUtil::flipPoint3D(_data->first_hit_pos, basis_org, symm);
	_data->symn_data.view_dir = MathUtil::flipVector3D(_data->view_dir, basis_org, symm);
	_data->symn_data.grab_delta = MathUtil::flipVector3D(_data->grab_delta, basis_org, symm);
}

void SculptStroke::push_undo_redo()
{
	BLI_assert(_data->undo_redo_logger);

	if (_step_logger){
		if (_data->undo_redo_logger){
			/*use std::move as we don't need data anymore. just move to SculptCommand for the sake of performace*/
			_data->undo_redo_logger->log_data(std::move(*_step_logger)); 
			delete _step_logger;
			_step_logger = nullptr;
		}
	}
	else{
		/*collect all BVH leaf nodes marked CHANGED*/
		std::vector<BMLeafNode*> nodes;
		if (_data->bvh->leaf_node_flagged_collect(LEAF_ORIGIN_DATA_SAVED, nodes)){
			if (_data->undo_redo_logger){
				_data->undo_redo_logger->log_data(nodes);
			}
		}
	}
}

void SculptStroke::save_origin_node_data(const std::vector<BMLeafNode*> &nodes)
{
	tbb::parallel_for(static_cast<size_t>(0), nodes.size(),
		[&](size_t i){
		BMLeafNode *node = nodes[i];
		node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);
		_data->bvh->leaf_node_org_data_save(node);
	});
}


void SculptStroke::update_topology()
{
	if (is_dynamic_topology())
	{
		BMSplitCollapseOp op(_data);
		op.run();
	}
}

bool SculptStroke::is_dynamic_topology()
{
	if (_data->brush_type == VbsDef::BRUSH_GRAB ||
		_data->brush_type == VbsDef::BRUSH_THUMB ||
		_data->brush_type == VbsDef::BRUSH_SMOOTH){
		return false;
	}
	else{
		return _data->flag & DYNAMIC_TOPOLOGY;
	}
}

