#include "BMeshIterators.h"
VM_BEGIN_NAMESPACE

void bmiter__elem_of_mesh_begin(struct BMIter__elem_of_mesh *iter)
{
#if 0
#ifdef USE_IMMUTABLE_ASSERT
	((BMIter *)iter)->count = BLI_mempool_count(iter->pooliter.pool);
#endif
	BLI_mempool_iternew(iter->pooliter.pool, &iter->pooliter);
#endif
}

void *bmiter__elem_of_mesh_step(struct BMIter__elem_of_mesh *iter)
{
#if 0
#ifdef USE_IMMUTABLE_ASSERT
	BLI_assert(((BMIter *)iter)->count <= BLI_mempool_count(iter->pooliter.pool));
#endif
	return BLI_mempool_iterstep(&iter->pooliter);
#else
	switch (iter->itype)
	{
		case BM_VERTS_OF_MESH:
		{
			BMVert *prevv = nullptr;
			if (iter->count > 0){
				prevv = iter->elem.v;
				if (iter->elem.v){
					iter->count--;
					iter->elem.v = iter->elem.v->next_elm;
				}
			}
			return prevv;
		}
		case BM_EDGES_OF_MESH:
		{
			BMEdge *preve = nullptr;
			if (iter->count > 0){
				preve = iter->elem.e;
				if (iter->elem.e){
					iter->count--;
					iter->elem.e = iter->elem.e->next_elm;
				}
			}
			return preve;
		}
		case BM_FACES_OF_MESH:
		{
			BMFace *prevf = nullptr;
			if (iter->count > 0){
				prevf = iter->elem.f;
				if (iter->elem.f){
					iter->count--;
					iter->elem.f = iter->elem.f->next_elm;
				}
			}
			return prevf;
		}
		default:
		{
			return nullptr;
		}
	}
#endif
	return nullptr;
}

/*
* EDGE OF VERT CALLBACKS
*/

void  bmiter__edge_of_vert_begin(struct BMIter__edge_of_vert *iter)
{
	if (iter->vdata->e) {
		iter->e_first = iter->vdata->e;
		iter->e_next = iter->vdata->e;
	}
	else {
		iter->e_first = NULL;
		iter->e_next = NULL;
	}
}

void  *bmiter__edge_of_vert_step(struct BMIter__edge_of_vert *iter)
{
	BMEdge *e_curr = iter->e_next;

	if (iter->e_next) {
		iter->e_next = bmesh_disk_edge_next(iter->e_next, iter->vdata);
		if (iter->e_next == iter->e_first) {
			iter->e_next = NULL;
		}
	}

	return e_curr;
}

/*
* FACE OF VERT CALLBACKS
*/

void  bmiter__face_of_vert_begin(struct BMIter__face_of_vert *iter)
{
	((BMIter *)iter)->count = bmesh_disk_facevert_count(iter->vdata);
	if (((BMIter *)iter)->count) {
		iter->e_first = bmesh_disk_faceedge_find_first(iter->vdata->e, iter->vdata);
		iter->e_next = iter->e_first;
		iter->l_first = bmesh_radial_faceloop_find_first(iter->e_first->l, iter->vdata);
		iter->l_next = iter->l_first;
	}
	else {
		iter->l_first = iter->l_next = NULL;
		iter->e_first = iter->e_next = NULL;
	}
}
void  *bmiter__face_of_vert_step(struct BMIter__face_of_vert *iter)
{
	BMLoop *l_curr = iter->l_next;

	if (((BMIter *)iter)->count && iter->l_next) {
		((BMIter *)iter)->count--;
		iter->l_next = bmesh_radial_faceloop_find_next(iter->l_next, iter->vdata);
		if (iter->l_next == iter->l_first) {
			iter->e_next = bmesh_disk_faceedge_find_next(iter->e_next, iter->vdata);
			iter->l_first = bmesh_radial_faceloop_find_first(iter->e_next->l, iter->vdata);
			iter->l_next = iter->l_first;
		}
	}

	if (!((BMIter *)iter)->count) {
		iter->l_next = NULL;
	}

	return l_curr ? l_curr->f : NULL;
}


/*
* LOOP OF VERT CALLBACKS
*
*/

void  bmiter__loop_of_vert_begin(struct BMIter__loop_of_vert *iter)
{
	((BMIter *)iter)->count = bmesh_disk_facevert_count(iter->vdata);
	if (((BMIter *)iter)->count) {
		iter->e_first = bmesh_disk_faceedge_find_first(iter->vdata->e, iter->vdata);
		iter->e_next = iter->e_first;
		iter->l_first = bmesh_radial_faceloop_find_first(iter->e_first->l, iter->vdata);
		iter->l_next = iter->l_first;
	}
	else {
		iter->l_first = iter->l_next = NULL;
		iter->e_first = iter->e_next = NULL;
	}
}
void  *bmiter__loop_of_vert_step(struct BMIter__loop_of_vert *iter)
{
	BMLoop *l_curr = iter->l_next;

	if (((BMIter *)iter)->count) {
		((BMIter *)iter)->count--;
		iter->l_next = bmesh_radial_faceloop_find_next(iter->l_next, iter->vdata);
		if (iter->l_next == iter->l_first) {
			iter->e_next = bmesh_disk_faceedge_find_next(iter->e_next, iter->vdata);
			iter->l_first = bmesh_radial_faceloop_find_first(iter->e_next->l, iter->vdata);
			iter->l_next = iter->l_first;
		}
	}

	if (!((BMIter *)iter)->count) {
		iter->l_next = NULL;
	}

	/* NULL on finish */
	return l_curr;
}

/*
* LOOP OF EDGE CALLBACKS
*/

void  bmiter__loop_of_edge_begin(struct BMIter__loop_of_edge *iter)
{
	iter->l_first = iter->l_next = iter->edata->l;
}

void  *bmiter__loop_of_edge_step(struct BMIter__loop_of_edge *iter)
{
	BMLoop *l_curr = iter->l_next;

	if (iter->l_next) {
		iter->l_next = iter->l_next->radial_next;
		if (iter->l_next == iter->l_first) {
			iter->l_next = NULL;
		}
	}

	/* NULL on finish */
	return l_curr;
}

/*
* LOOP OF LOOP CALLBACKS
*/

void  bmiter__loop_of_loop_begin(struct BMIter__loop_of_loop *iter)
{
	iter->l_first = iter->ldata;
	iter->l_next = iter->l_first->radial_next;

	if (iter->l_next == iter->l_first)
		iter->l_next = NULL;
}

void  *bmiter__loop_of_loop_step(struct BMIter__loop_of_loop *iter)
{
	BMLoop *l_curr = iter->l_next;

	if (iter->l_next) {
		iter->l_next = iter->l_next->radial_next;
		if (iter->l_next == iter->l_first) {
			iter->l_next = NULL;
		}
	}

	/* NULL on finish */
	return l_curr;
}

/*
* FACE OF EDGE CALLBACKS
*/

void  bmiter__face_of_edge_begin(struct BMIter__face_of_edge *iter)
{
	iter->l_first = iter->l_next = iter->edata->l;
}

void  *bmiter__face_of_edge_step(struct BMIter__face_of_edge *iter)
{
	BMLoop *current = iter->l_next;

	if (iter->l_next) {
		iter->l_next = iter->l_next->radial_next;
		if (iter->l_next == iter->l_first) {
			iter->l_next = NULL;
		}
	}

	return current ? current->f : NULL;
}

/*
* VERTS OF EDGE CALLBACKS
*/

void  bmiter__vert_of_edge_begin(struct BMIter__vert_of_edge *iter)
{
	((BMIter *)iter)->count = 0;
}

void  *bmiter__vert_of_edge_step(struct BMIter__vert_of_edge *iter)
{
	switch (((BMIter *)iter)->count++) {
	case 0:
		return iter->edata->v1;
	case 1:
		return iter->edata->v2;
	default:
		return NULL;
	}
}

/*
* VERT OF FACE CALLBACKS
*/

void  bmiter__vert_of_face_begin(struct BMIter__vert_of_face *iter)
{
	iter->l_first = iter->l_next = BM_FACE_FIRST_LOOP(iter->pdata);
}

void  *bmiter__vert_of_face_step(struct BMIter__vert_of_face *iter)
{
	BMLoop *l_curr = iter->l_next;

	if (iter->l_next) {
		iter->l_next = iter->l_next->next;
		if (iter->l_next == iter->l_first) {
			iter->l_next = NULL;
		}
	}

	return l_curr ? l_curr->v : NULL;
}

/*
* EDGE OF FACE CALLBACKS
*/

void  bmiter__edge_of_face_begin(struct BMIter__edge_of_face *iter)
{
	iter->l_first = iter->l_next = BM_FACE_FIRST_LOOP(iter->pdata);
}

void  *bmiter__edge_of_face_step(struct BMIter__edge_of_face *iter)
{
	BMLoop *l_curr = iter->l_next;

	if (iter->l_next) {
		iter->l_next = iter->l_next->next;
		if (iter->l_next == iter->l_first) {
			iter->l_next = NULL;
		}
	}

	return l_curr ? l_curr->e : NULL;
}

/*
* LOOP OF FACE CALLBACKS
*/

void  bmiter__loop_of_face_begin(struct BMIter__loop_of_face *iter)
{
	iter->l_first = iter->l_next = BM_FACE_FIRST_LOOP(iter->pdata);
}

void  *bmiter__loop_of_face_step(struct BMIter__loop_of_face *iter)
{
	BMLoop *l_curr = iter->l_next;

	if (iter->l_next) {
		iter->l_next = iter->l_next->next;
		if (iter->l_next == iter->l_first) {
			iter->l_next = NULL;
		}
	}

	return l_curr;
}


/**
* \brief Iterator as Array
*
* Sometimes its convenient to get the iterator as an array
* to avoid multiple calls to #BM_iter_at_index.
*/
int BM_iter_as_array(BMesh *bm, const char itype, void *data, void **array, const int len)
{
	int i = 0;

	/* sanity check */
	if (len > 0) {
		BMIter iter;
		void *ele;

		for (ele = BM_iter_new(&iter, bm, itype, data); ele; ele = BM_iter_step(&iter)) {
			array[i] = ele;
			i++;
			if (i == len) {
				return len;
			}
		}
	}

	return i;
}


VM_END_NAMESPACE
