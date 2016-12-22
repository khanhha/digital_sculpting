// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGL4TextureRT.h>
using namespace gte;

GL4TextureRT::GL4TextureRT(TextureRT const* texture)
    :
    GL4Texture2(texture)
{
}

GEObject* GL4TextureRT::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_RT)
    {
        return new GL4TextureRT(static_cast<TextureRT const*>(object));
    }

    qCritical("Invalid object type.");
    return nullptr;
}

bool GL4TextureRT::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture && texture->HasMipmaps() && texture->WantAutogenerateMipmaps();
}
