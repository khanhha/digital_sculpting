#include "BMBvhIsect.h"
#include "VBvhIterator.h"
#include <QtGlobal>


VBVH_BEGIN_NAMESPACE
/*only support tri and quad now*/
bool isect_ray_bm_face(BMFace *face, const Vector3f &org, const Vector3f &dir, float &lambda, float uv[2], const float &epsilon, bool test_cull)
{
	if (face->len == 3){
		BMVert *verts[3];
		BM_face_as_array_vert_tri(face, verts);
		return MathGeom::isect_ray_tri_epsilon_v3(org, dir, verts[0]->co, verts[1]->co, verts[2]->co, &lambda, uv, epsilon, test_cull);
	}
	else if(face->len == 4){
		BMVert *verts[4];
		bool hit;
		BM_face_as_array_vert_quad(face, verts);
		hit = MathGeom::isect_ray_tri_epsilon_v3(org, dir, verts[0]->co, verts[1]->co, verts[2]->co, &lambda, uv, epsilon, test_cull);
		if (hit){
			return true;
		}
		else{
			return MathGeom::isect_ray_tri_epsilon_v3(org, dir, verts[2]->co, verts[3]->co, verts[0]->co, &lambda, uv, epsilon, test_cull);
		}
	}
	else{
		return false;
	}
}

int isect_ray_backup_tris_nearest_hit(BMFaceBackup *faces, size_t num, const Vector3f &org, const Vector3f &dir, float &hitLambda, const float &epsilon)
{
	float minLambda = FLT_MAX, lambda;
	int hitidx = -1;
	for (size_t i = 0; i < num; i++){
		BMFaceBackup &f = faces[i];
		if (MathGeom::isect_ray_tri_epsilon_v3(org, dir, f.vcoods[0], f.vcoods[1], f.vcoods[2], &lambda, nullptr, epsilon, true) &&
			lambda < minLambda)
		{
			minLambda = lambda;
			hitidx = i;

		}
	}

	hitLambda = minLambda;
	return hitidx;
}


int isect_ray_bm_faces_nearest_hit(BMFace * const *faces, size_t num, const Vector3f &org, const Vector3f &dir, 
	float &hitLambda, const float &epsilon)
{
	float minLambda = FLT_MAX, lambda;
	int hitidx = -1;
	for (size_t i = 0; i < num; ++i){
		BMFace *f = faces[i];
		if (isect_ray_bm_face(f, org, dir, lambda, nullptr, epsilon, true)){
			if (lambda < minLambda){
				minLambda = lambda;
				hitidx = i;
			}
		}
	}
	hitLambda = minLambda;
	return hitidx;
}

bool isect_ray_bm_bvh_nearest_hit(const BMBvh *bvh, const Vector3f &org, const Vector3f dir, bool origin_data, Vector3f &hit, const float epsilon)
{
	Ray ray(org, dir, origin_data);
	VBvhRayIterator<BMLeafNode> iter(bvh->rootNode(), &ray);
	BMFace *hitFace;
	float   hitLambda;

	for (; iter; ++iter){
		BMLeafNode *lnode = *iter;
		hitFace = nullptr;

		if (origin_data && lnode->originData()){
			OriginLeafNodeData *orgnode = lnode->originData();
			int hitidx = isect_ray_backup_tris_nearest_hit(orgnode->_faces, orgnode->_totfaces, org, dir, hitLambda, epsilon);
			if (hitidx != -1 && hitLambda < ray.tfar){
				ray.tfar = hitLambda;
				ray.prim = reinterpret_cast<size_t>(orgnode->_faces[hitidx].face);
				ray.hit = true;
			}
		}
		else{
			int hitidx = isect_ray_bm_faces_nearest_hit(lnode->faces().data(), lnode->faces().size(), org, dir, hitLambda, epsilon);
			if (hitidx != -1  && hitLambda < ray.tfar){
				ray.tfar = hitLambda;
				ray.prim = reinterpret_cast<size_t>(lnode->faces()[hitidx]);
				ray.hit = true;
			}
		}

	}

	if (ray.hit){
		hit = ray.org + ray.tfar * ray.dir;
		return true;
	}
	else{
		return false;
	}
}

bool isect_ray_bm_bvh_all_hit(
	const BMBvh *bvh, const Vector3f &org, const Vector3f dir, bool origin_data, 
	std::vector<Vector3f> *hitpoints, std::vector<Vector2f> *hituvs, std::vector<BMFace*> *hitfaces, const float epsilon)
{
	Ray ray(org, dir, origin_data);
	VBvhRayIterator<BMLeafNode> iter(bvh->rootNode(), &ray);
			float hit_dst;
			Vector2f uv;

	for (; iter; ++iter){
		BMLeafNode *lnode = *iter;
		if (origin_data && lnode->originData()){
			OriginLeafNodeData *orgnode = lnode->originData();
			BMFaceBackup *faces = orgnode->_faces;
			const size_t totface = orgnode->_totfaces;
			for (size_t i = 0; i < totface; ++i){
				BMFaceBackup &fbacup = faces[i];
				if (MathGeom::isect_ray_tri_epsilon_v3(
					org, dir, fbacup.vcoods[0], fbacup.vcoods[1], fbacup.vcoods[2], &hit_dst, uv.data(), epsilon, false)){
					
					if (hitpoints) hitpoints->push_back(ray.org + hit_dst * ray.dir);
					if (hitfaces) hitfaces->push_back(fbacup.face);
					if (hituvs) hituvs->push_back(uv);
					
					ray.hit = true;
				}
			}

			ray.tfar = FLT_MAX; /*always recur to all possible leaf nodes*/
		}
		else{
			const BMFaceVector &faces = lnode->faces();
			const size_t totface = faces.size();
			for (size_t i = 0; i < totface; ++i){
				if (isect_ray_bm_face(faces[i], org, dir, hit_dst, uv.data(), epsilon, false)){
					
					if (hitpoints) hitpoints->push_back(ray.org + hit_dst * ray.dir);
					if (hitfaces) hitfaces->push_back(faces[i]);
					if (hituvs) hituvs->push_back(uv);
					
					ray.hit = true;
				}
			}

			ray.tfar = FLT_MAX; /*always recur to all possible leaf nodes*/
		}

	}

	if (ray.hit){
		return true;
	}
	else{
		return false;
	}
}

bool isect_sphere_bm_bvh(const BMBvh *bvh, const Vector3f &center, const float &radius, std::vector<BMLeafNode*> &leafs)
{
	using std::placeholders::_1;

	const SphereIsectFunctor functor(center, radius);
	VBvhIterator<BMLeafNode>::IsectFunc isect = std::bind(&SphereIsectFunctor::isect, functor, _1);

	VBvhIterator<BMLeafNode> iter(bvh->rootNode(), isect);
	for (; iter; ++iter){
		leafs.push_back(*iter);
	}

	return !leafs.empty();
}

bool isect_box_tube_bm_bvh(const BMBvh *bvh, const Eigen::Vector4f (*plane)[4], std::vector<BMLeafNode*> &leafs)
{
	using std::placeholders::_1;

	const BoxTubeIsectFunctor functor(plane);
	VBvhIterator<BMLeafNode>::IsectFunc isect = std::bind(&BoxTubeIsectFunctor::isect, functor, _1);
	
	VBvhIterator<BMLeafNode> iter(bvh->rootNode(), isect);
	for (; iter; ++iter){
		leafs.push_back(*iter);
	}

	return !leafs.empty();
}


bool bvh_closest_face_point_to_face(const BMBvh *bvh, const Vector3f &p, Vector3f &r_closest_p, BMFace **r_closest_f)
{
	using std::placeholders::_1;
	
	bool ret = false;
	float min_dst = FLT_MAX;

	const BBInsideFunctor functor(p);
	VBvhIterator<BMLeafNode>::IsectFunc isect = std::bind(&BBInsideFunctor::inside, functor, _1);
	VBvhIterator<BMLeafNode> iter(bvh->rootNode(), isect);
	for (; iter; ++iter){
		const BMFaceVector &faces = (*iter)->faces();
		const size_t totface = faces.size();
		for (size_t i = 0; i < totface; ++i){
			Vector3f cur_closest;
			if (BM_face_closest_point(faces[i], p, cur_closest)){
				float dst = (cur_closest - p).squaredNorm();
				if (dst < min_dst){
					r_closest_p = cur_closest;
					if (r_closest_f)  *r_closest_f = faces[i];
					min_dst = dst;
					ret = true;
				}
			}
		}
	}

	return ret;
}


/*shoot ray from p to some direction around it
*if p is inside mesh, it must hit the mesh odd times
*otherwise, it must hit the mes even times
*Note: me must check for some directions since this method
*will go wrong when ray hit the mesh's vertices or edges
*which means counting is incorrect*/
bool bvh_inside_outside_point(const BMBvh *bvh, const Vector3f &p, const float &epsilon)
{
	const int MAX_LIMIT = 50;
	std::vector<Vector2f> hit_uvs;
	Vector3f org(p);
	Vector3f dir(1.0, 0.0, 0.0);
	size_t cnt = 0;
	while (true)
	{
		if (cnt > MAX_LIMIT){
			assert(false);
		}

		bool allInteriorInterections = true;;
		hit_uvs.clear();
		if (isect_ray_bm_bvh_all_hit(bvh, org, dir, false, nullptr, &hit_uvs, nullptr, epsilon)){
			for (auto it = hit_uvs.begin(); it != hit_uvs.end(); ++it){
				const Vector2f &uv = *it;
				if (qFuzzyCompare(uv[0] + uv[1], 1.0f)) /*on edge or vertex*/
				{
					allInteriorInterections = false;
				}
			}
		}
		else{
			break;
		}

		if (allInteriorInterections){
			break;
		}

		cnt++;
		dir[cnt % 2 + 1] += 0.05;
	}

	if (hit_uvs.size() % 2 == 0){
		return false;
	}
	else{
		return true;
	}
}

VBVH_END_NAMESPACE
