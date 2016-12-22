#ifndef SCULPT_BM_SUBDIVISION_OP_H
#define SCULPT_BM_SUBDIVISION_OP_H
#include "BMesh\BMesh.h"
#include "sculpt\StrokeData.h"
#include <boost\heap\priority_queue.hpp>
#include <boost/heap/heap_concepts.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <queue>
#include "BaseLib/VQuadric.h"
using namespace VM;

class BMSplitCollapseOp
{
	typedef std::pair<BMVert*, BMVert*> SplitEdge;
	struct edge_compare_long
	{
	public:
		bool operator()(const SplitEdge &e0, const SplitEdge &e1)
		{
			return ((e0.first->co - e0.second->co).squaredNorm() < (e1.first->co - e1.second->co).squaredNorm());
		}
	};
	struct edge_compare_short
	{
	public:
		bool operator()(const SplitEdge &e0, const SplitEdge &e1)
		{
			return ((e0.first->co - e0.second->co).squaredNorm() > (e1.first->co - e1.second->co).squaredNorm());
		}
	};

	struct EdgeNode
	{
		EdgeNode(BMEdge *e_, float cost_) : v1(e_->v1), v2(e_->v2), cost(cost_){};
		
		bool operator<(EdgeNode const & rhs) const
		{
			return cost > rhs.cost;
		}

		BMVert *v1, *v2;
		float cost;
	};

private:
	typedef boost::heap::fibonacci_heap<EdgeNode> EdgeQueue;
	typedef boost::heap::fibonacci_heap<EdgeNode>::handle_type EdgeHandle;
	typedef boost::container::small_vector<BMFace*, 32>	FaceSmallBuffer;
public:
	typedef std::priority_queue<SplitEdge, std::vector<SplitEdge>, edge_compare_long>  LongEdgeQueue;
public:
	BMSplitCollapseOp(StrokeData *sdata);
	~BMSplitCollapseOp();
	void run();
private:
	void split_long_edges();
	void subdivide(EdgeQueue &equeue);
	void edge_split(EdgeQueue &equeue, BMEdge *e);
	void long_edge_queue_create(EdgeQueue &equeue);
	void long_edge_queue_face_add(EdgeQueue &equeue, BMFace *f);
	void long_edge_queue_edge_add_recur(EdgeQueue &queue, BMLoop *l_edge, BMLoop *l_end, float len_sq, float limit_len);
	void long_edge_queue_edge_add(EdgeQueue &equeue, BMEdge *e);
	
	/*collapse*/
	void collapse_short_edges();
	bool collapse_equeue(EdgeQueue &equeue
#ifdef OPTIMIZE_COLLAPSE 
		, std::vector<Qdr::Quadric> &vquadric
#endif
	);

#ifdef OPTIMIZE_COLLAPSE 
	void vert_quadric_compute(EdgeQueue &equeue, std::vector<Qdr::Quadric> &vquadric);
#endif

	void collapse_edge(BMEdge *e, FaceSmallBuffer &deleted_faces, const Vector3f &optimize_co);
	void short_edge_queue_create(EdgeQueue &equeue);
	void short_edge_queue_face_add(EdgeQueue &equeue, BMFace *f);
	void short_edge_queue_edge_add(EdgeQueue &equeue, BMEdge *e);
	void mark_tri_node_in_sphere_begin();
	void mark_tri_node_in_sphere_end();

	bool tri_in_sphere(BMFace *f);
	
	BMVert* bmesh_vert_create(const Vector3f &co, const Vector3f &no);
	BMFace* bvh_bmesh_face_create(BMLeafNode *node, BMVert *verts[3], BMEdge *edges[3]);
	void    bvh_bmesh_face_remove(BMLeafNode *node, BMFace *face);
	void	bvh_bmesh_vert_remove(BMVert *v);
	void	bm_edges_from_verts(BMVert *v_tri[3], BMEdge *e_tri[3]);
	bool	edge_queue_test(BMEdge *e){ return (BM_elem_flag_test(e, BM_ELEM_TAG) == false); }
	void	edge_queue_enable(BMEdge *e){BM_elem_flag_enable(e, BM_ELEM_TAG);}
	void	edge_queue_disable(BMEdge *e){ BM_elem_flag_disable(e, BM_ELEM_TAG); };
	void    leaf_node_test_first_touch(BMLeafNode *node);
private:
	StrokeData *_sdata;
	BMesh *_bm;
	BMBvh *_bvh;
	std::vector<BMLeafNode*> _nodes;
	Vector3f _center;
	float    _radius, _sqrRadius;
	float    _maxEdgeLen , _sqrMaxEdgeLen;
	float	 _minEdgeLen , _sqrMinEdgeLen;
};
#endif