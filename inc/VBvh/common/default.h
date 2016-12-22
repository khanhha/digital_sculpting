#ifndef SCULPT_DEFAULT_H
#define SCULPT_DEFAULT_H

#include "common/sys/platform.h"
#include "common/sys/intrinsics.h"
#include "common/sys/sysinfo.h"

#include "common/math/mymath.h"
#include "common/math/vec2.h"
#include "common/math/vec3.h"
#include "common/math/vec4.h"
#include "common/math/bbox.h"

#include "common/simd/simd.h"

#include <map>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <array>

#include "VBvh/VBvhDefine.h"
VBVH_BEGIN_NAMESPACE

    /* we consider floating point numbers in that range as valid input numbers */
#define VALID_FLOAT_RANGE  1.844E18f

    __forceinline bool inFloatRange(const float v) {
        return (v > -VALID_FLOAT_RANGE) && (v < +VALID_FLOAT_RANGE);
    };
    __forceinline bool inFloatRange(const Vec3fa& v) {
        return all(gt_mask(v, Vec3fa_t(-VALID_FLOAT_RANGE)) & lt_mask(v, Vec3fa_t(+VALID_FLOAT_RANGE)));
    };
    __forceinline bool inFloatRange(const BBox3fa& v) {
        return all(gt_mask(v.lower, Vec3fa_t(-VALID_FLOAT_RANGE)) & lt_mask(v.upper, Vec3fa_t(+VALID_FLOAT_RANGE)));
    };

    /*! CPU features */
    static const int SSE = CPU_FEATURE_SSE;
    static const int SSE2 = SSE | CPU_FEATURE_SSE2;
    static const int SSE3 = SSE2 | CPU_FEATURE_SSE3;
    static const int SSSE3 = SSE3 | CPU_FEATURE_SSSE3;
    static const int SSE41 = SSSE3 | CPU_FEATURE_SSE41;
    static const int SSE42 = SSE41 | CPU_FEATURE_SSE42 | CPU_FEATURE_POPCNT;
    static const int AVX = SSE42 | CPU_FEATURE_AVX;
    static const int AVXI = AVX | CPU_FEATURE_F16C | CPU_FEATURE_RDRAND;
    static const int AVX2 = AVXI | CPU_FEATURE_AVX2 | CPU_FEATURE_FMA3 | CPU_FEATURE_BMI1 | CPU_FEATURE_BMI2 | CPU_FEATURE_LZCNT;
    static const int KNC = CPU_FEATURE_KNC;

    __forceinline bool has_feature(const int feature) {
        int cpu_features = getCPUFeatures();
        return (cpu_features & feature) == feature;
    }

#if defined (__MIC__)
#  define ISA KNC
#elif defined (__AVX2__)
#  define ISA AVX2
#elif defined(__AVXI__)
#  define ISA AVXI
#elif defined(__AVX__)
#  define ISA AVX
#elif defined (__SSE4_2__)
#  define ISA SSE42
#elif defined (__SSE4_1__)
#  define ISA SSE41
#elif defined(__SSSE3__)
#  define ISA SSSE3
#elif defined(__SSE3__)
#  define ISA SSE3
#elif defined(__SSE2__)
#  define ISA SSE2
#elif defined(__SSE__)
#  define ISA SSE
#elif defined (__MACOSX__)
#  define ISA SSSE3
#elif defined (__LINUX__)
#  define ISA SSE2
#elif defined (__WIN32__)
#  define ISA SSE2
#else 
#  define ISA SSE2
#endif

    inline std::string stringOfISA(int features)
    {
        if (features == SSE) return "SSE";
        if (features == SSE2) return "SSE2";
        if (features == SSE3) return "SSE3";
        if (features == SSSE3) return "SSSE3";
        if (features == SSE41) return "SSE4_1";
        if (features == SSE42) return "SSE4_2";
        if (features == AVX) return "AVX";
        if (features == AVXI) return "AVXI";
        if (features == AVX2) return "AVX2";
        if (features == KNC) return "KNC";
        return "UNKNOWN";
    }

#if defined (__SSE__) // || defined (__MIC__)
    typedef Vec2<sseb> sse2b;
    typedef Vec3<sseb> sse3b;
    typedef Vec2<ssei> sse2i;
    typedef Vec3<ssei> sse3i;
    typedef Vec2<ssef> sse2f;
    typedef Vec3<ssef> sse3f;
    typedef Vec4<ssef> sse4f;
    typedef BBox<sse3f > BBoxSSE3f;
#endif

    typedef void(*ErrorFunc) ();
VBVH_END_NAMESPACE
#endif
