// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GteTexture2ColorEffect.h>

using namespace gte;


Texture2ColorEffect::Texture2ColorEffect(ProgramFactory& factory,
	std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
	SamplerState::Mode mode0, SamplerState::Mode mode1)
	:
	mTexture(texture),
	mPVWMatrix(nullptr)
{
	int api = factory.GetAPI();
	mProgram = factory.CreateFromSources(*msVSSource[api], *msPSSource[api], "");
	if (mProgram)
	{
		mPVWMatrixConstant = std::make_shared<ConstantBuffer>(
			sizeof(EMatrix4x4), true);
		mPVWMatrix = mPVWMatrixConstant->Get<EMatrix4x4>();
		*mPVWMatrix = EMatrix4x4::Identity();

		mSampler = std::make_shared<SamplerState>();
		mSampler->filter = filter;
		mSampler->mode[0] = mode0;
		mSampler->mode[1] = mode1;

		mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
		mProgram->GetPShader()->Set("baseSampler", texture);
		mProgram->GetPShader()->Set("baseSampler", mSampler);
	}
}


std::string const Texture2ColorEffect::msGLSLVSSource =
"uniform PVWMatrix\n"
"{\n"
"    mat4 pvwMatrix;\n"
"};\n"
"\n"
"in vec3 modelPosition;\n"
"in vec2 modelTCoord;\n"
"in vec4 modelColor;\n"
"out vec2 vertexTCoord;\n"
"out vec4 vertexColor;\n"
"\n"
"void main()\n"
"{\n"
"    vertexTCoord = modelTCoord;\n"
"    vertexColor  = modelColor;\n"
"    gl_Position  = pvwMatrix * vec4(modelPosition, 1.0f);\n"
"}\n";


std::string const Texture2ColorEffect::msGLSLPSSource =
"uniform sampler2D baseSampler;\n"
"\n"
"in vec2 vertexTCoord;\n"
"in vec4 vertexColor;\n"
"out vec4 pixelColor;\n"
"\n"
"void main()\n"
"{\n"
"    pixelColor = texture(baseSampler, vertexTCoord);\n"
"    pixelColor *= vertexColor;\n"
"}\n";


std::string const Texture2ColorEffect::msHLSLSource =
"cbuffer PVWMatrix\n"
"{\n"
"    float4x4 pvwMatrix;\n"
"};\n"
"\n"
"struct VS_INPUT\n"
"{\n"
"    float3 modelPosition : POSITION;\n"
"    float2 modelTCoord : TEXCOORD0;\n"
"};\n"
"\n"
"struct VS_OUTPUT\n"
"{\n"
"    float2 vertexTCoord : TEXCOORD0;\n"
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
"    output.vertexTCoord = input.modelTCoord;\n"
"    return output;\n"
"}\n"
"\n"
"Texture2D baseTexture;\n"
"SamplerState baseSampler;\n"
"\n"
"struct PS_INPUT\n"
"{\n"
"    float2 vertexTCoord : TEXCOORD0;\n"
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
"    output.pixelColor0 = baseTexture.Sample(baseSampler, input.vertexTCoord);\n"
"    return output;\n"
"}\n";

std::string const* Texture2ColorEffect::msVSSource[] =
{
	&msGLSLVSSource,
	&msHLSLSource
};

std::string const* Texture2ColorEffect::msPSSource[] =
{
	&msGLSLPSSource,
	&msHLSLSource
};

