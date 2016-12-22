// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGL4Texture3.h>
using namespace gte;

GL4Texture3::~GL4Texture3()
{
	GL::function().glDeleteTextures(1, &mGLHandle);
}

GL4Texture3::GL4Texture3(Texture3 const* texture)
    :
    GL4TextureSingle(texture, GL_TEXTURE_3D, GL_TEXTURE_BINDING_3D)
{
    // Create a texture structure.
	GL::function().glGenTextures(1, &mGLHandle);
	GL::function().glBindTexture(GL_TEXTURE_3D, mGLHandle);

    // Allocate (immutable) texture storage for all levels.
    auto const width = texture->GetDimension(0);
    auto const height = texture->GetDimension(1);
    auto const depth = texture->GetDimension(2);
	GL::function().glTexStorage3D(GL_TEXTURE_3D, mNumLevels, mInternalFormat, width, height, depth);

    Initialize();

    // Cannot leave this texture bound.
	GL::function().glBindTexture(GL_TEXTURE_3D, 0);

    // Create a staging texture if requested.
    CreateStaging();
}

GEObject* GL4Texture3::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE3)
    {
        return new GL4Texture3(static_cast<Texture3 const*>(object));
    }

    qCritical("Invalid object type.");
    return nullptr;
}

bool GL4Texture3::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture && texture->HasMipmaps() && texture->WantAutogenerateMipmaps();
}

void GL4Texture3::LoadTextureLevel(unsigned int level, void const* data)
{
    auto texture = GetTexture();
    if (texture && level < texture->GetNumLevels())
    {
        auto width = texture->GetDimensionFor(level, 0);
        auto height = texture->GetDimensionFor(level, 1);
        auto depth = texture->GetDimensionFor(level, 2);

		GL::function().glTexSubImage3D(GL_TEXTURE_3D, level, 0, 0, 0, width, height, depth,
            mExternalFormat, mExternalType, data);
    }
}
