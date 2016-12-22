#ifndef VBVH_ITERATOR_H
#define VBVH_ITERATOR_H
#include "VBvhDefine.h"
#include <boost/container/small_vector.hpp>
#include <boost/container/static_vector.hpp>
#include <Eigen/Dense>
#include "common/math/bbox.h"
#include "VBvhUtil.h"

using namespace VBvh;
using namespace Eigen;

extern std::vector<Point3Dd> g_vscene_testSegments;

VBVH_BEGIN_NAMESPACE

template<class LeafType>
class VBvhRayIterator
{
private:
	static const size_t STACK_SIZE = 1 + 3 * MAX_DEPTH;
	
	typedef std::pair<BaseNode*, float> IterNode;
	typedef boost::container::static_vector<IterNode, STACK_SIZE>  IterStack;
	typedef boost::container::static_vector<IterNode, 4>		    HitNodes;
public:
	VBvhRayIterator(BaseNode* root, Ray *ray)
		:
		_root(root),
		_ray(ray),
		_curleaf(nullptr)
	{
		_stack.push_back(IterNode(dynamic_cast<BaseNode*>(_root), std::numeric_limits<float>::lowest()));
		step();
	}

	~VBvhRayIterator()
	{}

	void operator++()
	{
		step();
	}

	LeafType* operator*()
	{
		return _curleaf;
	}

	operator bool()
	{
		return _curleaf != nullptr;
	}
private:
	void step()
	{
		_curleaf = nullptr;

		while (!_stack.empty()){
			IterNode cur = _stack.back();
			_stack.pop_back();

			if (cur.second > _ray->tfar) 
				continue;

			if (cur.first->isLeafNode()){
				_curleaf = static_cast<LeafType*>(cur.first);
				break;
			}
			else{
				BaseNode *node = cur.first;
				HitNodes hits;
				for (size_t ni = 0; ni < 4; ni++){
					BaseNode *child = node->child(ni);
					if (child){
						const BBox3fa &bb = child->bounds();
						Vector3f lower(bb.lower.x, bb.lower.y, bb.lower.z);
						Vector3f upper(bb.upper.x, bb.upper.y, bb.upper.z);
						float dst;
						if (isectRayBB(_ray, lower, upper, dst)){
							hits.push_back(IterNode(child, dst));
						}
					}
				}

				if (hits.size() > 0){
					sort(hits);
					_stack.insert(_stack.end(), hits.begin(), hits.end());
				}
			}
		}
	}

	void sort(HitNodes &nodes)
	{
		const size_t tot = nodes.size();
		if (tot == 2){
			if (nodes[0].second < nodes[1].second)
				std::swap(nodes[0], nodes[1]);
		}

		if (tot == 3){
			if (nodes[0].second < nodes[1].second)
				std::swap(nodes[0], nodes[1]);
			if (nodes[0].second < nodes[2].second)
				std::swap(nodes[0], nodes[2]);
			if (nodes[1].second < nodes[2].second)
				std::swap(nodes[1], nodes[2]);
		}
		
		if (tot == 4){
			std::sort(nodes.begin(), nodes.end(),
				[&](IterNode &a, IterNode &b){return a.second > b.second;});
		}
	}
private:
	BaseNode *_root;
	Ray		  *_ray;
	IterStack  _stack;
	LeafType  *_curleaf;
};


template<class LeafType>
class VBvhIterator
{
public:
	static const size_t STACK_SIZE = 1 + 3 * MAX_DEPTH;
	typedef std::function<bool(BaseNode*)> IsectFunc;
private:
	typedef boost::container::static_vector<BaseNode*, STACK_SIZE> IterStack;
public:
	VBvhIterator(BaseNode *root, const IsectFunc &func)
		:
		_root(root),
		_isect(func)
	{
		//g_vscene_testSegments.clear();

		_stack.push_back(_root);
		step();
	}

	~VBvhIterator(){}

	void operator++()
	{
		step();
	}
	
	LeafType* operator*()
	{
		return _curleaf;
	}

	operator bool()
	{
		return _curleaf != nullptr;
	}
private:
	void step() 
	{
		_curleaf = nullptr;
		while (!_stack.empty()){
			BaseNode *cur = _stack.back();
			_stack.pop_back();

			//debugOutputNodeBB(cur, g_vscene_testSegments);

			if (cur->isLeafNode()){
				_curleaf = static_cast<LeafType*>(cur);
				break;
			}
			else{
				for (size_t ni = 0; ni < 4; ni++){
					BaseNode *child = cur->child(ni);
					if (child && _isect(child)){
						_stack.push_back(child);
					}
				}
			}
		}
	}
private:
	BaseNode *_root;
	const IsectFunc  &_isect;
	IterStack  _stack;
	LeafType  *_curleaf;
};
VBVH_END_NAMESPACE

#endif