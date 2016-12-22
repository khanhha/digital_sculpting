#ifndef SCULPT_SMOOTH_BRUSH_OP_H
#define SCULPT_SMOOTH_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "VBvh/BMBvhIsect.h"
#include "BaseLib/MathUtil.h"

class SmoothBrushOp
{
public:
	SmoothBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~SmoothBrushOp(){}

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
		const float sqrRadius = _sdata->world_radius * _sdata->world_radius;
		const float invRadius = 1.0 / _sdata->world_radius;
		const Vector3f center = (_sdata->symn_data.cur_pos);
		const float bstrength = _sdata->brush_strength;
		Vector3f avg;
		float b, x, ri;

		for (size_t i = range.begin(); i != range.end(); ++i){
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(LEAF_UPDATE_STEP_DRAW_BUFFER | LEAF_UPDATE_STEP_BB);

			const BMVertVector &verts = node->verts();
			const size_t num = verts.size();

			for (size_t iv = 0; iv < num; ++iv)
			{
				BMVert* vertex = verts[iv];
				Vector3f& coord = vertex->co;
				if (SUtil::isVertexInsideBrush(center, sqrRadius, coord))
				{
					ri = (coord - center).norm();
					x = invRadius * ri;
					b = _sdata->deform_curve->evaluate(x);
					BM_vert_calc_mean(vertex, avg);
					avg = avg - coord;
					coord = coord + bstrength * b * avg;
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