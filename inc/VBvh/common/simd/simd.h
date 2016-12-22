#ifndef SCULPT_SIMD_H
#define SCULPT_SIMD_H
#include "VBvh/common/math/mymath.h"

/* include SSE emulation for Xeon Phi */
#if defined (__MIC__)
//#  include "simd/sse_mic.h"
#  include "VBvh/common/simd/mic.h"
#endif

/* include SSE wrapper classes */
#if defined(__SSE__)
#  include "VBvh/common/simd/sse.h"
#endif

/* include AVX wrapper classes */
#if defined(__AVX__)
#include "VBvh/common/simd/avx.h"
#endif

#if defined (__AVX__)
#define AVX_ZERO_UPPER() _mm256_zeroupper()
#else
#define AVX_ZERO_UPPER()
#endif

#endif
