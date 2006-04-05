//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Lightmap only shader
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"
#include "convar.h"

#include "ff_lightmappedgeneric_ps20.inc"
#include "ff_lightmappedgeneric_vs20.inc"


DEFINE_FALLBACK_SHADER( FF_LightmappedGeneric, FF_LightmappedGeneric_DX9 )

BEGIN_VS_SHADER( FF_LightmappedGeneric_DX9,
			  "Help for FF_LightmappedGeneric" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint" )
		SHADER_PARAM( DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture" )
		SHADER_PARAM( DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail" )
		SHADER_PARAM( DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture" )
		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap" )
		SHADER_PARAM( ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "", "" )
		SHADER_PARAM( ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "envmap mask" )
		SHADER_PARAM( ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "", "" )
		SHADER_PARAM( ENVMAPMASKTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$envmapmask texcoord transform" )
		SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint" )
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/WorldDiffuseBumpMap_bump", "bump map" )
		SHADER_PARAM( BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap" )
		SHADER_PARAM( BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform" )
		SHADER_PARAM( ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
		SHADER_PARAM( ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal" )
		SHADER_PARAM( FRESNELREFLECTION, SHADER_PARAM_TYPE_FLOAT, "1.0", "0.0 == no fresnel, 1.0 == full fresnel" )
		SHADER_PARAM( NODIFFUSEBUMPLIGHTING, SHADER_PARAM_TYPE_INTEGER, "0", "0 == Use diffuse bump lighting, 1 = No diffuse bump lighting" )
		SHADER_PARAM( PARALLAXMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/WorldDiffuseBumpMap_bump", "bump map" )
		SHADER_PARAM( PARALLAXMAPSCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "" )
		SHADER_PARAM( PARALLAXMAPBIAS, SHADER_PARAM_TYPE_FLOAT, "1.0", "" )
		SHADER_PARAM( PARALLAXMAP2, SHADER_PARAM_TYPE_TEXTURE, "shadertest/WorldDiffuseBumpMap_bump", "bump map" )
		SHADER_PARAM( PARALLAXMAPSCALE2, SHADER_PARAM_TYPE_FLOAT, "1.0", "" )
		SHADER_PARAM( PARALLAXMAPBIAS2, SHADER_PARAM_TYPE_FLOAT, "1.0", "" )
		SHADER_PARAM( BUMPMAP2, SHADER_PARAM_TYPE_TEXTURE, "shadertest/WorldDiffuseBumpMap_bump", "bump map" )
		SHADER_PARAM( BUMPFRAME2, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap" )
		SHADER_PARAM( BASETEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture" )
		SHADER_PARAM( FRAME2, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $basetexture2" )
		SHADER_PARAM( BASETEXTURENOENVMAP, SHADER_PARAM_TYPE_BOOL, "0", "" )
		SHADER_PARAM( BASETEXTURE2NOENVMAP, SHADER_PARAM_TYPE_BOOL, "0", "" )
		SHADER_PARAM( DETAIL_ALPHA_MASK_BASE_TEXTURE, SHADER_PARAM_TYPE_BOOL, "0", 
			"If this is 1, then when detail alpha=0, no base texture is blended and when "
			"detail alpha=1, you get detail*base*lightmap" )
	END_SHADER_PARAMS

	// Set up anything that is necessary to make decisions in SHADER_FALLBACK.
	SHADER_INIT_PARAMS()
	{
		params[PARALLAXMAP]->SetUndefined();
		params[PARALLAXMAP2]->SetUndefined();

		// FLASHLIGHTFIXME
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );
	
		// Write over $basetexture with $albedo if we are going to be using diffuse normal mapping.
		if( g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined() && params[ALBEDO]->IsDefined() &&
			params[BASETEXTURE]->IsDefined() && 
			!( params[NODIFFUSEBUMPLIGHTING]->IsDefined() && params[NODIFFUSEBUMPLIGHTING]->GetIntValue() ) )
		{
			params[BASETEXTURE]->SetStringValue( params[ALBEDO]->GetStringValue() );
		}
	
		if( IsUsingGraphics() && params[ENVMAP]->IsDefined() )
		{
			if( stricmp( params[ENVMAP]->GetStringValue(), "env_cubemap" ) == 0 )
			{
				Warning( "env_cubemap used on world geometry without rebuilding map. . ignoring: %s\n", pMaterialName );
				params[ENVMAP]->SetUndefined();
			}
		}
		
		if( !params[ENVMAPTINT]->IsDefined() )
			params[ENVMAPTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );

		if( !params[NODIFFUSEBUMPLIGHTING]->IsDefined() )
			params[NODIFFUSEBUMPLIGHTING]->SetIntValue( 0 );

		if( !params[SELFILLUMTINT]->IsDefined() )
			params[SELFILLUMTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );

		if( !params[DETAILSCALE]->IsDefined() )
			params[DETAILSCALE]->SetFloatValue( 4.0f );

		if( !params[FRESNELREFLECTION]->IsDefined() )
			params[FRESNELREFLECTION]->SetFloatValue( 1.0f );

		if( !params[ENVMAPMASKFRAME]->IsDefined() )
			params[ENVMAPMASKFRAME]->SetIntValue( 0 );
		
		if( !params[ENVMAPFRAME]->IsDefined() )
			params[ENVMAPFRAME]->SetIntValue( 0 );

		if( !params[BUMPFRAME]->IsDefined() )
			params[BUMPFRAME]->SetIntValue( 0 );

		if( !params[DETAILFRAME]->IsDefined() )
			params[DETAILFRAME]->SetIntValue( 0 );

		if( !params[ENVMAPCONTRAST]->IsDefined() )
			params[ENVMAPCONTRAST]->SetFloatValue( 0.0f );
		
		if( !params[ENVMAPSATURATION]->IsDefined() )
			params[ENVMAPSATURATION]->SetFloatValue( 1.0f );
		
		// No texture means no self-illum or env mask in base alpha
		if ( !params[BASETEXTURE]->IsDefined() )
		{
			CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
			CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
		}

		if( params[BUMPMAP]->IsDefined() )
		{
			params[ENVMAPMASK]->SetUndefined();
		}
		
		// If in decal mode, no debug override...
		if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
		{
			SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
		}

		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );
		if( g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined() && (params[NODIFFUSEBUMPLIGHTING]->GetIntValue() == 0) )
		{
			SET_FLAGS2( MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP );
		}

		// If mat_specular 0, then get rid of envmap
		if( !g_pConfig->UseSpecular() && params[ENVMAP]->IsDefined() && params[BASETEXTURE]->IsDefined() )
		{
			params[ENVMAP]->SetUndefined();
		}

		if( !params[BASETEXTURENOENVMAP]->IsDefined() )
		{
			params[BASETEXTURENOENVMAP]->SetIntValue( 0 );
		}
		if( !params[BASETEXTURE2NOENVMAP]->IsDefined() )
		{
			params[BASETEXTURE2NOENVMAP]->SetIntValue( 0 );
		}
	}

	SHADER_FALLBACK
	{
		if( g_pHardwareConfig->GetDXSupportLevel() < 90 )
			return "LightmappedGeneric_DX8";

		return 0;
	}

	SHADER_INIT
	{
		if( g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined() )
		{
			LoadBumpMap( BUMPMAP );
		}
		
		if( g_pConfig->UseBumpmapping() && params[BUMPMAP2]->IsDefined() )
		{
			LoadBumpMap( BUMPMAP2 );
		}

		if (params[BASETEXTURE]->IsDefined())
		{
			LoadTexture( BASETEXTURE );

			if (!params[BASETEXTURE]->GetTextureValue()->IsTranslucent())
			{
				CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
				CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
			}
		}

		if( params[BASETEXTURE2]->IsDefined() )
		{
			LoadTexture( BASETEXTURE2 );
		}
	
		if (params[DETAIL]->IsDefined())
		{
			LoadTexture( DETAIL );
		}

		if( g_pConfig->UseParallaxMapping() && params[PARALLAXMAP]->IsDefined() )
		{
			LoadTexture( PARALLAXMAP );
		}

		if( g_pConfig->UseParallaxMapping() && params[PARALLAXMAP2]->IsDefined() )
		{
			LoadTexture( PARALLAXMAP2 );
		}

		LoadTexture( FLASHLIGHTTEXTURE );
		
		// Don't alpha test if the alpha channel is used for other purposes
		if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK) )
		{
			CLEAR_FLAGS( MATERIAL_VAR_ALPHATEST );
		}
			
		if (params[ENVMAP]->IsDefined())
		{
			if( !IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE) )
				LoadCubeMap( ENVMAP );
			else
				LoadTexture( ENVMAP );

			if( !g_pHardwareConfig->SupportsCubeMaps() )
			{
				SET_FLAGS( MATERIAL_VAR_ENVMAPSPHERE );
			}

			if (params[ENVMAPMASK]->IsDefined())
				LoadTexture( ENVMAPMASK );
		}
		else
		{
			params[ENVMAPMASK]->SetUndefined();
		}

		// We always need this because of the flashlight.
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );

		if( !params[PARALLAXMAPSCALE]->IsDefined() )
		{
			params[PARALLAXMAP]->SetUndefined();
		}
		if( !params[PARALLAXMAPSCALE2]->IsDefined() )
		{
			params[PARALLAXMAP2]->SetUndefined();
		}
		if( params[PARALLAXMAP]->IsDefined() && !params[PARALLAXMAPBIAS]->IsDefined() )
		{
			params[PARALLAXMAPBIAS]->SetFloatValue( 128.0f );
		}
		if( params[PARALLAXMAP2]->IsDefined() && !params[PARALLAXMAPBIAS2]->IsDefined() )
		{
			params[PARALLAXMAPBIAS2]->SetFloatValue( 128.0f );
		}
	}

	SHADER_DRAW
	{
		bool hasBump = params[BUMPMAP]->IsTexture();
		bool hasBump2 = params[BUMPMAP2]->IsTexture();
		bool hasDiffuseBumpmap = hasBump && (params[NODIFFUSEBUMPLIGHTING]->GetIntValue() == 0);
		bool hasParallaxMap = hasBump && hasDiffuseBumpmap && params[PARALLAXMAP]->IsTexture() && g_pConfig->UseParallaxMapping();
		bool hasParallaxMap2 = hasBump && hasDiffuseBumpmap && params[PARALLAXMAP2]->IsTexture() && g_pConfig->UseParallaxMapping();
		bool hasBaseTexture = params[BASETEXTURE]->IsTexture();
		bool hasBaseTexture2 = params[BASETEXTURE2]->IsTexture();
		bool hasDetailTexture = params[DETAIL]->IsTexture();
		bool hasVertexColor = IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) != 0;
		bool bIsAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0;

		int nAlphaChannelTextureVar = hasBaseTexture ? (int)BASETEXTURE : (int)ENVMAPMASK;
		BlendType_t nBlendType = EvaluateBlendRequirements( nAlphaChannelTextureVar, hasBaseTexture );

		bool hasFlashlight = UsingFlashlight( params );
		if( hasFlashlight )
		{
			DrawFlashlight_dx90( params, pShaderAPI, pShaderShadow, hasBump, BUMPMAP, BUMPFRAME, BUMPTRANSFORM, 
				FLASHLIGHTTEXTURE, FLASHLIGHTTEXTUREFRAME, true, hasBaseTexture2, BASETEXTURE2, FRAME2, BUMPMAP2, BUMPFRAME2 );
			return;
		}

		SHADOW_STATE
		{
			bool hasEnvmap = params[ENVMAP]->IsTexture();
			bool hasEnvmapMask = params[ENVMAPMASK]->IsTexture();
			// Alpha test: FIXME: shouldn't this be handled in Shader_t::SetInitialShadowState
			pShaderShadow->EnableAlphaTest( bIsAlphaTested );
			SetDefaultBlendingShadowState( nAlphaChannelTextureVar, hasBaseTexture );

			unsigned int flags = VERTEX_POSITION;
			if( hasBaseTexture )
			{
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
			}
			if( hasBaseTexture2 )
			{
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE7, true );

			}

			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );

			if( hasEnvmap )
			{
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );

				flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T | VERTEX_NORMAL;
			}
			if( hasDetailTexture )
			{
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
			}
			if( hasBump )
			{
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE4, true );
			}
			if( hasBump2 )
			{
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE5, true );
			}
			if( hasEnvmapMask )
			{
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE5, true );
			}
			if( hasVertexColor )
			{
				flags |= VERTEX_COLOR;
			}

			// Normalizing cube map
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE6, true );

			// texcoord0 : base texcoord
			// texcoord1 : lightmap texcoord
			// texcoord2 : lightmap texcoord offset
			int numTexCoords = 0;
			if( hasBump )
			{
				numTexCoords = 3;
			}
			else 
			{
				numTexCoords = 2;
			}
			
			pShaderShadow->VertexShaderVertexFormat( 
				flags, numTexCoords, 0, 0, 0 );

			// Pre-cache pixel shaders
			bool hasBaseAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_BASEALPHAENVMAPMASK );
			bool hasSelfIllum = IS_FLAG_SET( MATERIAL_VAR_SELFILLUM );
			bool hasNormalMapAlphaEnvmapMask = IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );

			ff_lightmappedgeneric_vs20_Static_Index vshIndex;
			vshIndex.SetENVMAP_MASK( hasEnvmapMask );
			vshIndex.SetTANGENTSPACE( params[ENVMAP]->IsTexture() || params[PARALLAXMAP]->IsTexture()  );
			vshIndex.SetBUMPMAP( hasBump );
			vshIndex.SetDIFFUSEBUMPMAP( hasDiffuseBumpmap );
			vshIndex.SetVERTEXCOLOR( IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) != 0 );

			ff_lightmappedgeneric_ps20_Static_Index pshIndex;
			pshIndex.SetBASETEXTURE2( hasBaseTexture2 );
			pshIndex.SetDETAILTEXTURE( hasDetailTexture );
			pshIndex.SetBUMPMAP( hasBump );
			pshIndex.SetBUMPMAP2( hasBump2 );
			pshIndex.SetDIFFUSEBUMPMAP( hasDiffuseBumpmap );
			pshIndex.SetCUBEMAP( hasEnvmap );
			pshIndex.SetVERTEXCOLOR( hasVertexColor );
			pshIndex.SetENVMAPMASK( hasEnvmapMask );
			pshIndex.SetBASEALPHAENVMAPMASK( hasBaseAlphaEnvmapMask );
			pshIndex.SetSELFILLUM( hasSelfIllum );
			pshIndex.SetNORMALMAPALPHAENVMAPMASK( hasNormalMapAlphaEnvmapMask );
			pshIndex.SetBASETEXTURENOENVMAP( params[BASETEXTURENOENVMAP]->GetIntValue() );
			pshIndex.SetBASETEXTURE2NOENVMAP( params[BASETEXTURE2NOENVMAP]->GetIntValue() );

			// HACK HACK HACK - enable alpha writes all the time so that we have them for
			// underwater stuff. 
			// But only do it if we're not using the alpha already for translucency
			if( nBlendType != BT_BLENDADD && nBlendType != BT_BLEND && !bIsAlphaTested )
			{
				pShaderShadow->EnableAlphaWrites( true );
			}

			pShaderShadow->SetPixelShader( "FF_lightmappedgeneric_ps20", pshIndex.GetIndex() );
			pShaderShadow->SetVertexShader( "FF_lightmappedgeneric_vs20", vshIndex.GetIndex() );
			
			DefaultFog();
		}
		DYNAMIC_STATE
		{
			bool hasEnvmap = params[ENVMAP]->IsTexture();
			bool hasEnvmapMask = params[ENVMAPMASK]->IsTexture();
			if( hasBaseTexture )
			{
				BindTexture( SHADER_TEXTURE_STAGE0, BASETEXTURE, FRAME );
			}
			else
			{
				pShaderAPI->BindWhite( SHADER_TEXTURE_STAGE0 );
			}

			if( hasBaseTexture2 )
			{
				BindTexture( SHADER_TEXTURE_STAGE7, BASETEXTURE2, FRAME2 );
			}

			pShaderAPI->BindLightmap( SHADER_TEXTURE_STAGE1 );

			if( hasEnvmap )
			{
				BindTexture( SHADER_TEXTURE_STAGE2, ENVMAP, ENVMAPFRAME );
			}
			if( hasDetailTexture )
			{
				BindTexture( SHADER_TEXTURE_STAGE3, DETAIL, DETAILFRAME );
			}
			if( hasBump )
			{
				if( !g_pConfig->m_bFastNoBump )
				{
					BindTexture( SHADER_TEXTURE_STAGE4, BUMPMAP, BUMPFRAME );
				}
				else
				{
					pShaderAPI->BindFlatNormalMap( SHADER_TEXTURE_STAGE4 );
				}
			}
			if( hasBump2 )
			{
				if( !g_pConfig->m_bFastNoBump )
				{
					BindTexture( SHADER_TEXTURE_STAGE5, BUMPMAP2, BUMPFRAME2 );
				}
				else
				{
					pShaderAPI->BindFlatNormalMap( SHADER_TEXTURE_STAGE4 );
				}
			}
			if( hasEnvmapMask )
			{
				BindTexture( SHADER_TEXTURE_STAGE5, ENVMAPMASK, ENVMAPMASKFRAME );
			}

			pShaderAPI->BindSignedNormalizationCubeMap( SHADER_TEXTURE_STAGE6 );

			if( hasParallaxMap )
			{
				BindTexture( SHADER_TEXTURE_STAGE7, PARALLAXMAP, -1 );
			}

			// If we don't have a texture transform, we don't have
			// to set vertex shader constants or run vertex shader instructions
			// for the texture transform.
			bool bHasTextureTransform = 
				!( params[BASETEXTURETRANSFORM]->MatrixIsIdentity() &&
				   params[BUMPTRANSFORM]->MatrixIsIdentity() &&
				   params[ENVMAPMASKTRANSFORM]->MatrixIsIdentity() );
			
			bool bVertexShaderFastPath = !bHasTextureTransform;
			if( params[DETAIL]->IsTexture() )
			{
				bVertexShaderFastPath = false;
			}

			float color[4] = { 1.0, 1.0, 1.0, 1.0 };
			ComputeModulationColor( color );
			if( !( bVertexShaderFastPath && color[0] == 1.0f && color[1] == 1.0f && color[2] == 1.0f && color[3] == 1.0f ) )
			{
				bVertexShaderFastPath = false;
				s_pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_MODULATION_COLOR, color );
				SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, BASETEXTURETRANSFORM );
				if( hasBump )
				{
					SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, BUMPTRANSFORM );
					Assert( !hasDetailTexture );
				}
				if( hasEnvmapMask )
				{
					SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, ENVMAPMASKTRANSFORM );
				}
			}

			MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
			ff_lightmappedgeneric_vs20_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			vshIndex.SetFASTPATH( bVertexShaderFastPath );
			vshIndex.SetPARALLAXMAP( hasParallaxMap );
			pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );

			bool bPixelShaderFastPath = true;
			float envmapTintVal[4];
			float selfIllumTintVal[4];
			params[ENVMAPTINT]->GetVecValue( envmapTintVal, 3 );
			params[SELFILLUMTINT]->GetVecValue( selfIllumTintVal, 3 );
			float envmapContrast = params[ENVMAPCONTRAST]->GetFloatValue();
			float envmapSaturation = params[ENVMAPSATURATION]->GetFloatValue();
			float fresnelReflection = params[FRESNELREFLECTION]->GetFloatValue();

			bool bUsingContrast = hasEnvmap && ( (envmapContrast != 0.0f) && (envmapContrast != 1.0f) ) && (envmapSaturation != 1.0f);
			bool bUsingFresnel = hasEnvmap && (fresnelReflection != 1.0f);
			bool bUsingSelfIllumTint = IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) && (selfIllumTintVal[0] != 1.0f || selfIllumTintVal[1] != 1.0f || selfIllumTintVal[2] != 1.0f); 
			bool bUsingEnvMapTint = hasEnvmap && (envmapTintVal[0] != 1.0f || envmapTintVal[1] != 1.0f || envmapTintVal[2] != 1.0f); 
			if ( bUsingContrast || bUsingFresnel || bUsingSelfIllumTint || !g_pConfig->bShowSpecular )
			{
				bPixelShaderFastPath = false;
			}
			
			ff_lightmappedgeneric_ps20_Dynamic_Index pshIndex;
			pshIndex.SetFASTPATH( bPixelShaderFastPath );
			pshIndex.SetFASTPATHENVMAPTINT( bPixelShaderFastPath && bUsingEnvMapTint );
			pshIndex.SetFASTPATHENVMAPCONTRAST( bPixelShaderFastPath && envmapContrast == 1.0f );
			pshIndex.SetPARALLAXMAP( hasParallaxMap );
			pshIndex.SetPARALLAXMAP2( hasParallaxMap2 );

			// Don't write fog to alpha if we're using translucency
			pshIndex.SetWRITEWATERFOGTODESTALPHA( (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) && 
				(nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested );
			pshIndex.SetDOWATERFOG( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			pShaderAPI->SetPixelShaderIndex( pshIndex.GetIndex() );

			if( hasParallaxMap )
			{
				float scale = params[PARALLAXMAPSCALE]->GetFloatValue();
				float vScale[4] = { scale, scale, scale, scale };
				pShaderAPI->SetPixelShaderConstant( 8, vScale );

				float bias = -params[PARALLAXMAPBIAS]->GetFloatValue() / 255.0f * scale;
				float vBias[4] = { bias, bias, bias, bias };
				pShaderAPI->SetPixelShaderConstant( 9, vBias );
			}
			
			if( hasParallaxMap2 )
			{
				float scale = params[PARALLAXMAPSCALE2]->GetFloatValue();
				float vScale[4] = { scale, scale, scale, scale };
				pShaderAPI->SetPixelShaderConstant( 12, vScale );

				float bias = -params[PARALLAXMAPBIAS2]->GetFloatValue() / 255.0f * scale;
				float vBias[4] = { bias, bias, bias, bias };
				pShaderAPI->SetPixelShaderConstant( 13, vBias );
			}
			
			// always set the transform for detail textures since I'm assuming that you'll
			// always have a detailscale.
			if( hasDetailTexture )
			{
				SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, BASETEXTURETRANSFORM, DETAILSCALE );
			}
			
			if ( !bPixelShaderFastPath || bUsingEnvMapTint )
			{
				SetEnvMapTintPixelShaderDynamicState( 0, ENVMAPTINT, -1 );
			}

			if( !bPixelShaderFastPath )
			{
				SetPixelShaderConstant( 2, ENVMAPCONTRAST );
				SetPixelShaderConstant( 3, ENVMAPSATURATION );
				SetPixelShaderConstant( 7, SELFILLUMTINT );

				// [ 0, 0 ,0, R(0) ]
				float fresnel[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
				fresnel[3] = params[FRESNELREFLECTION]->GetFloatValue();
				pShaderAPI->SetPixelShaderConstant( 4, fresnel );

				// [ 0, 0 ,0, 1-R(0) ]
				fresnel[3] = 1.0f - fresnel[3];
				pShaderAPI->SetPixelShaderConstant( 5, fresnel );
			}

			float eyePos[4];
			pShaderAPI->GetWorldSpaceCameraPosition( eyePos );
			pShaderAPI->SetPixelShaderConstant( 10, eyePos, 1 );
			pShaderAPI->SetPixelShaderFogParams( 11 );

			if( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z )
			{
				SetWaterFogColorPixelShaderConstantGamma( 14 );
			}
		}
		Draw();
	}
END_SHADER
