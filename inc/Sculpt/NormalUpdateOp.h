#ifndef NORMAL_UPDATE_OP_H
#define NORMAL_UPDATE_OP_H
#include <vector>
#include "tbb/parallel_for.h"

class VNormalVertexUpdateOp
{
public:
	VNormalVertexUpdateOp(const std::vector<BMLeafNode*> &nodes, const Vector3f& center, float sqrRadius)
		:
		_nodes(nodes),
		_center(center),
		_sqrRadius(sqrRadius)
	{
	}
	~VNormalVertexUpdateOp(){}

	void run(bool threaded = true)
	{
		if (threaded) {
			tbb::parallel_for(tbb::blocked_range<size_t>(static_cast<size_t>(0), _nodes.size()), *this);
		}
		else {
			(*this)(tbb::blocked_range<size_t>(0, _nodes.size()));
		}
	}

	void operator()(const tbb::blocked_range<size_t>& range) const
	{
		for (size_t i = range.begin(); i != range.end(); ++i){
			const BMVertVector &verts = _nodes[i]->verts();
			const size_t numvert = verts.size();
			for (size_t v = 0; v < numvert; ++v){
				if ((verts[v]->co - _center).squaredNorm() <= _sqrRadius){
					BM_vert_normal_update_face(verts[v]);
				}
			}
		}
	}
private:
	const std::vector<BMLeafNode*> &_nodes;
	const Vector3f _center;
	const float    _sqrRadius;
};

class VNormalFaceVertexUpdateOp
{
public:
	VNormalFaceVertexUpdateOp(const std::vector<BMLeafNode*> &nodes)
		:_nodes(nodes)
	{}

	~VNormalFaceVertexUpdateOp(){}

	void run(bool threaded = true)
	{
		if (threaded) {
			tbb::parallel_for(tbb::blocked_range<size_t>(static_cast<size_t>(0), _nodes.size()), *this);
		}
		else {
			(*this)(tbb::blocked_range<size_t>(0, _nodes.size()));
		}
	}

	void operator()(const tbb::blocked_range<size_t>& range) const
	{
		for (size_t i = range.begin(); i != range.end(); ++i){
			const BMFaceVector &faces = _nodes[i]->faces();
			const size_t numface =  faces.size();
			for (size_t j = 0; j < numface; ++j){
				BM_face_normal_update(faces[j]);
			}
		}
	}
private:
	const std::vector<BMLeafNode*> &_nodes;
};
#endif