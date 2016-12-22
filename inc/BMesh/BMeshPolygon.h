#ifndef BMESH_BM_POLYGON_H
#define BMESH_BM_POLYGON_H
#include "BMUtilDefine.h"
#include "BaseLib/UtilMacro.h"
#include "BMeshClass.h"

VM_BEGIN_NAMESPACE

void BM_face_as_array_vert_tri(const BMFace *f, BMVert *r_verts[3]) ATTR_NONNULL();
void BM_face_as_array_vert_quad(const BMFace *f, BMVert *r_verts[4]) ATTR_NONNULL();

void BM_face_as_array_loop_tri(const BMFace *f, BMLoop *r_loops[3]) ATTR_NONNULL();
void BM_face_as_array_loop_quad(const BMFace *f, BMLoop *r_loops[4]) ATTR_NONNULL();
bool BM_face_closest_point(BMFace *f, const Vector3f &p, Vector3f &closest);

void  BM_face_normal_update(BMFace *f);
float BM_face_calc_normal(const BMFace *f, Vector3f &r_no) ATTR_NONNULL();
void  BM_face_calc_bounds(const BMFace *f, Vector3f &lower, Vector3f &upper);
float BM_face_calc_area(const BMFace *f) ATTR_WARN_UNUSED_RESULT ATTR_NONNULL();
void  BM_face_calc_center_mean(const BMFace *f, Vector3f &center) ATTR_NONNULL();

void  BM_vert_calc_mean(const BMVert *v, Vector3f &r_mean);
bool  BM_vert_calc_normal(const BMVert *v, Vector3f &r_no);
bool  BM_vert_calc_normal_ex(const BMVert *v, const char hflag, Vector3f &r_no);
void  BM_vert_normal_update(BMVert *v) ATTR_NONNULL();
void  BM_vert_normal_update_face(BMVert *v) ATTR_NONNULL();
void  BM_vert_normal_update_all(BMVert *v) ATTR_NONNULL();

/*TODO. Ignore it*/
void  BM_vert_default_color_set(BMVert *v);
VM_END_NAMESPACE
#endif