//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "ProxyEntity.h"
#include "materialsystem/IMaterialVar.h"
#include "materialsystem/IMaterial.h"
#include "view.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Used for 'card' beams on lamps, this material fades the sprite out
// as the viewer nears, so that the viewer can't see that the effect is really
// a card.
//-----------------------------------------------------------------------------
class CLampBeamProxy : public CEntityMaterialProxy
{
public:
						CLampBeamProxy( void );
	virtual				~CLampBeamProxy( void );
	virtual bool		Init( IMaterial *pMaterial, KeyValues* pKeyValues );
	virtual void		OnBind( C_BaseEntity *pC_BaseEntity );

private:
	
	IMaterialVar		*m_pFadeValue;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CLampBeamProxy::CLampBeamProxy( void )
{
	m_pFadeValue = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CLampBeamProxy::~CLampBeamProxy( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Get pointer to the color value
// Input  : *pMaterial - 
//-----------------------------------------------------------------------------
bool CLampBeamProxy::Init( IMaterial *pMaterial, KeyValues* pKeyValues )
{
	assert( pMaterial );

	// Need to get the color variable.
	bool found;
	m_pFadeValue = pMaterial->FindVar( "$alpha", &found );
	return found;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pC_BaseEntity - 
//-----------------------------------------------------------------------------
#define FADE_DIST	150

void CLampBeamProxy::OnBind( C_BaseEntity *pEnt )
{
	if( !m_pFadeValue )
	{
		return;
	}
	
	Vector vecLocal;

	vecLocal = pEnt->GetAbsOrigin() - CurrentViewOrigin();
	VectorNormalize( vecLocal );

	float fade;
	fade = 1.0 - fabs( vecLocal.z );

	m_pFadeValue->SetFloatValue( fade );
}

EXPOSE_INTERFACE( CLampBeamProxy, IMaterialProxy, "lampbeam" IMATERIAL_PROXY_INTERFACE_VERSION );
