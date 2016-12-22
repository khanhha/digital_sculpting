#pragma once

#include <Graphics/GteVisualEffect.h>
#include <Graphics/GteLightingEffect.h>
#include "Graphics/GtePointLightEffect.h"
#include <GTEngineDEF.h>

namespace gte
{
	class GTE_IMPEXP WireframeEffect : public VisualEffect
	{
	public:
		struct WireParam
		{
			EVector4 edgeColor;
			EVector4 winSize;
		};

		// Construction.
		WireframeEffect(ProgramFactory& factory, BufferUpdater const& updater, std::shared_ptr<Material> const& material, std::shared_ptr<Lighting> const& lighting, std::shared_ptr<LightCameraGeometry> const& geometry);

		// Member access.
		inline void SetMaterial(std::shared_ptr<Material> const& material);
		inline void SetLighting(std::shared_ptr<Lighting> const& lighting);
		inline void SetGeometry(std::shared_ptr<LightCameraGeometry> const& geometry);

		inline std::shared_ptr<Material> const& GetMaterial() const;
		inline std::shared_ptr<Lighting> const& GetLighting() const;
		inline std::shared_ptr<LightCameraGeometry> const& GetGeometry() const;
		inline std::shared_ptr<ConstantBuffer> const& GetPVWMatrixConstant() const;
		inline std::shared_ptr<ConstantBuffer> const& GetMaterialConstant() const;
		inline std::shared_ptr<ConstantBuffer> const& GetLightingConstant() const;
		inline std::shared_ptr<ConstantBuffer> const& GetGeometryConstant() const;

		inline void setWireframeWindow(size_t width, size_t height);
		inline void setWireframeEdgeColor(const EVector4 &color);
		// After you set or modify 'material', 'light', or 'geometry', call the update
		// to inform any listener that the corresponding constant buffer has changed.
		// The derived classes construct the constant buffers to store the minimal
		// information from Material, Light, or Camera.  The pvw-matrix constant update
		// requires knowledge of the world transform of the object to which the effect
		// is attached, so its update must occur outside of this class.  Derived
		// classes update the system memory of the constant buffers and the base class
		// updates video memory.
		virtual void UpdateMaterialConstant();
		virtual void UpdateLightingConstant();
		virtual void UpdateGeometryConstant();
		virtual void UpdatePVWMatrix(const EMatrix4x4 &mat);
		void		 UpdateWireframeParams();
	protected:
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
			EVector4  lightModelPosition;
			EVector4  cameraModelPosition;
		};

		std::shared_ptr<Material> mMaterial;
		std::shared_ptr<Lighting> mLighting;
		std::shared_ptr<LightCameraGeometry> mGeometry;

		std::shared_ptr<ConstantBuffer> mPVWMatrixConstant;

		// The derived-class constructors are responsible for creating these
		// according to their needs.
		std::shared_ptr<ConstantBuffer> mMaterialConstant;
		std::shared_ptr<ConstantBuffer> mLightingConstant;
		std::shared_ptr<ConstantBuffer> mGeometryConstant;

		// HLSL has a shader intrinsic lit() function.
		// This inline string of code reproduces that function for GLSL.
		// Static method used here because this string needs to be generated
		// before code (which may also be in global initializers) tries to use it.
		static std::string GetShaderSourceLitFunctionGLSL();

	private:
		// Vertex shader parameters.
		std::shared_ptr<ConstantBuffer> mWireParameters;
		WireParam *mWireParametersData;

		// Convenience pointers.
		EMatrix4x4 *mPVWMatrix;

		// Shader source code as strings.
		static std::string const msGLSLVSSource;
		static std::string const msGLSLGSSource;
		static std::string const msGLSLPSSource;
		static std::string const msHLSLSource;
		static std::string const* msVSSource[ProgramFactory::PF_NUM_API];
		static std::string const* msPSSource[ProgramFactory::PF_NUM_API];
	};

	inline void WireframeEffect::SetMaterial(std::shared_ptr<Material> const& material)
	{
		mMaterial = material;
	}

	inline void WireframeEffect::SetLighting(std::shared_ptr<Lighting> const& lighting)
	{
		mLighting = lighting;
	}

	inline void WireframeEffect::SetGeometry(std::shared_ptr<LightCameraGeometry> const& geometry)
	{
		mGeometry = geometry;
	}

	inline std::shared_ptr<Material> const& WireframeEffect::GetMaterial() const
	{
		return mMaterial;
	}

	inline std::shared_ptr<Lighting> const& WireframeEffect::GetLighting() const
	{
		return mLighting;
	}

	inline std::shared_ptr<LightCameraGeometry> const& WireframeEffect::GetGeometry() const
	{
		return mGeometry;
	}

	inline std::shared_ptr<ConstantBuffer> const& WireframeEffect::GetPVWMatrixConstant() const
	{
		return mPVWMatrixConstant;
	}

	inline std::shared_ptr<ConstantBuffer> const& WireframeEffect::GetMaterialConstant() const
	{
		return mMaterialConstant;
	}

	inline std::shared_ptr<ConstantBuffer> const& WireframeEffect::GetLightingConstant() const
	{
		return mLightingConstant;
	}

	inline std::shared_ptr<ConstantBuffer> const& WireframeEffect::GetGeometryConstant() const
	{
		return mGeometryConstant;
	}

	inline void WireframeEffect::setWireframeWindow(size_t width, size_t height)
	{
		mWireParametersData->winSize = EVector4(width, height, 0.0f, 0.0f);
	}

	inline void WireframeEffect::setWireframeEdgeColor(const EVector4 &color)
	{
		mWireParametersData->edgeColor = color;
	}
}
