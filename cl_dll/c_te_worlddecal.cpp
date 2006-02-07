//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
//
//-----------------------------------------------------------------------------
// $Log: $
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basetempentity.h"
#include "iefx.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: World Decal TE
//-----------------------------------------------------------------------------
class C_TEWorldDecal : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEWorldDecal, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEWorldDecal( void );
	virtual			~C_TEWorldDecal( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

	virtual void	Precache( void );

public:
	Vector			m_vecOrigin;
	int				m_nIndex;

	const ConVar	*m_pDecals;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEWorldDecal::C_TEWorldDecal( void )
{
	m_vecOrigin.Init();
	m_nIndex = 0;

	m_pDecals = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEWorldDecal::~C_TEWorldDecal( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEWorldDecal::Precache( void )
{
	m_pDecals = cvar->FindVar( "r_decals" );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEWorldDecal::PostDataUpdate( DataUpdateType_t updateType )
{
	if ( m_pDecals && m_pDecals->GetInt() )
	{
		C_BaseEntity *ent = cl_entitylist->GetEnt( 0 );
		if ( ent )
		{
			effects->DecalShoot( m_nIndex, 0, ent->GetModel(), ent->GetAbsOrigin(), ent->GetAbsAngles(), m_vecOrigin, 0, 0 );
		}
	}
}

void TE_WorldDecal( IRecipientFilter& filter, float delay,
	const Vector* pos, int index )
{
	ConVar const *decals = cvar->FindVar( "r_decals" );

	if ( decals && decals->GetInt() )
	{
		C_BaseEntity *ent = cl_entitylist->GetEnt( 0 );
		if ( ent )
		{
			effects->DecalShoot( index, 0, ent->GetModel(), ent->GetAbsOrigin(), ent->GetAbsAngles(), *pos, 0, 0 );
		}
	}
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEWorldDecal, DT_TEWorldDecal, CTEWorldDecal)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropInt( RECVINFO(m_nIndex)),
END_RECV_TABLE()
