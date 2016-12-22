#ifndef SCULPT_ROTATE_BRUSH_OP_H
#define SCULPT_ROTATE_BRUSH_OP_H
#include <cmath>
#include <Eigen/Dense>
#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "VBvh/BMeshBvh.h"

using namespace  VBvh;
class RotateBrushOp
{
public:
	RotateBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~RotateBrushOp(){}

	void run(bool threaded = true)
	{
		if (init()){

			if (threaded)
				tbb::parallel_for(tbb::blocked_range<size_t>(0, _nodes.size()), *this);
			else
				(*this)(tbb::blocked_range<size_t>(0, _nodes.size()));
		}
	}

	void operator()(const tbb::blocked_range<size_t>& range) const
	{
		const Vector3f rotationCenter = (_sdata->symn_data.first_hit_pos);
		const float radius = _sdata->world_radius;
		const float invRadius = 1.0f / radius;
		const float sqrWorldRadius	= radius * radius;
		float rotmatrix[3][3];
		float centerRotMatrix[3][3];
		float x, fade, angle;
		float sculptNorm[3], localCoord[3], rotatedCoord[3];
#if 0
		sculptNorm[0] = static_cast<float>(_sdata->_sym._viewDirection.x);
		sculptNorm[1] = static_cast<float>(_sdata->_sym._viewDirection.y);
		sculptNorm[2] = static_cast<float>(_sdata->_sym._viewDirection.z);
#else
		sculptNorm[0] = (_normal[0]);
		sculptNorm[1] = (_normal[1]);
		sculptNorm[2] = (_normal[2]);
#endif

		const float sqrCenterRadius = (0.3f * radius) * (0.3f * radius);
		fade = (float)_sdata->deform_curve->evaluate(0.3f);
		angle = fade * _maxAngle;
		SUtil::axis_angle_normalized_to_mat3_ex(centerRotMatrix, sculptNorm, std::sinf(angle), std::cosf(angle));

		for (size_t i = range.begin(); i != range.end(); ++i){

			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(LEAF_UPDATE_STEP_DRAW_BUFFER | LEAF_UPDATE_STEP_BB);

			const BMVertVector &verts = node->verts();
			size_t        num = verts.size();

			for (size_t i = 0; i < num; ++i){
				BMVert* vert = verts[i];
				Vector3f& coord = vert->co;

				/*relative coordinate to sphere center*/
				localCoord[0] = (coord[0] - rotationCenter[0]); 
				localCoord[1] = (coord[1] - rotationCenter[1]); 
				localCoord[2] = (coord[2] - rotationCenter[2]);

				if (SUtil::isVertexInsideBrush(rotationCenter, sqrCenterRadius, coord)){
					SUtil::mul_v3_m3v3(rotatedCoord, centerRotMatrix, localCoord);

					coord[0] = rotatedCoord[0] + rotationCenter[0];
					coord[1] = rotatedCoord[1] + rotationCenter[1];
					coord[2] = rotatedCoord[2] + rotationCenter[2];
				}
				else if (SUtil::isVertexInsideBrush(rotationCenter, sqrWorldRadius, coord)){
					x = (rotationCenter - coord).norm();
					x = x * invRadius;
					fade = _sdata->deform_curve->evaluate(x);
					angle = fade * _maxAngle;

					/*rotation matrix*/
					SUtil::axis_angle_normalized_to_mat3_ex(rotmatrix, sculptNorm, std::sinf(angle), std::cosf(angle));
					SUtil::mul_v3_m3v3(rotatedCoord, rotmatrix, localCoord);

					coord[0] = rotatedCoord[0] + rotationCenter[0];
					coord[1] = rotatedCoord[1] + rotationCenter[1];
					coord[2] = rotatedCoord[2] + rotationCenter[2];
				}
			}
		}
	}

	bool init()
	{
		if (_sdata->rotate_angle > FLT_EPSILON || _sdata->rotate_angle < -FLT_EPSILON){
			_maxAngle = _sdata->rotate_angle;

			const Vector3f first_pos = _sdata->symn_data.first_hit_pos;
			const Vector3f pos = _sdata->symn_data.cur_pos;
			const float radius = _sdata->world_radius;
			const float sqrRadius = radius * radius;

			isect_sphere_bm_bvh(_sdata->bvh, first_pos, radius, _nodes);

			VNormalFaceVertexUpdateOp faceNormalOp(_nodes);
			faceNormalOp.run();
			VNormalVertexUpdateOp vertNormOp(_nodes, pos, sqrRadius);
			vertNormOp.run();

			/*calculate average center and normal*/
			Vector3f viewdir = _sdata->symn_data.view_dir;
			VAverageBrushDataOp avgOp(_nodes, pos, sqrRadius, viewdir);
			avgOp.run();
			_normal = avgOp.avgNormal();

			return true;

		}
		else{
			return false;
		}
	}

public:
	StrokeData *_sdata;
	std::vector<BMLeafNode*> _nodes;
		
	float		_maxAngle;
	Vector3f	_normal;
};
#endif