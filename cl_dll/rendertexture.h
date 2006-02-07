//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// Define local cl_dll hooks to the renderable textures in material system
//
//=============================================================================//

#ifndef RENDERTARGETS_H
#define RENDERTARGETS_H

ITexture *GetPowerOfTwoFrameBufferTexture( void );
ITexture *GetFullFrameFrameBufferTexture( int textureIndex );
ITexture *GetWaterReflectionTexture( void );
ITexture *GetWaterRefractionTexture( void );
ITexture *GetCameraTexture( void );
ITexture *GetSmallBufferHDR0( void );
ITexture *GetSmallBufferHDR1( void );

#endif // RENDERTARGETS_H
