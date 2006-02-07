//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Just about as simple as a shader gets. Specify a vertex
//          and pixel shader, bind textures, and that's it.
//
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"
#include "convar.h"


/*

NOTE: this shader needs to be called from the client DLL. You could insert this code at the end of CViewRender::RenderView.

if ( g_pMaterialSystemHardwareConfig->GetDXSupportLevel() >= 80 )
{
		// Get whatever material references your postprocess shader.
		IMaterial *pMaterial = materials->FindMaterial( "ff/ff_postprocess", TEXTURE_GROUP_CLIENT_EFFECTS, true );
		if ( pMaterial )
		{
				// This copies the contents of the framebuffer (drawn during RenderView) into a texture that your shader can use.
				UpdateScreenEffectTexture( 0 );

				materials->MatrixMode( MATERIAL_PROJECTION );
				materials->PushMatrix();
				materials->LoadIdentity();	

				materials->MatrixMode( MATERIAL_VIEW );
				materials->PushMatrix();
				materials->LoadIdentity();	

				materials->DrawScreenSpaceQuad( pMaterial );

				materials->MatrixMode( MATERIAL_PROJECTION );
				materials->PopMatrix();
				materials->MatrixMode( MATERIAL_VIEW );
				materials->PopMatrix();
		}
}

*/


BEGIN_VS_SHADER( FF_PostProcess, "Help for FF_PostProcess" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( FILTERSIZE, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
		SHADER_PARAM( SHARPENFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
	END_SHADER_PARAMS

	// Set up anything that is necessary to make decisions in SHADER_FALLBACK.
	SHADER_INIT_PARAMS()
	{
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	bool NeedsFullFrameBufferTexture( IMaterialVar **params ) const
	{
		return true;
	}

	SHADER_INIT
	{
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			// Enable the texture for base texture and lightmap.
			pShaderShadow->EnableTexture( SHADER_TEXTURE_STAGE0, true );
			pShaderShadow->EnableDepthWrites( false );

			int fmt = VERTEX_POSITION;
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0, 0 );

			pShaderShadow->SetVertexShader( "ff_postprocess_vs20", 0 );
			pShaderShadow->SetPixelShader( "ff_postprocess_ps20" );

			// Optional, do some blending..
			//pShaderShadow->EnableBlending( true );
			//pShaderShadow->BlendFunc( SHADER_BLEND_DST_COLOR, SHADER_BLEND_SRC_COLOR );

			DefaultFog();
		}
		DYNAMIC_STATE
		{
			// Tell it how wide the blur is.
			int width, height;
			pShaderAPI->GetBackBufferDimensions( width, height );

			float v[4] = {0,0,0,0};
			v[0] = params[FILTERSIZE]->GetFloatValue() / width;
			pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, v, 1 );

			float factor[4];
			factor[0] = params[SHARPENFACTOR]->GetFloatValue();
			factor[1] = factor[2] = factor[3] = factor[0];
			pShaderAPI->SetPixelShaderConstant( 0, factor );

			pShaderAPI->BindFBTexture( SHADER_TEXTURE_STAGE0 );
			pShaderAPI->SetVertexShaderIndex( 0 );
		}
		Draw();
	}
END_SHADER
