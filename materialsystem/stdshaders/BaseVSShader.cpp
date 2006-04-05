//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
// This is what all vs/ps (dx8+) shaders inherit from.
//=============================================================================//

#include "basevsshader.h"
#include "vmatrix.h"
#include "bumpvects.h"

#include "ConVar.h"

#ifdef HDR
#include "vertexlit_and_unlit_generic_hdr_ps20.inc"
#endif

#include "ff_lightmappedgeneric_flashlight_vs11.inc"
#include "ff_lightmappedgeneric_flashlight_vs20.inc"
#include "ff_flashlight_ps11.inc"
#include "ff_flashlight_ps20.inc"
#include "ff_unlitgeneric_vs11.inc"
#include "ff_VertexLitGeneric_EnvmappedBumpmap_NoLighting_ps14.inc"
#include "ff_VertexLitGeneric_EnvmappedBumpmap_NoLighting.inc"
#include "ff_vertexlitgeneric_flashlight_vs11.inc"
#include "ff_lightmappedgeneric_basetexture.inc"
#include "ff_lightmappedGeneric_bumpmappedlightmap_base_ps14.inc"
#include "ff_lightmappedGeneric_bumpmappedlightmap_blend_ps14.inc"
#include "ff_lightmappedgeneric_bumpmappedenvmap_ps14.inc"
#include "ff_lightmappedgeneric_bumpmappedenvmap.inc"
#include "ff_lightmappedgeneric_basetextureblend.inc"
#include "ff_lightmappedgeneric_bumpmappedlightmap.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"



// These functions are to be called from the shaders.

//-----------------------------------------------------------------------------
// Pixel and vertex shader constants....
//-----------------------------------------------------------------------------
void CBaseVSShader::SetPixelShaderConstant( int pixelReg, int constantVar, int constantVar2 )
{
	Assert( !IsSnapshotting() );
	if ((!s_ppParams) || (constantVar == -1) || (constantVar2 == -1))
		return;

	IMaterialVar* pPixelVar = s_ppParams[constantVar];
	Assert( pPixelVar );
	IMaterialVar* pPixelVar2 = s_ppParams[constantVar2];
	Assert( pPixelVar2 );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
	{
		pPixelVar->GetVecValue( val, 3 );
	}
	else
	{
		val[0] = val[1] = val[2] = pPixelVar->GetFloatValue();
	}

	val[3] = pPixelVar2->GetFloatValue();
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetPixelShaderConstant( int pixelReg, int constantVar )
{
	Assert( !IsSnapshotting() );
	if ((!s_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pPixelVar = s_ppParams[constantVar];
	Assert( pPixelVar );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		pPixelVar->GetVecValue( val, 4 );
	else
		val[0] = val[1] = val[2] = val[3] = pPixelVar->GetFloatValue();
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

// GR - special version with fix for const/lerp issue
void CBaseVSShader::SetPixelShaderConstantFudge( int pixelReg, int constantVar )
{
	Assert( !IsSnapshotting() );
	if ((!s_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pPixelVar = s_ppParams[constantVar];
	Assert( pPixelVar );

	float val[4];
	if (pPixelVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
	{
		pPixelVar->GetVecValue( val, 4 );
		val[0] = val[0] * 0.992f + 0.0078f;
		val[1] = val[1] * 0.992f + 0.0078f;
		val[2] = val[2] * 0.992f + 0.0078f;
		val[3] = val[3] * 0.992f + 0.0078f;
	}
	else
		val[0] = val[1] = val[2] = val[3] = pPixelVar->GetFloatValue() * 0.992f + 0.0078f;
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, val );	
}

void CBaseVSShader::SetVertexShaderConstant( int vertexReg, int constantVar )
{
	Assert( !IsSnapshotting() );
	if ((!s_ppParams) || (constantVar == -1))
		return;

	IMaterialVar* pVertexVar = s_ppParams[constantVar];
	Assert( pVertexVar );

	float val[4];
	if (pVertexVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
		pVertexVar->GetVecValue( val, 4 );
	else
		val[0] = val[1] = val[2] = val[3] = pVertexVar->GetFloatValue();
	s_pShaderAPI->SetVertexShaderConstant( vertexReg, val );	
}

//-----------------------------------------------------------------------------
// Sets normalized light color for pixel shaders.
//-----------------------------------------------------------------------------
void CBaseVSShader::SetPixelShaderLightColors( int pixelReg )
{
	int i;
	int maxLights = s_pShaderAPI->GetMaxLights();
	for( i = 0; i < maxLights; i++ )
	{
		const LightDesc_t & lightDesc = s_pShaderAPI->GetLight( i );
		if( lightDesc.m_Type != MATERIAL_LIGHT_DISABLE )
		{
			Vector color( lightDesc.m_Color[0], lightDesc.m_Color[1], lightDesc.m_Color[2] );
			VectorNormalize( color );
			float val[4] = { color[0], color[1], color[2], 1.0f };
			s_pShaderAPI->SetPixelShaderConstant( pixelReg + i, val, 1 );
		}
		else
		{
			float zero[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
			s_pShaderAPI->SetPixelShaderConstant( pixelReg + i, zero, 1 );
		}
	}
}


//-----------------------------------------------------------------------------
// Sets vertex shader texture transforms
//-----------------------------------------------------------------------------
void CBaseVSShader::SetVertexShaderTextureTranslation( int vertexReg, int translationVar )
{
	float offset[2] = {0, 0};

	IMaterialVar* pTranslationVar = s_ppParams[translationVar];
	if (pTranslationVar)
	{
		if (pTranslationVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pTranslationVar->GetVecValue( offset, 2 );
		else
			offset[0] = offset[1] = pTranslationVar->GetFloatValue();
	}

	Vector4D translation[2];
	translation[0].Init( 1.0f, 0.0f, 0.0f, offset[0] );
	translation[1].Init( 0.0f, 1.0f, 0.0f, offset[1] );
	s_pShaderAPI->SetVertexShaderConstant( vertexReg, translation[0].Base(), 2 ); 
}

void CBaseVSShader::SetVertexShaderTextureScale( int vertexReg, int scaleVar )
{
	float scale[2] = {1, 1};

	IMaterialVar* pScaleVar = s_ppParams[scaleVar];
	if (pScaleVar)
	{
		if (pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pScaleVar->GetVecValue( scale, 2 );
		else if (pScaleVar->IsDefined())
			scale[0] = scale[1] = pScaleVar->GetFloatValue();
	}

	Vector4D scaleMatrix[2];
	scaleMatrix[0].Init( scale[0], 0.0f, 0.0f, 0.0f );
	scaleMatrix[1].Init( 0.0f, scale[1], 0.0f, 0.0f );
	s_pShaderAPI->SetVertexShaderConstant( vertexReg, scaleMatrix[0].Base(), 2 ); 
}

void CBaseVSShader::SetVertexShaderTextureTransform( int vertexReg, int transformVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = s_ppParams[transformVar];
	if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
	{
		const VMatrix &mat = pTransformationVar->GetMatrixValue();
		transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
		transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
	}
	else
	{
		transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
		transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
	}
	s_pShaderAPI->SetVertexShaderConstant( vertexReg, transformation[0].Base(), 2 ); 
}

void CBaseVSShader::SetVertexShaderTextureScaledTransform( int vertexReg, int transformVar, int scaleVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = s_ppParams[transformVar];
	if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
	{
		const VMatrix &mat = pTransformationVar->GetMatrixValue();
		transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
		transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
	}
	else
	{
		transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
		transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
	}

	Vector2D scale( 1, 1 );
	IMaterialVar* pScaleVar = s_ppParams[scaleVar];
	if (pScaleVar)
	{
		if (pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pScaleVar->GetVecValue( scale.Base(), 2 );
		else if (pScaleVar->IsDefined())
			scale[0] = scale[1] = pScaleVar->GetFloatValue();
	}

	// Apply the scaling
	transformation[0][0] *= scale[0];
	transformation[0][1] *= scale[1];
	transformation[1][0] *= scale[0];
	transformation[1][1] *= scale[1];
	transformation[0][3] *= scale[0];
	transformation[1][3] *= scale[1];
	s_pShaderAPI->SetVertexShaderConstant( vertexReg, transformation[0].Base(), 2 ); 
}


//-----------------------------------------------------------------------------
// Sets pixel shader texture transforms
//-----------------------------------------------------------------------------
void CBaseVSShader::SetPixelShaderTextureTranslation( int pixelReg, int translationVar )
{
	float offset[2] = {0, 0};

	IMaterialVar* pTranslationVar = s_ppParams[translationVar];
	if (pTranslationVar)
	{
		if (pTranslationVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pTranslationVar->GetVecValue( offset, 2 );
		else
			offset[0] = offset[1] = pTranslationVar->GetFloatValue();
	}

	Vector4D translation[2];
	translation[0].Init( 1.0f, 0.0f, 0.0f, offset[0] );
	translation[1].Init( 0.0f, 1.0f, 0.0f, offset[1] );
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, translation[0].Base(), 2 ); 
}

void CBaseVSShader::SetPixelShaderTextureScale( int pixelReg, int scaleVar )
{
	float scale[2] = {1, 1};

	IMaterialVar* pScaleVar = s_ppParams[scaleVar];
	if (pScaleVar)
	{
		if (pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pScaleVar->GetVecValue( scale, 2 );
		else if (pScaleVar->IsDefined())
			scale[0] = scale[1] = pScaleVar->GetFloatValue();
	}

	Vector4D scaleMatrix[2];
	scaleMatrix[0].Init( scale[0], 0.0f, 0.0f, 0.0f );
	scaleMatrix[1].Init( 0.0f, scale[1], 0.0f, 0.0f );
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, scaleMatrix[0].Base(), 2 ); 
}

void CBaseVSShader::SetPixelShaderTextureTransform( int pixelReg, int transformVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = s_ppParams[transformVar];
	if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
	{
		const VMatrix &mat = pTransformationVar->GetMatrixValue();
		transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
		transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
	}
	else
	{
		transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
		transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
	}
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, transformation[0].Base(), 2 ); 
}

void CBaseVSShader::SetPixelShaderTextureScaledTransform( int pixelReg, int transformVar, int scaleVar )
{
	Vector4D transformation[2];
	IMaterialVar* pTransformationVar = s_ppParams[transformVar];
	if (pTransformationVar && (pTransformationVar->GetType() == MATERIAL_VAR_TYPE_MATRIX))
	{
		const VMatrix &mat = pTransformationVar->GetMatrixValue();
		transformation[0].Init( mat[0][0], mat[0][1], mat[0][2], mat[0][3] );
		transformation[1].Init( mat[1][0], mat[1][1], mat[1][2], mat[1][3] );
	}
	else
	{
		transformation[0].Init( 1.0f, 0.0f, 0.0f, 0.0f );
		transformation[1].Init( 0.0f, 1.0f, 0.0f, 0.0f );
	}

	Vector2D scale( 1, 1 );
	IMaterialVar* pScaleVar = s_ppParams[scaleVar];
	if (pScaleVar)
	{
		if (pScaleVar->GetType() == MATERIAL_VAR_TYPE_VECTOR)
			pScaleVar->GetVecValue( scale.Base(), 2 );
		else if (pScaleVar->IsDefined())
			scale[0] = scale[1] = pScaleVar->GetFloatValue();
	}

	// Apply the scaling
	transformation[0][0] *= scale[0];
	transformation[0][1] *= scale[1];
	transformation[1][0] *= scale[0];
	transformation[1][1] *= scale[1];
	transformation[0][3] *= scale[0];
	transformation[1][3] *= scale[1];
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, transformation[0].Base(), 2 ); 
}


//-----------------------------------------------------------------------------
// Moves a matrix into vertex shader constants 
//-----------------------------------------------------------------------------
void CBaseVSShader::SetVertexShaderMatrix3x4( int vertexReg, int matrixVar )
{
	IMaterialVar* pTranslationVar = s_ppParams[matrixVar];
	if (pTranslationVar)
	{
		s_pShaderAPI->SetVertexShaderConstant( vertexReg, &pTranslationVar->GetMatrixValue( )[0][0], 3 ); 
	}
	else
	{
		VMatrix matrix;
		MatrixSetIdentity( matrix );
		s_pShaderAPI->SetVertexShaderConstant( vertexReg, &matrix[0][0], 3 ); 
	}
}

void CBaseVSShader::SetVertexShaderMatrix4x4( int vertexReg, int matrixVar )
{
	IMaterialVar* pTranslationVar = s_ppParams[matrixVar];
	if (pTranslationVar)
	{
		s_pShaderAPI->SetVertexShaderConstant( vertexReg, &pTranslationVar->GetMatrixValue( )[0][0], 4 ); 
	}
	else
	{
		VMatrix matrix;
		MatrixSetIdentity( matrix );
		s_pShaderAPI->SetVertexShaderConstant( vertexReg, &matrix[0][0], 4 ); 
	}
}


//-----------------------------------------------------------------------------
// Loads the view matrix into pixel shader constants
//-----------------------------------------------------------------------------
void CBaseVSShader::LoadViewMatrixIntoVertexShaderConstant( int vertexReg )
{
	VMatrix mat, transpose;
	s_pShaderAPI->GetMatrix( MATERIAL_VIEW, mat.m[0] );

	MatrixTranspose( mat, transpose );
	s_pShaderAPI->SetVertexShaderConstant( vertexReg, transpose.m[0], 3 );
}


//-----------------------------------------------------------------------------
// Loads bump lightmap coordinates into the pixel shader
//-----------------------------------------------------------------------------
void CBaseVSShader::LoadBumpLightmapCoordinateAxes_PixelShader( int pixelReg )
{
	Vector4D basis[3];
	for (int i = 0; i < 3; ++i)
	{
		memcpy( &basis[i], &g_localBumpBasis[i], 3 * sizeof(float) );
		basis[i][3] = 0.0f;
	}
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, (float*)basis, 3 );
}


//-----------------------------------------------------------------------------
// Loads bump lightmap coordinates into the pixel shader
//-----------------------------------------------------------------------------
void CBaseVSShader::LoadBumpLightmapCoordinateAxes_VertexShader( int vertexReg )
{
	Vector4D basis[3];

	// transpose
	int i;
	for (i = 0; i < 3; ++i)
	{
		basis[i][0] = g_localBumpBasis[0][i];
		basis[i][1] = g_localBumpBasis[1][i];
		basis[i][2] = g_localBumpBasis[2][i];
		basis[i][3] = 0.0f;
	}
	s_pShaderAPI->SetVertexShaderConstant( vertexReg, (float*)basis, 3 );
	for (i = 0; i < 3; ++i)
	{
		memcpy( &basis[i], &g_localBumpBasis[i], 3 * sizeof(float) );
		basis[i][3] = 0.0f;
	}
	s_pShaderAPI->SetVertexShaderConstant( vertexReg + 3, (float*)basis, 3 );
}


//-----------------------------------------------------------------------------
// Helper methods for pixel shader overbrighting
//-----------------------------------------------------------------------------
void CBaseVSShader::EnablePixelShaderOverbright( int reg, bool bEnable, bool bDivideByTwo )
{
	// can't have other overbright values with pixel shaders as it stands.
	float v[4];
	if( bEnable )
	{
		v[0] = v[1] = v[2] = v[3] = bDivideByTwo ? OVERBRIGHT / 2.0f : OVERBRIGHT;
	}
	else
	{
		v[0] = v[1] = v[2] = v[3] = bDivideByTwo ? 1.0f / 2.0f : 1.0f;
	}
	s_pShaderAPI->SetPixelShaderConstant( reg, v, 1 );
}


//-----------------------------------------------------------------------------
// Helper for dealing with modulation
//-----------------------------------------------------------------------------
void CBaseVSShader::SetModulationVertexShaderDynamicState()
{
 	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	ComputeModulationColor( color );
	s_pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_MODULATION_COLOR, color );
}

void CBaseVSShader::SetModulationPixelShaderDynamicState( int modulationVar )
{
	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	ComputeModulationColor( color );
	s_pShaderAPI->SetPixelShaderConstant( modulationVar, color );
}

void CBaseVSShader::SetModulationPixelShaderDynamicState_LinearColorSpace( int modulationVar )
{
	float color[4] = { 1.0, 1.0, 1.0, 1.0 };
	ComputeModulationColor( color );
	color[0] = GammaToLinear( color[0] );
	color[1] = GammaToLinear( color[1] );
	color[2] = GammaToLinear( color[2] );
	s_pShaderAPI->SetPixelShaderConstant( modulationVar, color );
}


//-----------------------------------------------------------------------------
// Helpers for dealing with envmap tint
//-----------------------------------------------------------------------------
// set alphaVar to -1 to ignore it.
void CBaseVSShader::SetEnvMapTintPixelShaderDynamicState( int pixelReg, int tintVar, int alphaVar, bool bConvertFromGammaToLinear )
{
	float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	if( g_pConfig->bShowSpecular )
	{
		IMaterialVar* pAlphaVar = NULL;
		if( alphaVar >= 0 )
		{
			pAlphaVar = s_ppParams[alphaVar];
		}
		if( pAlphaVar )
		{
			color[3] = pAlphaVar->GetFloatValue();
		}

		IMaterialVar* pTintVar = s_ppParams[tintVar];
		if( bConvertFromGammaToLinear )
		{
			pTintVar->GetLinearVecValue( color, 3 );
		}
		else
		{
			pTintVar->GetVecValue( color, 3 );
		}
	}
	else
	{
		color[0] = color[1] = color[2] = color[3] = 0.0f;
	}
	s_pShaderAPI->SetPixelShaderConstant( pixelReg, color, 1 );
}

void CBaseVSShader::SetAmbientCubeDynamicStateVertexShader( )
{
	s_pShaderAPI->SetVertexShaderStateAmbientLightCube();
}

const char *CBaseVSShader::UnlitGeneric_ComputePixelShaderName( bool bMask,
														  bool bEnvmap,
														  bool bBaseTexture,
														  bool bBaseAlphaEnvmapMask,
														  bool bDetail )
{
	static char const* s_pPixelShaders[] = 
	{
		"UnlitGeneric_NoTexture",
		"UnlitGeneric",
		"UnlitGeneric_EnvMapNoTexture",
		"UnlitGeneric_EnvMap",
		"UnlitGeneric_NoTexture",
		"UnlitGeneric",
		"UnlitGeneric_EnvMapMaskNoTexture",
		"UnlitGeneric_EnvMapMask",

		// Detail texture
		// The other commented-out versions are used if we want to
		// apply the detail *after* the environment map is added
		"UnlitGeneric_DetailNoTexture",
		"UnlitGeneric_Detail",
		"UnlitGeneric_EnvMapNoTexture", //"UnlitGeneric_DetailEnvMapNoTexture",
		"UnlitGeneric_DetailEnvMap",
		"UnlitGeneric_DetailNoTexture",
		"UnlitGeneric_Detail",
		"UnlitGeneric_EnvMapMaskNoTexture", //"UnlitGeneric_DetailEnvMapMaskNoTexture",
		"UnlitGeneric_DetailEnvMapMask",
	};

	if (!bMask && bEnvmap && bBaseTexture && bBaseAlphaEnvmapMask)
	{
		if (!bDetail)
			return "UnlitGeneric_BaseAlphaMaskedEnvMap";
		else
			return "UnlitGeneric_DetailBaseAlphaMaskedEnvMap";
	}
	else
	{
		int pshIndex = 0;
		if (bBaseTexture)
			pshIndex |= 0x1;
		if (bEnvmap)
			pshIndex |= 0x2;
		if (bMask)
			pshIndex |= 0x4;
		if (bDetail)
			pshIndex |= 0x8;
		return s_pPixelShaders[pshIndex];
	}
}

//-----------------------------------------------------------------------------
// Vertex shader unlit generic pass
//-----------------------------------------------------------------------------
void CBaseVSShader::VertexShaderUnlitGenericPass( bool doSkin, int baseTextureVar, int frameVar, 
	int baseTextureTransformVar, int detailVar, int detailTransform, bool bDetailTransformIsScale,
	int envmapVar, int envMapFrameVar, int envmapMaskVar, int envmapMaskFrameVar, 
	int envmapMaskScaleVar, int envmapTintVar, int alphaTestReferenceVar )
{
	IMaterialVar** params = s_ppParams;

	bool bBaseAlphaEnvmapMask = IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	bool bEnvmap = (envmapVar >= 0) && params[envmapVar]->IsTexture();
	bool bMask = false;
	if (bEnvmap)
	{
		bMask = params[envmapMaskVar]->IsTexture();
	}
	bool bDetail = (detailVar >= 0) && params[detailVar]->IsTexture(); 
	bool bBaseTexture = params[baseTextureVar]->IsTexture();
	bool bVertexColor = IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR);
	bool bEnvmapCameraSpace = IS_FLAG_SET(MATERIAL_VAR_ENVMAPCAMERASPACE);
	bool bEnvmapSphere = IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE);

	if (IsSnapshotting())
	{
		// Alpha test
		s_pShaderShadow->EnableAlphaTest( IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) );

		if( alphaTestReferenceVar != -1 && params[alphaTestReferenceVar]->GetFloatValue() > 0.0f )
		{
			s_pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[alphaTestReferenceVar]->GetFloatValue() );
		}

		// Base texture on stage 0
		if (params[baseTextureVar]->IsTexture())
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
		}

		if ((detailVar >= 0) && params[detailVar]->IsTexture())
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
		}

		bool bEnvMapDefined = (envmapVar >= 0) && params[envmapVar]->IsTexture();
		if (bEnvMapDefined)
		{
			// envmap on stage 1
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );

			// envmapmask on stage 2
			if (params[envmapMaskVar]->IsTexture() || bBaseAlphaEnvmapMask )
				s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
		}

		if (params[baseTextureVar]->IsTexture())
			SetDefaultBlendingShadowState( baseTextureVar, true );
		else
			SetDefaultBlendingShadowState( envmapMaskVar, false );

		int fmt = VERTEX_POSITION;
		if( bEnvMapDefined )
			fmt |= VERTEX_NORMAL;
		if (doSkin)
			fmt |= VERTEX_BONE_INDEX;
		if (bVertexColor)
			fmt |= VERTEX_COLOR;

		s_pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, doSkin ? 3 : 0, 0 );
		const char *pshName = UnlitGeneric_ComputePixelShaderName(
			bMask,
			bEnvmap,
			bBaseTexture,
			bBaseAlphaEnvmapMask,
			bDetail );
		s_pShaderShadow->SetPixelShader( pshName );

		// Compute the vertex shader index.
		ff_unlitgeneric_vs11_Static_Index vshIndex;
		vshIndex.SetDETAIL( bDetail );
		vshIndex.SetENVMAP( bEnvmap );
		vshIndex.SetENVMAPCAMERASPACE( bEnvmap && bEnvmapCameraSpace );
		vshIndex.SetENVMAPSPHERE( bEnvmap && bEnvmapSphere );
		vshIndex.SetVERTEXCOLOR( bVertexColor );
		s_pShaderShadow->SetVertexShader( "FF_unlitgeneric_vs11", vshIndex.GetIndex() );

		DefaultFog();
	}
	else
	{
		if (bBaseTexture)
		{
			BindTexture( SHADER_TEXTURE_STAGE0, baseTextureVar, frameVar );
			SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, baseTextureTransformVar );
		}

		if (bDetail)
		{
			BindTexture( SHADER_TEXTURE_STAGE3, detailVar, frameVar );

			if (bDetailTransformIsScale)
			{
				SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, baseTextureTransformVar, detailTransform );
			}
			else
			{
				SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, detailTransform );
			}
		}

		if (bEnvmap)
		{
			BindTexture( SHADER_TEXTURE_STAGE1, envmapVar, envMapFrameVar );

			if (bMask || bBaseAlphaEnvmapMask)
			{
				if (bMask)
					BindTexture( SHADER_TEXTURE_STAGE2, envmapMaskVar, envmapMaskFrameVar );
				else
					BindTexture( SHADER_TEXTURE_STAGE2, baseTextureVar, frameVar );

				SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, baseTextureTransformVar, envmapMaskScaleVar );
			}

			SetEnvMapTintPixelShaderDynamicState( 2, envmapTintVar, -1 );

			if (bEnvmapSphere || IS_FLAG_SET(MATERIAL_VAR_ENVMAPCAMERASPACE))
			{
				LoadViewMatrixIntoVertexShaderConstant( VERTEX_SHADER_VIEWMODEL );
			}
		}

		SetModulationVertexShaderDynamicState();

		// Compute the vertex shader index.
		ff_unlitgeneric_vs11_Dynamic_Index vshIndex;
		vshIndex.SetFOG_TYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
		vshIndex.SetNUM_BONES( s_pShaderAPI->GetCurrentNumBones() );
		s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
	}

	Draw();
}


void CBaseVSShader::DrawWorldBaseTexture( int baseTextureVar, int baseTextureTransformVar,
									int frameVar )
{
	if( IsSnapshotting() )
	{
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
		s_pShaderShadow->VertexShaderVertexFormat( 
			VERTEX_POSITION, 1, 0, 0, 0 );
		s_pShaderShadow->SetPixelShader( "FF_LightmappedGeneric_BaseTexture" );

		ff_lightmappedgeneric_basetexture_Static_Index vshIndex;
		s_pShaderShadow->SetVertexShader( "FF_LightmappedGeneric_BaseTexture", vshIndex.GetIndex() );

		FogToOOOverbright();
	}
	else
	{
		BindTexture( SHADER_TEXTURE_STAGE0, baseTextureVar, frameVar );
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, baseTextureTransformVar );

		ff_lightmappedgeneric_basetexture_Dynamic_Index vshIndex;
		vshIndex.SetFOG_TYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
		s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
	}
	Draw();
}

void CBaseVSShader::DrawWorldBumpedDiffuseLighting( int bumpmapVar, int bumpFrameVar,
											  int bumpTransformVar, bool bMultiply )
{
	if( IsSnapshotting() )
	{
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
		if( bMultiply )
		{
			s_pShaderShadow->EnableBlending( true );
			SingleTextureLightmapBlendMode();
		}
		s_pShaderShadow->VertexShaderVertexFormat( 
			VERTEX_POSITION, 3, 0, 0, 0 );

		ff_lightmappedgeneric_bumpmappedlightmap_Static_Index vshIndex;
		s_pShaderShadow->SetVertexShader( "FF_LightmappedGeneric_BumpmappedLightmap", vshIndex.GetIndex() );

		s_pShaderShadow->SetPixelShader( "FF_LightmappedGeneric_BumpmappedLightmap" );
		FogToFogColor();
	}
	else
	{
		if( !g_pConfig->m_bFastNoBump )
		{
			BindTexture( SHADER_TEXTURE_STAGE0, bumpmapVar, bumpFrameVar );
		}
		else
		{
			s_pShaderAPI->BindFlatNormalMap( SHADER_TEXTURE_STAGE0 );
		}
		LoadBumpLightmapCoordinateAxes_PixelShader( 0 );
		s_pShaderAPI->BindBumpLightmap( SHADER_TEXTURE_STAGE1 );
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, bumpTransformVar );
		SetModulationPixelShaderDynamicState( 3 );

		ff_lightmappedgeneric_bumpmappedlightmap_Dynamic_Index vshIndex;
		vshIndex.SetFOG_TYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
		s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
	}
	Draw();
}

void CBaseVSShader::DrawWorldBumpedDiffuseLighting_Base_ps14( int bumpmapVar, int bumpFrameVar,
											  int bumpTransformVar, 
											  int baseTextureVar, int baseTextureTransformVar, int frameVar )
{
	if( IsSnapshotting() )
	{
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE4, true );
		s_pShaderShadow->VertexShaderVertexFormat( 
			VERTEX_POSITION, 3, 0, 0, 0 );

		ff_lightmappedgeneric_bumpmappedlightmap_base_ps14_Static_Index vshIndex;
		s_pShaderShadow->SetVertexShader( "FF_LightmappedGeneric_BumpmappedLightmap_Base_ps14", vshIndex.GetIndex() );

		s_pShaderShadow->SetPixelShader( "FF_LightmappedGeneric_BumpmappedLightmap_Base_ps14" );
		FogToFogColor();
	}
	else
	{
		if( !g_pConfig->m_bFastNoBump )
		{
			BindTexture( SHADER_TEXTURE_STAGE0, bumpmapVar, bumpFrameVar );
		}
		else
		{
			s_pShaderAPI->BindFlatNormalMap( SHADER_TEXTURE_STAGE0 );
		}
		LoadBumpLightmapCoordinateAxes_PixelShader( 0 );
		s_pShaderAPI->BindBumpLightmap( SHADER_TEXTURE_STAGE1 );
		BindTexture( SHADER_TEXTURE_STAGE4, baseTextureVar, frameVar );
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, bumpTransformVar );
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, baseTextureTransformVar );
		SetModulationPixelShaderDynamicState( 3 );

		ff_lightmappedgeneric_bumpmappedlightmap_base_ps14_Dynamic_Index vshIndex;
		vshIndex.SetFOG_TYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
		s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
	}
	Draw();
}

void CBaseVSShader::DrawWorldBumpedDiffuseLighting_Blend_ps14( int bumpmapVar, int bumpFrameVar,
											int bumpTransformVar,
											int baseTextureVar, int baseTextureTransformVar, 
											int baseTextureFrameVar,
											int baseTexture2Var, int baseTextureTransform2Var, 
											int baseTextureFrame2Var)
{
	if( IsSnapshotting() )
	{
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE4, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE5, true );
		s_pShaderShadow->VertexShaderVertexFormat( 
			VERTEX_POSITION, 3, 0, 0, 0 );

		ff_lightmappedgeneric_bumpmappedlightmap_blend_ps14_Static_Index vshIndex;
		s_pShaderShadow->SetVertexShader( "FF_LightmappedGeneric_BumpmappedLightmap_Blend_ps14", vshIndex.GetIndex() );

		s_pShaderShadow->SetPixelShader( "FF_LightmappedGeneric_BumpmappedLightmap_Blend_ps14" );
		FogToFogColor();
	}
	else
	{
		if( !g_pConfig->m_bFastNoBump )
		{
			BindTexture( SHADER_TEXTURE_STAGE0, bumpmapVar, bumpFrameVar );
		}
		else
		{
			s_pShaderAPI->BindFlatNormalMap( SHADER_TEXTURE_STAGE0 );
		}
		LoadBumpLightmapCoordinateAxes_PixelShader( 0 );
		s_pShaderAPI->BindBumpLightmap( SHADER_TEXTURE_STAGE1 );
		BindTexture( SHADER_TEXTURE_STAGE4, baseTextureVar, baseTextureFrameVar );
		BindTexture( SHADER_TEXTURE_STAGE5, baseTexture2Var, baseTextureFrame2Var );
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, bumpTransformVar );
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, baseTextureTransformVar );
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, baseTextureTransform2Var );
		SetModulationPixelShaderDynamicState( 3 );

		ff_lightmappedgeneric_bumpmappedlightmap_blend_ps14_Dynamic_Index vshIndex;
		vshIndex.SetFOG_TYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
		s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
	}
	Draw();
}

//#define USE_DEST_ALPHA
#define USE_NORMALMAP_ALPHA

void CBaseVSShader::DrawWorldBumpedSpecularLighting( int bumpmapVar, int envmapVar,
											   int bumpFrameVar, int envmapFrameVar,
											   int envmapTintVar, int alphaVar,
											   int envmapContrastVar, int envmapSaturationVar,
											   int bumpTransformVar, int fresnelReflectionVar,
											   bool bBlend, bool bNoWriteZ )
{
	// + BUMPED CUBEMAP
	if( IsSnapshotting() )
	{
		SetInitialShadowState( );
		if ( bNoWriteZ )
		{
			s_pShaderShadow->EnableDepthWrites( false );
		}
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
		if( g_pHardwareConfig->SupportsPixelShaders_1_4() )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE4, true );
		}
		if( bBlend )
		{
			s_pShaderShadow->EnableBlending( true );
			s_pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE );
		}
		// FIXME: Remove the normal (needed for tangent space gen)
		s_pShaderShadow->VertexShaderVertexFormat( 
			VERTEX_POSITION | VERTEX_NORMAL | VERTEX_TANGENT_S |
			VERTEX_TANGENT_T, 1, 0, 0, 0 );

		IMaterialVar** params = s_ppParams;
		bool bHasNormalMapAlphaEnvMapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
		if( g_pHardwareConfig->SupportsPixelShaders_1_4() )
		{
			ff_lightmappedgeneric_bumpmappedenvmap_ps14_Static_Index vshIndex;
			s_pShaderShadow->SetVertexShader( "FF_LightmappedGeneric_BumpmappedEnvmap_ps14", vshIndex.GetIndex() );

			int nPshIndex = bHasNormalMapAlphaEnvMapMask ? 1 : 0;
			s_pShaderShadow->SetPixelShader( "FF_LightmappedGeneric_BumpmappedEnvmap_ps14", nPshIndex );
		}
		else
		{
			ff_lightmappedgeneric_bumpmappedenvmap_Static_Index vshIndex;
			s_pShaderShadow->SetVertexShader( "FF_LightmappedGeneric_BumpmappedEnvmap", vshIndex.GetIndex() );

			int nPshIndex = bHasNormalMapAlphaEnvMapMask ? 1 : 0;
			s_pShaderShadow->SetPixelShader( "FF_LightmappedGeneric_BumpmappedEnvmap", nPshIndex );
		}
		FogToBlack();
	}
	else
	{
		IMaterialVar** params = s_ppParams;
		s_pShaderAPI->SetDefaultState();
		BindTexture( SHADER_TEXTURE_STAGE0, bumpmapVar, bumpFrameVar );
		BindTexture( SHADER_TEXTURE_STAGE3, envmapVar, envmapFrameVar );
		if( g_pHardwareConfig->SupportsPixelShaders_1_4() )
		{
			s_pShaderAPI->BindNormalizationCubeMap( SHADER_TEXTURE_STAGE4 );

			ff_lightmappedgeneric_bumpmappedenvmap_ps14_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
		else
		{
			ff_lightmappedgeneric_bumpmappedenvmap_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
				
		SetEnvMapTintPixelShaderDynamicState( 0, envmapTintVar, alphaVar );
		// GR - fudge consts a bit to fix const/lerp issues
		SetPixelShaderConstantFudge( 1, envmapContrastVar );
		SetPixelShaderConstantFudge( 2, envmapSaturationVar );
		float greyWeights[4] = { 0.299f, 0.587f, 0.114f, 0.0f };
		s_pShaderAPI->SetPixelShaderConstant( 3, greyWeights );

		// [ 0, 0 ,0, R(0) ]
		float fresnel[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		fresnel[3] = params[fresnelReflectionVar]->GetFloatValue();
		s_pShaderAPI->SetPixelShaderConstant( 4, fresnel );
		// [ 0, 0 ,0, 1-R(0) ]
		fresnel[3] = 1.0f - fresnel[3];
		s_pShaderAPI->SetPixelShaderConstant( 6, fresnel );

		float one[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
		s_pShaderAPI->SetPixelShaderConstant( 5, one );
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, bumpTransformVar );
	}
	Draw();
}

void CBaseVSShader::DrawModelBumpedSpecularLighting( int bumpMapVar, int bumpMapFrameVar,
											   int envMapVar, int envMapVarFrame,
											   int envMapTintVar, int alphaVar,
											   int envMapContrastVar, int envMapSaturationVar,
											   int bumpTransformVar,
											   bool bBlendSpecular, bool bNoWriteZ )
{
	IMaterialVar** params = s_ppParams;
	
	if( IsSnapshotting() )
	{
		SetInitialShadowState( );
		if ( bNoWriteZ )
		{
			s_pShaderShadow->EnableDepthWrites( false );
		}
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
		if( g_pHardwareConfig->SupportsPixelShaders_1_4() )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE4, true );
		}
		s_pShaderShadow->EnableAlphaTest( false );
		if( bBlendSpecular )
		{
			s_pShaderShadow->EnableBlending( true );
			SetAdditiveBlendingShadowState( -1, false );
		}
		else
		{
			s_pShaderShadow->EnableBlending( false );
			SetNormalBlendingShadowState( -1, false );
		}
		
		int numLights = 0;
		
		s_pShaderShadow->VertexShaderVertexFormat( 
			VERTEX_POSITION | VERTEX_NORMAL | VERTEX_BONE_INDEX, 
			1, 0, numLights, 4 /* userDataSize */ );

		bool bHasNormalMapAlphaEnvMapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
		if( g_pHardwareConfig->SupportsPixelShaders_1_4() )
		{
			ff_vertexlitgeneric_envmappedbumpmap_nolighting_ps14_Static_Index vshIndex;
			s_pShaderShadow->SetVertexShader( "FF_VertexLitGeneric_EnvmappedBumpmap_NoLighting_ps14", vshIndex.GetIndex() );
			if( bHasNormalMapAlphaEnvMapMask )
			{
				s_pShaderShadow->SetPixelShader( "FF_VertexLitGeneric_EnvmappedBumpmapV2_MultByAlpha_ps14" );
			}
			else
			{
				s_pShaderShadow->SetPixelShader( "FF_VertexLitGeneric_EnvmappedBumpmapV2_ps14" );
			}
		}
		else
		{
			ff_vertexlitgeneric_envmappedbumpmap_nolighting_Static_Index vshIndex;
			s_pShaderShadow->SetVertexShader( "FF_VertexLitGeneric_EnvmappedBumpmap_NoLighting", vshIndex.GetIndex() );
			// This version does not multiply by lighting.
			// NOTE: We don't support multiplying by lighting for bumped specular stuff.
			if( bHasNormalMapAlphaEnvMapMask )
			{
				s_pShaderShadow->SetPixelShader( "FF_VertexLitGeneric_EnvmappedBumpmapV2_MultByAlpha" );
			}
			else
			{
				s_pShaderShadow->SetPixelShader( "FF_VertexLitGeneric_EnvmappedBumpmapV2" );
			}
		}
		FogToBlack();
	}
	else
	{
		s_pShaderAPI->SetDefaultState();
		BindTexture( SHADER_TEXTURE_STAGE0, bumpMapVar, bumpMapFrameVar );
		BindTexture( SHADER_TEXTURE_STAGE3, envMapVar, envMapVarFrame );
		if( g_pHardwareConfig->SupportsPixelShaders_1_4() )
		{
			s_pShaderAPI->BindNormalizationCubeMap( SHADER_TEXTURE_STAGE4 );
		}
				
		if( bBlendSpecular )
		{
			SetEnvMapTintPixelShaderDynamicState( 0, envMapTintVar, -1 );
		}
		else
		{
			SetEnvMapTintPixelShaderDynamicState( 0, envMapTintVar, alphaVar );
		}
		// GR - fudge consts a bit to fix const/lerp issues
		SetPixelShaderConstantFudge( 1, envMapContrastVar );
		SetPixelShaderConstantFudge( 2, envMapSaturationVar );
		float greyWeights[4] = { 0.299f, 0.587f, 0.114f, 0.0f };
		s_pShaderAPI->SetPixelShaderConstant( 3, greyWeights );
		
		// handle scrolling of bump texture
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, bumpTransformVar );

		if( g_pHardwareConfig->SupportsPixelShaders_1_4() )
		{
			ff_vertexlitgeneric_envmappedbumpmap_nolighting_ps14_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			vshIndex.SetNUM_BONES( s_pShaderAPI->GetCurrentNumBones() );
			s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
		else
		{
			ff_vertexlitgeneric_envmappedbumpmap_nolighting_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			vshIndex.SetNUM_BONES( s_pShaderAPI->GetCurrentNumBones() );
			s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
	}
	Draw();
}

void CBaseVSShader::DrawBaseTextureBlend( int baseTextureVar, int baseTextureTransformVar, 
									 int baseTextureFrameVar,
									 int baseTexture2Var, int baseTextureTransform2Var, 
									 int baseTextureFrame2Var )
{
	if( IsSnapshotting() )
	{
		SetInitialShadowState();
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );
		s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
		s_pShaderShadow->DrawFlags( SHADER_DRAW_POSITION | SHADER_DRAW_TEXCOORD0 |
			SHADER_DRAW_LIGHTMAP_TEXCOORD1 );
		// FIXME: Remove the normal (needed for tangent space gen)
		s_pShaderShadow->VertexShaderVertexFormat( 
			VERTEX_POSITION, 2, 0, 0, 0 );

		ff_lightmappedgeneric_basetextureblend_Static_Index vshIndex;
		s_pShaderShadow->SetVertexShader( "FF_lightmappedgeneric_basetextureblend", vshIndex.GetIndex() );

		s_pShaderShadow->SetPixelShader( "FF_lightmappedgeneric_basetextureblend", 0 );
		FogToOOOverbright();
	}
	else
	{
		s_pShaderAPI->SetDefaultState();
		BindTexture( SHADER_TEXTURE_STAGE0, baseTextureVar, baseTextureFrameVar );
		BindTexture( SHADER_TEXTURE_STAGE1, baseTexture2Var, baseTextureFrame2Var );
		s_pShaderAPI->BindLightmap( SHADER_TEXTURE_STAGE2 );
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, baseTextureTransformVar );
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, baseTextureTransform2Var );

		ff_lightmappedgeneric_basetextureblend_Dynamic_Index vshIndex;
		vshIndex.SetFOG_TYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
		s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
	}
	Draw();
}

void CBaseVSShader::DrawWorldBumpedUsingVertexShader( int baseTextureVar, int baseTextureTransformVar,
												int bumpmapVar, int bumpFrameVar, 
												int bumpTransformVar,
												int envmapMaskVar, int envmapMaskFrame,
												int envmapVar, 
												int envmapFrameVar,
												int envmapTintVar, int alphaVar,
												int envmapContrastVar, int envmapSaturationVar,
												int frameVar, int fresnelReflectionVar,
												bool doBaseTexture2,
												int baseTexture2Var, int baseTextureTransform2Var,
												int baseTextureFrame2Var
												)
{
	IMaterialVar** params = s_ppParams;
	// Draw base texture
	bool bMultiplyDiffuseLighting = false;
	bool bBlendSpecular = false;
#if 0
	if( g_pHardwareConfig->SupportsPixelShaders_1_4() && params[baseTextureVar]->IsTexture() )
	{
		if(	doBaseTexture2 && params[baseTexture2Var]->IsTexture() )
		{
			// draw blend( base, base2 ) * lighting
			DrawWorldBumpedDiffuseLighting_Blend_ps14( bumpmapVar, bumpFrameVar, bumpTransformVar, 
				baseTextureVar, baseTextureTransformVar, frameVar,
				baseTexture2Var, baseTextureTransform2Var, baseTextureFrame2Var );
			bBlendSpecular = true;
		}
		else
		{
			// draw base * lighting
			DrawWorldBumpedDiffuseLighting_Base_ps14( bumpmapVar, bumpFrameVar, bumpTransformVar, 
				baseTextureVar, baseTextureTransformVar, frameVar );
			bBlendSpecular = true;
		}
	}
	else 
	{
		if( params[baseTextureVar]->IsTexture() )
		{
			if( doBaseTexture2 && params[baseTexture2Var]->IsTexture() )
			{
				// draw blend( base, base2 )
				DrawBaseTextureBlend( baseTextureVar, baseTextureTransformVar, frameVar,
					baseTexture2Var, baseTextureTransform2Var, baseTextureFrame2Var );
				bMultiplyDiffuseLighting = true;
				bBlendSpecular = true;
			}
			else
			{
				// draw base
				DrawWorldBaseTexture( baseTextureVar, baseTextureTransformVar, frameVar );
				bMultiplyDiffuseLighting = true;
				bBlendSpecular = true;
			}
		}
		
		// Draw diffuse lighting (handles the lighting only case as well)
		if( params[baseTextureVar]->IsTexture() || !params[envmapVar]->IsTexture() )
		{
			DrawWorldBumpedDiffuseLighting( bumpmapVar, bumpFrameVar, bumpTransformVar, 
				bMultiplyDiffuseLighting );
			bBlendSpecular = true;
		}
	}
#else		
	if( doBaseTexture2 && params[baseTexture2Var]->IsTexture() && params[baseTextureVar]->IsTexture() )
	{
		DrawBaseTextureBlend( baseTextureVar, baseTextureTransformVar, frameVar,
			baseTexture2Var, baseTextureTransform2Var, baseTextureFrame2Var );
		bMultiplyDiffuseLighting = true;
		bBlendSpecular = true;
	}
	else if( params[baseTextureVar]->IsTexture() )
	{
		DrawWorldBaseTexture( baseTextureVar, baseTextureTransformVar, frameVar );
		bMultiplyDiffuseLighting = true;
		bBlendSpecular = true;
	}
	
	// Draw diffuse lighting
	if( params[baseTextureVar]->IsTexture() || !params[envmapVar]->IsTexture() )
	{
		DrawWorldBumpedDiffuseLighting( bumpmapVar, bumpFrameVar, bumpTransformVar, 
			bMultiplyDiffuseLighting );
		bBlendSpecular = true;
	}
#endif		
	// Add specular lighting
	if( params[envmapVar]->IsTexture() )
	{
		DrawWorldBumpedSpecularLighting(
			bumpmapVar, envmapVar,
			bumpFrameVar, envmapFrameVar,
			envmapTintVar, alphaVar,
			envmapContrastVar, envmapSaturationVar,
			bumpTransformVar, fresnelReflectionVar,
			bBlendSpecular );
	}
}

/*
//-----------------------------------------------------------------------------
// HDR-related methods lie below
//-----------------------------------------------------------------------------
void CBaseVSShader::InitParamsVertexLitAndUnlitGeneric_HDR(
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
	int envmapContrastVar,	int envmapSaturationVar  )
{
	InitParamsVertexLitAndUnlitGeneric_DX9( pMaterialName, bVertexLitGeneric, baseTextureVar, frameVar, baseTextureTransformVar,
		selfIllumTintVar, detailVar, detailFrameVar, detailScaleVar, envmapVar, envmapFrameVar, 
		envmapMaskVar,envmapMaskFrameVar, envmapMaskTransformVar, envmapTintVar,
		bumpmapVar, bumpFrameVar, bumpTransformVar, envmapContrastVar, envmapSaturationVar, -1, -1, -1, -1, -1, -1, -1, -1 );
}
*/
/*
void CBaseVSShader::InitVertexLitAndUnlitGeneric_HDR(
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
	int envmapContrastVar,	int envmapSaturationVar )
{
	InitVertexLitAndUnlitGeneric_DX9( pMaterialName, bVertexLitGeneric, baseTextureVar, frameVar, baseTextureTransformVar,
		selfIllumTintVar, detailVar, detailFrameVar, detailScaleVar, envmapVar, envmapFrameVar, 
		envmapMaskVar,envmapMaskFrameVar, envmapMaskTransformVar, envmapTintVar,
		bumpmapVar, bumpFrameVar, bumpTransformVar, envmapContrastVar, envmapSaturationVar, -1, -1, -1 );

	IMaterialVar** params = s_ppParams;
	if ( ( brightnessTextureVar != -1 ) && params[brightnessTextureVar]->IsDefined() )
	{
		LoadTexture( brightnessTextureVar );
	}
}
*/

/*
void CBaseVSShader::InitUnlitGeneric_HDR( 
	const char *pMaterialName,
	int baseTextureVar, int frameVar,			int baseTextureTransformVar, 
	int brightnessTextureVar,
	int detailVar,		int detailFrameVar,		int detailScaleVar, 
	int envmapVar,		int envmapFrameVar, 
	int envmapMaskVar,	int envmapMaskFrameVar, int envmapMaskTransformVar,
	int envmapTintVar,	int envmapContrastVar,	int envmapSaturationVar )
{
	InitVertexLitAndUnlitGeneric_HDR( pMaterialName, false, baseTextureVar, frameVar, baseTextureTransformVar,
		-1, brightnessTextureVar, 
		detailVar, detailFrameVar, detailScaleVar, 
		envmapVar, envmapFrameVar, 
		envmapMaskVar,envmapMaskFrameVar, envmapMaskTransformVar, 
		envmapTintVar,
		-1, -1, -1, envmapContrastVar, envmapSaturationVar );
}
*/

/*
void CBaseVSShader::DrawVertexLitAndUnlitGenericPass_HDR( bool bVertexLitGeneric,
		int baseTextureVar, 
		int frameVar, 
		int baseTextureTransformVar, 
		int selfIllumTintVar,
		int brightnessTextureVar,
		int detailVar, 
		int detailFrameVar, 
		int detailScaleVar, 
		int envmapVar, 
		int envmapFrameVar, 
		int envmapMaskVar, 
		int envmapMaskFrameVar, 
		int envmapMaskTransformVar,
		int envmapTintVar, 
		int bumpmapVar, 
		int bumpFrameVar,
		int bumpTransformVar,
		int envmapContrastVar,
		int envmapSaturationVar,
		BlendType_t blendType,
		bool alphaTest,
		int pass )
{
#ifdef HDR
	IMaterialVar** params = s_ppParams;
	bool hasBaseTexture = params[baseTextureVar]->IsTexture();
	bool hasBrightnessTexture = (brightnessTextureVar != -1) && params[brightnessTextureVar]->IsTexture();
	bool hasDetailTexture = params[detailVar]->IsTexture();
	bool hasBump = (bumpmapVar != -1) && params[bumpmapVar]->IsTexture();
	bool hasEnvmap = params[envmapVar]->IsTexture();
	bool hasDiffuseLighting = bVertexLitGeneric;
	bool hasEnvmapMask = params[envmapMaskVar]->IsTexture();
	bool hasBaseAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	bool hasSelfIllum = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM );
	bool hasNormalMapAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
	bool hasVertexColor = bVertexLitGeneric ? false : IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR );
	bool hasVertexAlpha = bVertexLitGeneric ? false : IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA );
	bool bHasNormal = bVertexLitGeneric || hasEnvmap;
	if( ( hasDetailTexture && hasBump ) ||
		( hasEnvmapMask && hasBump ) ||
		( hasNormalMapAlphaEnvmapMask && hasBaseAlphaEnvmapMask ) ||
		( hasNormalMapAlphaEnvmapMask && hasEnvmapMask ) ||
		( hasBaseAlphaEnvmapMask && hasEnvmapMask ) ||
		( hasBaseAlphaEnvmapMask && hasSelfIllum ) )
	{
		// illegal combos
		Assert( 0 );
	}

	bool bNeedsAlpha = false;

	if( alphaTest )
	{
		// Shader needs alpha for alpha testing
		bNeedsAlpha = true;
	}
	else
	{
		// When blending shader needs alpha on the first pass
		if (((blendType == BT_BLEND) || (blendType == BT_BLENDADD)) && (pass == 0))
			bNeedsAlpha = true;
	}

	// Check if rendering alpha (second pass of blending)
	bool bRenderAlpha;
	if (((blendType == BT_BLEND) || (blendType == BT_BLENDADD)) && (pass == 1))
		bRenderAlpha = true;
	else
		bRenderAlpha = false;

	int vshIndex = 0;
	if( IsSnapshotting() )
	{
		SetInitialShadowState();

		// Alpha test: FIXME: shouldn't this be handled in Shader_t::SetInitialShadowState
		s_pShaderShadow->EnableAlphaTest( IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) );

		if (params[baseTextureVar]->IsTexture())
			SetDefaultBlendingShadowState( baseTextureVar, true );
		else
			SetDefaultBlendingShadowState( envmapMaskVar, false );
		
		// Alpha output
		if( !bNeedsAlpha || ( blendType == BT_BLEND ) )
			s_pShaderShadow->EnableAlphaWrites( true );

		// On second pass we just adjust alpha portion, so don't need color
		if( pass == 1 )
			s_pShaderShadow->EnableColorWrites( false );

		// For blending-type modes we need to setup separate alpha blending
		if( blendType == BT_BLEND )
		{
			s_pShaderShadow->EnableBlendingSeparateAlpha( true );

			if( pass == 0)
				s_pShaderShadow->BlendFuncSeparateAlpha( SHADER_BLEND_ZERO, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			else
				s_pShaderShadow->BlendFuncSeparateAlpha( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
		}
		else if( blendType == BT_BLENDADD )
		{
			if( pass == 1)
			{
				s_pShaderShadow->EnableBlendingSeparateAlpha( true );
				s_pShaderShadow->BlendFuncSeparateAlpha( SHADER_BLEND_ONE, SHADER_BLEND_ONE );
			}
		}

		unsigned int flags = VERTEX_POSITION;
		if( bHasNormal )
		{
			flags |= VERTEX_NORMAL;
		}

		int userDataSize = 0;
		if( hasBaseTexture )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
			s_pShaderShadow->EnableSRGBRead( SHADER_TEXTURE_STAGE0, true );
		}
		if( hasEnvmap )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );
			userDataSize = 4; // tangent S
			s_pShaderShadow->EnableSRGBRead( SHADER_TEXTURE_STAGE1, true );
		}
		if( hasDetailTexture )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
		}
		if( hasBump )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
			userDataSize = 4; // tangent S
		}
		if( hasEnvmapMask )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE4, true );
		}
		if( hasBrightnessTexture )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE5, true );
		}

		if( hasVertexColor || hasVertexAlpha )
		{
			flags |= VERTEX_COLOR;
		}
		
		// texcoord0 : base texcoord
		const int numTexCoords = 1;
		int numBoneWeights = 0;
		if( IS_FLAG_SET( MATERIAL_VAR_MODEL ) )
		{
			numBoneWeights = 3;
			flags |= VERTEX_BONE_INDEX;
		}
		
		s_pShaderShadow->VertexShaderVertexFormat( 
			flags, numTexCoords, NULL, numBoneWeights, userDataSize );

		s_pShaderShadow->SetVertexShader( "FF_vertexlit_and_unlit_generic_hdr_vs20" );

		vertexlit_and_unlit_generic_hdr_ps20_Index pshIndex;
		pshIndex.SetBASETEXTURE( hasBaseTexture );
		pshIndex.SetBRIGHTNESS( hasBrightnessTexture );
		pshIndex.SetDETAILTEXTURE( hasDetailTexture );
		pshIndex.SetBUMPMAP( hasBump );
		pshIndex.SetCUBEMAP( hasEnvmap );
		pshIndex.SetDIFFUSELIGHTING( hasDiffuseLighting );
		pshIndex.SetENVMAPMASK( hasEnvmapMask );
		pshIndex.SetBASEALPHAENVMAPMASK( hasBaseAlphaEnvmapMask );
		pshIndex.SetSELFILLUM( hasSelfIllum );
		pshIndex.SetNORMALMAPALPHAENVMAPMASK( hasNormalMapAlphaEnvmapMask );
		pshIndex.SetVERTEXCOLOR( hasVertexColor);
		pshIndex.SetVERTEXALPHA( hasVertexAlpha );
		pshIndex.SetNEEDSALPHA( bNeedsAlpha );
		pshIndex.SetRENDERALPHA( bRenderAlpha );
		s_pShaderShadow->SetPixelShader( "FF_vertexlit_and_unlit_generic_hdr_ps20", pshIndex.GetIndex() );

		s_pShaderShadow->EnableSRGBWrite( true );
		FogToFogColor();
	}
	else
	{
		s_pShaderAPI->SetDefaultState();
		if( hasBaseTexture )
		{
			BindTexture( SHADER_TEXTURE_STAGE0, baseTextureVar, frameVar );
		}
		if( hasEnvmap )
		{
			BindTexture( SHADER_TEXTURE_STAGE1, envmapVar, envmapFrameVar );
		}
		if( hasDetailTexture )
		{
			BindTexture( SHADER_TEXTURE_STAGE2, detailVar, detailFrameVar );
		}
		if( hasBump )
		{
			if( !g_pConfig->m_bFastNoBump )
			{
				BindTexture( SHADER_TEXTURE_STAGE3, bumpmapVar, bumpFrameVar );
			}
			else
			{
				s_pShaderAPI->BindFlatNormalMap( SHADER_TEXTURE_STAGE3 );
			}
		}
		if( hasEnvmapMask )
		{
			BindTexture( SHADER_TEXTURE_STAGE4, envmapMaskVar, envmapMaskFrameVar );
		}
		if( hasBrightnessTexture )
		{
			BindTexture( SHADER_TEXTURE_STAGE5, brightnessTextureVar, frameVar );
		}

		vshIndex = ComputeVertexLitShaderIndex( bVertexLitGeneric, hasBump, hasEnvmap, hasVertexColor, bHasNormal );

		s_pShaderAPI->SetVertexShaderIndex( vshIndex );
		SetPixelShaderTextureTransform( 6, baseTextureTransformVar );
		if( hasDetailTexture )
		{
			SetPixelShaderTextureScaledTransform( 8, baseTextureTransformVar, detailScaleVar );
			Assert( !hasBump );
		}
		if( hasBump )
		{
			SetPixelShaderTextureTransform( 8, bumpTransformVar );
			Assert( !hasDetailTexture );
		}
		if( hasEnvmapMask )
		{
			SetPixelShaderTextureTransform( 10, envmapMaskTransformVar );
		}

		float color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
		IMaterialVar* pTintVar = s_ppParams[envmapTintVar];
		if( g_pConfig->bShowSpecular )
		{
			pTintVar->GetVecValue( color, 3 );
			color[3] = (color[0] + color[1] + color[2]) / 3.0f;
		}
		s_pShaderAPI->SetPixelShaderConstant( 0, color, 1 );

		SetModulationPixelShaderDynamicState( 1 );
		SetPixelShaderConstant( 2, envmapContrastVar );
		SetPixelShaderConstant( 3, envmapSaturationVar );
		EnablePixelShaderOverbright( 4, true, false );
		SetPixelShaderConstant( 5, selfIllumTintVar );
		SetAmbientCubeDynamicStateVertexShader();

		float preMultHack[4];
		if (((blendType == BT_BLEND) || (blendType == BT_BLENDADD)) && (pass == 1))
		{
			// Need to pre-multiply by alpha
			preMultHack[0] = 1.0f;
			preMultHack[1] = 0.0f;
		}
		else
		{
			preMultHack[0] = 0.0f;
			preMultHack[1] = 1.0f;
		}
		s_pShaderAPI->SetPixelShaderConstant( 8, preMultHack );

#if 1
		extern ConVar building_cubemaps;
		bool bBlendableOutput = !building_cubemaps.GetBool();
		vertexlit_and_unlit_generic_hdr_ps20_Index pshIndex;
		pshIndex.SetBLENDOUTPUT( bBlendableOutput );
		s_pShaderAPI->SetPixelShaderIndex( pshIndex.GetIndex() );
#endif
	}
	Draw();
#endif // HDR
}
*/

//-----------------------------------------------------------------------------
// GR - translucency query
//-----------------------------------------------------------------------------
BlendType_t CBaseVSShader::EvaluateBlendRequirements( int textureVar, bool isBaseTexture )
{
	// Either we've got a constant modulation
	bool isTranslucent = IsAlphaModulating();

	// Or we've got a vertex alpha
	isTranslucent = isTranslucent || (CurrentMaterialVarFlags() & MATERIAL_VAR_VERTEXALPHA);

	// Or we've got a texture alpha (for blending or alpha test)
	isTranslucent = isTranslucent || ( TextureIsTranslucent( textureVar, isBaseTexture ) &&
		                               !(CurrentMaterialVarFlags() & MATERIAL_VAR_ALPHATEST ) );

	if ( CurrentMaterialVarFlags() & MATERIAL_VAR_ADDITIVE )
	{
		// Additive
		return isTranslucent ? BT_BLENDADD : BT_ADD;
	}
	else
	{
		// Normal blending
		return isTranslucent ? BT_BLEND : BT_NONE;
	}
}


/*
void CBaseVSShader::DrawVertexLitAndUnlitGeneric_HDR( 
	bool bVertexLitGeneric,
	int baseTextureVar, int frameVar, int baseTextureTransformVar, 
	int selfIllumTintVar, int brightnessTextureVar,
	int detailVar, int detailFrameVar, int detailScaleVar, 
	int envmapVar, int envmapFrameVar, 
	int envmapMaskVar, int envmapMaskFrameVar, int envmapMaskTransformVar,
	int envmapTintVar, 
	int bumpmapVar, int bumpFrameVar, int bumpTransformVar,
	int envmapContrastVar, int envmapSaturationVar )
{
	IMaterialVar** params = s_ppParams;
	bool alphaTest = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST);
	BlendType_t blendType;

	if (params[baseTextureVar]->IsTexture())
	{
		blendType = EvaluateBlendRequirements( baseTextureVar, true );
	}
	else
	{
		blendType = EvaluateBlendRequirements( envmapMaskVar, false );
	}

	DrawVertexLitAndUnlitGenericPass_HDR(
		bVertexLitGeneric,
		baseTextureVar, 
		frameVar, 
		baseTextureTransformVar, 
		selfIllumTintVar,
		brightnessTextureVar,
		detailVar, 
		detailFrameVar, 
		detailScaleVar, 
		envmapVar, 
		envmapFrameVar, 
		envmapMaskVar, 
		envmapMaskFrameVar, 
		envmapMaskTransformVar,
		envmapTintVar, 
		bumpmapVar, 
		bumpFrameVar,
		bumpTransformVar,
		envmapContrastVar,
		envmapSaturationVar,
		blendType, alphaTest, 0 );

	if(!alphaTest && ((blendType == BT_BLEND) || (blendType == BT_BLENDADD)))
	{
		// Second pass to complete 
		DrawVertexLitAndUnlitGenericPass_HDR(
			bVertexLitGeneric,
			baseTextureVar, 
			frameVar, 
			baseTextureTransformVar, 
			selfIllumTintVar,
			brightnessTextureVar,
			detailVar, 
			detailFrameVar, 
			detailScaleVar, 
			envmapVar, 
			envmapFrameVar, 
			envmapMaskVar, 
			envmapMaskFrameVar, 
			envmapMaskTransformVar,
			envmapTintVar, 
			bumpmapVar, 
			bumpFrameVar,
			bumpTransformVar,
			envmapContrastVar,
			envmapSaturationVar,
			blendType, alphaTest, 1 );
	}
}


void CBaseVSShader::DrawUnlitGeneric_HDR( 
	int baseTextureVar,	int frameVar,			int baseTextureTransformVar, 
	int brightnessTextureVar,
	int detailVar,		int detailFrameVar, 	int detailScaleVar, 
	int envmapVar, 		int envmapFrameVar, 
	int envmapMaskVar, 	int envmapMaskFrameVar, int envmapMaskTransformVar,
	int envmapTintVar, 
	int envmapContrastVar, int envmapSaturationVar )
{
	DrawVertexLitAndUnlitGeneric_HDR( false,
		baseTextureVar,		frameVar,			baseTextureTransformVar,
		-1,
		brightnessTextureVar,
		detailVar,			detailFrameVar,		detailScaleVar,
		envmapVar,			envmapFrameVar,
		envmapMaskVar,		envmapMaskFrameVar,	envmapMaskTransformVar,
		envmapTintVar,
		-1, -1, -1,
		envmapContrastVar, envmapSaturationVar );

}


*/


void CBaseVSShader::SetFlashlightVertexShaderConstants( bool bBump, int bumpTransformVar, bool bSetTextureTransforms )
{
	Assert( !IsSnapshotting() );

	VMatrix worldToTexture;
	const FlashlightState_t &flashlightState = s_pShaderAPI->GetFlashlightState( worldToTexture );

	// Set the flashlight origin
	float pos[4];
	pos[0] = flashlightState.m_vecLightOrigin[0];
	pos[1] = flashlightState.m_vecLightOrigin[1];
	pos[2] = flashlightState.m_vecLightOrigin[2];
	pos[3] = 1.0f;
	s_pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, pos, 1 );

	s_pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, worldToTexture.Base(), 4 );

	// Set the flashlight attenuation factors
	float atten[4];
	atten[0] = flashlightState.m_fConstantAtten;
	atten[1] = flashlightState.m_fLinearAtten;
	atten[2] = flashlightState.m_fQuadraticAtten;
	atten[3] = flashlightState.m_FarZ;
	s_pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_5, atten, 1 );

	if( bSetTextureTransforms )
	{
		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, BASETEXTURETRANSFORM );
		if( bBump && bumpTransformVar != -1 )
		{
			SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_8, bumpTransformVar );
		}
	}
}

void CBaseVSShader::DrawFlashlight_dx80( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bBump,
										int bumpmapVar, int bumpmapFrame, int bumpTransform, int flashlightTextureVar, int flashlightTextureFrameVar,
										bool bLightmappedGeneric, bool bWorldVertexTransition, int nWorldVertexTransitionPassID, int baseTexture2Var,
										int baseTexture2FrameVar )
{
	// FLASHLIGHTFIXME: hack . . need to fix the vertex shader so that it can deal with and without bumps for vertexlitgeneric
	if( !bLightmappedGeneric )
	{
		bBump = false;
	}
	if( pShaderShadow )
	{
		SetInitialShadowState();
		pShaderShadow->EnableDepthWrites( false );

		// Alpha test
		pShaderShadow->EnableAlphaTest( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );

		// Alpha blend
		if( bWorldVertexTransition )
		{
			// use separeate alpha blend to make sure that we aren't adding alpha from source
			if( nWorldVertexTransitionPassID == 0 )
			{
				EnableAlphaBlending( SHADER_BLEND_DST_ALPHA, SHADER_BLEND_ONE );
			}
			else
			{
				EnableAlphaBlending( SHADER_BLEND_ONE_MINUS_DST_ALPHA, SHADER_BLEND_ONE );
			}
		}
		else
		{
			SetAdditiveBlendingShadowState( BASETEXTURE, true );
		}
		
		pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
		pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );
		pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
		pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
		
		if( bLightmappedGeneric )
		{
			ff_lightmappedgeneric_flashlight_vs11_Static_Index	vshIndex;
			vshIndex.SetNORMALMAP( bBump );
			pShaderShadow->SetVertexShader( "FF_lightmappedgeneric_flashlight_vs11", vshIndex.GetIndex() );

			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
			if( bBump )
			{
				flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T;
			}
			pShaderShadow->VertexShaderVertexFormat( 
				flags, 1, 0, 0, 0 );
		}
		else
		{
			ff_vertexlitgeneric_flashlight_vs11_Static_Index vshIndex;
			pShaderShadow->SetVertexShader( "FF_vertexlitgeneric_flashlight_vs11", vshIndex.GetIndex() );

			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
			pShaderShadow->VertexShaderVertexFormat( 
				flags, 1, 0, 3, bBump ? 4 : 0 );
		}

		ff_flashlight_ps11_Static_Index	pshIndex;
		pshIndex.SetNORMALMAP( bBump );
		pShaderShadow->SetPixelShader( "FF_flashlight_ps11", pshIndex.GetIndex() );

		FogToBlack();
	}
	else
	{
		// Specify that we have XYZ texcoords that need to be divided by W before the pixel shader.
		// NOTE Tried to divide XY by Z, but doesn't work.
		// The dx9.0c runtime says that we shouldn't have a non-zero dimension when using vertex and pixel shaders.
		pShaderAPI->SetTextureTransformDimension( 0, 0, true );

		BindTexture( SHADER_TEXTURE_STAGE0, flashlightTextureVar, flashlightTextureFrameVar );
		if( bWorldVertexTransition && ( nWorldVertexTransitionPassID == 1 ) )
		{
			BindTexture( SHADER_TEXTURE_STAGE1, baseTexture2Var, baseTexture2FrameVar );
		}
		else
		{
			if( params[BASETEXTURE]->IsTexture() )
			{
				BindTexture( SHADER_TEXTURE_STAGE1, BASETEXTURE, FRAME );
			}
			else
			{
				pShaderAPI->BindGrey( SHADER_TEXTURE_STAGE1 );
			}
		}
		pShaderAPI->BindNormalizationCubeMap( SHADER_TEXTURE_STAGE2 );
		if( bBump )
		{
			BindTexture( SHADER_TEXTURE_STAGE3, bumpmapVar, bumpmapFrame );
		}
		else
		{
			pShaderAPI->BindNormalizationCubeMap( SHADER_TEXTURE_STAGE3 );
		}

		if( bLightmappedGeneric )
		{
			ff_lightmappedgeneric_flashlight_vs11_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
		else
		{
			ff_vertexlitgeneric_flashlight_vs11_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			vshIndex.SetNUM_BONES( pShaderAPI->GetCurrentNumBones() );
			pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}

		ff_flashlight_ps11_Dynamic_Index pshIndex;
		pShaderAPI->SetPixelShaderIndex( pshIndex.GetIndex() );

		SetFlashlightVertexShaderConstants( bBump, bumpTransform, true );
	}
	Draw();
}

void CBaseVSShader::DrawFlashlight_dx90( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bBump,
										int bumpmapVar, int bumpmapFrame, int bumpTransform, int flashlightTextureVar, int flashlightTextureFrameVar,
										bool bLightmappedGeneric, bool bWorldVertexTransition, int baseTexture2Var,
										int baseTexture2FrameVar, int bumpmap2Var, int bumpmap2Frame )
{
	// FLASHLIGHTFIXME: hack . . need to fix the vertex shader so that it can deal with and without bumps for vertexlitgeneric
	if( !bLightmappedGeneric )
	{
		bBump = false;
	}
	bool bBump2 = bWorldVertexTransition && bBump && bumpmap2Var != -1 && params[bumpmap2Var]->IsTexture();
	if( pShaderShadow )
	{
		SetInitialShadowState();
		pShaderShadow->EnableDepthWrites( false );
		pShaderShadow->EnableAlphaWrites( false );

		// Alpha test
		pShaderShadow->EnableAlphaTest( IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) );

		// Alpha blend
		SetAdditiveBlendingShadowState( BASETEXTURE, true );
		
		pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
		pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );
		pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
		pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
		if( bWorldVertexTransition )
		{
			// $basetexture2
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE4, true );
			// lightmap
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE5, true );
		}
		if( bBump2 )
		{
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE6, true );
		}
		
		if( bLightmappedGeneric )
		{
			ff_lightmappedgeneric_flashlight_vs20_Static_Index	vshIndex;
			vshIndex.SetWORLDVERTEXTRANSITION( bWorldVertexTransition );
			vshIndex.SetNORMALMAP( bBump );
			pShaderShadow->SetVertexShader( "FF_lightmappedgeneric_flashlight_vs20", vshIndex.GetIndex() );

			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
			if( bBump )
			{
				flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T;
			}
			int numTexCoords = 1;
			if( bWorldVertexTransition )
			{
				numTexCoords = 2; // need lightmap texcoords to get alpha.
			}
			pShaderShadow->VertexShaderVertexFormat( 
				flags, numTexCoords, 0, 0, 0 );
		}
		else
		{
			ff_vertexlitgeneric_flashlight_vs11_Static_Index vshIndex;
			pShaderShadow->SetVertexShader( "FF_vertexlitgeneric_flashlight_vs11", vshIndex.GetIndex() );

			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
			int numTexCoords = 1;
			pShaderShadow->VertexShaderVertexFormat( 
				flags, numTexCoords, 0, 3, bBump ? 4 : 0 );
		}

		ff_flashlight_ps20_Static_Index	pshIndex;
		pshIndex.SetNORMALMAP( bBump );
		pshIndex.SetNORMALMAP2( bBump2 );
		pshIndex.SetWORLDVERTEXTRANSITION( bWorldVertexTransition );
		pShaderShadow->SetPixelShader( "FF_flashlight_ps20", pshIndex.GetIndex() );

		FogToBlack();
	}
	else
	{
		BindTexture( SHADER_TEXTURE_STAGE0, flashlightTextureVar, flashlightTextureFrameVar );
		if( params[BASETEXTURE]->IsTexture() )
		{
			BindTexture( SHADER_TEXTURE_STAGE1, BASETEXTURE, FRAME );
		}
		else
		{
			pShaderAPI->BindGrey( SHADER_TEXTURE_STAGE1 );
		}
		if( bWorldVertexTransition )
		{
			Assert( baseTexture2Var >= 0 && baseTexture2FrameVar >= 0 );
			BindTexture( SHADER_TEXTURE_STAGE4, baseTexture2Var, baseTexture2FrameVar );
		}
		pShaderAPI->BindNormalizationCubeMap( SHADER_TEXTURE_STAGE2 );
		if( bBump )
		{
			BindTexture( SHADER_TEXTURE_STAGE3, bumpmapVar, bumpmapFrame );
		}
		else
		{
			pShaderAPI->BindNormalizationCubeMap( SHADER_TEXTURE_STAGE3 );
		}

		if( bWorldVertexTransition )
		{
			pShaderAPI->BindLightmap( SHADER_TEXTURE_STAGE5 );
			if( bBump2 )
			{
				BindTexture( SHADER_TEXTURE_STAGE6, bumpmap2Var, bumpmap2Frame );
			}
		}

		if( bLightmappedGeneric )
		{
			ff_lightmappedgeneric_flashlight_vs20_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
		else
		{
			ff_vertexlitgeneric_flashlight_vs11_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			vshIndex.SetNUM_BONES( pShaderAPI->GetCurrentNumBones() );
			pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}

		ff_flashlight_ps20_Dynamic_Index pshIndex;
		pShaderAPI->SetPixelShaderIndex( pshIndex.GetIndex() );

		SetFlashlightVertexShaderConstants( bBump, bumpTransform, true );
	}
	Draw();
}

void CBaseVSShader::SetWaterFogColorPixelShaderConstantLinear( int reg )
{
	unsigned char fogRGB[3];
	s_pShaderAPI->GetSceneFogColor( fogRGB );
	float waterFogColor[4];
	waterFogColor[0] = GammaToLinear( fogRGB[0] * ( 1.0f / 255.0f ) );
	waterFogColor[1] = GammaToLinear( fogRGB[1] * ( 1.0f / 255.0f ) );
	waterFogColor[2] = GammaToLinear( fogRGB[2] * ( 1.0f / 255.0f ) );
	waterFogColor[3] = 0.0f;
	s_pShaderAPI->SetPixelShaderConstant( reg, waterFogColor );
}

void CBaseVSShader::SetWaterFogColorPixelShaderConstantGamma( int reg )
{
	unsigned char fogRGB[3];
	s_pShaderAPI->GetSceneFogColor( fogRGB );
	float waterFogColor[4];
	waterFogColor[0] = fogRGB[0] * ( 1.0f / 255.0f );
	waterFogColor[1] = fogRGB[1] * ( 1.0f / 255.0f );
	waterFogColor[2] = fogRGB[2] * ( 1.0f / 255.0f );
	waterFogColor[3] = 0.0f;
	s_pShaderAPI->SetPixelShaderConstant( reg, waterFogColor );
}
