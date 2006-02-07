//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "enginesprite.h"
#include "hud.h"
#include "materialsystem/imesh.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "c_sprite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Sprites are clipped to this rectangle (x,y,width,height) if ScissorTest is enabled
static int scissor_x = 0;
static int scissor_y = 0;
static int scissor_width = 0;
static int scissor_height = 0;
static bool giScissorTest = false;

//-----------------------------------------------------------------------------
// Purpose: 
// Set the scissor
//  the coordinate system for gl is upsidedown (inverted-y) as compared to software, so the
//  specified clipping rect must be flipped
// Input  : x - 
//			y - 
//			width - 
//			height - 
//-----------------------------------------------------------------------------
void EnableScissorTest( int x, int y, int width, int height )
{
	x = clamp( x, 0, ScreenWidth() );
	y = clamp( y, 0, ScreenHeight() );
	width = clamp( width, 0, ScreenWidth() - x );
	height = clamp( height, 0, ScreenHeight() - y );

	scissor_x = x;
	scissor_width = width;
	scissor_y = y;
	scissor_height = height;

	giScissorTest = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void DisableScissorTest( void )
{
	scissor_x = 0;
	scissor_width = 0;
	scissor_y = 0;
	scissor_height = 0;
	
	giScissorTest = false;
}

//-----------------------------------------------------------------------------
// Purpose: Verify that this is a valid, properly ordered rectangle.
// Input  : *prc - 
// Output : int
//-----------------------------------------------------------------------------
static int ValidateWRect(const wrect_t *prc)
{

	if (!prc)
		return false;

	if ((prc->left >= prc->right) || (prc->top >= prc->bottom))
	{
		//!!!UNDONE Dev only warning msg
		return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: classic interview question
// Input  : *prc1 - 
//			*prc2 - 
//			*prc - 
// Output : int
//-----------------------------------------------------------------------------
static int IntersectWRect(const wrect_t *prc1, const wrect_t *prc2, wrect_t *prc)
{	
	wrect_t rc;

	if (!prc)
		prc = &rc;

	prc->left = max(prc1->left, prc2->left);
	prc->right = min(prc1->right, prc2->right);

	if (prc->left < prc->right)
	{
		prc->top = max(prc1->top, prc2->top);
		prc->bottom = min(prc1->bottom, prc2->bottom);

		if (prc->top < prc->bottom)
			return 1;

	}

	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : x - 
//			y - 
//			width - 
//			height - 
//			u0 - 
//			v0 - 
//			u1 - 
//			v1 - 
// Output : static bool
//-----------------------------------------------------------------------------
static bool Scissor( int& x, int& y, int& width, int& height, float& u0, float& v0, float& u1, float& v1 )
{
	// clip sub rect to sprite
	if ((width == 0) || (height == 0))
		return false;

	if ((x + width <= scissor_x) || (x >= scissor_x + scissor_width) ||
		(y + height <= scissor_y) || (y >= scissor_y + scissor_height))
		return false;

	float dudx = (u1-u0) / width;
	float dvdy = (v1-v0) / height;
	if (x < scissor_x)
	{
		u0 += (scissor_x - x) * dudx;
		width -= scissor_x - x;
		x = scissor_x;
	}

	if (x + width > scissor_x + scissor_width)
	{
		u1 -= (x + width - (scissor_x + scissor_width)) * dudx;
		width = scissor_x + scissor_width - x;
	}

	if (y < scissor_y)
	{
		v0 += (scissor_y - y) * dvdy;
		height -= scissor_y - y;
		y = scissor_y;
	}

	if (y + height > scissor_y + scissor_height)
	{
		v1 -= (y + height - (scissor_y + scissor_height)) * dvdy;
		height = scissor_y + scissor_height - y;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSprite - 
//			frame - 
//			*pfLeft - 
//			*pfRight - 
//			*pfTop - 
//			*pfBottom - 
//			*pw - 
//			*ph - 
//			*prcSubRect - 
// Output : static void
//-----------------------------------------------------------------------------
static void AdjustSubRect(CEngineSprite *pSprite, int frame, float *pfLeft, float *pfRight, float *pfTop, 
						  float *pfBottom, int *pw, int *ph, const wrect_t *prcSubRect)
{
	wrect_t rc;
	float f;

	if (!ValidateWRect(prcSubRect))
		return;

	// clip sub rect to sprite

	rc.top = rc.left = 0;
	rc.right = *pw;
	rc.bottom = *ph;

	if (!IntersectWRect(prcSubRect, &rc, &rc))
		return;

	*pw = rc.right - rc.left;
	*ph = rc.bottom - rc.top;

	f = 1.0 / (float)pSprite->GetWidth();;
	*pfLeft = ((float)rc.left + 0.5) * f;
	*pfRight = ((float)rc.right - 0.5) * f;

	f = 1.0 / (float)pSprite->GetHeight();
	*pfTop = ((float)rc.top + 0.5) * f;
	*pfBottom = ((float)rc.bottom - 0.5) * f;

	return;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pName - 
// Output : static IMaterial
//-----------------------------------------------------------------------------

static IMaterial *FindSpriteMaterial( const char *pName )
{
	IMaterial *material;
	
	material = materials->FindMaterial( pName, TEXTURE_GROUP_CLIENT_EFFECTS );
	return material;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEngineSprite::Shutdown( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CEngineSprite::Init( const char *name )
{
	m_material = FindSpriteMaterial( name );
	if( m_material )
	{
		m_width = m_material->GetMappingWidth();
		m_height = m_material->GetMappingHeight();
		m_numFrames = m_material->GetNumAnimationFrames();

		bool found;
		IMaterialVar *orientationVar = m_material->FindVar( "$spriteorigin", &found, false );
		if( found )
		{
			m_orientation = orientationVar->GetIntValue();
		}
		else
		{
			m_orientation = C_SpriteRenderer::SPR_VP_PARALLEL_UPRIGHT;
		}

		IMaterialVar *originVar = m_material->FindVar( "$spriteorigin", &found, false );
		Vector origin, originVarValue;
		if( !found || ( originVar->GetType() != MATERIAL_VAR_TYPE_VECTOR ) )
		{
			origin[0] = -m_width * 0.5f;
			origin[1] = m_height * 0.5f;
		}
		else
		{
			originVar->GetVecValue( &originVarValue[0], 3 );
			origin[0] = -m_width * originVarValue[0];
			origin[1] = m_height * originVarValue[1];
		}

		up = origin[1];
		down = origin[1] - m_height;
		left = origin[0];
		right = m_width + origin[0];
	}
	return m_material ? true : false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : r - 
//			g - 
//			b - 
//-----------------------------------------------------------------------------
void CEngineSprite::SetColor( float r, float g, float b )
{
	assert( (r >= 0.0) && (g >= 0.0) && (b >= 0.0) );
	assert( (r <= 1.0) && (g <= 1.0) && (b <= 1.0) );
	m_hudSpriteColor[0] = r;
	m_hudSpriteColor[1] = g;
	m_hudSpriteColor[2] = b;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : color - 
//-----------------------------------------------------------------------------
void CEngineSprite::GetHUDSpriteColor( float* color )
{
	VectorCopy( m_hudSpriteColor, color );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : additive - 
//-----------------------------------------------------------------------------
void CEngineSprite::SetAdditive( bool additive )
{
	SetRenderMode( additive ? kRenderTransAdd : kRenderTransTexture ); 
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : frame - 
//-----------------------------------------------------------------------------
void CEngineSprite::SetFrame( float frame )
{
	bool found;
	IMaterialVar* pFrameVar = m_material->FindVar( "$frame", &found, false );
	if (found)
		pFrameVar->SetFloatValue( frame );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : renderMode - 
//-----------------------------------------------------------------------------

void CEngineSprite::SetRenderMode( int renderMode )
{
	bool found;
	IMaterialVar* pRenderModeVar = m_material->FindVar( "$spriteRenderMode", &found, false );
	if (found)
	{
		if ( pRenderModeVar->GetIntValue() != renderMode )
		{
			pRenderModeVar->SetIntValue( renderMode );
			m_material->RecomputeStateSnapshots();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : int
//-----------------------------------------------------------------------------
int CEngineSprite::GetOrientation( void )
{
	bool found;
	IMaterialVar *orientationVar = m_material->FindVar( "$spriteorientation", &found, false );
	if( found )
	{
		return orientationVar->GetIntValue();
	}
	else
	{
		return C_SpriteRenderer::SPR_VP_PARALLEL_UPRIGHT;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEngineSprite::UnloadMaterial( void )
{
	if( m_material )
	{
		m_material->DecrementReferenceCount();
	}
	m_material = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : frame - 
//			x - 
//			y - 
//			*prcSubRect - 
//-----------------------------------------------------------------------------
void CEngineSprite::DrawFrame( int frame, int x, int y, const wrect_t *prcSubRect )
{
	DrawFrameOfSize( frame, x, y, GetWidth(), GetHeight(), prcSubRect);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : frame - 
//			x - 
//			y - 
//			*prcSubRect - 
//-----------------------------------------------------------------------------
void CEngineSprite::DrawFrameOfSize( int frame, int x, int y, int iWidth, int iHeight, const wrect_t *prcSubRect )
{
	float fLeft = 0;
	float fRight = 1;
	float fTop = 0;
	float fBottom = 1;

	if ( prcSubRect )
	{
		AdjustSubRect( this, frame, &fLeft, &fRight, &fTop, &fBottom, &iWidth, &iHeight, prcSubRect );
	}

	if ( giScissorTest && !Scissor( x, y, iWidth, iHeight, fLeft, fTop, fRight, fBottom ) )
		return;

	SetFrame( frame );

	IMesh* pMesh = materials->GetDynamicMesh( true, NULL, NULL, GetMaterial() );

	CMeshBuilder meshBuilder;
	meshBuilder.Begin( pMesh, MATERIAL_QUADS, 1 );

	float color[3];
	GetHUDSpriteColor( color );
	
	meshBuilder.Color3fv( color );
	meshBuilder.TexCoord2f( 0, fLeft, fTop );
	meshBuilder.Position3f( x, y, 0.0f );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3fv( color );
	meshBuilder.TexCoord2f( 0, fRight, fTop );
	meshBuilder.Position3f( x + iWidth, y, 0.0f );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3fv( color );
	meshBuilder.TexCoord2f( 0, fRight, fBottom );
	meshBuilder.Position3f( x + iWidth, y + iHeight, 0.0f );
	meshBuilder.AdvanceVertex();

	meshBuilder.Color3fv( color );
	meshBuilder.TexCoord2f( 0, fLeft, fBottom );
	meshBuilder.Position3f( x, y + iHeight, 0.0f );
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
}