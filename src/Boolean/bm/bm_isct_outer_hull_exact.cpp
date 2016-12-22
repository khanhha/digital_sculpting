#include "bm/bm_isct_outer_hull_exact.h"
#include "bm/bm_isct_isct_problem.h"
#include <queue>
#include <boost/container/static_vector.hpp>
#include "BaseLib/MathGeom.h"

#ifdef ISCT_TEST
#include "BaseLib/MathUtil.h"
extern std::vector<Point3Dd> g_vscene_testSegments;
extern std::vector<Point3Dd> g_vscene_testSegments_1;
extern std::vector<Point3Dd> g_vscene_testPoints;
extern std::vector<Vector3f> g_test_color_triangles;
#endif

namespace bm_isct
{
	using namespace  VM;

	IsctFaceExtractor::IsctFaceExtractor(BMesh *bm, std::vector<BMFace*> &isct_faces)
		:
		_bm(bm),
		_isct_faces(isct_faces)
	{
		_bm->BM_mesh_elem_table_ensure(BM_FACE, true);
	}

	IsctFaceExtractor::~IsctFaceExtractor()
	{

	}

	void IsctFaceExtractor::run()
	{
		isct_faces_collect(_bm, _isct_faces);
	}

	void IsctFaceExtractor::isct_faces_collect(BMesh *bm, std::vector<BMFace*> &isct_faces)
	{
		VBvh::SpacialHash<TriPrim, float> shash;
		std::vector<TriPrim> prims;
		IsectFacePairs isctpairs;
		std::vector<CellPtr> cells;

		tri_prims_compute(prims);

		shash.Set<std::vector<TriPrim>::iterator>(prims.begin(), prims.end());

		cell_build(shash, cells);

		cell_filter(cells);

		cell_filter_tri_tri_isect(cells, isctpairs);

		for (auto it = isctpairs.begin(); it != isctpairs.end(); ++it){
			if (!BM_elem_app_flag_test(it->first, F_VISITED)){
				BM_elem_app_flag_enable(it->first, F_VISITED);
				isct_faces.push_back(it->first);
			}

			if (!BM_elem_app_flag_test(it->second, F_VISITED)){
				BM_elem_app_flag_enable(it->second, F_VISITED);
				isct_faces.push_back(it->second);
			}
		}

		for (auto it = isct_faces.begin(); it != isct_faces.end(); ++it){
			BM_elem_app_flag_disable(*it, F_VISITED);
		}

		//debug_output_all_cell_tris(shash, g_vscene_testSegments);
	}


	void IsctFaceExtractor::clone_bmesh(BMesh *origin, const std::vector<BMFace*> &org_isct_faces, BMesh **clone_mesh, std::vector<BMFace*> &clone_isct_faces)
	{
		BMesh *clone = new BMesh(*origin);

		const std::vector<BMFace*> &clone_faces = clone->BM_mesh_face_table();

		for (auto it = org_isct_faces.begin(); it != org_isct_faces.end(); ++it){
			BMFace *org_f = *it;
			clone_isct_faces.push_back(clone_faces[BM_elem_index_get(org_f)]);
		}

		*clone_mesh = clone;
	}


	void IsctFaceExtractor::tri_prims_compute(std::vector<TriPrim> &prims)
	{
		BMesh *bm = _bm;

		const std::vector<BMFace*> &faces = bm->BM_mesh_face_table();
		const size_t totface = faces.size();
		prims.resize(totface);

		auto range_compute = [&](const tbb::blocked_range<size_t> &range)
		{
			for (size_t i = range.begin(); i != range.end(); ++i){
				prims[i].face = faces[i];
				BM_face_calc_bounds(prims[i].face, prims[i].bb.min(), prims[i].bb.max());
			}
		};

		tbb::blocked_range<size_t> tot_range(0, totface);
		if (totface > 10000){
			tbb::parallel_for(tot_range, range_compute);
		}
		else{
			range_compute(tot_range);
		}
	}


	void IsctFaceExtractor::cell_build(const VBvh::SpacialHash<TriPrim, float> &spacial, std::vector<CellPtr> &cells)
	{
		typedef VBvh::SpacialHash<TriPrim, float>::HashType::const_local_iterator range_iter;
		const VBvh::SpacialHash<TriPrim, float>::HashType &hash = spacial.hash_table;
#if 0
		BBox3f bbox = spacial.bbox;
		using boost::math::lround;

		for (int i = 0; i < spacial.siz[0]; ++i){
			for (int j = 0; j < spacial.siz[1]; ++j){
				for (int k = 0; k < spacial.siz[2]; ++k){
					Vector3i index = Vector3i(i, j, k);

					int bk_idx = hash.bucket(index);
					range_iter t_first = hash.begin(bk_idx);
					range_iter t_end = hash.end(bk_idx);
					if (t_first != t_end){
						CellPtr cell = CellPtr(new Cell());


						Vector3f real_index = index.cast<float>();
						real_index[0] *= spacial.voxel[0];
						real_index[1] *= spacial.voxel[1];
						real_index[2] *= spacial.voxel[2];

						cell->lower = bbox.min() + real_index;
						cell->size = spacial.voxel;

						for (auto it = t_first; it != t_end; ++it){
							cell->tris.push_back(it->second);
						}
						cells.push_back(cell);
#if 1
						{
							BBox3f cbb(cell->lower, cell->lower + cell->size);
							debug_output_bb(cbb, g_vscene_testSegments);
						}
#endif
					}

}
			}
		}
#else
		const size_t totalbucket = hash.bucket_count();
		for (size_t bk_idx = 0; bk_idx < totalbucket; ++bk_idx){

			range_iter t_first = hash.begin(bk_idx);
			range_iter t_end = hash.end(bk_idx);
			if (t_first != t_end){
				CellPtr cell = CellPtr(new Cell());
				for (auto it = t_first; it != t_end; ++it){
					cell->tris.push_back(it->second);
				}
				cells.push_back(cell);
			}
		}
#endif

	}

#ifdef ISCT_TEST
	void IsctFaceExtractor::debug_output_all_cell_tris(VBvh::SpacialHash<TriPrim, float> &spacial, std::vector<Point3Dd> &segments)
	{
		for (auto it = spacial.hash_table.begin(); it != spacial.hash_table.end(); ++it){
			TriPrim *val = it->second;
			{
				BMVert *v[3];
				VM::BM_face_as_array_vert_tri(val->face, v);
				g_vscene_testSegments.push_back(MathUtil::convert(v[0]->co));
				g_vscene_testSegments.push_back(MathUtil::convert(v[1]->co));
				g_vscene_testSegments.push_back(MathUtil::convert(v[1]->co));
				g_vscene_testSegments.push_back(MathUtil::convert(v[2]->co));
				g_vscene_testSegments.push_back(MathUtil::convert(v[2]->co));
				g_vscene_testSegments.push_back(MathUtil::convert(v[0]->co));
			}
		}
	}

	void IsctFaceExtractor::debug_output_self_isect_cell()
	{
#if 0
		for (auto it = _cells.begin(); it != _cells.end(); ++it){
			CellPtr cell = *it;
#if 0
			if (!(cell->filter & Cell::FILTER_1_SINGLE_COMBINATORIAL_DISK)) continue;
#endif
#if 0
			if (!(cell->filter & Cell::FILTER_2_POSITIVE_DIRECTION)) continue;
#endif
#if 0
			if (!(cell->filter & Cell::FILTER_3_FREE_ISECT_BOUNDARY)) continue;
#endif
#if 1
			if (cell->filter & (Cell::FILTER_SINGLE_COMBINATORIAL_DISK) &&
				cell->filter & (Cell::FILTER_POSITIVE_DIRECTION) &&
				cell->filter & (Cell::FILTER_FREE_ISECT_BOUNDARY))
				continue;
#endif


			for (auto it = cell->tris.begin(); it != cell->tris.end(); ++it){

				BMFace *f = (*it)->face;
				BMVert *verts[3];
				BM_face_as_array_vert_tri(f, verts);

				g_test_color_triangles.push_back(f->no);
				for (size_t i = 0; i < 3; ++i){
					g_test_color_triangles.push_back(Vector3f(1.0f, 0.0f, 0.0f));
					g_test_color_triangles.push_back(verts[i]->co + 0.001f * f->no);
				}
			}

			BBox3f bb; bb.setEmpty();
			bb.min() = cell->lower;
			bb.max() = cell->lower + cell->size;

			debug_output_bb(bb, g_vscene_testSegments);

			//g_vscene_testSegments.push_back(MathUtil::convert(bb.center()));
			//g_vscene_testSegments.push_back(MathUtil::convert(bb.center() + 2.0 * cell->ref_dir));
			//g_vscene_testSegments.push_back(MathUtil::convert(bb.center()));
			//g_vscene_testSegments.push_back(MathUtil::convert(bb.center() + 2.0 * cell->norm));
		}

	

#endif


	}


	void IsctFaceExtractor::debug_output(VBvh::SpacialHash<TriPrim, float> &spacial)
	{
		g_vscene_testSegments.clear();

		typedef VBvh::SpacialHash<TriPrim, float>::HashType::local_iterator range_iter;
		std::vector<std::pair<range_iter, range_iter>> buckets;

		BBox3i ibox;
		spacial.BoxToIBox(spacial.bbox, ibox);

		int cnt = 0;
		auto &hash = spacial.hash_table;
		const size_t totalbucket = hash.bucket_count();
		for (int i = 0; i < spacial.siz[0]; ++i){
			for (int j = 0; j < spacial.siz[1]; ++j){
				for (int k = 0; k < spacial.siz[2]; ++k){
					int bk_idx = hash.bucket(ibox.min() + Vector3i(i, j, k));
					if (bk_idx){
						BBox3f bb; bb.setEmpty();
						bool exist = false;
						for (auto it = hash.begin(bk_idx); it != hash.end(bk_idx); ++it){
							TriPrim *tri = it->second;
							bb.extend(tri->bb);
							exist = true;
						}

						if (exist){
							debug_output_bb(bb, g_vscene_testSegments);
							cnt++;
							if (cnt > 30) return;
						}
					}
				}
			}
		}


	}

	void IsctFaceExtractor::debug_output_isect_tris()
	{
#if 0
		for (auto it = _isect_faces.begin(); it != _isect_faces.end(); ++it){
			BMFace *tris[2];
			tris[0] = it->first;
			tris[1] = it->second;

			for (size_t t = 0; t < 2; ++t){
				BMFace *f = tris[t];
				BMVert *verts[3];
				BM_face_as_array_vert_tri(f, verts);

				g_test_color_triangles.push_back(f->no);
				for (size_t i = 0; i < 3; ++i){
					g_test_color_triangles.push_back(Vector3f(1.0f, 0.0f, 0.0f));
					g_test_color_triangles.push_back(verts[i]->co + 0.001f * f->no);
				}
			}
		}
#endif
	}

	void IsctFaceExtractor::debug_output_bb(BBox3f &bb, std::vector<Point3Dd> &segments)
	{
		Point3Dd listVertex[8];
		Point3Dd bbmin = Point3Dd(bb.min()[0], bb.min()[1], bb.min()[2]);
		Point3Dd bbmax = Point3Dd(bb.max()[0], bb.max()[1], bb.max()[2]);

		double size[3];
		for (int i = 0; i < 3; i++)
			size[i] = bbmax[i] - bbmin[i];

		listVertex[0] = bbmin;

		listVertex[1][0] = bbmin[0];
		listVertex[1][1] = bbmin[1] + size[1];
		listVertex[1][2] = bbmin[2];

		listVertex[2][0] = bbmin[0] + size[0];
		listVertex[2][1] = bbmin[1] + size[1];
		listVertex[2][2] = bbmin[2];

		listVertex[3][0] = bbmin[0] + size[0];
		listVertex[3][1] = bbmin[1];
		listVertex[3][2] = bbmin[2];

		listVertex[4][0] = bbmin[0];
		listVertex[4][1] = bbmin[1];
		listVertex[4][2] = bbmin[2] + size[2];

		listVertex[5][0] = bbmin[0] + size[0];
		listVertex[5][1] = bbmin[1];
		listVertex[5][2] = bbmin[2] + size[2];

		listVertex[6][0] = bbmin[0] + size[0];;
		listVertex[6][1] = bbmin[1] + size[1];
		listVertex[6][2] = bbmin[2] + size[2];

		listVertex[7][0] = bbmin[0];
		listVertex[7][1] = bbmin[1] + size[1];
		listVertex[7][2] = bbmin[2] + size[2];

		segments.push_back(listVertex[0]);
		segments.push_back(listVertex[1]);
		segments.push_back(listVertex[1]);
		segments.push_back(listVertex[2]);
		segments.push_back(listVertex[2]);
		segments.push_back(listVertex[3]);
		segments.push_back(listVertex[3]);
		segments.push_back(listVertex[0]);

		segments.push_back(listVertex[4]);
		segments.push_back(listVertex[5]);
		segments.push_back(listVertex[5]);
		segments.push_back(listVertex[6]);
		segments.push_back(listVertex[6]);
		segments.push_back(listVertex[7]);
		segments.push_back(listVertex[7]);
		segments.push_back(listVertex[4]);

		segments.push_back(listVertex[0]);
		segments.push_back(listVertex[4]);
		segments.push_back(listVertex[1]);
		segments.push_back(listVertex[7]);
		segments.push_back(listVertex[2]);
		segments.push_back(listVertex[6]);
		segments.push_back(listVertex[3]);
		segments.push_back(listVertex[5]);
	}

	void IsctFaceExtractor::debug_output_tri_segments(BMFace *f, std::vector<Point3Dd> &segments)
	{
		BMVert *verts[3];
		BM_face_as_array_vert_tri(f, verts);
		g_vscene_testSegments.push_back(MathUtil::convert(verts[0]->co));
		g_vscene_testSegments.push_back(MathUtil::convert(verts[1]->co));
		g_vscene_testSegments.push_back(MathUtil::convert(verts[1]->co));
		g_vscene_testSegments.push_back(MathUtil::convert(verts[2]->co));
		g_vscene_testSegments.push_back(MathUtil::convert(verts[2]->co));
		g_vscene_testSegments.push_back(MathUtil::convert(verts[0]->co));
	}
#endif

	void IsctFaceExtractor::cell_filter(std::vector<CellPtr> &cells)
	{
		cell_filter_positive_direction(cells);
		cell_filter_single_combinatorial_disk(cells);
		cell_filter_free_isect_boundary(cells);
	}

	void IsctFaceExtractor::cell_filter_positive_direction(std::vector<CellPtr> &cells)
	{
		static const Vector3f aligned_dirs[6] =
		{
			Vector3f(1.0f, 0.0f, 0.0f),
			Vector3f(0.0f, 1.0f, 0.0f),
			Vector3f(0.0f, 0.0f, 1.0f),
			Vector3f(-1.0f, 0.0f, 0.0f),
			Vector3f(0.0f, -1.0f, 0.0f),
			Vector3f(0.0f, 0.0f, -1.0f)
		};

		auto range_process = [&](const tbb::blocked_range<size_t> &range)
		{
			typedef IsctFaceExtractor::Cell Cell;

			for (size_t i = range.begin(); i != range.end(); ++i){
				CellPtr cell = cells[i];

				/*compute cell's average normal*/
				Vector3f norm(Vector3f::Zero());
				for (auto it = cell->tris.begin(); it != cell->tris.end(); ++it){
					norm += (*it)->face->no;
				}
				cell->norm = norm.normalized();


				/*find reference direction*/
				float max_dot = std::numeric_limits<float>::lowest();
				Vector3f max_dir;
				for (size_t i = 0; i < sizeof(aligned_dirs) / sizeof(Vector3f); ++i){
					float dot = aligned_dirs[i].dot(cell->norm);
					if (dot > max_dot){
						max_dot = dot;
						max_dir = aligned_dirs[i];
					}
				}
				cell->ref_dir = max_dir;


				/*check positive reference direction*/
				bool all_positive = true;
				for (auto it = cell->tris.begin(); it != cell->tris.end(); ++it){
					BMFace *f = (*it)->face;
					if (cell->ref_dir.dot(f->no) < 0.0f){
						all_positive = false;
						break;
					}
				}

				if (all_positive){
					cell->filter |= Cell::FILTER_POSITIVE_DIRECTION;
				}
			}

		};

		/*calculate cell's average norm*/
#ifdef TBB_PARALLEL
		tbb::parallel_for(tbb::blocked_range<size_t>(0, cells.size()),
			[&](const tbb::blocked_range<size_t> &range)
		{
			range_process(range);
		});
#else
		range_process(tbb::blocked_range<size_t>(0, cells.size()));
#endif

	}

	void IsctFaceExtractor::cell_filter_single_combinatorial_disk(std::vector<CellPtr> &cells)
	{
		std::queue<BMFace*> fqueue;

		const std::vector<BMFace*> &faces = _bm->BM_mesh_face_table();
		const size_t totface = faces.size();

		for (auto it = faces.begin(); it != faces.end(); ++it){
			BM_elem_app_flag_clear(*it);
		}

		for (auto it = cells.begin(); it != cells.end(); ++it){
			CellPtr cell = *it;

			/*no need for further check with cell violating position direc constraint
			*such cell require a brute-force tri-tri intersection check */
			if (!(cell->filter & Cell::FILTER_POSITIVE_DIRECTION)) continue;

			/*un-mark all cell's faces*/
			for (auto it = cell->tris.begin(); it != cell->tris.end(); ++it){
				BM_elem_app_flag_clear((*it)->face);
				BM_elem_app_flag_enable((*it)->face, F_IN_CELL);
			}

			size_t tot_visited = 0;
			BMFace *seed = cell->tris.front()->face;
			BM_elem_app_flag_enable(seed, F_VISITED);

			fqueue.push(seed);
			BMLoop *l_first, *l_iter, *l_other;
			while (!fqueue.empty()){
				BMFace *f = fqueue.front();
				fqueue.pop();
				tot_visited++;

				l_first = BM_FACE_FIRST_LOOP(f);
				l_iter = l_first;
				do {
					l_other = (l_iter->radial_next != l_iter) ? l_iter->radial_next : nullptr;
					if (l_other && !BM_elem_app_flag_test(l_other->f, F_VISITED)) /*is not visited yet*/
					{
						if (BM_elem_app_flag_test(l_other->f, F_IN_CELL)){
							BM_elem_app_flag_enable(l_other->f, F_VISITED);
							fqueue.push(l_other->f);
						}
						else{
							cell->bdr_edges.push_back(l_iter->e);
						}
					}
				} while ((l_iter = l_iter->next) != l_first);
			}

			if (tot_visited == cell->tris.size()){
				/*from a seed face, we could iterate all other faces in cell*/
				cell->filter |= Cell::FILTER_SINGLE_COMBINATORIAL_DISK;
			}

			/*clear all flags*/
			for (auto it = cell->tris.begin(); it != cell->tris.end(); ++it){
				BM_elem_app_flag_clear((*it)->face);
			}
		}
	}
	void IsctFaceExtractor::cell_filter_free_isect_boundary(std::vector<CellPtr> &cells)
	{
		auto range_process = [&](const tbb::blocked_range<size_t> &range)
		{
			typedef IsctFaceExtractor::Cell Cell;

			for (size_t i = range.begin(); i != range.end(); ++i){
				CellPtr cell = cells[i];

				const std::vector<BMEdge*> bdr_edges = cell->bdr_edges;
				const size_t totsegs = bdr_edges.size();

				if (totsegs == 0) continue;

				/*find the projection plane*/
				size_t REF_AXIS = 0;
				for (size_t j = 0; j < 3; ++j){
					if (cell->ref_dir[j] > 0.99f || cell->ref_dir[j] < -0.99f){
						REF_AXIS = j; break;
					}
				}
				const size_t PROJ_DIM_0 = (REF_AXIS + 1) % 3;
				const size_t PROJ_DIM_1 = (REF_AXIS + 2) % 3;


				bool free_isect = true;

				for (size_t j = 0; j < totsegs - 1; ++j){
					BMEdge *e0 = bdr_edges[j];
					Vector2f p00(e0->v1->co[PROJ_DIM_0], e0->v1->co[PROJ_DIM_1]);
					Vector2f p01(e0->v2->co[PROJ_DIM_0], e0->v2->co[PROJ_DIM_1]);

					for (size_t k = j + 1; k < totsegs; ++k){
						BMEdge *e1 = bdr_edges[k];
						if (!BM_edge_share_vert(e0, e1)){
							Vector2f p10(e1->v1->co[PROJ_DIM_0], e1->v1->co[PROJ_DIM_1]);
							Vector2f p11(e1->v2->co[PROJ_DIM_0], e1->v2->co[PROJ_DIM_1]);
							if (MathGeom::isect_seg_seg_v2(p00, p01, p10, p11) == MathGeom::ISECT_LINE_LINE_CROSS){
								free_isect = false;
								break;;
							}
						}
					}

					if (!free_isect){
						break;
					}
				}

				if (free_isect){
					cell->filter |= Cell::FILTER_FREE_ISECT_BOUNDARY;
				}

				cell->bdr_edges.clear();
			}
		};

		tbb::blocked_range<size_t> total_range(0, cells.size());
#ifdef TBB_PARALLEL
		tbb::parallel_for(total_range, range_process);
#else
		range_process(total_range);
#endif

	}

	void IsctFaceExtractor::cell_filter_tri_tri_isect(std::vector<CellPtr> &cells, IsectFacePairs &isct_pairs)
	{
		std::vector<CellPtr> isect_cell_cands; /*self isect cell candidates*/
		for (auto it = cells.begin(); it != cells.end(); ++it){
			CellPtr cell = *it;

			if (cell->filter & (Cell::FILTER_SINGLE_COMBINATORIAL_DISK) &&
				cell->filter & (Cell::FILTER_POSITIVE_DIRECTION) &&
				cell->filter & (Cell::FILTER_FREE_ISECT_BOUNDARY))
				continue;

			isect_cell_cands.push_back(cell);
		}

		auto range_process = [&](tbb::blocked_range<size_t> &range)
		{
			for (size_t i = range.begin(); i != range.end(); ++i){
				CellPtr cell = isect_cell_cands[i];

				const std::vector<TriPrimPtr> &tris = cell->tris;
				const size_t tottris = tris.size();
				if (tottris > 0){
					for (size_t j = 0; j < tottris - 1; ++j){
						TriPrimPtr tri0 = tris[j];
						for (size_t k = j + 1; k < tottris; ++k){
							TriPrimPtr tri1 = tris[k];
							if (MathGeom::isect_aligned_box_aligned_box_v3(tri0->bb.min(), tri0->bb.max(), tri1->bb.min(), tri1->bb.max())){
								const BMLoop *l0 = BM_FACE_FIRST_LOOP(tri0->face);
								const BMLoop *l1 = BM_FACE_FIRST_LOOP(tri1->face);

								int cnt = tri_tri_isct::tri_tri_overlap_test_3d(
									l0->v->co.data(), l0->next->v->co.data(), l0->next->next->v->co.data(),
									l1->v->co.data(), l1->next->v->co.data(), l1->next->next->v->co.data());

								if (cnt == 1 && !tri_pair_adjacent(tri0->face, tri1->face)){
									isct_pairs.push_back(std::make_pair(tri0->face, tri1->face));
								}
							}
						}
					}
				}
			}
		};

		tbb::blocked_range<size_t> total_range(0, isect_cell_cands.size());
#ifdef TBB_PARALLEL
		tbb::parallel_for(total_range, range_process);
#else
		range_process(total_range);
#endif
	}

	bool IsctFaceExtractor::tri_pair_adjacent(const BMFace *tri0, const BMFace *tri1)
	{
		BMVert *verts[3];
		BM_face_as_array_vert_tri(tri1, verts);

		BMLoop *l_first = BM_FACE_FIRST_LOOP(tri0);
		BMLoop *l_iter = l_first;
		do {
			if (l_iter->v == verts[0] ||
				l_iter->v == verts[1] ||
				l_iter->v == verts[2])
				return true;
		} while ((l_iter = l_iter->next) != l_first);

		return false;
	}

	BmOuterHullExtract::BmOuterHullExtract(BMesh *mesh)
		:
		_org_bm(mesh),
		_bm(nullptr)
	{
	}

	BmOuterHullExtract::~BmOuterHullExtract()
	{
	}

	void BmOuterHullExtract::extract()
	{
		IsctFaceExtractor selfisct(_org_bm, _org_isct_faces);
		selfisct.run();

		if (!_org_isct_faces.empty()){

			clone_bmesh(_org_bm, _org_isct_faces, &_bm, _isct_faces);

			float avg_edge_len = edge_avg_len(_isct_faces);

			IsctProblem isct(_bm, _isct_faces);
			isct.resovleIntersections();

			outer_hull_peel();

			delete_non_manifol_edges(_bm);

			collapse_short_subdivided_isct_edges(_bm, 0.05f * avg_edge_len);

			clear(_bm);
		}
	}


	VM::BMesh * BmOuterHullExtract::result()
	{
		return _bm;
	}


	void BmOuterHullExtract::clear(VM::BMesh *mesh)
	{
		BMIter iter;
		BMVert *v;
		BMEdge *e;
		BMFace *f;
		BM_ITER_MESH(v, &iter, mesh, BM_VERTS_OF_MESH){
			BM_elem_app_flag_clear(v);
			BM_elem_flag_disable(v, BM_ELEM_TAG);
		}
		BM_ITER_MESH(f, &iter, mesh, BM_FACES_OF_MESH){
			BM_elem_flag_disable(f, BM_ELEM_TAG);
			BM_elem_app_flag_clear(f);
		}
		BM_ITER_MESH(e, &iter, mesh, BM_EDGES_OF_MESH){
			BM_elem_flag_disable(e, BM_ELEM_TAG);
			BM_elem_app_flag_clear(e);
		}
	}

	void BmOuterHullExtract::delete_non_manifol_edges(VM::BMesh *mesh)
	{
		std::vector<BMEdge*> kill_edges;
		BMIter iter;
		BMEdge *e;
		
		BM_ITER_MESH(e, &iter, mesh, BM_EDGES_OF_MESH){
			if (BM_edge_face_count(e) > 2){
				kill_edges.push_back(e);
			}
		}

		for (auto it = kill_edges.begin(); it != kill_edges.end(); ++it){
			mesh->BM_edge_kill(*it);
		}
	}


	void BmOuterHullExtract::clone_bmesh(BMesh *origin, const std::vector<BMFace*> &org_isct_faces, BMesh **clone_mesh, std::vector<BMFace*> &clone_isct_faces)
	{
		BMesh *clone = new BMesh(*origin);

		const std::vector<BMFace*> &clone_faces = clone->BM_mesh_face_table();

		for (auto it = org_isct_faces.begin(); it != org_isct_faces.end(); ++it){
			BMFace *org_f = *it;
			clone_isct_faces.push_back(clone_faces[BM_elem_index_get(org_f)]);
		}

		*clone_mesh = clone;
	}
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void BmOuterHullExtract::outer_hull_peel()
	{
		BMFace *seed_face = outer_hull_seed_face(_bm);
		outer_hull_propagate(seed_face);

		std::vector<BMFace*> kill_faces;
		BMIter iter;
		BMFace *f;
		BM_ITER_MESH(f, &iter, _bm, BM_FACES_OF_MESH){
			if (!BM_elem_app_flag_test(f, F_OUTER_HULL)){
				kill_faces.push_back(f);
			}
		}

		for (auto it = kill_faces.begin(); it != kill_faces.end(); ++it){
			_bm->BM_face_kill_loose(*it, false);
		}
	}


	void BmOuterHullExtract::outer_hull_propagate(BMFace* seed_face)
	{
		BMLoop *l_iter, *l_first;
		std::queue<BMFace*> fqueue;

		fqueue.push(seed_face);
		BM_elem_app_flag_enable(seed_face, F_VISITED);
		while (!fqueue.empty()){
			BMFace *f = fqueue.front();
			fqueue.pop();

			BM_elem_app_flag_enable(f, F_OUTER_HULL);

			l_iter = l_first = BM_FACE_FIRST_LOOP(f);
			do {
				BMFace *next_f = nullptr;
				if (BM_elem_app_flag_test(l_iter->e, E_SUBDIVIDED_ISECT)){
					next_f = outer_hull_next_outer_face_around_edge(l_iter->e, f);
				}
				else{
					next_f = l_iter->radial_next != l_iter ? l_iter->radial_next->f : nullptr;
				}

				if (next_f && !BM_elem_app_flag_test(next_f, F_VISITED)){
					BM_elem_app_flag_enable(next_f, F_VISITED);
					fqueue.push(next_f);
				}

			} while ((l_iter = l_iter->next) != l_first);
		}
	}

	VM::BMFace* BmOuterHullExtract::outer_hull_seed_face(VM::BMesh *bm)
	{
		/*find the extreme face*/
		const size_t AXIS = 0;
		float max_axis = std::numeric_limits<float>::lowest();
		BMVert *max_v = nullptr;

		BMIter iter;
		BMVert *v;
		BM_ITER_MESH(v, &iter, bm, BM_VERTS_OF_MESH){
			if (v->co[AXIS] > max_axis){
				max_v = v;
				max_axis = v->co[AXIS];
			}
		}

		BMFace *max_f = nullptr, *f;
		if (max_v){
			max_axis = std::numeric_limits<float>::lowest();
			BM_ITER_ELEM(f, &iter, max_v, BM_FACES_OF_VERT){
				if (f->no[AXIS] > max_axis){
					max_f = f;
					max_axis = f->no[AXIS];
				}
			}
		}
		return max_f;
	}

	BMFace* BmOuterHullExtract::outer_hull_next_outer_face_around_edge(BMEdge *e, BMFace *cur_face)
	{
		if (BM_edge_calc_length_squared(e) < 1.0e-5) return nullptr;

		const Vector3f e_mid = 0.5f * (e->v1->co + e->v2->co);
		const Vector3f &ref_no = cur_face->no;

		float	max_radial_dot = 0.0f;
		BMFace *max_radial_face = nullptr;

		BMLoop *l_first = e->l;
		BMLoop *l_iter = l_first;
		Vector3f dir;
		float tmp_scalar;
		size_t edge_valence = 0;
		do {
			if (l_iter->f != cur_face){
				BMVert *other_v = BM_face_tri_other_vert(l_iter->f, e->v1, e->v2);
				if (other_v){
					dir = other_v->co - e_mid; tmp_scalar = MathUtil::normalize(dir);
					
					if(tmp_scalar < 1.0e-2f) 
						return nullptr;

					tmp_scalar = dir.dot(ref_no);
					if (tmp_scalar > max_radial_dot){
						max_radial_face	= l_iter->f;
						max_radial_dot	= tmp_scalar;
					}
				}
				else{
					return nullptr;
				}
			}
			edge_valence++;
		} while ((l_iter = l_iter->radial_next) != l_first);

		return (edge_valence == 4 && max_radial_dot > 0.0f) ? max_radial_face : nullptr;
	}
}
