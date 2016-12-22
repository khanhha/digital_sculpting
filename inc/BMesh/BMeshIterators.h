#ifndef VMESH_ITERATORS_H
#define VMESH_ITERATORS_H

#include "BMUtilDefine.h"
#include "BaseLib/UtilMacro.h"
#include "BMCompilerTypeCheck.h"
#include "BMeshClass.h"
#include "BMeshCore.h"
#include "BMeshStructure.h"

VM_BEGIN_NAMESPACE

/* Defines for passing to BM_iter_new.
*
* "OF" can be substituted for "around"
* so BM_VERTS_OF_FACE means "vertices
* around a face."
*/

/* these iterator over all elements of a specific
* type in the mesh.
*
* be sure to keep 'bm_iter_itype_htype_map' in sync with any changes
*/
typedef enum BMIterType {
	BM_VERTS_OF_MESH = 1,
	BM_EDGES_OF_MESH = 2,
	BM_FACES_OF_MESH = 3,
	/* these are topological iterators. */
	BM_EDGES_OF_VERT = 4,
	BM_FACES_OF_VERT = 5,
	BM_LOOPS_OF_VERT = 6,
	BM_VERTS_OF_EDGE = 7, /* just v1, v2: added so py can use generalized sequencer wrapper */
	BM_FACES_OF_EDGE = 8,
	BM_VERTS_OF_FACE = 9,
	BM_EDGES_OF_FACE = 10,
	BM_LOOPS_OF_FACE = 11,
	/* returns elements from all boundaries, and returns
	* the first element at the end to flag that we're entering
	* a different face hole boundary*/
	// BM_ALL_LOOPS_OF_FACE = 12,
	/* iterate through loops around this loop, which are fetched
	* from the other faces in the radial cycle surrounding the
	* input loop's edge.*/
	BM_LOOPS_OF_LOOP = 12,
	BM_LOOPS_OF_EDGE = 13
} BMIterType;

#define BM_ITYPE_MAX 14

/* the iterator htype for each iterator */
extern const char bm_iter_itype_htype_map[BM_ITYPE_MAX];

#define BM_ITER_MESH(ele, iter, bm, itype) \
	for (BM_CHECK_TYPE_ELEM_ASSIGN(ele) = BM_iter_new(iter, bm, itype, NULL); \
	     ele; \
	     BM_CHECK_TYPE_ELEM_ASSIGN(ele) = BM_iter_step(iter))

#define BM_ITER_MESH_INDEX(ele, iter, bm, itype, indexvar) \
	for (BM_CHECK_TYPE_ELEM_ASSIGN(ele) = BM_iter_new(iter, bm, itype, NULL), indexvar = 0; \
	     ele; \
	     BM_CHECK_TYPE_ELEM_ASSIGN(ele) = BM_iter_step(iter), (indexvar)++)

/* a version of BM_ITER_MESH which keeps the next item in storage
* so we can delete the current item, see bug [#36923] */
#ifdef DEBUG
#  define BM_ITER_MESH_MUTABLE(ele, ele_next, iter, bm, itype) \
	for (BM_CHECK_TYPE_ELEM_ASSIGN(ele) = BM_iter_new(iter, bm, itype, NULL); \
	     ele ? ((void)((iter)->count = BM_iter_mesh_count(itype, bm)), \
	            (void)(ele_next = BM_iter_step(iter)), 1) : 0; \
	     BM_CHECK_TYPE_ELEM_ASSIGN(ele) = ele_next)
#else
#  define BM_ITER_MESH_MUTABLE(ele, ele_next, iter, bm, itype) \
	for (BM_CHECK_TYPE_ELEM_ASSIGN(ele) = BM_iter_new(iter, bm, itype, NULL); \
	     ele ? ((BM_CHECK_TYPE_ELEM_ASSIGN(ele_next) = BM_iter_step(iter)), 1) : 0; \
	     ele = ele_next)
#endif


#define BM_ITER_ELEM(ele, iter, data, itype) \
	for (BM_CHECK_TYPE_ELEM_ASSIGN(ele) = BM_iter_new(iter, NULL, itype, data); \
	     ele; \
	     BM_CHECK_TYPE_ELEM_ASSIGN(ele) = BM_iter_step(iter))

#define BM_ITER_ELEM_INDEX(ele, iter, data, itype, indexvar) \
	for (BM_CHECK_TYPE_ELEM_ASSIGN(ele) = BM_iter_new(iter, NULL, itype, data), indexvar = 0; \
	     ele; \
	     BM_CHECK_TYPE_ELEM_ASSIGN(ele) = BM_iter_step(iter), (indexvar)++)

/* iterator type structs */
struct BMIter__elem_of_mesh {
	union
	{
		BMVert *v;
		BMFace *f;
		BMEdge *e;
		BMLoop *l;
	}elem;

	int			count;
	BMIterType	itype;
};
struct BMIter__edge_of_vert {
	BMVert *vdata;
	BMEdge *e_first, *e_next;
};
struct BMIter__face_of_vert {
	BMVert *vdata;
	BMLoop *l_first, *l_next;
	BMEdge *e_first, *e_next;
};
struct BMIter__loop_of_vert {
	BMVert *vdata;
	BMLoop *l_first, *l_next;
	BMEdge *e_first, *e_next;
};
struct BMIter__loop_of_edge {
	BMEdge *edata;
	BMLoop *l_first, *l_next;
};
struct BMIter__loop_of_loop {
	BMLoop *ldata;
	BMLoop *l_first, *l_next;
};
struct BMIter__face_of_edge {
	BMEdge *edata;
	BMLoop *l_first, *l_next;
};
struct BMIter__vert_of_edge {
	BMEdge *edata;
};
struct BMIter__vert_of_face {
	BMFace *pdata;
	BMLoop *l_first, *l_next;
};
struct BMIter__edge_of_face {
	BMFace *pdata;
	BMLoop *l_first, *l_next;
};
struct BMIter__loop_of_face {
	BMFace *pdata;
	BMLoop *l_first, *l_next;
};

typedef void(*BMIter__begin_cb) (void *);
typedef void *(*BMIter__step_cb) (void *);

/* Iterator Structure */
/* note: some of these vars are not used,
* so they have been commented to save stack space since this struct is used all over */
typedef struct BMIter {
	/* keep union first */
	union {
		struct BMIter__elem_of_mesh elem_of_mesh;

		struct BMIter__edge_of_vert edge_of_vert;
		struct BMIter__face_of_vert face_of_vert;
		struct BMIter__loop_of_vert loop_of_vert;
		struct BMIter__loop_of_edge loop_of_edge;
		struct BMIter__loop_of_loop loop_of_loop;
		struct BMIter__face_of_edge face_of_edge;
		struct BMIter__vert_of_edge vert_of_edge;
		struct BMIter__vert_of_face vert_of_face;
		struct BMIter__edge_of_face edge_of_face;
		struct BMIter__loop_of_face loop_of_face;
	} data;

	BMIter__begin_cb begin;
	BMIter__step_cb step;

	int count;  /* note, only some iterators set this, don't rely on it */
	char itype;
} BMIter;

/* private for bmesh_iterators_inline.c */

#define BMITER_CB_DEF(name) \
	struct BMIter__##name; \
	void  bmiter__##name##_begin(struct BMIter__##name *iter); \
	void *bmiter__##name##_step(struct BMIter__##name *iter)

BMITER_CB_DEF(elem_of_mesh);
BMITER_CB_DEF(edge_of_vert);
BMITER_CB_DEF(face_of_vert);
BMITER_CB_DEF(loop_of_vert);
BMITER_CB_DEF(loop_of_edge);
BMITER_CB_DEF(loop_of_loop);
BMITER_CB_DEF(face_of_edge);
BMITER_CB_DEF(vert_of_edge);
BMITER_CB_DEF(vert_of_face);
BMITER_CB_DEF(edge_of_face);
BMITER_CB_DEF(loop_of_face);

#undef BMITER_CB_DEF


#define BM_ITER_CHECK_TYPE_DATA(data) \
	CHECK_TYPE_ANY(data, void *, BMFace *, BMEdge *, BMVert *, BMLoop *, BMElem *)

#define BM_iter_new(iter, bm, itype, data) \
	(BM_ITER_CHECK_TYPE_DATA(data), BM_iter_new_(iter, bm, itype, data))
#define BM_iter_init(iter, bm, itype, data) \
	(BM_ITER_CHECK_TYPE_DATA(data), BM_iter_init_(iter, bm, itype, data))


int BM_iter_as_array(BMesh *bm, const char itype, void *data, void **array, const int len);

/* inline here optimizes out the switch statement when called with
* constant values (which is very common), nicer for loop-in-loop situations */

/**
* \brief Iterator Step
*
* Calls an iterators step function to return the next element.
*/
BLI_INLINE void *BM_iter_step(BMIter *iter)
{
	return iter->step(iter);
}


/**
* \brief Iterator Init
*
* Takes a VMesh iterator structure and fills
* it with the appropriate function pointers based
* upon its type.
*/
BLI_INLINE bool BM_iter_init_(BMIter *iter, BMesh *bm, const char itype, void *data)
{
	/* int argtype; */
	iter->itype = itype;

	/* inlining optimizes out this switch when called with the defined type */
	switch ((BMIterType)itype) {
	case BM_VERTS_OF_MESH:
		BLI_assert(bm != NULL);
		BLI_assert(data == NULL);
		iter->begin = (BMIter__begin_cb)bmiter__elem_of_mesh_begin;
		iter->step = (BMIter__step_cb)bmiter__elem_of_mesh_step;
		iter->data.elem_of_mesh.elem.v = bm->BM_vert_begin_list();
		iter->data.elem_of_mesh.itype  = BM_VERTS_OF_MESH;
		iter->data.elem_of_mesh.count	= bm->BM_mesh_verts_total();
		break;
	case BM_EDGES_OF_MESH:
		BLI_assert(bm != NULL);
		BLI_assert(data == NULL);
		iter->begin = (BMIter__begin_cb)bmiter__elem_of_mesh_begin;
		iter->step = (BMIter__step_cb)bmiter__elem_of_mesh_step;
		iter->data.elem_of_mesh.elem.e = bm->BM_edge_begin_list();
		iter->data.elem_of_mesh.itype = BM_EDGES_OF_MESH;
		iter->data.elem_of_mesh.count = bm->BM_mesh_edges_total();
		break;
	case BM_FACES_OF_MESH:
		BLI_assert(bm != NULL);
		BLI_assert(data == NULL);
		iter->begin = (BMIter__begin_cb)bmiter__elem_of_mesh_begin;
		iter->step = (BMIter__step_cb)bmiter__elem_of_mesh_step;
		iter->data.elem_of_mesh.elem.f	= bm->BM_face_begin_list();
		iter->data.elem_of_mesh.count	= bm->BM_mesh_faces_total();
		iter->data.elem_of_mesh.itype	= BM_FACES_OF_MESH;
		break;
	case BM_EDGES_OF_VERT:
		BLI_assert(data != NULL);
		BLI_assert(((BMElem *)data)->head.htype == BM_VERT);
		iter->begin = (BMIter__begin_cb)bmiter__edge_of_vert_begin;
		iter->step = (BMIter__step_cb)bmiter__edge_of_vert_step;
		iter->data.edge_of_vert.vdata = (BMVert *)data;
		break;
	case BM_FACES_OF_VERT:
		BLI_assert(data != NULL);
		BLI_assert(((BMElem *)data)->head.htype == BM_VERT);
		iter->begin = (BMIter__begin_cb)bmiter__face_of_vert_begin;
		iter->step = (BMIter__step_cb)bmiter__face_of_vert_step;
		iter->data.face_of_vert.vdata = (BMVert *)data;
		break;
	case BM_LOOPS_OF_VERT:
		BLI_assert(data != NULL);
		BLI_assert(((BMElem *)data)->head.htype == BM_VERT);
		iter->begin = (BMIter__begin_cb)bmiter__loop_of_vert_begin;
		iter->step = (BMIter__step_cb)bmiter__loop_of_vert_step;
		iter->data.loop_of_vert.vdata = (BMVert *)data;
		break;
	case BM_VERTS_OF_EDGE:
		BLI_assert(data != NULL);
		BLI_assert(((BMElem *)data)->head.htype == BM_EDGE);
		iter->begin = (BMIter__begin_cb)bmiter__vert_of_edge_begin;
		iter->step = (BMIter__step_cb)bmiter__vert_of_edge_step;
		iter->data.vert_of_edge.edata = (BMEdge *)data;
		break;
	case BM_FACES_OF_EDGE:
		BLI_assert(data != NULL);
		BLI_assert(((BMElem *)data)->head.htype == BM_EDGE);
		iter->begin = (BMIter__begin_cb)bmiter__face_of_edge_begin;
		iter->step = (BMIter__step_cb)bmiter__face_of_edge_step;
		iter->data.face_of_edge.edata = (BMEdge *)data;
		break;
	case BM_VERTS_OF_FACE:
		BLI_assert(data != NULL);
		BLI_assert(((BMElem *)data)->head.htype == BM_FACE);
		iter->begin = (BMIter__begin_cb)bmiter__vert_of_face_begin;
		iter->step = (BMIter__step_cb)bmiter__vert_of_face_step;
		iter->data.vert_of_face.pdata = (BMFace *)data;
		break;
	case BM_EDGES_OF_FACE:
		BLI_assert(data != NULL);
		BLI_assert(((BMElem *)data)->head.htype == BM_FACE);
		iter->begin = (BMIter__begin_cb)bmiter__edge_of_face_begin;
		iter->step = (BMIter__step_cb)bmiter__edge_of_face_step;
		iter->data.edge_of_face.pdata = (BMFace *)data;
		break;
	case BM_LOOPS_OF_FACE:
		BLI_assert(data != NULL);
		BLI_assert(((BMElem *)data)->head.htype == BM_FACE);
		iter->begin = (BMIter__begin_cb)bmiter__loop_of_face_begin;
		iter->step = (BMIter__step_cb)bmiter__loop_of_face_step;
		iter->data.loop_of_face.pdata = (BMFace *)data;
		break;
	case BM_LOOPS_OF_LOOP:
		BLI_assert(data != NULL);
		BLI_assert(((BMElem *)data)->head.htype == BM_LOOP);
		iter->begin = (BMIter__begin_cb)bmiter__loop_of_loop_begin;
		iter->step = (BMIter__step_cb)bmiter__loop_of_loop_step;
		iter->data.loop_of_loop.ldata = (BMLoop *)data;
		break;
	case BM_LOOPS_OF_EDGE:
		BLI_assert(data != NULL);
		BLI_assert(((BMElem *)data)->head.htype == BM_EDGE);
		iter->begin = (BMIter__begin_cb)bmiter__loop_of_edge_begin;
		iter->step = (BMIter__step_cb)bmiter__loop_of_edge_step;
		iter->data.loop_of_edge.edata = (BMEdge *)data;
		break;
	default:
		/* should never happen */
		BLI_assert(0);
		return false;
		break;
	}

	iter->begin(iter);

	return true;
}

/**
* \brief Iterator New
*
* Takes a VMesh iterator structure and fills
* it with the appropriate function pointers based
* upon its type and then calls VMeshIter_step()
* to return the first element of the iterator.
*
*/
BLI_INLINE void *BM_iter_new_(BMIter *iter, BMesh *bm, const char itype, void *data)
{
	if (LIKELY(BM_iter_init(iter, bm, itype, data))) {
		return BM_iter_step(iter);
	}
	else {
		return NULL;
	}
}

VM_END_NAMESPACE

//#include "VMeshIteratorsInline.h"

#endif