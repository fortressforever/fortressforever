//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FF_WEAPON_PARSE_H
#define FF_WEAPON_PARSE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_parse.h"
#include "networkvar.h"


//--------------------------------------------------------------------------------------------------------
class CFFWeaponInfo : public FileWeaponInfo_t
{
public:
	DECLARE_CLASS_GAMEROOT( CFFWeaponInfo, FileWeaponInfo_t );
	
	CFFWeaponInfo();
	
	virtual void Parse( ::KeyValues *pKeyValuesData, const char *szWeaponName );

	char m_szAnimExtension[16];		// string used to generate player animations with this weapon

	// Variables for weapons
	float	m_flCycleTime;
	float	m_iCycleDecrement;
	bool	m_bReloadClip;

	int		m_iDamage;
	int		m_iDamageRadius;

	int		m_iBullets;
	float	m_flBulletSpread;

	int		m_iSpeed;

	float	m_flPreReloadTime;
	float	m_flReloadTime;
	float	m_flPostReloadTime;

	float	m_flSpinTime;

	float	m_flRange;
	
	float	m_flRecoilAmount;
};


#endif // FF_WEAPON_PARSE_H
