#include "BMeshCore.h"
#include "BMUtilDefine.h"
#include "BMeshInline.h"
#include "BMeshStructure.h"
#include "BMeshQueries.h"
#include "BMeshIterators.h"
#include "BMeshPolygon.h"
#include "BMeshUtility.h"
#include "tbb/task_group.h"
#include "tbb/parallel_for.h"
#include "BaseLib/MathBase.h"
#include "BaseLib/MathUtil.h"
#include "BaseLib/MathGeom.h"
#include <algorithm>
#include <boost/container/static_vector.hpp>
#include <boost/container/small_vector.hpp>
#include <tuple>

extern std::vector<Point3Dd> g_vscene_testPoints;

VM_BEGIN_NAMESPACE

BMesh::BMesh()
	:
	_elem_table_dirty(0),
	_elem_index_dirty(0),
	_tot_vert(0),
	_tot_edge(0),
	_tot_loop(0),
	_tot_face(0)
#ifdef USE_BOOST_POOL
#ifdef LIMIT_MEM_POOL
	_vpool(sizeof(BMVert), 32, 256),
	_epool(sizeof(BMEdge), 32, 256),
	_fpool(sizeof(BMFace), 32, 256),
	_lpool(sizeof(BMLoop), 32, 256)
#else
	_vpool(sizeof(BMVert)),
	_epool(sizeof(BMEdge)),
	_fpool(sizeof(BMFace)),
	_lpool(sizeof(BMLoop))
#endif
#endif
{
}

BMesh::~BMesh()
{}

BMesh::BMesh(BMesh &other)
	:
	_elem_table_dirty(0),
	_elem_index_dirty(0),
	_tot_vert(0),
	_tot_edge(0),
	_tot_loop(0),
	_tot_face(0),
#ifdef USE_BOOST_POOL
	_vpool(sizeof(BMVert), 32, 256),
	_epool(sizeof(BMEdge), 32, 256),
	_fpool(sizeof(BMFace), 32, 256),
	_lpool(sizeof(BMLoop), 32, 256),
#endif
	_vert_data(other._vert_data),
	_edge_data(other._edge_data),
	_face_data(other._face_data),
	_loop_data(other._loop_data)
{
	vtable.resize(other._tot_vert);
	etable.resize(other._tot_edge);
	ftable.resize(other._tot_face);

	BMIter iter;
	BMVert *v;
	size_t i = 0;
	BM_ITER_MESH_INDEX(v, &iter, &other, BM_VERTS_OF_MESH, i) {
		/* copy between meshes so cant use 'example' argument */
		BMVert *v_new = BM_vert_create(v->co, NULL, BM_CREATE_SKIP_CD);
		BM_elem_attrs_copy_ex(&other, this, v, v_new, 0xff);
		v_new->head.hflag = v->head.hflag;  /* low level! don't do this for normal api use */
		vtable[i] = v_new;
		BM_elem_index_set(v, i); /* set_inline */
		BM_elem_index_set(v_new, i); /* set_inline */
	}
	_elem_index_dirty &= ~BM_VERT;

	BLI_assert(i == other.BM_mesh_verts_total());

	BMEdge *e;
	BM_ITER_MESH_INDEX(e, &iter, &other, BM_EDGES_OF_MESH, i) {
		BMEdge *e_new = BM_edge_create(vtable[BM_elem_index_get(e->v1)], vtable[BM_elem_index_get(e->v2)], e, BM_CREATE_SKIP_CD);
		BM_elem_attrs_copy_ex((const BMesh*)&other, this, e, e_new, 0xff);
		e_new->head.hflag = e->head.hflag;  /* low level! don't do this for normal api use */
		etable[i] = e_new;
		BM_elem_index_set(e, i); /* set_inline */
		BM_elem_index_set(e_new, i); /* set_inline */
	}
	_elem_index_dirty &= ~BM_EDGE;
	/* safety check */
	BLI_assert(i == other.BM_mesh_edges_total());

	BMFace *f;
	BM_ITER_MESH_INDEX(f, &iter, &other, BM_FACES_OF_MESH, i) {
		BM_elem_index_set(f, i); /* set_inline */

		BMFace *f_new = bm_mesh_copy_new_face(this, &other, vtable, etable, f);

		ftable[i] = f_new;
	}
	_elem_index_dirty &= ~BM_FACE;
}

BMVert * BMesh::BM_vert_create(const Vector3f &co, const BMVert *v_example, const eBMCreateFlag create_flag)
{
#ifdef USE_BOOST_POOL
	BMVert *v = reinterpret_cast<BMVert*>(_vpool.malloc());
#else
	BMVert *v = reinterpret_cast<BMVert*>(_vpool.allocate(1));
#endif
	_vert_list.insert(v);

	BLI_assert((v_example == NULL) || (v_example->head.htype == BM_VERT));
	BLI_assert(!(create_flag & 1));

	/* --- assign all members --- */
	v->head.data = NULL;

	BM_elem_index_set(v, -1); /* set_ok_invalid */

	v->head.htype = BM_VERT;
	v->head.hflag = 0;
	v->head.api_flag = 0;
	v->head.app_flag = 0;

	/* allocate flags */

	/* 'v->no' is handled by BM_elem_attrs_copy */
	v->co = co;
	v->no.setZero();

	/*TODO: move vertex color out of here*/
	BM_vert_default_color_set(v);
	/* 'v->no' set below */

	v->e = NULL;
	/* --- done --- */


	/* disallow this flag for verts - its meaningless */
	BLI_assert((create_flag & BM_CREATE_NO_DOUBLE) == 0);

	/* may add to middle of the pool */
	_elem_index_dirty |= BM_VERT;
	_elem_table_dirty |= BM_VERT;

	_tot_vert++;

	if (!(create_flag & BM_CREATE_SKIP_CD)) {
		if (v_example) {
			/* handles 'v->no' too */
			BM_elem_attrs_copy(this, this, v_example, v);

			/* exception: don't copy the original shapekey index */
			/*	keyi = CustomData_bmesh_get(&bm->vdata, v->head.data, CD_SHAPE_KEYINDEX);
				if (keyi) {
				*keyi = ORIGINDEX_NONE;
				}*/
		}
		else {
			_vert_data.CustomData_bmesh_set_default(&v->head.data);
			v->no.setZero();
		}
	}
	else {
		if (v_example) {
			v->no = v_example->no;
		}
		else {
			v->no.setZero();
		}
	}

	BM_CHECK_ELEMENT(v);

	return v;
}

/**
* \brief Main function for creating a new edge.
*
* \note Duplicate edges are supported by the API however users should _never_ see them.
* so unless you need a unique edge or know the edge won't exist, you should call with \a no_double = true
*/
BMEdge * BMesh::BM_edge_create(BMVert *v1, BMVert *v2, const BMEdge *e_example, const eBMCreateFlag create_flag)
{
	BMEdge *e;

	BLI_assert(v1 != v2);
	BLI_assert(v1->head.htype == BM_VERT && v2->head.htype == BM_VERT);
	BLI_assert((e_example == NULL) || (e_example->head.htype == BM_EDGE));
	BLI_assert(!(create_flag & 1));

	if ((create_flag & BM_CREATE_NO_DOUBLE) && (e = BM_edge_exists(v1, v2)))
		return e;
#ifdef USE_BOOST_POOL
	e = reinterpret_cast<BMEdge*>(_epool.malloc());
#else
	e = reinterpret_cast<BMEdge*>(_epool.allocate(1));
#endif
	_edge_list.insert(e);

	/* --- assign all members --- */
	e->head.data = NULL;

	BM_elem_index_set(e, -1); /* set_ok_invalid */

	e->head.htype = BM_EDGE;
	e->head.hflag = 0/*BM_ELEM_SMOOTH | BM_ELEM_DRAW*/;
	e->head.api_flag = 0;
	e->head.app_flag = 0;

	/* allocate flags */

	e->v1 = v1;
	e->v2 = v2;
	e->l = NULL;

	memset(&e->v1_disk_link, 0, sizeof(BMDiskLink) * 2);
	/* --- done --- */


	bmesh_disk_edge_append(e, e->v1);
	bmesh_disk_edge_append(e, e->v2);

	/* may add to middle of the pool */
	_elem_index_dirty |= BM_EDGE;
	_elem_table_dirty |= BM_EDGE;

	_tot_edge++;

	if (!(create_flag & BM_CREATE_SKIP_CD)) {
		if (e_example) {
			BM_elem_attrs_copy(this, this, e_example, e);
		}
		else {
			_edge_data.CustomData_bmesh_set_default(&e->head.data);
		}
	}

	BM_CHECK_ELEMENT(e);

	return e;
}

/**
* \note In most cases a \a l_example should be NULL,
* since this is a low level API and we shouldn't attempt to be clever and guess whats intended.
* In cases where copying adjacent loop-data is useful, see #BM_face_copy_shared.
*/
BMLoop *BMesh::bm_loop_create(
	BMVert *v, BMEdge *e, BMFace *f,
	const BMLoop *l_example, const eBMCreateFlag create_flag)
{
	BMLoop *l = NULL;

#ifdef USE_BOOST_POOL
	l = reinterpret_cast<BMLoop*>(_lpool.malloc());
#else
	l = reinterpret_cast<BMLoop*>(_lpool.allocate(1));
#endif
	_loop_list.insert(l);

#ifdef USE_BMLOOP_HEAD
	BLI_assert((l_example == NULL) || (l_example->head.htype == BM_LOOP));
#endif
	BLI_assert(!(create_flag & 1));

#ifdef _DEBUG
	if (l_example) {
		/* ensure passing a loop is either sharing the same vertex, or entirely disconnected
		* use to catch mistake passing in loop offset-by-one. */
		BLI_assert((v == l_example->v) || !(v == l_example->prev->v || v == l_example->next->v));
	}
#endif

#ifdef USE_BMLOOP_HEAD
	/* --- assign all members --- */
	l->head.data = NULL;

	BM_elem_index_set(l, -1); /* set_ok_invalid */

	l->head.htype = BM_LOOP;
	l->head.hflag = 0;
	l->head.api_flag = 0;
#endif
	l->v = v;
	l->e = e;
	l->f = f;

	l->radial_next = NULL;
	l->radial_prev = NULL;
	l->next = NULL;
	l->prev = NULL;
	/* --- done --- */

	/* may add to middle of the pool */
	_elem_index_dirty |= BM_LOOP;

	_tot_loop++;

	if (!(create_flag & BM_CREATE_SKIP_CD)) {
#ifdef USE_BMLOOP_HEAD
		if (l_example) {
			/* no need to copy attrs, just handle customdata */
			//BM_elem_attrs_copy(this, this, l_example, l);
			_loop_data.CustomData_bmesh_free_block_data(l->head.data);
			CustomData::CustomData_bmesh_copy_data(&_loop_data, &_loop_data, l_example->head.data, &l->head.data);
		}
		else {
			_loop_data.CustomData_bmesh_set_default(&l->head.data);
		}
#endif
	}

	return l;
}
BMLoop * BMesh::bm_face_boundary_add(BMFace *f, BMVert *startv, BMEdge *starte, const eBMCreateFlag create_flag)
{
	BMLoop *l = bm_loop_create(startv, starte, f, NULL /* starte->l */, create_flag);

	bmesh_radial_append(starte, l);

	f->l_first = l;

	l->f = f;

	return l;
}

BMFace* BMesh::bm_mesh_copy_new_face(BMesh *bm_new, BMesh *bm_old, std::vector<BMVert*> &vtable, std::vector<BMEdge*> &etable, BMFace *f)
{
	boost::container::static_vector<BMLoop*, 5> loops;
	boost::container::static_vector<BMVert*, 5> verts;
	boost::container::static_vector<BMEdge*, 5> edges;

	BMFace *f_new;
	BMLoop *l_iter, *l_first;
	l_iter = l_first = BM_FACE_FIRST_LOOP(f);
	do {
		loops.push_back(l_iter);
		verts.push_back(vtable[BM_elem_index_get(l_iter->v)]);
		edges.push_back(etable[BM_elem_index_get(l_iter->e)]);
	} while ((l_iter = l_iter->next) != l_first);

	f_new = bm_new->BM_face_create(verts.data(), edges.data(), f->len, NULL, BM_CREATE_SKIP_CD);

	if (UNLIKELY(f_new == nullptr)) {
		return nullptr;
	}

	/* use totface in case adding some faces fails */
	BM_elem_index_set(f_new, (bm_new->_tot_face - 1)); /* set_inline */

	BM_elem_attrs_copy_ex((const BMesh*)bm_old, bm_new, f, f_new, 0xff);
	f_new->head.hflag = f->head.hflag;  /* low level! don't do this for normal api use */

#ifdef	USE_BMLOOP_HEAD
	size_t j = 0;
	l_iter = l_first = BM_FACE_FIRST_LOOP(f_new);
	do {
		BM_elem_attrs_copy(bm_old, bm_new, loops[j], l_iter);
		j++;
	} while ((l_iter = l_iter->next) != l_first);
#endif

	return f_new;
}
/**
* only create the face, since this calloc's the length is initialized to 0,
* leave adding loops to the caller.
*
* \note, caller needs to handle customdata.
*/
BMFace * BMesh::bm_face_create__internal()
{
	BMFace *f;
#ifdef USE_BOOST_POOL
	f = reinterpret_cast<BMFace*>(_fpool.malloc());
#else
	f = reinterpret_cast<BMFace*>(_fpool.allocate(1));
#endif
	_face_list.insert(f);

	/* --- assign all members --- */
	f->head.data = NULL;
	BM_elem_index_set(f, -1); /* set_ok_invalid */

	f->head.htype = BM_FACE;
	f->head.hflag = 0;
	f->head.api_flag = 0;
	f->head.app_flag = 0;

	/* allocate flags */
	//f->oflags = bm->ftoolflagpool ? BLI_mempool_calloc(bm->ftoolflagpool) : NULL;


	f->l_first = NULL;
	f->len = 0;
	f->no = Vector3f::Zero();

	/* caller must initialize */
	// zero_v3(f->no);
	/* --- done --- */

	/* may add to middle of the pool */
	_elem_index_dirty |= BM_FACE;
	_elem_table_dirty |= BM_FACE;

	_tot_face++;

	return f;
}

/**
* Main face creation function
*
* \param bm  The mesh
* \param verts  A sorted array of verts size of len
* \param edges  A sorted array of edges size of len
* \param len  Length of the face
* \param create_flag  Options for creating the face
*/
BMFace * BMesh::BM_face_create(BMVert **verts, BMEdge **edges, const int len, const BMFace *f_example, const eBMCreateFlag create_flag)
{
	BMFace *f = NULL;
	BMLoop *l, *startl, *lastl;
	int i;

	BLI_assert((f_example == NULL) || (f_example->head.htype == BM_FACE));
	BLI_assert(!(create_flag & 1));

	if (len == 0) {
		/* just return NULL for now */
		return NULL;
	}

	if (create_flag & BM_CREATE_NO_DOUBLE) {
		/* Check if face already exists */
		const bool is_overlap = BM_face_exists(verts, len, &f);
		if (is_overlap) {
			return f;
		}
		else {
			BLI_assert(f == NULL);
		}
	}

	f = bm_face_create__internal();

	startl = lastl = bm_face_boundary_add(f, verts[0], edges[0], create_flag);

	startl->v = verts[0];
	startl->e = edges[0];
	for (i = 1; i < len; i++) {
		l = bm_loop_create(verts[i], edges[i], f, NULL /* edges[i]->l */, create_flag);

		l->f = f;
		bmesh_radial_append(edges[i], l);

		l->prev = lastl;
		lastl->next = l;
		lastl = l;
	}

	startl->prev = lastl;
	lastl->next = startl;

	f->len = len;

	if (!(create_flag & BM_CREATE_SKIP_CD)) {
		if (f_example) {
			BM_elem_attrs_copy(this, this, f_example, f);
		}
		else {
			_face_data.CustomData_bmesh_set_default(&f->head.data);
			f->no.setZero();
		}
	}
	else {
		if (f_example) {
			f->no = f_example->no;
		}
		else {
			f->no.setZero();
		}
	}

	//BM_CHECK_ELEMENT(f);

	return f;
}

/**
* Wrapper for #BM_face_create when you don't have an edge array
*/
BMFace * BMesh::BM_face_create_verts(
	BMVert **vert_arr, const int len,
	const BMFace *f_example, const eBMCreateFlag create_flag, const bool create_edges)
{
	boost::container::small_vector<BMEdge*, 4> edge_arr;
	edge_arr.resize(len);

	if (create_edges) {
		BM_edges_from_verts_ensure(edge_arr.data(), vert_arr, len);
	}
	else {
		if (BM_edges_from_verts(edge_arr.data(), vert_arr, len) == false) {
			return NULL;
		}
	}

	return BM_face_create(vert_arr, edge_arr.data(), len, f_example, create_flag);
}

/**
* Fill in an edge array from a vertex array (connected polygon loop).
*
* \returns false if any edges aren't found .
*/
bool BMesh::BM_edges_from_verts(BMEdge **edge_arr, BMVert **vert_arr, const int len)
{
	int i, i_prev = len - 1;
	for (i = 0; i < len; i++) {
		edge_arr[i_prev] = BM_edge_exists(vert_arr[i_prev], vert_arr[i]);
		if (edge_arr[i_prev] == NULL) {
			return false;
		}
		i_prev = i;
	}
	return true;
}

/**
* Fill in an edge array from a vertex array (connected polygon loop).
* Creating edges as-needed.
*/
void BMesh::BM_edges_from_verts_ensure(BMEdge **edge_arr, BMVert **vert_arr, const int len)
{
	int i, i_prev = len - 1;
	for (i = 0; i < len; i++) {
		edge_arr[i_prev] = BM_edge_create(vert_arr[i_prev], vert_arr[i], NULL, BM_CREATE_NO_DOUBLE);
		i_prev = i;
	}
}

/**
* \brief Make Quad/Triangle
*
* Creates a new quad or triangle from a list of 3 or 4 vertices.
* If \a no_double is true, then a check is done to see if a face
* with these vertices already exists and returns it instead.
*
* If a pointer to an example face is provided, it's custom data
* and properties will be copied to the new face.
*
* \note The winding of the face is determined by the order
* of the vertices in the vertex array.
*/
BMFace * BMesh::BM_face_create_quad_tri(BMVert *v1, BMVert *v2, BMVert *v3, BMVert *v4, const BMFace *f_example, const eBMCreateFlag create_flag)
{
	BMVert *vtar[4] = { v1, v2, v3, v4 };
	return BM_face_create_verts(vtar, v4 ? 4 : 3, f_example, create_flag, true);
}

/**
* low level function, only move vertex to removed list,
* doesn't change or adjust surrounding geometry
*/
void BMesh::bm_kill_only_vert(BMVert *v, bool log)
{
	_tot_vert--;
	_elem_index_dirty |= BM_VERT;
	_elem_table_dirty |= BM_VERT;

	if (log){
		/*TODO: if we don't free custom data block, what happens if vertex custom data block change while
		this vertex is in removed vertex list. This coudl cause problem when this vertex is restored*/

		v->head.hflag |= BM_ELEM_REMOVED;
		_vert_list.remove(v);
		_removed_vert_list.insert(v);
	}
	else{
		if (v->head.data)
			_vert_data.CustomData_bmesh_free_block(&v->head.data);

		_vert_list.remove(v);
#ifdef USE_BOOST_POOL
		_vpool.free(v);
#else
		_vpool.deallocate(v, 1);
#endif
	}
}

/**
* low level function, only free the edge,
* doesn't change or adjust surrounding geometry
*/
void BMesh::bm_kill_only_edge(BMEdge *e)
{
	_tot_edge--;
	_elem_index_dirty |= BM_EDGE;
	_elem_table_dirty |= BM_EDGE;

	if (e->head.data)
		_edge_data.CustomData_bmesh_free_block(&e->head.data);

	_edge_list.remove(e);
#ifdef USE_BOOST_POOL
	_epool.free(e);
#else
	_epool.deallocate(e, 1);
#endif
}

/**
* low level function, only the face to the removed list,
* doesn't change or adjust surrounding geometry
*/
void BMesh::bm_kill_only_face(BMFace *f, bool log)
{
	_tot_face--;
	_elem_index_dirty |= BM_FACE;
	_elem_table_dirty |= BM_FACE;

	if (log){
		/*TODO: if we don't free custom data block, what happens if face custom data block change while
		this face is in removed face list. This could cause problem when this face is restored*/

		f->head.hflag |= BM_ELEM_REMOVED;
		_face_list.remove(f);
		_removed_face_list.insert(f);
	}
	else{
		if (f->head.data)
			_face_data.CustomData_bmesh_free_block(&f->head.data);

		_face_list.remove(f);
#ifdef USE_BOOST_POOL
		_fpool.free(f);
#else
		_fpool.deallocate(f,1);
#endif
	}
}

void BMesh::bm_kill_only_loop(BMLoop *l)
{
	_tot_loop--;
	_elem_index_dirty |= BM_LOOP;

#ifdef USE_BMLOOP_HEAD
	if (l->head.data)
		_loop_data.CustomData_bmesh_free_block(&l->head.data);
#endif

	_loop_list.remove(l);
#ifdef USE_BOOST_POOL
	_lpool.free(l);
#else
	_lpool.deallocate(l,1);
#endif
}

/**
* kills all edges associated with \a f, along with any other faces containing
* those edges
*/
void BMesh::BM_face_edges_kill(BMFace *f)
{
	BMEdge **edges = (BMEdge**)BLI_array_alloca(edges, f->len);
	BMLoop *l_iter;
	BMLoop *l_first;
	int i = 0;

	l_iter = l_first = BM_FACE_FIRST_LOOP(f);
	do {
		edges[i++] = l_iter->e;
	} while ((l_iter = l_iter->next) != l_first);

	for (i = 0; i < f->len; i++) {
		BM_edge_kill(edges[i]);
	}
}

/**
* kills all verts associated with \a f, along with any other faces containing
* those vertices
*/
void BMesh::BM_face_verts_kill(BMFace *f)
{
	BMVert **verts = (BMVert**)BLI_array_alloca(verts, f->len);
	BMLoop *l_iter;
	BMLoop *l_first;
	int i = 0;

	l_iter = l_first = BM_FACE_FIRST_LOOP(f);
	do {
		verts[i++] = l_iter->v;
	} while ((l_iter = l_iter->next) != l_first);

	for (i = 0; i < f->len; i++) {
		BM_vert_kill(verts[i], false);
	}
}

/**
* Kills \a f and its loops.
*/
void BMesh::BM_face_kill(BMFace *f, bool log)
{
	BM_CHECK_ELEMENT(f);
	if (f->l_first){
		BMLoop *l_iter, *l_next, *l_first;
		l_iter = l_first = f->l_first;

		do {
			l_next = l_iter->next;

			bmesh_radial_loop_remove(l_iter, l_iter->e);
			bm_kill_only_loop(l_iter);

		} while ((l_iter = l_next) != l_first);
	}

	f->len = 0;
	BM_FACE_FIRST_LOOP(f) = nullptr;

	bm_kill_only_face(f, log);
}

/**
* A version of #BM_face_kill which removes edges and verts
* which have no remaining connected geometry.
*/
void BMesh::BM_face_kill_loose(BMFace *f, bool log)
{
	BM_CHECK_ELEMENT(f);

	if (f->l_first){
		BMLoop *l_iter, *l_next, *l_first;
		l_iter = l_first = f->l_first;
		do {
			BMEdge *e;
			l_next = l_iter->next;

			e = l_iter->e;
			bmesh_radial_loop_remove(l_iter, e);
			bm_kill_only_loop(l_iter);

			if (e->l == NULL) {
				BMVert *v1 = e->v1, *v2 = e->v2;

				bmesh_disk_edge_remove(e, e->v1);
				bmesh_disk_edge_remove(e, e->v2);
				bm_kill_only_edge(e);

				if (v1->e == NULL) {
					bm_kill_only_vert(v1, log);
				}
				if (v2->e == NULL) {
					bm_kill_only_vert(v2, log);
				}
			}
		} while ((l_iter = l_next) != l_first);

	}

	bm_kill_only_face(f, log);
}

/**
* \brief Face Flip Normal
*
* Reverses the winding of a face.
* \note This updates the calculated normal.
*/
void  BMesh::BM_face_normal_flip(BMFace *f)
{
	bmesh_loop_reverse(f);
	f->no = -f->no;
}

void BMesh::BM_face_triangulate(
	BMFace *f,
	std::vector<BMFace*> *r_faces_new, std::vector<BMEdge*> *r_edges_new, std::vector<BMFace*> *r_faces_double,
	const int quad_method, const int ngon_method, const bool use_tag)
{
	const bool use_beauty = (ngon_method == MOD_TRIANGULATE_NGON_BEAUTY);
	BMLoop *l_first, *l_new;
	BMFace *f_new;
	int nf_i = 0;
	int ne_i = 0;

	//BLI_assert(BM_face_is_normal_valid(f));

	/* ensure both are valid or NULL */
	BLI_assert(f->len > 3);

	{
		typedef std::tuple<int, int, int> Triangle;
		boost::container::small_vector<BMLoop*, 4> loops; loops.resize(4);
		boost::container::small_vector<Triangle, 4> tris;
		const size_t totfilltri = f->len - 2;
		const size_t last_tri = f->len - 3;

		if (f->len == 4) {
			/* even though we're not using BLI_polyfill, fill in 'tris' and 'loops'
			* so we can share code to handle face creation afterwards. */
			BMLoop *l_v1, *l_v2;

			l_first = BM_FACE_FIRST_LOOP(f);

			switch (quad_method) {
			case MOD_TRIANGULATE_QUAD_FIXED:
			{
				l_v1 = l_first;
				l_v2 = l_first->next->next;
				break;
			}
			case MOD_TRIANGULATE_QUAD_ALTERNATE:
			{
				l_v1 = l_first->next;
				l_v2 = l_first->prev;
				break;
			}
			case MOD_TRIANGULATE_QUAD_SHORTEDGE:
			case MOD_TRIANGULATE_QUAD_BEAUTY:
			default:
			{
				BMLoop *l_v3, *l_v4;
				bool split_24;

				l_v1 = l_first->next;
				l_v2 = l_first->next->next;
				l_v3 = l_first->prev;
				l_v4 = l_first;

				if (quad_method == MOD_TRIANGULATE_QUAD_SHORTEDGE) {
					float d1, d2;
					d1 = (l_v4->v->co - l_v2->v->co).squaredNorm();
					d2 = (l_v1->v->co - l_v3->v->co).squaredNorm();
					split_24 = ((d2 - d1) > 0.0f);
				}
				else {
					/* first check if the quad is concave on either diagonal */
					const int flip_flag = MathGeom::is_quad_flip_v3(l_v1->v->co, l_v2->v->co, l_v3->v->co, l_v4->v->co);
					if (UNLIKELY(flip_flag & (1 << 0))) {
						split_24 = true;
					}
					else if (UNLIKELY(flip_flag & (1 << 1))) {
						split_24 = false;
					}
					else {
						split_24 = (BM_verts_calc_rotate_beauty(l_v1->v, l_v2->v, l_v3->v, l_v4->v, 0) > 0.0f);
					}
				}

				/* named confusingly, l_v1 is in fact the second vertex */
				if (split_24) {
					l_v1 = l_v4;
					//l_v2 = l_v2;
				}
				else {
					//l_v1 = l_v1;
					l_v2 = l_v3;
				}
				break;
			}
			}

			loops[0] = l_v1;
			loops[1] = l_v1->next;
			loops[2] = l_v2;
			loops[3] = l_v2->next;

			tris.push_back(Triangle(0, 1, 2));
			tris.push_back(Triangle(0, 2, 3));
		}
		else {
#if 0
			BMLoop *l_iter;
			float axis_mat[3][3];
			float(*projverts)[2] = BLI_array_alloca(projverts, f->len);

			axis_dominant_v3_to_m3_negate(axis_mat, f->no);

			for (i = 0, l_iter = BM_FACE_FIRST_LOOP(f); i < f->len; i++, l_iter = l_iter->next) {
				loops[i] = l_iter;
				mul_v2_m3v3(projverts[i], axis_mat, l_iter->v->co);
			}

			BLI_polyfill_calc_arena((const float(*)[2])projverts, f->len, 1, tris,
				pf_arena);

			if (use_beauty) {
				BLI_polyfill_beautify(
					(const float(*)[2])projverts, f->len, tris,
					pf_arena, pf_heap, pf_ehash);
			}

			BLI_memarena_clear(pf_arena);
#endif
		}

		/* loop over calculated triangles and create new geometry */
		for (size_t i = 0; i < totfilltri; i++) {
			BMLoop *l_tri[3] = {
				loops[std::get<0>(tris[i])],
				loops[std::get<1>(tris[i])],
				loops[std::get<2>(tris[i])] };

			BMVert *v_tri[3] = {
				l_tri[0]->v,
				l_tri[1]->v,
				l_tri[2]->v };

			f_new = BM_face_create_verts(v_tri, 3, f, BM_CREATE_NOP, true);
			l_new = BM_FACE_FIRST_LOOP(f_new);

			BLI_assert(v_tri[0] == l_new->v);

			/* check for duplicate */
			if (l_new->radial_next != l_new) {
				BMLoop *l_iter = l_new->radial_next;
				do {
					if (UNLIKELY((l_iter->f->len == 3) && (l_new->prev->v == l_iter->prev->v))) {
						r_faces_double->push_back(f_new);
						break;
					}
				} while ((l_iter = l_iter->radial_next) != l_new);
			}

			/* copy CD data */
#ifdef USE_BMLOOP_HEAD
			BM_elem_attrs_copy(this, this, l_tri[0], l_new);
			BM_elem_attrs_copy(this, this, l_tri[1], l_new->next);
			BM_elem_attrs_copy(this, this, l_tri[2], l_new->prev);
#endif
			/* add all but the last face which is swapped and removed (below) */
			if (i != last_tri) {
				if (use_tag) {
					BM_elem_flag_enable(f_new, BM_ELEM_TAG);
				}
				if (r_faces_new) {
					r_faces_new->push_back(f_new);
				}
			}

			if (use_tag || r_edges_new) {
				/* new faces loops */
				BMLoop *l_iter;

				l_iter = l_first = l_new;
				do {
					BMEdge *e = l_iter->e;
					/* confusing! if its not a boundary now, we know it will be later
					* since this will be an edge of one of the new faces which we're in the middle of creating */
					bool is_new_edge = (l_iter == l_iter->radial_next);

					if (is_new_edge) {
						if (use_tag) {
							BM_elem_flag_enable(e, BM_ELEM_TAG);
						}
						if (r_edges_new) {
							r_edges_new->push_back(e);
						}
					}
					/* note, never disable tag's */
				} while ((l_iter = l_iter->next) != l_first);
			}
		}

		{
			/* we can't delete the real face, because some of the callers expect it to remain valid.
			* so swap data and delete the last created tri */
			BM_face_kill(f, false);
		}
	}

	_elem_index_dirty |= BM_FACE;
}
/**
* kills \a e and all faces that use it.
*/
void BMesh::BM_edge_kill(BMEdge *e, bool log)
{
	bmesh_disk_edge_remove(e, e->v1);
	bmesh_disk_edge_remove(e, e->v2);

	if (e->l) {
		BMLoop *l = e->l, *lnext, *startl = e->l;

		do {
			lnext = l->radial_next;
			if (lnext->f == l->f) {
				BM_face_kill(l->f, log);
				break;
			}

			BM_face_kill(l->f, log);

			if (l == lnext)
				break;
			l = lnext;
		} while (l != startl);
	}

	bm_kill_only_edge(e);
}

/**
* kills \a v and all edges that use it.
*/
void BMesh::BM_vert_kill(BMVert *v, bool log)
{
	if (v->e) {
		BMEdge *e, *e_next;

		e = v->e;
		while (v->e) {
			e_next = bmesh_disk_edge_next(e, v);
			BM_edge_kill(e, log);
			e = e_next;
		}
	}

	bm_kill_only_vert(v, log);
}

/*delete logged face from removed list and memory pool*/
void BMesh::BM_face_logged_kill_only(BMFace *f)
{
	BLI_assert(BM_elem_flag_test(f, BM_ELEM_REMOVED) != 0);
	_removed_face_list.remove(f);
#ifdef USE_BOOST_POOL
	_fpool.free(f);
#else
	_fpool.deallocate(f, 1);
#endif
}

/*delete logged vertex from removed list and memory pool*/
void BMesh::BM_vert_logged_kill_only(BMVert *v)
{
	BLI_assert(BM_elem_flag_test(v, BM_ELEM_REMOVED) != 0);
	_removed_vert_list.remove(v);
#ifdef USE_BOOST_POOL
	_vpool.free(v);
#else
	_vpool.deallocate(v, 1);
#endif
}

void BMesh::BM_vert_logged_restore(BMVert *v, const eBMCreateFlag create_flag)
{
	BLI_assert(BM_elem_flag_test(v, BM_ELEM_REMOVED) != 0);
	
	BM_elem_flag_disable(v, BM_ELEM_REMOVED);
	
#ifdef _DEBUG
	BLI_assert(_removed_vert_list.exist(v));
	BLI_assert(!_vert_list.exist(v));
#endif

	_tot_vert++;
	_removed_vert_list.remove(v);
	_vert_list.insert(v);

	_elem_index_dirty |= BM_VERT;
	_elem_table_dirty |= BM_VERT;

#if 0
	/*TODO: consistence problem. is the current custom data block of this vertex
	have the same size as other verts now*/
	if (!(create_flag & BM_CREATE_SKIP_CD)) {
		_vert_data.CustomData_bmesh_set_default(&v->head.data);
		v->no.setZero();
	}
#endif

}

void BMesh::BM_face_logged_restore(BMFace *f, BMVert **verts, size_t len, const eBMCreateFlag create_flag)
{
	BLI_assert(BM_elem_flag_test(f, BM_ELEM_REMOVED) != 0);

	BM_elem_flag_disable(f, BM_ELEM_REMOVED);
	
#ifdef _DEBUG
	BLI_assert(_removed_face_list.exist(f));
	BLI_assert(!_face_list.exist(f));
#endif

	_tot_face++;
	_removed_face_list.remove(f);
	_face_list.insert(f);

	_elem_index_dirty |= BM_FACE;
	_elem_table_dirty |= BM_FACE;

	for (size_t i = 0; i < len; ++i){
		if (BM_elem_flag_test(verts[i], BM_ELEM_REMOVED) != 0){
			BM_vert_logged_restore(verts[i], create_flag);
		}
	}

	boost::container::small_vector<BMEdge*, 4> edges;
	for (size_t i = 0; i < len; ++i){
		BMEdge *e = BM_edge_create(verts[i], verts[(i + 1) % len], nullptr, create_flag);
		edges.push_back(e);
	}

	BMLoop *startl, *lastl;
	startl = lastl = bm_face_boundary_add(f, verts[0], edges[0], create_flag);

	startl->v = verts[0];
	startl->e = edges[0];
	for (size_t i = 1; i < len; i++) {
		BMLoop *l = bm_loop_create(verts[i], edges[i], f, NULL /* edges[i]->l */, create_flag);

		l->f = f;
		bmesh_radial_append(edges[i], l);

		l->prev = lastl;
		lastl->next = l;
		lastl = l;
	}

	startl->prev = lastl;
	lastl->next = startl;

	f->len = len;

#if 0
	/*TODO: consistence problem. is the current custom data block of this face
	have the same size as other faces now*/
	if (!(create_flag & BM_CREATE_SKIP_CD)) {
		_face_data.CustomData_bmesh_set_default(&f->head.data);
		f->no.setZero();
	}
#endif

	//BM_CHECK_ELEMENT(f);
}

/*
* \brief triangle edge split
* 
* \param e edge its faces are triangles
*/
void BMesh::BM_face_tri_edge_split(BMEdge *e, BMVert **r_nv, BMEdge **r_ne)
{
	BMEdge *ne = nullptr;
	BMVert *nv = nullptr;
	BMLoop *l_first, *l_iter, *nl;
	
	l_iter = l_first = e->l;
	do 
	{
		if (!l_iter || l_iter->f->len != 3)
			return;
	} while ((l_iter = l_iter->radial_next) != l_first);

	nv = BM_edge_split(e, e->v1, &ne, 0.5f);

	/*split e's radial faces*/
	l_first = l_iter = ne->l;
	if (l_first){
		do{
			BMFace *f = l_iter->f;
			if (f->len == 4){
				nl = BM_edge_vert_share_loop(l_iter, nv);
				bmesh_sfme(f, nl, nl->next->next, nullptr, nullptr, true);
			}
		} while ((l_iter = l_iter->radial_next) != l_first);
	}

	if (r_nv) *r_nv = nv;
	if (r_ne) *r_ne = ne;
}
/**
* \brief Face Split
*
* Split a face along two vertices. returns the newly made face, and sets
* the \a r_l member to a loop in the newly created edge.
*
* \param bm The bmesh
* \param f the original face
* \param l_a, l_b  Loops of this face, their vertices define
* the split edge to be created (must be differ and not can't be adjacent in the face).
* \param r_l pointer which will receive the BMLoop for the split edge in the new face
* \param example Edge used for attributes of splitting edge, if non-NULL
* \param no_double: Use an existing edge if found
*
* \return Pointer to the newly created face representing one side of the split
* if the split is successful (and the original original face will be the
* other side). NULL if the split fails.
*/
BMFace * BMesh::BM_face_split(BMFace *f, BMLoop *l_a, BMLoop *l_b, BMLoop **r_l, BMEdge *example, const bool no_double)
{
	BMFace *f_new;

	BLI_assert(l_a != l_b);
	BLI_assert(f == l_a->f && f == l_b->f);
	BLI_assert(!BM_loop_is_adjacent(l_a, l_b));

	/* could be an assert */
	if (UNLIKELY(BM_loop_is_adjacent(l_a, l_b)) ||
		UNLIKELY((f != l_a->f || f != l_b->f)))
	{
		if (r_l) {
			*r_l = NULL;
		}
		return NULL;
	}


	f_new = bmesh_sfme(f, l_a, l_b, r_l, example, no_double);

	return f_new;
}

/**
* \brief Edge Split
*
* <pre>
* Before: v
*         +-----------------------------------+
*                           e
*
* After:  v                 v_new (returned)
*         +-----------------+-----------------+
*                 r_e                e
* </pre>
*
* \param e  The edge to split.
* \param v  One of the vertices in \a e and defines the "from" end of the splitting operation,
* the new vertex will be \a fac of the way from \a v to the other end.
* \param r_e  The newly created edge.
* \return  The new vertex.
*/
BMVert * BMesh::BM_edge_split(BMEdge *e, BMVert *v, BMEdge **r_e, float fac)
{
	BMVert *v_new, *v_other;
	BMFace **oldfaces = NULL;
	BMEdge *e_dummy;

	BLI_assert(BM_vert_in_edge(e, v) == true);

	/* we need this for handling multi-res */
	if (!r_e) {
		r_e = &e_dummy;
	}

	v_other = BM_edge_other_vert(e, v);
	v_new = bmesh_semv(v, e, r_e);

	BLI_assert(v_new != NULL);
	BLI_assert(BM_vert_in_edge(*r_e, v) && BM_vert_in_edge(*r_e, v_new));
	BLI_assert(BM_vert_in_edge(e, v_new) && BM_vert_in_edge(e, v_other));

	v_new->co = v_other->co - v->co;
	v_new->co = v->co + v_new->co * fac;

	(*r_e)->head.hflag = e->head.hflag;
	BM_elem_attrs_copy(this, this, e, *r_e);

	/* v->v_new->v2 */
	BM_data_interp_face_vert_edge(v_other, v, v_new, e, fac);
	BM_data_interp_from_verts(v, v_other, v_new, fac);

	return v_new;
}

bool BMesh::BM_edge_collapse_tri_manifold_ok(BMEdge *e)
{
	BMEdge *e_iter;
	e_iter = e;

	if (BM_edge_is_manifold(e) || BM_edge_is_boundary(e)){
		BMLoop *l_f1 = nullptr, *l_f2 = nullptr;
		BMVert *op_v_f1 = nullptr, *op_v_f2 = nullptr;
		BMLoop *l_iter;

		l_f1 = e->l;
		l_f2 = (e->l != e->l->radial_next) ? e->l->radial_next : nullptr;
		
		/*at least one edge must be manifold*/
		if (l_f1){
			bool at_least_manifold_edge = false;
			l_iter = l_f1->next;
			do {
				if (!BM_edge_is_boundary(l_iter->e)){
					at_least_manifold_edge = true;
					break;
				}
			} while ((l_iter = l_iter->next) != l_f1);
			
			if (!at_least_manifold_edge){
				return false;
			}
		}

		if (l_f2){
			bool at_least_manifold_edge = false;
			l_iter = l_f2->next;
			do {
				if (!BM_edge_is_boundary(l_iter->e)){
					at_least_manifold_edge = true;
					break;
				}
			} while ((l_iter = l_iter->next) != l_f2);

			if (!at_least_manifold_edge){
				return false;
			}
		}


		/*check common vertex*/
		if (l_f1){
			op_v_f1 = l_f1->next->next->v;
			BLI_assert(!BM_vert_in_edge(e, op_v_f1));
		}
		if (l_f2){
			op_v_f2 = l_f2->next->next->v;
			BLI_assert(!BM_vert_in_edge(e, op_v_f2));
		}
		do {
			if (e_iter != e){
				BMVert *o_v = BM_edge_other_vert(e_iter, e->v1);
				if (o_v != op_v_f1 && o_v != op_v_f2){
					if (BM_edge_exists(o_v, e->v2)){
						return false;
					}
				}
			}
		} while ((e_iter = bmesh_disk_edge_next(e_iter, e->v1)) != e);

		return true;
	}

	return false;
}

/**
* special, highly limited edge collapse function
* intended for speed over flexibility.
* can only collapse edges connected to (1, 2) tris.
*
* Important - dont add vert/edge/face data on collapsing!
*
* \param r_e_clear_other: Let caller know what edges we remove besides \a e_clear
* \param customdata_flag: Merge factor, scales from 0 - 1 ('v_clear' -> 'v_other')
*/
bool BMesh::BM_edge_tri_collapse(BMEdge *e_clear, BMVert *v_clear)
{
	BMVert *v_other;

	v_other = BM_edge_other_vert(e_clear, v_clear);
	BLI_assert(v_other != NULL);

	if (BM_edge_is_manifold(e_clear)) {
		BMLoop *l_a, *l_b;
		BMEdge *e_a_other[2], *e_b_other[2];

		BM_edge_loop_pair(e_clear, &l_a, &l_b);

		BLI_assert(l_a->f->len == 3);
		BLI_assert(l_b->f->len == 3);

		/* keep 'v_clear' 0th */
		if (BM_vert_in_edge(l_a->prev->e, v_clear)) {
			e_a_other[0] = l_a->prev->e;
			e_a_other[1] = l_a->next->e;
		}
		else {
			e_a_other[1] = l_a->prev->e;
			e_a_other[0] = l_a->next->e;
		}

		if (BM_vert_in_edge(l_b->prev->e, v_clear)) {
			e_b_other[0] = l_b->prev->e;
			e_b_other[1] = l_b->next->e;
		}
		else {
			e_b_other[1] = l_b->prev->e;
			e_b_other[0] = l_b->next->e;
		}

		/* we could assert this case, but better just bail out */
#if 0
		BLI_assert(e_a_other[0] != e_b_other[0]);
		BLI_assert(e_a_other[0] != e_b_other[1]);
		BLI_assert(e_b_other[0] != e_a_other[0]);
		BLI_assert(e_b_other[0] != e_a_other[1]);
#endif
		/* not totally common but we want to avoid */
		if (ELEM(e_a_other[0], e_b_other[0], e_b_other[1]) ||
			ELEM(e_a_other[1], e_b_other[0], e_b_other[1]))
		{
			return false;
		}

		BLI_assert(BM_edge_share_vert(e_a_other[0], e_b_other[0]));
		BLI_assert(BM_edge_share_vert(e_a_other[1], e_b_other[1]));

		BM_edge_kill(e_clear);

		v_other->head.hflag |= v_clear->head.hflag;
		BM_vert_splice(v_other, v_clear);

		e_a_other[1]->head.hflag |= e_a_other[0]->head.hflag;
		e_b_other[1]->head.hflag |= e_b_other[0]->head.hflag;
		BM_edge_splice(e_a_other[1], e_a_other[0]);
		BM_edge_splice(e_b_other[1], e_b_other[0]);

		// BM_mesh_validate(bm);

		return true;
	}
	else if (BM_edge_is_boundary(e_clear)) {
		/* same as above but only one triangle */
		BMLoop *l_a;
		BMEdge *e_a_other[2];

		l_a = e_clear->l;

		BLI_assert(l_a->f->len == 3);

		/* keep 'v_clear' 0th */
		if (BM_vert_in_edge(l_a->prev->e, v_clear)) {
			e_a_other[0] = l_a->prev->e;
			e_a_other[1] = l_a->next->e;
		}
		else {
			e_a_other[1] = l_a->prev->e;
			e_a_other[0] = l_a->next->e;
		}


		BM_edge_kill(e_clear);

		v_other->head.hflag |= v_clear->head.hflag;
		BM_vert_splice(v_other, v_clear);

		e_a_other[1]->head.hflag |= e_a_other[0]->head.hflag;
		BM_edge_splice(e_a_other[1], e_a_other[0]);


		// BM_mesh_validate(bm);

		return true;
	}
	else {
		return false;
	}
}
/*BM_edge_collapse_manifold_ok should be called earlier*/
BMVert* BMesh::BM_edge_collapse(BMEdge *e_kill, BMVert *v_kill, const bool do_del, const bool kill_degenerate_faces)
{
	return bmesh_jvke(e_kill, v_kill, do_del, true, kill_degenerate_faces);
}

BMFace* BMesh::bm_face_create__sfme(BMFace *f_example)
{
	BMFace *f;

	f = bm_face_create__internal();

	BM_elem_attrs_copy(this, this, f_example, f);

	return f;
}

bool BMesh::BM_edge_tri_flip(BMEdge *e, char check_flag)
{
	BMFace *f1, *f2;
	if (BM_edge_face_pair(e, &f1, &f2)){
		if (f1->len == 3 && f2->len == 3){
			BMLoop *l_f1, *l_f2, *l1, *l2;
			BMVert *v1, *v2;

			/* verify that e is in both f1 and f2 */
			if (!((l_f1 = BM_face_edge_share_loop(f1, e)) &&
				(l_f2 = BM_face_edge_share_loop(f2, e)))){
				return false;
			}

			/* validate direction of f2's loop cycle is compatible */
			if (l_f1->v == l_f2->v){
				return false;
			}

			/* the loops will be freed so assign verts */
			BM_edge_calc_rotate(e, true, &l1, &l2);
			v1 = l1->v; v2 = l2->v;

			/* check before applying */
			if (check_flag & BM_EDGEROT_CHECK_EXISTS) {
				if (BM_edge_exists(v1, v2)) {
					return false;
				}
			}

			/* slowest, check last */
			if (check_flag & BM_EDGEROT_CHECK_DEGENERATE) {
				if (!BM_edge_rotate_check_degenerate(e, l1, l2)) {
					return false;
				}
			}

			/* join two triangles*/
			/* what will be removed: l_f1, l_f2 , e, f2 */
			{
				/* join the two loop */
				l_f1->prev->next = l_f2->next;
				l_f2->next->prev = l_f1->prev;

				l_f1->next->prev = l_f2->prev;
				l_f2->prev->next = l_f1->next;

				/* if l_f1 was base-loop, make l_f1->next the base. */
				if (BM_FACE_FIRST_LOOP(f1) == l_f1)
					BM_FACE_FIRST_LOOP(f1) = l_f1->next;

				/*f1->len = 4;*/

				/* make sure each loop points to the proper face */
				size_t i;
				BMLoop *l_iter;
				for (i = 0, l_iter = BM_FACE_FIRST_LOOP(f1); i < 4; i++, l_iter = l_iter->next)
					l_iter->f = f1;

				/* remove edge from the disk cycle of its two vertices */
				bmesh_disk_edge_remove(e, e->v1);
				bmesh_disk_edge_remove(e, e->v2);
			}

			/*now split face*/
			{
				BMLoop *l_iter, *l_first; 
				int first_loop_f1;
				BMLoop *l_v1, *l_v2;
				l_v1 = BM_face_vert_share_loop(f1, v1);
				l_v2 = BM_face_vert_share_loop(f1, v2);

				/* allocate new edge between v1 and v2 */
				memset(&e->v1_disk_link, 0, sizeof(BMDiskLink) * 2);
				e->v1 = v1; e->v2 = v2;
				bmesh_disk_edge_append(e, e->v1);
				bmesh_disk_edge_append(e, e->v2);

				l_f1->v = v2; l_f1->e = e; l_f1->f = f1;
				l_f2->v = v1; l_f2->e = e; l_f2->f = f2;

				l_f1->prev = l_v2->prev;
				l_f2->prev = l_v1->prev;
				l_v2->prev->next = l_f1;
				l_v1->prev->next = l_f2;

				l_f1->next = l_v1;
				l_f2->next = l_v2;
				l_v1->prev = l_f1;
				l_v2->prev = l_f2;

				/* find which of the faces the original first loop is in */
				l_iter = l_first = l_f1;
				first_loop_f1 = 0;
				do {
					if (l_iter == f1->l_first)
						first_loop_f1 = 1;
				} while ((l_iter = l_iter->next) != l_first);

				if (first_loop_f1) {
					/* original first loop was in f1, find a suitable first loop for f2
					* which is as similar as possible to f1. the order matters for tools
					* such as duplifaces. */
					if (f1->l_first->prev == l_f1)
						f2->l_first = l_f2->prev;
					else if (f1->l_first->next == l_f1)
						f2->l_first = l_f2->next;
					else
						f2->l_first = l_f2;
				}
				else {
					/* original first loop was in f2, further do same as above */
					f2->l_first = f1->l_first;

					if (f1->l_first->prev == l_f2)
						f1->l_first = l_f1->prev;
					else if (f1->l_first->next == l_f2)
						f1->l_first = l_f1->next;
					else
						f1->l_first = l_f1;
				}


				/*f1->len = 3;f2->len = 3;*/

				/* go through all of f2's loops and make sure they point to it properly */
				l_iter = l_first = BM_FACE_FIRST_LOOP(f2);
				do {
					l_iter->f = f2;
				} while ((l_iter = l_iter->next) != l_first);

				/* validate both loop */
				/* I don't know how many loops are supposed to be in each face at this point! FIXME */

				/* link up the new loops into the new edges radial */
				bmesh_radial_append(e, l_f1);
				bmesh_radial_append(e, l_f2);

				BM_CHECK_ELEMENT(e);
				BM_CHECK_ELEMENT(f1);
				BM_CHECK_ELEMENT(f2);

				_elem_index_dirty |= BM_VERT | BM_EDGE | BM_FACE;
				_elem_table_dirty |= BM_VERT | BM_EDGE | BM_FACE;

				return true;
			}
		}
	}

	return false;
}

void BMesh::BM_edge_calc_rotate(BMEdge *e, const bool ccw, BMLoop **r_l1, BMLoop **r_l2)
{
	BMVert *v1, *v2;
	BMFace *fa, *fb;

	/* this should have already run */
	BLI_assert(BM_edge_rotate_check(e) == true);

	/* we know this will work */
	BM_edge_face_pair(e, &fa, &fb);

	/* so we can use ccw variable correctly,
	* otherwise we could use the edges verts direct */
	BM_edge_ordered_verts(e, &v1, &v2);

	/* we could swap the verts _or_ the faces, swapping faces
	* gives more predictable results since that way the next vert
	* just stitches from face fa / fb */
	if (!ccw) {
		std::swap(fa, fb);
	}

	*r_l1 = BM_face_other_vert_loop(fb, v2, v1);
	*r_l2 = BM_face_other_vert_loop(fa, v1, v2);
}

/**
* \brief Check if Edge Rotate Gives Degenerate Faces
*
* Check 2 cases
* 1) does the newly forms edge form a flipped face (compare with previous cross product)
* 2) does the newly formed edge cause a zero area corner (or close enough to be almost zero)
*
* \param e The edge to test rotation.
* \param l1,l2 are the loops of the proposed verts to rotate too and should
* be the result of calling #BM_edge_calc_rotate
*/
bool BMesh::BM_edge_rotate_check_degenerate(BMEdge *e, BMLoop *l1, BMLoop *l2)
{
	/* note: for these vars 'old' just means initial edge state. */

	Vector3f ed_dir_old; /* edge vector */
	Vector3f ed_dir_new; /* edge vector */
	Vector3f ed_dir_new_flip; /* edge vector */

	Vector3f ed_dir_v1_old;
	Vector3f ed_dir_v2_old;

	Vector3f ed_dir_v1_new;
	Vector3f ed_dir_v2_new;

	Vector3f cross_old;
	Vector3f cross_new;

	/* original verts - these will be in the edge 'e' */
	BMVert *v1_old, *v2_old;

	/* verts from the loops passed */

	BMVert *v1, *v2;
	/* these are the opposite verts - the verts that _would_ be used if 'ccw' was inverted*/
	BMVert *v1_alt, *v2_alt;

	/* this should have already run */
	BLI_assert(BM_edge_rotate_check(e) == true);

	BM_edge_ordered_verts(e, &v1_old, &v2_old);

	v1 = l1->v;
	v2 = l2->v;

	/* get the next vert along */
	v1_alt = BM_face_other_vert_loop(l1->f, v1_old, v1)->v;
	v2_alt = BM_face_other_vert_loop(l2->f, v2_old, v2)->v;

	/* normalize all so comparisons are scale independent */

	BLI_assert(BM_edge_exists(v1_old, v1));
	BLI_assert(BM_edge_exists(v1, v1_alt));

	BLI_assert(BM_edge_exists(v2_old, v2));
	BLI_assert(BM_edge_exists(v2, v2_alt));

	/* old and new edge vecs */
	ed_dir_old = (v1_old->co - v2_old->co).normalized();
	ed_dir_new = (v1->co - v2->co).normalized();

	/* old edge corner vecs */
	ed_dir_v1_old = (v1_old->co - v1->co).normalized();
	ed_dir_v2_old = (v2_old->co - v2->co).normalized();

	/* old edge corner vecs */
	ed_dir_v1_new = (v1->co - v1_alt->co).normalized();
	ed_dir_v2_new = (v2->co - v2_alt->co).normalized();

	/* compare */
	cross_old = ed_dir_old.cross(ed_dir_v1_old);
	cross_new = ed_dir_new.cross(ed_dir_v1_new);
	if (cross_old.dot(cross_new) < 0.0f) { /* does this flip? */
		return false;
	}
	cross_old = ed_dir_old.cross(ed_dir_v2_old);
	cross_new = ed_dir_new.cross(ed_dir_v2_new);
	if (cross_old.dot(cross_new) < 0.0f) { /* does this flip? */
		return false;
	}

	ed_dir_new_flip = -ed_dir_new;

	/* result is zero area corner */
	if ((ed_dir_new.dot(ed_dir_v1_new) > 0.999f) ||
		(ed_dir_new_flip.dot(ed_dir_v2_new) > 0.999f))
	{
		return false;
	}

	return true;
}

/**
* \brief Split Face Make Edge (SFME)
*
* \warning this is a low level function, most likely you want to use #BM_face_split()
*
* Takes as input two vertices in a single face. An edge is created which divides the original face
* into two distinct regions. One of the regions is assigned to the original face and it is closed off.
* The second region has a new face assigned to it.
*
* \par Examples:
* <pre>
*     Before:               After:
*      +--------+           +--------+
*      |        |           |        |
*      |        |           |   f1   |
*     v1   f1   v2          v1======v2
*      |        |           |   f2   |
*      |        |           |        |
*      +--------+           +--------+
* </pre>
*
* \note the input vertices can be part of the same edge. This will
* result in a two edged face. This is desirable for advanced construction
* tools and particularly essential for edge bevel. Because of this it is
* up to the caller to decide what to do with the extra edge.
*
* \note If \a holes is NULL, then both faces will lose
* all holes from the original face.  Also, you cannot split between
* a hole vert and a boundary vert; that case is handled by higher-
* level wrapping functions (when holes are fully implemented, anyway).
*
* \note that holes represents which holes goes to the new face, and of
* course this requires removing them from the existing face first, since
* you cannot have linked list links inside multiple lists.
*
* \return A BMFace pointer
*/
BMFace * BMesh::bmesh_sfme(BMFace *f, BMLoop *l_v1, BMLoop *l_v2, BMLoop **r_l, BMEdge *e_example, const bool no_double)
{
	int first_loop_f1;

	BMFace *f2;
	BMLoop *l_iter, *l_first;
	BMLoop *l_f1 = NULL, *l_f2 = NULL;
	BMEdge *e;
	BMVert *v1 = l_v1->v, *v2 = l_v2->v;
	int f1len, f2len;

	BLI_assert(f == l_v1->f && f == l_v2->f);

	/* allocate new edge between v1 and v2 */
	e = BM_edge_create(v1, v2, e_example, no_double ? BM_CREATE_NO_DOUBLE : BM_CREATE_NOP);

	f2 = bm_face_create__sfme(f);
	l_f1 = bm_loop_create(v2, e, f,  l_v2, BM_CREATE_NOP);
	l_f2 = bm_loop_create(v1, e, f2, l_v1, BM_CREATE_NOP);

	l_f1->prev = l_v2->prev;
	l_f2->prev = l_v1->prev;
	l_v2->prev->next = l_f1;
	l_v1->prev->next = l_f2;

	l_f1->next = l_v1;
	l_f2->next = l_v2;
	l_v1->prev = l_f1;
	l_v2->prev = l_f2;

	/* find which of the faces the original first loop is in */
	l_iter = l_first = l_f1;
	first_loop_f1 = 0;
	do {
		if (l_iter == f->l_first)
			first_loop_f1 = 1;
	} while ((l_iter = l_iter->next) != l_first);

	if (first_loop_f1) {
		/* original first loop was in f1, find a suitable first loop for f2
		* which is as similar as possible to f1. the order matters for tools
		* such as duplifaces. */
		if (f->l_first->prev == l_f1)
			f2->l_first = l_f2->prev;
		else if (f->l_first->next == l_f1)
			f2->l_first = l_f2->next;
		else
			f2->l_first = l_f2;
	}
	else {
		/* original first loop was in f2, further do same as above */
		f2->l_first = f->l_first;

		if (f->l_first->prev == l_f2)
			f->l_first = l_f1->prev;
		else if (f->l_first->next == l_f2)
			f->l_first = l_f1->next;
		else
			f->l_first = l_f1;
	}

	/* validate both loop */
	/* I don't know how many loops are supposed to be in each face at this point! FIXME */

	/* go through all of f2's loops and make sure they point to it properly */
	l_iter = l_first = BM_FACE_FIRST_LOOP(f2);
	f2len = 0;
	do {
		l_iter->f = f2;
		f2len++;
	} while ((l_iter = l_iter->next) != l_first);

	/* link up the new loops into the new edges radial */
	bmesh_radial_append(e, l_f1);
	bmesh_radial_append(e, l_f2);

	f2->len = f2len;

	f1len = 0;
	l_iter = l_first = BM_FACE_FIRST_LOOP(f);
	do {
		f1len++;
	} while ((l_iter = l_iter->next) != l_first);

	f->len = f1len;

	if (r_l) *r_l = l_f2;

	BM_CHECK_ELEMENT(e);
	BM_CHECK_ELEMENT(f);
	BM_CHECK_ELEMENT(f2);

	return f2;
}

/**
* \brief Split Edge Make Vert (SEMV)
*
* Takes \a e edge and splits it into two, creating a new vert.
* \a tv should be one end of \a e : the newly created edge
* will be attached to that end and is returned in \a r_e.
*
* \par Examples:
*
* <pre>
*                     E
*     Before: OV-------------TV
*                 E       RE
*     After:  OV------NV-----TV
* </pre>
*
* \return The newly created BMVert pointer.
*/
BMVert* BMesh::bmesh_semv(BMVert *tv, BMEdge *e, BMEdge **r_e)
{
	BMLoop *l_next;
	BMEdge *e_new;
	BMVert *v_new, *v_old;
#ifndef NDEBUG
	int valence1, valence2;
	bool edok;
	int i;
#endif

	BLI_assert(BM_vert_in_edge(e, tv) != false);

	v_old = BM_edge_other_vert(e, tv);

#ifndef NDEBUG
	valence1 = bmesh_disk_count(v_old);
	valence2 = bmesh_disk_count(tv);
#endif

	/* order of 'e_new' verts should match 'e'
	* (so extruded faces don't flip) */
	v_new = BM_vert_create(tv->co, tv, BM_CREATE_NOP);
	e_new = BM_edge_create(tv, v_new, e, BM_CREATE_NOP);

	bmesh_disk_edge_remove(e_new, tv);
	bmesh_disk_edge_remove(e_new, v_new);

	bmesh_disk_vert_replace(e, v_new, tv);

	/* add e_new to v_new's disk cycle */
	bmesh_disk_edge_append(e_new, v_new);

	/* add e_new to tv's disk cycle */
	bmesh_disk_edge_append(e_new, tv);

#ifndef NDEBUG
	/* verify disk cycles */
	edok = bmesh_disk_validate(valence1, v_old->e, v_old);
	BMESH_ASSERT(edok != false);
	edok = bmesh_disk_validate(valence2, tv->e, tv);
	BMESH_ASSERT(edok != false);
	edok = bmesh_disk_validate(2, v_new->e, v_new);
	BMESH_ASSERT(edok != false);
#endif

	/* Split the radial cycle if present */
	l_next = e->l;
	e->l = NULL;
	if (l_next) {
		BMLoop *l_new, *l;
#ifndef NDEBUG
		int radlen = bmesh_radial_length(l_next);
#endif
		int first1 = 0, first2 = 0;

		/* Take the next loop. Remove it from radial. Split it. Append to appropriate radials */
		while (l_next) {
			l = l_next;
			l->f->len++;
			l_next = l_next != l_next->radial_next ? l_next->radial_next : NULL;
			bmesh_radial_loop_remove(l, NULL);

			l_new = bm_loop_create(NULL, NULL, l->f, l, BM_CREATE_NOP);
			l_new->prev = l;
			l_new->next = (l->next);
			l_new->prev->next = l_new;
			l_new->next->prev = l_new;
			l_new->v = v_new;

			/* assign the correct edge to the correct loop */
			if (BM_verts_in_edge(l_new->v, l_new->next->v, e)) {
				l_new->e = e;
				l->e = e_new;

				/* append l into e_new's rad cycle */
				if (!first1) {
					first1 = 1;
					l->radial_next = l->radial_prev = NULL;
				}

				if (!first2) {
					first2 = 1;
					l->radial_next = l->radial_prev = NULL;
				}

				bmesh_radial_append(l_new->e, l_new);
				bmesh_radial_append(l->e, l);
			}
			else if (BM_verts_in_edge(l_new->v, l_new->next->v, e_new)) {
				l_new->e = e_new;
				l->e = e;

				/* append l into e_new's rad cycle */
				if (!first1) {
					first1 = 1;
					l->radial_next = l->radial_prev = NULL;
				}

				if (!first2) {
					first2 = 1;
					l->radial_next = l->radial_prev = NULL;
				}

				bmesh_radial_append(l_new->e, l_new);
				bmesh_radial_append(l->e, l);
			}

		}

#ifndef NDEBUG
		/* verify length of radial cycle */
		edok = bmesh_radial_validate(radlen, e->l);
		BMESH_ASSERT(edok != false);
		edok = bmesh_radial_validate(radlen, e_new->l);
		BMESH_ASSERT(edok != false);

		/* verify loop->v and loop->next->v pointers for e */
		for (i = 0, l = e->l; i < radlen; i++, l = l->radial_next) {
			BMESH_ASSERT(l->e == e);
			//BMESH_ASSERT(l->radial_next == l);
			BMESH_ASSERT(!(l->prev->e != e_new && l->next->e != e_new));

			edok = BM_verts_in_edge(l->v, l->next->v, e);
			BMESH_ASSERT(edok != false);
			BMESH_ASSERT(l->v != l->next->v);
			BMESH_ASSERT(l->e != l->next->e);

			/* verify loop cycle for kloop->f */
#ifdef USE_BMHEAD_LOOP
			BM_CHECK_ELEMENT(l);
#endif
			BM_CHECK_ELEMENT(l->v);
			BM_CHECK_ELEMENT(l->e);
			BM_CHECK_ELEMENT(l->f);
		}
		/* verify loop->v and loop->next->v pointers for e_new */
		for (i = 0, l = e_new->l; i < radlen; i++, l = l->radial_next) {
			BMESH_ASSERT(l->e == e_new);
			// BMESH_ASSERT(l->radial_next == l);
			BMESH_ASSERT(!(l->prev->e != e && l->next->e != e));
			edok = BM_verts_in_edge(l->v, l->next->v, e_new);
			BMESH_ASSERT(edok != false);
			BMESH_ASSERT(l->v != l->next->v);
			BMESH_ASSERT(l->e != l->next->e);

#ifdef USE_BMHEAD_LOOP
			BM_CHECK_ELEMENT(l);
#endif
			BM_CHECK_ELEMENT(l->v);
			BM_CHECK_ELEMENT(l->e);
			BM_CHECK_ELEMENT(l->f);
		}
#endif
	}

	BM_CHECK_ELEMENT(e_new);
	BM_CHECK_ELEMENT(v_new);
	BM_CHECK_ELEMENT(v_old);
	BM_CHECK_ELEMENT(e);
	BM_CHECK_ELEMENT(tv);

	if (r_e) *r_e = e_new;
	return v_new;
}


/**
* \brief Join Vert Kill Edge (JVKE)
*
* Collapse an edge, merging surrounding data.
*
* Unlike #BM_vert_collapse_edge & #bmesh_jekv which only handle 2 valence verts,
* this can handle any number of connected edges/faces.
*
* <pre>
* Before: -> After:
* +-+-+-+    +-+-+-+
* | | | |    | \ / |
* +-+-+-+    +--+--+
* | | | |    | / \ |
* +-+-+-+    +-+-+-+
* </pre>
*/


BMVert * BMesh::bmesh_jvke(BMEdge *e_kill, BMVert *v_kill, const bool do_del, const bool check_edge_double, const bool kill_degenerate_faces)
{
	boost::container::small_vector<BMFace*, 4> faces_degenerate;
	BMVert *v_target = BM_edge_other_vert(e_kill, v_kill);

	BLI_assert(BM_vert_in_edge(e_kill, v_kill));

	if (e_kill->l) {
		BMLoop *l_kill, *l_first, *l_kill_next;
		l_kill = l_first = e_kill->l;
		do {
			/* relink loops and fix vertex pointer */
			if (l_kill->next->v == v_kill) {
				l_kill->next->v = v_target;
			}

			l_kill->next->prev = l_kill->prev;
			l_kill->prev->next = l_kill->next;
			if (BM_FACE_FIRST_LOOP(l_kill->f) == l_kill) {
				BM_FACE_FIRST_LOOP(l_kill->f) = l_kill->next;
			}

			/* fix len attribute of face */
			l_kill->f->len--;
			if (kill_degenerate_faces) {
				if (l_kill->f->len < 3) {
					faces_degenerate.push_back(l_kill->f);
				}
			}
			l_kill_next = l_kill->radial_next;

			bm_kill_only_loop(l_kill);

		} while ((l_kill = l_kill_next) != l_first);

		e_kill->l = NULL;
	}

	BM_edge_kill(e_kill);
	BM_CHECK_ELEMENT(v_kill);
	BM_CHECK_ELEMENT(v_target);

	if (v_target->e && v_kill->e) {
		/* inline BM_vert_splice(bm, v_target, v_kill); */
		BMEdge *e;
		while ((e = v_kill->e)) {
			BMEdge *e_target = nullptr;

			if (check_edge_double) {
				e_target = BM_edge_exists(v_target, BM_edge_other_vert(e, v_kill));
			}

			bmesh_edge_vert_swap(e, v_target, v_kill);
			BLI_assert(e->v1 != e->v2);

			if (check_edge_double) {
				if (e_target) {
					BM_edge_splice(e_target, e);
				}
			}
		}
	}

	if (kill_degenerate_faces) {
		BMFace *f_kill;
		while (!faces_degenerate.empty()){
			f_kill = faces_degenerate.back();
			faces_degenerate.pop_back();
			BM_face_kill(f_kill, false);
		}
	}

	if (do_del) {
		BLI_assert(v_kill->e == NULL);
		bm_kill_only_vert(v_kill, false);
	}

	return v_target;
}

/**
* \brief Splice Edge
*
* Splice two unique edges which share the same two vertices into one edge.
*  (\a e_src into \a e_dst, removing e_src).
*
* \return Success
*
* \note Edges must already have the same vertices.
*/
bool BMesh::BM_edge_splice(BMEdge *e_dst, BMEdge *e_src)
{
	BMLoop *l;

	if (!BM_vert_in_edge(e_src, e_dst->v1) || !BM_vert_in_edge(e_src, e_dst->v2)) {
		/* not the same vertices can't splice */

		/* the caller should really make sure this doesn't happen ever
		* so assert on release builds */
		BLI_assert(0);

		return false;
	}

	while (e_src->l) {
		l = e_src->l;
		BLI_assert(BM_vert_in_edge(e_dst, l->v));
		BLI_assert(BM_vert_in_edge(e_dst, l->next->v));
		bmesh_radial_loop_remove(l, e_src);
		bmesh_radial_append(e_dst, l);
	}

	BLI_assert(bmesh_radial_length(e_src->l) == 0);

	BM_CHECK_ELEMENT(e_src);
	BM_CHECK_ELEMENT(e_dst);

	/* removes from disks too */
	BM_edge_kill(e_src);

	return true;
}



/**
* \brief Splice Vert
*
* Merges two verts into one
* (\a v_src into \a v_dst, removing \a v_src).
*
* \return Success
*
* \warning This doesn't work for collapsing edges,
* where \a v and \a vtarget are connected by an edge
* (assert checks for this case).
*/
bool BMesh::BM_vert_splice(BMVert *v_dst, BMVert *v_src)
{
	BMEdge *e;

	/* verts already spliced */
	if (v_src == v_dst) {
		return false;
	}

	BLI_assert(BM_vert_pair_share_face_check(v_src, v_dst) == false);

	/* move all the edges from 'v_src' disk to 'v_dst' */
	while ((e = v_src->e)) {
		bmesh_edge_vert_swap(e, v_dst, v_src);
		BLI_assert(e->v1 != e->v2);
	}

	BM_CHECK_ELEMENT(v_src);
	BM_CHECK_ELEMENT(v_dst);

	/* 'v_src' is unused now, and can be killed */
	BM_vert_kill(v_src, false);

	return true;
}
/**
* \brief Join Face Kill Edge (JFKE)
*
* Takes two faces joined by a single 2-manifold edge and fuses them together.
* The edge shared by the faces must not be connected to any other edges which have
* Both faces in its radial cycle
*
* \par Examples:
* <pre>
*           A                   B
*      +--------+           +--------+
*      |        |           |        |
*      |   f1   |           |   f1   |
*     v1========v2 = Ok!    v1==V2==v3 == Wrong!
*      |   f2   |           |   f2   |
*      |        |           |        |
*      +--------+           +--------+
* </pre>
*
* In the example A, faces \a f1 and \a f2 are joined by a single edge,
* and the euler can safely be used.
* In example B however, \a f1 and \a f2 are joined by multiple edges and will produce an error.
* The caller in this case should call #bmesh_jekv on the extra edges
* before attempting to fuse \a f1 and \a f2.
*
* \note The order of arguments decides whether or not certain per-face attributes are present
* in the resultant face. For instance vertex winding, material index, smooth flags, etc are inherited
* from \a f1, not \a f2.
*
* \return A BMFace pointer
*/
BMFace * BMesh::bmesh_jfke(BMFace *f1, BMFace *f2, BMEdge *e)
{
	BMLoop *l_iter, *l_f1 = NULL, *l_f2 = NULL;
	int newlen = 0, i, f1len = 0, f2len = 0;
	/* can't join a face to itself */
	if (f1 == f2) {
		return NULL;
	}

	/* validate that edge is 2-manifold edge */
	if (!BM_edge_is_manifold(e)) {
		return NULL;
	}

	/* verify that e is in both f1 and f2 */
	f1len = f1->len;
	f2len = f2->len;

	if (!((l_f1 = BM_face_edge_share_loop(f1, e)) &&
		(l_f2 = BM_face_edge_share_loop(f2, e))))
	{
		return NULL;
	}

	/* validate direction of f2's loop cycle is compatible */
	if (l_f1->v == l_f2->v) {
		return NULL;
	}

	/* validate that for each face, each vertex has another edge in its disk cycle that is
	* not e, and not shared. */
	if (BM_edge_in_face(l_f1->next->e, f2) ||
		BM_edge_in_face(l_f1->prev->e, f2) ||
		BM_edge_in_face(l_f2->next->e, f1) ||
		BM_edge_in_face(l_f2->prev->e, f1))
	{
		return NULL;
	}

	/* validate only one shared edge */
	if (BM_face_share_edge_count(f1, f2) > 1) {
		return NULL;
	}

	/* validate no internal join */
	for (i = 0, l_iter = BM_FACE_FIRST_LOOP(f1); i < f1len; i++, l_iter = l_iter->next) {
		BM_elem_flag_disable(l_iter->v, (char)BM_ELEM_INTERNAL_TAG);
	}
	for (i = 0, l_iter = BM_FACE_FIRST_LOOP(f2); i < f2len; i++, l_iter = l_iter->next) {
		BM_elem_flag_disable(l_iter->v, (char)BM_ELEM_INTERNAL_TAG);
	}

	for (i = 0, l_iter = BM_FACE_FIRST_LOOP(f1); i < f1len; i++, l_iter = l_iter->next) {
		if (l_iter != l_f1) {
			BM_elem_flag_enable(l_iter->v, (char)BM_ELEM_INTERNAL_TAG);
		}
	}
	for (i = 0, l_iter = BM_FACE_FIRST_LOOP(f2); i < f2len; i++, l_iter = l_iter->next) {
		if (l_iter != l_f2) {
			/* as soon as a duplicate is found, bail out */
			if (BM_elem_flag_test(l_iter->v, (char)BM_ELEM_INTERNAL_TAG)) {
				return NULL;
			}
		}
	}

	/* join the two loop */
	l_f1->prev->next = l_f2->next;
	l_f2->next->prev = l_f1->prev;

	l_f1->next->prev = l_f2->prev;
	l_f2->prev->next = l_f1->next;

	/* if l_f1 was baseloop, make l_f1->next the base. */
	if (BM_FACE_FIRST_LOOP(f1) == l_f1)
		BM_FACE_FIRST_LOOP(f1) = l_f1->next;

	/* increase length of f1 */
	f1->len += (f2->len - 2);

	/* make sure each loop points to the proper face */
	newlen = f1->len;
	for (i = 0, l_iter = BM_FACE_FIRST_LOOP(f1); i < newlen; i++, l_iter = l_iter->next)
		l_iter->f = f1;

	/* remove edge from the disk cycle of its two vertices */
	bmesh_disk_edge_remove(l_f1->e, l_f1->e->v1);
	bmesh_disk_edge_remove(l_f1->e, l_f1->e->v2);

	/* deallocate edge and its two loops as well as f2 */
	//if (bm->etoolflagpool) {
	//	BLI_mempool_free(bm->etoolflagpool, l_f1->e->oflags);
	//}

	bm_kill_only_edge(l_f1->e);
	bm_kill_only_loop(l_f1);
	bm_kill_only_loop(l_f2);

	//if (bm->ftoolflagpool) {
	//	BLI_mempool_free(bm->ftoolflagpool, f2->oflags);
	//}
	bm_kill_only_face(f2, false);
	
	/* account for both above */
	_elem_index_dirty |= BM_EDGE | BM_LOOP | BM_FACE;

	BM_CHECK_ELEMENT(f1);

	/* validate the new loop cycle */
	BMESH_ASSERT(bmesh_loop_validate(f1) != false);

	return f1;
}

void  BMesh::BM_data_layer_add_named(char htype, int type, const char *name)
{
	switch (htype)
	{
	case BM_VERT:
		BM_data_layer_add_named(&_vert_data, type, name);
		break;
	case BM_FACE:
		BM_data_layer_add_named(&_face_data, type, name);
		break;
	case BM_EDGE:
		BM_data_layer_add_named(&_edge_data, type, name);
		break;
	default:
		BLI_assert(0);
		break;
	}
}
void  BMesh::BM_data_layer_free(char htype, int type)
{
	switch (htype)
	{
	case BM_VERT:
		BM_data_layer_free(&_vert_data, type);
		break;
	case BM_FACE:
		BM_data_layer_free(&_face_data, type);
		break;
	case BM_EDGE:
		BM_data_layer_free(&_edge_data, type);
		break;
	default:
		BLI_assert(0);
		break;
	}
}
void  BMesh::BM_data_layer_free_n(char htype, int type, int n)
{
	switch (htype)
	{
	case BM_VERT:
		BM_data_layer_free_n(&_vert_data, type, n);
		break;
	case BM_FACE:
		BM_data_layer_free_n(&_face_data, type, n);
		break;
	case BM_EDGE:
		BM_data_layer_free_n(&_edge_data, type, n);
		break;
	default:
		BLI_assert(0);
		break;
	}
}
void  BMesh::BM_data_layer_copy(char htype, int type, int src_n, int dst_n)
{
	switch (htype)
	{
	case BM_VERT:
		BM_data_layer_copy(&_vert_data, type, src_n, dst_n);
		break;
	case BM_FACE:
		BM_data_layer_copy(&_face_data, type, src_n, dst_n);
		break;
	case BM_EDGE:
		BM_data_layer_copy(&_edge_data, type, src_n, dst_n);
		break;
	default:
		BLI_assert(0);
		break;
	}
}

int BMesh::BM_data_get_n_offset(char htype, int type, int n)
{
	switch (htype)
	{
	case BM_VERT:
		return _vert_data.CustomData_get_n_offset(type, n);
		break;
	case BM_FACE:
		return _face_data.CustomData_get_n_offset(type, n);
		break;
	case BM_EDGE:
		return _edge_data.CustomData_get_n_offset(type, n);
		break;
	default:
		BLI_assert(0);
		return -1;
		break;
	}
}

int	BMesh::BM_data_get_offset(char htype, int type)
{
	switch (htype)
	{
	case BM_VERT:
		return _vert_data.CustomData_get_offset(type);
		break;
	case BM_FACE:
		return _face_data.CustomData_get_offset(type);
		break;
	case BM_EDGE:
		return _edge_data.CustomData_get_offset(type);
		break;
	default:
		BLI_assert(0);
		return -1;
		break;
	}
}

void BMesh::BM_data_layer_copy(CustomData *data, int type, int src_n, int dst_n)
{
	BMIter iter;

	if (&_vert_data == data) {
		BMVert *eve;

		BM_ITER_MESH(eve, &iter, this, BM_VERTS_OF_MESH) {
			void *ptr = data->CustomData_bmesh_get_n(eve->head.data, type, src_n);
			data->CustomData_bmesh_set_n(eve->head.data, type, dst_n, ptr);
		}
	}
	else if (&_edge_data == data) {
		BMEdge *eed;

		BM_ITER_MESH(eed, &iter, this, BM_EDGES_OF_MESH) {
			void *ptr = data->CustomData_bmesh_get_n(eed->head.data, type, src_n);
			data->CustomData_bmesh_set_n(eed->head.data, type, dst_n, ptr);
		}
	}
	else if (&_face_data == data) {
		BMFace *efa;

		BM_ITER_MESH(efa, &iter, this, BM_FACES_OF_MESH) {
			void *ptr = data->CustomData_bmesh_get_n(efa->head.data, type, src_n);
			data->CustomData_bmesh_set_n(efa->head.data, type, dst_n, ptr);
		}
	}
#ifdef USE_BMLOOP_HEAD
	else if (&_loop_data == data) {
		BMIter liter;
		BMFace *efa;
		BMLoop *l;

		BM_ITER_MESH(efa, &iter, this, BM_FACES_OF_MESH) {
			BM_ITER_ELEM(l, &liter, efa, BM_LOOPS_OF_FACE) {
				void *ptr = data->CustomData_bmesh_get_n(l->head.data, type, src_n);
				data->CustomData_bmesh_set_n(l->head.data, type, dst_n, ptr);
			}
		}
	}
#endif
	else {
		/* should never reach this! */
		BLI_assert(0);
	}
}


void BMesh::BM_data_layer_add_named(CustomData *data, int type, const char *name)
{
	CustomData olddata(data->layers, data->pool);

	/* the pool is now owned by olddata and must not be shared */
	data->pool = NULL;
	data->CustomData_add_layer_named(type, NULL, 0, name);

	update_data_blocks(&olddata, data);
}

int	BMesh::BM_data_get_named_layer_index(char htype, int type, const char *name)
{
	switch (htype)
	{
	case BM_VERT:
		return _vert_data.CustomData_get_named_layer_index(type, name);
		break;
	case BM_FACE:
		return _face_data.CustomData_get_named_layer_index(type, name);
		break;
	case BM_EDGE:
		return _edge_data.CustomData_get_named_layer_index(type, name);
		break;
	default:
		BLI_assert(0);
		return -1;
		break;
	}
}

int BMesh::BM_data_get_layer_index(char htype, int type)
{
	switch (htype)
	{
	case BM_VERT:
		return _vert_data.CustomData_get_layer_index(type);
		break;
	case BM_FACE:
		return _face_data.CustomData_get_layer_index(type);
		break;
	case BM_EDGE:
		return _edge_data.CustomData_get_layer_index(type);
		break;
	default:
		BLI_assert(0);
		return -1;
		break;
	}
}
void BMesh::update_data_blocks(CustomData *olddata, CustomData *data)
{
	BMIter iter;
	void *block;

	if (data == &_vert_data) {
		BMVert *eve;

		data->init_pool();

		BM_ITER_MESH(eve, &iter, this, BM_VERTS_OF_MESH) {
			block = NULL;
			data->CustomData_bmesh_set_default(&block);
			data->CustomData_bmesh_copy_data(olddata, data, eve->head.data, &block);
			olddata->CustomData_bmesh_free_block(&eve->head.data);
			eve->head.data = block;
		}
	}
	else if (data == &_edge_data) {
		BMEdge *eed;

		data->init_pool();

		BM_ITER_MESH(eed, &iter, this, BM_EDGES_OF_MESH) {
			block = NULL;
			data->CustomData_bmesh_set_default(&block);
			data->CustomData_bmesh_copy_data(olddata, data, eed->head.data, &block);
			olddata->CustomData_bmesh_free_block(&eed->head.data);
			eed->head.data = block;
		}
	}
	else if (data == &_face_data) {
		BMFace *efa;

		data->init_pool();

		BM_ITER_MESH(efa, &iter, this, BM_FACES_OF_MESH) {
			block = NULL;
			data->CustomData_bmesh_set_default(&block);
			data->CustomData_bmesh_copy_data(olddata, data, efa->head.data, &block);
			olddata->CustomData_bmesh_free_block(&efa->head.data);
			efa->head.data = block;
		}
	}
	else {
		/* should never reach this! */
		BLI_assert(0);
	}
}


void BMesh::BM_data_layer_free(CustomData *data, int type)
{
	bool has_layer;
	CustomData olddata(data->layers, data->pool);

	/* the pool is now owned by olddata and must not be shared */
	data->pool = NULL;

	has_layer = data->CustomData_free_layer_active(type, 0);
	/* assert because its expensive to realloc - better not do if layer isnt present */
	BLI_assert(has_layer != false);

	update_data_blocks(&olddata, data);
}

void BMesh::BM_data_layer_free_n(CustomData *data, int type, int n)
{
	CustomData olddata;
	bool has_layer;

	olddata = *data;
	olddata.layers = olddata.layers;

	/* the pool is now owned by olddata and must not be shared */
	data->pool = NULL;

	has_layer = data->CustomData_free_layer(type, 0, data->CustomData_get_layer_index_n(type, n));
	/* assert because its expensive to realloc - better not do if layer isnt present */
	BLI_assert(has_layer != false);

	update_data_blocks(&olddata, data);
}


void BMesh::bm_vert_attrs_copy(
	const BMesh *source_mesh, BMesh *target_mesh,
	const BMVert *source_vertex, BMVert *target_vertex)
{
	if ((source_mesh == target_mesh) && (source_vertex == target_vertex)) {
		BLI_assert(!"BMVert: source and targer match");
		return;
	}
	target_vertex->no = source_vertex->no;
	target_mesh->_vert_data.CustomData_bmesh_free_block_data(target_vertex->head.data);
	CustomData::CustomData_bmesh_copy_data(&source_mesh->_vert_data, &target_mesh->_vert_data, source_vertex->head.data, &target_vertex->head.data);
}

void BMesh::bm_edge_attrs_copy(
	const BMesh *source_mesh, BMesh *target_mesh,
	const BMEdge *source_edge, BMEdge *target_edge)
{
	if ((source_mesh == target_mesh) && (source_edge == target_edge)) {
		BLI_assert(!"BMEdge: source and targer match");
		return;
	}
	target_mesh->_edge_data.CustomData_bmesh_free_block_data(target_edge->head.data);
	CustomData::CustomData_bmesh_copy_data(&source_mesh->_edge_data, &target_mesh->_edge_data, source_edge->head.data, &target_edge->head.data);
}

void BMesh::bm_loop_attrs_copy(
	const BMesh *source_mesh, BMesh *target_mesh,
	const BMLoop *source_loop, BMLoop *target_loop)
{
#ifdef USE_BMLOOP_HEAD
	if ((source_mesh == target_mesh) && (source_loop == target_loop)) {
		BLI_assert(!"BMLoop: source and targer match");
		return;
	}
	target_mesh->_loop_data.CustomData_bmesh_free_block_data(target_loop->head.data);
	CustomData::CustomData_bmesh_copy_data(&source_mesh->_loop_data, &target_mesh->_loop_data, source_loop->head.data, &target_loop->head.data);
#endif
}

void BMesh::bm_face_attrs_copy(
	const BMesh *source_mesh, BMesh *target_mesh,
	const BMFace *source_face, BMFace *target_face)
{
	if ((source_mesh == target_mesh) && (source_face == target_face)) {
		BLI_assert(!"BMFace: source and targer match");
		return;
	}
	target_face->no = source_face->no;
	target_mesh->_face_data.CustomData_bmesh_free_block_data(target_face->head.data);
	CustomData::CustomData_bmesh_copy_data(&source_mesh->_face_data, &target_mesh->_face_data, source_face->head.data, &target_face->head.data);
}

void BMesh::BM_elem_attrs_copy(const BMesh *bm_src, BMesh *bm_dst, const void *ele_src, void *ele_dst)
{
	/* BMESH_TODO, default 'use_flags' to false */
	BM_elem_attrs_copy_ex(bm_src, bm_dst, ele_src, ele_dst, BM_ELEM_SELECT);
}

/**
* Copies attributes, e.g. customdata, header flags, etc, from one element
* to another of the same type.
*/
void BMesh::BM_elem_attrs_copy_ex(const BMesh *bm_src, BMesh *bm_dst, const void *ele_src_v, void *ele_dst_v, const unsigned char hflag_mask)
{
	const BMHeader *ele_src = reinterpret_cast<const BMHeader*>(ele_src_v);
	BMHeader *ele_dst = reinterpret_cast<BMHeader*>(ele_dst_v);

	BLI_assert(ele_src->htype == ele_dst->htype);
	BLI_assert(ele_src != ele_dst);

#if 0
	if ((hflag_mask & BM_ELEM_SELECT) == 0) {
		/* First we copy select */
		if (BM_elem_flag_test((BMElem *)ele_src, BM_ELEM_SELECT)) {
			BM_elem_select_set(bm_dst, (BMElem *)ele_dst, true);
		}
	}
#endif

	/* Now we copy flags */
	if (hflag_mask == 0) {
		ele_dst->hflag = ele_src->hflag;
	}
	else if (hflag_mask == 0xff) {
		/* pass */
	}
	else {
		ele_dst->hflag = ((ele_dst->hflag & hflag_mask) | (ele_src->hflag & ~hflag_mask));
	}

	/* Copy specific attributes */
	switch (ele_dst->htype) {
	case BM_VERT:
		bm_vert_attrs_copy(bm_src, bm_dst, (const BMVert *)ele_src, (BMVert *)ele_dst);
		break;
	case BM_EDGE:
		bm_edge_attrs_copy(bm_src, bm_dst, (const BMEdge *)ele_src, (BMEdge *)ele_dst);
		break;
	case BM_LOOP:
		bm_loop_attrs_copy(bm_src, bm_dst, (const BMLoop *)ele_src, (BMLoop *)ele_dst);
		break;
	case BM_FACE:
		bm_face_attrs_copy(bm_src, bm_dst, (const BMFace *)ele_src, (BMFace *)ele_dst);
		break;
	default:
		BLI_assert(0);
		break;
	}
}


/**
* \brief Data Face-Vert Edge Interp
*
* Walks around the faces of \a e and interpolates
* the loop data between two sources.
*/
void BMesh::BM_data_interp_face_vert_edge(const BMVert *v_src_1, const BMVert *v_src_2, BMVert *v, BMEdge *e, const float fac)
{
	float w[2];
	BMLoop *l_v1 = NULL, *l_v = NULL, *l_v2 = NULL;
	BMLoop *l_iter = NULL;

	if (!e->l) {
		return;
	}

	w[1] = 1.0f - fac;
	w[0] = fac;

	l_iter = e->l;
	do {
		if (l_iter->v == v_src_1) {
			l_v1 = l_iter;
			l_v = l_v1->next;
			l_v2 = l_v->next;
		}
		else if (l_iter->v == v) {
			l_v1 = l_iter->next;
			l_v = l_iter;
			l_v2 = l_iter->prev;
		}

		if (!l_v1 || !l_v2) {
			return;
		}
		else {
#ifdef USE_BMLOOP_HEAD
			const void *src[2];
			src[0] = l_v1->head.data;
			src[1] = l_v2->head.data;

			this->_loop_data.CustomData_bmesh_interp(src, w, NULL, 2, l_v->head.data);
#endif
		}
	} while ((l_iter = l_iter->radial_next) != e->l);
}

/**
* \brief Data, Interp From Verts
*
* Interpolates per-vertex data from two sources to \a v_dst
*
* \note This is an exact match to #BM_data_interp_from_edges
*/
void  BMesh::BM_data_interp_from_verts(const BMVert *v_src_1, const BMVert *v_src_2, BMVert *v_dst, const float fac)
{
	bm_data_interp_from_elem(&_vert_data, (const BMElem *)v_src_1, (const BMElem *)v_src_2, (BMElem *)v_dst, fac);
}

/**
* \brief Data, Interp From Edges
*
* Interpolates per-edge data from two sources to \a e_dst.
*
* \note This is an exact match to #BM_data_interp_from_verts
*/
void BMesh::BM_data_interp_from_edges(const BMEdge *e_src_1, const BMEdge *e_src_2, BMEdge *e_dst, const float fac)
{
	bm_data_interp_from_elem(&_edge_data, (const BMElem *)e_src_1, (const BMElem *)e_src_2, (BMElem *)e_dst, fac);
}

/* edge and vertex share, currently theres no need to have different logic */
void BMesh::bm_data_interp_from_elem(CustomData *data_layer, const BMElem *ele_src_1, const BMElem *ele_src_2, BMElem *ele_dst, const float fac)
{
	if (ele_src_1->head.data && ele_src_2->head.data) {
		/* first see if we can avoid interpolation */
		if (fac <= 0.0f) {
			if (ele_src_1 == ele_dst) {
				/* do nothing */
			}
			else {
				data_layer->CustomData_bmesh_free_block_data(ele_dst->head.data);
				CustomData::CustomData_bmesh_copy_data(data_layer, data_layer, ele_src_1->head.data, &ele_dst->head.data);
			}
		}
		else if (fac >= 1.0f) {
			if (ele_src_2 == ele_dst) {
				/* do nothing */
			}
			else {
				data_layer->CustomData_bmesh_free_block_data(ele_dst->head.data);
				CustomData::CustomData_bmesh_copy_data(data_layer, data_layer, ele_src_2->head.data, &ele_dst->head.data);
			}
		}
		else {
			const void *src[2];
			float w[2];

			src[0] = ele_src_1->head.data;
			src[1] = ele_src_2->head.data;
			w[0] = 1.0f - fac;
			w[1] = fac;
			data_layer->CustomData_bmesh_interp(src, w, NULL, 2, ele_dst->head.data);
		}
	}
}


void BMesh::BM_mesh_elem_index_table_check(const char htype)
{
	if (htype & BM_VERT){
		BLI_assert(_tot_vert == vtable.size());
		for (size_t i = 0; i < vtable.size(); ++i){
			BLI_assert(BM_elem_index_get(vtable[i]) == i);
		}
	}

	if (htype & BM_FACE){
		BLI_assert(_tot_face == ftable.size());
		for (size_t i = 0; i < ftable.size(); ++i){
			BLI_assert(BM_elem_index_get(ftable[i]) == i);
		}
	}

	if (htype & BM_EDGE){
		BLI_assert(_tot_edge == etable.size());
		for (size_t i = 0; i < etable.size(); ++i){
			BLI_assert(BM_elem_index_get(etable[i]) == i);
		}
	}
}

void BMesh::BM_mesh_elem_index_check(const char htype)
{
	if (htype & BM_VERT){
		BMVert *v;
		int index;
		BMIter iter;
		BM_ITER_MESH_INDEX(v, &iter, this, BM_VERTS_OF_MESH, index){
			BLI_assert(BM_elem_index_get(v) == index);
		}
	}

	if (htype & BM_FACE){
		BMEdge *e;
		int index;
		BMIter iter;
		BM_ITER_MESH_INDEX(e, &iter, this, BM_EDGES_OF_MESH, index){
			BLI_assert(BM_elem_index_get(e) == index);
		}
	}

	if (htype & BM_EDGE){
		BMFace *f;
		int index;
		BMIter iter;
		BM_ITER_MESH_INDEX(f, &iter, this, BM_FACES_OF_MESH, index){
			BLI_assert(BM_elem_index_get(f) == index);
		}
	}
}

bool BMesh::BM_mesh_elem_table_dirty(const char htype)
{
	if (htype == BM_VERT) return (_elem_table_dirty & htype) != 0;
	if (htype == BM_FACE) return (_elem_table_dirty & htype) != 0;
	if (htype == BM_EDGE) return (_elem_table_dirty & htype) != 0;
	return false;
}
bool BMesh::BM_mesh_elem_index_dirty(const char htype)
{
	if (htype == BM_VERT) return (_elem_index_dirty & htype) != 0;
	if (htype == BM_FACE) return (_elem_index_dirty & htype) != 0;
	if (htype == BM_EDGE) return (_elem_index_dirty & htype) != 0;
	return false;
}

void BMesh::BM_mesh_elem_index_dirty_set(const char htype)
{
	if (htype == BM_VERT) (_elem_index_dirty |= htype);
	if (htype == BM_FACE) (_elem_index_dirty |= htype);
	if (htype == BM_EDGE) (_elem_index_dirty |= htype);
}

void BMesh::BM_mesh_elem_table_ensure(const char htype, bool ensure_index)
{
	/* assume if the array is non-null then its valid and no need to recalc */
	const char htype_needed = 
		(
		(((_elem_table_dirty & BM_VERT) == 0) ? 0 : BM_VERT) |
		(((_elem_table_dirty & BM_EDGE) == 0) ? 0 : BM_EDGE) |
		(((_elem_table_dirty & BM_FACE) == 0) ? 0 : BM_FACE)
		) & htype;

	BLI_assert((htype & ~BM_ALL_NOLOOP) == 0);

	/* in debug mode double check we didn't need to recalculate */
	BLI_assert(BM_mesh_elem_table_check() == true);

	if (ensure_index){
	}

	if (htype_needed == 0) {
		goto finally;
	}

	if (htype_needed & BM_VERT) {
		vtable.resize(_tot_vert);
	}
	if (htype_needed & BM_EDGE) {
		etable.resize(_tot_edge);
	}
	if (htype_needed & BM_FACE) {
		ftable.resize(_tot_face);
	}

	if ((!ELEM(htype_needed, BM_VERT, BM_EDGE, BM_FACE)) && \
		(_tot_vert + _tot_edge + _tot_face >= BM_OMP_LIMIT))
	{
		tbb::task_group tasks;
		if (htype_needed & BM_VERT) {
			tasks.run([&](){
				BM_iter_as_array(this, BM_VERTS_OF_MESH, NULL, (void **)vtable.data(), _tot_vert);
				if (ensure_index){
					for (size_t i = 0; i < _tot_vert; ++i){
						BM_elem_index_set(vtable[i], i);
					}
				}
			});
		}

		if (htype_needed & BM_EDGE) {
			tasks.run([&](){
				BM_iter_as_array(this, BM_EDGES_OF_MESH, NULL, (void **)etable.data(), _tot_edge);
				if (ensure_index){
					for (size_t i = 0; i < _tot_edge; ++i){
						BM_elem_index_set(etable[i], i);
					}
				}
			});
		}

		if (htype_needed & BM_FACE) {
			tasks.run([&](){
				BM_iter_as_array(this, BM_FACES_OF_MESH, NULL, (void **)ftable.data(), _tot_face);
				
				if (ensure_index){
					for (size_t i = 0; i < _tot_face; ++i){
						BM_elem_index_set(ftable[i], i);
					}
				}
			});
		}

		tasks.wait();
	}
	else{
		if (htype_needed & BM_VERT) {
			BM_iter_as_array(this, BM_VERTS_OF_MESH, NULL, (void **)vtable.data(), _tot_vert);
			if (ensure_index){
				tbb::parallel_for(
					tbb::blocked_range<size_t>(0, _tot_vert),
					[&](const tbb::blocked_range<size_t> & range)
				{
					for (size_t i = range.begin(); i < range.end(); ++i){
						BM_elem_index_set(vtable[i], i);
					}
				});
			}
		}

		if (htype_needed & BM_EDGE) {
			BM_iter_as_array(this, BM_EDGES_OF_MESH, NULL, (void **)etable.data(), _tot_edge);
			if (ensure_index){
				tbb::parallel_for(
					tbb::blocked_range<size_t>(0, _tot_edge),
					[&](const tbb::blocked_range<size_t> & range)
				{
					for (size_t i = range.begin(); i < range.end(); ++i){
						BM_elem_index_set(etable[i], i);
					}
				});
			}
		}

		if (htype_needed & BM_FACE) {
			BM_iter_as_array(this, BM_FACES_OF_MESH, NULL, (void **)ftable.data(), _tot_face);
			if (ensure_index){
				tbb::parallel_for(
					tbb::blocked_range<size_t>(0, _tot_face),
					[&](const tbb::blocked_range<size_t> & range)
				{
					for (size_t i = range.begin(); i < range.end(); ++i){
						BM_elem_index_set(ftable[i], i);
					}
				});
			}
		}
	}

	finally:
	/* Only clear dirty flags when all the pointers and data are actually valid.
	* This prevents possible threading issues when dirty flag check failed but
	* data wasn't ready still.
	*/
	_elem_table_dirty &= ~htype_needed;
	if (ensure_index){
		_elem_index_dirty &= ~htype_needed;
	}
}

bool BMesh::BM_mesh_elem_table_check()
{
	BMIter iter;
	BMElem *ele;
	int i;

	if (!vtable.empty() && ((_elem_table_dirty & BM_VERT) == 0)) {
		BM_ITER_MESH_INDEX(ele, &iter, this, BM_VERTS_OF_MESH, i) {
			if (ele != (BMElem*)vtable[i]) {
				return false;
			}
		}
	}

	if (etable.empty() && ((_elem_table_dirty & BM_EDGE) == 0)) {
		BM_ITER_MESH_INDEX(ele, &iter, this, BM_EDGES_OF_MESH, i) {
			if (ele != (BMElem*)etable[i]) {
				return false;
			}
		}
	}

	if (!ftable.empty() && ((_elem_table_dirty & BM_FACE) == 0)) {
		BM_ITER_MESH_INDEX(ele, &iter, this, BM_FACES_OF_MESH, i) {
			if (ele != (BMElem*)ftable[i]) {
				return false;
			}
		}
	}

	return true;
}


/**
* Array checking/setting macros
*
* Currently vert/edge/loop/face index data is being abused, in a few areas of the code.
*
* To avoid correcting them afterwards, set 'bm->elem_index_dirty' however its possible
* this flag is set incorrectly which could crash blender.
*
* These functions ensure its correct and are called more often in debug mode.
*/

void BMesh::BM_mesh_elem_index_validate(
	const char *location, const char *func,
	const char *msg_a, const char *msg_b)
{
	const char iter_types[3] = { BM_VERTS_OF_MESH,
		BM_EDGES_OF_MESH,
		BM_FACES_OF_MESH };

	const char flag_types[3] = { BM_VERT, BM_EDGE, BM_FACE };
	const char *type_names[3] = { "vert", "edge", "face" };

	BMIter iter;
	BMElem *ele;
	int i;
	bool is_any_error = 0;

	for (i = 0; i < 3; i++) {
		const bool is_dirty = (flag_types[i] & _elem_index_dirty) != 0;
		int index = 0;
		bool is_error = false;
		int err_val = 0;
		int err_idx = 0;

		BM_ITER_MESH(ele, &iter, this, iter_types[i]) {
			if (!is_dirty) {
				if (BM_elem_index_get(ele) != index) {
					err_val = BM_elem_index_get(ele);
					err_idx = index;
					is_error = true;
				}
			}

			BM_elem_index_set(ele, index); /* set_ok */
			index++;
		}

		if ((is_error == true) && (is_dirty == false)) {
			is_any_error = true;
			fprintf(stderr,
				"Invalid Index: at %s, %s, %s[%d] invalid index %d, '%s', '%s'\n",
				location, func, type_names[i], err_idx, err_val, msg_a, msg_b);
		}
		else if ((is_error == false) && (is_dirty == true)) {

#if 0       /* mostly annoying */

			/* dirty may have been incorrectly set */
			fprintf(stderr,
				"Invalid Dirty: at %s, %s (%s), dirty flag was set but all index values are correct, '%s', '%s'\n",
				location, func, type_names[i], msg_a, msg_b);
#endif
		}
	}

#if 0 /* mostly annoying, even in debug mode */
#ifdef DEBUG
	if (is_any_error == 0) {
		fprintf(stderr,
			"Valid Index Success: at %s, %s, '%s', '%s'\n",
			location, func, msg_a, msg_b);
	}
#endif
#endif
	(void)is_any_error; /* shut up the compiler */

}

void BMesh::BM_mesh_elem_index_ensure(const char htype)
{
	const char htype_needed = _elem_index_dirty & htype;

#ifdef DEBUG
	BM_ELEM_INDEX_VALIDATE(bm, "Should Never Fail!", __func__);
#endif

	if (htype_needed == 0) {
		goto finally;
	}

	/* skip if we only need to operate on one element */
	/* multi-thread */
	if ((!ELEM(htype_needed, BM_VERT, BM_EDGE, BM_FACE, BM_LOOP, BM_FACE | BM_LOOP)) && 
	    (_tot_vert + _tot_edge + _tot_face >= BM_OMP_LIMIT)){

		tbb::task_group tasks;
		if (htype & BM_VERT) {
			if (_elem_index_dirty & BM_VERT) {
				tasks.run([&]
				{
					BMIter iter;
					BMElem *ele;

					int index;
					BM_ITER_MESH_INDEX(ele, &iter, this, BM_VERTS_OF_MESH, index) {
						BM_elem_index_set(ele, index); /* set_ok */
					}
					BLI_assert(index == _tot_vert);
				});
			}
			else {
				// printf("%s: skipping vert index calc!\n", __func__);
			}
		}

		if (htype & BM_EDGE) {
			if (_elem_index_dirty & BM_EDGE) {
				tasks.run([&]
				{
					BMIter iter;
					BMElem *ele;

					int index;
					BM_ITER_MESH_INDEX(ele, &iter, this, BM_EDGES_OF_MESH, index) {
						BM_elem_index_set(ele, index); /* set_ok */
					}
					BLI_assert(index == _tot_edge);
				});

			}
			else {
				// printf("%s: skipping edge index calc!\n", __func__);
			}
		}

		if (htype & (BM_FACE | BM_LOOP)) {
#if USE_BMLOOP_HEAD
			if (_elem_index_dirty & (BM_FACE | BM_LOOP)) {

				tasks.run([&]
				{
					BMIter iter;
					BMElem *ele;

					const bool update_face = (htype & BM_FACE) && (_elem_index_dirty & BM_FACE);
					const bool update_loop = (htype & BM_LOOP) && (_elem_index_dirty & BM_LOOP);

					int index;
					int index_loop = 0;

					BM_ITER_MESH_INDEX(ele, &iter, this, BM_FACES_OF_MESH, index) {
						if (update_face) {
							BM_elem_index_set(ele, index); /* set_ok */
						}

						if (update_loop) {
							BMLoop *l_iter, *l_first;

							l_iter = l_first = BM_FACE_FIRST_LOOP((BMFace *)ele);
							do {
								BM_elem_index_set(l_iter, index_loop++); /* set_ok */
							} while ((l_iter = l_iter->next) != l_first);
						}
					}

					BLI_assert(index == _tot_face);
					if (update_loop) {
						BLI_assert(index_loop == _tot_loop);
					}
				});
			}
			else {
				// printf("%s: skipping face/loop index calc!\n", __func__);
			}
#endif
		}
	}
	else{
		/*single thread*/
		{
			if (htype & BM_VERT) {
				if (_elem_index_dirty & BM_VERT) {
					BMIter iter;
					BMElem *ele;

					int index;
					BM_ITER_MESH_INDEX(ele, &iter, this, BM_VERTS_OF_MESH, index) {
						BM_elem_index_set(ele, index); /* set_ok */
					}
					BLI_assert(index == _tot_vert);
				}
				else {
					// printf("%s: skipping vert index calc!\n", __func__);
				}
			}
		}

		{
			if (htype & BM_EDGE) {
				if (_elem_index_dirty & BM_EDGE) {
					BMIter iter;
					BMElem *ele;

					int index;
					BM_ITER_MESH_INDEX(ele, &iter, this, BM_EDGES_OF_MESH, index) {
						BM_elem_index_set(ele, index); /* set_ok */
					}
					BLI_assert(index == _tot_edge);
				}
				else {
					// printf("%s: skipping edge index calc!\n", __func__);
				}
			}
		}
#ifdef USE_BMLOOP_HEAD
		{
			if (htype & (BM_FACE | BM_LOOP)) {
				if (_elem_index_dirty & (BM_FACE | BM_LOOP)) {
					BMIter iter;
					BMElem *ele;

					const bool update_face = (htype & BM_FACE) && (_elem_index_dirty & BM_FACE);
					const bool update_loop = (htype & BM_LOOP) && (_elem_index_dirty & BM_LOOP);

					int index;
					int index_loop = 0;

					BM_ITER_MESH_INDEX(ele, &iter, this, BM_FACES_OF_MESH, index) {
						if (update_face) {
							BM_elem_index_set(ele, index); /* set_ok */
						}

						if (update_loop) {
							BMLoop *l_iter, *l_first;

							l_iter = l_first = BM_FACE_FIRST_LOOP((BMFace *)ele);
							do {
								BM_elem_index_set(l_iter, index_loop++); /* set_ok */
							} while ((l_iter = l_iter->next) != l_first);
						}
					}

					BLI_assert(index == _tot_face);
					if (update_loop) {
						BLI_assert(index_loop == _tot_loop);
					}
				}
				else {
					// printf("%s: skipping face/loop index calc!\n", __func__);
				}
			}
		}
#endif

	}/*end single thread*/

	finally:
	_elem_index_dirty &= ~htype;
}

void BMesh::BM_mesh_normals_update_parallel()
{
	/* calculate all face normals */
	BM_mesh_elem_table_ensure(BM_VERT | BM_EDGE | BM_FACE, true);
	
	tbb::task_group tasks;

	tasks.run([&]
	{
		tbb::parallel_for(
			tbb::blocked_range<size_t>(0, ftable.size()),
			[&](const tbb::blocked_range<size_t> &range)
		{
			for (size_t i = range.begin(); i < range.end(); ++i){
				BM_face_normal_update(ftable[i]);
			}
		});
	});


	std::vector<Vector3f> edgevec(_tot_edge);
	tasks.run([&]{
		tbb::parallel_for(tbb::blocked_range<size_t>(0, _tot_edge),
			[&](const tbb::blocked_range<size_t> &range){

			for (size_t i = range.begin(); i < range.end(); ++i){
				edgevec[i] = (etable[i]->v2->co - etable[i]->v1->co).normalized();
			}
		});

	});

	tasks.wait();

	tbb::parallel_for(tbb::blocked_range<size_t>(0, _tot_vert),
		[&](const tbb::blocked_range<size_t> &range){

		for (size_t i = range.begin(); i < range.end(); ++i){
				
			BMVert *v = vtable[i];
			if (v->e) {
				const BMEdge *e = v->e;
				v->no.setZero();
				size_t len = 0;
				do {
					if (e->l) {
						const BMLoop *l = e->l;
						do {
							if (l->v == v) {
								/* calculate the dot product of the two edges that
								* meet at the loop's vertex */
								const Vector3f &e1diff = edgevec[BM_elem_index_get(l->prev->e)];
								const Vector3f &e2diff = edgevec[BM_elem_index_get(l->e)];
								float dotprod = e1diff.dot(e2diff);

								/* edge vectors are calculated from e->v1 to e->v2, so
								* adjust the dot product if one but not both loops
								* actually runs from from e->v2 to e->v1 */
								if ((l->prev->e->v1 == l->prev->v) ^ (l->e->v1 == l->v)) {
									dotprod = -dotprod;
								}

								float fac = MathBase::saacos(-dotprod);

								/* accumulate weighted face normal into the vertex's normal */
								v->no += l->f->no *fac;
								//v->no += l->f->no *1.0f;
								len++;
							}
						} while ((l = l->radial_next) != e->l);
					}
				} while ((e = bmesh_disk_edge_next(e, v)) != v->e);
				
				if (len) {
					Vector3f &v_no = v->no;
					if (UNLIKELY((MathUtil::normalize(v_no) == 0.0f))) {
						const Vector3f &v_co = v->co;
						v_no = v_co.normalized();
					}
				}
			}

		}
	});

}


void BMesh::BM_mesh_normals_update_area()
{
	/* calculate all face normals */

	BM_mesh_elem_table_ensure(BM_VERT | BM_FACE, true);

	std::vector<float> f_area(_tot_face, 0.0f);

	tbb::parallel_for(
		tbb::blocked_range<size_t>(0, ftable.size()),
		[&](const tbb::blocked_range<size_t> &range)
	{
		for (size_t i = range.begin(); i < range.end(); ++i){
			BM_face_normal_update(ftable[i]);
			f_area[i] = BM_face_calc_area(ftable[i]);
		}
	});


	tbb::parallel_for(tbb::blocked_range<size_t>(0, _tot_vert),
		[&](const tbb::blocked_range<size_t> &range){

		for (size_t i = range.begin(); i < range.end(); ++i){

			BMVert *v = vtable[i];
			if (v->e) {
				const BMEdge *e = v->e;
				v->no.setZero();
				float weights = 0.0f, w;
				do {
					if (e->l) {
						const BMLoop *l = e->l;
						do {
							if (l->v == v) {
								w = f_area[BM_elem_index_get(l->f)];
								v->no += l->f->no * w;
								weights += w;
							}
						} while ((l = l->radial_next) != e->l);
					}
				} while ((e = bmesh_disk_edge_next(e, v)) != v->e);

				if (weights > 0.0f) {
					v->no /= weights;
					if (UNLIKELY((MathUtil::normalize(v->no) == 0.0f))) {
						v->no = v->co.normalized();
					}
				}
			}

		}
	});
}
/**
* \brief BMesh Compute Normals
*
* Updates the normals of a mesh.
*/
void BMesh::BM_mesh_normals_update()
{
	std::vector<Vector3f> edgevec(_tot_edge);

	{
		/* calculate all face normals */
		BMIter fiter;
		BMFace *f;
		int i;

		BM_ITER_MESH_INDEX(f, &fiter, this, BM_FACES_OF_MESH, i) {
			BM_elem_index_set(f, i); /* set_inline */
			BM_face_normal_update(f);
		}
		_elem_index_dirty &= ~BM_FACE;
	}

	{
		/* Zero out vertex normals */
		BMIter viter;
		BMVert *v;
		int i;

		BM_ITER_MESH_INDEX(v, &viter, this, BM_VERTS_OF_MESH, i) {
			BM_elem_index_set(v, i); /* set_inline */
			v->no.setZero();
		}
		_elem_index_dirty &= ~BM_VERT;
	}

	{
		/* Compute normalized direction vectors for each edge.
		* Directions will be used for calculating the weights of the face normals on the vertex normals.
		*/
		bm_mesh_edges_calc_vectors(edgevec, std::vector<Vector3f>());
	}
	/* end omp */

	/* Add weighted face normals to vertices, and normalize vert normals. */
	bm_mesh_verts_calc_normals(edgevec, std::vector<Vector3f>(), std::vector<Vector3f>(), std::vector<Vector3f>());
}

/**
* Helpers for #BM_mesh_normals_update and #BM_verts_calc_normal_vcos
*/
void BMesh::bm_mesh_edges_calc_vectors(std::vector<Vector3f> &edgevec, const std::vector<Vector3f> &vcos)
{
	BMIter eiter;
	BMEdge *e;
	int index;

	if (!vcos.empty()) {
		BM_mesh_elem_index_ensure(BM_VERT);
	}

	BM_ITER_MESH_INDEX(e, &eiter, this, BM_EDGES_OF_MESH, index) {
		BM_elem_index_set(e, index); /* set_inline */

		if (e->l) {
			const Vector3f &v1_co = !vcos.empty() ? vcos[BM_elem_index_get(e->v1)] : e->v1->co;
			const Vector3f &v2_co = !vcos.empty() ? vcos[BM_elem_index_get(e->v2)] : e->v2->co;
			edgevec[index] = v2_co - v1_co;
			edgevec[index].normalize();
		}
		else {
			/* the edge vector will not be needed when the edge has no radial */
		}
	}
	_elem_index_dirty &= ~BM_EDGE;
}

void BMesh::bm_mesh_verts_calc_normals(
	const std::vector<Vector3f> &edgevec, const std::vector<Vector3f> &fnos, 
	const std::vector<Vector3f> &vcos, std::vector<Vector3f> &vnos)
{
	BM_mesh_elem_index_ensure((!vnos.empty()) ? (BM_EDGE | BM_VERT) : BM_EDGE);

	/* add weighted face normals to vertices */
	{
		BMIter fiter;
		BMFace *f;
		int i;

		BM_ITER_MESH_INDEX(f, &fiter, this, BM_FACES_OF_MESH, i) {
			BMLoop *l_first, *l_iter;
			const Vector3f &f_no = !fnos.empty() ? fnos[i] : f->no;

			l_iter = l_first = BM_FACE_FIRST_LOOP(f);
			do {
				Vector3f e1diff, e2diff;
				float dotprod;
				float fac;
				Vector3f &v_no = !vnos.empty() ? vnos[BM_elem_index_get(l_iter->v)] : l_iter->v->no;

				/* calculate the dot product of the two edges that
				* meet at the loop's vertex */
				e1diff = edgevec[BM_elem_index_get(l_iter->prev->e)];
				e2diff = edgevec[BM_elem_index_get(l_iter->e)];
				dotprod = e1diff.dot(e2diff);

				/* edge vectors are calculated from e->v1 to e->v2, so
				* adjust the dot product if one but not both loops
				* actually runs from from e->v2 to e->v1 */
				if ((l_iter->prev->e->v1 == l_iter->prev->v) ^ (l_iter->e->v1 == l_iter->v)) {
					dotprod = -dotprod;
				}

				fac = MathBase::saacos(-dotprod);

				/* accumulate weighted face normal into the vertex's normal */
				v_no += f_no *fac;
			} while ((l_iter = l_iter->next) != l_first);
		}
	}


	/* normalize the accumulated vertex normals */
	{
		BMIter viter;
		BMVert *v;
		int i;

		BM_ITER_MESH_INDEX(v, &viter, this, BM_VERTS_OF_MESH, i) {
			Vector3f &v_no = !vnos.empty() ? vnos[i] : v->no;
			if (UNLIKELY((MathUtil::normalize(v_no) == 0.0f))) {
				const Vector3f &v_co = !vcos.empty() ? vcos[i] : v->co;
				v_no = v_co.normalized();
			}
		}
	}
}

bool BMesh::bmesh_loop_reverse(BMFace *f)
{
	BMLoop *l_first = f->l_first;
	const int len = f->len;
	BMLoop *l_iter, *oldprev, *oldnext;
	boost::container::static_vector<BMEdge*, 4> edar;
	
	int i, j, edok;
	for (i = 0, l_iter = l_first; i < len; i++, l_iter = l_iter->next) {
		edar.push_back(l_iter->e);
		bmesh_radial_loop_remove(l_iter, edar[i]);
	}

	/* actually reverse the loop */
	for (i = 0, l_iter = l_first; i < len; i++) {
		oldnext = l_iter->next;
		oldprev = l_iter->prev;
		l_iter->next = oldprev;
		l_iter->prev = oldnext;
		l_iter = oldnext;
	}

	if (len == 2) { /* two edged face */
		/* do some verification here! */
		l_first->e = edar[1];
		l_first->next->e = edar[0];
	}
	else {
		for (i = 0, l_iter = l_first; i < len; i++, l_iter = l_iter->next) {
			edok = 0;
			for (j = 0; j < len; j++) {
				edok = BM_verts_in_edge(l_iter->v, l_iter->next->v, edar[j]);
				if (edok) {
					l_iter->e = edar[j];
					break;
				}
			}
		}
	}
	/* rebuild radial */
	for (i = 0, l_iter = l_first; i < len; i++, l_iter = l_iter->next)
		bmesh_radial_append(l_iter->e, l_iter);

#ifndef NDEBUG
	/* validate radial */
	for (i = 0, l_iter = l_first; i < len; i++, l_iter = l_iter->next) {
		bmesh_elem_loop_check(l_iter);
		BM_CHECK_ELEMENT(l_iter->e);
		BM_CHECK_ELEMENT(l_iter->v);
		BM_CHECK_ELEMENT(l_iter->f);
	}

	BM_CHECK_ELEMENT(f);
#endif

	/* Loop indices are no more valid! */
	_elem_index_dirty |= BM_LOOP;

	return true;
}



VM_END_NAMESPACE




