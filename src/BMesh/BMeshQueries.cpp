
#include "BMeshQueries.h"
#include "BMeshIterators.h"
#include "BMeshStructure.h"
#include "BaseLib/MathUtil.h"
#include "BaseLib/MathBase.h"
#include "BMeshPolygon.h"

VM_BEGIN_NAMESPACE

/**
* Returns true if the vertex is used in a given face.
*/
bool BM_vert_in_face(BMVert *v, BMFace *f)
{
	BMLoop *l_iter, *l_first;

	{
		l_iter = l_first = f->l_first;
		do {
			if (l_iter->v == v) {
				return true;
			}
		} while ((l_iter = l_iter->next) != l_first);
	}

	return false;
}

/**
* Compares the number of vertices in an array
* that appear in a given face
*/
int BM_verts_in_face_count(BMVert **varr, int len, BMFace *f)
{
	BMLoop *l_iter, *l_first;

	int i, count = 0;

	for (i = 0; i < len; i++) {
		BM_ELEM_API_FLAG_ENABLE(varr[i], _FLAG_OVERLAP);
	}

	{
		l_iter = l_first = f->l_first;

		do {
			if (BM_ELEM_API_FLAG_TEST(l_iter->v, _FLAG_OVERLAP)) {
				count++;
			}

		} while ((l_iter = l_iter->next) != l_first);
	}

	for (i = 0; i < len; i++) {
		BM_ELEM_API_FLAG_DISABLE(varr[i], _FLAG_OVERLAP);
	}

	return count;
}

/**
* Return true if all verts are in the face.
*/
bool BM_verts_in_face(BMVert **varr, int len, BMFace *f) 
{
	BMLoop *l_iter, *l_first;

	int i;
	bool ok = true;

	/* simple check, we know can't succeed */
	if (f->len < len) {
		return false;
	}

	for (i = 0; i < len; i++) {
		BM_ELEM_API_FLAG_ENABLE(varr[i], _FLAG_OVERLAP);
	}

	{

		l_iter = l_first = f->l_first;
		
		do {
			if (BM_ELEM_API_FLAG_TEST(l_iter->v, _FLAG_OVERLAP)) {
				/* pass */
			}
			else {
				ok = false;
				break;
			}

		} while ((l_iter = l_iter->next) != l_first);
	}

	for (i = 0; i < len; i++) {
		BM_ELEM_API_FLAG_DISABLE(varr[i], _FLAG_OVERLAP);
	}

	return ok;
}

int BM_vert_edge_count_ex(const BMVert *v, const int count_max)
{
	return bmesh_disk_count_ex(v, count_max);
}

/**
*	Returns the number of edges around this vertex.
*/
int BM_vert_edge_count(const BMVert *v)
{
	return bmesh_disk_count(v);
}

bool BM_vert_is_boundary(const BMVert *v)
{
	if (v->e) {
		BMEdge *e_first, *e_iter;

		e_first = e_iter = v->e;
		do {
			if (BM_edge_is_boundary(e_iter)) {
				return true;
			}
		} while ((e_iter = bmesh_disk_edge_next(e_iter, v)) != e_first);

		return false;
	}
	else {
		return false;
	}
}

/**
* Check if verts share a face.
*/
bool BM_vert_pair_share_face_check(BMVert *v_a, BMVert *v_b)
{
	if (v_a->e && v_b->e) {
		BMIter iter;
		BMFace *f;

		BM_ITER_ELEM(f, &iter, v_a, BM_FACES_OF_VERT) {
			if (BM_vert_in_face(v_b, f)) {
				return true;
			}
		}
	}

	return false;
}

/**
* Return true if the vertex is connected to _any_ faces.
*
* same as ``BM_vert_face_count(v) != 0`` or ``BM_vert_find_first_loop(v) == NULL``
*/
bool BM_vert_face_check(BMVert *v)
{
	return v->e && (bmesh_disk_faceedge_find_first(v->e, v) != NULL);
}


/**
* Check if the collapse will result in a degenerate mesh,
* that is - duplicate edges or faces.
*
* This situation could be checked for when calculating collapse cost
* however its quite slow and a degenerate collapse could eventuate
* after the cost is calculated, so instead, check just before collapsing.
*/

static void bm_edge_tag_enable(BMEdge *e)
{
	BM_elem_flag_enable(e->v1, BM_ELEM_TAG);
	BM_elem_flag_enable(e->v2, BM_ELEM_TAG);
	if (e->l) {
		BM_elem_flag_enable(e->l->f, BM_ELEM_TAG);
		if (e->l != e->l->radial_next) {
			BM_elem_flag_enable(e->l->radial_next->f, BM_ELEM_TAG);
		}
	}
}

static void bm_edge_tag_disable(BMEdge *e)
{
	BM_elem_flag_disable(e->v1, BM_ELEM_TAG);
	BM_elem_flag_disable(e->v2, BM_ELEM_TAG);
	if (e->l) {
		BM_elem_flag_disable(e->l->f, BM_ELEM_TAG);
		if (e->l != e->l->radial_next) {
			BM_elem_flag_disable(e->l->radial_next->f, BM_ELEM_TAG);
		}
	}
}

static bool bm_edge_tag_test(BMEdge *e)
{
	/* is the edge or one of its faces tagged? */
	return (BM_elem_flag_test(e->v1, BM_ELEM_TAG) ||
		BM_elem_flag_test(e->v2, BM_ELEM_TAG) ||
		(e->l && (BM_elem_flag_test(e->l->f, BM_ELEM_TAG) ||
		(e->l != e->l->radial_next &&
		BM_elem_flag_test(e->l->radial_next->f, BM_ELEM_TAG))))
		);
}

bool    BM_edge_face_collapse_is_degenearte_topology_general(BMEdge *e_first)
{
	BMEdge *e_iter;
	BMVert *v_other;
	/* clear flags on both disks */
	e_iter = e_first;
	do {
		BM_elem_flag_disable(e_iter->v1, BM_ELEM_TAG);
		BM_elem_flag_disable(e_iter->v2, BM_ELEM_TAG);
	} while ((e_iter = bmesh_disk_edge_next(e_iter, e_first->v1)) != e_first);

	e_iter = e_first;
	do {
		BM_elem_flag_disable(e_iter->v1, BM_ELEM_TAG);
		BM_elem_flag_disable(e_iter->v2, BM_ELEM_TAG);
	} while ((e_iter = bmesh_disk_edge_next(e_iter, e_first->v2)) != e_first);


	/*tag all adjacent vertices to v1*/
	e_iter = e_first;
	do {
		v_other = BM_edge_other_vert(e_iter, e_first->v1);
		BM_elem_flag_enable(v_other, BM_ELEM_TAG);
	} while ((e_iter = bmesh_disk_edge_next(e_iter, e_first->v1)) != e_first);

	/*check*/
	bool found_common_vert = false;
	e_iter = e_first;
	do {
		v_other = BM_edge_other_vert(e_iter, e_first->v2);
		if (v_other != e_first->v1 && BM_elem_flag_test(v_other, BM_ELEM_TAG)){
			found_common_vert = true;
			break;
		}
	} while ((e_iter = bmesh_disk_edge_next(e_iter, e_first->v2)) != e_first);

	return found_common_vert;
}
bool BM_edge_tri_collapse_is_degenerate_topology(BMEdge *e_first)
{
	/* simply check that there is no overlap between faces and edges of each vert,
	* (excluding the 2 faces attached to 'e' and 'e' its self) */

	BMEdge *e_iter;

	/* clear flags on both disks */
	e_iter = e_first;
	do {
		if (!BM_edge_is_manifold_or_boundary(e_iter->l)) {
			return true;
		}
		bm_edge_tag_disable(e_iter);
	} while ((e_iter = bmesh_disk_edge_next(e_iter, e_first->v1)) != e_first);

	e_iter = e_first;
	do {
		if (!BM_edge_is_manifold_or_boundary(e_iter->l)) {
			return true;
		}
		bm_edge_tag_disable(e_iter);
	} while ((e_iter = bmesh_disk_edge_next(e_iter, e_first->v2)) != e_first);

	/* now enable one side... */
	e_iter = e_first;
	do {
		bm_edge_tag_enable(e_iter);
	} while ((e_iter = bmesh_disk_edge_next(e_iter, e_first->v1)) != e_first);

	/* ... except for the edge we will collapse, we know thats shared,
	* disable this to avoid false positive. We could be smart and never enable these
	* face/edge tags in the first place but easier to do this */
	// bm_edge_tag_disable(e_first);
	/* do inline... */
	{
#if 0
		BMIter iter;
		BMIter liter;
		BMLoop *l;
		BMVert *v;
		BM_ITER_ELEM(l, &liter, e_first, BM_LOOPS_OF_EDGE) {
			BM_elem_flag_disable(l->f, BM_ELEM_TAG);
			BM_ITER_ELEM(v, &iter, l->f, BM_VERTS_OF_FACE) {
				BM_elem_flag_disable(v, BM_ELEM_TAG);
			}
		}
#else
		/* we know each face is a triangle, no looping/iterators needed here */

		BMLoop *l_radial;
		BMLoop *l_face;

		l_radial = e_first->l;
		l_face = l_radial;
		BLI_assert(l_face->f->len == 3);
		BM_elem_flag_disable(l_face->f, BM_ELEM_TAG);
		BM_elem_flag_disable((l_face = l_radial)->v, BM_ELEM_TAG);
		BM_elem_flag_disable((l_face = l_face->next)->v, BM_ELEM_TAG);
		BM_elem_flag_disable((l_face->next)->v, BM_ELEM_TAG);
		l_face = l_radial->radial_next;
		if (l_radial != l_face) {
			BLI_assert(l_face->f->len == 3);
			BM_elem_flag_disable(l_face->f, BM_ELEM_TAG);
			BM_elem_flag_disable((l_face = l_radial->radial_next)->v, BM_ELEM_TAG);
			BM_elem_flag_disable((l_face = l_face->next)->v, BM_ELEM_TAG);
			BM_elem_flag_disable((l_face->next)->v, BM_ELEM_TAG);
		}
#endif
	}

	/* and check for overlap */
	e_iter = e_first;
	do {
		if (bm_edge_tag_test(e_iter)) {
			return true;
		}
	} while ((e_iter = bmesh_disk_edge_next(e_iter, e_first->v2)) != e_first);

	return false;
}


bool BM_edge_collapse_is_degenerate_flip(BMEdge *e, const Vector3f &target_co)
{
	BMIter liter;
	BMLoop *l;
	unsigned int i;
	Vector3f vec_other;  /* line between the two outer verts, re-use for both cross products */
	Vector3f vec_exist;  /* before collapse */
	Vector3f vec_optim;  /* after collapse */
	Vector3f cross_exist;
	Vector3f cross_optim;

	for (i = 0; i < 2; i++) {
		/* loop over both verts */
		BMVert *v = *((&e->v1) + i);

		BM_ITER_ELEM(l, &liter, v, BM_LOOPS_OF_VERT) {
			if (l->e != e && l->prev->e != e) {
				const Vector3f &co_prev = l->prev->v->co;
				const Vector3f &co_next = l->next->v->co;

				vec_other = co_prev - co_next;
				vec_exist = co_prev - v->co;
				vec_optim = co_prev - target_co;

				cross_exist = vec_other.cross(vec_exist);
				cross_optim = vec_other.cross(vec_optim);

				/* avoid normalize */
				if (cross_exist.dot(cross_optim) <=
					(cross_exist.squaredNorm() + cross_optim.squaredNorm()) * 0.01f)
				{
					return true;
				}
			}
		}
	}

	return false;
}


/**
* Returns whether or not a given edge is part of a given face.
*/
bool BM_edge_in_face(const BMEdge *e, const BMFace *f) 
{
	if (e->l) {
		const BMLoop *l_iter, *l_first;

		l_iter = l_first = e->l;
		do {
			if (l_iter->f == f) {
				return true;
			}
		} while ((l_iter = l_iter->radial_next) != l_first);
	}

	return false;
}


BMEdge * BM_edge_exists(BMVert *v_a, BMVert *v_b)
{
	/* speedup by looping over both edges verts
	* where one vert may connect to many edges but not the other. */

	BMEdge *e_a, *e_b;

	BLI_assert(v_a != v_b);
	BLI_assert(v_a->head.htype == BM_VERT && v_b->head.htype == BM_VERT);

	if ((e_a = v_a->e) && (e_b = v_b->e)) {
		BMEdge *e_a_iter = e_a, *e_b_iter = e_b;

		do {
			if (BM_vert_in_edge(e_a_iter, v_b)) {
				return e_a_iter;
			}
			if (BM_vert_in_edge(e_b_iter, v_a)) {
				return e_b_iter;
			}
		} while (((e_a_iter = bmesh_disk_edge_next(e_a_iter, v_a)) != e_a) &&
			((e_b_iter = bmesh_disk_edge_next(e_b_iter, v_b)) != e_b));
	}

	return NULL;
}


/**
* Utility function, since enough times we have an edge
* and want to access 2 connected faces.
*
* \return true when only 2 faces are found.
*/
bool BM_edge_face_pair(BMEdge *e, BMFace **r_fa, BMFace **r_fb)
{
	BMLoop *la, *lb;

	if ((la = e->l) &&
		(lb = la->radial_next) &&
		(la != lb) &&
		(lb->radial_next == la))
	{
		*r_fa = la->f;
		*r_fb = lb->f;
		return true;
	}
	else {
		*r_fa = NULL;
		*r_fb = NULL;
		return false;
	}
}

/**
* Utility function, since enough times we have an edge
* and want to access 2 connected loops.
*
* \return true when only 2 faces are found.
*/
bool BM_edge_loop_pair(BMEdge *e, BMLoop **r_la, BMLoop **r_lb)
{
	BMLoop *la, *lb;

	if ((la = e->l) &&
		(lb = la->radial_next) &&
		(la != lb) &&
		(lb->radial_next == la))
	{
		*r_la = la;
		*r_lb = lb;
		return true;
	}
	else {
		*r_la = NULL;
		*r_lb = NULL;
		return false;
	}
}

float BM_edge_calc_length(const BMEdge *e)
{
	return (e->v1->co - e->v2->co).norm();
}

float BM_edge_calc_length_squared(const BMEdge *e)
{
	return (e->v1->co - e->v2->co).squaredNorm();
}

/**
* Check if the edge is convex or concave
* (depends on face winding)
*/
bool BM_edge_is_convex(const BMEdge *e)
{
	if (BM_edge_is_manifold(e)) {
		BMLoop *l1 = e->l;
		BMLoop *l2 = e->l->radial_next;
		if (!(l1->f->no.isApprox(l2->f->no))) {
			Vector3f cross;
			Vector3f l_dir;
			cross = l1->f->no.cross(l2->f->no);
			/* we assume contiguous normals, otherwise the result isn't meaningful */
			l_dir = l1->next->v->co - l1->v->co;
			return (l_dir.dot(cross) > 0.0f);
		}
	}
	return true;
}

/**
* Given a edge and a loop (assumes the edge is manifold). returns
* the other faces loop, sharing the same vertex.
*
* <pre>
* +-------------------+
* |                   |
* |                   |
* |l_other <-- return |
* +-------------------+ <-- A manifold edge between 2 faces
* |l    e  <-- edge   |
* |^ <-------- loop   |
* |                   |
* +-------------------+
* </pre>
*/
BMLoop *BM_edge_other_loop(BMEdge *e, BMLoop *l)
{
	BMLoop *l_other;

	// BLI_assert(BM_edge_is_manifold(e));  // TOO strict, just check if we have another radial face
	BLI_assert(e->l && e->l->radial_next != e->l);
	BLI_assert(BM_vert_in_edge(e, l->v));

	l_other = (l->e == e) ? l : l->prev;
	l_other = l_other->radial_next;
	BLI_assert(l_other->e == e);

	if (l_other->v == l->v) {
		/* pass */
	}
	else if (l_other->next->v == l->v) {
		l_other = l_other->next;
	}
	else {
		BLI_assert(0);
	}

	return l_other;
}

/**
* Returns an edge sharing the same vertices as this one.
* This isn't an invalid state but tools should clean up these cases before
* returning the mesh to the user.
*/
BMEdge * BM_edge_find_double(BMEdge *e)
{
	BMVert *v = e->v1;
	BMVert *v_other = e->v2;

	BMEdge *e_iter;

	e_iter = e;
	while ((e_iter = bmesh_disk_edge_next(e_iter, v)) != e) {
		if (UNLIKELY(BM_vert_in_edge(e_iter, v_other))) {
			return e_iter;
		}
	}

	return NULL;
}

int BM_edge_face_count_ex(const BMEdge *e, const int count_max)
{
	int count = 0;

	if (e->l) {
		BMLoop *l_iter, *l_first;

		l_iter = l_first = e->l;
		do {
			count++;
			if (count == count_max) {
				break;
			}
		} while ((l_iter = l_iter->radial_next) != l_first);
	}

	return count;
}
int BM_edge_face_count(const BMEdge *e)
{
	int count = 0;

	if (e->l) {
		BMLoop *l_iter, *l_first;

		l_iter = l_first = e->l;
		do {
			count++;
		} while ((l_iter = l_iter->radial_next) != l_first);
	}

	return count;
}
/**
* Given a set of vertices (varr), find out if
* there is a face with exactly those vertices
* (and only those vertices).
*
* \note there used to be a BM_face_exists_overlap function that checks for partial overlap.
*/
bool BM_face_exists(BMVert **varr, int len, BMFace **r_existface)
{
	if (varr[0]->e) {
		BMEdge *e_iter, *e_first;
		e_iter = e_first = varr[0]->e;

		/* would normally use BM_LOOPS_OF_VERT, but this runs so often,
		* its faster to iterate on the data directly */
		do {
			if (e_iter->l) {
				BMLoop *l_iter_radial, *l_first_radial;
				l_iter_radial = l_first_radial = e_iter->l;

				do {
					if ((l_iter_radial->v == varr[0]) &&
						(l_iter_radial->f->len == len))
					{
						/* the fist 2 verts match, now check the remaining (len - 2) faces do too
						* winding isn't known, so check in both directions */
						int i_walk = 2;

						if (l_iter_radial->next->v == varr[1]) {
							BMLoop *l_walk = l_iter_radial->next->next;
							do {
								if (l_walk->v != varr[i_walk]) {
									break;
								}
							} while ((l_walk = l_walk->next), ++i_walk != len);
						}
						else if (l_iter_radial->prev->v == varr[1]) {
							BMLoop *l_walk = l_iter_radial->prev->prev;
							do {
								if (l_walk->v != varr[i_walk]) {
									break;
								}
							} while ((l_walk = l_walk->prev), ++i_walk != len);
						}

						if (i_walk == len) {
							if (r_existface) {
								*r_existface = l_iter_radial->f;
							}
							return true;
						}
					}
				} while ((l_iter_radial = l_iter_radial->radial_next) != l_first_radial);

			}
		} while ((e_iter = BM_DISK_EDGE_NEXT(e_iter, varr[0])) != e_first);
	}

	if (r_existface) {
		*r_existface = NULL;
	}
	return false;
}

/**
*	Return the shared vertex between the two edges or NULL
*/
BMVert *BM_edge_share_vert(BMEdge *e1, BMEdge *e2)
{
	BLI_assert(e1 != e2);
	if (BM_vert_in_edge(e2, e1->v1)) {
		return e1->v1;
	}
	else if (BM_vert_in_edge(e2, e1->v2)) {
		return e1->v2;
	}
	else {
		return NULL;
	}
}


/**
* \brief Return the Loop Shared by Edge and Vert
*
* Finds the loop used which uses \a  in face loop \a l
*
* \note this function takes a loop rather then an edge
* so we can select the face that the loop should be from.
*/
BMLoop *BM_edge_vert_share_loop(BMLoop *l, BMVert *v)
{
	BLI_assert(BM_vert_in_edge(l->e, v));
	if (l->v == v) {
		return l;
	}
	else {
		return l->next;
	}
}

/**
* Returns the verts of an edge as used in a face
* if used in a face at all, otherwise just assign as used in the edge.
*
* Useful to get a deterministic winding order when calling
* BM_face_create_ngon() on an arbitrary array of verts,
* though be sure to pick an edge which has a face.
*
* \note This is in fact quite a simple check, mainly include this function so the intent is more obvious.
* We know these 2 verts will _always_ make up the loops edge
*/
void BM_edge_ordered_verts_ex(
	const BMEdge *edge, BMVert **r_v1, BMVert **r_v2,
	const BMLoop *edge_loop)
{
	BLI_assert(edge_loop->e == edge);
	(void)edge; /* quiet warning in release build */
	*r_v1 = edge_loop->v;
	*r_v2 = edge_loop->next->v;
}

void BM_edge_ordered_verts(const BMEdge *edge, BMVert **r_v1, BMVert **r_v2)
{
	BM_edge_ordered_verts_ex(edge, r_v1, r_v2, edge->l);
}

/**
* \brief Return the Loop Shared by Face and Vertex
*
* Finds the loop used which uses \a v in face loop \a l
*
* \note currently this just uses simple loop in future may be sped up
* using radial vars
*/
BMLoop *BM_face_vert_share_loop(BMFace *f, BMVert *v)
{
	BMLoop *l_first;
	BMLoop *l_iter;

	l_iter = l_first = BM_FACE_FIRST_LOOP(f);
	do {
		if (l_iter->v == v) {
			return l_iter;
		}
	} while ((l_iter = l_iter->next) != l_first);

	return NULL;
}


/**
* \brief Return the Loop Shared by Face and Edge
*
* Finds the loop used which uses \a e in face loop \a l
*
* \note currently this just uses simple loop in future may be sped up
* using radial vars
*/
BMLoop *BM_face_edge_share_loop(BMFace *f, BMEdge *e)
{
	BMLoop *l_first;
	BMLoop *l_iter;

	l_iter = l_first = e->l;
	do {
		if (l_iter->f == f) {
			return l_iter;
		}
	} while ((l_iter = l_iter->radial_next) != l_first);

	return NULL;
}

/**
* \brief Other Loop in Face Sharing a Vertex
*
* Finds the other loop in a face.
*
* This function returns a loop in \a f that shares an edge with \a v
* The direction is defined by \a v_prev, where the return value is
* the loop of what would be 'v_next'
* <pre>
*     +----------+ <-- return the face loop of this vertex.
*     |          |
*     |    f     |
*     |          |
*     +----------+
*     v_prev --> v
*     ^^^^^^     ^ <-- These vert args define direction
*                      in the face to check.
*                      The faces loop direction is ignored.
* </pre>
*
* \note \a v_prev and \a v _implicitly_ define an edge.
*/
BMLoop * BM_face_other_vert_loop(BMFace *f, BMVert *v_prev, BMVert *v) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL()
{
	BMIter liter;
	BMLoop *l_iter;

	BLI_assert(BM_edge_exists(v_prev, v) != NULL);

	BM_ITER_ELEM(l_iter, &liter, v, BM_LOOPS_OF_VERT) {
		if (l_iter->f == f) {
			break;
		}
	}

	if (l_iter) {
		if (l_iter->prev->v == v_prev) {
			return l_iter->next;
		}
		else if (l_iter->next->v == v_prev) {
			return l_iter->prev;
		}
		else {
			/* invalid args */
			BLI_assert(0);
			return NULL;
		}
	}
	else {
		/* invalid args */
		BLI_assert(0);
		return NULL;
	}
}

BMVert *BM_face_tri_other_vert(BMFace *f, BMVert *v1, BMVert *v2)
{
	BLI_assert(f->len == 3);
	BMLoop *l_iter = BM_FACE_FIRST_LOOP(f);
	BMLoop *l_first = l_iter;
	do {
		if (l_iter->v != v1 && l_iter->v != v2)
			return l_iter->v;
	} while ((l_iter = l_iter->next) != l_first);

	return nullptr;
}
/**
*  Counts the number of edges two faces share (if any)
*/
int	BM_face_share_edge_count(BMFace *f_a, BMFace *f_b)
{
	BMLoop *l_iter;
	BMLoop *l_first;
	int count = 0;

	l_iter = l_first = BM_FACE_FIRST_LOOP(f_a);
	do {
		if (BM_edge_in_face(l_iter->e, f_b)) {
			count++;
		}
	} while ((l_iter = l_iter->next) != l_first);

	return count;
}

/**
* Use within assert's to check normals are valid.
*/
bool BM_face_is_normal_valid(const BMFace *f)
{
	const float eps = 0.001f;
	Vector3f no;
	BM_face_calc_normal(f, no);
	return (no - f->no).squaredNorm() < (eps * eps);
}
/**
* \brief BMESH EDGE/FACE ANGLE
*
*  Calculates the angle between two faces.
*  Assumes the face normals are correct.
*
* \return angle in radians
*/
float BM_edge_calc_face_angle_ex(const BMEdge *e, const float fallback)
{
	if (BM_edge_is_manifold(e)) {
		const BMLoop *l1 = e->l;
		const BMLoop *l2 = e->l->radial_next;
		return MathUtil::angle_normalized_v3v3(l1->f->no, l2->f->no);
	}
	else {
		return fallback;
	}
}
float BM_edge_calc_face_angle(const BMEdge *e)
{
	return BM_edge_calc_face_angle_ex(e, DEG2RADF(90.0f));
}

/**
* \brief BMESH EDGE/FACE ANGLE
*
*  Calculates the angle between two faces.
*  Assumes the face normals are correct.
*
* \return angle in radians
*/
float BM_edge_calc_face_angle_signed_ex(const BMEdge *e, const float fallback)
{
	if (BM_edge_is_manifold(e)) {
		BMLoop *l1 = e->l;
		BMLoop *l2 = e->l->radial_next;
		const float angle = MathUtil::angle_normalized_v3v3(l1->f->no, l2->f->no);
		return BM_edge_is_convex(e) ? angle : -angle;
	}
	else {
		return fallback;
	}
}
float BM_edge_calc_face_angle_signed(const BMEdge *e)
{
	return BM_edge_calc_face_angle_signed_ex(e, DEG2RADF(90.0f));
}

/**
* \brief BMESH EDGE/FACE TANGENT
*
* Calculate the tangent at this loop corner or fallback to the face normal on straight lines.
* This vector always points inward into the face.
*
* \brief BM_edge_calc_face_tangent
* \param e
* \param e_loop The loop to calculate the tangent at,
* used to get the face and winding direction.
* \param r_tangent The loop corner tangent to set
*/

void BM_edge_calc_face_tangent(const BMEdge *e, const BMLoop *e_loop, Vector3f &r_tangent)
{
	Vector3f tvec;
	BMVert *v1, *v2;
	BM_edge_ordered_verts_ex(e, &v1, &v2, e_loop);

	tvec = v1->co - v2->co; /* use for temp storage */
	/* note, we could average the tangents of both loops,
	* for non flat ngons it will give a better direction */
	r_tangent = tvec.cross(e_loop->f->no);
	r_tangent.normalize();
}

/**
* \brief Check if Rotate Edge is OK
*
* Quick check to see if we could rotate the edge,
* use this to avoid calling exceptions on common cases.
*/
bool BM_edge_rotate_check(BMEdge *e)
{
	BMFace *fa, *fb;
	if (BM_edge_face_pair(e, &fa, &fb)) {
		BMLoop *la, *lb;

		la = BM_face_other_vert_loop(fa, e->v2, e->v1);
		lb = BM_face_other_vert_loop(fb, e->v2, e->v1);

		/* check that the next vert in both faces isn't the same
		* (ie - the next edge doesn't share the same faces).
		* since we can't rotate usefully in this case. */
		if (la->v == lb->v) {
			return false;
		}

		/* mirror of the check above but in the opposite direction */
		la = BM_face_other_vert_loop(fa, e->v1, e->v2);
		lb = BM_face_other_vert_loop(fb, e->v1, e->v2);

		if (la->v == lb->v) {
			return false;
		}

		return true;
	}
	else {
		return false;
	}
}

/**
* Calculate the 2 loops which _would_ make up the newly rotated Edge
* but don't actually change anything.
*
* Use this to further inspect if the loops to be connected have issues:
*
* Examples:
* - the newly formed edge already exists
* - the new face would be degenerate (zero area / concave /  bow-tie)
* - may want to measure if the new edge gives improved results topology.
*   over the old one, as with beauty fill.
*
* \note #BM_edge_rotate_check must have already run.
*/
void BM_edge_calc_rotate(BMEdge *e, const bool ccw, BMLoop **r_l1, BMLoop **r_l2)
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


VM_END_NAMESPACE

