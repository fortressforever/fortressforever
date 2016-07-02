//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Client side CTFTeam class
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_FF_TEAM_H
#define C_FF_TEAM_H
#ifdef _WIN32
#pragma once
#endif

#include "c_team.h"
#include "shareddefs.h"

class C_BaseEntity;
class C_BaseObject;
class CBaseTechnology;

//-----------------------------------------------------------------------------
// Purpose: TF's Team manager
//-----------------------------------------------------------------------------
class C_FFTeam : public C_Team
{
	DECLARE_CLASS( C_FFTeam, C_Team );
	DECLARE_CLIENTCLASS();

public:

					C_FFTeam();
	virtual			~C_FFTeam();

	int GetAlliedTeams( int (&iAlliedTeams)[TEAM_COUNT] );

	// --> Mirv: Menus need to know limits
	virtual int		Get_Classes( int );
	virtual int		Get_Teams( void );
	virtual int		GetAllies( void );

	bool IsFFA() { return m_bFFA; };
	void SetFFA( bool bFFA ) { m_bFFA = bFFA; };

private:

	int		m_iClasses[12];
	int		m_iMaxPlayers;
	int		m_iAllies;
	bool	m_bFFA;
	// <-- Mirv: Menus need to know limits

};

// Global team handling functions
C_FFTeam *GetGlobalFFTeam( int iTeamNumber );


#endif // C_FF_TEAM_H
