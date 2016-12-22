// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGL4TextureCube.h>
using namespace gte;

GL4TextureCube::~GL4TextureCube()
{
	GL::function().glDeleteTextures(1, &mGLHandle);
}

GL4TextureCube::GL4TextureCube(TextureCube const* texture)
    :
    GL4TextureArray(texture, GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BINDING_CUBE_MAP)
{
    // Create a texture structure.
	GL::function().glGenTextures(1, &mGLHandle);
	GL::function().glBindTexture(GL_TEXTURE_CUBE_MAP, mGLHandle);

    // Allocate (immutable) texture storage for all levels.
    auto const width = texture->GetDimension(0);
    auto const height = texture->GetDimension(1);
	GL::function().glTexStorage2D(GL_TEXTURE_CUBE_MAP, mNumLevels, mInternalFormat, width, height);

    Initialize();

    // Cannot leave this texture bound.
	GL::function().glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    // Create a staging texture if requested.
    CreateStaging();
}

GEObject* GL4TextureCube::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_TEXTURE_CUBE)
    {
        return new GL4TextureCube(static_cast<TextureCube const*>(object));
    }

    qCritical("Invalid object type.");
    return nullptr;
}

bool GL4TextureCube::CanAutoGenerateMipmaps() const
{
    auto texture = GetTexture();

    return texture && texture->HasMipmaps() && texture->WantAutogenerateMipmaps();
}

void GL4TextureCube::LoadTextureLevel(unsigned int item, unsigned int level, void const* data)
{
    auto texture = GetTexture();
    if (texture && level < texture->GetNumLevels())
    {
        auto const width = texture->GetDimension(0);
        auto const height = texture->GetDimension(1);

        // Each face in the TextureCube has a unique GL target.
        GLenum targetFace = msCubeFaceTarget[item];

		GL::function().glTexSubImage2D(targetFace, level, 0, 0, width, height,
            mExternalFormat, mExternalType, data);
    }
}
