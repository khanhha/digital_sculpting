#ifndef SCULPT_SNAKE_HOOK_BRUSH_OP_H
#define SCULPT_SNAKE_HOOK_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"

class SnakeHookBrushOp
{
public:
	SnakeHookBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~SnakeHookBrushOp(){}

	void run(bool threaded = true)
	{
		init();

		if (threaded)
			tbb::parallel_for(tbb::blocked_range<size_t>(0, _nodes.size()), *this);
		else
			(*this)(tbb::blocked_range<size_t>(0, _nodes.size()));
	}

	void operator()(const tbb::blocked_range<size_t>& range) const
	{
		float ri, x, b;

		const float sqrRadius = _sdata->world_radius * _sdata->world_radius;
		const float invRadius = 1.0 / _sdata->world_radius;
		const Vector3f pos = (_sdata->symn_data.cur_pos);
		const Vector3f grabDelta = (_sdata->symn_data.grab_delta);
		BezierCurve *curve = _sdata->deform_curve;

		for (size_t i = range.begin(); i != range.end(); ++i){
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(VBvh::LEAF_UPDATE_STEP_BB | VBvh::LEAF_UPDATE_STEP_DRAW_BUFFER);

			const BMVertVector &verts = node->verts();
			const size_t num = verts.size();
			for (size_t i = 0; i < num; i++)
			{
				BMVert* vert = verts[i];
				Vector3f& coord = vert->co;
				if (SUtil::isVertexInsideBrush(pos, sqrRadius, coord))
				{
					ri = (coord - pos).norm();
					x = ri * invRadius;
					b = curve->evaluate((float)x);
					coord = coord + /*scale **/ b * grabDelta;
				}
			}
		}

	}

	void init()
	{
		const float radius = _sdata->world_radius;
		const Vector3f pos = (_sdata->symn_data.cur_pos);
		isect_sphere_bm_bvh(_sdata->bvh, pos, radius, _nodes);
	}

public:
	StrokeData *_sdata;
	std::vector<BMLeafNode*> _nodes;
};
#endif