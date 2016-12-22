// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGLSLVisualProgram.h>
using namespace gte;

GLSLVisualProgram::~GLSLVisualProgram()
{
    if (GL::function().glIsProgram(mProgramHandle))
    {
		if (GL::function().glIsShader(mVShaderHandle))
        {
			GL::function().glDetachShader(mProgramHandle, mVShaderHandle);
			GL::function().glDeleteShader(mVShaderHandle);
        }

		if (GL::function().glIsShader(mPShaderHandle))
        {
			GL::function().glDetachShader(mProgramHandle, mPShaderHandle);
			GL::function().glDeleteShader(mPShaderHandle);
        }

		if (GL::function().glIsShader(mGShaderHandle))
        {
			GL::function().glDetachShader(mProgramHandle, mGShaderHandle);
			GL::function().glDeleteShader(mGShaderHandle);
        }

		GL::function().glDeleteProgram(mProgramHandle);
    }
}

GLSLVisualProgram::GLSLVisualProgram(GLuint programHandle,
    GLuint vshaderHandle, GLuint pshaderHandle, GLuint gshaderHandle)
    :
    mProgramHandle(programHandle),
    mVShaderHandle(vshaderHandle),
    mPShaderHandle(pshaderHandle),
    mGShaderHandle(gshaderHandle),
    mReflector(programHandle)
{
}
