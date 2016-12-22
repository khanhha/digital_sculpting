#ifndef VMESH_VMESH_CORE_H
#define VMESH_VMESH_CORE_H

#include "BMeshClass.h"
#include <boost/pool/object_pool.hpp>
#include <boost/container/small_vector.hpp>
#include "BMUtilDefine.h"
#include "BaseLib/UtilMacro.h"
#include "BMCustomData.h"
#include <Eigen/Dense>
#include "tbb/scalable_allocator.h"
using namespace  Eigen;

VM_BEGIN_NAMESPACE

template<typename T>
class ElementList
{
public:
	ElementList() :_head(nullptr), _tail(nullptr), _total(0) {}

	bool exist(T *elm)
	{
		T *iter = _head;
		if (iter){
			do{
				if (iter == elm) return true;
			} while ((iter = iter->next_elm) != nullptr);
		}
		return false;
	}

	void insert(T *elm)
	{
		if (_head != nullptr && _tail != nullptr){
			
			elm->next_elm = nullptr; elm->prev_elm = nullptr;

			_tail->next_elm = elm;
			elm->prev_elm = _tail;
			_tail = elm;
			_tail->next_elm = nullptr;
		}
		else{
			elm->next_elm = elm->prev_elm = nullptr;
			_head = _tail = elm;
		}

		_total++;
	}

	void remove(T *elm)
	{
		if (elm->next_elm){
			elm->next_elm->prev_elm = elm->prev_elm;
		}
		if (elm->prev_elm){
			elm->prev_elm->next_elm = elm->next_elm;
		}
		if (elm == _head){
			_head = elm->next_elm;
		}
		if (elm == _tail){
			_tail = elm->prev_elm;
		}
		
		elm->next_elm = elm->prev_elm = nullptr;
		
		_total--;
	}

	T* head(){ return _head; }

	T* tail(){ return _tail; }

private:
	T		*_head, *_tail;
	size_t	_total;
};

class BMesh
{
public:
	/* flags for BM_edge_rotate */
	enum {
		BM_EDGEROT_CHECK_EXISTS = (1 << 0), /* disallow to rotate when the new edge matches an existing one */
		BM_EDGEROT_CHECK_SPLICE = (1 << 1), /* overrides existing check, if the edge already, rotate and merge them */
		BM_EDGEROT_CHECK_DEGENERATE = (1 << 2), /* disallow creating bow-tie, concave or zero area faces */
		BM_EDGEROT_CHECK_BEAUTY = (1 << 3)  /* disallow to rotate into ugly topology */
	};
	/* Triangulate methods - NGons */
	enum {
		MOD_TRIANGULATE_NGON_BEAUTY = 0,
		MOD_TRIANGULATE_NGON_EARCLIP,
	};

	/* Triangulate methods - Quads */
	enum {
		MOD_TRIANGULATE_QUAD_BEAUTY = 0,
		MOD_TRIANGULATE_QUAD_FIXED,
		MOD_TRIANGULATE_QUAD_ALTERNATE,
		MOD_TRIANGULATE_QUAD_SHORTEDGE
	};
public:
	BMesh();
	~BMesh();
	BMesh(BMesh &other);

	BMVert *BM_vert_create(const Vector3f &co, const BMVert *v_example, const eBMCreateFlag create_flag);
	BMEdge *BM_edge_create(BMVert *v1, BMVert *v2, const BMEdge *e_example, const eBMCreateFlag create_flag);
	BMFace *BM_face_create(BMVert **verts, BMEdge **edges, const int len, const BMFace *f_example, const eBMCreateFlag create_flag);
	BMFace *BM_face_create_verts(BMVert **verts, const int len, const BMFace *f_example, const eBMCreateFlag create_flag, const bool create_edges);
	BMFace *BM_face_create_quad_tri(BMVert *v1, BMVert *v2, BMVert *v3, BMVert *v4, const BMFace *f_example, const eBMCreateFlag create_flag);

	bool BM_edges_from_verts(BMEdge **edge_arr, BMVert **vert_arr, const int len);
	void BM_edges_from_verts_ensure(BMEdge **edge_arr, BMVert **vert_arr, const int len);

	void BM_face_edges_kill(BMFace *f);
	void BM_face_verts_kill(BMFace *f);
	void BM_face_kill(BMFace *f, bool log);
	void BM_face_kill_loose(BMFace *f, bool log);
	void BM_face_normal_flip(BMFace *f);
	void BM_face_triangulate(BMFace *f,
		std::vector<BMFace*> *r_faces_new, std::vector<BMEdge*> *r_edges_new, std::vector<BMFace*> *r_faces_double,
		const int quad_method, const int ngon_method, const bool use_tag);

	void BM_edge_kill(BMEdge *e, bool log = false);
	void BM_vert_kill(BMVert *v, bool log = false);
	void BM_face_logged_kill_only(BMFace *f);
	void BM_vert_logged_kill_only(BMVert *v);
	void BM_vert_logged_restore(BMVert *v, const eBMCreateFlag create_flag);
	void BM_face_logged_restore(BMFace *f, BMVert **verts, size_t len, const eBMCreateFlag create_flag);

	void  	BM_face_tri_edge_split(BMEdge *e, BMVert **r_nv, BMEdge **r_ne);
	BMFace *BM_face_split(BMFace *f, BMLoop *l_a, BMLoop *l_b, BMLoop **r_l, BMEdge *example, const bool no_double);
	BMVert *BM_edge_split(BMEdge *e, BMVert *v, BMEdge **r_e, float fac);
	bool    BM_edge_collapse_tri_manifold_ok(BMEdge *e);
	BMVert *BM_edge_collapse(BMEdge *e_kill, BMVert *v_kill, const bool do_del, const bool kill_degenerate_faces);
	bool	BM_edge_tri_collapse(BMEdge *e_clear, BMVert *v_clear);
	bool	BM_edge_tri_flip(BMEdge *e, char check_flag);
	void	BM_edge_calc_rotate(BMEdge *e, const bool ccw, BMLoop **r_l1, BMLoop **r_l2);
	bool    BM_edge_rotate_check_degenerate( BMEdge *e, BMLoop *l1, BMLoop *l2);

	void  BM_data_layer_add_named(char htype, int type, const char *name);
	void  BM_data_layer_free(char htype, int type);
	void  BM_data_layer_free_n(char htype, int type, int n);
	void  BM_data_layer_copy(char htype, int type, int src_n, int dst_n);
	int	  BM_data_get_offset(char htype, int type);
	int	  BM_data_get_n_offset(char htype, int type, int n);
	int   BM_data_get_layer_index(char htype, int type);
	int	  BM_data_get_named_layer_index(char htype, int type, const char *name);
	
	CustomData &BM_data_vert(){ return _vert_data;}
	CustomData &BM_data_face(){ return _face_data;}
	CustomData &BM_data_edge(){ return _edge_data;}

	BMVert *BM_vert_begin_list(){ return _vert_list.head(); };
	BMEdge *BM_edge_begin_list(){ return _edge_list.head(); };
	BMFace *BM_face_begin_list(){ return _face_list.head(); };

	size_t  BM_mesh_faces_total(){ return _tot_face; };
	size_t  BM_mesh_edges_total(){ return _tot_edge; };
	size_t  BM_mesh_verts_total(){ return _tot_vert; };
	size_t  BM_mesh_loops_total(){ return _tot_loop; };


	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void	BM_mesh_normals_update();
	void	BM_mesh_normals_update_parallel();
	void	BM_mesh_normals_update_area();
	bool	BM_edge_splice(BMEdge *e_dst, BMEdge *e_src);
	bool    BM_vert_splice(BMVert *v_dst, BMVert *v_src);

private:
	/* helper function for 'BM_mesh_copy' */
	BMLoop *bm_loop_create(BMVert *v, BMEdge *e, BMFace *f, const BMLoop *l_example, const eBMCreateFlag create_flag);
	BMFace *bm_face_create__internal();
	BMLoop *bm_face_boundary_add(BMFace *f, BMVert *startv, BMEdge *starte, const eBMCreateFlag create_flag);
	BMFace *bm_mesh_copy_new_face(BMesh *bm_new, BMesh *bm_old, std::vector<BMVert*> &vtable, std::vector<BMEdge*> &etable, BMFace *f);
	bool    bmesh_loop_reverse(BMFace *f);

	void    bm_kill_only_vert(BMVert *v, bool log);
	void    bm_kill_only_edge(BMEdge *e);
	void    bm_kill_only_face(BMFace *f, bool log);
	void    bm_kill_only_loop(BMLoop *l);

	BMVert *bmesh_semv(BMVert *tv, BMEdge *e, BMEdge **r_e);
	BMFace *bmesh_sfme(BMFace *f, BMLoop *l_v1, BMLoop *l_v2, BMLoop **r_l, BMEdge *e_example, const bool no_double);
	BMVert *bmesh_jvke(BMEdge *e_kill, BMVert *v_kill, const bool do_del, const bool check_edge_double, const bool kill_degenerate_faces);


	BMFace *bmesh_jfke(BMFace *f1, BMFace *f2, BMEdge *e);


	BMFace *bm_face_create__sfme(BMFace *f_example);
	void	BM_data_layer_add_named(CustomData *data, int type, const char *name);
	void	BM_data_layer_free(CustomData *data, int type);
	void	BM_data_layer_free_n(CustomData *data, int type, int n);
	void	BM_data_layer_copy(CustomData *data, int type, int src_n, int dst_n);
	void	update_data_blocks(CustomData *olddata, CustomData *data);

public:
	void  BM_data_interp_from_edges(const BMEdge *e_src_1, const BMEdge *e_src_2, BMEdge *e_dst, const float fac);
	void  BM_data_interp_from_verts(const BMVert *v_src_1, const BMVert *v_src_2, BMVert *v_dst, const float fac);
	void BM_data_interp_face_vert_edge(const BMVert *v_src_1, const BMVert *v_src_2, BMVert *v, BMEdge *e, const float fac);
	static void BM_elem_attrs_copy_ex(const BMesh *bm_src, BMesh *bm_dst, const void *ele_src_v, void *ele_dst_v, const unsigned char hflag_mask);
	static void BM_elem_attrs_copy(const BMesh *bm_src, BMesh *bm_dst, const void *ele_src, void *ele_dst);

	void BM_mesh_elem_index_ensure(const char hflag);
	void BM_mesh_elem_index_validate(const char *location, const char *func, const char *msg_a, const char *msg_b);
	void BM_mesh_elem_index_check(const char htype);
	void BM_mesh_elem_index_table_check(const char htype);
	bool BM_mesh_elem_table_check();
	void BM_mesh_elem_table_ensure(const char htype, bool ensure_index = false);
	bool BM_mesh_elem_table_dirty(const char htype);
	bool BM_mesh_elem_index_dirty(const char htype);
	void BM_mesh_elem_index_dirty_set(const char htype);
	const std::vector<BMFace*>& BM_mesh_face_table(){ return ftable; };
	const std::vector<BMVert*>& BM_mesh_vert_table(){ return vtable; };
	const std::vector<BMEdge*>& BM_mesh_edge_table(){ return etable; };
private:
	/* edge and vertex share, currently theres no need to have different logic */
	static void bm_data_interp_from_elem(CustomData *data_layer, const BMElem *ele_src_1, const BMElem *ele_src_2, BMElem *ele_dst, const float fac);
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	static void bm_vert_attrs_copy(const BMesh *source_mesh, BMesh *target_mesh, const BMVert *source_vertex, BMVert *target_vertex);
	static void bm_edge_attrs_copy(const BMesh *source_mesh, BMesh *target_mesh, const BMEdge *source_edge, BMEdge *target_edge);
	static void bm_loop_attrs_copy(const BMesh *source_mesh, BMesh *target_mesh, const BMLoop *source_loop, BMLoop *target_loop);
	static void bm_face_attrs_copy(const BMesh *source_mesh, BMesh *target_mesh, const BMFace *source_face, BMFace *target_face);
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	void bm_mesh_edges_calc_vectors(std::vector<Vector3f> &edgevec, const std::vector<Vector3f> &vcos);
	void bm_mesh_verts_calc_normals(const std::vector<Vector3f> &edgevec, const std::vector<Vector3f> &fnos, const std::vector<Vector3f> &vcos, std::vector<Vector3f> &vnos);
private:
#ifdef USE_BOOST_POOL
	boost::pool<> _vpool;
	boost::pool<> _epool;
	boost::pool<> _lpool;
	boost::pool<> _fpool;
#else
	tbb::scalable_allocator<BMVert> _vpool;
	tbb::scalable_allocator<BMEdge> _epool;
	tbb::scalable_allocator<BMLoop> _lpool;
	tbb::scalable_allocator<BMFace> _fpool;

#endif
	CustomData _vert_data, _edge_data, _face_data, _loop_data;

	size_t _tot_vert, _tot_edge, _tot_loop, _tot_face;

	/*active element*/
	ElementList<BMVert> _vert_list;
	ElementList<BMEdge> _edge_list;
	ElementList<BMLoop> _loop_list;
	ElementList<BMFace> _face_list;

	/*removed element*/
	/*just save removed vertices and faces for undo/redo*/
	ElementList<BMVert> _removed_vert_list;
	ElementList<BMFace> _removed_face_list;

	std::vector<BMVert*> vtable;
	std::vector<BMEdge*> etable;
	std::vector<BMFace*> ftable;


	/* may add to middle of the pool */
	char _elem_index_dirty;
	char _elem_table_dirty;
};




VM_END_NAMESPACE
#endif