//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Entity that propagates general data needed by clients for every player.
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "player.h"
#include "player_resource.h"
#include <coordsize.h>

#include "ff_player.h"		// |-- Mirv: Needed for channels

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Datatable
IMPLEMENT_SERVERCLASS_ST_NOBASE(CPlayerResource, DT_PlayerResource)
//	SendPropArray( SendPropString( SENDINFO(m_szName[0]) ), SENDARRAYINFO(m_szName) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iPing), SendPropInt( SENDINFO_ARRAY(m_iPing), 10, SPROP_UNSIGNED ) ),
//	SendPropArray( SendPropInt( SENDINFO_ARRAY(m_iPacketloss), 7, SPROP_UNSIGNED ), m_iPacketloss ),
	SendPropArray3( SENDINFO_ARRAY3(m_iScore), SendPropInt( SENDINFO_ARRAY(m_iScore), 15 ) ),	// |- Mirv: Upped transmission bits from 12->15
	SendPropArray3( SENDINFO_ARRAY3(m_iFortPoints), SendPropInt( SENDINFO_ARRAY(m_iFortPoints), 20 ) ),	// |- Shock: Upped transmission bits from 15->20 (needs to be big!)
	SendPropArray3( SENDINFO_ARRAY3(m_iDeaths), SendPropInt( SENDINFO_ARRAY(m_iDeaths), 12 ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_bConnected), SendPropInt( SENDINFO_ARRAY(m_bConnected), 1, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iTeam), SendPropInt( SENDINFO_ARRAY(m_iTeam), 4 ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_bAlive), SendPropInt( SENDINFO_ARRAY(m_bAlive), 1, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iHealth), SendPropInt( SENDINFO_ARRAY(m_iHealth), 8, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iArmor), SendPropInt( SENDINFO_ARRAY(m_iArmor), 9, SPROP_UNSIGNED ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iClass), SendPropInt( SENDINFO_ARRAY(m_iClass), 5 ) ),	// |-- Mirv: Current class
	
	SendPropArray3( SENDINFO_ARRAY3(m_iChannel), SendPropInt( SENDINFO_ARRAY(m_iChannel), 4 ) ), // |-- Mirv: Channel info

	SendPropBool(SENDINFO(m_bIsIntermission)),

END_SEND_TABLE()

BEGIN_DATADESC( CPlayerResource )

	// DEFINE_ARRAY( m_iPing, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_iPacketloss, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_iScore, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_iDeaths, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_bConnected, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_FIELD( m_flNextPingUpdate, FIELD_FLOAT ),
	// DEFINE_ARRAY( m_iTeam, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_bAlive, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_ARRAY( m_iHealth, FIELD_INTEGER, MAX_PLAYERS+1 ),
	// DEFINE_FIELD( m_nUpdateCounter, FIELD_INTEGER ),

	// Function Pointers
	DEFINE_FUNCTION( ResourceThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( player_manager, CPlayerResource );

CPlayerResource *g_pPlayerResource;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerResource::Spawn( void )
{
	for ( int i=0; i < MAX_PLAYERS+1; i++ )
	{
		m_iPing.Set( i, 0 );
		m_iScore.Set( i, 0 );
		m_iFortPoints.Set( i, 0 );
		m_iDeaths.Set( i, 0 );
		m_bConnected.Set( i, 0 );
		m_iTeam.Set( i, 0 );
		m_bAlive.Set( i, 0 );
		m_iClass.Set( i, 0 );	// |-- Mirv: Current class
		
		m_iChannel.Set( i, 0 );	// |-- Mirv: Channel info
	}

	m_bIsIntermission = false;

	SetThink( &CPlayerResource::ResourceThink );
	SetNextThink( gpGlobals->curtime );
	m_nUpdateCounter = 0;
}

//-----------------------------------------------------------------------------
// Purpose: The Player resource is always transmitted to clients
//-----------------------------------------------------------------------------
int CPlayerResource::UpdateTransmitState()
{
	// ALWAYS transmit to all clients.
	return SetTransmitState( FL_EDICT_ALWAYS );
}

extern bool Server_IsIntermission();

//-----------------------------------------------------------------------------
// Purpose: Wrapper for the virtual GrabPlayerData Think function
//-----------------------------------------------------------------------------
void CPlayerResource::ResourceThink( void )
{
	m_nUpdateCounter++;

	UpdatePlayerData();

	m_bIsIntermission = Server_IsIntermission();

	SetNextThink( gpGlobals->curtime + 0.1f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerResource::UpdatePlayerData( void )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CFFPlayer *pPlayer = (CFFPlayer* )UTIL_PlayerByIndex( i );	// |-- Mirv: Use our class instead
		
		if ( pPlayer && pPlayer->IsConnected() )
		{
			m_iScore.Set( i, pPlayer->FragCount() );
			m_iFortPoints.Set( i, pPlayer->FortPointsCount() );
			m_iDeaths.Set( i, pPlayer->DeathCount() );
			m_bConnected.Set( i, 1 );
			m_iTeam.Set( i, pPlayer->GetTeamNumber() );
			m_bAlive.Set( i, pPlayer->IsAlive()?1:0 );
			m_iHealth.Set(i, max( 0, pPlayer->GetHealth() ) );
			m_iArmor.Set(i, max( 0, pPlayer->GetArmor() ) );
			m_iClass.Set(i, pPlayer->GetClassSlot() );	// |-- Mirv: Update our class

			// Don't update ping / packetloss everytime

			if ( !(m_nUpdateCounter%20) )
			{
				// update ping all 20 think ticks = (20*0.1=2seconds)
				int ping, packetloss;
				UTIL_GetPlayerConnectionInfo( i, ping, packetloss );
				
				// calc avg for scoreboard so it's not so jittery
				ping = 0.8f * m_iPing.Get(i) + 0.2f * ping;

				
				m_iPing.Set( i, ping );
				// m_iPacketloss.Set( i, packetloss );

				// --> Mirv: Update the player's channel
				CFFPlayer *plyr = (CFFPlayer *)pPlayer;
				m_iChannel.Set( i, plyr->m_iChannel );
				// <-- Mirv: Update the player's channel
			}
		}
		else
		{
			m_bConnected.Set( i, 0 );
		}
	}
}
