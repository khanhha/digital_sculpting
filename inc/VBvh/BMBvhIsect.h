#ifndef VBVH_BMBVH_RAY_INTERSECTOR_H
#define VBVH_BMBVH_RAY_INTERSECTOR_H
#include "VBvhDefine.h"
#include "BMeshBvh.h"
#include "BaseLib/MathGeom.h"
#include "BaseLib/MathUtil.h"

VBVH_BEGIN_NAMESPACE

struct SphereIsectFunctor
{
	SphereIsectFunctor(const Vector3f &center_, const float &rad_)
		:center(center_), rad(rad_){};

	bool isect(BaseNode *node) const
	{
		const BBox3fa &bb = node->bounds();
		Vector3f lower(bb.lower.x, bb.lower.y, bb.lower.z);
		Vector3f upper(bb.upper.x, bb.upper.y, bb.upper.z);
		return MathGeom::isect_aligned_box_sphere_v3(lower, upper, center, rad);
	}

	Vector3f center;
	float    rad;
};


struct BoxTubeIsectFunctor
{
	BoxTubeIsectFunctor(const Eigen::Vector4f (*planes_)[4])
		:
		planes(planes_)
	{}

	bool isect(BaseNode *node) const
	{
		const Eigen::AlignedBox3f bb = node->boundsEigen();
		return MathGeom::isect_aligned_box_box_tube_v3((bb.min)(), (bb.max)(), planes);
	}

	const Eigen::Vector4f (*planes)[4];
};


struct BBInsideFunctor
{
	BBInsideFunctor(const Vector3f &p_)
		: p(p_){}

	bool inside(BaseNode *node) const
	{
		const BBox3fa &bb = node->bounds();
		Vec3fa pp(p[0], p[1], p[2]);
		return VBvh::inside(bb, pp);
	}

	Vector3f p;
};

int		isect_ray_backup_tris_nearest_hit(BMFaceBackup *faces, size_t num, const Vector3f &org, const Vector3f &dir, float &hitLambda, const float &epsilon);
int		isect_ray_bm_faces_nearest_hit(BMFace * const *faces, size_t num, const Vector3f &org, const Vector3f &dir, float &hitLambda, const float &epsilon);
int		isect_ray_bm_faces_all(BMFace * const *faces, size_t num, const Vector3f &org, const Vector3f &dir, float &hitLambda, const float &epsilon, bool test_cull);
bool	isect_ray_bm_face(BMFace *face, const Vector3f &org, const Vector3f &dir, float &lambda, float uv[2], const float &epsilon, bool test_cull);

bool	isect_ray_bm_bvh_nearest_hit(const BMBvh *bvh, const Vector3f &org, const Vector3f dir, bool origin_data, Vector3f &hit, const float epsilon);
bool	isect_ray_bm_bvh_all_hit(const BMBvh *bvh, const Vector3f &org, const Vector3f dir, bool origin_data, std::vector<Vector3f> *hitpoints, std::vector<Vector2f> *hituvs, std::vector<BMFace*> *hitfaces, const float epsilon);

bool	isect_sphere_bm_bvh(const BMBvh *bvh, const Eigen::Vector3f &center, const float &radius, std::vector<BMLeafNode*> &leafs);
bool	isect_box_tube_bm_bvh(const BMBvh *bvh, const Eigen::Vector4f(*plane)[4], std::vector<BMLeafNode*> &leafs);

bool	bvh_closest_face_point_to_face(const BMBvh *bvh, const Vector3f &p, Vector3f &r_closest_p, BMFace **r_closest_f);
bool    bvh_inside_outside_point(const BMBvh *bvh, const Vector3f &p, const float &epsilon);
VBVH_END_NAMESPACE
#endif