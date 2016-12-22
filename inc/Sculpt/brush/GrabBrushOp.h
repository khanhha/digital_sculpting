#ifndef SCULPT_GRAB_BRUSH_OP_H
#define SCULPT_GRAB_BRUSH_OP_H

#include "sculpt/commonDefine.h"
#include "sculpt/SUtil.h"
#include "tbb/parallel_for.h"
#include "tbb/blocked_range.h"
#include "sculpt/NormalUpdateOp.h"
#include "sculpt/AverageBrushDataOp.h"
#include "Vbvh/BMeshBvh.h"

class GrabBrushOp
{
public:
	GrabBrushOp(StrokeData *sdata)
		:_sdata(sdata)
	{
	}

	~GrabBrushOp(){}

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
		const float sqrradius = _sdata->world_radius * _sdata->world_radius;
		const float invRadius = 1.0f / _sdata->world_radius;
		const Vector3f center = (_sdata->symn_data.cur_pos);
		Vector3f acentric_dir;
		const	Vector3f grabDelta = (_sdata->symn_data.grab_delta);
		float	bstrength = _sdata->brush_strength;
		float	ri, x, b;

		for (size_t i = range.begin(); i != range.end(); ++i){
			BMLeafNode *node = _nodes[i];
			node->setAppFlagBit(LEAF_UPDATE_STEP_DRAW_BUFFER | LEAF_UPDATE_STEP_BB);
			BLI_assert(node->originData() != nullptr);

			BMVertBackup *org_verts = node->originData()->_verts;
			const size_t num = node->originData()->_totverts;

			for (size_t i = 0; i < num; i++){

				BMVert *vert = org_verts[i].v;
				Vector3f &org_co = org_verts[i].co;

				acentric_dir = org_co - center;
				float sqrdist = acentric_dir.squaredNorm();
				
				if (sqrdist <= sqrradius){
					ri = std::sqrt(sqrdist);
					x = ri * invRadius;
					b = (float)_sdata->deform_curve->evaluate(x);

					//Vector3f tmp = vert->co;
					vert->co = org_co + bstrength  * b * grabDelta;

					//////this vertex has been deformed before by one of its symmetric brushes
					//if (vert->getPosition() == _stepID)
					//{
					//	Point3Dd org(orgCoord.x(), orgCoord.y(), orgCoord.z());
					//	vert->_coord = vert->_coord + tmp - org;
					//}
				}
			}
		}
	}

	void init()
	{
		const Vector3f &pos = (_sdata->symn_data.cur_pos);
		const float radius = (float)_sdata->world_radius;
		isect_sphere_bm_bvh(_sdata->bvh, pos, radius, _nodes);
	}

public:
	StrokeData *_sdata;
	std::vector<BMLeafNode*> _nodes;
};
#endif