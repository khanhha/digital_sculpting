#ifndef VBVH_DEFINE_H
#define VBVH_DEFINE_H

#include "BMesh/BMesh.h"
#include "tbb/scalable_allocator.h"
#include <Eigen/Dense>

#define VBVH_BEGIN_NAMESPACE  namespace VBvh {
#define VBVH_END_NAMESPACE    }


/*little macro so inline keyword works*/
#if defined(_MSC_VER)
#  define VBVH_INLINE __forceinline
#else
#  if (defined(__APPLE__) && defined(__ppc__))
/* static inline __attribute__ here breaks osx ppc gcc42 build */
#    define VBVH_INLINE  __attribute__((always_inline)) __attribute__((__unused__))
#  else
#    define VBVH_INLINE  inline __attribute__((always_inline)) __attribute__((__unused__))
#  endif
#endif

using namespace VM;
using namespace  Eigen;
VBVH_BEGIN_NAMESPACE
const std::string cd_node_off = "nodePtrLayer";
const std::string cd_arr_off  = "arrayOffLayer";

typedef std::vector<BMFace*, tbb::scalable_allocator<BMFace*>> BMFaceVector;
typedef std::vector<BMVert*, tbb::scalable_allocator<BMVert*>> BMVertVector;


/*! Maximal depth of the BVH. */
static const size_t MAX_BUILD_DEPTH = 32;
static const size_t MAX_BUILD_DEPTH_LEAF = MAX_BUILD_DEPTH + 16;
static const size_t MAX_DEPTH = MAX_BUILD_DEPTH_LEAF + MAX_BUILD_DEPTH_LEAF + MAX_BUILD_DEPTH;

// BVH4 single ray traversal implementation. 
// Ray structure. Contains all information about a ray including
//precomputed reciprocal direction. 
struct Ray
{
	// Default construction does nothing.
	Ray() {}

	// Constructs a ray from origin, direction, and ray segment. Near
	//  has to be smaller than far. 
	Ray(const Vector3f& org, const Vector3f& dir, bool useOrigin = false, float tnear = 0.0f, float tfar = FLT_MAX)
		: org(org), dir(dir), tnear(tnear), tfar(tfar), prim(0), useOriginCoord(useOrigin), hit(false)
	{
		invDir = Vector3f(1.0f / dir.x(), 1.0f / dir.y(), 1.0f / dir.z());
		sign[0] = dir[0] < 0.0f;
		sign[1] = dir[1] < 0.0f;
		sign[2] = dir[2] < 0.0f;
	}

	// Tests if we hit something. 
	operator bool() const { return hit; }

public:
	Vector3f org;        // Ray origin
	Vector3f dir;        // Ray direction
	Vector3f invDir;
	size_t sign[3];
	float tnear;       // Start of ray segment
	float tfar;        // End of ray segment

	bool hit;
	size_t prim;        // primitive ID
	bool useOriginCoord;
};

VBVH_END_NAMESPACE

#endif