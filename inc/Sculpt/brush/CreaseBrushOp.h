#ifndef SCULPT_CREASE_BRUSH_OP_H
#define SCULPT_CREASE_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "VBvh//BMBvhIsect.h"
#include "BaseLib/MathUtil.h"
#include <Eigen/Dense>

class CreaseBrushOp
{
public:
	CreaseBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~CreaseBrushOp(){}

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
		double ri, x, fade;
		Vector3f inter;

		const float sqrRadius = _sdata->world_radius * _sdata->world_radius;
		const float invRadius = 1.0f / _sdata->world_radius;
		const float pinchFactor = _sdata->pinch_factor * _sdata->pinch_factor;
		const Vector3f center = (_sdata->symn_data.cur_pos);
		const Vector3f offset = -_sdata->brush_strength * _sdata->scale * _sdata->world_radius * (_avgNormal);
		Vector3f disp, curOff;

		for (size_t i = range.begin(); i != range.end(); ++i){
	
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);

			const BMVertVector &verts = node->verts();
			const size_t num = verts.size();

			float creaseCorrection = pinchFactor * pinchFactor;
			/* we always want crease to pinch or blob to relax even when draw is negative */
			//creaseCorrection = creaseCorrection * _data->_bstrength;

			for (size_t i = 0; i < num; i++){
				BMVert* vertex = verts[i];
				Vector3f& coord = vertex->co;

				if (SUtil::isVertexInsideBrush(center, sqrRadius, coord)){
					/*toward center direction*/
					disp = center - coord;

					ri = disp.norm();
					x = (invRadius * ri);
					fade = _sdata->deform_curve->evaluate(x);

					//first we pinch
					disp = creaseCorrection * fade * disp;

					//then we draw
					curOff = fade * offset;

					coord = coord + curOff + disp;
				}
			}
		}
	}

	void init()
	{
		const float radius = _sdata->world_radius;
		const float sqrRadius = radius * radius;
		const Vector3f pos = (_sdata->symn_data.cur_pos);

		isect_sphere_bm_bvh(_sdata->bvh, pos, radius, _nodes);

		/*calculate deform data*/
		VNormalFaceVertexUpdateOp faceNormalOp(_nodes);
		faceNormalOp.run();
		VNormalVertexUpdateOp vertNormOp(_nodes, pos, sqrRadius);
		vertNormOp.run();

		Vector3f vdir = (_sdata->symn_data.view_dir);
		VAverageBrushDataOp avgOp(_nodes, pos, sqrRadius, vdir);
		avgOp.run();
		_avgNormal = avgOp.avgNormal();
	}

public:
	StrokeData *_sdata;
	std::vector<BMLeafNode*> _nodes;
	Vector3f _avgNormal;
};
#endif