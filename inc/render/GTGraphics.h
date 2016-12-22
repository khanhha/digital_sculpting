// David Eberly, Geometric Tools, Redmond WA 98052
// Copyright (c) 1998-2016
// Distributed under the Boost Software License, Version 1.0.
// http://www.boost.org/LICENSE_1_0.txt
// http://www.geometrictools.com/License/Boost/LICENSE_1_0.txt
// File Version: 2.1.0 (2016/01/25)

#pragma once

// Abstract bases class for DX11 and OpenGL.
#include <Graphics/GteGEDrawTarget.h>
#include <Graphics/GteGEInputLayoutManager.h>
#include <Graphics/GteGEObject.h>
#include <Graphics/GteGraphicsEngine.h>

// Effects
#include <Graphics/GteAmbientLightEffect.h>
#include <Graphics/GteConstantColorEffect.h>
#include <Graphics/GteDirectionalLightEffect.h>
#include <Graphics/GteDirectionalLightTextureEffect.h>
#include <Graphics/GteFont.h>
#include <Graphics/GteFontArialW400H18.h>
#include <Graphics/GteLightCameraGeometry.h>
#include <Graphics/GteLighting.h>
#include <Graphics/GteLightingEffect.h>
#include <Graphics/GteMaterial.h>
#include <Graphics/GteOverlayEffect.h>
#include <Graphics/GtePointLightEffect.h>
#include <Graphics/GtePointLightTextureEffect.h>
#include <Graphics/GteSpotLightEffect.h>
#include <Graphics/GteTextEffect.h>
#include <Graphics/GteTexture2Effect.h>
#include <Graphics/GteTexture2ColorEffect.h>
#include <Graphics/GteTexture3Effect.h>
#include <Graphics/GteVertexColorEffect.h>
#include <Graphics/GteVisualEffect.h>

// Resources
#include <Graphics/GteDataFormat.h>
#include <Graphics/GteGraphicsObject.h>
#include <Graphics/GteResource.h>

// Resources/Buffers
#include <Graphics/GteBuffer.h>
#include <Graphics/GteConstantBuffer.h>
#include <Graphics/GteIndexBuffer.h>
#include <Graphics/GteIndexFormat.h>
#include <Graphics/GteIndirectArgumentsBuffer.h>
#include <Graphics/GteMemberLayout.h>
#include <Graphics/GteRawBuffer.h>
#include <Graphics/GteStructuredBuffer.h>
#include <Graphics/GteTypedBuffer.h>
#include <Graphics/GteVertexBuffer.h>
#include <Graphics/GteVertexFormat.h>

// Resources/Textures
#include <Graphics/GteDrawTarget.h>
#include <Graphics/GteTexture.h>
#include <Graphics/GteTexture1.h>
#include <Graphics/GteTexture1Array.h>
#include <Graphics/GteTexture2.h>
#include <Graphics/GteTexture2Array.h>
#include <Graphics/GteTexture3.h>
#include <Graphics/GteTextureArray.h>
#include <Graphics/GteTextureBuffer.h>
#include <Graphics/GteTextureCube.h>
#include <Graphics/GteTextureCubeArray.h>
#include <Graphics/GteTextureDS.h>
#include <Graphics/GteTextureRT.h>
#include <Graphics/GteTextureSingle.h>

// Shaders
#include <Graphics/GteComputeProgram.h>
#include <Graphics/GteComputeShader.h>
#include <Graphics/GteGeometryShader.h>
#include <Graphics/GtePixelShader.h>
#include <Graphics/GteProgramDefines.h>
#include <Graphics/GteProgramFactory.h>
#include <Graphics/GteShader.h>
#include <Graphics/GteVertexShader.h>
#include <Graphics/GteVisualProgram.h>

// State
#include <Graphics/GteBlendState.h>
#include <Graphics/GteDepthStencilState.h>
#include <Graphics/GteDrawingState.h>
#include <Graphics/GteRasterizerState.h>
#include <Graphics/GteSamplerState.h>

#include <Graphics/GL4/GteGLFunction.h>
#include <Graphics/GL4/GteGL4Engine.h>
#include <Graphics/GL4/GteGLSLProgramFactory.h>
#include <Graphics/GteMeshFactory.h>
#include <Graphics/GL4/GteGL4DrawTarget.h>