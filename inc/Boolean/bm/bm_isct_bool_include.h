#ifndef BOOLEAN_BM_ISCT_BOOL_INCLUDE_H
#define BOOLEAN_BM_ISCT_BOOL_INCLUDE_H
#include "BMesh/BMesh.h"
#include "VBvh/BMeshBvh.h"

namespace bm_isct
{
	BMesh *compute_union(VM::BMesh *bm0, VBvh::BMBvh *bvh0, VM::BMesh *bm1, VBvh::BMBvh *bvh1);
	BMesh *compute_difference(VM::BMesh *bm0, VBvh::BMBvh *bvh0, VM::BMesh *bm1, VBvh::BMBvh *bvh1);
	BMesh *compute_intersection(VM::BMesh *bm0, VBvh::BMBvh *bvh0, VM::BMesh *bm1, VBvh::BMBvh *bvh1);
	BMesh *resolve_self_isct(VM::BMesh *bm0);
}
#endif