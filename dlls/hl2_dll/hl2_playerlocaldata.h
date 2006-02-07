//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef HL2_PLAYERLOCALDATA_H
#define HL2_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "networkvar.h"

#include "hl_movedata.h"

//-----------------------------------------------------------------------------
// Purpose: Player specific data for HL2 ( sent only to local player, too )
//-----------------------------------------------------------------------------
class CHL2PlayerLocalData
{
public:
	// Save/restore
	DECLARE_SIMPLE_DATADESC();
	DECLARE_CLASS_NOBASE( CHL2PlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	CHL2PlayerLocalData();

	CNetworkVar( float, m_flSuitPower );
	CNetworkVar( bool,	m_bZooming );
	CNetworkVar( int,	m_bitsActiveDevices );
	CNetworkVar( int,	m_iSquadMemberCount );
	CNetworkVar( int,	m_iSquadMedicCount );
	CNetworkVar( bool,	m_fSquadInFollowMode );
	CNetworkVar( bool,	m_bWeaponLowered );

	// Ladder related data
	CNetworkVar( EHANDLE, m_hLadder );
	LadderMove_t			m_LadderMove;
};

EXTERN_SEND_TABLE(DT_HL2Local);

// Uses same send proxy as normal local data tables
void* SendProxy_SendLocalDataTable( const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID );

#endif // HL2_PLAYERLOCALDATA_H
