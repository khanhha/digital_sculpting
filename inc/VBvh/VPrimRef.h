#ifndef VBVH_PRIM_REF_H
#define VBVH_PRIM_REF_H
#include "VBvhDefine.h"
#include "common\math\bbox.h"

using namespace VBvh;

VBVH_BEGIN_NAMESPACE

/*! A primitive reference stores the bounds of the primitive and its ID. */
struct __declspec(align(32))  VPrimRef
{
	VBVH_INLINE VPrimRef() {}

	VBVH_INLINE VPrimRef(const BBox3fa& bounds, size_t id) {
#if defined(__X86_64__)
		lower = bounds.lower; lower.u = id & 0xFFFFFFFF;
		upper = bounds.upper; upper.u = (id >> 32) & 0xFFFFFFFF;
#else
		lower = bounds.lower; lower.u = id;
		upper = bounds.upper; upper.u = 0;
#endif
	}

	VBVH_INLINE void set(const Vec3fa& l, const Vec3fa& u, size_t id)
	{
#if defined(__X86_64__)
		lower = l; lower.u = id & 0xFFFFFFFF;
		upper = u; upper.u = (id >> 32) & 0xFFFFFFFF;
#else
		lower = l; lower.u = id;
		upper = u; upper.u = 0;
#endif
	}

	/*! calculates twice the center of the primitive */
	VBVH_INLINE const Vec3fa center2() const {
		return lower + upper;
	}

	VBVH_INLINE const BBox3fa bounds() const {
		return BBox3fa(lower, upper);
	}

	VBVH_INLINE size_t ID() const {
#if defined(__X86_64__)
		return size_t(lower.u) + (size_t(upper.u) << 32);
#else
		return size_t(lower.u);
#endif
	}

	friend VBVH_INLINE bool operator<(const VPrimRef& p0, const VPrimRef& p1) {
		return p0.ID() < p1.ID();
	}
public:
	Vec3fa lower;
	Vec3fa upper;
};

__forceinline void xchg(VPrimRef& a, VPrimRef& b)
{
#if defined(__AVX__) || defined(__AVX2__)

	const avxf aa = load8f((float*)&a);
	const avxf bb = load8f((float*)&b);
	store8f((float*)&a, bb);
	store8f((float*)&b, aa);
#elif defined(__MIC__)
	const mic_f aa = uload16f_low((float*)&a.lower);
	const mic_f bb = uload16f_low((float*)&b.lower);
	compactustore16f_low(0xff, (float*)&b.lower, aa);
	compactustore16f_low(0xff, (float*)&a.lower, bb);
#else
	std::swap(a, b);
#endif
}

__forceinline bool subset(const VPrimRef& a, const VPrimRef& b)
{
	for (size_t i = 0; i < 3; i++) if (a.lower[i] < b.lower[i]) return false;
	for (size_t i = 0; i < 3; i++) if (a.upper[i] > b.upper[i]) return false;
	return true;
}


__forceinline float area(const VPrimRef& a)
{
	const Vec3fa d = a.upper - a.lower;
	return 2.0f*(d.x*(d.y + d.z) + d.y*d.z);
}

VBVH_END_NAMESPACE
#endif