#include "BMeshStructure.h"
#include "BMeshQueries.h"

VM_BEGIN_NAMESPACE


/*****loop cycle functions, e.g. loops surrounding a face**** */
bool bmesh_loop_validate(BMFace *f)
{
	int i;
	int len = f->len;
	BMLoop *l_iter, *l_first;

	l_first = BM_FACE_FIRST_LOOP(f);

	if (l_first == NULL) {
		return false;
	}

	/* Validate that the face loop cycle is the length specified by f->len */
	for (i = 1, l_iter = l_first->next; i < len; i++, l_iter = l_iter->next) {
		if ((l_iter->f != f) ||
			(l_iter == l_first))
		{
			return false;
		}
	}
	if (l_iter != l_first) {
		return false;
	}

	/* Validate the loop->prev links also form a cycle of length f->len */
	for (i = 1, l_iter = l_first->prev; i < len; i++, l_iter = l_iter->prev) {
		if (l_iter == l_first) {
			return false;
		}
	}
	if (l_iter != l_first) {
		return false;
	}

	return true;
}

BMDiskLink * bmesh_disk_edge_link_from_vert(const BMEdge *e, const BMVert *v)
{
	BLI_assert(BM_vert_in_edge(e, v));
	return (BMDiskLink *)&(&e->v1_disk_link)[v == e->v2];
}


/**
* \section bm_cycles BMesh Cycles
* (this is somewhat outdate, though bits of its API are still used) - joeedh
*
* Cycles are circular doubly linked lists that form the basis of adjacency
* information in the BME modeler. Full adjacency relations can be derived
* from examining these cycles very quickly. Although each cycle is a double
* circular linked list, each one is considered to have a 'base' or 'head',
* and care must be taken by Euler code when modifying the contents of a cycle.
*
* The contents of this file are split into two parts. First there are the
* bmesh_cycle family of functions which are generic circular double linked list
* procedures. The second part contains higher level procedures for supporting
* modification of specific cycle types.
*
* The three cycles explicitly stored in the BM data structure are as follows:
*
*
* 1: The Disk Cycle - A circle of edges around a vertex
* Base: vertex->edge pointer.
*
* This cycle is the most complicated in terms of its structure. Each bmesh_Edge contains
* two bmesh_CycleNode structures to keep track of that edges membership in the disk cycle
* of each of its vertices. However for any given vertex it may be the first in some edges
* in its disk cycle and the second for others. The bmesh_disk_XXX family of functions contain
* some nice utilities for navigating disk cycles in a way that hides this detail from the
* tool writer.
*
* Note that the disk cycle is completely independent from face data. One advantage of this
* is that wire edges are fully integrated into the topology database. Another is that the
* the disk cycle has no problems dealing with non-manifold conditions involving faces.
*
* Functions relating to this cycle:
* - #bmesh_disk_vert_replace
* - #bmesh_disk_edge_append
* - #bmesh_disk_edge_remove
* - #bmesh_disk_edge_next
* - #bmesh_disk_edge_prev
* - #bmesh_disk_facevert_count
* - #bmesh_disk_faceedge_find_first
* - #bmesh_disk_faceedge_find_next
*
*
* 2: The Radial Cycle - A circle of face edges (bmesh_Loop) around an edge
* Base: edge->l->radial structure.
*
* The radial cycle is similar to the radial cycle in the radial edge data structure.*
* Unlike the radial edge however, the radial cycle does not require a large amount of memory
* to store non-manifold conditions since BM does not keep track of region/shell information.
*
* Functions relating to this cycle:
* - #bmesh_radial_append
* - #bmesh_radial_loop_remove
* - #bmesh_radial_facevert_count
* - #bmesh_radial_facevert_check
* - #bmesh_radial_faceloop_find_first
* - #bmesh_radial_faceloop_find_next
* - #bmesh_radial_validate
*
*
* 3: The Loop Cycle - A circle of face edges around a polygon.
* Base: polygon->lbase.
*
* The loop cycle keeps track of a faces vertices and edges. It should be noted that the
* direction of a loop cycle is either CW or CCW depending on the face normal, and is
* not oriented to the faces editedges.
*
* Functions relating to this cycle:
* - bmesh_cycle_XXX family of functions.
*
*
* \note the order of elements in all cycles except the loop cycle is undefined. This
* leads to slightly increased seek time for deriving some adjacency relations, however the
* advantage is that no intrinsic properties of the data structures are dependent upon the
* cycle order and all non-manifold conditions are represented trivially.
*/
void bmesh_disk_edge_append(BMEdge *e, BMVert *v)
{
	if (!v->e) {
		BMDiskLink *dl1 = bmesh_disk_edge_link_from_vert(e, v);

		v->e = e;
		dl1->next = dl1->prev = e;
	}
	else {
		BMDiskLink *dl1, *dl2, *dl3;

		dl1 = bmesh_disk_edge_link_from_vert(e, v);
		dl2 = bmesh_disk_edge_link_from_vert(v->e, v);
		dl3 = dl2->prev ? bmesh_disk_edge_link_from_vert(dl2->prev, v) : NULL;

		dl1->next = v->e;
		dl1->prev = dl2->prev;

		dl2->prev = e;
		if (dl3)
			dl3->next = e;
	}
}

void bmesh_disk_edge_remove(BMEdge *e, BMVert *v)
{
	BMDiskLink *dl1, *dl2;

	dl1 = bmesh_disk_edge_link_from_vert(e, v);
	if (dl1->prev) {
		dl2 = bmesh_disk_edge_link_from_vert(dl1->prev, v);
		dl2->next = dl1->next;
	}

	if (dl1->next) {
		dl2 = bmesh_disk_edge_link_from_vert(dl1->next, v);
		dl2->prev = dl1->prev;
	}

	if (v->e == e)
		v->e = (e != dl1->next) ? dl1->next : NULL;

	dl1->next = dl1->prev = NULL;
}

BMEdge * bmesh_disk_edge_next(const BMEdge *e, const BMVert *v)
{
	if (v == e->v1)
		return e->v1_disk_link.next;
	if (v == e->v2)
		return e->v2_disk_link.next;
	return NULL;
}

BMEdge * bmesh_disk_edge_prev(const BMEdge *e, const BMVert *v)
{
	if (v == e->v1)
		return e->v1_disk_link.prev;
	if (v == e->v2)
		return e->v2_disk_link.prev;
	return NULL;
}

void bmesh_disk_vert_replace(BMEdge *e, BMVert *v_dst, BMVert *v_src)
{
	BLI_assert(e->v1 == v_src || e->v2 == v_src);
	bmesh_disk_edge_remove(e, v_src);		/* remove e from tv's disk cycle */
	bmesh_disk_vert_swap(e, v_dst, v_src);	/* swap out tv for v_new in e */
	bmesh_disk_edge_append(e, v_dst);		/* add e to v_dst's disk cycle */
	BLI_assert(e->v1 != e->v2);
}


void bmesh_disk_vert_swap(BMEdge *e, BMVert *v_dst, BMVert *v_src)
{
	if (e->v1 == v_src) {
		e->v1 = v_dst;
		e->v1_disk_link.next = e->v1_disk_link.prev = NULL;
	}
	else if (e->v2 == v_src) {
		e->v2 = v_dst;
		e->v2_disk_link.next = e->v2_disk_link.prev = NULL;
	}
	else {
		BLI_assert(0);
	}
}

bool bmesh_disk_validate(int len, BMEdge *e, BMVert *v)
{
	BMEdge *e_iter;

	if (!BM_vert_in_edge(e, v))
		return false;
	if (bmesh_disk_count_ex(v, len + 1) != len || len == 0)
		return false;

	e_iter = e;
	do {
		if (len != 1 && bmesh_disk_edge_prev(e_iter, v) == e_iter) {
			return false;
		}
	} while ((e_iter = bmesh_disk_edge_next(e_iter, v)) != e);

	return true;
}

int bmesh_disk_count(const BMVert *v)
{
	int count = 0;
	if (v->e) {
		BMEdge *e_first, *e_iter;
		e_iter = e_first = v->e;
		do {
			count++;
		} while ((e_iter = bmesh_disk_edge_next(e_iter, v)) != e_first);
	}
	return count;
}

int bmesh_disk_count_ex(const BMVert *v, const int count_max)
{
	int count = 0;
	if (v->e) {
		BMEdge *e_first, *e_iter;
		e_iter = e_first = v->e;
		do {
			count++;
			if (count == count_max) {
				break;
			}
		} while ((e_iter = bmesh_disk_edge_next(e_iter, v)) != e_first);
	}
	return count;
}

void bmesh_radial_append(BMEdge *e, BMLoop *l)
{
	if (e->l == NULL) {
		e->l = l;
		l->radial_next = l->radial_prev = l;
	}
	else {
		l->radial_prev = e->l;
		l->radial_next = e->l->radial_next;

		e->l->radial_next->radial_prev = l;
		e->l->radial_next = l;

		e->l = l;
	}

	if ((l->e && l->e != e)) {
		/* l is already in a radial cycle for a different edge */
		BMESH_ASSERT(0);
	}

	l->e = e;
}

/**
* \brief BMESH RADIAL REMOVE LOOP
*
* Removes a loop from an radial cycle. If edge e is non-NULL
* it should contain the radial cycle, and it will also get
* updated (in the case that the edge's link into the radial
* cycle was the loop which is being removed from the cycle).
*/
void bmesh_radial_loop_remove(BMLoop *l, BMEdge *e)
{
	/* if e is non-NULL, l must be in the radial cycle of e */
	if ((e && e != l->e)) {
		BMESH_ASSERT(0);
	}

	if (l->radial_next != l) {
		if (e && l == e->l)
			e->l = l->radial_next;

		l->radial_next->radial_prev = l->radial_prev;
		l->radial_prev->radial_next = l->radial_next;
	}
	else {
		if (e) {
			if (l == e->l) {
				e->l = NULL;
			}
			else {
				BMESH_ASSERT(0);
			}
		}
	}

	/* l is no longer in a radial cycle; empty the links
	* to the cycle and the link back to an edge */
	l->radial_next = l->radial_prev = NULL;
	l->e = NULL;
}

int bmesh_radial_length(const BMLoop *l)
{
	const BMLoop *l_iter = l;
	int i = 0;

	if (!l)
		return 0;

	do {
		if ((!l_iter)) {
			/* radial cycle is broken (not a circulat loop) */
			BMESH_ASSERT(0);
			return 0;
		}

		i++;
		if (UNLIKELY(i >= BM_LOOP_RADIAL_MAX)) {
			BMESH_ASSERT(0);
			return -1;
		}
	} while ((l_iter = l_iter->radial_next) != l);

	return i;
}

/*****radial cycle functions, e.g. loops surrounding edges**** */
bool bmesh_radial_validate(int radlen, BMLoop *l)
{
	BMLoop *l_iter = l;
	int i = 0;

	if (bmesh_radial_length(l) != radlen)
		return false;

	do {
		if (UNLIKELY(!l_iter)) {
			BMESH_ASSERT(0);
			return false;
		}

		if (l_iter->e != l->e)
			return false;
		if (l_iter->v != l->e->v1 && l_iter->v != l->e->v2)
			return false;

		if (UNLIKELY(i > BM_LOOP_RADIAL_MAX)) {
			BMESH_ASSERT(0);
			return false;
		}

		i++;
	} while ((l_iter = l_iter->radial_next) != l);

	return true;
}



int bmesh_elem_loop_check(BMLoop *l)
{
	int err = 0;

	if (!BM_vert_in_edge(l->e, l->v)) {
		fprintf(stderr, "%s: fatal bmesh error (vert not in edge)! (bmesh internal error)\n", __FUNCTION__);
		err |= (1 << 11);
	}

	if (l->radial_next == NULL || l->radial_prev == NULL)
		err |= (1 << 12);
	if (l->f->len <= 0)
		err |= (1 << 13);

	/* validate boundary loop -- invalid for hole loops, of course,
	* but we won't be allowing those for a while yet */
	BMLoop *l2 = l;
	size_t i = 0;
	do {
		if (i >= BM_NGON_MAX) {
			break;
		}

		i++;
	} while ((l2 = l2->next) != l);

	if (i != l->f->len || l2 != l)
		err |= (1 << 14);

	if (!bmesh_radial_validate(bmesh_radial_length(l), l))
		err |= (1 << 15);

	BMESH_ASSERT(err == 0);

	return err;
}

/**
* Check the element is valid.
*
* BMESH_TODO, when this raises an error the output is incredible confusing.
* need to have some nice way to print/debug what the hecks going on.
*/
int bmesh_elem_check(void *element, const char htype)
{
	BMHeader *head = (BMHeader*)element;
	int err = 0;

	if (!element)
		return (1 << 0);

	if (head->htype != htype)
		return (1 << 1);

	switch (htype) {
	case BM_VERT:
	{
		BMVert *v = (BMVert*)element;
		if (v->e && v->e->head.htype != BM_EDGE) {
			err |= (1 << 2);
		}
		break;
	}
	case BM_EDGE:
	{
		BMEdge *e = (BMEdge*)element;
#ifdef USE_BMLOOP_HEAD
		if (e->l && e->l->head.htype != BM_LOOP)
			err |= (1 << 3);
#endif
		if (e->l && e->l->f->head.htype != BM_FACE)
			err |= (1 << 4);
		if (e->v1_disk_link.prev == NULL ||
			e->v2_disk_link.prev == NULL ||
			e->v1_disk_link.next == NULL ||
			e->v2_disk_link.next == NULL)
		{
			err |= (1 << 5);
		}
		if (e->l && (e->l->radial_next == NULL || e->l->radial_prev == NULL))
			err |= (1 << 6);
		if (e->l && e->l->f->len <= 0)
			err |= (1 << 7);
		break;
	}
	case BM_LOOP:
	{
		BMLoop *l = (BMLoop*)element, *l2;
		int i;

		if (l->f->head.htype != BM_FACE)
			err |= (1 << 8);
		if (l->e->head.htype != BM_EDGE)
			err |= (1 << 9);
		if (l->v->head.htype != BM_VERT)
			err |= (1 << 10);
		if (!BM_vert_in_edge(l->e, l->v)) {
			fprintf(stderr, "%s: fatal bmesh error (vert not in edge)! (bmesh internal error)\n", __FUNCTION__);
			err |= (1 << 11);
		}

		if (l->radial_next == NULL || l->radial_prev == NULL)
			err |= (1 << 12);
		if (l->f->len <= 0)
			err |= (1 << 13);

		/* validate boundary loop -- invalid for hole loops, of course,
		* but we won't be allowing those for a while yet */
		l2 = l;
		i = 0;
		do {
			if (i >= BM_NGON_MAX) {
				break;
			}

			i++;
		} while ((l2 = l2->next) != l);

		if (i != l->f->len || l2 != l)
			err |= (1 << 14);

		if (!bmesh_radial_validate(bmesh_radial_length(l), l))
			err |= (1 << 15);

		break;
	}
	case BM_FACE:
	{
		BMFace *f = (BMFace*)element;
		BMLoop *l_iter;
		BMLoop *l_first;
		int len = 0;

		if (!f->l_first)
		{
			err |= (1 << 16);
		}
		l_iter = l_first = BM_FACE_FIRST_LOOP(f);
		do {
			if (l_iter->f != f) {
				fprintf(stderr, "%s: loop inside one face points to another! (bmesh internal error)\n", __FUNCTION__);
				err |= (1 << 17);
			}

			if (!l_iter->e)
				err |= (1 << 18);
			if (!l_iter->v)
				err |= (1 << 19);
			if (l_iter->e && l_iter->v) {
				if (!BM_vert_in_edge(l_iter->e, l_iter->v) || !BM_vert_in_edge(l_iter->e, l_iter->next->v)) {
					err |= (1 << 20);
				}

				if (!bmesh_radial_validate(bmesh_radial_length(l_iter), l_iter))
					err |= (1 << 21);

				if (!bmesh_disk_count(l_iter->v) || !bmesh_disk_count(l_iter->next->v))
					err |= (1 << 22);
			}

			len++;
		} while ((l_iter = l_iter->next) != l_first);

		if (len != f->len)
			err |= (1 << 23);
		break;
	}
	default:
		BLI_assert(0);
		break;
	}

	BMESH_ASSERT(err == 0);

	return err;
}

/**
* \brief DISK COUNT FACE VERT
*
* Counts the number of loop users
* for this vertex. Note that this is
* equivalent to counting the number of
* faces incident upon this vertex
*/
int bmesh_disk_facevert_count(const BMVert *v)
{
	/* is there an edge on this vert at all */
	int count = 0;
	if (v->e) {
		BMEdge *e_first, *e_iter;

		/* first, loop around edge */
		e_first = e_iter = v->e;
		do {
			if (e_iter->l) {
				count += bmesh_radial_facevert_count(e_iter->l, v);
			}
		} while ((e_iter = bmesh_disk_edge_next(e_iter, v)) != e_first);
	}
	return count;
}

/**
* \brief RADIAL COUNT FACE VERT
*
* Returns the number of times a vertex appears
* in a radial cycle
*/
int bmesh_radial_facevert_count(const BMLoop *l, const BMVert *v)
{
	const BMLoop *l_iter;
	int count = 0;
	l_iter = l;
	do {
		if (l_iter->v == v) {
			count++;
		}
	} while ((l_iter = l_iter->radial_next) != l);

	return count;
}

/**
* \brief FIND FIRST FACE EDGE
*
* Finds the first edge in a vertices
* Disk cycle that has one of this
* vert's loops attached
* to it.
*/
BMEdge * bmesh_disk_faceedge_find_first(const BMEdge *e, const BMVert *v)
{
	const BMEdge *e_find = e;
	do {
		if (e_find->l && bmesh_radial_facevert_check(e_find->l, v)) {
			return (BMEdge *)e_find;
		}
	} while ((e_find = bmesh_disk_edge_next(e_find, v)) != e);

	return NULL;
}

bool bmesh_radial_facevert_check(const BMLoop *l, const BMVert *v)
{
	const BMLoop *l_iter;
	l_iter = l;
	do {
		if (l_iter->v == v) {
			return true;
		}
	} while ((l_iter = l_iter->radial_next) != l);

	return false;
}

/**
* \brief BME RADIAL FIND FIRST FACE VERT
*
* Finds the first loop of v around radial
* cycle
*/
BMLoop * bmesh_radial_faceloop_find_first(const BMLoop *l, const BMVert *v)
{
	const BMLoop *l_iter;
	l_iter = l;
	do {
		if (l_iter->v == v) {
			return (BMLoop *)l_iter;
		}
	} while ((l_iter = l_iter->radial_next) != l);
	return NULL;
}

BMLoop * bmesh_radial_faceloop_find_next(const BMLoop *l, const BMVert *v)
{
	BMLoop *l_iter;
	l_iter = l->radial_next;
	do {
		if (l_iter->v == v) {
			return l_iter;
		}
	} while ((l_iter = l_iter->radial_next) != l);
	return (BMLoop *)l;
}

BMEdge * bmesh_disk_faceedge_find_next(const BMEdge *e, const BMVert *v)
{
	BMEdge *e_find;
	e_find = bmesh_disk_edge_next(e, v);
	do {
		if (e_find->l && bmesh_radial_facevert_check(e_find->l, v)) {
			return e_find;
		}
	} while ((e_find = bmesh_disk_edge_next(e_find, v)) != e);
	return (BMEdge *)e;
}

/**
* Handles all connected data, use with care.
*
* Assumes caller has setup correct state before the swap is done.
*/
void bmesh_edge_vert_swap(BMEdge *e, BMVert *v_dst, BMVert *v_src)
{
	/* swap out loops */
	if (e->l) {
		BMLoop *l_iter, *l_first;
		l_iter = l_first = e->l;
		do {
			if (l_iter->v == v_src) {
				l_iter->v = v_dst;
			}
			else if (l_iter->next->v == v_src) {
				l_iter->next->v = v_dst;
			}
			else {
				BLI_assert(l_iter->prev->v != v_src);
			}
		} while ((l_iter = l_iter->radial_next) != l_first);
	}

	/* swap out edges */
	bmesh_disk_vert_replace(e, v_dst, v_src);
}



VM_END_NAMESPACE

