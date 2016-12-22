#ifndef BASELIB_MATHBASE_H
#define BASELIB_MATHBASE_H

#include "BaseLib/UtilMacro.h"

//#define RAD2DEG(_rad) ((_rad) * (180.0 / M_PI))
//#define DEG2RAD(_deg) ((_deg) * (M_PI / 180.0))


#define RAD2DEGF(_rad) ((_rad) * (float)(180.0 / M_PI))
#define DEG2RADF(_deg) ((_deg) * (float)(M_PI / 180.0))

namespace MathBase
{
	MINLINE float saacos(float fac)
	{
		if (UNLIKELY(fac <= -1.0f)) return (float)M_PI;
		else if (UNLIKELY(fac >= 1.0f)) return 0.0f;
		else                             return acosf(fac);
	}

	MINLINE float saasin(float fac)
	{
		if (UNLIKELY(fac <= -1.0f)) return (float)-M_PI / 2.0f;
		else if (UNLIKELY(fac >= 1.0f)) return (float)M_PI / 2.0f;
		else                             return asinf(fac);
	}

	MINLINE float sasqrt(float fac)
	{
		if (UNLIKELY(fac <= 0.0f)) return 0.0f;
		else                       return sqrtf(fac);
	}

	MINLINE float saacosf(float fac)
	{
		if (UNLIKELY(fac <= -1.0f)) return (float)M_PI;
		else if (UNLIKELY(fac >= 1.0f)) return 0.0f;
		else                             return acosf(fac);
	}

	MINLINE float saasinf(float fac)
	{
		if (UNLIKELY(fac <= -1.0f)) return (float)-M_PI / 2.0f;
		else if (UNLIKELY(fac >= 1.0f)) return (float)M_PI / 2.0f;
		else                             return asinf(fac);
	}
};
#endif