#ifndef VBVH_OVERLAP_H
#define VBVH_OVERLAP_H

#include "VBvh/VBvhDefine.h"
#include "VBvh/VBvhNode.h"
#include <stack>
#include <functional>

VBVH_BEGIN_NAMESPACE
template<class LeafType>
class VBvhOverlap
{
public:
	typedef std::function<void(LeafType* n0, LeafType* n1)> CallBack;
private:
	typedef std::pair<BaseNode*, BaseNode*> SElem;
	typedef std::stack<SElem> MyStack;
public:
	VBvhOverlap(BaseNode *root0, BaseNode *root1, CallBack callback)
		:
		_root0(root0), _root1(root1), _callback(callback){}
	
	~VBvhOverlap(){}

	void run()
	{
		markOverlap(_root0, _root1);
	}
private:

	void markOverlap(BaseNode *n0, BaseNode *n1)
	{
		MyStack stack;
		BaseNode *a = n0;
		BaseNode *b = n1;

		while (1){

			assert(a && b);

			if (conjoint(a->bounds(), b->bounds())){

				if (a->isLeafNode() && b->isLeafNode()){
					/*mark overlap*/
					_callback(static_cast<LeafType*>(a), static_cast<LeafType*>(b));
				}
				else{
					if (!a->isLeafNode()){
						/*swap = true: ensure that the first node of stack element belongs to BVH A*/
						a = descend(a, b, stack, true);
						continue;
					}
					else{
						b = descend(b, a, stack, false);
						continue;
					}
				}
			}

			if (stack.empty()) break;

			SElem elm = stack.top(); stack.pop();
			a = elm.first;
			b = elm.second;
		}
	}
	
	/*
	*return the first child child of n0, and push other children with n1 into stack
	*/
	BaseNode*	descend(BaseNode *n0, BaseNode *n1, MyStack &stack, bool swap)
	{
		BaseNode *tmp;
		BaseNode *next_child = nullptr;
		for (size_t i = 0; i < 4; ++i){
			tmp = n0->child(i);
			if (tmp){
				if (!next_child)
					next_child = tmp;
				else
					swap ? stack.push(SElem(tmp, n1)) : stack.push(SElem(n1, tmp));
			}
		}

		return next_child;
	}
private:
	BaseNode *_root0;
	BaseNode *_root1;
	CallBack _callback;
};
VBVH_END_NAMESPACE
#endif