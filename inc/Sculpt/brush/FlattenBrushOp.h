#ifndef SCULPT_FLATTEN_BRUSH_OP_H
#define SCULPT_FLATTEN_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "VBvh/BMBvhIsect.h"
#include "BaseLib/MathUtil.h"

class FlattenBrushOp
{
public:
	FlattenBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~FlattenBrushOp(){}

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
		const Vector3f pos = (_sdata->symn_data.cur_pos);
		Vector3f proj;
		const float bstrength = _sdata->brush_strength; 
		float ri, x, fade;

		for (size_t i = range.begin(); i != range.end(); ++i){
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);

			const BMVertVector &verts = node->verts();
			const size_t num = verts.size();

			for (size_t i = 0; i < num; i++)
			{
				Vector3f& coord = verts[i]->co;
				if (SUtil::isVertexInsideBrush(pos, sqrRadius, coord))
				{
					SUtil::projectPointPlane(_flattenCenter, _normal, coord, proj);

					ri = (pos - coord).norm();
					x = (invRadius * ri);
					fade = _sdata->deform_curve->evaluate(x);

					proj = bstrength * fade * (proj - coord);
					coord = coord + proj;
				}
			}
		}
	}

	void init()
	{
		const float sqrRadius = _sdata->world_radius * _sdata->world_radius;
		const float radius = _sdata->world_radius;
		const Vector3f pos = (_sdata->symn_data.cur_pos);

		isect_sphere_bm_bvh(_sdata->bvh, pos, radius, _nodes);

		/*calculate deform data*/
		VNormalFaceVertexUpdateOp faceNormalOp(_nodes);
		faceNormalOp.run();
		VNormalVertexUpdateOp vertNormOp(_nodes, pos, sqrRadius);
		vertNormOp.run();

		/*calculate average center and normal*/
		Vector3f vdir = (_sdata->symn_data.view_dir);
		VAverageBrushDataOp avgOp(_nodes, pos, sqrRadius, vdir);
		avgOp.run();
		_flattenCenter = avgOp.avgCenter();
		_normal        = avgOp.avgNormal();
	}
public:
	StrokeData *_sdata;
	std::vector<BMLeafNode*> _nodes;

	Vector3f _flattenCenter;
	Vector3f _normal;
};
#endif