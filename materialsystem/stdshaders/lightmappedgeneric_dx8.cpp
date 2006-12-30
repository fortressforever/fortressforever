//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Lightmap only shader
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"


#include "FF_lightmappedgeneric_vs11.inc"
#include "FF_unlitgeneric_vs11.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


DEFINE_FALLBACK_SHADER( FF_LightmappedGeneric, FF_LightmappedGeneric_DX8 )

BEGIN_VS_SHADER( FF_LightmappedGeneric_DX8,
			  "Help for FF_LightmappedGeneric_DX8" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( ALBEDO, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "albedo (Base texture with no baked lighting)" )
		SHADER_PARAM( SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint" )
		SHADER_PARAM( DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture" )
		SHADER_PARAM( DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture" )
		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap" )
		SHADER_PARAM( ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "", "" )
		SHADER_PARAM( ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "envmap mask" )
		SHADER_PARAM( ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "", "" )
		SHADER_PARAM( ENVMAPMASKSCALE, SHADER_PARAM_TYPE_FLOAT, "1", "envmap mask scale" )
		SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint" )
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map" )
		SHADER_PARAM( BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap" )
		SHADER_PARAM( BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform" )
		SHADER_PARAM( ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
		SHADER_PARAM( ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal" )
		SHADER_PARAM( FRESNELREFLECTION, SHADER_PARAM_TYPE_FLOAT, "1.0", "0.0 == no fresnel, 1.0 == full fresnel" )
		SHADER_PARAM( ENVMAPOPTIONAL, SHADER_PARAM_TYPE_INTEGER, "90", "Do specular pass only on dxlevel or higher (ie.80, 81, 90)" )
		SHADER_PARAM( NODIFFUSEBUMPLIGHTING, SHADER_PARAM_TYPE_BOOL, "0", "0 == Use diffuse bump lighting, 1 = No diffuse bump lighting" )
		SHADER_PARAM( FORCEBUMP, SHADER_PARAM_TYPE_BOOL, "0", "0 == Do bumpmapping if the card says it can handle it. 1 == Always do bumpmapping." )
	END_SHADER_PARAMS

	virtual bool ShouldUseBumpmapping( IMaterialVar **params ) 
	{ 
		return g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined(); 
	}

	// Set up anything that is necessary to make decisions in SHADER_FALLBACK.
	SHADER_INIT_PARAMS()
	{
		// FLASHLIGHTFIXME
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );

		// Write over $basetexture with $albedo if we are going to be using diffuse normal mapping.
		if( ShouldUseBumpmapping( params ) && params[ALBEDO]->IsDefined() &&
			params[BASETEXTURE]->IsDefined() && 
			!( params[NODIFFUSEBUMPLIGHTING]->IsDefined() && params[NODIFFUSEBUMPLIGHTING]->GetIntValue() ) )
		{
			params[BASETEXTURE]->SetStringValue( params[ALBEDO]->GetStringValue() );
		}

		if( IsUsingGraphics() && params[ENVMAP]->IsDefined() && !CanUseEditorMaterials() )
		{
			if( stricmp( params[ENVMAP]->GetStringValue(), "env_cubemap" ) == 0 )
			{
				Warning( "env_cubemap used on world geometry without rebuilding map. . ignoring: %s\n", pMaterialName );
				params[ENVMAP]->SetUndefined();
			}
		}
		
		if( !params[ENVMAPMASKSCALE]->IsDefined() )
		{
			params[ENVMAPMASKSCALE]->SetFloatValue( 1.0f );
		}

		if( !params[ENVMAPTINT]->IsDefined() )
		{
			params[ENVMAPTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
		}

		if( !params[SELFILLUMTINT]->IsDefined() )
		{
			params[SELFILLUMTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
		}

		if( !params[DETAILSCALE]->IsDefined() )
		{
			params[DETAILSCALE]->SetFloatValue( 4.0f );
		}

		if( !params[FRESNELREFLECTION]->IsDefined() )
		{
			params[FRESNELREFLECTION]->SetFloatValue( 1.0f );
		}

		if( !params[ENVMAPMASKFRAME]->IsDefined() )
		{
			params[ENVMAPMASKFRAME]->SetIntValue( 0 );
		}
		
		if( !params[ENVMAPFRAME]->IsDefined() )
		{
			params[ENVMAPFRAME]->SetIntValue( 0 );
		}

		if( !params[BUMPFRAME]->IsDefined() )
		{
			params[BUMPFRAME]->SetIntValue( 0 );
		}

		if( !params[ENVMAPCONTRAST]->IsDefined() )
		{
			params[ENVMAPCONTRAST]->SetFloatValue( 0.0f );
		}
		
		if( !params[ENVMAPSATURATION]->IsDefined() )
		{
			params[ENVMAPSATURATION]->SetFloatValue( 1.0f );
		}
		
		// No texture means no self-illum or env mask in base alpha
		if ( !params[BASETEXTURE]->IsDefined() )
		{
			CLEAR_FLAGS( MATERIAL_VAR_SELFILLUM );
			CLEAR_FLAGS( MATERIAL_VAR_BASEALPHAENVMAPMASK );
		}

		// If in decal mode, no debug override...
		if (IS_FLAG_SET(MATERIAL_VAR_DECAL))
		{
			SET_FLAGS( MATERIAL_VAR_NO_DEBUG_OVERRIDE );
		}

		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_LIGHTMAP );
		if( ShouldUseBumpmapping( params ) && (params[NODIFFUSEBUMPLIGHTING]->GetIntValue() == 0) )
		{
			SET_FLAGS2( MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP );
		}

		// Get rid of the envmap if it's optional for this dx level.
		if( params[ENVMAPOPTIONAL]->IsDefined() && (params[ENVMAPOPTIONAL]->GetIntValue() > g_pHardwareConfig->GetDXSupportLevel()) )
		{
			params[ENVMAP]->SetUndefined();
		}

		// If mat_specular 0, then get rid of envmap
		if( !g_pConfig->UseSpecular() && params[ENVMAP]->IsDefined() && params[BASETEXTURE]->IsDefined() )
		{
			params[ENVMAP]->SetUndefined();
		}

		// Get rid of envmap if we aren't using bumpmapping 
		// *and* we have normalmapalphaenvmapmask *and* we don't have envmapmask elsewhere
		if ( params[ENVMAP]->IsDefined() && params[BUMPMAP]->IsDefined() && IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK ) && !ShouldUseBumpmapping( params ) )
		{
			if ( !IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK) && !params[ENVMAPMASK]->IsDefined() )
			{
				params[ENVMAP]->SetUndefined();
			}
		}
	}

	SHADER_FALLBACK
	{
		if ( IsPC() && g_pHardwareConfig->GetDXSupportLevel() < 80)
			return "FF_LightmappedGeneric_DX6";

		if ( IsPC() && g_pHardwareConfig->PreferReducedFillrate() )
			return "FF_LightmappedGeneric_NoBump_DX8";

		return 0;
	}

	SHADER_INIT
	{
		LoadTexture( FLASHLIGHTTEXTURE );
		
		if( ShouldUseBumpmapping( params ) )
		{
			LoadBumpMap( BUMPMAP );
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

		if (params[DETAIL]->IsDefined())
		{
			LoadTexture( DETAIL );
		}

		// Don't alpha test if the alpha channel is used for other purposes
		if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK) )
			CLEAR_FLAGS( MATERIAL_VAR_ALPHATEST );
			
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

		if( ShouldUseBumpmapping( params ) )
		{
			SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
		}
	}

#ifndef USE_HLSL_PIXEL_SHADERS
	inline const char *GetPixelShaderName( IMaterialVar** params, bool bBumpedEnvMap )
	{
		static char const* s_pPixelShaders[] = 
		{
			// Unmasked
			"FF_LightmappedGeneric_EnvMapV2",
			"FF_LightmappedGeneric_SelfIlluminatedEnvMapV2",

			"FF_LightmappedGeneric_BaseAlphaMaskedEnvMapV2",
			"FF_LightmappedGeneric_SelfIlluminatedEnvMapV2",

			// Env map mask
			"FF_LightmappedGeneric_MaskedEnvMapV2",
			"FF_LightmappedGeneric_SelfIlluminatedMaskedEnvMapV2",

			"FF_LightmappedGeneric_MaskedEnvMapV2",
			"FF_LightmappedGeneric_SelfIlluminatedMaskedEnvMapV2",
		};

		if (!params[BASETEXTURE]->IsTexture())
		{
			if (params[ENVMAP]->IsTexture() && !bBumpedEnvMap )
			{
				if (!params[ENVMAPMASK]->IsDefined() )
				{
					return "FF_LightmappedGeneric_EnvmapNoTexture";
				}
				else
				{
					return "FF_LightmappedGeneric_MaskedEnvmapNoTexture";
				}
			}
			else
			{
				return "FF_LightmappedGeneric_NoTexture";
			}
		}
 		else
		{
			if (params[ENVMAP]->IsTexture() && !bBumpedEnvMap )
			{
				int pshIndex = 0;
				if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM))
					pshIndex |= 0x1;
				if (IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK))
					pshIndex |= 0x2;
				if (params[ENVMAPMASK]->IsTexture())
					pshIndex |= 0x4;
				return s_pPixelShaders[pshIndex];
			}
			else
			{
				if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM))
					return "FF_LightmappedGeneric_SelfIlluminated";
				else
					return "FF_LightmappedGeneric";
			}
		}
	}
#endif

	void DrawUnbumpedUsingVertexShader( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bBumpedEnvMap )
	{
		bool hasEnvmap = params[ENVMAP]->IsTexture() && !bBumpedEnvMap;
		bool hasBaseTexture = params[BASETEXTURE]->IsTexture();
		bool hasVertexColor = IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR );
		bool hasEnvmapCameraSpace = IS_FLAG_SET( MATERIAL_VAR_ENVMAPCAMERASPACE );
		bool hasEnvmapSphere = IS_FLAG_SET( MATERIAL_VAR_ENVMAPSPHERE );

		if ( hasEnvmap || hasBaseTexture || hasVertexColor || !bBumpedEnvMap )
		{
			SHADOW_STATE
			{
				// Alpha test
				pShaderShadow->EnableAlphaTest( IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) );

				// Base texture on stage 0
				if (params[BASETEXTURE]->IsTexture())
				{
					pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
				}

				// Lightmap on stage 1
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );

				int fmt = VERTEX_POSITION;

				if ( hasEnvmap )
				{
					fmt |= VERTEX_NORMAL;

					// envmap on stage 2
					pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );

					// envmapmask on stage 3
					if (params[ENVMAPMASK]->IsTexture() || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK ) )
					{
						pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
					}
				}

				if (params[BASETEXTURE]->IsTexture() || bBumpedEnvMap)
				{
					SetDefaultBlendingShadowState( BASETEXTURE, true );
				}
				else
				{
					SetDefaultBlendingShadowState( ENVMAPMASK, false );
				}

				if (IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR))
				{
					fmt |= VERTEX_COLOR;
				}

				pShaderShadow->VertexShaderVertexFormat( fmt, 2, 0, 0, 0 );
				ff_lightmappedgeneric_vs11_Static_Index vshIndex;
				vshIndex.SetDETAIL( false );
				vshIndex.SetENVMAP( hasEnvmap );
				vshIndex.SetENVMAPCAMERASPACE( hasEnvmap && hasEnvmapCameraSpace );
				vshIndex.SetENVMAPSPHERE( hasEnvmap && hasEnvmapSphere );
				vshIndex.SetVERTEXCOLOR( hasVertexColor );
				pShaderShadow->SetVertexShader( "FF_LightmappedGeneric_vs11", vshIndex.GetIndex() );

				const char *pshName = GetPixelShaderName( params, bBumpedEnvMap );
				pShaderShadow->SetPixelShader( pshName );
				DefaultFog();
			}
			DYNAMIC_STATE
			{
				if (hasBaseTexture)
				{
					BindTexture( SHADER_TEXTURE_STAGE0, BASETEXTURE, FRAME );
					SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, BASETEXTURETRANSFORM );
				}

				pShaderAPI->BindLightmap( SHADER_TEXTURE_STAGE1 );

				if ( hasEnvmap )
				{
					BindTexture( SHADER_TEXTURE_STAGE2, ENVMAP, ENVMAPFRAME );

					if (params[ENVMAPMASK]->IsTexture() || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK) )
					{
						if (params[ENVMAPMASK]->IsTexture() )
							BindTexture( SHADER_TEXTURE_STAGE3, ENVMAPMASK, ENVMAPMASKFRAME );
						else
							BindTexture( SHADER_TEXTURE_STAGE3, BASETEXTURE, FRAME );
			
						SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, BASETEXTURETRANSFORM, ENVMAPMASKSCALE );
					}

					if (IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE) ||
						IS_FLAG_SET(MATERIAL_VAR_ENVMAPCAMERASPACE))
					{
						LoadViewMatrixIntoVertexShaderConstant( VERTEX_SHADER_VIEWMODEL );
					}
					SetEnvMapTintPixelShaderDynamicState( 2, ENVMAPTINT, -1 );
				}

				if ( !hasEnvmap || hasBaseTexture || hasVertexColor )
				{
					SetModulationVertexShaderDynamicState();
				}
				EnablePixelShaderOverbright( 0, true, true );
				SetPixelShaderConstant( 1, SELFILLUMTINT );

				ff_lightmappedgeneric_vs11_Dynamic_Index vshIndex;
				vshIndex.SetDOWATERFOG( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
				pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
			}
			Draw();
		}

		if ( bBumpedEnvMap )
		{
			DrawWorldBumpedSpecularLighting(
				BUMPMAP, ENVMAP, BUMPFRAME, ENVMAPFRAME,
				ENVMAPTINT, ALPHA, ENVMAPCONTRAST, ENVMAPSATURATION,
				BUMPTRANSFORM, FRESNELREFLECTION,
				hasEnvmap || hasBaseTexture || hasVertexColor );
		}
	}

	void DrawDetailNoEnvmap( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool doSelfIllum )
	{
		SHADOW_STATE
		{
			// Alpha test
			pShaderShadow->EnableAlphaTest( IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) );

			// Base texture on stage 0
			if (params[BASETEXTURE]->IsTexture())
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );

			// Lightmap on stage 1
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );

			// Detail on stage 2
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );

			int fmt = VERTEX_POSITION;

			SetDefaultBlendingShadowState( BASETEXTURE, true );

			if (IS_FLAG_SET(MATERIAL_VAR_VERTEXCOLOR))
				fmt |= VERTEX_COLOR;

			pShaderShadow->VertexShaderVertexFormat( fmt, 2, 0, 0, 0 );

			ff_lightmappedgeneric_vs11_Static_Index vshIndex;
			vshIndex.SetDETAIL( true );
			vshIndex.SetENVMAP( false );
			vshIndex.SetENVMAPCAMERASPACE( false );
			vshIndex.SetENVMAPSPHERE( false );
			vshIndex.SetVERTEXCOLOR( IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) );
			pShaderShadow->SetVertexShader( "FF_LightmappedGeneric_vs11", vshIndex.GetIndex() );

			if (!params[BASETEXTURE]->IsTexture())
			{
				pShaderShadow->SetPixelShader("FF_LightmappedGeneric_DetailNoTexture");
			}
			else
			{
				if (!IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) || (!doSelfIllum))
				{
					pShaderShadow->SetPixelShader("FF_LightmappedGeneric_Detail");
				}
				else
				{
					pShaderShadow->SetPixelShader("FF_LightmappedGeneric_DetailSelfIlluminated");
				}
			}
			DefaultFog();
		}
		DYNAMIC_STATE
		{
			if (params[BASETEXTURE]->IsTexture())
			{
				BindTexture( SHADER_TEXTURE_STAGE0, BASETEXTURE, FRAME );
				SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, BASETEXTURETRANSFORM );
			}

			pShaderAPI->BindLightmap( SHADER_TEXTURE_STAGE1 );

			BindTexture( SHADER_TEXTURE_STAGE2, DETAIL, FRAME );
			SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, BASETEXTURETRANSFORM, DETAILSCALE );

			SetModulationVertexShaderDynamicState();
			EnablePixelShaderOverbright( 0, true, true );

			if (doSelfIllum)
			{
				SetPixelShaderConstant( 1, SELFILLUMTINT );
			}

			ff_lightmappedgeneric_vs11_Dynamic_Index vshIndex;
			vshIndex.SetDOWATERFOG( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
		Draw();
	}

	inline const char *GetAdditiveEnvmapPixelShaderName( bool usingMask, 
		bool usingBaseTexture, bool usingBaseAlphaEnvmapMask )
	{
		static char const* s_pPixelShaders[] = 
		{
			"FF_LightmappedGeneric_AddEnvmapNoTexture",
			"FF_LightmappedGeneric_AddEnvmapMaskNoTexture",
		};

		if ( !usingMask && usingBaseTexture && usingBaseAlphaEnvmapMask )
			return "FF_LightmappedGeneric_AddBaseAlphaMaskedEnvMap";

		int pshIndex = 0;
		if (usingMask)
			pshIndex |= 0x1;
		return s_pPixelShaders[pshIndex];
	}
	
	void DrawAdditiveEnvmap( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow )
	{
		bool usingBaseTexture = params[BASETEXTURE]->IsTexture();
		bool usingMask = params[ENVMAPMASK]->IsTexture();
		bool usingBaseAlphaEnvmapMask = IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		SHADOW_STATE
		{
			// Alpha test
			pShaderShadow->EnableAlphaTest( false );

			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, false );
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, false );

			// envmap on stage 2
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );

			// envmapmask on stage 3
			if (params[ENVMAPMASK]->IsTexture() || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK ) )
			{
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
			}

			if (params[BASETEXTURE]->IsTexture())
			{
				SetAdditiveBlendingShadowState( BASETEXTURE, true );
			}
			else
			{
				SetAdditiveBlendingShadowState( ENVMAPMASK, false );
			}

			int fmt = VERTEX_POSITION | VERTEX_NORMAL;

			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0, 0 );

			// Compute the vertex shader index.
			ff_lightmappedgeneric_vs11_Static_Index vshIndex;
			vshIndex.SetDETAIL( false );
			vshIndex.SetENVMAP( true );
			vshIndex.SetENVMAPCAMERASPACE( IS_FLAG_SET(MATERIAL_VAR_ENVMAPCAMERASPACE) );
			vshIndex.SetENVMAPSPHERE( IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE) );
			vshIndex.SetVERTEXCOLOR( false );
			s_pShaderShadow->SetVertexShader( "FF_LightmappedGeneric_vs11", vshIndex.GetIndex() );

			const char *pshName = GetAdditiveEnvmapPixelShaderName( usingMask, 
				usingBaseTexture, usingBaseAlphaEnvmapMask );
			pShaderShadow->SetPixelShader( pshName );
			FogToBlack();
		}
		DYNAMIC_STATE
		{
			BindTexture( SHADER_TEXTURE_STAGE2, ENVMAP, ENVMAPFRAME );

			if (usingMask || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK))
			{
				if (usingMask)
					BindTexture( SHADER_TEXTURE_STAGE3, ENVMAPMASK, ENVMAPMASKFRAME );
				else
					BindTexture( SHADER_TEXTURE_STAGE3, BASETEXTURE, FRAME );

				SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, BASETEXTURETRANSFORM, ENVMAPMASKSCALE );
			}

			SetPixelShaderConstant( 2, ENVMAPTINT );

			if (IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE) || IS_FLAG_SET(MATERIAL_VAR_ENVMAPCAMERASPACE))
			{
				LoadViewMatrixIntoVertexShaderConstant( VERTEX_SHADER_VIEWMODEL );
			}

			SetModulationVertexShaderDynamicState();

			// Compute the vertex shader index.
			ff_lightmappedgeneric_vs11_Dynamic_Index vshIndex;
			vshIndex.SetDOWATERFOG( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
		Draw();
	}

	void DrawDetailMode1( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bBumpedEnvMap )
	{
		// Mode 1 :
		// Pass 1 : B * L * D + Self Illum
		// Pass 2 : Add E * M

		// Draw the detail w/ no envmap
	 	DrawDetailNoEnvmap( params, pShaderAPI, pShaderShadow, true );

		if ( !bBumpedEnvMap )
		{
			DrawAdditiveEnvmap( params, pShaderAPI, pShaderShadow );
		}
		else
		{
			DrawWorldBumpedSpecularLighting(
				BUMPMAP, ENVMAP, BUMPFRAME, ENVMAPFRAME,
				ENVMAPTINT, ALPHA, ENVMAPCONTRAST, ENVMAPSATURATION,
				BUMPTRANSFORM, FRESNELREFLECTION,
				true );
		}
	}

	void DrawDetailUsingVertexShader( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bBumpedEnvMap )
	{
		// We don't have enough textures; gotta do this in two passes if there's envmapping
		if (!params[ENVMAP]->IsTexture())
		{
			DrawDetailNoEnvmap( params, pShaderAPI, pShaderShadow, IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) );
		}
		else
		{
			if (!params[BASETEXTURE]->IsTexture())
			{
				// If there's an envmap but no base texture, ignore detail
				DrawUnbumpedUsingVertexShader( params, pShaderAPI, pShaderShadow, bBumpedEnvMap );
			}
			else
			{
				DrawDetailMode1( params, pShaderAPI, pShaderShadow, bBumpedEnvMap );
			}
		}
	}

	SHADER_DRAW
	{
		bool hasFlashlight = UsingFlashlight( params );
		bool bBump = ShouldUseBumpmapping( params ) && params[BUMPMAP]->IsTexture() && 
			(params[NODIFFUSEBUMPLIGHTING]->GetIntValue() == 0);

		if( hasFlashlight )
		{
			DrawFlashlight_dx80( params, pShaderAPI, pShaderShadow, bBump, BUMPMAP, BUMPFRAME, BUMPTRANSFORM, 
				FLASHLIGHTTEXTURE, FLASHLIGHTTEXTUREFRAME, true, false, 0, -1, -1 );
		}
		else if( bBump )
		{
			DrawWorldBumpedUsingVertexShader( BASETEXTURE, BASETEXTURETRANSFORM,
				BUMPMAP, BUMPFRAME, BUMPTRANSFORM, ENVMAPMASK, ENVMAPMASKFRAME, ENVMAP, 
				ENVMAPFRAME, ENVMAPTINT, COLOR, ALPHA, ENVMAPCONTRAST, ENVMAPSATURATION, FRAME, FRESNELREFLECTION );
		}
		else
		{
			bool bBumpedEnvMap = ShouldUseBumpmapping( params ) && params[BUMPMAP]->IsTexture() && params[ENVMAP]->IsTexture();
			if (!params[DETAIL]->IsTexture())
			{
				DrawUnbumpedUsingVertexShader( params, pShaderAPI, pShaderShadow, bBumpedEnvMap );
			}
			else
			{
				DrawDetailUsingVertexShader( params, pShaderAPI, pShaderShadow, bBumpedEnvMap );
			}
		}
	}
END_SHADER


//-----------------------------------------------------------------------------
// Version that doesn't do bumpmapping
//-----------------------------------------------------------------------------
BEGIN_INHERITED_SHADER( FF_LightmappedGeneric_NoBump_DX8, FF_LightmappedGeneric_DX8,
			  "Help for FF_LightmappedGeneric_NoBump_DX8" )

	SHADER_FALLBACK
	{
		if (g_pHardwareConfig->GetDXSupportLevel() < 80)
			return "FF_LightmappedGeneric_DX6";

		return 0;
	}

	virtual bool ShouldUseBumpmapping( IMaterialVar **params ) 
	{
		if ( !g_pConfig->UseBumpmapping() )
			return false;

		if ( !params[BUMPMAP]->IsDefined() )
			return false;

		return ( params[FORCEBUMP]->GetIntValue() != 0 );
	}

END_INHERITED_SHADER
