#ifndef VBVH_MORTON_H
#define VBVH_MORTON_H
#include "VBvhDefine.h"
VBVH_BEGIN_NAMESPACE

static const size_t LATTICE_BITS_PER_DIM = 10;
static const size_t LATTICE_SIZE_PER_DIM = size_t(1) << LATTICE_BITS_PER_DIM;

struct __declspec(align(8)) VMortonID32Bit
{
	union {
		struct {
			unsigned int code;
			unsigned int index;
			//uint64 index;
		};
		//int64 all;
	};

	__forceinline operator unsigned() const { return code; }

	__forceinline unsigned int get(const unsigned int shift, const unsigned and_mask) const {
		return (code >> shift) & and_mask;
	}

	/*__forceinline void operator=(const MortonID32Bit& v) {
	all = v.all;
	};*/

	__forceinline friend std::ostream &operator<<(std::ostream &o, const VMortonID32Bit& mc) {
		o << "index " << mc.index << " code = " << mc.code;
		return o;
	}

	__forceinline bool operator<(const VMortonID32Bit &m) const { return code < m.code; }
	__forceinline bool operator>(const VMortonID32Bit &m) const { return code > m.code; }
};

VBVH_END_NAMESPACE
#endif