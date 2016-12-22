#include "VBvh/VObjectPartition.h"
#include "BaseLib/Point3Dd.h"

VBVH_BEGIN_NAMESPACE

//////////////////////////////////////////////////////////////////////////////
//                        Bin Mapping                                       //
//////////////////////////////////////////////////////////////////////////////

ObjectPartition::Mapping::Mapping(const VPrimInfo& pinfo)
{
	num = std::min<size_t>(maxBins, size_t(4.0f + 0.05f*pinfo.size()));
	const ssef diag = (ssef)pinfo.centBounds.size();
	scale = select(diag > ssef(1E-19f), rcp(diag) * ssef(0.99f*num), ssef(0.0f));
	ofs = (ssef)pinfo.centBounds.lower;
}

Vec3ia ObjectPartition::Mapping::bin(const Vec3fa& p) const
{
	const ssei i = floori((ssef(p) - ofs)*scale);
#if 1
	assert(i[0] >= 0 && i[0] < num);
	assert(i[1] >= 0 && i[1] < num);
	assert(i[2] >= 0 && i[2] < num);
	return Vec3ia(i);
#else
	return Vec3ia(clamp(i, ssei(0), ssei(num - 1)));
#endif
}

Vec3ia ObjectPartition::Mapping::bin_unsafe(const Vec3fa& p) const {
	return Vec3ia(floori((ssef(p) - ofs)*scale));
}

bool ObjectPartition::Mapping::invalid(const int dim) const {
	return scale[dim] == 0.0f;
}

//////////////////////////////////////////////////////////////////////////////
//                             Binning                                      //
//////////////////////////////////////////////////////////////////////////////
ObjectPartition::BinInfo::BinInfo() {
	clear();
}

void ObjectPartition::BinInfo::clear()
{
	for (size_t i = 0; i < maxBins; i++) {
		bounds[i][0] = bounds[i][1] = bounds[i][2] = bounds[i][3] = empty;
		counts[i] = 0;
	}
}

void ObjectPartition::BinInfo::bin(const VPrimRef* prims, size_t num, const Mapping& mapping)
{
	if (num == 0) return;

	size_t i;
	for (i = 0; i < num - 1; i += 2)
	{
		/*! map even and odd primitive to bin */
		const BBox3fa prim0 = prims[i + 0].bounds(); const Vec3fa center0 = Vec3fa(center2(prim0)); const Vec3ia bin0 = mapping.bin(center0);
		const BBox3fa prim1 = prims[i + 1].bounds(); const Vec3fa center1 = Vec3fa(center2(prim1)); const Vec3ia bin1 = mapping.bin(center1);

		/*! increase bounds for bins for even primitive */
		const int b00 = bin0.x; counts[b00][0]++; bounds[b00][0].extend(prim0);
		const int b01 = bin0.y; counts[b01][1]++; bounds[b01][1].extend(prim0);
		const int b02 = bin0.z; counts[b02][2]++; bounds[b02][2].extend(prim0);

		/*! increase bounds of bins for odd primitive */
		const int b10 = bin1.x; counts[b10][0]++; bounds[b10][0].extend(prim1);
		const int b11 = bin1.y; counts[b11][1]++; bounds[b11][1].extend(prim1);
		const int b12 = bin1.z; counts[b12][2]++; bounds[b12][2].extend(prim1);
	}

	/*! for uneven number of primitives */
	if (i < num)
	{
		/*! map primitive to bin */
		const BBox3fa prim0 = prims[i].bounds(); const Vec3fa center0 = Vec3fa(center2(prim0)); const Vec3ia bin0 = mapping.bin(center0);

		/*! increase bounds of bins */
		const int b00 = bin0.x; counts[b00][0]++; bounds[b00][0].extend(prim0);
		const int b01 = bin0.y; counts[b01][1]++; bounds[b01][1].extend(prim0);
		const int b02 = bin0.z; counts[b02][2]++; bounds[b02][2].extend(prim0);
	}
}

const ObjectPartition::Split ObjectPartition::find(
	VPrimRef *__restrict__ const prims, const size_t begin, const size_t end,
	const VPrimInfo& pinfo, const size_t logBlockSize)
{
	BinInfo binner;
	const Mapping mapping(pinfo);
	binner.bin(prims + begin, end - begin, mapping);
	return binner.best(mapping, logBlockSize);
}


ObjectPartition::Split ObjectPartition::BinInfo::best(const Mapping& mapping, const size_t blocks_shift)
{
	/* sweep from right to left and compute parallel prefix of merged bounds */
	ssef rAreas[maxBins];
	ssei rCounts[maxBins];
	ssei count = 0; BBox3fa bx = empty; BBox3fa by = empty; BBox3fa bz = empty;
	for (size_t i = mapping.size() - 1; i > 0; i--)
	{
		count += counts[i];
		rCounts[i] = count;
		bx.extend(bounds[i][0]); rAreas[i][0] = halfArea(bx);
		by.extend(bounds[i][1]); rAreas[i][1] = halfArea(by);
		bz.extend(bounds[i][2]); rAreas[i][2] = halfArea(bz);
	}

	/* sweep from left to right and compute SAH */
	ssei blocks_add = (1 << blocks_shift) - 1;
	ssei ii = 1; ssef vbestSAH = pos_inf; ssei vbestPos = 0;
	count = 0; bx = empty; by = empty; bz = empty;
	for (size_t i = 1; i < mapping.size(); i++, ii += 1)
	{
		count += counts[i - 1];
		bx.extend(bounds[i - 1][0]); float Ax = halfArea(bx);
		by.extend(bounds[i - 1][1]); float Ay = halfArea(by);
		bz.extend(bounds[i - 1][2]); float Az = halfArea(bz);
		const ssef lArea = ssef(Ax, Ay, Az, Az);
		const ssef rArea = rAreas[i];
		const ssei lCount = (count + blocks_add) >> blocks_shift;
		const ssei rCount = (rCounts[i] + blocks_add) >> blocks_shift;
		const ssef sah = lArea*ssef(lCount) + rArea*ssef(rCount);
		vbestPos = select(sah < vbestSAH, ii, vbestPos);
		vbestSAH = select(sah < vbestSAH, sah, vbestSAH);
	}

	/* find best dimension */
	float bestSAH = inf;
	int   bestDim = -1;
	int   bestPos = 0;
	int   bestLeft = 0;
	for (size_t dim = 0; dim < 3; dim++)
	{
		/* ignore zero sized dimensions */
		if (unlikely(mapping.invalid(dim)))
			continue;

		/* test if this is a better dimension */
		if (vbestSAH[dim] < bestSAH && vbestPos[dim] != 0) {
			bestDim = dim;
			bestPos = vbestPos[dim];
			bestSAH = vbestSAH[dim];
		}
	}
	return ObjectPartition::Split(bestSAH, bestDim, bestPos, mapping);
}

void ObjectPartition::Split::partition(
	VPrimRef *__restrict__ const prims, const size_t begin, const size_t end, 
	VPrimInfo& left, VPrimInfo& right) const
{
	assert(valid());
	VCentGeomBBox3fa local_left(empty);
	VCentGeomBBox3fa local_right(empty);

	assert(begin <= end);
	VPrimRef* l = prims + begin;
	VPrimRef* r = prims + end - 1;

	while (1)
	{
		while (likely(l <= r && mapping.bin_unsafe(center2(l->bounds()))[dim] < pos))
		{
			local_left.extend(l->bounds());
			++l;
		}
		while (likely(l <= r && mapping.bin_unsafe(center2(r->bounds()))[dim] >= pos))
		{
			local_right.extend(r->bounds());
			--r;
		}
		if (r < l) break;

		const BBox3fa bl = l->bounds();
		const BBox3fa br = r->bounds();
		local_left.extend(br);
		local_right.extend(bl);
		*(BBox3fa*)l = br;
		*(BBox3fa*)r = bl;
		l++; r--;
	}

	unsigned int center = l - prims;
	new (&left)		VPrimInfo(begin, center, local_left.geomBounds, local_left.centBounds);
	new (&right)	VPrimInfo(center, end, local_right.geomBounds, local_right.centBounds);
	assert(area(left.geomBounds) >= 0.0f);
	assert(area(right.geomBounds) >= 0.0f);
}

VBVH_END_NAMESPACE