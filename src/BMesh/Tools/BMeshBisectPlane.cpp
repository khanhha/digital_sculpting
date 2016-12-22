#include "Tools/BMeshBisectPlane.h"
#include <boost/container/small_vector.hpp>
#include "BaseLib/UtilMacro.h"

VM_BEGIN_NAMESPACE

BMehsBisectPlane::BMehsBisectPlane(BMesh *bm, 
	const Eigen::Vector4f &plane, const bool use_snap_center, 
	const bool use_tag, const short oflag_center, const float eps, bool clear_outer, bool clear_inner, float dist)
	:
	_bm(bm),
	_plane(plane),
	_use_snap_center(use_snap_center),
	_use_tag(use_tag),
	_oflag_center(oflag_center),
	_eps(eps),
	_clear_inner(clear_inner),
	_clear_outer(clear_outer),
	_dist(dist)
{
}

void BMehsBisectPlane::run()
{
	bmesh_bisect_plane();
	kill_inner_outer_faces();
}

BMehsBisectPlane::~BMehsBisectPlane()
{
}

void BMehsBisectPlane::bmesh_bisect_plane()
{
	unsigned int i;
	std::vector<BMEdge*> edges_arr; edges_arr.reserve(_bm->BM_mesh_edges_total());
	std::vector<BMFace*> face_stack;

	BMVert *v;
	BMFace *f;

	BMIter iter;

	if (_use_tag) {
		/* build tagged edge array */
		BMVert *v;
		BMEdge *e;

		/* flush edge tags to verts */
		BM_ITER_MESH_INDEX(v, &iter, _bm, BM_VERTS_OF_MESH, i){
			BM_elem_flag_disable(v, BM_ELEM_TAG);
		}

		/* keep face tags as is */
		BM_ITER_MESH_INDEX(e, &iter, _bm, BM_EDGES_OF_MESH, i) {
			if (edge_is_cut_test(e)) {
				edges_arr.push_back(e);
				/* flush edge tags to verts */
				BM_elem_flag_enable(e->v1, BM_ELEM_TAG);
				BM_elem_flag_enable(e->v2, BM_ELEM_TAG);
			}
		}
		/* face tags are set by caller */
	}
	else {
		BMEdge *e;
		BM_ITER_MESH_INDEX(e, &iter, _bm, BM_EDGES_OF_MESH, i) {
			edge_is_cut_enable(e);
			edges_arr.push_back(e);
		}

		BM_ITER_MESH(f, &iter, _bm, BM_FACES_OF_MESH) {
			face_in_stack_disable(f);
		}
	}


	BM_ITER_MESH(v, &iter, _bm, BM_VERTS_OF_MESH) {

		if (_use_tag && !BM_elem_flag_test(v, BM_ELEM_TAG)) {
			vert_is_center_disable(v);

			/* these should never be accessed */
			BM_VERT_DIR(v) = 0;
			BM_VERT_DIST(v) = 0.0f;

			continue;
		}

		vert_is_center_disable(v);
		BM_VERT_DIR(v) = plane_point_test_v3(_plane, v->co, _eps, &(BM_VERT_DIST(v)));

		if (BM_VERT_DIR(v) == 0) {
			if (_oflag_center) {
				BM_elem_app_flag_enable(v, _oflag_center);
			}
			if (_use_snap_center) {
				MathGeom::closest_to_plane_v3(v->co, _plane, v->co);
			}
		}
	}

	/* store a stack of faces to be evaluated for splitting */
	for (auto it = edges_arr.begin(); it != edges_arr.end(); it++) {
		/* we could check edge_is_cut_test(e) but there is no point */
		BMEdge *e = *it;
		const int side[2] = { BM_VERT_DIR(e->v1), BM_VERT_DIR(e->v2) };
		const float dist[2] = { BM_VERT_DIST(e->v1), BM_VERT_DIST(e->v2) };

		if (side[0] && side[1] && (side[0] != side[1])) {
			const float e_fac = std::abs(dist[0]) / std::abs(dist[0] - dist[1]);
			BMVert *v_new;

			if (e->l) {
				BMLoop *l_iter, *l_first;
				l_iter = l_first = e->l;
				do {
					if (!face_in_stack_test(l_iter->f)) {
						face_in_stack_enable(l_iter->f);
						face_stack.push_back(l_iter->f);
					}
				} while ((l_iter = l_iter->radial_next) != l_first);
			}

			v_new = _bm->BM_edge_split(e, e->v1, NULL, e_fac);
			vert_is_center_enable(v_new);
			if (_oflag_center) {
				BM_elem_app_flag_enable(v_new, _oflag_center);
			}

			BM_VERT_DIR(v_new) = 0;
			BM_VERT_DIST(v_new) = 0.0f;
		}
		else if (side[0] == 0 || side[1] == 0) {
			/* check if either edge verts are aligned,
			* if so - tag and push all faces that use it into the stack */
			unsigned int j;
			BM_ITER_ELEM_INDEX(v, &iter, e, BM_VERTS_OF_EDGE, j) {
				if (
					side[j] == 0) {
					if (vert_is_center_test(v) == 0) {
						BMIter itersub;
						BMLoop *l_iter;

						vert_is_center_enable(v);

						BM_ITER_ELEM(l_iter, &itersub, v, BM_LOOPS_OF_VERT) {
							if (!face_in_stack_test(l_iter->f)) {
								face_in_stack_enable(l_iter->f);
								face_stack.push_back(l_iter->f);
							}
						}

					}
				}
			}

			/* if both verts are on the center - tag it */
			if (_oflag_center) {
				if (side[0] == 0 && side[1] == 0) {
					BM_elem_app_flag_enable(e, _oflag_center);
				}
			}
		}
	}

	while (!face_stack.empty()) {
		BMFace *f = face_stack.back(); face_stack.pop_back();
		bmesh_face_split(f);
	}
}

void BMehsBisectPlane::bmesh_face_split(BMFace *f)
{
	/* unlikely more than 2 verts are needed */
	boost::container::small_vector<BMVert*, 4> vert_split_arr;
	
	BMLoop *l_iter, *l_first;
	bool use_dirs[3] = { false, false, false };
	bool is_inside = false;

	l_first = BM_FACE_FIRST_LOOP(f);

	/* add plane-aligned verts to the stack
	* and check we have verts from both sides in this face,
	* ... that the face doesn't only have boundary verts on the plane for eg. */
	l_iter = l_first;
	do {
		if (vert_is_center_test(l_iter->v)) {
			BLI_assert(BM_VERT_DIR(l_iter->v) == 0);

			/* if both are -1 or 1, or both are zero:
			* don't flip 'inside' var while walking */
			BM_VERT_SKIP(l_iter->v) = (((BM_VERT_DIR(l_iter->prev->v) ^ BM_VERT_DIR(l_iter->next->v))) == 0);

			vert_split_arr.push_back(l_iter->v);
		}
		use_dirs[BM_VERT_DIR(l_iter->v) + 1] = true;
	} while ((l_iter = l_iter->next) != l_first);

	if ((vert_split_arr.size() > 1) && (use_dirs[0] && use_dirs[2]))
	{
		if (LIKELY(vert_split_arr.size() == 2)) {
			BMLoop *l_new;
			BMLoop *l_a, *l_b;

			l_a = BM_face_vert_share_loop(f, vert_split_arr[0]);
			l_b = BM_face_vert_share_loop(f, vert_split_arr[1]);

			/* common case, just cut the face once */
			_bm->BM_face_split(f, l_a, l_b, &l_new, NULL, true);
			if (l_new) {
				if (_oflag_center) {
					BM_elem_app_flag_enable(l_new->e, _oflag_center);
					BM_elem_app_flag_enable(l_new->f, _oflag_center);
					BM_elem_app_flag_enable(f, _oflag_center);
				}
			}
		}
		else {
			/* less common case, _complicated_ we need to calculate how to do multiple cuts */
			boost::container::small_vector<BMFace*, 4> face_split_arr;
			Vector3f sort_dir;
			unsigned int i;

			/* ---- */
			/* Calculate the direction to sort verts in the face intersecting the plane */

			/* exact dir isn't so important,
			* just need a dir for sorting verts across face,
			* 'sort_dir' could be flipped either way, it not important, we only need to order the array
			*/
			sort_dir = f->no.cross(Vector3f(_plane[0], _plane[1], _plane[2]));
			if (UNLIKELY(sort_dir.norm() == 0.0f)) {
				/* find any 2 verts and get their direction */
				for (i = 0; i < vert_split_arr.size(); i++) {
					if (!vert_split_arr[0]->co.isApprox(vert_split_arr[i]->co)) {
						sort_dir = (vert_split_arr[0]->co - vert_split_arr[i]->co).normalized();
					}
				}
				if (UNLIKELY(i == vert_split_arr.size())) {
					/* ok, we can't do anything useful here,
					* face has no area or so, bail out, this is highly unlikely but not impossible */
					goto finally;
				}
			}

			std::sort(vert_split_arr.begin(), vert_split_arr.end(), 
				[&](BMVert *v_a_v, BMVert *v_b_v)
			{
				return sort_dir.dot(v_a_v->co) < sort_dir.dot(v_b_v->co);
			});


			/* ---- */
			/* Split the face across sorted splits */

			/* note: we don't know which face gets which splits,
			* so at the moment we have to search all faces for the vert pair,
			* while not all that nice, typically there are < 5 resulting faces,
			* so its not _that_ bad. */

			face_split_arr.push_back(f);

			for (i = 0; i < vert_split_arr.size() - 1; i++) {
				BMVert *v_a = vert_split_arr[i];
				BMVert *v_b = vert_split_arr[i + 1];

				if (!BM_VERT_SKIP(v_a)) {
					is_inside = !is_inside;
				}

				if (is_inside) {
					BMLoop *l_a, *l_b;
					bool found = false;
					unsigned int j;

					for (j = 0; j < face_split_arr.size(); j++) {
						/* would be nice to avoid loop lookup here,
						* but we need to know which face the verts are in */
						if ((l_a = BM_face_vert_share_loop(face_split_arr[j], v_a)) &&
							(l_b = BM_face_vert_share_loop(face_split_arr[j], v_b)))
						{
							found = true;
							break;
						}
					}

					/* ideally wont happen, but it can for self intersecting faces */
					// BLI_assert(found == true);

					/* in fact this simple test is good enough,
					* test if the loops are adjacent */
					if (found && !BM_loop_is_adjacent(l_a, l_b)) {
						BMLoop *l_new;
						BMFace *f_tmp;
						f_tmp = _bm->BM_face_split(face_split_arr[j], l_a, l_b, &l_new, NULL, true);

						if (l_new) {
							if (_oflag_center) {
								BM_elem_app_flag_enable(l_new->e, _oflag_center);
								BM_elem_app_flag_enable(l_new->f, _oflag_center);
								BM_elem_app_flag_enable(face_split_arr[j], _oflag_center);
							}
						}

						if (f_tmp) {
							if (f_tmp != face_split_arr[j]) {
								face_split_arr.push_back(f_tmp);
							}
						}
					}
				}
				else {
					// printf("no intersect\n");
				}
			}
		}
	}

	finally:
	(void)vert_split_arr;
}

void BMehsBisectPlane::kill_inner_outer_faces()
{
	if (_clear_outer || _clear_inner) {
		/* Use an array of vertices because 'geom' contains both vers and edges that may use them.
		* Removing a vert may remove and edge which is later checked by BMO_ITER.
		* over-alloc the total possible vert count */
		std::vector<BMVert*> vert_arr;
		BMVert *v;
		BMIter iter;
		Vector4f plane_inner = _plane;
		Vector4f plane_outer = _plane;

		plane_outer[3] = _plane[3] - _dist;
		plane_inner[3] = _plane[3] + _dist;
	
		BM_ITER_MESH(v, &iter, _bm, BM_VERTS_OF_MESH){
			if ((_clear_outer && MathGeom::plane_point_side_v3(plane_outer, v->co) > 0.0f) ||
				(_clear_inner && MathGeom::plane_point_side_v3(plane_inner, v->co) < 0.0f))
			{
				vert_arr.push_back(v);
			}
		}

		for (auto it = vert_arr.begin(); it != vert_arr.end(); ++it){
			_bm->BM_vert_kill(*it, false);
		}
	}
}

VM_END_NAMESPACE