#ifndef SCULPT_NUDGET_BRUSH_OP_H
#define SCULPT_NUDGET_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include <Eigen/Dense>
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "VBvh/BMBvhIsect.h"
#include "BaseLib/MathUtil.h"

class NudgetBrushOp
{
public:
	NudgetBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~NudgetBrushOp(){}

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
		Vector3f inter;
		float ri, x, b;
		float bstrength = _sdata->brush_strength;
		bstrength = std::abs(bstrength);

		for (size_t i = range.begin(); i != range.end(); ++i){
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(LEAF_UPDATE_STEP_DRAW_BUFFER | LEAF_UPDATE_STEP_BB);

			const BMVertVector &verts = node->verts();
			const size_t num = verts.size();
	
			for (size_t iv = 0; iv < num; ++iv){
				BMVert* vertex = verts[iv];
				Vector3f& coord = vertex->co;
				if (SUtil::isVertexInsideBrush(center, sqrRadius, coord)){
					ri = (coord - center).norm();
					x = ri * invRadius;;

					b = _sdata->deform_curve->evaluate(x);

					b *= 0.5 * bstrength;
					coord = coord + b * _tangent;
				}
			}

		}
	}

	void init()
	{
		const float radius = _sdata->world_radius;
		const float sqrRadius = _sdata->world_radius * _sdata->world_radius;
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
		const Vector3f avgNormal = avgOp.avgNormal();
		const Vector3f grab = (_sdata->symn_data.grab_delta);
		const Vector3f binorm = avgNormal.cross(grab);
		_tangent = binorm.cross(avgNormal);
	}

public:
	StrokeData *_sdata;
	std::vector<BMLeafNode*> _nodes;

	Vector3f _tangent;

};
#endif