// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GteConstantColorEffect.h>
using namespace gte;


ConstantColorEffect::ConstantColorEffect(ProgramFactory& factory,
	EVector4  const& color)
    :
    mPVWMatrix(nullptr),
    mColor(nullptr)
{
    int i = factory.GetAPI();
    mProgram = factory.CreateFromSources(*msVSSource[i], *msPSSource[i], "");
    if (mProgram)
    {
        mPVWMatrixConstant = std::make_shared<ConstantBuffer>(
            sizeof(EMatrix4x4), true);
        mPVWMatrix = mPVWMatrixConstant->Get<EMatrix4x4>();
		*mPVWMatrix  = EMatrix4x4::Identity();

        mColorConstant = std::make_shared<ConstantBuffer>(
			sizeof(EVector4), true);
        mColor = mColorConstant->Get<EVector4>();
        *mColor = color;

        mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
        mProgram->GetVShader()->Set("ConstantColor", mColorConstant);
    }
}


std::string const ConstantColorEffect::msGLSLVSSource =
"uniform PVWMatrix\n"
"{\n"
"    mat4 pvwMatrix;\n"
"};\n"
"\n"
"uniform ConstantColor\n"
"{\n"
"    vec4 constantColor;\n"
"};\n"
"\n"
"in vec3 modelPosition;\n"
"\n"
"out vec4 vertexColor;\n"
"\n"
"void main()\n"
"{\n"
"    gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);\n"
"    vertexColor = constantColor;\n"
"}\n";

std::string const ConstantColorEffect::msGLSLPSSource =
"in vec4 vertexColor;\n"
"\n"
"out vec4 pixelColor0;\n"
"\n"
"void main()\n"
"{\n"
"    pixelColor0 = vertexColor;\n"
"}\n";

std::string const ConstantColorEffect::msHLSLSource =
"cbuffer PVWMatrix\n"
"{\n"
"    float4x4 pvwMatrix;\n"
"};\n"
"\n"
"cbuffer ConstantColor\n"
"{\n"
"    float4 constantColor;\n"
"};\n"
"\n"
"struct VS_INPUT\n"
"{\n"
"    float3 modelPosition : POSITION;\n"
"};\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"    float4 vertexColor : COLOR0;\n"
"    float4 clipPosition : SV_POSITION;\n"
"};\n"
"\n"
"VS_OUTPUT VSMain(VS_INPUT input)\n"
"{\n"
"    VS_OUTPUT output;\n"
"#if GTE_USE_MAT_VEC\n"
"    output.clipPosition = mul(pvwMatrix, float4(input.modelPosition, 1.0f));\n"
"#else\n"
"    output.clipPosition = mul(float4(input.modelPosition, 1.0f), pvwMatrix);\n"
"#endif\n"
"    output.vertexColor = constantColor;\n"
"    return output;\n"
"}\n"
"\n"
"struct PS_INPUT\n"
"{\n"
"    float4 vertexColor : COLOR0;\n"
"};\n"
"\n"
"struct PS_OUTPUT\n"
"{\n"
"    float4 pixelColor0 : SV_TARGET0;\n"
"};\n"
"\n"
"PS_OUTPUT PSMain(PS_INPUT input)\n"
"{\n"
"    PS_OUTPUT output;\n"
"    output.pixelColor0 = input.vertexColor;\n"
"    return output;\n"
"}\n";

std::string const* ConstantColorEffect::msVSSource[] =
{
    &msGLSLVSSource,
    &msHLSLSource
};

std::string const* ConstantColorEffect::msPSSource[] =
{
    &msGLSLPSSource,
    &msHLSLSource
};
