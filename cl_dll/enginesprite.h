//===== Copyright � 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef ENGINESPRITE_H
#define ENGINESPRITE_H
#ifdef _WIN32
#pragma once
#endif

#include "vector.h"
#include "avi/iavi.h"


//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IMaterial;
class IMaterialVar;
typedef struct wrect_s wrect_t;


//-----------------------------------------------------------------------------
// Purpose: Sprite Models
//-----------------------------------------------------------------------------
class CEngineSprite
{
	// NOTE: don't define a constructor or destructor so that this can be allocated
	// as before.
public:
	int GetWidth() { return m_width; }
	int GetHeight() { return m_height; }
	int GetNumFrames() { return m_numFrames; }
	IMaterial *GetMaterial() { return m_material; } // hack - should keep this internal and draw internally.
	bool Init( const char *name );
	void Shutdown( void );
	void UnloadMaterial();
	void SetColor( float r, float g, float b );
	void SetAdditive( bool enable );
	void SetFrame( float frame );
	void SetRenderMode( int renderMode );
	int GetOrientation( void );
	void GetHUDSpriteColor( float* color );
	float GetUp() { return up; }
	float GetDown() { return down; }
	float GetLeft() { return left; }
	float GetRight() { return right; }
	void DrawFrame( int frame, int x, int y, const wrect_t *prcSubRect );
	void DrawFrameOfSize( int frame, int x, int y, int iWidth, int iHeight, const wrect_t *prcSubRect);
	bool IsAVI();
	void GetTexCoordRange( float *pMinU, float *pMinV, float *pMaxU, float *pMaxV );

private:
	AVIMaterial_t m_hAVIMaterial;
	int m_width;
	int m_height;
	int m_numFrames;
	IMaterial *m_material;
	int m_orientation;
	float m_hudSpriteColor[3];
	float up, down, left, right;
};

#endif // ENGINESPRITE_H
