//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "ff_team.h"
#include "entitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


// Datatable
IMPLEMENT_SERVERCLASS_ST(CFFTeam, DT_FFTeam)
	// --> Mirv: Some limits that the client needs to know about for the menu
	SendPropInt( SENDINFO( m_iAllies ) ), //AfterShock: this has a flag for each team
	SendPropInt( SENDINFO( m_iMaxPlayers ) ),
	SendPropArray3( SENDINFO_ARRAY3(m_iClasses), SendPropInt( SENDINFO_ARRAY(m_iClasses), 4 ) ),
	
	// <-- Mirv: Some limits that the client needs to know about for the menu
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( ff_team_manager, CFFTeam );

// --> Mirv: Class restrictions
extern ConVar cr_scout, cr_sniper, cr_soldier, cr_demoman, cr_medic, cr_hwguy, cr_pyro, cr_spy, cr_engineer;
// <-- Mirv: Class restrictions

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to the specified TF team manager
//-----------------------------------------------------------------------------
CFFTeam *GetGlobalFFTeam( int iIndex )
{
	return (CFFTeam*)GetGlobalTeam( iIndex );
}


//-----------------------------------------------------------------------------
// Purpose: Needed because this is an entity, but should never be used
//-----------------------------------------------------------------------------
void CFFTeam::Init( const char *pName, int iNumber )
{
	BaseClass::Init( pName, iNumber );

	// --> Mirv: Some default settings
	memset( &m_iClasses, -1, sizeof( m_iClasses ) );	// Jiggles: All classes start as "disallowed" so players can't pick
														//			a disallowed class in the time it takes to update the client menu
	m_iAllies = 0;										// no allies
	// <-- Mirv: Some default settings

	// Only detect changes every half-second.
	NetworkProp()->SetUpdateInterval( 0.75f );
}

// --> Mirv: Some allies and avail classes functions
void CFFTeam::SetAllies( int allies )
{
	m_iAllies = allies;
}

void CFFTeam::SetEasyAllies( int iTeam )
{
	m_iAllies |= (1<<iTeam);
}

void CFFTeam::ClearAllies()
{
	m_iAllies = 0;
}

int CFFTeam::GetAllies( void )
{
	return m_iAllies;
}

void CFFTeam::SetTeamLimits( int val )
{
	m_iMaxPlayers = val;
	UpdateLimits();
}
int CFFTeam::GetTeamLimits( void )
{
	return m_iMaxPlayers;
}

// This is the class limit  for the MAP
void CFFTeam::SetClassLimit( int classnum, int val )
{
	m_iClassesMap[classnum] = val;
	UpdateLimits();
}

// This is the class limit taking into account cr_ too
int CFFTeam::GetClassLimit( int classnum )
{
	return m_iClasses[classnum];
}

// Bah, just made this to save time
inline int minifnotzero( int a, int b )
{
	if( a == 0 )
		return b;
	else if( b == 0 )
		return a;
	else
		return min( a, b );
}

void CFFTeam::UpdateLimits( void )
{
	// This is a really messy way of doing it
	m_iClasses.Set( 1, minifnotzero( cr_scout.GetInt(), m_iClassesMap[1] ) );
	m_iClasses.Set( 2, minifnotzero( cr_sniper.GetInt(), m_iClassesMap[2] ) );
	m_iClasses.Set( 3, minifnotzero( cr_soldier.GetInt(), m_iClassesMap[3] ) );
	m_iClasses.Set( 4, minifnotzero( cr_demoman.GetInt(), m_iClassesMap[4] ) );
	m_iClasses.Set( 5, minifnotzero( cr_medic.GetInt(), m_iClassesMap[5] ) );
	m_iClasses.Set( 6, minifnotzero( cr_hwguy.GetInt(), m_iClassesMap[6] ) );
	m_iClasses.Set( 7, minifnotzero( cr_pyro.GetInt(), m_iClassesMap[7] ) );
	m_iClasses.Set( 8, minifnotzero( cr_spy.GetInt(), m_iClassesMap[8] ) );
	m_iClasses.Set( 9, minifnotzero( cr_engineer.GetInt(), m_iClassesMap[9] ) );

	m_iClasses.Set( 10, m_iClassesMap[10] );
}
// <-- Mirv: Some allies and avail classes functions