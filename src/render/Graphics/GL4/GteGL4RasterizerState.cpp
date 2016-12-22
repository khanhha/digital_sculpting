// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGL4RasterizerState.h>
using namespace gte;

GL4RasterizerState::GL4RasterizerState(RasterizerState const* rasterizerState)
    :
    GL4DrawingState(rasterizerState)
{
    mFillMode = msFillMode[rasterizerState->fillMode];
    mCullFace = msCullFace[rasterizerState->cullMode];
    mFrontFace = (rasterizerState->frontCCW ? GL_CCW : GL_CW);
    mDepthScale = rasterizerState->slopeScaledDepthBias;
    mDepthBias = static_cast<float>(rasterizerState->depthBias);
    mEnableScissor = (rasterizerState->enableScissor ? GL_TRUE : GL_FALSE);
}

GEObject* GL4RasterizerState::Create(void*, GraphicsObject const* object)
{
    if (object->GetType() == GT_RASTERIZER_STATE)
    {
        return new GL4RasterizerState(
            static_cast<RasterizerState const*>(object));
    }

    qCritical("Invalid object type.");
    return nullptr;
}

void GL4RasterizerState::Enable(HGLRC)
{
#if 0
	/*why is it disabled in OpenGL ES*/
	//GL::function().glPolygonMode(GL_FRONT_AND_BACK, mFillMode);
#endif

    if (mCullFace != 0)
    {
        GL::function().glEnable(GL_CULL_FACE);
		GL::function().glFrontFace(mFrontFace);
		GL::function().glCullFace(mCullFace);
    }
    else
    {
		GL::function().glDisable(GL_CULL_FACE);
    }

    if (mDepthScale != 0.0f && mDepthBias != 0.0f)
    {
		GL::function().glEnable(GL_POLYGON_OFFSET_FILL);
		GL::function().glEnable(GL_POLYGON_OFFSET_LINE);
		GL::function().glEnable(GL_POLYGON_OFFSET_POINT);
		GL::function().glPolygonOffset(mDepthScale, mDepthBias);
    }
    else
    {
		GL::function().glDisable(GL_POLYGON_OFFSET_FILL);
		GL::function().glDisable(GL_POLYGON_OFFSET_LINE);
		GL::function().glDisable(GL_POLYGON_OFFSET_POINT);
    }
}


GLenum const GL4RasterizerState::msFillMode[] =
{
    GL_FILL,
    GL_LINE
};

GLenum const GL4RasterizerState::msCullFace[] =
{
    0,
    GL_FRONT,
    GL_BACK
};
