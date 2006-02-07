//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
// This is what all vs/ps (dx8+) shaders inherit from.
//=============================================================================//

#ifndef BASEVSSHADER_H
#define BASEVSSHADER_H

#ifdef _WIN32		   
#pragma once
#endif

#include "shaderlib/cshader.h"
#include "shaderlib/baseshader.h"


//-----------------------------------------------------------------------------
// Standard vertex shader constants used by shaders in the std shader DLLs
//-----------------------------------------------------------------------------
enum
{
	VERTEX_SHADER_MODULATION_COLOR = 37,
};

//-----------------------------------------------------------------------------
// Helper macro for vertex shaders
//-----------------------------------------------------------------------------
#define BEGIN_VS_SHADER(name,help)	__BEGIN_SHADER_INTERNAL( CBaseVSShader, name, help )


// GR - indication of alpha usage
enum BlendType_t
{
	// no alpha blending
	BT_NONE = 0,



	// src * srcAlpha + dst * (1-srcAlpha)
	// two passes for HDR:
	//		pass 1:
	//			color: src * srcAlpha + dst * (1-srcAlpha)
	//			alpha: srcAlpha * zero + dstAlpha * (1-srcAlpha)
	//		pass 2:
	//			color: none
	//			alpha: srcAlpha * one + dstAlpha * one
	//
	BT_BLEND,


	
	// src * one + dst * one
	// one pass for HDR
	BT_ADD,


	
	// Why do we ever use this instead of using premultiplied alpha?
	// src * srcAlpha + dst * one
	// two passes for HDR
	//		pass 1:
	//			color: src * srcAlpha + dst * one
	//			alpha: srcAlpha * one + dstAlpha * one
	//		pass 2:
	//			color: none
	//			alpha: srcAlpha * one + dstAlpha * one
	BT_BLENDADD
};

//-----------------------------------------------------------------------------
// Base class for shaders, contains helper methods.
//-----------------------------------------------------------------------------
class CBaseVSShader : public CBaseShader
{
public:

protected:
	// Loads bump lightmap coordinates into the pixel shader
	void LoadBumpLightmapCoordinateAxes_PixelShader( int pixelReg );

	// Loads bump lightmap coordinates into the vertex shader
	void LoadBumpLightmapCoordinateAxes_VertexShader( int vertexReg );

	// Pixel and vertex shader constants....
	void SetPixelShaderConstant( int pixelReg, int constantVar );

	// This version will put constantVar into x,y,z, and constantVar2 into the w
	void SetPixelShaderConstant( int pixelReg, int constantVar, int constantVar2 );

	void SetVertexShaderConstant( int vertexReg, int constantVar );

	// GR - fix for const/lerp issues
	void SetPixelShaderConstantFudge( int pixelReg, int constantVar );

	// Sets light direction for pixel shaders.
	void SetPixelShaderLightColors( int pixelReg );

	// Sets vertex shader texture transforms
	void SetVertexShaderTextureTranslation( int vertexReg, int translationVar );
	void SetVertexShaderTextureScale( int vertexReg, int scaleVar );
 	void SetVertexShaderTextureTransform( int vertexReg, int transformVar );
	void SetVertexShaderTextureScaledTransform( int vertexReg, 
											int transformVar, int scaleVar );

	// Set pixel shader texture transforms
	void SetPixelShaderTextureTranslation( int pixelReg, int translationVar );
	void SetPixelShaderTextureScale( int pixelReg, int scaleVar );
 	void SetPixelShaderTextureTransform( int pixelReg, int transformVar );
	void SetPixelShaderTextureScaledTransform( int pixelReg, 
											int transformVar, int scaleVar );

	// Moves a matrix into vertex shader constants 
	void SetVertexShaderMatrix3x4( int vertexReg, int matrixVar );
	void SetVertexShaderMatrix4x4( int vertexReg, int matrixVar );

	// Loads the view matrix into pixel shader constants
	void LoadViewMatrixIntoVertexShaderConstant( int vertexReg );

	// Sets up ambient light cube...
	void SetAmbientCubeDynamicStateVertexShader( );

	// Helpers for dealing with envmaptint
	void SetEnvMapTintPixelShaderDynamicState( int pixelReg, int tintVar, int alphaVar, bool bConvertFromGammaToLinear = false );
	
	// Helper methods for pixel shader overbrighting
	void EnablePixelShaderOverbright( int reg, bool bEnable, bool bDivideByTwo );

	// Helper for dealing with modulation
	void SetModulationVertexShaderDynamicState();
	void SetModulationPixelShaderDynamicState( int modulationVar );
	void SetModulationPixelShaderDynamicState_LinearColorSpace( int modulationVar );

	//
	// Standard shader passes!
	//

	// Dx8 Unlit Generic pass
	void VertexShaderUnlitGenericPass( bool doSkin, int baseTextureVar, int frameVar, 
		int baseTextureTransformVar, int detailVar, int detailTransform, bool bDetailTransformIsScale, 
		int envmapVar, int envMapFrameVar, int envmapMaskVar, int envmapMaskFrameVar, 
		int envmapMaskScaleVar, int envmapTintVar, int alphaTestReferenceVar ) ;

	// Helpers for drawing world bump mapped stuff.
	void DrawModelBumpedSpecularLighting( int bumpMapVar, int bumpMapFrameVar,
											   int envMapVar, int envMapVarFrame,
											   int envMapTintVar, int alphaVar,
											   int envMapContrastVar, int envMapSaturationVar,
											   int bumpTransformVar,
											   bool bBlendSpecular, bool bNoWriteZ = false );
	void DrawWorldBumpedSpecularLighting( int bumpmapVar, int envmapVar,
											   int bumpFrameVar, int envmapFrameVar,
											   int envmapTintVar, int alphaVar,
											   int envmapContrastVar, int envmapSaturationVar,
											   int bumpTransformVar, int fresnelReflectionVar,
											   bool bBlend, bool bNoWriteZ = false );
	const char *UnlitGeneric_ComputeVertexShaderName( bool bMask,
														  bool bEnvmap,
														  bool bBaseTexture,
														  bool bBaseAlphaEnvmapMask,
														  bool bDetail,
														  bool bVertexColor,
														  bool bEnvmapCameraSpace,
														  bool bEnvmapSphere );
	const char *UnlitGeneric_ComputePixelShaderName( bool bMask,
														  bool bEnvmap,
														  bool bBaseTexture,
														  bool bBaseAlphaEnvmapMask,
														  bool bDetail );
	void DrawWorldBaseTexture( int baseTextureVar, int baseTextureTransformVar, int frameVar );
	void DrawWorldBumpedDiffuseLighting( int bumpmapVar, int bumpFrameVar,
		int bumpTransformVar, bool bMultiply );
	void DrawWorldBumpedSpecularLighting( int envmapMaskVar, int envmapMaskFrame,
		int bumpmapVar, int envmapVar,
		int bumpFrameVar, int envmapFrameVar,
		int envmapTintVar, int alphaVar,
		int envmapContrastVar, int envmapSaturationVar,
		int bumpTransformVar,  int fresnelReflectionVar,
		bool bBlend );
	void DrawBaseTextureBlend( int baseTextureVar, int baseTextureTransformVar, 
		int baseTextureFrameVar,
		int baseTexture2Var, int baseTextureTransform2Var, 
		int baseTextureFrame2Var );
	void DrawWorldBumpedDiffuseLighting_Base_ps14( int bumpmapVar, int bumpFrameVar,
		int bumpTransformVar, int baseTextureVar, int baseTextureTransformVar, int frameVar );
	void DrawWorldBumpedDiffuseLighting_Blend_ps14( int bumpmapVar, int bumpFrameVar, int bumpTransformVar, 
		int baseTextureVar, int baseTextureTransformVar, int baseTextureFrameVar, 
		int baseTexture2Var, int baseTextureTransform2Var, int baseTextureFrame2Var);
	void DrawWorldBumpedUsingVertexShader( int baseTextureVar, int baseTextureTransformVar,
		int bumpmapVar, int bumpFrameVar, 
		int bumpTransformVar,
		int envmapMaskVar, int envmapMaskFrame,
		int envmapVar, 
		int envmapFrameVar,
		int envmapTintVar, int alphaVar,
		int envmapContrastVar, int envmapSaturationVar, int frameVar, int fresnelReflectionVar,
		bool doBaseTexture2 = false,
		int baseTexture2Var = -1, int baseTextureTransform2Var = -1,
		int baseTextureFrame2Var = -1 
		);

	void InitParamsVertexLitAndUnlitGeneric_DX9( 
		const char *pMaterialName,
		bool bVertexLitGeneric,
		int baseTextureVar, int frameVar,			int baseTextureTransformVar, 
		int selfIllumTintVar,
		int detailVar, 		int detailFrameVar, 	int detailScaleVar, 
		int envmapVar, 		int envmapFrameVar, 	
		int envmapMaskVar, 	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar, 
		int bumpmapVar,    	int bumpFrameVar,		int bumpTransformVar,
		int envmapContrastVar, int envmapSaturationVar, int alphaTestReferenceVar, 
		int parallaxMapVar, int parallaxMapScaleVar, int parallaxMapBiasVar, 
		int albedoVar, int flashlightTextureVar, int flashlightTextureFrameVar );

	void InitVertexLitAndUnlitGeneric_DX9( 
		const char *pMaterialName,
		bool bVertexLitGeneric,
		int baseTextureVar,	int frameVar,			int baseTextureTransformVar, 
		int selfIllumTintVar,
		int detailVar, 		int detailFrameVar, 	int detailScaleVar, 
		int envmapVar, 		int envmapFrameVar, 
		int envmapMaskVar, 	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar, 
		int bumpmapVar,    	int bumpFrameVar,		int bumpTransformVar,
		int envmapContrastVar, int envmapSaturationVar, int alphaTestReferenceVar, 
		int parallaxMapVar, int parallaxMapScaleVar, int flashlightTextureVar, 
		int flashlightTextureFrameVar );

	void DrawVertexLitAndUnlitGeneric_DX9( 
		bool bVertexLitGeneric,
		int baseTextureVar,	int frameVar,			int baseTextureTransformVar, 
		int selfIllumTintVar,
		int detailVar,		int detailFrameVar,		int detailScaleVar, 
		int envmapVar,		int envmapFrameVar, 
		int envmapMaskVar, 	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar, 
		int bumpmapVar, 	int bumpFrameVar,		int bumpTransformVar,
		int envmapContrastVar, int envmapSaturationVar, int alphaTestReferenceVar, int parallaxMapVar, 
		int parallaxMapScaleVar, int parallaxMapBiasVar,
		int flashlightTextureVar, int flashlightTextureFrameVar );

	void InitParamsUnlitGeneric_DX9(
		const char *pMaterialName,
		int baseTextureVar, int frameVar,			int baseTextureTransformVar, 
		int detailVar,		int detailFrameVar,		int detailScaleVar, 
		int envmapVar,		int envmapFrameVar,  
		int envmapMaskVar,	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar,	int envmapContrastVar,	int envmapSaturationVar,
		int alphaTestReferenceVar, int flashlightTextureVar, int flashlightTextureFrameVar  );

	void InitUnlitGeneric_DX9( 
		const char *pMaterialName,
		int baseTextureVar, int frameVar,			int baseTextureTransformVar, 
		int detailVar,		int detailFrameVar,		int detailScaleVar, 
		int envmapVar,		int envmapFrameVar, 
		int envmapMaskVar,	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar,	int envmapContrastVar,	int envmapSaturationVar,
		int alphaTestReferenceVar, int flashlightTextureVar, int flashlightTextureFrameVar );

	void DrawUnlitGeneric_DX9( 
		int baseTextureVar, int frameVar,			int baseTextureTransformVar, 
		int detailVar,		int detailFrameVar,		int detailScaleVar, 
		int envmapVar,		int envmapFrameVar, 
		int envmapMaskVar,	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar,	int envmapContrastVar,	int envmapSaturationVar,
		int alphaTestReferenceVar, int flashlightTextureVar, int flashlightTextureFrameVar );

	// HDR-related methods
	void InitParamsVertexLitAndUnlitGeneric_HDR(
		const char *pMaterialName,
		bool bVertexLitGeneric,													 
		int baseTextureVar, int frameVar,			int baseTextureTransformVar,
		int selfIllumTintVar,
		int brightnessTextureVar,
		int detailVar,		int detailFrameVar,		int detailScaleVar, 
		int envmapVar,		int envmapFrameVar, 
		int envmapMaskVar,	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar,	
		int bumpmapVar,     int bumpFrameVar,		int bumpTransformVar,
		int envmapContrastVar,	int envmapSaturationVar );

	void InitVertexLitAndUnlitGeneric_HDR(
		const char *pMaterialName,
		bool bVertexLitGeneric,													 
		int baseTextureVar, int frameVar,			int baseTextureTransformVar,
		int selfIllumTintVar,
		int brightnessTextureVar,
		int detailVar,		int detailFrameVar,		int detailScaleVar, 
		int envmapVar,		int envmapFrameVar, 
		int envmapMaskVar,	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar,	
		int bumpmapVar,     int bumpFrameVar,		int bumpTransformVar,
		int envmapContrastVar,	int envmapSaturationVar );

	void InitUnlitGeneric_HDR( 
		const char *pMaterialName,
		int baseTextureVar, int frameVar,			int baseTextureTransformVar, 
		int brightnessTextureVar,
		int detailVar,		int detailFrameVar,		int detailScaleVar, 
		int envmapVar,		int envmapFrameVar, 
		int envmapMaskVar,	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar,	int envmapContrastVar,	int envmapSaturationVar );

	void DrawVertexLitAndUnlitGeneric_HDR( 
		bool bVertexLitGeneric,
		int baseTextureVar,	int frameVar,			int baseTextureTransformVar, 
		int selfIllumTintVar,
		int brightnessTextureVar,
		int detailVar,		int detailFrameVar,		int detailScaleVar, 
		int envmapVar,		int envmapFrameVar, 
		int envmapMaskVar, 	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar, 
		int bumpmapVar, 	int bumpFrameVar,		int bumpTransformVar,
		int envmapContrastVar, int envmapSaturationVar );

	void DrawUnlitGeneric_HDR( 
		int baseTextureVar,	int frameVar,			int baseTextureTransformVar, 
		int brightnessTextureVar,
		int detailVar,		int detailFrameVar, 	int detailScaleVar, 
		int envmapVar, 		int envmapFrameVar, 
		int envmapMaskVar, 	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar, 
		int envmapContrastVar, int envmapSaturationVar );
	
	// Computes the shader index for vertex lit materials
	int ComputeVertexLitShaderIndex( bool bVertexLitGeneric, bool hasBump, bool hasEnvmap, bool hasVertexColor, bool bHasNormal ) const;

	BlendType_t EvaluateBlendRequirements( int textureVar, bool isBaseTexture );

	// Helper for setting up flashlight constants
	void SetFlashlightVertexShaderConstants( bool bBump, int bumpTransformVar, bool bSetTextureTransforms );

	void DrawFlashlight_dx80( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bBump,
										int bumpmapVar, int bumpmapFrame, int bumpTransform, int flashlightTextureVar, int flashlightTextureFrameVar,
										bool bLightmappedGeneric, bool bWorldVertexTransition, int nWorldVertexTransitionPassID, int baseTexture2Var,
										int baseTexture2FrameVar );
	void DrawFlashlight_dx90( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bBump,
										int bumpmapVar, int bumpmapFrame, int bumpTransform, int flashlightTextureVar, int flashlightTextureFrameVar,
										bool bLightmappedGeneric, bool bWorldVertexTransition, int baseTexture2Var,
										int baseTexture2FrameVar, int bumpmap2Var, int bumpmap2Frame );
	void SetWaterFogColorPixelShaderConstantLinear( int reg );
	void SetWaterFogColorPixelShaderConstantGamma( int reg );
private:
	// Helper methods for VertexLitGenericPass
	void UnlitGenericShadowState( int baseTextureVar, int detailVar, int envmapVar, int envmapMaskVar, bool doSkin );
	void UnlitGenericDynamicState( int baseTextureVar, int frameVar, int baseTextureTransformVar,
		int detailVar, int detailTransform, bool bDetailTransformIsScale, int envmapVar, 
		int envMapFrameVar, int envmapMaskVar, int envmapMaskFrameVar,
		int envmapMaskScaleVar, int envmapTintVar );

	void DrawVertexLitAndUnlitGenericPass_HDR( 
		bool bVertexLitGeneric,
		int baseTextureVar,	int frameVar,			int baseTextureTransformVar, 
		int selfIllumTintVar,
		int brightnessTextureVar,
		int detailVar,		int detailFrameVar, 	int detailScaleVar, 
		int envmapVar, 		int envmapFrameVar, 
		int envmapMaskVar, 	int envmapMaskFrameVar, int envmapMaskTransformVar,
		int envmapTintVar, 
		int bumpmapVar, 	int bumpFrameVar,		int bumpTransformVar,
		int envmapContrastVar, int envmapSaturationVar,
		BlendType_t blendType, bool alphaTest, int pass );
};



#endif // BASEVSSHADER_H
