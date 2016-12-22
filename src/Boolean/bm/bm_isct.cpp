#include "bm/bm_isct.h"
namespace bm_isct
{
	using namespace  VM;
	void collapse_short_subdivided_isct_edges(VM::BMesh *mesh, float threshold)
	{
		typedef std::pair<BMVert*, BMVert*> CoEdge;
		std::vector<CoEdge> collapse_edges;
		const float sqr_threshold = threshold * threshold;
		BMIter iter;
		BMEdge *e;
		BM_ITER_MESH(e, &iter, mesh, BM_EDGES_OF_MESH){
			if (BM_elem_app_flag_test(e, E_SUBDIVIDED_ISECT)){
				if (BM_edge_calc_length_squared(e) < sqr_threshold &&
					BM_edge_face_collapse_is_degenearte_topology_general(e)){
					collapse_edges.push_back(CoEdge(e->v1, e->v2));
				}
			}
		}

		if (collapse_edges.empty()) return;

		std::sort(collapse_edges.begin(), collapse_edges.end(),
			[&](const CoEdge &a, const CoEdge &b) -> bool
		{
			return (a.first->co - a.second->co).squaredNorm() < (b.first->co - b.second->co).squaredNorm();
		});

		for (auto it = collapse_edges.begin(); it != collapse_edges.end(); ++it){
			BMEdge *e = BM_edge_exists(it->first, it->second);
			if (e && BM_edge_calc_length_squared(e) < sqr_threshold && BM_edge_face_collapse_is_degenearte_topology_general(e))
			{
				Vector3f optimize_co;
				BM_edge_collapse_optimize_co(e, optimize_co);

				BMVert *v_other = e->v2;
				mesh->BM_edge_collapse(e, e->v1, true, true);
				v_other->co = optimize_co;

			}
		}
	}

	float edge_avg_len(const std::vector<VM::BMFace*> &faces)
	{
		float avg_len = 0.0f;
		size_t count = 0;

		BMLoop *l_iter, *l_first;
		for (auto it = faces.begin(); it != faces.end(); ++it){
			l_iter = l_first = BM_FACE_FIRST_LOOP(*it);
			do {
				avg_len += BM_edge_calc_length(l_iter->e);
				count++;
			} while ((l_iter = l_iter->next) != l_first);
		}

		return (avg_len / (float)(count));
	}
}
