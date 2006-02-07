//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "rotorwash.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// ==============================================
//  Rotorwash entity
// ==============================================

class CRotorWashEmitter : public CBaseEntity
{
public:
	DECLARE_CLASS( CRotorWashEmitter, CBaseEntity );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	void SetAltitude( float flAltitude ) { m_flAltitude = flAltitude; }
	void SetEmit( bool state ) { m_bEmit = state; }

protected:

	CNetworkVar( bool, m_bEmit );
	CNetworkVar( float, m_flAltitude );
};

IMPLEMENT_SERVERCLASS_ST( CRotorWashEmitter, DT_RotorWashEmitter )
	SendPropFloat(SENDINFO(m_flAltitude), -1, SPROP_NOSCALE ),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( env_rotorwash_emitter, CRotorWashEmitter );

BEGIN_DATADESC( CRotorWashEmitter )

	DEFINE_FIELD( 		m_bEmit, 		FIELD_BOOLEAN ),
	DEFINE_FIELD( 		m_flAltitude, 	FIELD_FLOAT ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &localOrigin - 
//			&localAngles - 
//			*pOwner - 
//			flAltitude - 
// Output : CBaseEntity
//-----------------------------------------------------------------------------
CBaseEntity *CreateRotorWashEmitter( const Vector &localOrigin, const QAngle &localAngles, CBaseEntity *pOwner, float flAltitude )
{
	CRotorWashEmitter *pEmitter = (CRotorWashEmitter *) CreateEntityByName( "env_rotorwash_emitter" );

	if ( pEmitter == NULL )
		return NULL;

	pEmitter->SetAbsOrigin( localOrigin );
	pEmitter->SetAbsAngles( localAngles );
	pEmitter->FollowEntity( pOwner );

	pEmitter->SetAltitude( flAltitude );
	pEmitter->SetEmit( false );

	return pEmitter;
}

