// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGL4Buffer.h>
using namespace gte;

GL4Buffer::GL4Buffer(Buffer const* buffer, GLenum type)
    :
    GL4Resource(buffer),
    mType(type)
{
    GL::function().glGenBuffers(1, &mGLHandle);

    Resource::Usage usage = buffer->GetUsage();
    if (usage == Resource::IMMUTABLE)
    {
        mUsage = GL_STATIC_DRAW;
    }
    else if (usage == Resource::DYNAMIC_UPDATE)
    {
        mUsage = GL_DYNAMIC_DRAW;
    }
    else  // usage == Resource::SHADER_OUTPUT
    {
        // TODO: In GLSL, is it possible to write to a buffer other than a
        // vertex buffer?
        if (mType == GL_ARRAY_BUFFER)
        {
            mUsage = GL_STREAM_DRAW;
        }
        else if (mType == GL_SHADER_STORAGE_BUFFER)
        {
            mUsage = GL_DYNAMIC_DRAW;
        }
        else
        {
            qCritical("Can this buffer type be a shader output?");
            mUsage = GL_STATIC_DRAW;
        }
    }
}

GL4Buffer::~GL4Buffer()
{
	GL::function().glDeleteBuffers(1, &mGLHandle);
}

void GL4Buffer::Initialize()
{
	GL::function().glBindBuffer(mType, mGLHandle);

    // Access the buffer.
    auto buffer = GetBuffer();

    // Create and initialize a buffer.
	GL::function().glBufferData(mType, buffer->GetNumBytes(), buffer->GetData(), mUsage);

	GL::function().glBindBuffer(mType, 0);
}

bool GL4Buffer::Update()
{
    Buffer* buffer = GetBuffer();
    if (buffer->GetUsage() != Resource::DYNAMIC_UPDATE)
    {
		qCritical("Buffer usage is not DYNAMIC_UPDATE.");
        return false;
    }

    UINT numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from CPU memory to GPU memory.
		GL::function().glBindBuffer(mType, mGLHandle);
#if 0
		GL::function().glBufferSubData(mType, buffer->GetOffset(), numActiveBytes, buffer->GetData());
#else
		GL::function().glBufferData(mType, buffer->GetNumBytes(), buffer->GetData(), mUsage);
#endif
		GL::function().glBindBuffer(mType, 0);
    }
    else
    {
		qCritical("Buffer has zero active bytes.");
    }
    return true;
}

bool GL4Buffer::CopyCpuToGpu()
{
    if (!PreparedForCopy(GL_WRITE_ONLY))
    {
        return false;
    }

    Buffer* buffer = GetBuffer();
    UINT numActiveBytes = buffer->GetNumActiveBytes();
    if (numActiveBytes > 0)
    {
        // Copy from CPU memory to GPU memory.
		GL::function().glBindBuffer(mType, mGLHandle);
		GL::function().glBufferSubData(mType, buffer->GetOffset(), numActiveBytes, buffer->GetData());
		GL::function().glBindBuffer(mType, 0);
    }
    else
    {
		qWarning("Buffer has zero active bytes.");
    }
    return true;
}

bool GL4Buffer::CopyGpuToCpu()
{
#if 0
	if (!PreparedForCopy(GL_READ_ONLY))
	{
		return false;
	}

	Buffer* buffer = GetBuffer();
	UINT numActiveBytes = buffer->GetNumActiveBytes();
	if (numActiveBytes > 0)
	{
		// Copy from GPU memory to CPU memory.
		GL::function().glBindBuffer(mType, mGLHandle);
		GL::function().glGetBufferSubData(mType, buffer->GetOffset(), numActiveBytes, buffer->GetData());
		GL::function().glBindBuffer(mType, 0);
	}
	else
	{
		qWarning("Buffer has zero active bytes.");
	}
#endif
    return true;
}
