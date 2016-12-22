#include "bm/bm_isct_bool_include.h"
#include "bm/bm_isct_boolean.h"
#include "bm/bm_isct_outer_hull_exact.h"
namespace bm_isct
{
	BMesh* compute_union(VM::BMesh *bm0, VBvh::BMBvh *bvh0, VM::BMesh *bm1, VBvh::BMBvh *bvh1)
	{
		BmBoolean boolean(bm0, bvh0, bm1, bvh1);
		boolean.compute_union();
		return boolean.result();
	}

	BMesh* compute_difference(VM::BMesh *bm0, VBvh::BMBvh *bvh0, VM::BMesh *bm1, VBvh::BMBvh *bvh1)
	{
		BmBoolean boolean(bm0, bvh0, bm1, bvh1);
		boolean.compute_differnece();
		return boolean.result();
	}

	BMesh* compute_intersection(VM::BMesh *bm0, VBvh::BMBvh *bvh0, VM::BMesh *bm1, VBvh::BMBvh *bvh1)
	{
		BmBoolean boolean(bm0, bvh0, bm1, bvh1);
		boolean.compute_intersection();
		return boolean.result();
	}

	BMesh * resolve_self_isct(VM::BMesh *bm0)
	{
		BmOuterHullExtract outerhull(bm0);
		outerhull.extract();
		return outerhull.result();
	}

}

