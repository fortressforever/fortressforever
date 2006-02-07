//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_te_particlesystem.h"
#include "IEffects.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Sparks TE
//-----------------------------------------------------------------------------
class C_TESparks : public C_TEParticleSystem
{
public:
	DECLARE_CLASS( C_TESparks, C_TEParticleSystem );
	DECLARE_CLIENTCLASS();

					C_TESparks( void );
	virtual			~C_TESparks( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );
	virtual void	Precache( void );

	int m_nMagnitude;
	int m_nTrailLength;
	Vector m_vecDir;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TESparks::C_TESparks( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TESparks::~C_TESparks( void )
{
}

void C_TESparks::Precache( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TESparks::PostDataUpdate( DataUpdateType_t updateType )
{
	g_pEffects->Sparks( m_vecOrigin, m_nMagnitude, m_nTrailLength, &m_vecDir );
}

void TE_Sparks( IRecipientFilter& filter, float delay,
	const Vector* pos, int nMagnitude, int nTrailLength, const Vector *pDir )
{
	g_pEffects->Sparks( *pos, nMagnitude, nTrailLength, pDir );
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TESparks, DT_TESparks, CTESparks)
	RecvPropInt( RECVINFO( m_nMagnitude ) ),
	RecvPropInt( RECVINFO( m_nTrailLength ) ),
	RecvPropVector( RECVINFO( m_vecDir ) ),
END_RECV_TABLE()
