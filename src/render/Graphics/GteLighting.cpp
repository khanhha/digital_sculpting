// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <boost/math/constants/constants.hpp>
#include <Graphics/GteLighting.h>
using namespace gte;

Lighting::Lighting()
    :
    ambient({ 1.0f, 1.0f, 1.0f, 1.0f }),
    diffuse({ 1.0f, 1.0f, 1.0f, 1.0f }),
    specular({ 1.0f, 1.0f, 1.0f, 1.0f }),
    spotCutoff({boost::math::constants::half_pi<float>(), 0.0f, 1.0f, 1.0f }),
    attenuation({ 1.0f, 0.0f, 0.0f, 1.0f })
{
}
