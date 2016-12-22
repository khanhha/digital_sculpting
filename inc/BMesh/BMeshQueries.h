#ifndef VM_VMESH_QUERIES_H
#define VM_VMESH_QUERIES_H

#include "BMUtilDefine.h"
#include "BaseLib/UtilMacro.h"
#include "BMeshClass.h"
#include "BMeshInline.h"
#include "BMCompilerTypeCheck.h"

VM_BEGIN_NAMESPACE

#define BM_DISK_EDGE_NEXT(e, v)  ( \
	CHECK_TYPE_INLINE(e, BMEdge *), CHECK_TYPE_INLINE(v, BMVert *), \
	BLI_assert(BM_vert_in_edge(e, v)), \
	(((&e->v1_disk_link)[v == e->v2]).next))

#define BM_DISK_EDGE_PREV(e, v)  ( \
	CHECK_TYPE_INLINE(e, BMEdge *), CHECK_TYPE_INLINE(v, BMVert *), \
	BLI_assert(BM_vert_in_edge(e, v)), \
	(((&e->v1_disk_link)[v == e->v2]).prev))

/**
* Returns whether or not a given vertex is
* is part of a given edge.
*/
ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1)
BLI_INLINE bool BM_vert_in_edge(const BMEdge *e, const BMVert *v)
{
	return (ELEM(v, e->v1, e->v2));
}

/**
* Returns whether or not a given edge is part of a given loop.
*/
ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1, 2)
BLI_INLINE bool BM_edge_in_loop(const BMEdge *e, const BMLoop *l)
{
	return (l->e == e || l->prev->e == e);
}

/**
* Returns whether or not two vertices are in
* a given edge
*/
ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1, 2, 3)
BLI_INLINE bool BM_verts_in_edge(const BMVert *v1, const BMVert *v2, const BMEdge *e)
{
	return ((e->v1 == v1 && e->v2 == v2) ||
		(e->v1 == v2 && e->v2 == v1));
}

/**
* Given a edge and one of its vertices, returns
* the other vertex.
*/
ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1, 2)
BLI_INLINE BMVert *BM_edge_other_vert(BMEdge *e, const BMVert *v)
{
	if (e->v1 == v) {
		return e->v2;
	}
	else if (e->v2 == v) {
		return e->v1;
	}
	return NULL;
}

/**
* Tests whether or not the edge is part of a wire.
* (ie: has no faces attached to it)
*/
ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1)
BLI_INLINE bool BM_edge_is_wire(const BMEdge *e)
{
	return (e->l == NULL);
}

/**
* Tests whether or not this edge is manifold.
* A manifold edge has exactly 2 faces attached to it.
*/

#if 1 /* fast path for checking manifold */
ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1)
BLI_INLINE bool BM_edge_is_manifold(const BMEdge *e)
{
	const BMLoop *l = e->l;
	return (l && (l->radial_next != l) &&             /* not 0 or 1 face users */
		(l->radial_next->radial_next == l)); /* 2 face users */
}
#else
BLI_INLINE int BM_edge_is_manifold(BMEdge *e)
{
	return (BM_edge_face_count(e) == 2);
}
#endif

/**
* Tests that the edge is manifold and
* that both its faces point the same way.
*/
ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1)
BLI_INLINE bool BM_edge_is_contiguous(const BMEdge *e)
{
	const BMLoop *l = e->l;
	const BMLoop *l_other;
	return (l && ((l_other = l->radial_next) != l) &&  /* not 0 or 1 face users */
		(l_other->radial_next == l) &&        /* 2 face users */
		(l_other->v != l->v));
}

/**
* Tests whether or not an edge is on the boundary
* of a shell (has one face associated with it)
*/

#if 1 /* fast path for checking boundary */
ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1)
BLI_INLINE bool BM_edge_is_boundary(const BMEdge *e)
{
	const BMLoop *l = e->l;
	return (l && (l->radial_next == l));
}
#else
BLI_INLINE int BM_edge_is_boundary(BMEdge *e)
{
	return (BM_edge_face_count(e) == 1);
}
#endif

ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1)
BLI_INLINE int BM_edge_is_manifold_or_boundary(BMLoop *l)
{
#if 0
	/* less optimized version of check below */
	return (BM_edge_is_manifold(l->e) || BM_edge_is_boundary(l->e);
#else
	/* if the edge is a boundary it points to its self, else this must be a manifold */
	return LIKELY(l) && LIKELY(l->radial_next->radial_next == l);
#endif
}

/**
* Tests whether one loop is next to another within the same face.
*/
ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1, 2)
BLI_INLINE bool BM_loop_is_adjacent(const BMLoop *l_a, const BMLoop *l_b)
{
	BLI_assert(l_a->f == l_b->f);
	BLI_assert(l_a != l_b);
	return (ELEM(l_b, l_a->next, l_a->prev));
}

/**
* Check if we have a single wire edge user.
*/
ATTR_WARN_UNUSED_RESULT ATTR_NONNULL(1)
BLI_INLINE bool BM_vert_is_wire_endpoint(const BMVert *v)
{
	const BMEdge *e = v->e;
	if (e && e->l == NULL) {
		return (BM_DISK_EDGE_NEXT(e, v) == e);
	}
	return false;
}


bool    BM_vert_in_face(BMVert *v, BMFace *f) ;
int     BM_verts_in_face_count(BMVert **varr, int len, BMFace *f) ;
bool    BM_verts_in_face(BMVert **varr, int len, BMFace *f) ;
int     BM_vert_edge_count_ex(const BMVert *v, const int count_max) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
int     BM_vert_edge_count(const BMVert *v) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
bool    BM_vert_is_boundary(const BMVert *v) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
bool    BM_vert_pair_share_face_check(BMVert *v_a, BMVert *v_b) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
bool    BM_vert_face_check(BMVert *v) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

bool    BM_edge_in_face(const BMEdge *e, const BMFace *f) ;
BMEdge *BM_edge_exists(BMVert *v1, BMVert *v2);
bool    BM_edge_face_pair(BMEdge *e, BMFace **r_fa, BMFace **r_fb) ATTR_NONNULL();
bool    BM_edge_loop_pair(BMEdge *e, BMLoop **r_la, BMLoop **r_lb) ATTR_NONNULL();
float   BM_edge_calc_length(const BMEdge *e) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
float   BM_edge_calc_length_squared(const BMEdge *e) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
BMVert *BM_edge_share_vert(BMEdge *e1, BMEdge *e2) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
BMLoop *BM_edge_vert_share_loop(BMLoop *l, BMVert *v) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
void    BM_edge_ordered_verts(const BMEdge *edge, BMVert **r_v1, BMVert **r_v2) ATTR_NONNULL();
void    BM_edge_ordered_verts_ex(const BMEdge *edge, BMVert **r_v1, BMVert **r_v2, const BMLoop *edge_loop) ATTR_NONNULL();
bool    BM_edge_is_convex(const BMEdge *e) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
BMLoop *BM_edge_other_loop(BMEdge *e, BMLoop *l) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
BMEdge *BM_edge_find_double(BMEdge *e) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();


int     BM_edge_face_count_ex(const BMEdge *e, const int count_max) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
int     BM_edge_face_count(const BMEdge *e) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

BMLoop *BM_face_vert_share_loop(BMFace *f, BMVert *v) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
BMLoop *BM_face_edge_share_loop(BMFace *f, BMEdge *e) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
int		BM_face_share_edge_count(BMFace *f_a, BMFace *f_b);

bool    BM_face_exists(BMVert **varr, int len, BMFace **r_existface);
BMLoop *BM_face_other_vert_loop(BMFace *f, BMVert *v_prev, BMVert *v) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
BMVert *BM_face_tri_other_vert(BMFace *f, BMVert *v1, BMVert *v2);
bool	BM_face_is_normal_valid(const BMFace *f);

float   BM_edge_calc_face_angle_ex(const BMEdge *e, const float fallback) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
float   BM_edge_calc_face_angle(const BMEdge *e) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
float   BM_edge_calc_face_angle_signed_ex(const BMEdge *e, const float fallback) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
float   BM_edge_calc_face_angle_signed(const BMEdge *e) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
void    BM_edge_calc_face_tangent(const BMEdge *e, const BMLoop *e_loop, Vector3f &r_tangent) ATTR_NONNULL();

bool    BM_edge_rotate_check(BMEdge *e);
void    BM_edge_calc_rotate(BMEdge *e, const bool ccw, BMLoop **r_l1, BMLoop **r_l2);

bool	BM_edge_collapse_is_degenerate_flip(BMEdge *e, const Vector3f &target_co);
bool	BM_edge_tri_collapse_is_degenerate_topology(BMEdge *e_first);
bool    BM_edge_face_collapse_is_degenearte_topology_general(BMEdge *e);
VM_END_NAMESPACE

#endif