//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VIEW_SCENE_H
#define VIEW_SCENE_H
#ifdef _WIN32
#pragma once
#endif


#include "convar.h"

#include "rendertexture.h"


extern ConVar mat_wireframe;


// Transform into view space (translate and rotate the camera into the origin).
void ViewTransform( const Vector &worldSpace, Vector &viewSpace );

// Transform a world point into normalized screen space (X and Y from -1 to 1).
// Returns 0 if the point is behind the viewer.
int ScreenTransform( const Vector& point, Vector& screen );

extern ConVar r_updaterefracttexture;

inline void UpdateRefractTexture( void )
{
	if ( !r_updaterefracttexture.GetBool() )
	{
		return;
	}
	ITexture *pTexture = GetPowerOfTwoFrameBufferTexture();
	materials->CopyRenderTargetToTexture( pTexture );
	materials->SetFrameBufferCopyTexture( pTexture );
}

inline void UpdateScreenEffectTexture( int textureIndex )
{
	ITexture *pTexture = GetFullFrameFrameBufferTexture( textureIndex );
	materials->CopyRenderTargetToTexture( pTexture );
	materials->SetFrameBufferCopyTexture( pTexture, textureIndex );
}

#endif // VIEW_SCENE_H
