#include "VBvhUtil.h"
#include "tbb/parallel_for.h"
VBVH_BEGIN_NAMESPACE
bool isectRayBB(const Ray* ray, const Vector3f &lower, const Vector3f &upper, float& hitParam)
{
	Vector3f bbox[2];
	float tmin, tmax, tymin, tymax, tzmin, tzmax;

	bbox[0] = lower;
	bbox[1] = upper;

	tmin = (bbox[ray->sign[0]].x() - ray->org.x()) * ray->invDir.x();
	tmax = (bbox[1 - ray->sign[0]].x() - ray->org.x()) * ray->invDir.x();

	tymin = (bbox[ray->sign[1]].y() - ray->org.y()) * ray->invDir.y();
	tymax = (bbox[1 - ray->sign[1]].y() - ray->org.y()) * ray->invDir.y();

	if ((tmin > tymax) || (tymin > tmax))
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	tzmin = (bbox[ray->sign[2]].z()     - ray->org.z()) * ray->invDir.z();
	tzmax = (bbox[1 - ray->sign[2]].z() - ray->org.z()) * ray->invDir.z();

	if ((tmin > tzmax) || (tzmin > tmax))
		return false;

	if (tzmin > tmin)
		tmin = tzmin;

	hitParam = tmin;

	return true;
}

void facesBoundsNormalUpdate(BMFace * const *faces, size_t num, Vector3f &lower, Vector3f &upper)
{
	const float fltmax = std::numeric_limits<float>::max();
	const float fltmin = std::numeric_limits<float>::lowest();
	lower = Vector3f(fltmax, fltmax, fltmax);
	upper = Vector3f(fltmin, fltmin, fltmin);
	Vector3f flower, fupper;
	for (size_t i = 0; i < num; ++i){
		BM_face_calc_bounds(faces[i], flower, fupper);
		BM_face_normal_update(faces[i]);
		lower = lower.cwiseMin(flower);
		upper = upper.cwiseMax(fupper);
	}
}


void BMFacesPrimInfoCompute(BMFace * const *faces, size_t num, VPrimRef *prims, VPrimInfo &info, bool threaded/*true*/)
{
	info = VPrimInfo(empty);
	auto range_functor = [&](const tbb::blocked_range<size_t> &range)
	{
		Vector3f flower, fupper;
		for (size_t i = range.begin(); i < range.end(); ++i){
			const BMFace* face = faces[i];
			VPrimRef& prim = prims[i];
			BM_face_calc_bounds(face, flower, fupper);
			prim.set(
				Vec3fa(flower[0], flower[1], flower[2]),
				Vec3fa(fupper[0], fupper[1], fupper[2]),
				reinterpret_cast<size_t>(face));

			info.add(prim.bounds(), prim.center2());
		}
	};

	if (threaded){
		tbb::parallel_for(tbb::blocked_range<size_t>(0, num), range_functor);
	}
	else{
		range_functor(tbb::blocked_range<size_t>(0, num));
	}
}


void debugOutputNodeBB(BaseNode *node, std::vector<Point3Dd> &segments)
{
	Point3Dd listVertex[8];
	Point3Dd bbmin = Point3Dd(node->lower().x, node->lower().y, node->lower().z);
	Point3Dd bbmax = Point3Dd(node->upper().x, node->upper().y, node->upper().z);

	double size[3];
	for (int i = 0; i < 3; i++)
		size[i] = bbmax[i] - bbmin[i];

	listVertex[0] = bbmin;

	listVertex[1][0] = bbmin[0];
	listVertex[1][1] = bbmin[1] + size[1];
	listVertex[1][2] = bbmin[2];

	listVertex[2][0] = bbmin[0] + size[0];
	listVertex[2][1] = bbmin[1] + size[1];
	listVertex[2][2] = bbmin[2];

	listVertex[3][0] = bbmin[0] + size[0];
	listVertex[3][1] = bbmin[1];
	listVertex[3][2] = bbmin[2];

	listVertex[4][0] = bbmin[0];
	listVertex[4][1] = bbmin[1];
	listVertex[4][2] = bbmin[2] + size[2];

	listVertex[5][0] = bbmin[0] + size[0];
	listVertex[5][1] = bbmin[1];
	listVertex[5][2] = bbmin[2] + size[2];

	listVertex[6][0] = bbmin[0] + size[0];;
	listVertex[6][1] = bbmin[1] + size[1];
	listVertex[6][2] = bbmin[2] + size[2];

	listVertex[7][0] = bbmin[0];
	listVertex[7][1] = bbmin[1] + size[1];
	listVertex[7][2] = bbmin[2] + size[2];

	segments.push_back(listVertex[0]);
	segments.push_back(listVertex[1]);
	segments.push_back(listVertex[1]);
	segments.push_back(listVertex[2]);
	segments.push_back(listVertex[2]);
	segments.push_back(listVertex[3]);
	segments.push_back(listVertex[3]);
	segments.push_back(listVertex[0]);

	segments.push_back(listVertex[4]);
	segments.push_back(listVertex[5]);
	segments.push_back(listVertex[5]);
	segments.push_back(listVertex[6]);
	segments.push_back(listVertex[6]);
	segments.push_back(listVertex[7]);
	segments.push_back(listVertex[7]);
	segments.push_back(listVertex[4]);

	segments.push_back(listVertex[0]);
	segments.push_back(listVertex[4]);
	segments.push_back(listVertex[1]);
	segments.push_back(listVertex[7]);
	segments.push_back(listVertex[2]);
	segments.push_back(listVertex[6]);
	segments.push_back(listVertex[3]);
	segments.push_back(listVertex[5]);
}

Vector3f convert(const Vec3fa &p)
{
	return Vector3f(p.x, p.y, p.z);
}

VBVH_END_NAMESPACE



