// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGL4SamplerState.h>
using namespace gte;

GL4SamplerState::~GL4SamplerState()
{
    GL::function().glDeleteSamplers(1, &mGLHandle);
}

GL4SamplerState::GL4SamplerState(SamplerState const* samplerState)
    :
    GL4DrawingState(samplerState)
{
	GL::function().glGenSamplers(1, &mGLHandle);

	GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_WRAP_S, msMode[samplerState->mode[0]]);
	GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_WRAP_T, msMode[samplerState->mode[1]]);
	GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_WRAP_R, msMode[samplerState->mode[2]]);

    // TODO - GL_TEXTURE_MAX_ANISOTROPY_EXT is not defined?
    // glSamplerParameterf(samplerState, GL_TEXTURE_MAX_ANISOTROPY_EXT, samplerState->maxAnisotropy);

	GL::function().glSamplerParameterf(mGLHandle, GL_TEXTURE_MIN_LOD, samplerState->minLOD);
	GL::function().glSamplerParameterf(mGLHandle, GL_TEXTURE_MAX_LOD, samplerState->maxLOD);
	GL::function().glSamplerParameterf(mGLHandle, GL_TEXTURE_LOD_BIAS, samplerState->mipLODBias);

    float borderColor[4];
    borderColor[0] = samplerState->borderColor[0];
    borderColor[1] = samplerState->borderColor[1];
    borderColor[2] = samplerState->borderColor[2];
    borderColor[3] = samplerState->borderColor[3];
	GL::function().glSamplerParameterfv(mGLHandle, GL_TEXTURE_BORDER_COLOR, borderColor);

    switch(samplerState->filter)
    {
        case SamplerState::MIN_P_MAG_P_MIP_P:
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerState::MIN_P_MAG_P_MIP_L:
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerState::MIN_P_MAG_L_MIP_P:
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerState::MIN_P_MAG_L_MIP_L:
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerState::MIN_L_MAG_P_MIP_P:
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerState::MIN_L_MAG_P_MIP_L:
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            break;
        case SamplerState::MIN_L_MAG_L_MIP_P:
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        case SamplerState::MIN_L_MAG_L_MIP_L:
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            break;
        default:
			qWarning("GL4 does not support samplerState filter = " + samplerState->filter);
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MAG_FILTER, 0);
			GL::function().glSamplerParameteri(mGLHandle, GL_TEXTURE_MIN_FILTER, 0);
            break;
    }
}

GEObject* GL4SamplerState::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_SAMPLER_STATE)
    {
        return new GL4SamplerState(static_cast<SamplerState const*>(object));
    }

    qCritical("Invalid object type.");
    return nullptr;
}


GLint const GL4SamplerState::msMode[] =
{
    GL_REPEAT,          // WRAP
    GL_MIRRORED_REPEAT, // MIRROR
    GL_CLAMP_TO_EDGE,   // CLAMP
    GL_CLAMP_TO_BORDER, // BORDER
    GL_MIRRORED_REPEAT  // MIRROR_ONCE
};
