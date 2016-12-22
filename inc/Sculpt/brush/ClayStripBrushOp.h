#ifndef SCULPT_CLAY_STRIP_BRUSH_OP_H
#define SCULPT_CLAY_STRIP_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include <Eigen/Dense>
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "VBvh/BMBvhIsect.h"
#include "BaseLib/MathUtil.h"
#include <boost/math/constants/constants.hpp>

class ClayStripBrushOp
{
public:
	ClayStripBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
		//_side = M_SQRT1_2;
		_side = boost::math::constants::half_root_two<float>();

		_pow = 4.0f;
		_invPow = 1.0f / powf(_side, _pow);
	}

	~ClayStripBrushOp(){}

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
		float distance, fade;
		float coo[3];
		Vector3f proj;
		for (size_t i = range.begin(); i != range.end(); ++i){
			
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(LEAF_UPDATE_STEP_BB | LEAF_UPDATE_STEP_DRAW_BUFFER);

			const BMVertVector &verts = node->verts();
			const size_t num = verts.size();

			for (size_t iv = 0; iv < num; ++iv)
			{
				BMVert* vertex = verts[iv];
				Vector3f& coord = vertex->co;

				coo[0] = coord(0); 
				coo[1] = coord(1); 
				coo[2] = coord(2);

				if (insideCube(coo, distance))
				{
					if (SUtil::planePointSideFlip(coord, _deformCenter, _normal, _flip))
					{
						SUtil::projectPointPlane(_deformCenter, _normal, coord, proj);
						proj = proj - coord;
						fade = _sdata->deform_curve->evaluate(distance);
						coord = coord + _bstrength * fade * proj;
					}
				}
			}
		}
	}

	void init()
	{
		const Vector3f pos = (_sdata->symn_data.cur_pos);
		float radius = (float)_sdata->world_radius;
		const float sqrRadius = radius * radius;

		isect_sphere_bm_bvh(_sdata->bvh, pos, radius, _nodes);

		/*calculate deform data*/
		VNormalFaceVertexUpdateOp faceNormalOp(_nodes);
		faceNormalOp.run();
		VNormalVertexUpdateOp vertNormOp(_nodes, pos, sqrRadius);
		vertNormOp.run();

		Vector3f vdir = (_sdata->symn_data.view_dir);
		VAverageBrushDataOp avgOp(_nodes, pos, sqrRadius, vdir);
		avgOp.run();
		const Vector3f avgCenter = avgOp.avgCenter();
		const Vector3f norm = avgOp.avgNormal();

		_bstrength = _sdata->brush_strength;
		_flip = _bstrength < 0;
		if (_flip) {
			_bstrength = -_bstrength;
			radius = -radius;
		}

		_deformCenter = avgCenter + 0.05 * radius * norm;
		_normal = norm;

		Vector3f grab = (_sdata->symn_data.grab_delta); grab.normalize();
		Vector3f right = norm.cross(grab);  right.normalize();
		Vector3f forward = right.cross(norm); forward.normalize();

		float mat[4][4], smat[4][4];
		_transform[0][0] = right(0);	_transform[0][1] = right(1);	_transform[0][2] = right(2);		_transform[0][3] = 0;
		_transform[1][0] = forward(0);	_transform[1][1] = forward(1);	_transform[1][2] = forward(2);	_transform[1][3] = 0;
		_transform[2][0] = norm(0);		_transform[2][1] = norm(1);		_transform[2][2] = norm(2);		_transform[2][3] = 0;
		_transform[3][0] = pos(0);		_transform[3][1] = pos(1);		_transform[3][2] = pos(2);		_transform[3][3] = 1.0;
		//_transform[3][0] = avgCenter.x; _transform[3][1] = avgCenter.y; _transform[3][2] = avgCenter.z; _transform[3][3] = 1.0;

		float scale = 0.9;
		SUtil::scale_m4_fl(smat, scale * radius);
		SUtil::mul_m4_m4m4(mat, _transform, smat);
		SUtil::invert_m4_m4(_transform, mat);
	}


	bool insideCube(float coord[3], float& distance) const
	{
		float local[3];
		SUtil::mul_v3_m4v3(local, _transform, coord);

		local[0] = std::fabsf(local[0]);
		local[1] = std::fabsf(local[1]);
		local[2] = std::fabsf(local[2]);

		if (local[0] <= _side && local[1] <= _side && local[1] <= _side)
		{
			distance = _invPow * (powf(local[0], _pow) + powf(local[1], _pow) + powf(local[2], _pow));
			return 1;
		}
		else
		{
			return 0;
		}
	}

public:
	StrokeData *_sdata;
	std::vector<BMLeafNode*> _nodes;
	
	Vector3f _deformCenter;
	Vector3f _normal;
	int      _flip;
	float    _bstrength;

	float _side;
	float _pow;
	float _invPow;
	float _transform[4][4];
};
#endif