#ifndef SCULPT_PINCH_BRUSH_OP_H
#define SCULPT_PINCH_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "VBvh/BMBvhIsect.h"
#include "BaseLib/MathUtil.h"
class PinchBrushOp
{
public:
	PinchBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~PinchBrushOp(){}

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
		float ri, x, fade;
		Vector3f inter;

		const float sqrRadius = _sdata->world_radius * _sdata->world_radius;
		const float invRadius = 1.0 / _sdata->world_radius;
		const Vector3f pos = (_sdata->symn_data.cur_pos);
		Vector3f disp;

		float bstrength = 0.5 * std::abs(_sdata->brush_strength); /*too strong, reduce strength a bit*/

		for (size_t i = range.begin(); i != range.end(); ++i){
			
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);

			const BMVertVector &verts = node->verts();
			const size_t num = verts.size();

			for (size_t i = 0; i < num; i++){
				BMVert* vertex = verts[i];
				Vector3f &coord = vertex->co;
				if (SUtil::isVertexInsideBrush(pos, sqrRadius, coord))
				{
					disp = pos - coord;
					ri   = disp.norm();
					x    = (invRadius * ri);
					fade = _sdata->deform_curve->evaluate(x);

					disp = bstrength * fade * disp;
					coord = coord + disp;
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