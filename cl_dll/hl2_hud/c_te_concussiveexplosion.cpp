//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_te_particlesystem.h"
#include "fx.h"
#include "RagdollExplosionEnumerator.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Concussive explosion entity
//-----------------------------------------------------------------------------
class C_TEConcussiveExplosion : public C_TEParticleSystem
{
public:
	DECLARE_CLASS( C_TEConcussiveExplosion, C_TEParticleSystem );
	DECLARE_CLIENTCLASS();

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

	void			AffectRagdolls( void );

	Vector	m_vecNormal;
	float	m_flScale;
	int		m_nRadius;
	int		m_nMagnitude;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEConcussiveExplosion::AffectRagdolls( void )
{
	if ( ( m_nRadius == 0 ) || ( m_nMagnitude == 0 ) )
		return;

	CRagdollExplosionEnumerator	ragdollEnum( m_vecOrigin, m_nRadius, m_nMagnitude );
	partition->EnumerateElementsInSphere( PARTITION_CLIENT_RESPONSIVE_EDICTS, m_vecOrigin, m_nRadius, false, &ragdollEnum );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bNewEntity - 
//-----------------------------------------------------------------------------
void C_TEConcussiveExplosion::PostDataUpdate( DataUpdateType_t updateType )
{
	AffectRagdolls();

	FX_ConcussiveExplosion( m_vecOrigin, m_vecNormal );
}

IMPLEMENT_CLIENTCLASS_EVENT_DT( C_TEConcussiveExplosion, DT_TEConcussiveExplosion, CTEConcussiveExplosion )
	RecvPropVector( RECVINFO(m_vecNormal)),
	RecvPropFloat( RECVINFO(m_flScale)),
	RecvPropInt( RECVINFO(m_nRadius)),	
	RecvPropInt( RECVINFO(m_nMagnitude)),
END_RECV_TABLE()