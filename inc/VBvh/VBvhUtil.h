#ifndef VBVH_UTIL_H
#define VBVH_UTIL_H
#include "VBvhDefine.h"
#include "BaseLib/Point3Dd.h"
#include "BMeshBvh.h"
#include "VBvh/VPrimRef.h"
#include "VBvh/VPrimInfor.h"
#include <vector>

VBVH_BEGIN_NAMESPACE

bool		isectRayBB(const Ray* ray, const Vector3f &lower, const Vector3f &upper, float& hitParam);
void		facesBoundsNormalUpdate(BMFace * const *faces, size_t num, Vector3f &lower, Vector3f &upper);
void		BMFacesPrimInfoCompute(BMFace *const* faces, size_t num, VPrimRef *prims, VPrimInfo &info, bool threaded = true);
void		debugOutputNodeBB(BaseNode *node, std::vector<Point3Dd> &segments);
Vector3f	convert(const Vec3fa &p);

VBVH_END_NAMESPACE
#endif