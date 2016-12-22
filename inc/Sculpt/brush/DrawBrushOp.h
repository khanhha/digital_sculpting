#ifndef SCULPT_DRAW_BRUSH_OP_H
#define SCULPT_DRAW_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include <Eigen/Dense>
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "VBvh/BMBvhIsect.h"
#include "BaseLib/MathUtil.h"

class DrawBrushOp
{
public:
	DrawBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}
	~DrawBrushOp(){}

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
		const float bstrength = _sdata->brush_strength;
		const float sqrRadius = _sdata->world_radius * _sdata->world_radius;
		const float invRadius = 1.0f / _sdata->world_radius;
		const Vector3f center = (_sdata->symn_data.cur_pos);
		const Vector3f offset = (_sdata->symn_data.view_dir) * _sdata->scale * _sdata->world_radius;
		float ri, x, b;

		for (size_t i = range.begin(); i != range.end(); ++i){
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);

			const BMVertVector &verts = node->verts();
			const size_t num = verts.size();
			for (size_t iv = 0; iv < num; ++iv)
			{
				BMVert* vertex = verts[iv];
				Vector3f& coord = vertex->co;
				if (SUtil::isVertexInsideBrush(center, sqrRadius, coord))
				{
					
					ri = (center - coord).norm();
					x = invRadius * ri;
					b = _sdata->deform_curve->evaluate(x);
					coord = coord + bstrength * b * offset;
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