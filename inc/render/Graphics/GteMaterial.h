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

class GTE_IMPEXP Material
{
public:
    // Construction.
    Material();

    // (r,g,b,*): default (0,0,0,1)
	EVector4 emissive;

    // (r,g,b,*): default (0,0,0,1)
	EVector4 ambient;

    // (r,g,b,a): default (0,0,0,1)
	EVector4 diffuse;

    // (r,g,b,specularPower): default (0,0,0,1)
	EVector4 specular;
};

}
