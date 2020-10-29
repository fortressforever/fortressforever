//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include <KeyValues.h>
#include "ff_weapon_parse.h"


FileWeaponInfo_t* CreateWeaponInfo()
{
	return new CFFWeaponInfo;
}


CFFWeaponInfo::CFFWeaponInfo()
{
}


void CFFWeaponInfo::Parse( KeyValues *pKeyValuesData, const char *szWeaponName )
{
	// we only want to validate scripts that have ff_ in front
	if (szWeaponName[0] && szWeaponName[0] == 'f' && szWeaponName[1] == 'f' && szWeaponName[2] == '_')
	{
		// this is a secret key value so that scripts without it will be rejected and not loaded
		if (pKeyValuesData->GetInt("ffencrypted", 0) != 1)
		{
			Warning("WeaponInfo: Invalid script format for %s\n", szWeaponName);
			return;
		}
	}

	BaseClass::Parse( pKeyValuesData, szWeaponName );

	m_flCycleTime		= pKeyValuesData->GetFloat( "CycleTime", 0.15f );
	m_iCycleDecrement	= pKeyValuesData->GetInt( "CycleDecrement", 0.15f );
	m_bReloadClip		= pKeyValuesData->GetInt( "ReloadClip", 0.0f );

	m_iDamage			= pKeyValuesData->GetInt( "Damage", 42 ); // Douglas Adams 1952 - 2001
	m_iDamageRadius		= pKeyValuesData->GetInt( "DamageRadius", m_iDamage );

	m_iSpeed			= pKeyValuesData->GetInt( "Speed", 800 );

	m_iBullets			= pKeyValuesData->GetInt( "Bullets", 1 );
	m_flBulletSpread	= pKeyValuesData->GetFloat( "BulletSpread", 0.5f );

	m_flPreReloadTime	= pKeyValuesData->GetFloat( "PreReloadTime", 0.15f );
	m_flReloadTime		= pKeyValuesData->GetFloat( "ReloadTime", 0.15f );
	m_flPostReloadTime	= pKeyValuesData->GetFloat( "PostReloadTime", 0.15f );

	m_flSpinTime		= pKeyValuesData->GetFloat( "SpinTime", 0.6 );

	m_flRange			= pKeyValuesData->GetFloat( "Range", 75.0 );

	m_flRecoilAmount	= pKeyValuesData->GetFloat( "RecoilAmount", 0 );
	m_flDeployDelay		= pKeyValuesData->GetFloat( "DeployDelay", 0.25f );

	const char *pAnimEx = pKeyValuesData->GetString( "PlayerAnimationExtension", "mp5" );
	Q_strncpy( m_szAnimExtension, pAnimEx, sizeof( m_szAnimExtension ) );
}


