// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
//#include <LowLevel/GteLogger.h>
#include <Graphics/GL4/GteGLSLComputeProgram.h>
#include <Graphics/GL4/GteGLSLProgramFactory.h>
#include <Graphics/GL4/GteGLSLVisualProgram.h>

#include <qdebug.h>

using namespace gte;

std::string GLSLProgramFactory::defaultVersion = "#version 330";
std::string GLSLProgramFactory::defaultVSEntry = "main";
std::string GLSLProgramFactory::defaultPSEntry = "main";
std::string GLSLProgramFactory::defaultGSEntry = "main";
std::string GLSLProgramFactory::defaultCSEntry = "main";
unsigned int GLSLProgramFactory::defaultFlags = 0;  // unused in GLSL for now

GLSLProgramFactory::GLSLProgramFactory()
{
    version = defaultVersion;
    vsEntry = defaultVSEntry;
    psEntry = defaultPSEntry;
    gsEntry = defaultGSEntry;
    csEntry = defaultCSEntry;
    flags = defaultFlags;
}

std::shared_ptr<VisualProgram> GLSLProgramFactory::CreateFromNamedSources(
    std::string const&, std::string const& vsSource,
    std::string const&, std::string const& psSource,
    std::string const&, std::string const& gsSource)
{
    if (vsSource == "" || psSource == "")
    {
        qCritical("A program must have a vertex shader and a pixel shader.");
        return nullptr;
    }

    GLuint vsHandle = Compile(GL_VERTEX_SHADER, vsSource);
    if (vsHandle == 0)
    {
        return nullptr;
    }

    GLuint psHandle = Compile(GL_FRAGMENT_SHADER, psSource);
    if (psHandle == 0)
    {
        return nullptr;
    }

    GLuint gsHandle = 0;
    if (gsSource != "")
    {
        gsHandle = Compile(GL_GEOMETRY_SHADER, gsSource);
        if (gsHandle == 0)
        {
            return nullptr;
        }
    }

    GLuint programHandle = GL::function().glCreateProgram();
    if (programHandle == 0)
    {
        qCritical("Program creation failed.");
        return nullptr;
    }

	GL::function().glAttachShader(programHandle, vsHandle);
	GL::function().glAttachShader(programHandle, psHandle);
    if (gsHandle > 0)
    {
		GL::function().glAttachShader(programHandle, gsHandle);
    }

    if (!Link(programHandle))
    {
		GL::function().glDetachShader(programHandle, vsHandle);
		GL::function().glDeleteShader(vsHandle);
		GL::function().glDetachShader(programHandle, psHandle);
		GL::function().glDeleteShader(psHandle);
        if (gsHandle)
        {
			GL::function().glDetachShader(programHandle, gsHandle);
			GL::function().glDeleteShader(gsHandle);
        }
		GL::function().glDeleteProgram(programHandle);
        return nullptr;
    }

    std::shared_ptr<GLSLVisualProgram> program =
        std::make_shared<GLSLVisualProgram>(programHandle, vsHandle,
        psHandle, gsHandle);

    GLSLReflection const& reflector = program->GetReflector();
    program->SetVShader(std::make_shared<VertexShader>(reflector));
    program->SetPShader(std::make_shared<PixelShader>(reflector));
    if (gsHandle > 0)
    {
        program->SetGShader(std::make_shared<GeometryShader>(reflector));
    }
    return program;
}

std::shared_ptr<ComputeProgram> GLSLProgramFactory::CreateFromNamedSource(
    std::string const&, std::string const& csSource)
{
    if (csSource == "")
    {
        qCritical("A program must have a compute shader.");
        return nullptr;
    }

    GLuint csHandle = Compile(GL_COMPUTE_SHADER, csSource);
    if (csHandle == 0)
    {
        return nullptr;
    }

	GLuint programHandle = GL::function().glCreateProgram();
    if (programHandle == 0)
    {
        qCritical("Program creation failed.");
        return nullptr;
    }

	GL::function().glAttachShader(programHandle, csHandle);

    if (!Link(programHandle))
    {
		GL::function().glDetachShader(programHandle, csHandle);
		GL::function().glDeleteShader(csHandle);
		GL::function().glDeleteProgram(programHandle);
        return nullptr;
    }

    std::shared_ptr<GLSLComputeProgram> program =
        std::make_shared<GLSLComputeProgram>(programHandle, csHandle);

    GLSLReflection const& reflector = program->GetReflector();
    program->SetCShader(std::make_shared<ComputeShader>(reflector));
    return program;
}

GLuint GLSLProgramFactory::Compile(GLenum shaderType,
    std::string const& source)
{
	GLuint handle = GL::function().glCreateShader(shaderType);
	if (handle > 0)
	{
		// Prepend to the definitions
		// 1. The version of the GLSL program; for example, "#version 400".
		// 2. A define for the matrix-vector multiplication convention if
		//    it is selected as GTE_USE_MAT_VEC: "define GTE_USE_MAT_VEC 1"
		//    else "define GTE_USE_MAT_VEC 0".
		// 3. "layout(std140, *_major) uniform;" for either row_major or column_major
		//    to select default for all uniform matrices and select std140 layout.
		// 4. "layout(std430, *_major) buffer;" for either row_major or column_major
		//    to select default for all buffer matrices and select std430 layout.
		// Append to the definitions the source-code string.
		auto const& definitions = defines.Get();
		std::vector<std::string> glslDefines;
		glslDefines.reserve(definitions.size() + 5);
		glslDefines.push_back(version + "\n");
#if 0
#if defined(GTE_USE_MAT_VEC)
		glslDefines.push_back("#define GTE_USE_MAT_VEC 1\n");
#else
		glslDefines.push_back("#define GTE_USE_MAT_VEC 0\n");
#endif
#endif
#if defined(GTE_USE_ROW_MAJOR)
		glslDefines.push_back("layout(std140, row_major) uniform;\n");
		glslDefines.push_back("layout(std330, row_major) buffer;\n");
#else
		//glslDefines.push_back("layout(column_major) uniform;\n");
		//glslDefines.push_back("layout(column_major) buffer;\n");
#endif
		for (auto d : definitions)
		{
			glslDefines.push_back("#define " + d.first + " " + d.second + "\n");
		}
		glslDefines.push_back(source);

		// Repackage the definitions for glShaderSource.
		std::vector<GLchar const*> code;
		code.reserve(glslDefines.size());
		for (auto const& d : glslDefines)
		{
			code.push_back(d.c_str());
		}

		GL::function().glShaderSource(handle, static_cast<GLsizei>(code.size()), &code[0],
			nullptr);

		GL::function().glCompileShader(handle);
		GLint status;
		GL::function().glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
		if (status == GL_FALSE)
		{
			GLint logLength;
			GL::function().glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &logLength);
			if (logLength > 0)
			{
				GLchar* log = new GLchar[logLength];
				GLsizei numWritten;
				GL::function().glGetShaderInfoLog(handle, static_cast<GLsizei>(logLength),
					&numWritten, log);
				qCritical() << "Compile failed:\n"  << QString(log);
				delete[] log;
			}
			else
			{
				qCritical("Invalid info log length.");
			}
			GL::function().glDeleteShader(handle);
			handle = 0;
		}
}
	else
	{
		qCritical("Cannot create shader.");
	}
	return handle;
}

bool GLSLProgramFactory::Link(GLuint programHandle)
{
	GL::function().glLinkProgram(programHandle);
    int status;
	GL::function().glGetProgramiv(programHandle, GL_LINK_STATUS, &status);
    if (status == GL_FALSE)
    {
        int logLength;
		GL::function().glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0)
        {
            char* log = new char[logLength];
            int numWritten;
			GL::function().glGetProgramInfoLog(programHandle, logLength, &numWritten, log);
            qCritical("Link failed:\n" /*+ std::string(log)*/);
            delete[] log;
        }
        else
        {
            qCritical("Invalid info log length.");
        }
        return false;
    }
    else
    {
        return true;
    }
}
