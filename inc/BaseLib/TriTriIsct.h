/*
*
*  Triangle-Triangle Overlap Test Routines
*  July, 2002
*  Updated December 2003
*
*  This file contains C implementation of algorithms for
*  performing two and three-dimensional triangle-triangle intersection test
*  The algorithms and underlying theory are described in
*
* "Fast and Robust Triangle-Triangle Overlap Test
*  Using Orientation Predicates"  P. Guigue - O. Devillers
*
*  Journal of Graphics Tools, 8(1), 2003
*
*  Several geometric predicates are defined.  Their parameters are all
*  points.  Each point is an array of two or three Scalar precision
*  floating point numbers. The geometric predicates implemented in
*  this file are:
*
*    int tri_tri_overlap_test_3d(p1,q1,r1,p2,q2,r2)
*    int tri_tri_overlap_test_2d(p1,q1,r1,p2,q2,r2)
*
*    int tri_tri_intersection_test_3d(p1,q1,r1,p2,q2,r2,
*                                     coplanar,source,target)
*
*       is a version that computes the segment of intersection when
*       the triangles overlap (and are not coplanar)
*
*    each function returns 1 if the triangles (including their
*    boundary) intersect, otherwise 0
*
*
*  Other information are available from the Web page
*  http://www.acm.org/jgt/papers/GuigueDevillers03/
*
*/

#ifndef BASELIB_TRI_TRI_ISCT_H
#define BASELIB_TRI_TRI_ISCT_H
namespace tri_tri_isct
{
	typedef float Scalar;

	/* function prototype */

	int tri_tri_overlap_test_3d(
		const Scalar p1[3], const Scalar q1[3], const Scalar r1[3],
		const Scalar p2[3], const Scalar q2[3], const Scalar r2[3]);

	int coplanar_tri_tri3d(
		const Scalar  p1[3], const Scalar  q1[3], const Scalar  r1[3],
		const Scalar  p2[3], const Scalar  q2[3], const Scalar  r2[3],
		const Scalar  N1[3], const Scalar  N2[3]);

	int tri_tri_overlap_test_2d(
		const Scalar p1[2], const Scalar q1[2], const Scalar r1[2],
		const Scalar p2[2], const Scalar q2[2], const Scalar r2[2]);

	int tri_tri_intersection_test_3d(
		const Scalar p1[3], const Scalar q1[3], const Scalar r1[3],
		const Scalar p2[3], const Scalar q2[3], const Scalar r2[3],
		bool & coplanar,
		Scalar source[3], Scalar target[3]);
}

#endif

