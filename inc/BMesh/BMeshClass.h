#ifndef VSCULPT_VMESH_CLASS_H
#define VSCULPT_VMESH_CLASS_H

#include <Eigen/Dense>
#include <vector>
#include "BMUtilDefine.h"
#include "BaseLib/UtilMacro.h"
#include <array>
using namespace Eigen;

VM_BEGIN_NAMESPACE

#define AUX_DATA_SIZE 2*sizeof(void*) /*support at least two pointer size*/

struct BMVert;
struct BMEdge;
struct BMLoop;
struct BMFace;

struct BMHeader {
	void *data; /* customdata layers */
	int index; /* notes:
				* - Use BM_elem_index_get/set macros for index
				* - Uninitialized to -1 so we can easily tell its not set.
				* - Used for edge/vert/face/loop, check BMesh.elem_index_dirty for valid index values,
				*   this is abused by various tools which set it dirty.
				* - For loops this is used for sorting during tessellation. */

	char htype;    /* element geometric type (verts/edges/loops/faces) */
	char hflag;    /* this would be a CD layer, see below */

	/* internal use only!
	* note,.we are very picky about not bloating this struct
	* but in this case its padded up to 16 bytes anyway,
	* so adding a flag here gives no increase in size */
	char api_flag;
	char app_flag;
	std::array<char, AUX_DATA_SIZE> aux_data;
};

typedef Matrix<unsigned char, 4, 1> Vector4c;
struct BMEdge;
struct BMVert {
	BMHeader head;
	BMVert *next_elm, *prev_elm;
	Vector3f co;  /* vertex coordinates */
	Vector3f no;  /* vertex normal */
	Vector4c color;
	/* pointer to (any) edge using this vertex (for disk cycles)*/
	BMEdge *e;
};

/* disk link structure, only used by edges */
struct BMDiskLink {
	BMEdge *next, *prev;
};

struct BMEdge {
	BMHeader head;
	BMEdge *next_elm, *prev_elm;

	BMVert *v1, *v2;  /* vertices (unordered) */

	/* the list of loops around the edge (use l->radial_prev/next)
	* to access the other loops using the edge */
	BMLoop *l;

	/* disk cycle pointers
	* relative data: d1 indicates indicates the next/prev edge around vertex v1 and d2 does the same for v2 */
	BMDiskLink v1_disk_link, v2_disk_link;
};

struct BMLoop {
	//BMHeader head;
	BMLoop *next_elm, *prev_elm;

	BMVert *v;
	BMEdge *e; /* edge, using verts (v, next->v) */
	BMFace *f;

	/* circular linked list of loops which all use the same edge as this one '->e',
	* but not necessarily the same vertex (can be either v1 or v2 of our own '->e') */
	BMLoop *radial_next, *radial_prev;

	/* these were originally commented as private but are used all over the code */
	/* can't use ListBase API, due to head */
	BMLoop *next, *prev; /* next/prev verts around the face */
};

struct BMFace {
	BMHeader head;
	BMFace *next_elm, *prev_elm;

	BMLoop *l_first;
	int   len;			 /* number of vertices in the face */
	Vector3f no;  /* face normal */
};

/* can cast BMFace/BMEdge/BMVert, but NOT BMLoop, since these don't have a flag layer */
typedef struct BMElemF {
	BMHeader head;
} BMElemF;

/* can cast anything to this, including BMLoop */
typedef struct BMElem {
	BMHeader head;
} BMElem;

/* VMHeader->htype (char) */
enum {
	BM_VERT = 1,
	BM_EDGE = 2,
	BM_LOOP = 4,
	BM_FACE = 8
};

typedef enum eBMCreateFlag {
	BM_CREATE_NOP = 0,
	/* faces and edges only */
	BM_CREATE_NO_DOUBLE = (1 << 1),
	/* Skip CustomData - for all element types data,
	* use if we immediately write customdata into the element so this skips copying from 'example'
	* args or setting defaults, speeds up conversion when data is converted all at once. */
	BM_CREATE_SKIP_CD = (1 << 2),
} eVMCreateFlag;


#define BM_ALL (BM_VERT | BM_EDGE | BM_LOOP | BM_FACE)
#define BM_ALL_NOLOOP (BM_VERT | BM_EDGE | BM_FACE)

/* args for _Generic */
#define _BM_GENERIC_TYPE_ELEM_NONCONST \
	void *, BMVert *, BMEdge *, BMLoop *, BMFace *, \
	BMElem *, BMElemF *, BMHeader *

#define _BM_GENERIC_TYPE_ELEM_CONST \
	const void *, const BMVert *, const BMEdge *, const BMLoop *, const BMFace *, \
	const BMElem *, const BMElemF *, const BMHeader *, \
	void * const, BMVert * const, BMEdge * const, BMLoop * const, BMFace * const, \
	BMElem * const, BMElemF * const, BMHeader * const

//#define BM_CHECK_TYPE_ELEM_CONST(ele) \
//	CHECK_TYPE_ANY(ele, _BM_GENERIC_TYPES_CONST)
//
//#define BM_CHECK_TYPE_ELEM_NONCONST(ele) \
//	CHECK_TYPE_ANY(ele, _BM_GENERIC_TYPE_ELEM_NONCONST)

#define BM_CHECK_TYPE_ELEM(ele) \
	CHECK_TYPE_ANY(ele, _BM_GENERIC_TYPE_ELEM_NONCONST, _BM_GENERIC_TYPE_ELEM_CONST)

/* Assignment from a void* to a typed pointer is not allowed in C++,
* casting the LHS to void works fine though.
*/
#ifdef __cplusplus
#define BM_CHECK_TYPE_ELEM_ASSIGN(ele) \
	(BM_CHECK_TYPE_ELEM(ele)), *((void **)&ele)
#else
#define BM_CHECK_TYPE_ELEM_ASSIGN(ele) \
	(BM_CHECK_TYPE_ELEM(ele)), ele
#endif


/* BMHeader->hflag (char) */
enum {
	BM_ELEM_SELECT = (1 << 0),
	BM_ELEM_HIDDEN = (1 << 1),
	BM_ELEM_SEAM = (1 << 2),
	/**
	* used for faces and edges, note from the user POV,
	* this is a sharp edge when disabled */
	BM_ELEM_SMOOTH = (1 << 3),
	/**
	* internal flag, used for ensuring correct normals
	* during multires interpolation, and any other time
	* when temp tagging is handy.
	* always assume dirty & clear before use. */
	BM_ELEM_TAG = (1 << 4),

	BM_ELEM_REMOVED = (1 << 5), /*element removed*/

	/* spare tag, assumed dirty, use define in each function to name based on use */
	// _BM_ELEM_TAG_ALT = (1 << 6),  // UNUSED
	/**
	* for low level internal API tagging,
	* since tools may want to tag verts and
	* not have functions clobber them */
	BM_ELEM_INTERNAL_TAG = (1 << 7),
};

#define BM_FACE_FIRST_LOOP(p) ((p)->l_first)


/* defines */
#define BM_ELEM_CD_SET_INT(ele, offset, f) { CHECK_TYPE_NONCONST(ele); \
	assert(offset != -1); *((int *)((char *)(ele)->head.data + (offset))) = (f); } (void)0

#define BM_ELEM_CD_GET_INT(ele, offset) \
	(assert(offset != -1), *((int *)((char *)(ele)->head.data + (offset))))

#if defined(__STDC_VERSION__) && (__STDC_VERSION__ >= 201112L)
#define BM_ELEM_CD_GET_VOID_P(ele, offset) \
	(assert(offset != -1), \
	_Generic(ele, \
		GENERIC_TYPE_ANY(              POINTER_OFFSET((ele)->head.data, offset), _BM_GENERIC_TYPE_ELEM_NONCONST), \
		GENERIC_TYPE_ANY((const void *)POINTER_OFFSET((ele)->head.data, offset), _BM_GENERIC_TYPE_ELEM_CONST)) \
	)
#else
#define BM_ELEM_CD_GET_VOID_P(ele, offset) \
	(assert(offset != -1), (void *)((char *)(ele)->head.data + (offset)))
#endif

#define BM_ELEM_CD_SET_FLOAT(ele, offset, f) { CHECK_TYPE_NONCONST(ele); \
	assert(offset != -1); *((float *)((char *)(ele)->head.data + (offset))) = (f); } (void)0

#define BM_ELEM_CD_GET_FLOAT(ele, offset) \
	(assert(offset != -1), *((float *)((char *)(ele)->head.data + (offset))))

#define BM_ELEM_CD_SET_POINTER(ele, offset, f) { CHECK_TYPE_NONCONST(ele); \
	assert(offset != -1); *((uintptr_t*)((char *)(ele)->head.data + (offset))) = (f); } (void)0

#define BM_ELEM_CD_GET_POINTER(ele, offset) \
	(assert(offset != -1), *((uintptr_t*)((char *)(ele)->head.data + (offset))))

#define BM_ELEM_CD_GET_FLOAT_AS_UCHAR(ele, offset) \
	(assert(offset != -1), (unsigned char)(BM_ELEM_CD_GET_FLOAT(ele, offset) * 255.0f))

/**
* size to use for stack arrays when dealing with NGons,
* alloc after this limit is reached.
* this value is rather arbitrary */
#define BM_DEFAULT_NGON_STACK_SIZE 32
/**
* size to use for stack arrays dealing with connected mesh data
* verts of faces, edges of vert - etc.
* often used with #BM_iter_as_arrayN() */
#define BM_DEFAULT_ITER_STACK_SIZE 16

/* avoid inf loop, this value is arbitrary
* but should not error on valid cases */
#define BM_LOOP_RADIAL_MAX 10000
#define BM_NGON_MAX 100000

/* setting zero so we can catch bugs in OpenMP/BMesh */
#ifdef DEBUG
#  define BM_OMP_LIMIT 0
#else
#  define BM_OMP_LIMIT 10000
#endif

VM_END_NAMESPACE
#endif