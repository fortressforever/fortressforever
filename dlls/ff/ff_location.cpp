/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file ff_location.cpp
/// @author Patrick O'Leary [Mulchman]
/// @date 02/20/2006
/// @brief Location entity
/// 
/// Revisions
/// ---------
/// 02/20/2006	Initial version

#include "cbase.h"
#include "ff_location.h"
#include "ff_player.h"
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CFFLocation )
	DEFINE_KEYFIELD( m_iTeam, FIELD_INTEGER, "team" ),
	DEFINE_KEYFIELD( m_szAreaName, FIELD_STRING, "areaname" ),

	// Function Pointers
	DEFINE_FUNCTION(MultiTouch),
END_DATADESC();

LINK_ENTITY_TO_CLASS( trigger_ff_location, CFFLocation );
PRECACHE_REGISTER( trigger_ff_location );

CFFLocation::CFFLocation( void ) 
{
	m_szAreaName = NULL_STRING;
	m_iTeam = 1;
}

void CFFLocation::Spawn( void ) 
{
	BaseClass::Spawn();

	InitTrigger();

	m_flWait = 0.4;

	SetTouch( &CFFLocation::MultiTouch );
}

//-----------------------------------------------------------------------------
// Purpose: Touch function. Activates the trigger.
// Input  : pOther - The thing that touched us.
//-----------------------------------------------------------------------------
void CFFLocation::MultiTouch( CBaseEntity *pOther )
{
	if( pOther && pOther->IsPlayer() && pOther->IsAlive() )
	{
		CFFPlayer *pPlayer = ToFFPlayer( pOther );
		if( !pPlayer )
		{
			return;
		}

		CSingleUserRecipientFilter filter( pPlayer );
		filter.MakeReliable();
		UserMessageBegin( filter, "SetPlayerLocation" );
			WRITE_STRING( STRING( m_szAreaName ) );
			WRITE_SHORT( m_iTeam ); // changed
		MessageEnd();
	}
}
