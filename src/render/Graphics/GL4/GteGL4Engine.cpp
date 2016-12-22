// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GteFontArialW400H18.h>
#include <Graphics/GL4/GteGL4Engine.h>
#include <Graphics/GL4/GteGL4BlendState.h>
#include <Graphics/GL4/GteGL4ConstantBuffer.h>
#include <Graphics/GL4/GteGL4DepthStencilState.h>
#include <Graphics/GL4/GteGL4DrawTarget.h>
#include <Graphics/GL4/GteGL4IndexBuffer.h>
#include <Graphics/GL4/GteGL4RasterizerState.h>
#include <Graphics/GL4/GteGL4SamplerState.h>
#include <Graphics/GL4/GteGL4Texture2.h>
#include <Graphics/GL4/GteGL4Texture3.h>
#include <Graphics/GL4/GteGL4TextureCube.h>
#include <Graphics/GL4/GteGL4VertexBuffer.h>
#include <Graphics/GL4/GteGLSLProgramFactory.h>
#include <Graphics/GL4/GteGLSLComputeProgram.h>
#include <Graphics/GL4/GteGLSLVisualProgram.h>

using namespace gte;

//----------------------------------------------------------------------------
// Public interface specific to GL4.
//----------------------------------------------------------------------------
GL4Engine::~GL4Engine()
{
    // The render state objects (and fonts) are destroyed first so that the
    // render state objects are removed from the bridges before they are
    // cleared later in the destructor.
#if 0
	DestroyDefaultFont();
	DestroyDefaultGlobalState();
#endif

    // Need to remove all the RawBuffer objects used to manage atomic
    // counter buffers.
    mAtomicCounterRawBuffers.clear();

    GraphicsObject::UnsubscribeForDestruction(mGOListener);
    mGOListener = nullptr;

    DrawTarget::UnsubscribeForDestruction(mDTListener);
    mDTListener = nullptr;

    if (mGOMap.HasElements())
    {
        if (mWarnOnNonemptyBridges)
        {
            qWarning("Bridge map is nonempty on destruction.");
        }

        std::vector<GEObject*> objects;
        mGOMap.GatherAll(objects);
        for (auto object : objects)
        {
            delete object;
        }
        mGOMap.RemoveAll();
    }

    if (mDTMap.HasElements())
    {
        if (mWarnOnNonemptyBridges)
        {
            qWarning("Draw target map nonempty on destruction.");
        }

        std::vector<GEDrawTarget*> targets;
        mDTMap.GatherAll(targets);
        for (auto target : targets)
        {
            delete target;
        }
        mDTMap.RemoveAll();
    }

    if (mILMap->HasElements())
    {
        if (mWarnOnNonemptyBridges)
        {
            qWarning("Input layout map nonempty on destruction.");
        }

        mILMap->UnbindAll();
    }
    mILMap = nullptr;

#if 0
	if (mHandle && mDevice && mImmediate)
	{
		wglDeleteContext(mImmediate);
		ReleaseDC(mHandle, mDevice);
	}

	if (mComputeWindowAtom)
	{
		DestroyWindow(mHandle);
		UnregisterClass(mComputeWindowClass.c_str(), 0);
	}
#endif
}

GL4Engine::GL4Engine()
{
	Initialize(true, false);
}

//----------------------------------------------------------------------------
// Private interface specific to GL4.
//----------------------------------------------------------------------------
void GL4Engine::Initialize(bool pixelFormatKnown, bool saveDriverInfo)
{
    // Initialization of GraphicsEngine members that depend on GL4.
    mILMap = std::make_unique<GL4InputLayoutManager>();

    mCreateGEObject =
    {
        nullptr, // GT_GRAPHICS_OBJECT (abstract base)
        nullptr, // GT_RESOURCE (abstract base)
        nullptr, // GT_BUFFER (abstract base)
        &GL4ConstantBuffer::Create,
        nullptr, // &DX11TextureBuffer::Create,
        &GL4VertexBuffer::Create,
        &GL4IndexBuffer::Create,
        nullptr/*&GL4StructuredBuffer::Create*/,
        nullptr, // TODO:  Implement TypedBuffer
        nullptr, // &DX11RawBuffer::Create,
        nullptr, // &DX11IndirectArgumentsBuffer::Create,
        nullptr, // GT_TEXTURE (abstract base)
        nullptr, // GT_TEXTURE_SINGLE (abstract base)
		nullptr/*&GL4Texture1::Create*/,
        &GL4Texture2::Create,
        &GL4TextureRT::Create,
        &GL4TextureDS::Create,
        &GL4Texture3::Create,
        nullptr, // GT_TEXTURE_ARRAY (abstract base)
        nullptr/*&GL4Texture1Array::Create*/,
        nullptr/*&GL4Texture2Array::Create*/,
        &GL4TextureCube::Create,
        nullptr/*&GL4TextureCubeArray::Create*/,
        nullptr, // GT_SHADER (abstract base)
        nullptr, // &DX11VertexShader::Create,
        nullptr, // &DX11GeometryShader::Create,
        nullptr, // &DX11PixelShader::Create,
        nullptr, // &DX11ComputeShader::Create,
        nullptr, // GT_DRAWING_STATE (abstract base)
        &GL4SamplerState::Create,
        &GL4BlendState::Create,
        &GL4DepthStencilState::Create,
        &GL4RasterizerState::Create
    };

	mCreateGEDrawTarget = &GL4DrawTarget::Create;

#if 0

	if (!mHandle)
	{
		qCritical("Invalid window handle.");
		return;
	}

	mDevice = GetDC(mHandle);
	if (!mDevice)
	{
		qCritical("Invalid device context.");
		mHandle = nullptr;
		return;
	}

	RECT rect;
	BOOL result = GetClientRect(mHandle, &rect); (void)result;
	mXSize = static_cast<unsigned int>(rect.right - rect.left);
	mYSize = static_cast<unsigned int>(rect.bottom - rect.top);

	if (!pixelFormatKnown)
	{
		// Select the format for the drawing surface.
		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(PIXELFORMATDESCRIPTOR));
		pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
		pfd.nVersion = 1;
		pfd.dwFlags =
			PFD_DRAW_TO_WINDOW |
			PFD_SUPPORT_OPENGL |
			PFD_GENERIC_ACCELERATED |
			PFD_DOUBLEBUFFER;

		// Create an R8G8B8A8 buffer.
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.cColorBits = 32;

		// Create a D24S8 depth-stencil buffer.
		pfd.cDepthBits = 24;
		pfd.cStencilBits = 8;

		// Set the pixel format for the rendering context.
		int pixelFormat = ChoosePixelFormat(mDevice, &pfd);
		if (pixelFormat == 0)
		{
			qCritical("ChoosePixelFormat failed.");
			ReleaseDC(mHandle, mDevice);
			mXSize = 0;
			mYSize = 0;
			mDevice = nullptr;
			mHandle = nullptr;
			return;
		}

		if (!SetPixelFormat(mDevice, pixelFormat, &pfd))
		{
			qCritical("SetPixelFormat failed.");
			ReleaseDC(mHandle, mDevice);
			mXSize = 0;
			mYSize = 0;
			mDevice = nullptr;
			mHandle = nullptr;
			return;
		}
	}

	// Create an OpenGL context.
	mImmediate = wglCreateContext(mDevice);
	if (!mImmediate)
	{
		qCritical("wglCreateContext failed.");
		ReleaseDC(mHandle, mDevice);
		mXSize = 0;
		mYSize = 0;
		mDevice = nullptr;
		mHandle = nullptr;
		return;
	}

	// Activate the context.
	if (!wglMakeCurrent(mDevice, mImmediate))
	{
		qCritical("wglMakeCurrent failed.");
		wglDeleteContext(mImmediate);
		ReleaseDC(mHandle, mDevice);
		mXSize = 0;
		mYSize = 0;
		mImmediate = nullptr;
		mDevice = nullptr;
		mHandle = nullptr;
		return;
	}

	if (saveDriverInfo)
	{
		InitializeOpenGL("OpenGLDriverInfo.txt");
	}
	else
	{
		InitializeOpenGL(nullptr);
	}
	InitializeWGL();

	SetViewport(0, 0, mXSize, mYSize);
	SetDepthRange(0.0f, 1.0f);
	CreateDefaultGlobalState();
	CreateDefaultFont();
#endif
	SetDepthRange(0.0f, 1.0f);
	CreateDefaultGlobalState();
}

void GL4Engine::CreateDefaultFont()
{
    GLSLProgramFactory factory;
    mDefaultFont = std::make_shared<FontArialW400H18>(factory, 256);
    SetDefaultFont();
}

void GL4Engine::DestroyDefaultFont()
{
    if (mDefaultFont)
    {
        mDefaultFont = nullptr;
        mActiveFont = nullptr;
    }
}

uint64_t GL4Engine::DrawPrimitive(VertexBuffer const* vbuffer, IndexBuffer const* ibuffer)
{
    unsigned int numActiveVertices = vbuffer->GetNumActiveElements();
    unsigned int vertexOffset = vbuffer->GetOffset();

    unsigned int numActiveIndices = ibuffer->GetNumActiveIndices();
    unsigned int indexSize = ibuffer->GetElementSize();
    GLenum indexType = (indexSize == 4 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT);

    GLenum topology = 0;
    IPType type = ibuffer->GetPrimitiveType();
    switch (type)
    {
    case IPType::IP_POLYPOINT:
        topology = GL_POINTS;
        break;
    case IPType::IP_POLYSEGMENT_DISJOINT:
        topology = GL_LINES;
        break;
    case IPType::IP_POLYSEGMENT_CONTIGUOUS:
        topology = GL_LINE_STRIP;
        break;
    case IPType::IP_TRIMESH:
        topology = GL_TRIANGLES;
        break;
    case IPType::IP_TRISTRIP:
        topology = GL_TRIANGLE_STRIP;
        break;
    default:
        qCritical("Unknown primitive topology = "/* + std::to_string(type)*/);
        return 0;
    }

    unsigned int offset = ibuffer->GetOffset();
    if (ibuffer->IsIndexed())
    {
        void const* data = (char*)0 + indexSize * offset;
        GL::function().glDrawRangeElements(topology, 0, numActiveVertices - 1,
            static_cast<GLsizei>(numActiveIndices), indexType, data);
    }
    else
    {
        // From the OpenGL documentation on gl_VertexID vertex shader variable:
        // "gl_VertexID is a vertex language input variable that holds an integer index
        // for the vertex. The index is impliclty generated by glDrawArrays and other
        // commands that do not reference the content of the GL_ELEMENT_ARRAY_BUFFER,
        // or explicitly generated from the content of the GL_ELEMENT_ARRAY_BUFFER
        // by commands such as glDrawElements."
		GL::function().glDrawArrays(topology, static_cast<GLint>(vertexOffset),
            static_cast<GLint>(numActiveVertices));
    }
    return 0;
}

bool GL4Engine::EnableShaders(std::shared_ptr<VisualEffect> const& effect, GLuint program)
{
    VertexShader* vshader = effect->GetVertexShader().get();
    if (!vshader)
    {
        qCritical("Effect does not have a vertex shader.");
        return false;
    }

    PixelShader* pshader = effect->GetPixelShader().get();
    if (!pshader)
    {
        qCritical("Effect does not have a pixel shader.");
        return false;
    }

    GeometryShader* gshader = effect->GetGeometryShader().get();

    // Enable the shader resources.
    Enable(vshader, program);
    Enable(pshader, program);
    if (gshader)
    {
        Enable(gshader, program);
    }

    return true;
}

void GL4Engine::DisableShaders(std::shared_ptr<VisualEffect> const& effect, GLuint program)
{
    VertexShader* vshader = effect->GetVertexShader().get();
    PixelShader* pshader = effect->GetPixelShader().get();
    GeometryShader* gshader = effect->GetGeometryShader().get();

    if (gshader)
    {
        Disable(gshader, program);
    }
    Disable(pshader, program);
    Disable(vshader, program);
}

void GL4Engine::Enable(Shader const* shader, GLuint program)
{
    EnableCBuffers(shader, program);
    EnableTBuffers(shader, program);
#ifdef GL_4
	EnableSBuffers(shader, program);
#endif
    EnableRBuffers(shader, program);
    EnableTextures(shader, program);
    EnableTextureArrays(shader, program);
    EnableSamplers(shader, program);
}

void GL4Engine::Disable(Shader const* shader, GLuint program)
{
    DisableSamplers(shader, program);
    DisableTextureArrays(shader, program);
    DisableTextures(shader, program);
    DisableRBuffers(shader, program);
#ifdef GL_4
    DisableSBuffers(shader, program);
#endif
    DisableTBuffers(shader, program);
    DisableCBuffers(shader, program);
}

void GL4Engine::EnableCBuffers(Shader const* shader, GLuint program)
{
    int const index = ConstantBuffer::shaderDataLookup;
    for (auto const& cb : shader->GetData(index))
    {
        if (cb.object)
        {
            auto gl4CB = static_cast<GL4ConstantBuffer*>(Bind(cb.object));
            if (gl4CB)
            {
                auto const blockIndex = cb.bindPoint;
                if (GL_INVALID_INDEX != blockIndex)
                {
                    auto const unit = mUniformUnitMap.AcquireUnit(program, blockIndex);
					GL::function().glUniformBlockBinding(program, blockIndex, unit);
                    gl4CB->AttachToUnit(unit);
                }
            }
            else
            {
                qCritical("Failed to bind constant buffer.");
            }
        }
        else
        {
            qCritical(/*cb.name + */" is null constant buffer.");
        }
    }
}

void GL4Engine::DisableCBuffers(Shader const* shader, GLuint program)
{
    int const index = ConstantBuffer::shaderDataLookup;
    for (auto const& cb : shader->GetData(index))
    {
        auto const blockIndex = cb.bindPoint;
        if (GL_INVALID_INDEX != blockIndex)
        {
            auto const unit = mUniformUnitMap.GetUnit(program, blockIndex);
			GL::function().glBindBufferBase(GL_UNIFORM_BUFFER, unit, 0);
            mUniformUnitMap.ReleaseUnit(unit);
        }
    }
}

void GL4Engine::EnableTBuffers(Shader const*, GLuint)
{
    // TODO
}

void GL4Engine::DisableTBuffers(Shader const*, GLuint)
{
    // TODO
}

#if GL_4
void GL4Engine::EnableSBuffers(Shader const* shader, GLuint program)
{
    // Configure atomic counter buffer objects used by the shader.
    auto const& atomicCounters = shader->GetData(Shader::AtomicCounterShaderDataLookup);
    auto const& atomicCounterBuffers = shader->GetData(Shader::AtomicCounterBufferShaderDataLookup);
    for (unsigned acbIndex = 0; acbIndex < atomicCounterBuffers.size(); ++acbIndex)
    {
        auto const& acb = atomicCounterBuffers[acbIndex];

        // Allocate a new raw buffer?
        if (acbIndex >= mAtomicCounterRawBuffers.size())
        {
            mAtomicCounterRawBuffers.push_back(nullptr);
        }

        // Look at the current raw buffer defined at this index.
        // Could be nullptr if a new location was just inserted.
        auto& rawBuffer = mAtomicCounterRawBuffers[acbIndex];

        // If the raw buffer is not large enough, then unbind old one and
        // ready to create new one.
        if (rawBuffer && (acb.numBytes > static_cast<int>(rawBuffer->GetNumBytes())))
        {
            Unbind(rawBuffer.get());
            rawBuffer = nullptr;
        }

        // Find the currently mapped GL4AtomicCounterBuffer.
        GL4AtomicCounterBuffer* gl4ACB = nullptr;
        if (rawBuffer)
        {
            gl4ACB = static_cast<GL4AtomicCounterBuffer*>(Get(rawBuffer));
        }

        // Create a new buffer?
        else
        {
            // By definition, RawBuffer contains 4-byte elements.  Do not need
            // CPU side storage but must be able to copy values between buffers.
            rawBuffer = std::make_shared<RawBuffer>((acb.numBytes + 3) / 4, false);
            rawBuffer->SetUsage(Resource::DYNAMIC_UPDATE);

            // Manual Bind operation since this is a special mapping from
            // RawBuffer to GL4AtomicCounterBuffer.
            gl4ACB = static_cast<GL4AtomicCounterBuffer*>(
                GL4AtomicCounterBuffer::Create(mGEObjectCreator, rawBuffer.get()));
            mGOMap.Insert(rawBuffer.get(), gl4ACB);
        }

        // TODO:
        // ShaderStorage blocks have a glShaderStorageBlockBinding() call
        // Uniform blocks have glUniforBlockBinding() call
        // Is there something equivalent for atomic counters buffers?

        // Bind this atomic counter buffer
        gl4ACB->AttachToUnit(acb.bindPoint);
    }

    int const indexSB = StructuredBuffer::shaderDataLookup;
    for (auto const& sb : shader->GetData(indexSB))
    {
        if (sb.object)
        {
            auto gl4SB = static_cast<GL4StructuredBuffer*>(Bind(sb.object));
            if (gl4SB)
            {
                auto const blockIndex = sb.bindPoint;
                if (GL_INVALID_INDEX != blockIndex)
                {
                    auto const unit = mShaderStorageUnitMap.AcquireUnit(program, blockIndex);
                    glShaderStorageBlockBinding(program, blockIndex, unit);

                    // Do not use glBindBufferBase here.  Use AttachToUnit
                    // method in GL4StructuredBuffer.
                    gl4SB->AttachToUnit(unit);

                    // The sb.isGpuWritable flag is used to indicate whether
                    // or not there is atomic counter associated with this
                    // structured buffer.
                    if (sb.isGpuWritable)
                    {
                        // Does the structured buffer counter need to be reset?
                        gl4SB->SetNumActiveElements();

                        // This structured buffer has index to associated
                        // atomic counter table entry.
                        auto const acIndex = sb.extra;

                        // Where does the associated counter exist in the shader?
                        auto const acbIndex = atomicCounters[acIndex].bindPoint;
                        auto const acbOffset = atomicCounters[acIndex].extra;

                        // Retrieve the GL4 atomic counter buffer object.
                        auto gl4ACB = static_cast<GL4AtomicCounterBuffer*>(Get(mAtomicCounterRawBuffers[acbIndex]));

                        // Copy the counter value from the structured buffer object
                        // to the appropriate place in the atomic counter buffer.
                        gl4SB->CopyCounterValueToBuffer(gl4ACB, acbOffset);
                    }
                }
            }
            else
            {
                qCritical("Failed to bind structured buffer.");
            }
        }
        else
        {
            qCritical(sb.name + " is null structured buffer.");
        }
    }
}
void GL4Engine::DisableSBuffers(Shader const* shader, GLuint program)
{
    // Unbind any atomic counter buffers.
    auto const& atomicCounters = shader->GetData(Shader::AtomicCounterShaderDataLookup);
    auto const& atomicCounterBuffers = shader->GetData(Shader::AtomicCounterBufferShaderDataLookup);
    for (unsigned acbIndex = 0; acbIndex < atomicCounterBuffers.size(); ++acbIndex)
    {
        auto const& acb = atomicCounterBuffers[acbIndex];
        glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, acb.bindPoint, 0);
    }

    int const index = StructuredBuffer::shaderDataLookup;
    for (auto const& sb : shader->GetData(index))
    {
        if (sb.object)
        {
            auto gl4SB = static_cast<GL4StructuredBuffer*>(Get(sb.object));

            if (gl4SB)
            {
                auto const blockIndex = sb.bindPoint;
                if (GL_INVALID_INDEX != blockIndex)
                {
                    auto const unit = mShaderStorageUnitMap.GetUnit(program, blockIndex);
                    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, unit, 0);
                    mShaderStorageUnitMap.ReleaseUnit(unit);

                    // The sb.isGpuWritable flag is used to indicate whether
                    // or not there is atomic counter associated with this
                    // structured buffer.
                    if (sb.isGpuWritable)
                    {
                        // This structured buffer has index to associated
                        // atomic counter table entry.
                        auto const acIndex = sb.extra;

                        // Where does the associated counter exist in the shader?
                        auto const acbIndex = atomicCounters[acIndex].bindPoint;
                        auto const acbOffset = atomicCounters[acIndex].extra;

                        // Retrieve the GL4 atomic counter buffer object.
                        auto gl4ACB = static_cast<GL4AtomicCounterBuffer*>(Get(mAtomicCounterRawBuffers[acbIndex]));

                        // Copy the counter value from the appropriate place
                        // in the atomic counter buffer to the structured buffer
                        // object.
                        gl4SB->CopyCounterValueFromBuffer(gl4ACB, acbOffset);
                    }
                }
            }
        }
    }
}
#endif
void GL4Engine::EnableRBuffers(Shader const*, GLuint)
{
    // TODO
}

void GL4Engine::DisableRBuffers(Shader const*, GLuint)
{
    // TODO
}

void GL4Engine::EnableTextures(Shader const* shader, GLuint program)
{
    int const index = TextureSingle::shaderDataLookup;
    for (auto const& ts : shader->GetData(index))
    {
        if (ts.object)
        {
            auto gl4Tex = static_cast<GL4TextureSingle*>(Bind(ts.object));
            if (gl4Tex)
            {
                auto const location = ts.bindPoint;

                // Access to the GL4 resource
                auto const glHandle = gl4Tex->GetGLHandle();
                auto const glTarget = gl4Tex->GetTarget();

                // By convension, ts.isGpuWritable is true for "image*" and
                // false for "sampler*"
                if (ts.isGpuWritable)
                {
                    auto const unit = mTextureImageUnitMap.AcquireUnit(program, location);
                    GL::function().glProgramUniform1i(program, location, unit);

                    // For "image*" objects in the shader, use "readonly" or
                    // "writeonly" attributes in the layout to control R/W/RW
                    // access using shader compiler and then connect as
                    // GL_READ_WRITE here.  Always bind level=0 and all layers.
                    auto texture = gl4Tex->GetTexture();
                    auto const gl4InternalFormat = gl4Tex->GetInternalFormat(texture->GetFormat());
					GL::function().glBindImageTexture(unit, glHandle, 0, GL_TRUE, 0, GL_READ_WRITE, gl4InternalFormat);
                }
                else
                {
                    auto const unit = mTextureSamplerUnitMap.AcquireUnit(program, location);
					GL::function().glProgramUniform1i(program, location, unit);
					GL::function().glActiveTexture(GL_TEXTURE0 + unit);
					GL::function().glBindTexture(glTarget, glHandle);
                }
            }
            else
            {
                qCritical("Failed to bind texture.");
            }
        }
        else
        {
            qCritical(/*ts.name + */" is null texture.");
        }
    }
}

void GL4Engine::DisableTextures(Shader const* shader, GLuint program)
{
    int const index = TextureSingle::shaderDataLookup;
    for (auto const& ts : shader->GetData(index))
    {
        if (ts.object)
        {
            auto gl4Tex = static_cast<GL4TextureSingle*>(Get(ts.object));
            if (gl4Tex)
            {
                auto const location = ts.bindPoint;

                // Access to the GL4 resource
                auto const glTarget = gl4Tex->GetTarget();

                // By convension, ts.isGpuWritable is true for "image*"
                // and false for "sampler*"
                if (ts.isGpuWritable)
                {
                    auto const unit = mTextureImageUnitMap.GetUnit(program, location);

                    // For "image*" objects in the shader, use "readonly" or
                    // "writeonly" attributes in the layout to control R/W/RW
                    // access using shader compiler and then connect as
                    // GL_READ_WRITE here.  Always bind level=0 and all layers.
                    // TODO: Decide if unbinding the texture from the image unit
                    // is necessary.
                    // glBindImageTexture(unit, 0, 0, 0, 0, 0, 0);
                    mTextureImageUnitMap.ReleaseUnit(unit);
                }
                else
                {
                    auto const unit = mTextureSamplerUnitMap.GetUnit(program, location);
					GL::function().glActiveTexture(GL_TEXTURE0 + unit);
					GL::function().glBindTexture(glTarget, 0);
                    mTextureSamplerUnitMap.ReleaseUnit(unit);
                }
            }
            else
            {
                qCritical("Failed to get texture.");
            }
        }
        else
        {
            qCritical(/*ts.name + */" is null texture.");
        }
    }
}

void GL4Engine::EnableTextureArrays(Shader const* shader, GLuint program)
{
    int const index = TextureArray::shaderDataLookup;
    for (auto const& ta : shader->GetData(index))
    {
        if (ta.object)
        {
            auto gl4Tex = static_cast<GL4TextureArray*>(Bind(ta.object));
            if (gl4Tex)
            {
                auto const location = ta.bindPoint;

                // Access to the GL4 resource
                auto const glHandle = gl4Tex->GetGLHandle();
                auto const glTarget = gl4Tex->GetTarget();

                // By convension, ta.isGpuWritable is true for "image*"
                // and false for "sampler*"
                if (ta.isGpuWritable)
                {
                    auto const unit = mTextureImageUnitMap.AcquireUnit(program, location);
					GL::function().glProgramUniform1i(program, location, unit);

                    // For "image*" objects in the shader, use "readonly" or
                    // "writeonly" attributes in the layout to control R/W/RW
                    // access using shader compiler and then connect as
                    // GL_READ_WRITE here.  Always bind level=0 and all layers.
                    auto texture = gl4Tex->GetTexture();
                    auto const gl4InternalFormat = gl4Tex->GetInternalFormat(texture->GetFormat());
					GL::function().glBindImageTexture(unit, glHandle, 0, GL_TRUE, 0, GL_READ_WRITE, gl4InternalFormat);
                }
                else
                {
                    auto const unit = mTextureSamplerUnitMap.AcquireUnit(program, location);
					GL::function().glProgramUniform1i(program, location, unit);
					GL::function().glActiveTexture(GL_TEXTURE0 + unit);
					GL::function().glBindTexture(glTarget, glHandle);
                }
            }
            else
            {
                qCritical("Failed to bind texture array.");
            }
        }
        else
        {
            qCritical(/*ta.name + */" is null texture array.");
        }
    }
}

void GL4Engine::DisableTextureArrays(Shader const* shader, GLuint program)
{
    int const index = TextureArray::shaderDataLookup;
    for (auto const& ta : shader->GetData(index))
    {
        if (ta.object)
        {
            auto gl4Tex = static_cast<GL4TextureArray*>(Get(ta.object));
            if (gl4Tex)
            {
                auto const location = ta.bindPoint;

                // Access to the GL4 resource
                auto const glTarget = gl4Tex->GetTarget();

                // By convension, ta.isGpuWritable is true for "image*"
                // and false for "sampler*"
                if (ta.isGpuWritable)
                {
                    auto const unit = mTextureImageUnitMap.GetUnit(program, location);

                    // For "image*" objects in the shader, use "readonly" or
                    // "writeonly" attributes in the layout to control R/W/RW
                    // access using shader compiler and then connect as
                    // GL_READ_WRITE here.  Always bind level=0 and all layers.
                    // TODO: Decide if unbinding the texture from the image unit
                    // is necessary.
                    // glBindImageTexture(unit, 0, 0, 0, 0, 0, 0);
                    mTextureImageUnitMap.ReleaseUnit(unit);
                }
                else
                {
                    auto const unit = mTextureSamplerUnitMap.GetUnit(program, location);
					GL::function().glActiveTexture(GL_TEXTURE0 + unit);
					GL::function().glBindTexture(glTarget, 0);
                    mTextureSamplerUnitMap.ReleaseUnit(unit);
                }
            }
            else
            {
                qCritical("Failed to get texture array.");
            }
        }
        else
        {
            qCritical(/*ta.name + */" is null texture array.");
        }
    }
}

void GL4Engine::EnableSamplers(Shader const* shader, GLuint program)
{
    int const index = SamplerState::shaderDataLookup;
    for (auto const& ts : shader->GetData(index))
    {
        if (ts.object)
        {
            auto gl4Sampler = static_cast<GL4SamplerState*>(Bind(ts.object));
            if (gl4Sampler)
            {
                auto const location = ts.bindPoint;
                auto const unit = mTextureSamplerUnitMap.AcquireUnit(program, location);
				GL::function().glBindSampler(unit, gl4Sampler->GetGLHandle());
            }
            else
            {
                qCritical("Failed to bind sampler.");
            }
        }
        else
        {
            qCritical(/*ts.name + */" is null sampler.");
        }
    }
}

void GL4Engine::DisableSamplers(Shader const* shader, GLuint program)
{
    int const index = SamplerState::shaderDataLookup;
    for (auto const& ts : shader->GetData(index))
    {
        if (ts.object)
        {
            auto gl4Sampler = static_cast<GL4SamplerState*>(Get(ts.object));

            if (gl4Sampler)
            {
                auto const location = ts.bindPoint;
                auto const unit = mTextureSamplerUnitMap.GetUnit(program, location);
				GL::function().glBindSampler(unit, 0);
                mTextureSamplerUnitMap.ReleaseUnit(unit);
            }
            else
            {
                qCritical("Failed to get sampler.");
            }
        }
        else
        {
            qCritical(/*ts.name + */" is null sampler.");
        }
    }
}

GL4Engine::ProgramIndexUnitMap::~ProgramIndexUnitMap()
{
}

GL4Engine::ProgramIndexUnitMap::ProgramIndexUnitMap()
{
}

int GL4Engine::ProgramIndexUnitMap::AcquireUnit(GLint program, GLint index)
{
    int availUnit = -1;
    for (int unit = 0; unit < static_cast<int>(mLinkMap.size()); ++unit)
    {
        auto& item = mLinkMap[unit];

        // Increment link count if already assigned and in use?
        if (program == item.program && index == item.index)
        {
            ++item.linkCount;
            return unit;
        }

        // Found a unit that was previously used but is now avaialble.
        if (0 == item.linkCount)
        {
            if (-1 == availUnit)
            {
                availUnit = unit;
            }
        }
    }

    // New unit number not previously used?
    if (-1 == availUnit)
    {
        // TODO: Consider querying the max number of units
        // and check that this number is not exceeded.
        availUnit = static_cast<int>(mLinkMap.size());
        mLinkMap.push_back({ 0, 0, 0 });
    }

    auto& item = mLinkMap[availUnit];
    item.linkCount = 1;
    item.program = program;
    item.index = index;
    return availUnit;
}

int GL4Engine::ProgramIndexUnitMap::GetUnit(GLint program, GLint index) const
{
    for (int unit = 0; unit < static_cast<int>(mLinkMap.size()); ++unit)
    {
        auto& item = mLinkMap[unit];
        if (program == item.program && index == item.index)
        {
            return unit;
        }
    }
    return -1;
}

void GL4Engine::ProgramIndexUnitMap::ReleaseUnit(unsigned index)
{
    if (index < mLinkMap.size())
    {
        auto& item = mLinkMap[index];
        if (item.linkCount > 0)
        {
            --item.linkCount;
        }
    }
}

unsigned GL4Engine::ProgramIndexUnitMap::GetUnitLinkCount(unsigned unit) const
{
    if (unit < mLinkMap.size())
    {
        return mLinkMap[unit].linkCount;
    }
    return 0;
}

bool GL4Engine::ProgramIndexUnitMap::GetUnitProgramIndex(unsigned unit, GLint &program, GLint &index) const
{
    if (unit < mLinkMap.size())
    {
        auto& item = mLinkMap[index];
        if (item.linkCount > 0)
        {
            program = item.program;
            index = item.index;
            return true;
        }
    }
    return false;
}

//----------------------------------------------------------------------------
// Public overrides from GraphicsEngine
//----------------------------------------------------------------------------
void GL4Engine::SetViewport(int x, int y, int w, int h)
{
	GL::function().glViewport(x, y, w, h);
}

void GL4Engine::GetViewport(int& x, int& y, int& w, int& h) const
{
    int param[4];
	GL::function().glGetIntegerv(GL_VIEWPORT, param);
    x = param[0];
    y = param[1];
    w = param[2];
    h = param[3];
}

void GL4Engine::SetDepthRange(float zmin, float zmax)
{
	GL::function().glDepthRangef(static_cast<GLfloat>(zmin), static_cast<GLfloat>(zmax));
}

void GL4Engine::GetDepthRange(float& zmin, float& zmax) const
{
    GLfloat param[2];
	GL::function().glGetFloatv(GL_DEPTH_RANGE, param);
    zmin = (param[0]);
    zmax = (param[1]);
}

bool GL4Engine::Resize(unsigned int w, unsigned int h)
{
    mXSize = w;
    mYSize = h;
    int param[4];
	GL::function().glGetIntegerv(GL_VIEWPORT, param);
	GL::function().glViewport(param[0], param[1], static_cast<GLint>(w), static_cast<GLint>(h));
    return true;
}

void GL4Engine::ClearColorBuffer()
{
	GL::function().glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
	GL::function().glClear(GL_COLOR_BUFFER_BIT);
}

void GL4Engine::ClearDepthBuffer()
{
	GL::function().glClearDepthf(mClearDepth);
	GL::function().glClear(GL_DEPTH_BUFFER_BIT);
}

void GL4Engine::ClearStencilBuffer()
{
	GL::function().glClearStencil(static_cast<GLint>(mClearStencil));
	GL::function().glClear(GL_STENCIL_BUFFER_BIT);
}

void GL4Engine::ClearBuffers()
{
	GL::function().glClearColor(mClearColor[0], mClearColor[1], mClearColor[2], mClearColor[3]);
	GL::function().glClearDepthf(mClearDepth);
	GL::function().glClearStencil(static_cast<GLint>(mClearStencil));
	GL::function().glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

#if 0
void GL4Engine::DisplayColorBuffer(unsigned int syncInterval)
{
	wglSwapIntervalEXT(syncInterval > 0 ? 1 : 0);
	SwapBuffers(mDevice);
}
#endif

void GL4Engine::SetBlendState(std::shared_ptr<BlendState> const& state)
{
    if (state)
    {
        if (state != mActiveBlendState)
        {
            GL4BlendState* gl4State = static_cast<GL4BlendState*>(Bind(state));
            if (gl4State)
            {
                gl4State->Enable(mImmediate);
                mActiveBlendState = state;
            }
            else
            {
                qCritical("Failed to bind blend state.");
            }
        }
    }
    else
    {
        qCritical("Input state is null.");
    }
}

void GL4Engine::SetDepthStencilState(std::shared_ptr<DepthStencilState> const& state)
{
    if (state)
    {
        if (state != mActiveDepthStencilState)
        {
            GL4DepthStencilState* gl4State = static_cast<GL4DepthStencilState*>(Bind(state));
            if (gl4State)
            {
                gl4State->Enable(mImmediate);
                mActiveDepthStencilState = state;
            }
            else
            {
                qCritical("Failed to bind depth-stencil state.");
            }
        }
    }
    else
    {
        qCritical("Input state is null.");
    }
}

void GL4Engine::SetRasterizerState(std::shared_ptr<RasterizerState> const& state)
{
    if (state)
    {
        if (state != mActiveRasterizerState)
        {
            GL4RasterizerState* gl4State = static_cast<GL4RasterizerState*>(Bind(state));
            if (gl4State)
            {
                gl4State->Enable(mImmediate);
                mActiveRasterizerState = state;
            }
            else
            {
                qCritical("Failed to bind rasterizer state.");
            }
        }
    }
    else
    {
        qCritical("Input state is null.");
    }
}

void GL4Engine::Enable(std::shared_ptr<DrawTarget> const& target)
{
	gte::GL4DrawTarget *gl4Target = static_cast<gte::GL4DrawTarget*>(Bind(target));
    gl4Target->Enable();
}

void GL4Engine::Disable(std::shared_ptr<DrawTarget> const& target)
{
	gte::GL4DrawTarget *gl4Target = static_cast<gte::GL4DrawTarget*>(Get(target));
    if (gl4Target)
    {
        gl4Target->Disable();
    }
}

bool GL4Engine::Update(std::shared_ptr<Buffer> const& buffer)
{
    if (!buffer->GetData())
    {
        qWarning("Buffer does not have system memory, creating it.");
        buffer->CreateStorage();
    }

    auto glBuffer = static_cast<GL4Buffer*>(Bind(buffer));
    return glBuffer->Update();
}

bool GL4Engine::Update(std::shared_ptr<TextureSingle> const& texture)
{
    if (!texture->GetData())
    {
        qWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL4TextureSingle*>(Bind(texture));
    return glTexture->Update();
}

bool GL4Engine::Update(std::shared_ptr<TextureSingle> const& texture, unsigned int level)
{
    if (!texture->GetData())
    {
        qWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL4TextureSingle*>(Bind(texture));
    return glTexture->Update(level);
}

bool GL4Engine::Update(std::shared_ptr<TextureArray> const& textureArray)
{
    if (!textureArray->GetData())
    {
        qWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL4TextureArray*>(Bind(textureArray));
    return glTextureArray->Update();
}

bool GL4Engine::Update(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level)
{
    if (!textureArray->GetData())
    {
        qWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL4TextureArray*>(Bind(textureArray));
    return glTextureArray->Update(item, level);
}

bool GL4Engine::CopyCpuToGpu(std::shared_ptr<Buffer> const& buffer)
{
    if (!buffer->GetData())
    {
        qWarning("Buffer does not have system memory, creating it.");
        buffer->CreateStorage();
    }

    auto glBuffer = static_cast<GL4Buffer*>(Bind(buffer));
    return glBuffer->CopyCpuToGpu();
}

bool GL4Engine::CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture)
{
    if (!texture->GetData())
    {
        qWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL4TextureSingle*>(Bind(texture));
    return glTexture->CopyCpuToGpu();
}

bool GL4Engine::CopyCpuToGpu(std::shared_ptr<TextureSingle> const& texture, unsigned int level)
{
    if (!texture->GetData())
    {
        qWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL4TextureSingle*>(Bind(texture));
    return glTexture->CopyCpuToGpu(level);
}

bool GL4Engine::CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray)
{
    if (!textureArray->GetData())
    {
        qWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL4TextureArray*>(Bind(textureArray));
    return glTextureArray->CopyCpuToGpu();
}

bool GL4Engine::CopyCpuToGpu(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level)
{
    if (!textureArray->GetData())
    {
        qWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL4TextureArray*>(Bind(textureArray));
    return glTextureArray->CopyCpuToGpu(item, level);
}

bool GL4Engine::CopyGpuToCpu(std::shared_ptr<Buffer> const& buffer)
{
    if (!buffer->GetData())
    {
        qWarning("Buffer does not have system memory, creating it.");
        buffer->CreateStorage();
    }

    auto glBuffer = static_cast<GL4Buffer*>(Bind(buffer));
    return glBuffer->CopyGpuToCpu();
}

bool GL4Engine::CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture)
{
    if (!texture->GetData())
    {
        qWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL4TextureSingle*>(Bind(texture));
    return glTexture->CopyGpuToCpu();
}

bool GL4Engine::CopyGpuToCpu(std::shared_ptr<TextureSingle> const& texture, unsigned int level)
{
    if (!texture->GetData())
    {
        qWarning("Texture does not have system memory, creating it.");
        texture->CreateStorage();
    }

    auto glTexture = static_cast<GL4TextureSingle*>(Bind(texture));
    return glTexture->CopyGpuToCpu(level);
}

bool GL4Engine::CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray)
{
    if (!textureArray->GetData())
    {
        qWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL4TextureArray*>(Bind(textureArray));
    return glTextureArray->CopyGpuToCpu();
}

bool GL4Engine::CopyGpuToCpu(std::shared_ptr<TextureArray> const& textureArray, unsigned int item, unsigned int level)
{
    if (!textureArray->GetData())
    {
        qWarning("Texture array does not have system memory, creating it.");
        textureArray->CreateStorage();
    }

    auto glTextureArray = static_cast<GL4TextureArray*>(Bind(textureArray));
    return glTextureArray->CopyGpuToCpu(item, level);
}

#ifdef GL_4
bool GL4Engine::GetNumActiveElements(std::shared_ptr<StructuredBuffer> const& buffer)
{
    auto gl4Object = Get(buffer);
    if (gl4Object)
    {
        auto gl4SBuffer = static_cast<GL4StructuredBuffer*>(gl4Object);
        return gl4SBuffer->GetNumActiveElements();
    }
    return false;
}

void GL4Engine::Execute(std::shared_ptr<ComputeProgram> const& program,
    unsigned int numXGroups, unsigned int numYGroups, unsigned int numZGroups)
{
    auto glslProgram = std::dynamic_pointer_cast<GLSLComputeProgram>(program);
    if (glslProgram && numXGroups > 0 && numYGroups > 0 && numZGroups > 0)
    {
        auto cshader = glslProgram->GetCShader();
        auto programHandle = glslProgram->GetProgramHandle();
        if (cshader && programHandle > 0)
        {
            GL::function().glUseProgram(programHandle);
            Enable(cshader.get(), programHandle);
			GL::function().glDispatchCompute(numXGroups, numYGroups, numZGroups);
            Disable(cshader.get(), programHandle);
			GL::function().glUseProgram(0);
        }
    }
    else
    {
        qCritical("Invalid input parameter.");
    }
}
#endif
void GL4Engine::WaitForFinish()
{
    // TODO.  Determine whether OpenGL can wait for a compute program to finish.
}

//----------------------------------------------------------------------------
// Public overrides from GraphicsEngine
//----------------------------------------------------------------------------
uint64_t GL4Engine::DrawPrimitive(std::shared_ptr<VertexBuffer> const& vbuffer,
    std::shared_ptr<IndexBuffer> const& ibuffer, std::shared_ptr<VisualEffect> const& effect)
{
    GLSLVisualProgram* gl4program = dynamic_cast<GLSLVisualProgram*>(effect->GetProgram().get());
    if (!gl4program)
    {
        qCritical("HLSL effect passed to GLSL engine.");
        return 0;
    }

    uint64_t numPixelsDrawn = 0;
    auto programHandle = gl4program->GetProgramHandle();

	GL::function().glUseProgram(programHandle);

    if (EnableShaders(effect, programHandle))
    {
        // Enable the vertex buffer and input layout.
        GL4VertexBuffer* gl4VBuffer = nullptr;
        GL4InputLayout* gl4Layout = nullptr;
        if (vbuffer->IsFormatted())
        {
            gl4VBuffer = static_cast<GL4VertexBuffer*>(Bind(vbuffer));
            GL4InputLayoutManager* manager = static_cast<GL4InputLayoutManager*>(mILMap.get());
            gl4Layout = manager->Bind(programHandle, gl4VBuffer->GetGLHandle(), vbuffer.get());
            gl4Layout->Enable();
        }

        // Enable the index buffer.
        GL4IndexBuffer* gl4IBuffer = nullptr;
        if (ibuffer->IsIndexed())
        {
            gl4IBuffer = static_cast<GL4IndexBuffer*>(Bind(ibuffer));
            gl4IBuffer->Enable();
        }

        numPixelsDrawn = DrawPrimitive(vbuffer.get(), ibuffer.get());

        // Disable the vertex buffer and input layout.
        if (vbuffer->IsFormatted())
        {
            gl4Layout->Disable();
        }

        // Disable the index buffer.
        if (gl4IBuffer)
        {
            gl4IBuffer->Disable();
        }

        DisableShaders(effect, programHandle);
    }

	GL::function().glUseProgram(0);

    return numPixelsDrawn;
}

bool GL4Engine::Enable(std::shared_ptr<VisualEffect> const& effect)
{
	GLSLVisualProgram* gl4program = dynamic_cast<GLSLVisualProgram*>(effect->GetProgram().get());
	if (!gl4program)
	{
		qCritical("HLSL effect passed to GLSL engine.");
		return 0;
	}

	uint64_t numPixelsDrawn = 0;
	auto programHandle = gl4program->GetProgramHandle();

	GL::function().glUseProgram(programHandle);

	return EnableShaders(effect, programHandle);
}

void GL4Engine::Disable(std::shared_ptr<VisualEffect> const& effect)
{
	GLSLVisualProgram* gl4program = dynamic_cast<GLSLVisualProgram*>(effect->GetProgram().get());
	if (!gl4program){
		qCritical("HLSL effect passed to GLSL engine.");
	}
	auto programHandle = gl4program->GetProgramHandle();
	DisableShaders(effect, programHandle);
}

/*effect must be enabled before*/
void GL4Engine::DrawVertexBuffer(std::shared_ptr<VisualEffect> const& effect, std::shared_ptr<VertexBuffer> const& vbuffer, GLenum topology /*GL_TRIANGLES*/)
{
	GLSLVisualProgram* gl4program = dynamic_cast<GLSLVisualProgram*>(effect->GetProgram().get());
	if(gl4program){
		auto programHandle = gl4program->GetProgramHandle();

		// Enable the vertex buffer and input layout.
		GL4VertexBuffer* gl4VBuffer = nullptr;
		GL4InputLayout* gl4Layout = nullptr;
		if (vbuffer->IsFormatted())
		{
			gl4VBuffer = static_cast<GL4VertexBuffer*>(Bind(vbuffer));
			GL4InputLayoutManager* manager = static_cast<GL4InputLayoutManager*>(mILMap.get());
			gl4Layout = manager->Bind(programHandle, gl4VBuffer->GetGLHandle(), vbuffer.get());
			gl4Layout->Enable();
		}

		GL::function().glDrawArrays(
			topology, static_cast<GLint>(vbuffer->GetOffset()), static_cast<GLint>(vbuffer->GetNumElements()));
	}
}

BufferUpdater GL4Engine::GetBufferUpdater()
{
	return ([this](std::shared_ptr<Buffer> const& buffer){ this->Update(buffer); });
}

uint64_t GL4Engine::DrawBuffer(std::shared_ptr<VertexBuffer> const& vbuffer, std::shared_ptr<IndexBuffer> const& ibuffer, std::shared_ptr<VisualEffect> const& effect)
{
	GLSLVisualProgram* gl4program = dynamic_cast<GLSLVisualProgram*>(effect->GetProgram().get());
	if (!gl4program)
	{
		qCritical("HLSL effect passed to GLSL engine.");
		return 0;
	}
	auto programHandle = gl4program->GetProgramHandle();

	// Enable the vertex buffer and input layout.
	GL4VertexBuffer* gl4VBuffer = nullptr;
	GL4InputLayout* gl4Layout = nullptr;
	if (vbuffer->IsFormatted())
	{
		gl4VBuffer = static_cast<GL4VertexBuffer*>(Bind(vbuffer));
		GL4InputLayoutManager* manager = static_cast<GL4InputLayoutManager*>(mILMap.get());
		gl4Layout = manager->Bind(programHandle, gl4VBuffer->GetGLHandle(), vbuffer.get());
		gl4Layout->Enable();
	}

	// Enable the index buffer.
	GL4IndexBuffer* gl4IBuffer = nullptr;
	if (ibuffer->IsIndexed())
	{
		gl4IBuffer = static_cast<GL4IndexBuffer*>(Bind(ibuffer));
		gl4IBuffer->Enable();
	}

	DrawPrimitive(vbuffer.get(), ibuffer.get());

	// Disable the vertex buffer and input layout.
	if (vbuffer->IsFormatted())
	{
		gl4Layout->Disable();
	}

	// Disable the index buffer.
	if (gl4IBuffer)
	{
		gl4IBuffer->Disable();
	}

	return 0;
}
