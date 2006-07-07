//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FF_SHAREDDEFS_H
#define FF_SHAREDDEFS_H
#ifdef _WIN32
#pragma once
#endif

#define FF_PLAYER_VIEW_OFFSET	Vector( 0, 0, 53.5 )

enum FFPlayerGrenadeState
{
    FF_GREN_NONE,
    FF_GREN_PRIMEONE,
    FF_GREN_PRIMETWO
};

enum FFStatusIconTypes
{
    FF_ICON_CONCUSSION,
    FF_ICON_ONFIRE,
    FF_ICON_TRANQ,
	FF_ICON_CALTROP,
	FF_ICON_LEGSHOT,
	FF_ICON_GAS,
	FF_ICON_INFECTED,
	FF_NUMICONS
};

struct SpyDisguiseWeapon
{
	char szWeaponModel[6][MAX_WEAPON_STRING];
	char szAnimExt[6][MAX_WEAPON_PREFIX];
};

#ifdef CLIENT_DLL
struct SpyInfo_s
{
	char	m_szName[ MAX_PLAYER_NAME_LENGTH ];	// Name we're using
	int		m_iTeam;	// Disguised team
	int		m_iClass;	// Disguised class

	void	Set( const char *pszName, int iTeam, int iClass )
	{
		Q_strcpy( m_szName, pszName );
		m_iTeam = iTeam;
		m_iClass = iClass;
	}

	void	SetTeam( int iTeam ) { m_iTeam = iTeam; }
	void	SetClass( int iClass ) { m_iClass = iClass; }
	void	SetName( const char *pszName ) { Q_strcpy( m_szName, pszName ); }

	bool	SameGuy( int iTeam, int iClass )
	{
		return( ( m_iTeam == iTeam ) && ( m_iClass == iClass ) );
	}

};

// Want to re-use this so renaming
// so the name isn't confusing
typedef SpyInfo_s CrosshairInfo_s;
#endif

#endif // FF_SHAREDDEFS_H
