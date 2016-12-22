// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGL4TextureDS.h>
using namespace gte;

GL4TextureDS::GL4TextureDS(TextureDS const* texture)
    :
    GL4Texture2(texture)
{
}

GEObject* GL4TextureDS::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_DS)
    {
        return new GL4TextureDS(static_cast<TextureDS const*>(object));
    }

    qCritical("Invalid object type.");
    return nullptr;
}

bool GL4TextureDS::CanAutoGenerateMipmaps() const
{
    return false;
}
