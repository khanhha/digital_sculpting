#ifndef BMESH_BMESH_SIMPLIFY_H
#define BMESH_BMESH_SIMPLIFY_H

#include "BMesh/BMesh.h"
#include "BaseLib/VQuadric.h"
#include <boost/heap/heap_concepts.hpp>
#include <boost/heap/fibonacci_heap.hpp>
using namespace  boost::heap;
VM_BEGIN_NAMESPACE

#define BOUNDARY_PRESERVE_WEIGHT 100.0f
#define OPTIMIZE_EPS 0.01f  /* FLT_EPSILON is too small, see [#33106] */
#define COST_INVALID FLT_MAX


class BMeshDecimate
{
private:
	struct EHeapNode
	{

		EHeapNode(BMEdge *e_, float cost_): e(e_), cost(cost_){};

		bool operator<(EHeapNode const & rhs) const
		{
			return cost > rhs.cost;
		}
		BMEdge* e;
		float cost;
	};

private:
	typedef boost::heap::fibonacci_heap<EHeapNode> EHeap;
	typedef boost::heap::fibonacci_heap<EHeapNode>::handle_type EHeapHanle;
public:
	BMeshDecimate(BMesh *bm, float ratio = 0.5f): _bm(bm), _ratio(ratio){};
	~BMeshDecimate(){};
	void run();
	void setRatio(float rat);
private:


	void  bm_decim_build_quadrics();
	float bm_decim_build_edge_cost_single(BMEdge *e);
	void  bm_decim_build_edge_cost_single_heap(BMEdge *e, bool update = false);
	void  bm_decim_calc_target_co(BMEdge *e, Vector3f &optimize_co);
	void  bm_decim_build_edge_cost();
	bool  bm_decim_edge_collapse(BMEdge *e,Vector3f &optimize_co, bool optimize_co_calc);
	bool  bm_edge_collapse_is_degenerate_topology(BMEdge *e_first);
	bool  bm_edge_collapse(BMEdge *e_clear, BMVert *v_clear, int r_e_clear_other[2]);
	void  bm_decim_invalid_edge_cost_single(BMEdge *e);
private:
	BMesh *_bm;
	EHeap _eheap;
	float _ratio;
	std::vector<EHeapHanle>		_eheaphandles;
	std::vector<Qdr::Quadric>	_vquadrics;
};

VM_END_NAMESPACE
#endif