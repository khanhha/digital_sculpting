// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGL4GraphicsObject.h>
using namespace gte;

GL4GraphicsObject::GL4GraphicsObject(GraphicsObject const* gtObject)
    :
    GEObject(gtObject),
    mGLHandle(0)
{
}

void GL4GraphicsObject::SetName(std::string const& name)
{
    // TODO:  Determine how to tag OpenGL objects with names?
    mName = name;
}
