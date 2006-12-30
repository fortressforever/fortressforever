//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//


#include "BaseVSShader.h"

#include "FF_refract_model_vs11.inc"
#include "FF_refract_world_vs11.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define MAXBLUR 1

DEFINE_FALLBACK_SHADER( FF_Refract, FF_Refract_DX80 )

BEGIN_VS_SHADER( FF_Refract_DX80, 
			  "Help for Refract_DX80" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM_OVERRIDE( COLOR, SHADER_PARAM_TYPE_COLOR, "{255 255 255}", "unused", SHADER_PARAM_NOT_EDITABLE )
		SHADER_PARAM_OVERRIDE( ALPHA, SHADER_PARAM_TYPE_FLOAT, "1.0", "unused", SHADER_PARAM_NOT_EDITABLE )
		SHADER_PARAM( REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "" )
		SHADER_PARAM( REFRACTTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "refraction tint" )
		SHADER_PARAM( DUDVMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_dudv", "dudv bump map" )
		SHADER_PARAM( NORMALMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "normal map" )
		SHADER_PARAM( BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap" )
		SHADER_PARAM( DUDVFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $dudvmap" )
		SHADER_PARAM( BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform" )
		SHADER_PARAM( TIME, SHADER_PARAM_TYPE_FLOAT, "0.0f", "" )
		SHADER_PARAM( BLURAMOUNT, SHADER_PARAM_TYPE_INTEGER, "1", "0, 1, or 2 for how much blur you want" )
		SHADER_PARAM( FADEOUTONSILHOUETTE, SHADER_PARAM_TYPE_BOOL, "1", "0 for no fade out on silhouette, 1 for fade out on sillhouette" )
		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap" )
		SHADER_PARAM( ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "envmap frame number" )
		SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint" )
		SHADER_PARAM( ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
		SHADER_PARAM( ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal" )
		SHADER_PARAM( REFRACTTINTTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shield", "" )
		SHADER_PARAM( REFRACTTINTTEXTUREFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( FRESNELREFLECTION, SHADER_PARAM_TYPE_FLOAT, "1.0", "0.0 == no fresnel, 1.0 == full fresnel" )
		SHADER_PARAM( FALLBACK, SHADER_PARAM_TYPE_STRING, "", "Name of the fallback shader" )
		SHADER_PARAM( FORCEREFRACT, SHADER_PARAM_TYPE_BOOL, "0", "Forces refraction on boards that have poor performance" )
		SHADER_PARAM( NOWRITEZ, SHADER_PARAM_TYPE_INTEGER, "0", "0 == write z, 1 = no write z" )
		SHADER_PARAM( MASKED, SHADER_PARAM_TYPE_BOOL, "0", "mask using dest alpha" )
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS2( MATERIAL_VAR2_NEEDS_TANGENT_SPACES );
		SET_FLAGS( MATERIAL_VAR_TRANSLUCENT );
		if( !params[ENVMAPTINT]->IsDefined() )
		{
			params[ENVMAPTINT]->SetVecValue( 1.0f, 1.0f, 1.0f );
		}
		if( !params[ENVMAPCONTRAST]->IsDefined() )
		{
			params[ENVMAPCONTRAST]->SetFloatValue( 0.0f );
		}
		if( !params[ENVMAPSATURATION]->IsDefined() )
		{
			params[ENVMAPSATURATION]->SetFloatValue( 1.0f );
		}
		if( !params[ENVMAPFRAME]->IsDefined() )
		{
			params[ENVMAPFRAME]->SetIntValue( 0 );
		}
		if( !params[FRESNELREFLECTION]->IsDefined() )
		{
			params[FRESNELREFLECTION]->SetFloatValue( 1.0f );
		}
		if( !params[MASKED]->IsDefined() )
		{
			params[MASKED]->SetIntValue( 0 );
		}
		if( !params[BLURAMOUNT]->IsDefined() )
		{
			params[BLURAMOUNT]->SetIntValue( 0 );
		}
		if( !params[FADEOUTONSILHOUETTE]->IsDefined() )
		{
			params[FADEOUTONSILHOUETTE]->SetIntValue( 0 );
		}
	}

	bool NeedsPowerOfTwoFrameBufferTexture( IMaterialVar **params ) const
	{
		return !params[BASETEXTURE]->IsDefined();
	}

	SHADER_FALLBACK
	{
		if ( IsPC() )
		{
			const char *pFallback = (params && params[FALLBACK]->IsDefined()) ? params[FALLBACK]->GetStringValue() : "";
			if (!pFallback[0])
			{
				pFallback = "FF_Refract_DX60";
			}

			if( g_pHardwareConfig->GetDXSupportLevel() < 80 || !g_pHardwareConfig->HasProjectedBumpEnv() )
				return pFallback;

			if ( g_pHardwareConfig->PreferReducedFillrate() && (params && (params[FORCEREFRACT]->GetIntValue() == 0)) )
				return pFallback;
		}

		return 0;
	}

	SHADER_INIT
	{
		if (params[BASETEXTURE]->IsDefined() )
		{
			LoadTexture( BASETEXTURE );
		}
		if (params[DUDVMAP]->IsDefined() )
		{
			LoadTexture( DUDVMAP );
		}
		if (params[NORMALMAP]->IsDefined() )
		{
			LoadBumpMap( NORMALMAP );
		}
		if( params[ENVMAP]->IsDefined() )
		{
			LoadCubeMap( ENVMAP );
		}
		if( params[REFRACTTINTTEXTURE]->IsDefined() )
		{
			LoadTexture( REFRACTTINTTEXTURE );
		}
	}

	inline int ComputePixelShaderIndex( bool bRefractTintTexture, bool bNormalMapAlpha )
	{
		// "REFRACTTINTTEXTURE" "0..1"
		// "NORMALMAPALPHA" "0..1"
		int pshIndex = 0;
		if( bRefractTintTexture ) pshIndex |= 0x1;
		if( bNormalMapAlpha ) pshIndex |= 0x2;
		return pshIndex;
	}
	
	SHADER_DRAW
	{
		bool bIsModel = IS_FLAG_SET( MATERIAL_VAR_MODEL );
		bool bHasEnvmap = params[ENVMAP]->IsTexture();
		int blurAmount = params[BLURAMOUNT]->GetIntValue();
		bool bRefractTintTexture = params[REFRACTTINTTEXTURE]->IsTexture();
		if( blurAmount < 0 )
		{
			blurAmount = 0;
		}
		else if( blurAmount > MAXBLUR )
		{
			blurAmount = MAXBLUR;
		}
		bool bMasked = (params[MASKED]->GetIntValue() != 0);

		SHADOW_STATE
		{
			if ( params[NOWRITEZ]->GetIntValue() != 0 )
			{
				pShaderShadow->EnableDepthWrites( false );
			}

			// Alpha test: FIXME: shouldn't this be handled in Shader_t::SetInitialShadowState
			pShaderShadow->EnableAlphaTest( IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) );

			// If envmap is not specified, the alpha channel is the translucency
			// (If envmap *is* specified, alpha channel is the reflection amount)
			bool bNormalMapAlpha = false;
			if ( params[NORMALMAP]->IsTexture() && !bHasEnvmap )
			{
				SetDefaultBlendingShadowState( NORMALMAP, false );
				if ( !bMasked && TextureIsTranslucent( NORMALMAP, false ) )
				{
					pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE3, true );
					bNormalMapAlpha = true;
				}
			}

			// dudv map
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
#ifdef _XBOX
			pShaderShadow->SetColorSign( SHADER_TEXTURE_STAGE0, true );
#endif
			// renderable texture for refraction
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE1, true );

			if( bRefractTintTexture )
			{
				// refract tint texture
				pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE2, true );
			}

			int fmt = VERTEX_POSITION | VERTEX_NORMAL;
			int numBoneWeights = 0;
			int userDataSize = 0;
			if( bIsModel )
			{
				numBoneWeights = 3;
				userDataSize = 4;
				fmt |= VERTEX_BONE_INDEX;
			}
			else
			{
				fmt |= VERTEX_TANGENT_S | VERTEX_TANGENT_T;
			}
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, numBoneWeights, userDataSize );

			if( bIsModel )
			{
				ff_refract_model_vs11_Static_Index vshIndex;
				pShaderShadow->SetVertexShader( "FF_Refract_model_vs11", vshIndex.GetIndex() );
			}
			else
			{
				ff_refract_world_vs11_Static_Index vshIndex;
				pShaderShadow->SetVertexShader( "FF_Refract_world_vs11", vshIndex.GetIndex() );
			}

			int pshIndex;
			pshIndex = ComputePixelShaderIndex( bRefractTintTexture, bNormalMapAlpha );
			pShaderShadow->SetPixelShader( "FF_Refract_ps11", pshIndex );

			if( bMasked )
			{
				EnableAlphaBlending( SHADER_BLEND_ONE_MINUS_SRC_ALPHA, SHADER_BLEND_SRC_ALPHA );
			}
			DefaultFog();
		}
		DYNAMIC_STATE
		{
			pShaderAPI->SetDefaultState();
#ifndef _XBOX
			// The dx9.0c runtime says that we shouldn't have a non-zero dimension when using vertex and pixel shaders.
			pShaderAPI->SetTextureTransformDimension( 1, 0, true );
#else
			// xboxissue - projected texture coord not available with texbem
			// divide must be in vsh
#endif
			if ( params[DUDVFRAME]->GetIntValue() == 0 )
			{
				BindTexture( SHADER_TEXTURE_STAGE0, DUDVMAP, BUMPFRAME );
			}
			else
			{
				BindTexture( SHADER_TEXTURE_STAGE0, DUDVMAP, DUDVFRAME );
			}

			if ( params[BASETEXTURE]->IsTexture() )
			{
				BindTexture( SHADER_TEXTURE_STAGE1, BASETEXTURE, FRAME );
			}
			else
			{
				pShaderAPI->BindFBTexture( SHADER_TEXTURE_STAGE1 );
			}

			if( bRefractTintTexture )
			{
				BindTexture( SHADER_TEXTURE_STAGE2, REFRACTTINTTEXTURE, REFRACTTINTTEXTUREFRAME );
			}

			if ( params[NORMALMAP]->IsTexture() )
			{
				BindTexture( SHADER_TEXTURE_STAGE3, NORMALMAP, BUMPFRAME );
			}

			float fRefractionAmount = params[REFRACTAMOUNT]->GetFloatValue();
			pShaderAPI->SetBumpEnvMatrix( 1, fRefractionAmount, 0.0f, 0.0f, fRefractionAmount );

			SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, BUMPTRANSFORM );

			// used to invert y
			// xboxfixme - move this into defined constants
			float c[4] = { 0.0f, 0.0f, 0.0f, -1.0f };
			pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, c, 1 );

			SetPixelShaderConstant( 0, REFRACTTINT );
			if( bIsModel )
			{
				ff_refract_model_vs11_Dynamic_Index vshIndex;
				vshIndex.SetDOWATERFOG( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
#if !defined( _XBOX )
				vshIndex.SetNUM_BONES( pShaderAPI->GetCurrentNumBones() );
#else
				vshIndex.SetSKINNING( pShaderAPI->GetCurrentNumBones() > 0 );
#endif
				pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
			}
			else
			{
				ff_refract_world_vs11_Dynamic_Index vshIndex;
				vshIndex.SetDOWATERFOG( pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
				pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
			}
		}

		Draw();

		if( bHasEnvmap )
		{
			bool bNoWriteZ = (params[NOWRITEZ]->GetIntValue() != 0);
			const int bBlendSpecular = true;
			if( bIsModel )
			{
				DrawModelBumpedSpecularLighting( NORMALMAP, BUMPFRAME,
					ENVMAP, ENVMAPFRAME,
					ENVMAPTINT, ALPHA,
					ENVMAPCONTRAST, ENVMAPSATURATION,
					BUMPTRANSFORM,
					bBlendSpecular, bNoWriteZ );
			}
			else
			{
				DrawWorldBumpedSpecularLighting( NORMALMAP, ENVMAP,
					BUMPFRAME, ENVMAPFRAME,
					ENVMAPTINT, ALPHA,
					ENVMAPCONTRAST, ENVMAPSATURATION,
					BUMPTRANSFORM, FRESNELREFLECTION,
					bBlendSpecular, bNoWriteZ );
			}
		}
	}
END_SHADER

