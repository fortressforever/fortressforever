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
#include "iefx.h"
#include "engine/IStaticPropMgr.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// UNDONE:  Get rid of this?
#define FDECAL_PERMANENT			0x01

//-----------------------------------------------------------------------------
// Purpose: Projected Decal TE
//-----------------------------------------------------------------------------
class C_TEProjectedDecal : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEProjectedDecal, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEProjectedDecal( void );
	virtual			~C_TEProjectedDecal( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

	virtual void	Precache( void );

public:
	Vector			m_vecOrigin;
	QAngle			m_angRotation;
	float			m_flDistance;
	int				m_nIndex;

	const ConVar	*m_pDecals;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEProjectedDecal::C_TEProjectedDecal( void )
{
	m_vecOrigin.Init();
	m_angRotation.Init();
	m_flDistance = 0.0f;
	m_nIndex = 0;

	m_pDecals = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEProjectedDecal::~C_TEProjectedDecal( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEProjectedDecal::Precache( void )
{
	m_pDecals = cvar->FindVar( "r_decals" );
}

void TE_ProjectDecal( IRecipientFilter& filter, float delay,
	const Vector* pos, const QAngle *angles, float distance, int index )
{
	trace_t	tr;

	Vector fwd;
	AngleVectors( *angles, &fwd );

	Vector endpos;
	VectorMA( *pos, distance, fwd, endpos );

	CTraceFilterHitAll traceFilter;
	UTIL_TraceLine( *pos, endpos, MASK_ALL, &traceFilter, &tr );

	if ( tr.fraction == 1.0f )
	{
		return;
	}

	C_BaseEntity* ent = tr.m_pEnt;
	Assert( ent );

	int hitbox = tr.hitbox;

	if ( tr.hitbox != 0 )
	{
		staticpropmgr->AddDecalToStaticProp( *pos, endpos, hitbox - 1, index, false, tr );
	}
	else
	{
		// Only decal the world + brush models
		ent->AddDecal( *pos, endpos, endpos, hitbox, 
			index, false, tr );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEProjectedDecal::PostDataUpdate( DataUpdateType_t updateType )
{
	CBroadcastRecipientFilter filter;
	TE_ProjectDecal( filter, 0.0f, &m_vecOrigin, &m_angRotation, m_flDistance, m_nIndex );
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEProjectedDecal, DT_TEProjectedDecal, CTEProjectedDecal)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropQAngles( RECVINFO( m_angRotation )),
	RecvPropFloat( RECVINFO(m_flDistance)),
	RecvPropInt( RECVINFO(m_nIndex)),
END_RECV_TABLE()

