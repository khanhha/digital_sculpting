#ifndef SCULPT_AVERAGE_BRUSH_DATA_OP_H
#define SCULPT_AVERAGE_BRUSH_DATA_OP_H
#include <vector>
#include "tbb/parallel_reduce.h"
#include "tbb/blocked_range.h"
class VAverageBrushDataOp
{
public:

	VAverageBrushDataOp(std::vector<BMLeafNode*> &nodes, const Vector3f& center, const float& sqrRad, const Vector3f& viewDir)
		:
		_nodes(nodes),
		_sphereCenter(center),
		_sqrRadius(sqrRad),
		_viewDir(viewDir)
	{
		_areaNorm.setZero();
		_center.setZero();
		_cnt = 0;

		_areaNormFlip.setZero();
		_centerFlip.setZero();
		_cntFlip = 0;
	}

	VAverageBrushDataOp(VAverageBrushDataOp &rhs, tbb::split)
		:
		_nodes(rhs._nodes),
		_sphereCenter(rhs._sphereCenter),
		_sqrRadius(rhs._sqrRadius),
		_viewDir(rhs._viewDir)
	{
		_areaNorm.setZero();
		_center.setZero();
		_cnt = 0;

		_areaNormFlip.setZero();
		_centerFlip.setZero();
		_cntFlip = 0;
	}

	Vector3f avgNormal(){
		Vector3f normal;
		if (_cnt > 0){
			normal = (1.0 / _cnt) * _areaNorm;
		}
		else if (_cntFlip > 0){
			normal = (1.0 / _cntFlip) * _areaNormFlip;
		}
		else{
			normal = _viewDir;
		}

		normal.normalize();
		return normal;
	}

	Vector3f avgCenter(){
		Vector3f center;
		if (_cnt > 0){
			center = (1.0f / _cnt) * _center;
		}
		else if (_cntFlip > 0){
			center = (1.0f / _cntFlip) * _centerFlip;
		}
		else{
			center = _sphereCenter;
		}
		return center;
	}

	void run(bool threaded = true)
	{
		if (threaded) {
			tbb::parallel_reduce(tbb::blocked_range<size_t>(0, _nodes.size()), *this);
		}
		else {
			(*this)(tbb::blocked_range<size_t>(0, _nodes.size()));
		}
	}

	void operator()(const tbb::blocked_range<size_t>& range)
	{

		for (size_t i = range.begin(); i != range.end(); ++i){
			BMLeafNode *node = _nodes[i];
			const BMVertVector &verts = node->verts();
			const size_t numVerts = verts.size();
			for (size_t i = 0; i < numVerts; i++)
			{
				BMVert* vert = verts[i];
				if ((vert->co - _sphereCenter).squaredNorm() <= _sqrRadius){
					const Vector3f& norm = vert->no;
					if (norm.dot(_viewDir) > 0)
					{
						_center += vert->co;
						_areaNorm += norm;
						_cnt++;
					}
					else
					{
						_centerFlip += vert->co;
						_areaNormFlip += norm;
						_cntFlip++;
					}
				}
			}
		}
	}

	void join(const VAverageBrushDataOp& rhs)
	{
		_areaNorm += rhs._areaNorm;
		_center += rhs._center;
		_cnt += rhs._cnt;

		_areaNormFlip += rhs._areaNormFlip;
		_centerFlip += rhs._centerFlip;
		_cntFlip += rhs._cntFlip;
	}

private:
	std::vector<BMLeafNode*> &_nodes;
	const Vector3f			 &_sphereCenter;
	const float			     &_sqrRadius;
	const Vector3f &_viewDir;

	/*calculation data*/
	size_t   _cnt;
	Vector3f _areaNorm;
	Vector3f _center;

	size_t   _cntFlip;
	Vector3f _areaNormFlip;
	Vector3f _centerFlip;
};

#endif