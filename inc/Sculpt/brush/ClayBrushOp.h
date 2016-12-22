#ifndef SCULPT_CLAY_BRUSH_OP_H
#define SCULPT_CLAY_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "VBvh/BMBvhIsect.h"
#include "BaseLib/MathUtil.h"
#include <Eigen/Dense>

class ClayBrushOp
{
public:
	ClayBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~ClayBrushOp(){}

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
		float coo[3];
		float ri, x, fade;
		Vector3f inter;

		const float sqrRadius = _sdata->world_radius * _sdata->world_radius;
		const float invRadius = 1.0 / _sdata->world_radius;
		const Vector3f pos = (_sdata->symn_data.cur_pos);

		for (size_t i = range.begin(); i != range.end(); ++i){
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(VBvh::LEAF_UPDATE_STEP_BB | VBvh::LEAF_UPDATE_STEP_DRAW_BUFFER);

			const BMVertVector &verts = node->verts();
			const size_t num = verts.size();

			for (size_t iv = 0; iv < num; ++iv)
			{
				BMVert* vertex = verts[iv];
				Vector3f&  coord  = vertex->co;

				coo[0] = coord(0);
				coo[1] = coord(1);
				coo[2] = coord(2);

				if (SUtil::isVertexInsideBrush(pos, sqrRadius, coord))
				{
					if (SUtil::planePointSideFlip(coord, _deformCenter, _normal, _flip))
					{
						SUtil::projectPointPlane(_deformCenter, _normal, coord, inter);
						inter = inter - coord;

						ri = (pos - coord).norm();
						x = invRadius * ri;
						fade = _sdata->deform_curve->evaluate((float)x);

						coord = coord + _bstrength * fade * inter;
					}
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

		/*calculate average center and normal*/
		Vector3f vdir = (_sdata->symn_data.view_dir);
		VAverageBrushDataOp avgOp(_nodes, pos, sqrRadius, vdir);
		avgOp.run();
		const Vector3f avgCenter = avgOp.avgCenter();
		const Vector3f norm = avgOp.avgNormal();

		_bstrength = _sdata->brush_strength;
		double deformRadius = _sdata->scale * _sdata->world_radius;

		_flip = _bstrength < 0;
		if (_flip) {
			_bstrength = -_bstrength;
			deformRadius = -deformRadius;
		}

		_deformCenter = avgCenter + deformRadius * norm;
		_normal = norm;
	}

public:
	StrokeData *_sdata;
	std::vector<BMLeafNode*> _nodes;

	Vector3f _deformCenter;
	Vector3f _pos;
	Vector3f _normal;
	bool     _flip;
	double   _bstrength;
};
#endif