//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"

#include "ff_vertexlitgeneric_vs11.inc"
#include "ff_vertexlitgeneric_selfillumonly.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

DEFINE_FALLBACK_SHADER( FF_VertexLitGeneric, FF_VertexLitGeneric_DX8 )

BEGIN_VS_SHADER( FF_VertexLitGeneric_DX8, 
				"Help for FF_VertexLitGeneric" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint" )
		SHADER_PARAM( DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture" )
		SHADER_PARAM( DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture" )
		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap" )
		SHADER_PARAM( ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "", "envmap frame number" )
		SHADER_PARAM( ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTimesLightmapPlusMaskedCubicEnvMap_glass", "envmap mask" )
		SHADER_PARAM( ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "", "" )
		SHADER_PARAM( ENVMAPMASKSCALE, SHADER_PARAM_TYPE_FLOAT, "1", "envmap mask scale" )
		SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint" )
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/WorldDiffuseBumpMap_bump", "bump map" )
		SHADER_PARAM( BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap" )
		SHADER_PARAM( BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform" )
		SHADER_PARAM( ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
		SHADER_PARAM( ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal" )
		SHADER_PARAM( ENVMAPOPTIONAL, SHADER_PARAM_TYPE_BOOL, "0", "Make the envmap only apply to dx9 and higher hardware" )
		SHADER_PARAM( FORCEBUMP, SHADER_PARAM_TYPE_BOOL, "0", "0 == Do bumpmapping if the card says it can handle it. 1 == Always do bumpmapping." )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		// FLASHLIGHTFIXME
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );

		// We don't want no stinking bump mapping on models in dx8.
		// Wait a minute!  We want specular bump. .need to make that work by itself.
//		params[BUMPMAP]->SetUndefined();
//		if( IS_FLAG_SET( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK ) )
//		{
//			CLEAR_FLAGS( MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK );
//			params[ENVMAP]->SetUndefined();
//		}
		// default to 'MODEL' mode...
		if (!IS_FLAG_DEFINED( MATERIAL_VAR_MODEL ))
			SET_FLAGS( MATERIAL_VAR_MODEL );

		if( !params[ENVMAPMASKSCALE]->IsDefined() )
			params[ENVMAPMASKSCALE]->SetFloatValue( 1.0f );

		if( !params[ENVMAPMASKFRAME]->IsDefined() )
			params[ENVMAPMASKFRAME]->SetIntValue( 0 );
		
		if( !params[ENVMAPTINT]->IsDefined() )
			params[ENVMAPTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );

		if( !params[SELFILLUMTINT]->IsDefined() )
			params[SELFILLUMTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );

		if( !params[DETAILSCALE]->IsDefined() )
			params[DETAILSCALE]->SetFloatValue( 4.0f );

		if( !params[ENVMAPCONTRAST]->IsDefined() )
			params[ENVMAPCONTRAST]->SetFloatValue( 0.0f );
		
		if( !params[ENVMAPSATURATION]->IsDefined() )
			params[ENVMAPSATURATION]->SetFloatValue( 1.0f );
		
		if( !params[ENVMAPFRAME]->IsDefined() )
			params[ENVMAPFRAME]->SetIntValue( 0 );

		if( !params[BUMPFRAME]->IsDefined() )
			params[BUMPFRAME]->SetIntValue( 0 );

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

		// Force software skinning if the file says so
		if (!IS_FLAG_SET(MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING))
		{
#ifdef DIFFUSE_BUMP_ON_DX8
			// Software skin if diffuse bump mapping is used at all.
			if( g_pConfig->bBumpmap && 
				( ( params[BASETEXTURE]->IsDefined() && params[BUMPMAP]->IsDefined() ) ||
				  ( params[BUMPMAP]->IsDefined() && !params[ENVMAP]->IsDefined() ) ) )
			{
				SET_FLAGS( MATERIAL_VAR_NEEDS_SOFTWARE_SKINNING );
			}
#endif
		}

		if( g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined() )
		{
			SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
		}
		
		SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );

		if( !params[DETAIL]->IsDefined() && !params[BUMPMAP]->IsDefined() )
		{
			SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_TEXKILL );
		}

		// Get rid of the envmap if it's optional for this dx level.
		if( params[ENVMAPOPTIONAL]->IsDefined() && params[ENVMAPOPTIONAL]->GetIntValue() )
		{
			params[ENVMAP]->SetUndefined();
		}

		// If mat_specular 0, then get rid of envmap
		if( !g_pConfig->UseSpecular() && params[ENVMAP]->IsDefined() && params[BASETEXTURE]->IsDefined() )
		{
			params[ENVMAP]->SetUndefined();
		}
	}

	SHADER_FALLBACK
	{	
		if (g_pHardwareConfig->GetDXSupportLevel() < 70)
			return "FF_VertexLitGeneric_DX6";

		if (g_pHardwareConfig->GetDXSupportLevel() < 80)
			return "FF_VertexLitGeneric_DX7";

		if ( g_pHardwareConfig->PreferReducedFillrate() )
			return "FF_VertexLitGeneric_NoBump_DX8";

		return 0;
	}

	SHADER_INIT
	{
		LoadTexture( FLASHLIGHTTEXTURE );

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

		if (g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined())
		{
			LoadBumpMap( BUMPMAP );
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
	}

	inline const char *GetUnbumpedPixelShaderName( IMaterialVar** params, bool bSkipEnvmap )
	{
		static char const* s_pPixelShaders[] = 
		{
			"FF_VertexLitGeneric_EnvmapV2",
			"FF_VertexLitGeneric_SelfIlluminatedEnvmapV2",

			"FF_VertexLitGeneric_BaseAlphaMaskedEnvmapV2",
			"FF_VertexLitGeneric_SelfIlluminatedEnvmapV2",

			// Env map mask
			"FF_VertexLitGeneric_MaskedEnvmapV2",
			"FF_VertexLitGeneric_SelfIlluminatedMaskedEnvmapV2",

			"FF_VertexLitGeneric_MaskedEnvmapV2",
			"FF_VertexLitGeneric_SelfIlluminatedMaskedEnvmapV2",

			// Detail
			"FF_VertexLitGeneric_DetailEnvmapV2",
			"FF_VertexLitGeneric_DetailSelfIlluminatedEnvmapV2",

			"FF_VertexLitGeneric_DetailBaseAlphaMaskedEnvmapV2",
			"FF_VertexLitGeneric_DetailSelfIlluminatedEnvmapV2",

			// Env map mask
			"FF_VertexLitGeneric_DetailMaskedEnvmapV2",
			"FF_VertexLitGeneric_DetailSelfIlluminatedMaskedEnvmapV2",

			"FF_VertexLitGeneric_DetailMaskedEnvmapV2",
			"FF_VertexLitGeneric_DetailSelfIlluminatedMaskedEnvmapV2",
		};

		if (!params[BASETEXTURE]->IsTexture())
		{
			if (params[ENVMAP]->IsTexture() && !bSkipEnvmap )
			{
				if (!params[ENVMAPMASK]->IsTexture())
				{
					return "FF_VertexLitGeneric_EnvmapNoTexture";
				}
				else
				{
					return "FF_VertexLitGeneric_MaskedEnvmapNoTexture";
				}
			}
			else
			{
				if (params[DETAIL]->IsTexture())
				{
					return "FF_VertexLitGeneric_DetailNoTexture";
				}
				else
				{
					return "FF_VertexLitGeneric_NoTexture";
				}
			}
		}
		else
		{
			if (params[ENVMAP]->IsTexture() && !bSkipEnvmap )
			{
				int pshIndex = 0;
				if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM))
					pshIndex |= 0x1;
				if (IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK))
					pshIndex |= 0x2;
				if (params[ENVMAPMASK]->IsTexture())
					pshIndex |= 0x4;
				if (params[DETAIL]->IsTexture())
					pshIndex |= 0x8;
				return s_pPixelShaders[pshIndex];
			}
			else
			{
				if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM))
				{
					if (params[DETAIL]->IsTexture())
						return "FF_VertexLitGeneric_DetailSelfIlluminated";
					else
						return "FF_VertexLitGeneric_SelfIlluminated";
				}
				else
					if (params[DETAIL]->IsTexture())
						return "FF_VertexLitGeneric_Detail";
					else
						return "FF_VertexLitGeneric";
			}
		}
	}

	void DrawUnbumpedUsingVertexShader( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bSkipEnvmap )
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );


			pShaderShadow->EnableAlphaTest( IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) );

			int fmt = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_BONE_INDEX;

			// FIXME: We could enable this, but we'd never get it working on dx7 or lower
			// FIXME: This isn't going to work until we make more vertex shaders that
			// pass the vertex color and alpha values through.
#if 0
			if ( IS_FLAG_SET( MATERIAL_VAR_VERTEXCOLOR ) || IS_FLAG_SET( MATERIAL_VAR_VERTEXALPHA ) )
				fmt |= VERTEX_COLOR;
#endif

			if (params[ENVMAP]->IsTexture() && !bSkipEnvmap )
			{
				// envmap on stage 1
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );

				// envmapmask on stage 2
				if (params[ENVMAPMASK]->IsTexture() || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK ) )
				{
					pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
				}
			}

			if (params[BASETEXTURE]->IsTexture())
			{
				SetDefaultBlendingShadowState( BASETEXTURE, true );
			}
			else
			{
				SetDefaultBlendingShadowState( ENVMAPMASK, false );
			}

 			if (params[DETAIL]->IsTexture())
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );

			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 3, 0 );

			// Set up the vertex shader index.
			ff_vertexlitgeneric_vs11_Static_Index vshIndex;
			vshIndex.SetHALF_LAMBERT( IS_FLAG_SET( MATERIAL_VAR_HALFLAMBERT ) );
			vshIndex.SetDETAIL( params[DETAIL]->IsTexture() );
			if( params[ENVMAP]->IsTexture() && !bSkipEnvmap )
			{
				vshIndex.SetENVMAP( true );
				vshIndex.SetENVMAPCAMERASPACE( IS_FLAG_SET(MATERIAL_VAR_ENVMAPCAMERASPACE) );
				if( IS_FLAG_SET(MATERIAL_VAR_ENVMAPCAMERASPACE) )
				{
					vshIndex.SetENVMAPSPHERE( false );
				}
				else
				{
					vshIndex.SetENVMAPSPHERE( IS_FLAG_SET( MATERIAL_VAR_ENVMAPSPHERE ) );
				}
			}
			else
			{
				vshIndex.SetENVMAP( false );
				vshIndex.SetENVMAPCAMERASPACE( false );
				vshIndex.SetENVMAPSPHERE( false );
			}
			vshIndex.SetDECAL( false );
			pShaderShadow->SetVertexShader( "ff_vertexlitgeneric_vs11", vshIndex.GetIndex() );

			const char *pshName = GetUnbumpedPixelShaderName( params, bSkipEnvmap );
			pShaderShadow->SetPixelShader( pshName );
			DefaultFog();
		}
		DYNAMIC_STATE
		{
			if (params[BASETEXTURE]->IsTexture())
			{
				BindTexture( SHADER_TEXTURE_STAGE0, BASETEXTURE, FRAME );
				SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, BASETEXTURETRANSFORM );
			}

//			if (params[ENVMAP]->IsTexture())
			if (params[ENVMAP]->IsTexture() && !bSkipEnvmap )
			{
				BindTexture( SHADER_TEXTURE_STAGE1, ENVMAP, ENVMAPFRAME );

				if (params[ENVMAPMASK]->IsTexture() || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK) )
				{
					if (params[ENVMAPMASK]->IsTexture() )
						BindTexture( SHADER_TEXTURE_STAGE2, ENVMAPMASK, ENVMAPMASKFRAME );
					else
						BindTexture( SHADER_TEXTURE_STAGE2, BASETEXTURE, FRAME );
		
					SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, BASETEXTURETRANSFORM, ENVMAPMASKSCALE );
				}

				if (IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE) || 
					IS_FLAG_SET(MATERIAL_VAR_ENVMAPCAMERASPACE))
				{
					LoadViewMatrixIntoVertexShaderConstant( VERTEX_SHADER_VIEWMODEL );
				}
				SetEnvMapTintPixelShaderDynamicState( 2, ENVMAPTINT, -1 );
			}

			if (params[DETAIL]->IsTexture())
			{
				BindTexture( SHADER_TEXTURE_STAGE3, DETAIL, FRAME );
				SetVertexShaderTextureScaledTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, BASETEXTURETRANSFORM, DETAILSCALE );
			}

			SetAmbientCubeDynamicStateVertexShader();
			SetModulationPixelShaderDynamicState( 3 );
			EnablePixelShaderOverbright( 0, true, true );
			SetPixelShaderConstant( 1, SELFILLUMTINT );

			ff_vertexlitgeneric_vs11_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			vshIndex.SetNUM_BONES( pShaderAPI->GetCurrentNumBones() );
			vshIndex.SetLIGHT_COMBO( pShaderAPI->GetCurrentLightCombo() );
			pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
		Draw();
	}

	inline const char *GetBumpedDiffusePixelShaderName( IMaterialVar **params )
	{
		static char const* s_pPixelShaders[] = 
		{
			"FF_VertexLitGeneric_DiffBumpLightingOnly_Overbright2",
			"FF_VertexLitGeneric_DiffBumpLightingOnly_Overbright2_Translucent",
			"FF_VertexLitGeneric_DiffBumpTimesBase_Overbright2",
			"FF_VertexLitGeneric_DiffBumpTimesBase_Overbright2_Translucent",
		};

		
		int pixelShaderID = 0x0;
		if( params[BASETEXTURE]->IsTexture() && TextureIsTranslucent( BASETEXTURE, true ) )
		{
			pixelShaderID |= 0x1;
		}
		if( params[BASETEXTURE]->IsTexture() )
		{
			pixelShaderID |= 0x2;
		}
		return s_pPixelShaders[pixelShaderID];
	}
	
	void DrawBumpedDiffusePass( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow )
	{
		Assert( 0 ); // We don't do bumped diffuse on dx8 anymore.
/*
		SHADOW_STATE
		{
			SET_FLAGS2( MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL );
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );
			
			SetDefaultBlendingShadowState( BASETEXTURE, true );

			int fmt = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_BONE_INDEX;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 3, 4 );
			pShaderShadow->SetVertexShader( "VertexLitGeneric_DiffBumpTimesBase" );
			const char *pshName = GetBumpedDiffusePixelShaderName( params );
			pShaderShadow->SetPixelShader( pshName );
		}
		DYNAMIC_STATE
		{

			if( params[BASETEXTURE]->IsTexture() )
			{
				BindTexture( SHADER_TEXTURE_STAGE0, BASETEXTURE, FRAME );
			}
			BindTexture( SHADER_TEXTURE_STAGE1, BUMPMAP, BUMPFRAME );
			SetAmbientCubeDynamicStateVertexShader();

			LoadBumpLightmapCoordinateAxes_VertexShader( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0 );
			LoadBumpLightmapCoordinateAxes_PixelShader( 0 );
			
			//SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_3, BASETEXTURETRANSFORM );
			SetModulationPixelShaderDynamicState( 3 );
		}
		Draw();
*/
	}
	
	void DrawBumpedSelfIllumPass( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow )
	{
		SHADOW_STATE
		{
			SetInitialShadowState( );
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
			// Don't bother with z writes here...
 			pShaderShadow->EnableDepthWrites( false );
			// Override the Modulation+blending shadow State this time; we're always modulating+blending
			pShaderShadow->EnableBlending( true );
			pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			int fmt = VERTEX_POSITION | VERTEX_BONE_INDEX;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, NULL, 3, 0 /* userDataSize */ );

			ff_vertexlitgeneric_selfillumonly_Static_Index vshIndex;
			pShaderShadow->SetVertexShader( "FF_VertexLitGeneric_SelfIllumOnly", vshIndex.GetIndex() );

			pShaderShadow->SetPixelShader( "FF_VertexLitGeneric_SelfIllumOnly" );
		}
					
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();
			// keep the base texture bound from the previous pass.

			float constantColor[4];
			params[SELFILLUMTINT]->GetVecValue( constantColor, 3 );
			constantColor[3] = params[ALPHA]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant( 0, constantColor, 1 );

			ff_vertexlitgeneric_selfillumonly_Dynamic_Index vshIndex;
			vshIndex.SetFOG_TYPE( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			vshIndex.SetNUM_BONES( pShaderAPI->GetCurrentNumBones() );
			pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
		Draw();
	}
	

	void DrawBumpedUsingVertexShader( IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow )
	{
		bool bBlendSpecular = false;
		// diffuse pass
		if( ( params[BASETEXTURE]->IsTexture() && params[BUMPMAP]->IsTexture() ) ||
			( params[BUMPMAP]->IsTexture() && !params[ENVMAP]->IsTexture() ) )
		{
			bBlendSpecular = true;
			DrawBumpedDiffusePass( params, pShaderAPI, pShaderShadow );
		}
		
		// OPTIMIZE!!  Can we fit self-illum and self-illum tint into the previous pass?
		// In some case, we can, but probably not the common cases.
		if( IS_FLAG_SET( MATERIAL_VAR_SELFILLUM ) )
		{
			DrawBumpedSelfIllumPass( params, pShaderAPI, pShaderShadow );
		}
		
		// specular pass
		if( params[ENVMAP]->IsTexture() )
		{
			DrawModelBumpedSpecularLighting( BUMPMAP, BUMPFRAME,
				ENVMAP, ENVMAPFRAME,
				ENVMAPTINT, ALPHA,
				ENVMAPCONTRAST, ENVMAPSATURATION,
				BUMPTRANSFORM,
				bBlendSpecular );
		}
	}

	SHADER_DRAW
	{
		// FLASHLIGHTFIXME: need to make these the same.
		bool hasFlashlight = UsingFlashlight( params );
		bool bBump = g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsTexture();

		if( hasFlashlight )
		{
			DrawFlashlight_dx80( params, pShaderAPI, pShaderShadow, bBump, BUMPMAP, BUMPFRAME, BUMPTRANSFORM, 
				FLASHLIGHTTEXTURE, FLASHLIGHTTEXTUREFRAME, false, false, 0, -1, -1 );
		}
		else if( bBump )
		{
#ifdef DIFFUSE_BUMP_ON_DX8
			DrawBumpedUsingVertexShader( params, pShaderAPI, pShaderShadow );
#else
			bool bSkipEnvmap = true;
			DrawUnbumpedUsingVertexShader( params, pShaderAPI, pShaderShadow, bSkipEnvmap );

			// specular pass
			bool bBlendSpecular = true;
			if( params[ENVMAP]->IsTexture() )
			{
				DrawModelBumpedSpecularLighting( BUMPMAP, BUMPFRAME, ENVMAP, ENVMAPFRAME,
					ENVMAPTINT, ALPHA, ENVMAPCONTRAST, ENVMAPSATURATION, BUMPTRANSFORM, bBlendSpecular );
			}
#endif
		}
		else
		{
			bool bSkipEnvmap = false;
			DrawUnbumpedUsingVertexShader( params, pShaderAPI, pShaderShadow, bSkipEnvmap );
		}
	}
END_SHADER


//-----------------------------------------------------------------------------
// Version that doesn't do bumpmapping
//-----------------------------------------------------------------------------
BEGIN_INHERITED_SHADER( FF_VertexLitGeneric_NoBump_DX8, FF_VertexLitGeneric_DX8,
			  "Help for FF_VertexLitGeneric_NoBump_DX8" )

	SHADER_FALLBACK
	{
		if (g_pConfig->bSoftwareLighting)
			return "FF_VertexLitGeneric_DX6";

		if (!g_pHardwareConfig->SupportsVertexAndPixelShaders())
			return "FF_VertexLitGeneric_DX7";

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

