//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "basevsshader.h"
#include "vmatrix.h"
#include "bumpvects.h"

#include "ConVar.h"

#ifdef DX9
#include "vertexlit_and_unlit_generic_vs20.inc"
#include "vertexlit_and_unlit_generic_ps20.inc"
#include "vertexlit_and_unlit_generic_bump_vs20.inc"
#include "vertexlit_and_unlit_generic_bump_ps20.inc"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void CBaseVSShader::InitParamsVertexLitAndUnlitGeneric_DX9( 
	const char *pMaterialName,
	bool bVertexLitGeneric,
	int baseTextureVar, int frameVar, int baseTextureTransformVar, 
	int selfIllumTintVar,
	int detailVar, int detailFrameVar, int detailScaleVar, 
	int envmapVar, int envmapFrameVar, 
	int envmapMaskVar, int envmapMaskFrameVar, int envmapMaskTransformVar,
	int envmapTintVar, 
	int bumpmapVar, int bumpFrameVar, int bumpTransformVar,
	int envmapContrastVar, int envmapSaturationVar, int alphaTestReferenceVar, 
	int parallaxMapVar, int parallaxMapScaleVar, int parallaxMapBiasVar, int albedoVar,
	int flashlightTextureVar, int flashlightTextureFrameVar )
{
	IMaterialVar** params = s_ppParams;
	
	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	Assert( flashlightTextureVar >= 0 );
	params[flashlightTextureVar]->SetStringValue( "effects/flashlight001" );
	
	// Write over $basetexture with $bumpmapVar if we are going to be using diffuse normal mapping.
	if( albedoVar != -1 && g_pConfig->UseBumpmapping() && bumpmapVar != -1 && params[bumpmapVar]->IsDefined() && params[albedoVar]->IsDefined() &&
		params[baseTextureVar]->IsDefined() )
	{
		params[BASETEXTURE]->SetStringValue( params[ALBEDO]->GetStringValue() );
	}

	if( bVertexLitGeneric )
	{
		SET_FLAGS( MATERIAL_VAR_MODEL );
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
	}
	else
	{
		CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
	}

	if( !params[envmapMaskFrameVar]->IsDefined() )
	{
		params[envmapMaskFrameVar]->SetIntValue( 0 );
	}
	
	if( !params[envmapTintVar]->IsDefined() )
	{
		params[envmapTintVar]->SetVecValue( 1.0f, 1.0f, 1.0f );
	}

	if( (selfIllumTintVar != -1) && (!params[selfIllumTintVar]->IsDefined()) )
	{
		params[selfIllumTintVar]->SetVecValue( 1.0f, 1.0f, 1.0f );
	}

	if( !params[detailScaleVar]->IsDefined() )
	{
		params[detailScaleVar]->SetFloatValue( 4.0f );
	}

	if( !params[envmapContrastVar]->IsDefined() )
	{
		params[envmapContrastVar]->SetFloatValue( 0.0f );
	}
	
	if( !params[envmapSaturationVar]->IsDefined() )
	{
		params[envmapSaturationVar]->SetFloatValue( 1.0f );
	}
	
	if( !params[envmapFrameVar]->IsDefined() )
	{
		params[envmapFrameVar]->SetIntValue( 0 );
	}

	if( (bumpFrameVar != -1) && !params[bumpFrameVar]->IsDefined() )
	{
		params[bumpFrameVar]->SetIntValue( 0 );
	}

	// No texture means no self-illum or env mask in base alpha
	if ( !params[baseTextureVar]->IsDefined() )
	{
		CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	// If in decal mode, no debug override...
	if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
	{
		SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
	}

	if( ( (bumpmapVar != -1) && g_pConfig->UseBumpmapping() && params[bumpmapVar]->IsDefined() ) || params[envmapVar]->IsDefined() )
	{
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
	}
	else
	{
		CLEAR_FLAGS( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
	}

	bool hasNormalMapAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
	if( hasNormalMapAlphaEnvmapMask )
	{
		params[envmapMaskVar]->SetUndefined();
		CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	}

	if( IS_FLAG_SET( MATERIAL_VAR_BASEALPHAENVMAPMASK ) && bumpmapVar != -1 && 
		params[bumpmapVar]->IsDefined() && !hasNormalMapAlphaEnvmapMask )
	{
		Warning( "material %s has a normal map and $basealphaenvmapmask.  Must use $normalmapalphaenvmapmask to get specular.\n\n", pMaterialName );
		params[envmapVar]->SetUndefined();
	}
	
	if( params[envmapMaskVar]->IsDefined() && bumpmapVar != -1 && params[bumpmapVar]->IsDefined() )
	{
		params[envmapMaskVar]->SetUndefined();
		if( !hasNormalMapAlphaEnvmapMask )
		{
			Warning( "material %s has a normal map and an envmapmask.  Must use $normalmapalphaenvmapmask.\n\n", pMaterialName );
			params[envmapVar]->SetUndefined();
		}
	}

	// If mat_specular 0, then get rid of envmap
	if( !g_pConfig->UseSpecular() && params[envmapVar]->IsDefined() && params[baseTextureVar]->IsDefined() )
	{
		params[envmapVar]->SetUndefined();
	}

	if( parallaxMapVar != -1 && parallaxMapScaleVar != -1 && !params[parallaxMapScaleVar]->IsDefined() )
	{
		params[parallaxMapVar]->SetUndefined();
	}
	if( parallaxMapVar != -1 && parallaxMapBiasVar != -1 && params[parallaxMapVar]->IsDefined() )
	{
		params[parallaxMapBiasVar]->SetFloatValue( 128.0f );
	}
}

void CBaseVSShader::InitParamsUnlitGeneric_DX9(
	const char *pMaterialName, int baseTextureVar, int frameVar, int baseTextureTransformVar, 
	int detailVar, int detailFrameVar, int detailScaleVar, 
	int envmapVar, int envmapFrameVar, 
	int envmapMaskVar, int envmapMaskFrameVar, int envmapMaskTransformVar,
	int envmapTintVar, int envmapContrastVar, int envmapSaturationVar, int alphaTestReferenceVar,
	int flashlightTextureVar, int flashlightTextureFrameVar )
{
	InitParamsVertexLitAndUnlitGeneric_DX9( pMaterialName, false, baseTextureVar, frameVar, baseTextureTransformVar,
		-1, detailVar, detailFrameVar, detailScaleVar, envmapVar, envmapFrameVar, 
		envmapMaskVar,envmapMaskFrameVar, envmapMaskTransformVar, envmapTintVar,
		-1, -1, -1, envmapContrastVar, envmapSaturationVar, alphaTestReferenceVar, -1, -1, -1, -1,
		flashlightTextureVar, flashlightTextureFrameVar );
}


void CBaseVSShader::InitVertexLitAndUnlitGeneric_DX9( 
		const char *pMaterialName,
  	    bool bVertexLitGeneric,
		int baseTextureVar, 
		int frameVar, 
		int baseTextureTransformVar, 
		int selfIllumTintVar,
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
		int alphaTestReferenceVar,
		int parallaxMapVar, int parallaxMapScaleVar, 
		int flashlightTextureVar, int flashlightTextureFrameVar )
{
	IMaterialVar** params = s_ppParams;

	Assert( flashlightTextureVar >= 0 );
	LoadTexture( flashlightTextureVar );
	
	if (params[baseTextureVar]->IsDefined())
	{
		LoadTexture( baseTextureVar );
		
		if (!params[baseTextureVar]->GetTextureValue()->IsTranslucent())
		{
			CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
			CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
		}
	}
	
	if (params[detailVar]->IsDefined())
	{
		LoadTexture( detailVar );
	}
	
	if ((bumpmapVar != -1) && g_pConfig->UseBumpmapping() && params[bumpmapVar]->IsDefined())
	{
		LoadBumpMap( bumpmapVar );
		SET_FLAGS2( MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL );
	}
	
	// Don't alpha test if the alpha channel is used for other purposes
	if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK) )
	{
		CLEAR_FLAGS( MATERIAL_VAR_ALPHATEST );
	}
	
	if (params[envmapVar]->IsDefined())
	{
		if( !IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE) )
		{
			LoadCubeMap( envmapVar );
		}
		else
		{
			LoadTexture( envmapVar );
		}
		
		if( !g_pHardwareConfig->SupportsCubeMaps() )
		{
			SET_FLAGS( MATERIAL_VAR_ENVMAPSPHERE );
		}
		
		if (params[envmapMaskVar]->IsDefined())
		{
			LoadTexture( envmapMaskVar );
		}
	}

	if( g_pConfig->UseParallaxMapping() && parallaxMapVar != -1 && params[parallaxMapVar]->IsDefined() )
	{
		LoadTexture( parallaxMapVar );
	}
}

void CBaseVSShader::InitUnlitGeneric_DX9( 
	const char *pMaterialName,
    int baseTextureVar, int frameVar,			int baseTextureTransformVar, 
	int detailVar,		int detailFrameVar,		int detailScaleVar, 
	int envmapVar,		int envmapFrameVar, 
	int envmapMaskVar,	int envmapMaskFrameVar, int envmapMaskTransformVar,
	int envmapTintVar,	int envmapContrastVar,	int envmapSaturationVar, int alphaTestReferenceVar,
	int flashlightTextureVar, int flashlightTextureFrameVar )
{
	InitVertexLitAndUnlitGeneric_DX9( pMaterialName, false, baseTextureVar, frameVar, baseTextureTransformVar,
		-1, detailVar, detailFrameVar, detailScaleVar, envmapVar, envmapFrameVar, 
		envmapMaskVar,envmapMaskFrameVar, envmapMaskTransformVar, envmapTintVar,
		-1, -1, -1, envmapContrastVar, envmapSaturationVar, alphaTestReferenceVar, -1, -1,
		flashlightTextureVar, flashlightTextureFrameVar );
}

extern ConVar mat_skipspecmask;

void CBaseVSShader::DrawVertexLitAndUnlitGeneric_DX9( 
		bool bVertexLitGeneric,
   	    int baseTextureVar, 
		int frameVar, 
		int baseTextureTransformVar, 
		int selfIllumTintVar,
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
		int alphaTestReferenceVar,
		int parallaxMapVar, 
		int parallaxMapScaleVar,
		int parallaxMapBiasVar,
		int flashlightTextureVar, 
		int flashlightTextureFrameVar
		)
{
#ifdef DX9
	IMaterialVar** params = s_ppParams;
	bool hasBaseTexture = params[baseTextureVar]->IsTexture();
	bool hasDetailTexture = params[detailVar]->IsTexture();
	bool hasBump = (bumpmapVar != -1) && params[bumpmapVar]->IsTexture();
	bool hasDiffuseLighting = bVertexLitGeneric;
	bool hasBaseAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_BASEALPHAENVMAPMASK );
	bool hasNormalMapAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
	bool hasVertexColor = bVertexLitGeneric ? false : IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR );
	bool hasVertexAlpha = bVertexLitGeneric ? false : IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA );
	bool hasParallaxMap = parallaxMapVar != -1 && parallaxMapScaleVar != -1 && parallaxMapBiasVar != -1 && hasBump &&  
		params[parallaxMapVar]->IsTexture() && g_pConfig->UseParallaxMapping();
	bool bIsAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0;

	BlendType_t blendType;
	if (params[BASETEXTURE]->IsTexture())
		blendType = EvaluateBlendRequirements( BASETEXTURE, true );
	else
		blendType = EvaluateBlendRequirements( envmapMaskVar, false );
	
	
	if( IsSnapshotting() )
	{
		// look at color and alphamod stuff.
		// Unlit generic never uses the flashlight
		bool hasFlashlight = hasDiffuseLighting && CShader_IsFlag2Set( params, MATERIAL_VAR2_USE_FLASHLIGHT );
		bool hasEnvmap = !hasFlashlight && params[envmapVar]->IsTexture();
		bool hasEnvmapMask = !hasFlashlight && params[envmapMaskVar]->IsTexture();
		bool bHasNormal = bVertexLitGeneric || hasEnvmap;
		bool hasSelfIllum = !hasFlashlight && IS_FLAG_SET( MATERIAL_VAR_SELFILLUM );
		
		bool bHalfLambert = IS_FLAG_SET( MATERIAL_VAR_HALFLAMBERT );
		// Alpha test: FIXME: shouldn't this be handled in CBaseVSShader::SetInitialShadowState
		s_pShaderShadow->EnableAlphaTest( bIsAlphaTested );

		if( alphaTestReferenceVar != -1 && params[alphaTestReferenceVar]->GetFloatValue() > 0.0f )
		{
			s_pShaderShadow->AlphaFunc( SHADER_ALPHAFUNC_GEQUAL, params[alphaTestReferenceVar]->GetFloatValue() );
		}

		if( hasFlashlight )
		{
			if (params[baseTextureVar]->IsTexture())
			{
				SetAdditiveBlendingShadowState( baseTextureVar, true );
			}
			else
			{
				SetAdditiveBlendingShadowState( envmapMaskVar, false );
			}
			if( bIsAlphaTested )
			{
				// disable alpha test and use the zfunc zequals since alpha isn't guaranteed to 
				// be the same on both the regular pass and the flashlight pass.
				s_pShaderShadow->EnableAlphaTest( false );
				s_pShaderShadow->DepthFunc( SHADER_DEPTHFUNC_EQUAL );
			}
			s_pShaderShadow->EnableBlending( true );
			s_pShaderShadow->EnableDepthWrites( false );
		}
		else
		{
			if (params[baseTextureVar]->IsTexture())
				SetDefaultBlendingShadowState( baseTextureVar, true );
			else
				SetDefaultBlendingShadowState( envmapMaskVar, false );
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
			s_pShaderShadow->EnableSRGBRead( SHADER_TEXTURE_STAGE1, true );
		}
		if( hasFlashlight )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE7, true );
			userDataSize = 4; // tangent S
		}
		if( hasDetailTexture )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
		}
		if( hasBump )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
			userDataSize = 4; // tangent S
			// Normalizing cube map
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE5, true );
		}
		if( hasEnvmapMask )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE4, true );
		}

		if( hasParallaxMap )
		{
			s_pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE6, true );
		}

		if( hasVertexColor || hasVertexAlpha )
		{
			flags |= VERTEX_COLOR;
		}

		s_pShaderShadow->EnableSRGBWrite( true );
		
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

		if ( hasBump )
		{
			ff_vertexlit_and_unlit_generic_bump_vs20_Static_Index vshIndex;
			vshIndex.SetHALFLAMBERT( bHalfLambert);
			vshIndex.SetPARALLAXMAP( hasParallaxMap );
			s_pShaderShadow->SetVertexShader( "vertexlit_and_unlit_generic_bump_vs20", vshIndex.GetIndex() );
			
			ff_vertexlit_and_unlit_generic_bump_ps20_Static_Index pshIndex;
			pshIndex.SetBASETEXTURE( hasBaseTexture );
			pshIndex.SetCUBEMAP( hasEnvmap );
			pshIndex.SetDIFFUSELIGHTING( hasDiffuseLighting );
			pshIndex.SetSELFILLUM( hasSelfIllum );
			pshIndex.SetNORMALMAPALPHAENVMAPMASK( hasNormalMapAlphaEnvmapMask );
			pshIndex.SetHALFLAMBERT( bHalfLambert);
			pshIndex.SetPARALLAXMAP( hasParallaxMap );
			pshIndex.SetFLASHLIGHT( hasFlashlight );

			s_pShaderShadow->SetPixelShader( "vertexlit_and_unlit_generic_bump_ps20", pshIndex.GetIndex() );
		}
		else
		{
			ff_vertexlit_and_unlit_generic_vs20_Static_Index vshIndex;
			vshIndex.SetVERTEXCOLOR( hasVertexColor || hasVertexAlpha );
			vshIndex.SetCUBEMAP( hasEnvmap );
			vshIndex.SetHALFLAMBERT( bHalfLambert );
			vshIndex.SetFLASHLIGHT( hasFlashlight );
			s_pShaderShadow->SetVertexShader( "vertexlit_and_unlit_generic_vs20", vshIndex.GetIndex() );
			
			ff_vertexlit_and_unlit_generic_ps20_Static_Index pshIndex;
			pshIndex.SetBASETEXTURE( hasBaseTexture );
			pshIndex.SetDETAILTEXTURE( hasDetailTexture );
			pshIndex.SetCUBEMAP( hasEnvmap );
			pshIndex.SetDIFFUSELIGHTING( hasDiffuseLighting );
			pshIndex.SetENVMAPMASK( hasEnvmapMask );
			pshIndex.SetBASEALPHAENVMAPMASK( hasBaseAlphaEnvmapMask );
			pshIndex.SetSELFILLUM( hasSelfIllum );
			pshIndex.SetVERTEXCOLOR( hasVertexColor );
			pshIndex.SetVERTEXALPHA( hasVertexAlpha );
			pshIndex.SetFLASHLIGHT( hasFlashlight );

			s_pShaderShadow->SetPixelShader( "vertexlit_and_unlit_generic_ps20", pshIndex.GetIndex() );
		}

		if( hasFlashlight )
		{
			FogToBlack();
		}
		else
		{
			DefaultFog();
		}

		// HACK HACK HACK - enable alpha writes all the time so that we have them for
		// underwater stuff
		if( blendType != BT_BLENDADD && blendType != BT_BLEND && !bIsAlphaTested )
		{
			s_pShaderShadow->EnableAlphaWrites( true );
		}
	}
	else
	{
		bool hasFlashlight = hasDiffuseLighting && s_pShaderAPI->InFlashlightMode();
		bool hasEnvmap = !hasFlashlight && params[envmapVar]->IsTexture();
		bool hasEnvmapMask = !hasFlashlight && params[envmapMaskVar]->IsTexture();

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
		if( hasFlashlight )
		{
			Assert( flashlightTextureVar >= 0 && flashlightTextureFrameVar >= 0 );
			BindTexture( SHADER_TEXTURE_STAGE7, flashlightTextureVar, flashlightTextureFrameVar );
		}

		int lightCombo = 0;
		if( bVertexLitGeneric && !hasFlashlight )
		{
			lightCombo = s_pShaderAPI->GetCurrentLightCombo();
		}
		MaterialFogMode_t fogType = s_pShaderAPI->GetSceneFogMode();
		int fogIndex = ( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ? 1 : 0;
		int numBones	= s_pShaderAPI->GetCurrentNumBones();

		if ( hasBump )
		{
			ff_vertexlit_and_unlit_generic_bump_vs20_Dynamic_Index vshIndex;
			vshIndex.SetLIGHT_COMBO( lightCombo );
			vshIndex.SetFOG_TYPE( fogIndex );
			vshIndex.SetNUM_BONES( numBones );
			s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );

			ff_vertexlit_and_unlit_generic_bump_ps20_Dynamic_Index pshIndex;
			pshIndex.SetLIGHT_COMBO( lightCombo );
			pshIndex.SetWRITEWATERFOGTODESTALPHA( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z &&
				blendType != BT_BLENDADD && blendType != BT_BLEND && !bIsAlphaTested );
			pshIndex.SetDOWATERFOG( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			s_pShaderAPI->SetPixelShaderIndex( pshIndex.GetIndex() );

			if( hasParallaxMap )
			{
				float scale = params[parallaxMapScaleVar]->GetFloatValue();
				float vScale[4] = { scale, scale, scale, scale };
				s_pShaderAPI->SetPixelShaderConstant( 23, vScale );

				float bias = -params[parallaxMapBiasVar]->GetFloatValue() / 255.0f * scale;
				float vBias[4] = { bias, bias, bias, bias };
				s_pShaderAPI->SetPixelShaderConstant( 24, vBias );
			}
		}
		else
		{
			ff_vertexlit_and_unlit_generic_vs20_Dynamic_Index vshIndex;
			vshIndex.SetLIGHT_COMBO( lightCombo );
			vshIndex.SetFOG_TYPE( fogIndex );
			vshIndex.SetNUM_BONES( numBones );
			s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );

			ff_vertexlit_and_unlit_generic_ps20_Dynamic_Index pshIndex;
			pshIndex.SetWRITEWATERFOGTODESTALPHA( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z &&
				blendType != BT_BLENDADD && blendType != BT_BLEND && !bIsAlphaTested );
			pshIndex.SetDOWATERFOG( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			s_pShaderAPI->SetPixelShaderIndex( pshIndex.GetIndex() );
		}

		SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, baseTextureTransformVar );
		if( hasDetailTexture )
		{
			SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, baseTextureTransformVar, detailScaleVar );
			Assert( !hasBump );
		}
		if( hasBump )
		{
			SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, bumpTransformVar );
			Assert( !hasDetailTexture );
		}
		if( hasEnvmapMask )
		{
			SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, envmapMaskTransformVar );
		}
		
		if( hasEnvmap )
		{
			SetEnvMapTintPixelShaderDynamicState( 0, envmapTintVar, -1, true );
		}
		SetModulationPixelShaderDynamicState_LinearColorSpace( 1 );
		SetPixelShaderConstant( 2, envmapContrastVar );
		SetPixelShaderConstant( 3, envmapSaturationVar );
		EnablePixelShaderOverbright( 4, true, false );
		SetPixelShaderConstant( 5, selfIllumTintVar );
		SetAmbientCubeDynamicStateVertexShader();
		if( hasBump )
		{
			s_pShaderAPI->BindSignedNormalizationCubeMap( SHADER_TEXTURE_STAGE5 );
			if( hasParallaxMap )
			{
				BindTexture( SHADER_TEXTURE_STAGE6, parallaxMapVar, -1 );
			}
		}

		s_pShaderAPI->SetPixelShaderStateAmbientLightCube( 6 );
		s_pShaderAPI->CommitPixelShaderLighting( 13 );

		float eyePos[4];
		s_pShaderAPI->GetWorldSpaceCameraPosition( eyePos );
		s_pShaderAPI->SetPixelShaderConstant( 25, eyePos, 1 );
		s_pShaderAPI->SetPixelShaderFogParams( 26 );

		// flashlightfixme: put this in common code.
		if( hasFlashlight )
		{
			VMatrix worldToTexture;
			const FlashlightState_t &flashlightState = s_pShaderAPI->GetFlashlightState( worldToTexture );

			// Set the flashlight attenuation factors
			float atten[4];
			atten[0] = flashlightState.m_fConstantAtten;
			atten[1] = flashlightState.m_fLinearAtten;
			atten[2] = flashlightState.m_fQuadraticAtten;
			atten[3] = flashlightState.m_FarZ;
			s_pShaderAPI->SetPixelShaderConstant( 2, atten, 1 );

			// Set the flashlight origin
			float pos[4];
			pos[0] = flashlightState.m_vecLightOrigin[0];
			pos[1] = flashlightState.m_vecLightOrigin[1];
			pos[2] = flashlightState.m_vecLightOrigin[2];
			pos[3] = 1.0f;
			s_pShaderAPI->SetPixelShaderConstant( 27, pos, 1 );

			s_pShaderAPI->SetPixelShaderConstant( 28, worldToTexture.Base(), 4 );
		}		

		if( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z )
		{
			SetWaterFogColorPixelShaderConstantLinear( 23 );
		}
	}
	Draw();
#endif
}

void CBaseVSShader::DrawUnlitGeneric_DX9( 
	int baseTextureVar, int frameVar,			int baseTextureTransformVar, 
	int detailVar,		int detailFrameVar,		int detailScaleVar, 
	int envmapVar,		int envmapFrameVar, 
	int envmapMaskVar,	int envmapMaskFrameVar, int envmapMaskTransformVar,
	int envmapTintVar,	int envmapContrastVar,	int envmapSaturationVar, int alphaTestReferenceVar,
	int flashlightTextureVar, int flashlightTextureFrameVar )
{
	DrawVertexLitAndUnlitGeneric_DX9( 
		false,
		baseTextureVar,		frameVar,			baseTextureTransformVar,
		-1,
		detailVar,			detailFrameVar,		detailScaleVar,
		envmapVar,			envmapFrameVar,
		envmapMaskVar,		envmapMaskFrameVar,	envmapMaskTransformVar,
		envmapTintVar,
		-1, -1, -1,
		envmapContrastVar,	envmapSaturationVar, alphaTestReferenceVar, -1, -1, -1,
		flashlightTextureVar, flashlightTextureFrameVar );
}


