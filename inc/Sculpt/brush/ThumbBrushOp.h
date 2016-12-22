#ifndef SCULPT_THUMB_BRUSH_OP_H
#define SCULPT_THUMB_BRUSH_OP_H
#include <Eigen/Dense>
#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"

using namespace Eigen;

class ThumbBrushOp
{
public:
	ThumbBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~ThumbBrushOp(){}

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
		BezierCurve *curve = _sdata->deform_curve;
		const float sqrRadius = _sdata->world_radius * _sdata->world_radius;
		const float invRadius = 1.0f / (float)_sdata->world_radius;
		const Vector3f center = (_sdata->symn_data.cur_pos);
		float bstrength = (float)_sdata->brush_strength;
		float ri, x, b;

		Vector3f displacement = (_sdata->symn_data.grab_delta);
		Vector3f tmp = _avgNormal.cross(displacement);
		displacement = tmp.cross(_avgNormal);

		for (size_t i = range.begin(); i != range.end(); ++i){

			BMLeafNode *node = _nodes[i];

			BMVertBackup *org_verts = node->originData()->_verts;
			const size_t        num = node->originData()->_totverts;

			for (size_t i = 0; i < num; i++)
			{
				BMVert* vert = org_verts[i].v;
				Vector3f& orgCoord = org_verts[i].co;
				Vector3f relativePos = (orgCoord - center);
				if (relativePos.squaredNorm() < sqrRadius)
				{
					ri = relativePos.norm();
					x = ri * invRadius;
					b = (float)curve->evaluate(x);
					vert->co = orgCoord + bstrength * b* displacement;
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

		Vector3f viewdir = (_sdata->symn_data.view_dir);
		VAverageBrushDataOp avgOp(_nodes, pos, sqrRadius, viewdir);
		avgOp.run();
		_avgNormal = avgOp.avgNormal();
	}

public:
	StrokeData *_sdata;
	std::vector<BMLeafNode*> _nodes;
	Vector3f _avgNormal;
};
#endif