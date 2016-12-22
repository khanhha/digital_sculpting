// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <Graphics/GteLightingEffect.h>
#include <Graphics/GteTexture2.h>

namespace gte
{

class GTE_IMPEXP DirectionalLightTextureEffect : public LightingEffect
{
public:
    // Construction.
    DirectionalLightTextureEffect(ProgramFactory& factory, BufferUpdater const& updater,
        std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting,
        std::shared_ptr<LightCameraGeometry> const& geometry,
        std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
        SamplerState::Mode mode0, SamplerState::Mode mode1);

    // Member access.
    inline std::shared_ptr<Texture2> const& GetTexture() const;
    inline std::shared_ptr<SamplerState> const& GetSampler() const;

    // After you set or modify 'material', 'light', or 'geometry', call the update
    // to inform any listener that the corresponding constant buffer has changed.
    virtual void UpdateMaterialConstant();
    virtual void UpdateLightingConstant();
    virtual void UpdateGeometryConstant();

private:
    struct InternalMaterial
    {
        EVector4  emissive;
		EVector4  ambient;
		EVector4  diffuse;
		EVector4  specular;
    };

    struct InternalLighting
    {
		EVector4  ambient;
		EVector4  diffuse;
		EVector4  specular;
		EVector4  attenuation;
    };

    struct InternalGeometry
    {
		EVector4  lightModelDirection;
		EVector4  cameraModelPosition;
    };

    std::shared_ptr<Texture2> mTexture;
    std::shared_ptr<SamplerState> mSampler;

    // Shader source code as strings.
    static std::string const msGLSLVSSource;
    static std::string const msGLSLPSSource;
    static std::string const msHLSLSource;
    static std::string const* msVSSource[ProgramFactory::PF_NUM_API];
    static std::string const* msPSSource[ProgramFactory::PF_NUM_API];
};

inline std::shared_ptr<Texture2> const& DirectionalLightTextureEffect::GetTexture() const
{
    return mTexture;
}

inline std::shared_ptr<SamplerState> const& DirectionalLightTextureEffect::GetSampler() const
{
    return mSampler;
}

}
