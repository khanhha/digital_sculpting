#ifndef SCULPT_INFLATE_BRUSH_OP_H
#define SCULPT_INFLATE_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include <Eigen/Dense>
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "BaseLib/MathUtil.h"
#include "VBvh/BMBvhIsect.h"

class InflateBrushOp
{
public:
	InflateBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~InflateBrushOp(){}

	void run(bool threaded = false)
	{
		init();

		if (threaded)
			tbb::parallel_for(tbb::blocked_range<size_t>(0, _nodes.size()), *this);
		else
			(*this)(tbb::blocked_range<size_t>(0, _nodes.size()));
	}

	void operator()(const tbb::blocked_range<size_t>& range) const
	{
		float  ri, x, b;
		Point3Dd inter;

		const float sqrRadius = _sdata->world_radius * _sdata->world_radius;
		const float invRadius = (float)1.0 / _sdata->world_radius;
		const Vector3f center = (_sdata->symn_data.cur_pos);
		const float scaleDownn = 0.15f; /*reduce self intersection*/
		float maxDisp = (float)(scaleDownn * _sdata->scale * _sdata->brush_strength * _sdata->world_radius);

		for (size_t i = range.begin(); i != range.end(); ++i){
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(VBvh::LEAF_UPDATE_STEP_BB | VBvh::LEAF_UPDATE_STEP_DRAW_BUFFER);

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
					coord = coord + b * maxDisp *  vertex->no;

				}
			}
		}
	}

	void init()
	{
		const float radius = (float)_sdata->world_radius;
		const Vector3f pos = (_sdata->symn_data.cur_pos);
		isect_sphere_bm_bvh(_sdata->bvh, pos, radius, _nodes);

		/*calculate deform data*/
		VNormalFaceVertexUpdateOp faceNormalOp(_nodes);
		faceNormalOp.run();
		VNormalVertexUpdateOp vertNormOp(_nodes, pos, radius * radius);
		vertNormOp.run();
	}

public:
	StrokeData *_sdata;
	std::vector<BMLeafNode*> _nodes;
};
#endif