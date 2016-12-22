#ifndef BMESH_BISECT_PLANE_H
#define BMESH_BISECT_PLANE_H
#include "BMesh/BMesh.h"
#include "BaseLib/MathGeom.h"
#include <Eigen/Dense>

VM_BEGIN_NAMESPACE
class BMehsBisectPlane
{
	EIGEN_MAKE_ALIGNED_OPERATOR_NEW
public:
	BMehsBisectPlane(BMesh *bm,
		const Eigen::Vector4f &plane, const bool use_snap_center,
		const bool use_tag, const short oflag_center, const float eps, bool clear_outer, bool clear_inner, float dist);
	~BMehsBisectPlane();
	void run();

private:
	void kill_inner_outer_faces();
	void bmesh_bisect_plane();
	void bmesh_face_split(BMFace *f);

	BLI_INLINE short plane_point_test_v3(const Vector4f &plane, const Vector3f &co, const float eps, float *r_depth)
	{
		const float f = MathGeom::plane_point_side_v3(plane, co);
		*r_depth = f;

		if (f <= -eps) return -1;
		else if (f >= eps) return  1;
		else                return  0;
	}


	/* -------------------------------------------------------------------- */
	/* Wrappers to hide internal data-structure abuse,
	* later we may want to move this into some hash lookup
	* to a separate struct, but for now we can store in BMesh data */

	/* Direction -1/0/1 */
	BLI_INLINE short& BM_VERT_DIR(BMVert *v)
	{
		return  ((short *)(&(v)->head.index))[0];
	}

	/* Skip Vert 0/1 */
	BLI_INLINE short& BM_VERT_SKIP(BMVert *v)
	{
		return ((short *)(&(v)->head.index))[1];
	}

	/* Distance from the plane. */
	BLI_INLINE float& BM_VERT_DIST(BMVert *v)
	{
		return (v)->no[0];
	}

	/* enable when vertex is in the center and its faces have been added to the stack */
	BLI_INLINE void vert_is_center_enable(BMVert *v) { BM_elem_flag_enable(v, BM_ELEM_TAG); }
	BLI_INLINE void vert_is_center_disable(BMVert *v) { BM_elem_flag_disable(v, BM_ELEM_TAG); }
	BLI_INLINE bool vert_is_center_test(BMVert *v) { return (BM_elem_flag_test(v, BM_ELEM_TAG) != 0); }

	/* enable when the edge can be cut */
	BLI_INLINE void edge_is_cut_enable(BMEdge *e) { BM_elem_flag_enable(e, BM_ELEM_TAG); }
	BLI_INLINE void edge_is_cut_disable(BMEdge *e) { BM_elem_flag_disable(e, BM_ELEM_TAG); }
	BLI_INLINE bool edge_is_cut_test(BMEdge *e) { return (BM_elem_flag_test(e, BM_ELEM_TAG) != 0); }

	/* enable when the faces are added to the stack */
	BLI_INLINE void face_in_stack_enable(BMFace *f) { BM_elem_flag_disable(f, BM_ELEM_TAG); }
	BLI_INLINE void face_in_stack_disable(BMFace *f) { BM_elem_flag_enable(f, BM_ELEM_TAG); }
	BLI_INLINE bool face_in_stack_test(BMFace *f) { return (BM_elem_flag_test(f, BM_ELEM_TAG) == 0); }
private:
	BMesh *_bm;
	Eigen::Vector4f _plane;
	const bool		_use_snap_center;
	const bool		_use_tag;
	const short		_oflag_center;
	const float		_eps;
	const float		_dist;
	const bool		_clear_outer;
	const bool		_clear_inner;
};
VM_END_NAMESPACE

#endif