// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#include <GTEnginePCH.h>
#include <Graphics/GteLightingEffect.h>
#include <GTEngineDEF.h>

using namespace gte;

LightingEffect::LightingEffect(ProgramFactory& factory, BufferUpdater const& updater,
	std::string const* vsSource[], std::string const* psSource[],
	std::shared_ptr<Material> const& material,
	std::shared_ptr<Lighting> const& lighting,
	std::shared_ptr<LightCameraGeometry> const& geometry)
    :
    mMaterial(material),
	mLighting(lighting),
	mGeometry(geometry)
{
    int api = factory.GetAPI();
    mProgram = factory.CreateFromSources(*vsSource[api], *psSource[api], "");
    if (mProgram)
    {
        mBufferUpdater = updater;
        mPVWMatrixConstant = std::make_shared<ConstantBuffer>(sizeof(EMatrix4x4), true);
        mProgram->GetVShader()->Set("PVWMatrix", mPVWMatrixConstant);
    }
}


void gte::LightingEffect::UpdatePVWMatrix(const EMatrix4x4 &mat)
{
	if (mPVWMatrixConstant){
		mPVWMatrixConstant->SetMember("pvwMatrix", mat);
		mBufferUpdater(mPVWMatrixConstant);
	}
}

void LightingEffect::UpdateMaterialConstant()
{
    if (mMaterialConstant)
    {
        mBufferUpdater(mMaterialConstant);
    }
}

void LightingEffect::UpdateLightingConstant()
{
    if (mLightingConstant)
    {
        mBufferUpdater(mLightingConstant);
    }
}

void LightingEffect::UpdateGeometryConstant()
{
    if (mGeometryConstant)
    {
        mBufferUpdater(mGeometryConstant);
    }
}

std::string LightingEffect::GetShaderSourceLitFunctionGLSL()
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

