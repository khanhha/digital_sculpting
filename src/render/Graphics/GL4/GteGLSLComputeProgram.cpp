// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GL4/GteGLSLComputeProgram.h>
using namespace gte;

GLSLComputeProgram::~GLSLComputeProgram()
{
	if (GL::function().glIsProgram(mProgramHandle))
    {
		if (GL::function().glIsShader(mCShaderHandle))
        {
			GL::function().glDetachShader(mProgramHandle, mCShaderHandle);
			GL::function().glDeleteShader(mCShaderHandle);
        }

		GL::function().glDeleteProgram(mProgramHandle);
    }
}

GLSLComputeProgram::GLSLComputeProgram(GLuint programHandle, GLuint cshaderHandle)
    :
    mProgramHandle(programHandle),
    mCShaderHandle(cshaderHandle),
    mReflector(programHandle)
{
}
