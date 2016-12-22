#include <smmintrin.h>

// Simple vector class
_MM_ALIGN16 class vector3f
{
public:
	// constructors
	inline vector3() : mmvalue(_mm_setzero_ps()) {}
	inline vector3(float x, float y, float z) : mmvalue(_mm_set_ps(0, z, y, x)) {}
	inline vector3(__m128 m) : mmvalue(m) {}

	// arithmetic operators with vector3
	inline vector3 operator+(const vector3& b) const { return _mm_add_ps(mmvalue, b.mmvalue); }
	inline vector3 operator-(const vector3& b) const { return _mm_sub_ps(mmvalue, b.mmvalue); }
	inline vector3 operator*(const vector3& b) const { return _mm_mul_ps(mmvalue, b.mmvalue); }
	inline vector3 operator/(const vector3& b) const { return _mm_div_ps(mmvalue, b.mmvalue); }

	// op= operators
	inline vector3& operator+=(const vector3& b) { mmvalue = _mm_add_ps(mmvalue, b.mmvalue); return *this; }
	inline vector3& operator-=(const vector3& b) { mmvalue = _mm_sub_ps(mmvalue, b.mmvalue); return *this; }
	inline vector3& operator*=(const vector3& b) { mmvalue = _mm_mul_ps(mmvalue, b.mmvalue); return *this; }
	inline vector3& operator/=(const vector3& b) { mmvalue = _mm_div_ps(mmvalue, b.mmvalue); return *this; }

	// arithmetic operators with float
	inline vector3 operator+(float b) const { return _mm_add_ps(mmvalue, _mm_set1_ps(b)); }
	inline vector3 operator-(float b) const { return _mm_sub_ps(mmvalue, _mm_set1_ps(b)); }
	inline vector3 operator*(float b) const { return _mm_mul_ps(mmvalue, _mm_set1_ps(b)); }
	inline vector3 operator/(float b) const { return _mm_div_ps(mmvalue, _mm_set1_ps(b)); }

	// op= operators with float
	inline vector3& operator+=(float b) { mmvalue = _mm_add_ps(mmvalue, _mm_set1_ps(b)); return *this; }
	inline vector3& operator-=(float b) { mmvalue = _mm_sub_ps(mmvalue, _mm_set1_ps(b)); return *this; }
	inline vector3& operator*=(float b) { mmvalue = _mm_mul_ps(mmvalue, _mm_set1_ps(b)); return *this; }
	inline vector3& operator/=(float b) { mmvalue = _mm_div_ps(mmvalue, _mm_set1_ps(b)); return *this; }

	// cross product
	inline vector3 cross(const vector3& b) const
	{
		return _mm_sub_ps(
			_mm_mul_ps(_mm_shuffle_ps(mmvalue, mmvalue, _MM_SHUFFLE(3, 0, 2, 1)), _mm_shuffle_ps(b.mmvalue, b.mmvalue, _MM_SHUFFLE(3, 1, 0, 2))), 
			_mm_mul_ps(_mm_shuffle_ps(mmvalue, mmvalue, _MM_SHUFFLE(3, 1, 0, 2)), _mm_shuffle_ps(b.mmvalue, b.mmvalue, _MM_SHUFFLE(3, 0, 2, 1)))
			);
	}

	// dot product with another vector
	inline float dot(const vector3& b) const { return _mm_cvtss_f32(_mm_dp_ps(mmvalue, b.mmvalue, 0x71)); }
	// length of the vector
	inline float length() const { return _mm_cvtss_f32(_mm_sqrt_ss(_mm_dp_ps(mmvalue, mmvalue, 0x71))); }
	// 1/length() of the vector
	inline float rlength() const { return _mm_cvtss_f32(_mm_rsqrt_ss(_mm_dp_ps(mmvalue, mmvalue, 0x71))); }
	// returns the vector scaled to unit length
	inline vector3 normalize() const { return _mm_mul_ps(mmvalue, _mm_rsqrt_ps(_mm_dp_ps(mmvalue, mmvalue, 0x7F))); }

	// overloaded operators that ensure alignment
	inline void* operator new[](size_t x) { return _aligned_malloc(x, 16); }
	inline void operator delete[](void* x) { if (x) _aligned_free(x); }

	// Member variables
	union
	{
		struct { float x, y, z; };    
		__m128 mmvalue;
	};
};

inline vector3 operator+(float a, const vector3& b) { return b + a; }
inline vector3 operator-(float a, const vector3& b) { return vector3(_mm_set1_ps(a)) - b; }
inline vector3 operator*(float a, const vector3& b) { return b * a; }
inline vector3 operator/(float a, const vector3& b) { return vector3(_mm_set1_ps(a)) / b; }