// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <GteWireframeEffect.h>
#include <GteGLSLProgramFactory.h>
#include <fstream>
#include <string>
using namespace gte;

WireframeEffect::WireframeEffect(ProgramFactory& factory, BufferUpdater const& updater, 
	std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting, 
	std::shared_ptr<LightCameraGeometry> const& geometry)
	: 
	mMaterial(material),
	mLighting(lighting),
	mGeometry(geometry)
{
	mBufferUpdater = updater;

	GLSLProgramFactory programFactory;
	mProgram = programFactory.CreateFromSources(msGLSLVSSource, msGLSLPSSource, msGLSLGSSource);
	if (mProgram)
	{
		{
			mPVWMatrixConstant = std::make_shared<ConstantBuffer>(
				sizeof(EMatrix4x4), true);
			mPVWMatrix = mPVWMatrixConstant->Get<EMatrix4x4>();
			*mPVWMatrix = EMatrix4x4::Identity();

			mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
		}
		
		{
			mMaterialConstant = std::make_shared<ConstantBuffer>(sizeof(InternalMaterial), true);
			UpdateMaterialConstant();

			mLightingConstant = std::make_shared<ConstantBuffer>(sizeof(InternalLighting), true);
			UpdateLightingConstant();

			mGeometryConstant = std::make_shared<ConstantBuffer>(sizeof(InternalGeometry), true);
			UpdateGeometryConstant();

			mProgram->GetPShader()->Set("Material", mMaterialConstant);
			mProgram->GetPShader()->Set("Lighting", mLightingConstant);
			mProgram->GetPShader()->Set("LightCameraGeometry", mGeometryConstant);
		}

		{
			mWireParameters = std::make_shared<ConstantBuffer>(3 * sizeof(EVector4), true);
			mWireParametersData = reinterpret_cast<WireParam*>(mWireParameters->Get<EVector4>());
			mProgram->GetPShader()->Set("WireParameters", mWireParameters);
			mProgram->GetGShader()->Set("WireParameters", mWireParameters);
		}
	}
}

void gte::WireframeEffect::UpdatePVWMatrix(const EMatrix4x4 &mat)
{
	if (mPVWMatrixConstant){
		mPVWMatrixConstant->SetMember("pvwMatrix", mat);
		mBufferUpdater(mPVWMatrixConstant);
	}
}

void WireframeEffect::UpdateMaterialConstant()
{
	InternalMaterial* internalMaterial = mMaterialConstant->Get<InternalMaterial>();
	internalMaterial->emissive = mMaterial->emissive;
	internalMaterial->ambient = mMaterial->ambient;
	internalMaterial->diffuse = mMaterial->diffuse;
	internalMaterial->specular = mMaterial->specular;
	if (mMaterialConstant)
	{
		mBufferUpdater(mMaterialConstant);
	}
}

void WireframeEffect::UpdateLightingConstant()
{
	InternalLighting* internalLighting = mLightingConstant->Get<InternalLighting>();
	internalLighting->ambient = mLighting->ambient;
	internalLighting->diffuse = mLighting->diffuse;
	internalLighting->specular = mLighting->specular;
	internalLighting->attenuation = mLighting->attenuation;
	if (mLightingConstant)
	{
		mBufferUpdater(mLightingConstant);
	}
}

void WireframeEffect::UpdateGeometryConstant()
{
	InternalGeometry* internalGeometry = mGeometryConstant->Get<InternalGeometry>();
	internalGeometry->lightModelPosition = mGeometry->lightModelPosition;
	internalGeometry->cameraModelPosition = mGeometry->cameraModelPosition;
	if (mGeometryConstant)
	{
		mBufferUpdater(mGeometryConstant);
	}
}


void WireframeEffect::UpdateWireframeParams()
{
	if (mWireParameters){
		mBufferUpdater(mWireParameters);
	}
}

std::string WireframeEffect::GetShaderSourceLitFunctionGLSL()
{
	// From MSDN documentation:
	// Returns a lighting coefficient vector.
	// ret lit(n_dot_l, n_dot_h, m)
	// This function returns a lighting coefficient vector (ambient, diffuse, specular, 1) where:
	//  * ambient = 1.
	//  * diffuse = ((n dot l) < 0) ? 0 : n dot l.
	//  * specular = ((n dot l) < 0) || ((n dot h) < 0) ? 0 : (pow(n dot h, m)).
	// Where the vector n is the normal vector, l is the direction to light and h is the half vector.
	// Parameters Description
	//  n_dot_l [in] The dot product of the normalized surface normal and the light vector.
	//  n_dot_h [in] The dot product of the half-angle vector and the surface normal.
	//  m [in] A specular exponent.
	static std::string const sourceGLSL =
		"vec4 lit(float NdotL, float NdotH, float m)\n"
		"{\n"
		"  float ambient = 1.0;\n"
		"  float diffuse = max(NdotL, 0.0);\n"
		"  float specular = step(0.0, NdotL) * max(pow(NdotH, m), 0.0);\n"
		"  return vec4(ambient, diffuse, specular, 1.0);\n"
		"}\n";

	return sourceGLSL;
}

std::string const WireframeEffect::msGLSLVSSource =
"#extension GL_ARB_separate_shader_objects : enable \n"
"uniform PVWMatrix\n"
"{\n"
"    mat4 pvwMatrix;\n"
"};\n"
"\n"
"layout(location = 0) in vec3 modelPosition;\n"
"layout(location = 1) in vec3 modelNormal;\n"
"\n"
"layout(location = 0) out vec3 vertexPosition;\n"
"layout(location = 1) out vec3 vertexNormal;\n"
"\n"
"void main()\n"
"{\n"
"    vertexPosition = modelPosition;\n"
"    vertexNormal = modelNormal;\n"
"    gl_Position = pvwMatrix * vec4(modelPosition, 1.0f);\n"
"}\n";

std::string const WireframeEffect::msGLSLGSSource =
"#extension GL_ARB_separate_shader_objects : enable \n"
"uniform WireParameters \n"
"{\n"
"	vec4 edgeColor;\n"
"	vec2 windowSize;\n"
"};\n"
"\n"
"const vec3 basis[3] = vec3[](\n"
"	vec3(1.0f, 0.0f, 0.0f),\n"
"	vec3(0.0f, 1.0f, 0.0f),\n"
"	vec3(0.0f, 0.0f, 1.0f)\n"
"	);\n"
"\n"
"layout(location = 0) in vec3 vertexPosition[];\n"
"layout(location = 1) in vec3 vertexNormal[];\n"
"\n"
"layout(location = 0) out vec3 vertexPositionG;\n"
"layout(location = 1) out vec3 vertexNormalG;\n"
"layout(location = 2) noperspective out vec3 edgeDistanceG;\n"
"\n"
"layout(triangles) in;\n"
"layout(triangle_strip, max_vertices = 3) out;\n"
"void main()\n"
"{\n"
"	vec2 pixel[3];\n"
"	float W[3];\n"
"	int i;\n"
"	for (i = 0; i < 3; ++i)\n"
"	{\n"
"		vec2 ndc = gl_in[i].gl_Position.xy / gl_in[i].gl_Position.w;\n"
"		pixel[i] = 0.5f * windowSize * (ndc + 1.0f);\n"
"	}\n"
"\n"
"	int j0[3] = int[](2, 0, 1); \n"
"	int j1[3] = int[](1, 2, 0);\n"
"	for (i = 0; i < 3; ++i)\n"
"	{\n"
"		vec2 diff = pixel[i] - pixel[j1[i]];\n"
"		vec2 edge = pixel[j0[i]] - pixel[j1[i]];\n"
"		float edgeLength = length(edge);\n"
"		float distance;\n"
"		if (edgeLength > 0.0f)\n"
"		{\n"
"			distance = abs(dot(diff, vec2(edge.y, -edge.x)) / edgeLength);\n"
"		}\n"
"		else\n"
"		{\n"
"			distance = 0.0f;\n"
"		}\n"
"\n"
"		gl_Position = gl_in[i].gl_Position;\n"
"		vertexPositionG = vertexPosition[i];\n"
"		vertexNormalG	= vertexNormal[i];"
"		edgeDistanceG	= distance * basis[i];\n"
"		EmitVertex();\n"
"	}\n"
"\n"
"	EndPrimitive();\n"
"}\n";

std::string const WireframeEffect::msGLSLPSSource =
"#extension GL_ARB_separate_shader_objects : enable \n" +
GetShaderSourceLitFunctionGLSL() +
"uniform WireParameters \n"
"{\n"
"	vec4 edgeColor;\n"
"	vec2 windowSize;\n"
"};\n"
"\n"
"uniform Material\n"
"{\n"
"    vec4 materialEmissive;\n"
"    vec4 materialAmbient;\n"
"    vec4 materialDiffuse;\n"
"    vec4 materialSpecular;\n"
"};\n"
"\n"
"uniform Lighting\n"
"{\n"
"    vec4 lightingAmbient;\n"
"    vec4 lightingDiffuse;\n"
"    vec4 lightingSpecular;\n"
"    vec4 lightingAttenuation;\n"
"};\n"
"\n"
"uniform LightCameraGeometry\n"
"{\n"
"    vec4 lightModelPosition;\n"
"    vec4 cameraModelPosition;\n"
"};\n"
"\n"
"layout(location = 0) in vec3 vertexPosition;\n"
"layout(location = 1) in vec3 vertexNormal;\n"
"layout(location = 2) noperspective in vec3 edgeDistance;\n"
"\n"
"layout(location = 0) out vec4 pixelColor0;\n"
"\n"
"void main()\n"
"{\n"
"    vec3 modelLightDiff = vertexPosition - lightModelPosition.xyz;\n"
"    vec3 vertexDirection = normalize(modelLightDiff);\n"
"    float NDotL = -dot(vertexNormal, vertexDirection);\n"
"    vec3 viewVector = normalize(cameraModelPosition.xyz - vertexPosition.xyz);\n"
"    vec3 halfVector = normalize(viewVector - vertexDirection);\n"
"    float NDotH = dot(vertexNormal, halfVector);\n"
"    vec4 lighting = lit(NDotL, NDotH, materialSpecular.a);\n"
"\n"
"    float distance = length(modelLightDiff);\n"
"    float attenuation = lightingAttenuation.w / (lightingAttenuation.x + distance *\n"
"        (lightingAttenuation.y + distance * lightingAttenuation.z));\n"
"\n"
"    vec3 color = materialAmbient.rgb * lightingAmbient.rgb +\n"
"        lighting.y * materialDiffuse.rgb * lightingDiffuse.rgb +\n"
"        lighting.z * materialSpecular.rgb * lightingSpecular.rgb;\n"
"\n"
"    vec4 lightColor;\n"
"	 lightColor.rgb		= materialEmissive.rgb + attenuation * color;\n"
"    lightColor.a		= materialDiffuse.a;\n"
"\n"
"	float dmin = min(min(edgeDistance[0], edgeDistance[1]), edgeDistance[2]);\n"
"	float blend = smoothstep(0.0f, 1.0f, dmin);\n"
"	pixelColor0 = mix(edgeColor, lightColor, blend);\n"
"}\n";

std::string const WireframeEffect::msHLSLSource =
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

std::string const* WireframeEffect::msVSSource[] =
{
	&msGLSLVSSource,
	&msHLSLSource
};

std::string const* WireframeEffect::msPSSource[] =
{
	&msGLSLPSSource,
	&msHLSLSource
};
