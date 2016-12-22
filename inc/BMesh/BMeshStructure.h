#ifndef VMESH_STRUCTURE_H
#define VMESH_STRUCTURE_H

#include "BMUtilDefine.h"
#include "BMeshClass.h"

VM_BEGIN_NAMESPACE

#ifndef _DEBUG
/* no error checking for release,
* it can take most of the CPU time when running some tools */
#  define BM_CHECK_ELEMENT(el)  (void)(el)
#else
int bmesh_elem_check(void *element, const char htype);
#  define BM_CHECK_ELEMENT(el)                                                \
	if (bmesh_elem_check(el, ((BMHeader *)el)->htype)) {                      \
	    printf("check_element failure, with code %i on line %i in file\n"     \
	    "    \"%s\"\n\n",                                                     \
	    bmesh_elem_check(el, ((BMHeader *)el)->htype),                        \
	    __LINE__, __FILE__);                                                  \
		} (void)0
#endif

int		bmesh_elem_loop_check(BMLoop *loop);

bool    bmesh_loop_validate(BMFace *f) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

/* DISK CYCLE MANAGMENT */
BMDiskLink *bmesh_disk_edge_link_from_vert(const BMEdge *e, const BMVert *v);
void    bmesh_disk_edge_append(BMEdge *e, BMVert *v);
void    bmesh_disk_edge_remove(BMEdge *e, BMVert *v);
BMEdge *bmesh_disk_edge_next(const BMEdge *e, const BMVert *v);
BMEdge *bmesh_disk_edge_prev(const BMEdge *e, const BMVert *v);
int		bmesh_disk_count(const BMVert *v);
int		bmesh_disk_count_ex(const BMVert *v, const int count_max);
int     bmesh_disk_facevert_count(const BMVert *v);
BMEdge *bmesh_disk_faceedge_find_first(const BMEdge *e, const BMVert *v);
BMEdge *bmesh_disk_faceedge_find_next(const BMEdge *e, const BMVert *v);
void    bmesh_disk_vert_replace(BMEdge *e, BMVert *v_dst, BMVert *v_src);
void    bmesh_disk_vert_swap(BMEdge *e, BMVert *v_dst, BMVert *v_src) ATTR_NONNULL();
bool    bmesh_disk_validate(int len, BMEdge *e, BMVert *v) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();

/* RADIAL CYCLE MANAGMENT */
void    bmesh_radial_append(BMEdge *e, BMLoop *l);
void    bmesh_radial_loop_remove(BMLoop *l, BMEdge *e);
int		bmesh_radial_length(const BMLoop *l);
bool    bmesh_radial_validate(int radlen, BMLoop *l);
int     bmesh_radial_facevert_count(const BMLoop *l, const BMVert *v);
bool	bmesh_radial_facevert_check(const BMLoop *l, const BMVert *v);
BMLoop *bmesh_radial_faceloop_find_first(const BMLoop *l, const BMVert *v);
BMLoop *bmesh_radial_faceloop_find_next(const BMLoop *l, const BMVert *v);


void    bmesh_edge_vert_swap(BMEdge *e, BMVert *v_dst, BMVert *v_src) ATTR_NONNULL();

VM_END_NAMESPACE
#endif