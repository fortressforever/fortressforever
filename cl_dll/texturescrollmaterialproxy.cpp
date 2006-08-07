//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "materialsystem/IMaterialProxy.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"
#include <KeyValues.h>
#include "VMatrix.h"
#include "functionproxy.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// $textureScrollVar
// $textureScrollRate
// $textureScrollAngle
class CTextureScrollMaterialProxy : public IMaterialProxy
{
public:
	CTextureScrollMaterialProxy();
	virtual ~CTextureScrollMaterialProxy();
	virtual bool Init( IMaterial *pMaterial, KeyValues *pKeyValues );
	virtual void OnBind( void *pC_BaseEntity );
	virtual void Release( void ) { delete this; }
private:
	IMaterialVar *m_pTextureScrollVar;
	CFloatInput m_TextureScrollRate;
	CFloatInput m_TextureScrollAngle;
	CFloatInput m_TextureScale;

#ifdef _XBOX
	bool m_bWaterShader;
#endif
};

CTextureScrollMaterialProxy::CTextureScrollMaterialProxy()
{
	m_pTextureScrollVar = NULL;

#ifdef _XBOX
	m_bWaterShader = false;
#endif
}

CTextureScrollMaterialProxy::~CTextureScrollMaterialProxy()
{
}


bool CTextureScrollMaterialProxy::Init( IMaterial *pMaterial, KeyValues *pKeyValues )
{
	char const* pScrollVarName = pKeyValues->GetString( "textureScrollVar" );
	if( !pScrollVarName )
		return false;

	bool foundVar;
	m_pTextureScrollVar = pMaterial->FindVar( pScrollVarName, &foundVar, false );
	if( !foundVar )
		return false;

	m_TextureScrollRate.Init( pMaterial, pKeyValues, "textureScrollRate", 1.0f );
	m_TextureScrollAngle.Init( pMaterial, pKeyValues, "textureScrollAngle", 0.0f );
	m_TextureScale.Init( pMaterial, pKeyValues, "textureScale", 1.0f );

#ifdef _XBOX
	const char *pShaderName = pMaterial->GetShaderName();
	m_bWaterShader = !Q_stricmp( pShaderName, "water" ) ||
		!Q_stricmp( pShaderName, "water_dx80" ) ||	!Q_stricmp( pShaderName, "water_dudv" ) ||
		!Q_stricmp( pShaderName, "water_firstpass" ) ||	!Q_stricmp( pShaderName, "water_secondpass" );
#endif

	return true;
}

void CTextureScrollMaterialProxy::OnBind( void *pC_BaseEntity )
{
	if( !m_pTextureScrollVar )
	{
		return;
	}

	float rate, angle, scale;

	// set default values if these variables don't exist.
	rate		= m_TextureScrollRate.GetFloat();
	angle		= m_TextureScrollAngle.GetFloat();
	scale		= m_TextureScale.GetFloat();

#ifdef _XBOX
	// Hack for water
	if ( m_bWaterShader )
	{
		scale = 0.5f;
	}
#endif

	float sOffset, tOffset;
	
	sOffset = gpGlobals->curtime * cos( angle * ( M_PI / 180.0f ) ) * rate;
	tOffset = gpGlobals->curtime * sin( angle * ( M_PI / 180.0f ) ) * rate;

	// make sure that we are positive
	if( sOffset < 0.0f )
	{
		sOffset += 1.0f + -( int )sOffset;
	}
	if( tOffset < 0.0f )
	{
		tOffset += 1.0f + -( int )tOffset;
	}
			    
	// make sure that we are in a [0,1] range
	sOffset = sOffset - ( int )sOffset;
	tOffset = tOffset - ( int )tOffset;
	
	if (m_pTextureScrollVar->GetType() == MATERIAL_VAR_TYPE_MATRIX)
	{
		VMatrix mat( scale, 0.0f, 0.0f, sOffset,
			0.0f, scale, 0.0f, tOffset,
			0.0f, 0.0f, 1.0f, 0.0f,
			0.0f, 0.0f, 0.0f, 1.0f );
		m_pTextureScrollVar->SetMatrixValue( mat );
	}
	else
	{
		m_pTextureScrollVar->SetVecValue( sOffset, tOffset, 0.0f );
	}
}

EXPOSE_INTERFACE( CTextureScrollMaterialProxy, IMaterialProxy, "TextureScroll" IMATERIAL_PROXY_INTERFACE_VERSION );
