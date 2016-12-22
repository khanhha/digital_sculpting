// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <Graphics/GteLightingEffect.h>
#include <GTEngineDEF.h>


namespace gte
{

class GTE_IMPEXP SpotLightEffect : public LightingEffect
{
public:
    // Construction.  Set 'select' to 0 for per-vertex effects or to 1 for per-pixel effects.
    SpotLightEffect(ProgramFactory& factory, BufferUpdater const& updater, int select,
        std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting,
        std::shared_ptr<LightCameraGeometry> const& geometry);

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
		EVector4  spotCutoff;
		EVector4  attenuation;
    };

    struct InternalGeometry
    {
		EVector4  lightModelPosition;
		EVector4  lightModelDirection;
		EVector4  cameraModelPosition;
    };

    // Shader source code as strings.
    static std::string const msGLSLVSSource[2];
    static std::string const msGLSLPSSource[2];
    static std::string const msHLSLSource[2];
    static std::string const* msVSSource[2][ProgramFactory::PF_NUM_API];
    static std::string const* msPSSource[2][ProgramFactory::PF_NUM_API];
};

}
