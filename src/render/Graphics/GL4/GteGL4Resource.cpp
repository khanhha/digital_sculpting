// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGL4Resource.h>
using namespace gte;

GL4Resource::GL4Resource(Resource const* gtResource)
    :
    GL4GraphicsObject(gtResource)
{
}

void* GL4Resource::MapForWrite(GLenum target, const GLintptr offset, const GLsizeiptr len)
{
    GL::function().glBindBuffer(target, mGLHandle);
	GLvoid* mapped = GL::function().glMapBufferRange(target, offset, len, GL_MAP_WRITE_BIT);
	GL::function().glBindBuffer(target, 0);
    return mapped;
}

void GL4Resource::Unmap(GLenum target)
{
	GL::function().glBindBuffer(target, mGLHandle);
	GL::function().glUnmapBuffer(target);
	GL::function().glBindBuffer(target, 0);
}

bool GL4Resource::PreparedForCopy(GLenum access) const
{
    // TODO: DX11 requires a staging resource when copying between CPU and
    // GPU.  Does OpenGL hide the double hop?

    // Verify existence of objects.
    if (mGLHandle == 0)
    {
        qCritical("Resource does not exist.");
        return false;
    }

    // Verify the copy type.  TODO: Change the Resource::CopyType names to
    // be COPY_CPU_TO_GPU and COPY_GPU_TO_CPU.
    Resource::CopyType copyType = GetResource()->GetCopyType();
    if (copyType == Resource::COPY_CPU_TO_STAGING)  // CPU -> GPU
    {
        if (access == GL_WRITE_ONLY)
        {
            return true;
        }
    }
    else if (copyType == Resource::COPY_STAGING_TO_CPU)  // GPU -> CPU
    {
        if (access == GL_READ_ONLY)
        {
            return true;
        }
    }
    else if (copyType == Resource::COPY_BIDIRECTIONAL)
    {
        if (access == GL_READ_WRITE)
        {
            return true;
        }
        if (access == GL_WRITE_ONLY)
        {
            return true;
        }
        if (access == GL_READ_ONLY)
        {
            return true;
        }
    }

    qCritical("Resource has incorrect copy type.");
    return false;
}
