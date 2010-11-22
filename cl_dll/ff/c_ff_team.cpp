//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side C_FFTeam class
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "engine/IEngineSound.h"
#include "hud.h"
#include "recvproxy.h"
#include "c_ff_team.h"

#include <vgui/vgui.h>
#include "vgui/ILocalize.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


IMPLEMENT_CLIENTCLASS_DT(C_FFTeam, DT_FFTeam, CFFTeam)
	// --> Mirv: Some limits that the client needs to know about for the menu
	RecvPropInt( RECVINFO( m_iAllies ) ),
	RecvPropInt( RECVINFO( m_iMaxPlayers ) ),
	RecvPropArray3( RECVINFO_ARRAY(m_iClasses), RecvPropInt( RECVINFO(m_iClasses[0]))),
	// <-- Mirv: Some limits that the client needs to know about for the menu
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_FFTeam::C_FFTeam() : C_Team()
{
	memset( &m_iClasses, 0, sizeof(m_iClasses) );	// |-- Mirv: Classes
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_FFTeam::~C_FFTeam()
{
}

// --> Mirv: Menues need to know limits
int C_FFTeam::Get_Classes( int classnum )
{
	return m_iClasses[classnum];
}

int C_FFTeam::Get_Teams( void )
{
	return m_iMaxPlayers;
}

int C_FFTeam::GetAllies( void )
{
	return m_iAllies;
}
// <-- Mirv: Menues need to know limits

// --> hlstriker: Sets array of allies by ref, and returns the number of allies
int C_FFTeam::GetAlliedTeams( int (&iAlliedTeams)[TEAM_COUNT] )
{
	int iCount = 0;
	for (int i = TEAM_BLUE; i < TEAM_COUNT; i++ )
	{
		if ( m_iTeamNum == i )
			continue;

		if ( GetAllies() & ( 1 << i ) )
		{
			iAlliedTeams[iCount++] = i;
			if ( iCount >= TEAM_COUNT )
				break;
		}
	}
	return iCount;
}
// <--

//-----------------------------------------------------------------------------
// Purpose: Get a pointer to the specified TF team manager
//-----------------------------------------------------------------------------
C_FFTeam *GetGlobalFFTeam( int iTeamNumber )
{
	return (C_FFTeam *)GetGlobalTeam( iTeamNumber );
}
