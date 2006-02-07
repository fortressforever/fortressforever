//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
// Implements local hooks into named renderable textures.
// See matrendertexture.cpp in material system for list of available RT's
//
//=============================================================================//

#include "materialsystem/imesh.h"
#include "materialsystem/ITexture.h"
#include "materialsystem/MaterialSystemUtil.h"
#include "vstdlib/strtools.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
// 
//=============================================================================
static CTextureReference s_pPowerOfTwoFrameBufferTexture;

ITexture *GetPowerOfTwoFrameBufferTexture( void )
{
	if( !s_pPowerOfTwoFrameBufferTexture )
	{
		s_pPowerOfTwoFrameBufferTexture.Init( materials->FindTexture( "_rt_PowerOfTwoFB", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pPowerOfTwoFrameBufferTexture ) );
	}
	
	return s_pPowerOfTwoFrameBufferTexture;
}

//=============================================================================
// 
//=============================================================================
static CTextureReference s_pCameraTexture;

ITexture *GetCameraTexture( void )
{
	if( !s_pCameraTexture )
	{
		s_pCameraTexture.Init( materials->FindTexture( "_rt_Camera", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pCameraTexture ) );
	}
	
	return s_pCameraTexture;
}

//=============================================================================
// 
//=============================================================================
static CTextureReference s_pFullFrameFrameBufferTexture[MAX_FB_TEXTURES];

ITexture *GetFullFrameFrameBufferTexture( int textureIndex )
{
	if( !s_pFullFrameFrameBufferTexture[textureIndex] )
	{
		char name[256];
		if( textureIndex != 0 )
		{
			sprintf( name, "_rt_FullFrameFB%d", textureIndex );
		}
		else
		{
			Q_strcpy( name, "_rt_FullFrameFB" );
		}
		s_pFullFrameFrameBufferTexture[textureIndex].Init( materials->FindTexture( name, TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pFullFrameFrameBufferTexture[textureIndex] ) );
	}
	
	return s_pFullFrameFrameBufferTexture[textureIndex];
}


//=============================================================================
// Water reflection
//=============================================================================
static CTextureReference s_pWaterReflectionTexture;

ITexture *GetWaterReflectionTexture( void )
{
	if( !s_pWaterReflectionTexture )
	{
		s_pWaterReflectionTexture.Init( materials->FindTexture( "_rt_WaterReflection", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pWaterReflectionTexture ) );
	}
	
	return s_pWaterReflectionTexture;
}

//=============================================================================
// Water refraction
//=============================================================================
static CTextureReference s_pWaterRefractionTexture;

ITexture *GetWaterRefractionTexture( void )
{
	if( !s_pWaterRefractionTexture )
	{
		s_pWaterRefractionTexture.Init( materials->FindTexture( "_rt_WaterRefraction", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pWaterRefractionTexture ) );
	}
	
	return s_pWaterRefractionTexture;
}


//=============================================================================
// 
//=============================================================================
static CTextureReference s_pSmallBufferHDR0;

ITexture *GetSmallBufferHDR0( void )
{
	if( !s_pSmallBufferHDR0 )
	{
		s_pSmallBufferHDR0.Init( materials->FindTexture( "_rt_SmallHDR0", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pSmallBufferHDR0 ) );
	}
	
	return s_pSmallBufferHDR0;
}

//=============================================================================
// 
//=============================================================================
static CTextureReference s_pSmallBufferHDR1;

ITexture *GetSmallBufferHDR1( void )
{
	if( !s_pSmallBufferHDR1 )
	{
		s_pSmallBufferHDR1.Init( materials->FindTexture( "_rt_SmallHDR1", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pSmallBufferHDR1 ) );
	}
	
	return s_pSmallBufferHDR1;
}

