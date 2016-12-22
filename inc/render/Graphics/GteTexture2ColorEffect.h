// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

#include <Graphics/GteTexture2.h>
#include <Graphics/GteVisualEffect.h>
#include <GTEngineDEF.h>


namespace gte
{

	class GTE_IMPEXP Texture2ColorEffect : public VisualEffect
	{
	public:
		// Constructionn.
		Texture2ColorEffect(ProgramFactory& factory,
			std::shared_ptr<Texture2> const& texture, SamplerState::Filter filter,
			SamplerState::Mode mode0, SamplerState::Mode mode1);

		// Member access.
		inline void SetPVWMatrix(EMatrix4x4  const& pvwMatrix);
		inline EMatrix4x4  const& GetPVWMatrix() const;
		inline void SetFactor(float factor);
		inline float const &GetFactor();

		// Required to bind and update resources.
		inline std::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;
		inline std::shared_ptr<Texture2> const& GetTexture() const;
		inline std::shared_ptr<SamplerState> const& GetSampler() const;

	private:
		// Vertex shader parameters.
		std::shared_ptr<ConstantBuffer> mPVWMatrixConstant;
		std::shared_ptr<ConstantBuffer> mFactorConstant;

		// Pixel shader parameters.
		std::shared_ptr<Texture2> mTexture;
		std::shared_ptr<SamplerState> mSampler;

		// Convenience pointer.
		EMatrix4x4 *mPVWMatrix;
		float	   *mFactor;

		// Shader source code as strings.
		static std::string const msGLSLVSSource;
		static std::string const msGLSLPSSource;
		static std::string const msHLSLSource;
		static std::string const* msVSSource[ProgramFactory::PF_NUM_API];
		static std::string const* msPSSource[ProgramFactory::PF_NUM_API];
	};


	inline void Texture2ColorEffect::SetPVWMatrix(EMatrix4x4  const& pvwMatrix)
	{
		*mPVWMatrix = pvwMatrix;
	}

	inline EMatrix4x4  const& Texture2ColorEffect::GetPVWMatrix() const
	{
		return *mPVWMatrix;
	}

	inline std::shared_ptr<ConstantBuffer> const&
		Texture2ColorEffect::GetPVWMatrixConstant() const
	{
		return mPVWMatrixConstant;
	}

	inline std::shared_ptr<Texture2> const& Texture2ColorEffect::GetTexture() const
	{
		return mTexture;
	}

	inline std::shared_ptr<SamplerState> const& Texture2ColorEffect::GetSampler() const
	{
		return mSampler;
	}

	void Texture2ColorEffect::SetFactor(float factor)
	{
		*mFactor = factor;
	}

	float const& Texture2ColorEffect::GetFactor()
	{
		return *mFactor;
	}


}
