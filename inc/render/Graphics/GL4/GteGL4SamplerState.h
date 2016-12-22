// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <Graphics/GteSamplerState.h>
#include <Graphics/GL4/GteGL4DrawingState.h>

namespace gte
{

class GTE_IMPEXP GL4SamplerState : public GL4DrawingState
{
public:
    // Construction and destruction.
    virtual ~GL4SamplerState();
    GL4SamplerState(SamplerState const* samplerState);
    static GEObject* Create(void* unused, GraphicsObject const* object);

    // Member access.
    inline SamplerState* GetSamplerState();

private:
    // Conversions from GTEngine values to GL4 values.
    static GLint const msMode[];
};

inline SamplerState* GL4SamplerState::GetSamplerState()
{
    return static_cast<SamplerState*>(mGTObject);
}

}
