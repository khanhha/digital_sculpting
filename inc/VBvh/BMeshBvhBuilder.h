#ifndef VBVH_BMESH_BVH_BUILDER_H
#define VBVH_BMESH_BVH_BUILDER_H

#include "BMesh/BMesh.h"
#include "VBvhMortonBuilder.h"
#include "VPrimRef.h"
#include "BMeshBvh.h"
#include "tbb/scalable_allocator.h"

using namespace VM;
using namespace VBvh;

VBVH_BEGIN_NAMESPACE

class BMeshBvhBuilder
{
public:
	BMeshBvhBuilder(BMesh *bm);
	BMBvh* build();
	void computeBounds(const std::vector<BMFace*> &faces, VPrimRef *prims, size_t tot);
	void addCustomDataLayer(BMeshBvhContext &info);
private:
	void computePrimInfo(VPrimRef *prims, size_t tot, VPrimInfo &infor);
	BMesh *_bmesh;
};

VBVH_END_NAMESPACE

#endif