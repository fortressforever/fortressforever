//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Team management class. Contains all the details for a specific team
//
// $NoKeywords: $
//=============================================================================//

#ifndef FF_TEAM_H
#define FF_TEAM_H

#ifdef _WIN32
#pragma once
#endif


#include "utlvector.h"
#include "team.h"


//-----------------------------------------------------------------------------
// Purpose: Team Manager
//-----------------------------------------------------------------------------
class CFFTeam : public CTeam
{
	DECLARE_CLASS( CFFTeam, CTeam );
	DECLARE_SERVERCLASS();

public:

	// Initialization
	virtual void Init( const char *pName, int iNumber );

	// --> Mirv: Team classes available and allies
	//int m_iAllies;
	CNetworkVar( int, m_iAllies ); // |-- Mulch: as per mirv 02/03/06

	CNetworkArray( int, m_iClasses, 12 );	// this is the actual limit, needed by the client
	int m_iClassesMap[12];					// this is just the map limits

	CNetworkVar( int, m_iMaxPlayers );
	

public:
	void SetClassLimit( int, int );	// Set the map's class limit
	int GetClassLimit( int );			// Get the class limit (inc. cr_)

	void SetTeamLimits( int );
	int GetTeamLimits( void );

	void SetAllies( int );
	void SetEasyAllies( int );
	int GetAllies( void );

	void UpdateLimits( void );
	// <-- Mirv: Team classes available and allies
};


extern CFFTeam *GetGlobalFFTeam( int iIndex );


#endif // TF_TEAM_H
