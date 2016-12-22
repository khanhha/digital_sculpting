#ifndef SCULPT_SSE_H
#define SCULPT_SSE_H
#include "VBvh/common/sys/platform.h"
#include "VBvh/common/sys/intrinsics.h"
#include "VBvh/common/simd/sse_special.h"
#include "VBvh/VBvhDefine.h"

/* Workaround for Compiler bug in VS2008 */
#if (!defined(__SSE4_1__) || defined(_MSC_VER) && (_MSC_VER < 1600)) && !defined(__INTEL_COMPILER)
  #define _mm_blendv_ps __emu_mm_blendv_ps
  __forceinline __m128 _mm_blendv_ps( __m128 f, __m128 t, __m128 mask ) { 
    return _mm_or_ps(_mm_and_ps(mask, t), _mm_andnot_ps(mask, f)); 
  }
#endif

/* Workaround for Compiler bug in VS2008 */
#if defined(_MSC_VER) && (_MSC_VER < 1600) && !defined(__INTEL_COMPILER)
  #define _mm_extract_epi32 __emu_mm_extract_epi32
  __forceinline int _mm_extract_epi32( __m128i input, const int i ) {
    return input.m128i_i32[i];
  }
#endif

VBVH_BEGIN_NAMESPACE

  extern const __m128 _mm_lookupmask_ps[16];

  struct sseb;
  struct ssei;
  struct ssef;

#if !defined(__MIC__)
  typedef ssef ssef_t;
  typedef ssei ssei_t;

  typedef ssef ssef_m;
  typedef ssei ssei_m;
#endif

VBVH_END_NAMESPACE

#include "VBvh/common/simd/sseb.h"
#include "VBvh/common/simd/ssei.h"
#include "VBvh/common/simd/ssef.h"
#endif
