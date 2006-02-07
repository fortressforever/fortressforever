//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "dlight.h"
#include "iefx.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Dynamic Light
//-----------------------------------------------------------------------------
class C_TEDynamicLight : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEDynamicLight, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEDynamicLight( void );
	virtual			~C_TEDynamicLight( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

public:
	Vector			m_vecOrigin;
	float			m_fRadius;
	int				r;
	int				g;
	int				b;
	int				exponent;
	float			m_fTime;
	float			m_fDecay;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEDynamicLight::C_TEDynamicLight( void )
{
	m_vecOrigin.Init();
	r = 0;
	g = 0;
	b = 0;
	exponent = 0;
	m_fRadius = 0.0;
	m_fTime = 0.0;
	m_fDecay = 0.0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEDynamicLight::~C_TEDynamicLight( void )
{
}

void TE_DynamicLight( IRecipientFilter& filter, float delay,
	const Vector* org, int r, int g, int b, int exponent, float radius, float time, float decay )
{
	dlight_t	*dl = effects->CL_AllocDlight( LIGHT_INDEX_TE_DYNAMIC );
	if ( !dl )
	{
		return;
	}

	dl->origin	= *org;
	dl->radius	= radius;
	dl->color.r	= r;
	dl->color.g	= g;
	dl->color.b	= b;
	dl->color.exponent	= exponent;
	dl->die		= gpGlobals->curtime + time;
	dl->decay	= decay;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEDynamicLight::PostDataUpdate( DataUpdateType_t updateType )
{
	CBroadcastRecipientFilter filter;
	TE_DynamicLight( filter, 0.0f, &m_vecOrigin, r, g, b, exponent, m_fRadius, m_fTime, m_fDecay );
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEDynamicLight, DT_TEDynamicLight, CTEDynamicLight)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropInt( RECVINFO(r)),
	RecvPropInt( RECVINFO(g)),
	RecvPropInt( RECVINFO(b)),
	RecvPropInt( RECVINFO(exponent)),
	RecvPropFloat( RECVINFO(m_fRadius)),
	RecvPropFloat( RECVINFO(m_fTime)),
	RecvPropFloat( RECVINFO(m_fDecay)),
END_RECV_TABLE()
