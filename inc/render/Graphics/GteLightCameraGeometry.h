// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once
#include <GTEngineDEF.h>

namespace gte
{

struct GTE_IMPEXP LightCameraGeometry
{
    // Construction.
    LightCameraGeometry();

	EVector4 lightModelPosition;      // default: (0,0,0,1)
	EVector4 lightModelDirection;     // default: (0,0,-1,0)
	EVector4 lightModelUp;            // default: (0,1,0,0)
	EVector4 lightModelRight;         // default: (1,0,0,0)

	EVector4 cameraModelPosition;     // default: (0,0,0,1)
	EVector4 cameraModelDirection;    // default: (0,0,-1,0)
	EVector4 cameraModelUp;           // default: (0,1,0,0)
	EVector4 cameraModelRight;        // default: (1,0,0,0)
};

}
