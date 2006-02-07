//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#include "materialsystem/materialsystemutil.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/ITexture.h"
#include "materialsystem/IMaterialSystem.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


//-----------------------------------------------------------------------------
// Little utility class to deal with material references
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
CMaterialReference::CMaterialReference( char const* pMaterialName, const char *pTextureGroupName ) : m_pMaterial( 0 )
{
	if (pMaterialName)
	{
		Assert( pTextureGroupName );
		Init(pMaterialName, pTextureGroupName);
	}
}

CMaterialReference::~CMaterialReference()
{
	Shutdown();
}

//-----------------------------------------------------------------------------
// Attach to a material
//-----------------------------------------------------------------------------
void CMaterialReference::Init( char const* pMaterialName, const char *pTextureGroupName )
{
	m_pMaterial = materials->FindMaterial(pMaterialName, pTextureGroupName);
	Assert( m_pMaterial );
	if (m_pMaterial)
		m_pMaterial->IncrementReferenceCount();
}

void CMaterialReference::Init( IMaterial* pMaterial )
{
	m_pMaterial = pMaterial;
	if (m_pMaterial)
	{
		m_pMaterial->IncrementReferenceCount();
	}
}

void CMaterialReference::Init( CMaterialReference& ref )
{
	m_pMaterial = ref.m_pMaterial;
	if (m_pMaterial)
	{
		m_pMaterial->IncrementReferenceCount();
	}
}

//-----------------------------------------------------------------------------
// Detach from a material
//-----------------------------------------------------------------------------
void CMaterialReference::Shutdown( )
{
	if ( m_pMaterial )
	{
		m_pMaterial->DecrementReferenceCount();
		m_pMaterial = NULL;
	}
}


//-----------------------------------------------------------------------------
// Little utility class to deal with texture references
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// constructor, destructor
//-----------------------------------------------------------------------------
CTextureReference::CTextureReference( ) : m_pTexture(NULL)
{
}

CTextureReference::~CTextureReference( )
{
	Shutdown();
}

//-----------------------------------------------------------------------------
// Attach to a texture
//-----------------------------------------------------------------------------
void CTextureReference::Init( ITexture* pTexture )
{
	Shutdown();

	m_pTexture = pTexture;
	if (m_pTexture)
	{
		m_pTexture->IncrementReferenceCount();
	}
}

void CTextureReference::InitRenderTarget( int w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat fmt, MaterialRenderTargetDepth_t depth )
{
	Shutdown();

	// NOTE: Refcount returned by CreateRenderTargetTexture is 1
	m_pTexture = materials->CreateRenderTargetTexture( w, h, sizeMode, fmt, depth );
#ifdef _DEBUG
	ITexture *pSaveRenderTarget = materials->GetRenderTarget();
	int saveX, saveY, saveWidth, saveHeight;
	materials->GetViewport( saveX, saveY, saveWidth, saveHeight );
	materials->SetRenderTarget( m_pTexture );
	materials->Viewport( 0, 0, w, h );
	materials->ClearColor4ub( 0, 0, 0, 0 );
	materials->ClearBuffers( true, false );
	materials->SetRenderTarget( pSaveRenderTarget );
	materials->Viewport( saveX, saveY, saveWidth, saveHeight );
#endif
	Assert( m_pTexture );
}

//-----------------------------------------------------------------------------
// Detach from a texture
//-----------------------------------------------------------------------------
void CTextureReference::Shutdown()
{
	if ( m_pTexture )
	{
		m_pTexture->DecrementReferenceCount();
		m_pTexture = NULL;
	}
}

