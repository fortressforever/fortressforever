//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#ifndef MATERIALSYSTEMUTIL_H
#define MATERIALSYSTEMUTIL_H

#ifdef _WIN32
#pragma once
#endif

enum MaterialRenderTargetDepth_t;
enum RenderTargetSizeMode_t;
class IMaterial;
class ITexture;
enum ImageFormat;

//-----------------------------------------------------------------------------
// Little utility class to deal with material references
//-----------------------------------------------------------------------------
class CMaterialReference
{
public:
	// constructor, destructor
	CMaterialReference( char const* pMaterialName = 0, const char *pTextureGroupName = 0 );
	~CMaterialReference();

	// Attach to a material
	void Init( char const* pMaterialName, const char *pTextureGroupName );
	void Init( IMaterial* pMaterial );
	void Init( CMaterialReference& ref );

	// Detach from a material
	void Shutdown();

	// Automatic casts to IMaterial
	operator IMaterial*() { return m_pMaterial; }
	operator IMaterial const*() const { return m_pMaterial; }
	IMaterial* operator->() { return m_pMaterial; }
	
private:
	IMaterial* m_pMaterial;
};

//-----------------------------------------------------------------------------
// Little utility class to deal with texture references
//-----------------------------------------------------------------------------
class CTextureReference
{
public:
	// constructor, destructor
	CTextureReference( );
	~CTextureReference();

	// Attach to a texture
	void InitRenderTarget( int w, int h, RenderTargetSizeMode_t sizeMode, ImageFormat fmt, MaterialRenderTargetDepth_t depth );
	void Init( ITexture* pTexture );

	// Detach from a texture
	void Shutdown();

	// Automatic casts to ITexture
	operator ITexture*() { return m_pTexture; }
	operator ITexture const*() const { return m_pTexture; }
	ITexture* operator->() { return m_pTexture; }
	
private:
	ITexture* m_pTexture;
};


#endif // !MATERIALSYSTEMUTIL_H