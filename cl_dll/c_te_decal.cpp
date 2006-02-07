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

//-----------------------------------------------------------------------------
// Purpose: Decal TE
//-----------------------------------------------------------------------------
class C_TEDecal : public C_BaseTempEntity
{
public:
	DECLARE_CLASS( C_TEDecal, C_BaseTempEntity );
	DECLARE_CLIENTCLASS();

					C_TEDecal( void );
	virtual			~C_TEDecal( void );

	virtual void	PostDataUpdate( DataUpdateType_t updateType );

	virtual void	Precache( void );

public:
	Vector			m_vecOrigin;
	Vector			m_vecStart;
	int				m_nEntity;
	int				m_nHitbox;
	int				m_nIndex;

	const ConVar	*m_pDecals;
};

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEDecal::C_TEDecal( void )
{
	m_vecOrigin.Init();
	m_vecStart.Init();
	m_nEntity = 0;
	m_nIndex = 0;
	m_nHitbox = 0;

	m_pDecals = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_TEDecal::~C_TEDecal( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_TEDecal::Precache( void )
{											 
	m_pDecals = cvar->FindVar( "r_decals" );
}

void TE_Decal( IRecipientFilter& filter, float delay,
	const Vector* pos, const Vector* start, int entity, int hitbox, int index )
{
	trace_t tr;

	// Special case for world entity with hitbox:
	if ( (entity == 0) && (hitbox != 0) )
	{
		Ray_t ray;
		ray.Init( *start, *pos );
		staticpropmgr->AddDecalToStaticProp( *start, *pos, hitbox - 1, index, false, tr );
	}
	else
	{
		// Only decal the world + brush models
		// Here we deal with decals on entities.
		C_BaseEntity* ent;
		if ( ( ent = cl_entitylist->GetEnt( entity ) ) == false )
			return;

		ent->AddDecal( *start, *pos, *pos, hitbox, 
			index, false, tr );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : bool - 
//-----------------------------------------------------------------------------
void C_TEDecal::PostDataUpdate( DataUpdateType_t updateType )
{
	CBroadcastRecipientFilter filter;
	TE_Decal( filter, 0.0f, &m_vecOrigin, &m_vecStart, m_nEntity, m_nHitbox, m_nIndex );
}

IMPLEMENT_CLIENTCLASS_EVENT_DT(C_TEDecal, DT_TEDecal, CTEDecal)
	RecvPropVector( RECVINFO(m_vecOrigin)),
	RecvPropVector( RECVINFO(m_vecStart)),
	RecvPropInt( RECVINFO(m_nEntity)),
	RecvPropInt( RECVINFO(m_nHitbox)),
	RecvPropInt( RECVINFO(m_nIndex)),
END_RECV_TABLE()
