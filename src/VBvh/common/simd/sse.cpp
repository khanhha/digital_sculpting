#include "common/simd/sse.h"

namespace VBvh {
  const __m128 _mm_lookupmask_ps[16] = {
    _mm_castsi128_ps(_mm_set_epi32( 0, 0, 0, 0)),
    _mm_castsi128_ps(_mm_set_epi32( 0, 0, 0,-1)),
    _mm_castsi128_ps(_mm_set_epi32( 0, 0,-1, 0)),
    _mm_castsi128_ps(_mm_set_epi32( 0, 0,-1,-1)),
    _mm_castsi128_ps(_mm_set_epi32( 0,-1, 0, 0)),
    _mm_castsi128_ps(_mm_set_epi32( 0,-1, 0,-1)),
    _mm_castsi128_ps(_mm_set_epi32( 0,-1,-1, 0)),
    _mm_castsi128_ps(_mm_set_epi32( 0,-1,-1,-1)),
    _mm_castsi128_ps(_mm_set_epi32(-1, 0, 0, 0)),
    _mm_castsi128_ps(_mm_set_epi32(-1, 0, 0,-1)),
    _mm_castsi128_ps(_mm_set_epi32(-1, 0,-1, 0)),
    _mm_castsi128_ps(_mm_set_epi32(-1, 0,-1,-1)),
    _mm_castsi128_ps(_mm_set_epi32(-1,-1, 0, 0)),
    _mm_castsi128_ps(_mm_set_epi32(-1,-1, 0,-1)),
    _mm_castsi128_ps(_mm_set_epi32(-1,-1,-1, 0)),
    _mm_castsi128_ps(_mm_set_epi32(-1,-1,-1,-1))
  };
}
