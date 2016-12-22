// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <Graphics/GteVisualEffect.h>
#include <GTEngineDEF.h>

namespace gte
{

class GTE_IMPEXP ConstantColorEffect : public VisualEffect
{
public:
    // Construction.
	ConstantColorEffect(ProgramFactory& factory, EVector4  const& color);

    // Member access.
    inline void SetPVWMatrix(EMatrix4x4  const& pvwMatrix);
	inline EMatrix4x4  const& GetPVWMatrix() const;

	inline void SetColor(const EVector4 &color);

    // Required to bind and update resources.
    inline std::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;
    inline std::shared_ptr<ConstantBuffer> const& GetColorConstant() const;

private:
    // Vertex shader parameters.
    std::shared_ptr<ConstantBuffer> mPVWMatrixConstant;
    std::shared_ptr<ConstantBuffer> mColorConstant;

    // Convenience pointers.
	EMatrix4x4 *mPVWMatrix;
    EVector4	  *mColor;

    // Shader source code as strings.
    static std::string const msGLSLVSSource;
    static std::string const msGLSLPSSource;
    static std::string const msHLSLSource;
    static std::string const* msVSSource[ProgramFactory::PF_NUM_API];
    static std::string const* msPSSource[ProgramFactory::PF_NUM_API];
};


inline void ConstantColorEffect::SetPVWMatrix(
    EMatrix4x4 const& pvwMatrix)
{
    *mPVWMatrix = pvwMatrix;
}

inline EMatrix4x4  const& ConstantColorEffect::GetPVWMatrix() const
{
    return *mPVWMatrix;
}

void ConstantColorEffect::SetColor(const EVector4 &color)
{
	*mColor = color;
}

inline std::shared_ptr<ConstantBuffer> const&
ConstantColorEffect::GetPVWMatrixConstant() const
{
    return mPVWMatrixConstant;
}

inline std::shared_ptr<ConstantBuffer> const&
ConstantColorEffect::GetColorConstant() const
{
    return mColorConstant;
}


}
