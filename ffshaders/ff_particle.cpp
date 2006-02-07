//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"

#include "ff_particle_vs11.inc"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


BEGIN_VS_SHADER( FF_Particle, 
			  "Help for FF_Particle" )
			   
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( USINGPIXELSHADER, SHADER_PARAM_TYPE_BOOL, "0", "Tells to client code whether the shader is using DX8 vertex/pixel shaders or not" )
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "base texture" )
		SHADER_PARAM( LIGHTS, SHADER_PARAM_TYPE_FOURCC, "", "array of lights" )
		SHADER_PARAM( LIGHT_POSITION, SHADER_PARAM_TYPE_VEC3, "0 0 0", "This is the directional light position." )
		SHADER_PARAM( LIGHT_COLOR, SHADER_PARAM_TYPE_VEC3, "1 1 1", "This is the directional light color." )
	END_SHADER_PARAMS

	bool UsePixelShaders( IMaterialVar **params ) const
	{
		return params[BUMPMAP]->IsDefined() && g_pHardwareConfig->SupportsVertexAndPixelShaders();
	}

	SHADER_INIT
	{
		// If this would return false, then we should have fallen back to the DX6 one.
		Assert( UsePixelShaders( params ) );

		params[USINGPIXELSHADER]->SetIntValue( true );
		LoadBumpMap( BUMPMAP );
	}

	SHADER_FALLBACK
	{
		// If their hardware doesn't support pixel shaders, or if there is 
		// no bump map specified in the materal, then fall back to a simple shader.
		if( !UsePixelShaders(params) )
		{
			return "UnlitGeneric_DX6";
		}
		return 0;
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
			
			int tCoordDimensions[] = {2};
			pShaderShadow->VertexShaderVertexFormat( 
				VERTEX_POSITION | VERTEX_COLOR, 1, tCoordDimensions, 0, 0 );

			pShaderShadow->EnableBlending( true );
			pShaderShadow->BlendFunc( SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA );
			pShaderShadow->EnableDepthWrites( false );
			
			ff_particle_vs11_Static_Index vshIndex;
			pShaderShadow->SetVertexShader( "ff_particle_vs11", vshIndex.GetIndex() );

			pShaderShadow->SetPixelShader( "ff_particle_ps11" );
			FogToFogColor();
		}
		DYNAMIC_STATE
		{
			BindTexture( SHADER_TEXTURE_STAGE0, BUMPMAP );

			pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, params[LIGHT_POSITION]->GetVecValue() );
			
			// Separate the light color into something that has a max value of 1 and a scale
			// so the vertex shader can determine if it's going to overflow the color and scale back
			// if it needs to.
			//
			// (It does this by seeing if the intensity*1/distSqr is > 1. If so, then it scales it so
			// it is equal to 1).
			const float *f = params[LIGHT_COLOR]->GetVecValue();
			Vector vLightColor( f[0], f[1], f[2] );
			float flScale = max( vLightColor.x, max( vLightColor.y, vLightColor.z ) );
			if ( flScale < 0.01f )
				flScale = 0.01f;
			float vScaleVec[3] = { flScale, flScale, flScale };
			vLightColor /= flScale;

			pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, vLightColor.Base() );
			pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_2, vScaleVec );

			// Compute the vertex shader index.
			ff_particle_vs11_Dynamic_Index vshIndex;
			vshIndex.SetFOGTYPE( s_pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z );
			s_pShaderAPI->SetVertexShaderIndex( vshIndex.GetIndex() );
		}
		Draw();
	}
END_SHADER
