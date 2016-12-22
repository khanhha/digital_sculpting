#ifndef BMESH_UTILITY_H
#define BMESH_UTILITY_H
#include "BMUtilDefine.h"
#include "BaseLib/UtilMacro.h"
#include "BMeshClass.h"
#include "BaseLib/VQuadric.h"

VM_BEGIN_NAMESPACE
void	BM_edge_collapse_optimize_co(BMEdge *e, Vector3f &co);
void	BM_vert_quadric_calc(BMVert *v, Qdr::Quadric &vquadric);
float	BM_verts_calc_rotate_beauty(const BMVert *v1, const BMVert *v2, const BMVert *v3, const BMVert *v4, const short method);
VM_END_NAMESPACE
#endif