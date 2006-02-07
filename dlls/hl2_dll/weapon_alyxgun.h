//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_ALYXGUN_H
#define WEAPON_ALYXGUN_H

#include "basehlcombatweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

class CWeaponAlyxGun : public CHLSelectFireMachineGun
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( CWeaponAlyxGun, CHLSelectFireMachineGun );

	CWeaponAlyxGun();
	~CWeaponAlyxGun();

	DECLARE_SERVERCLASS();
	
	void	Precache( void );

	int		GetMinBurst() { return 4; }
	int		GetMaxBurst() { return 7; }

	virtual void Equip( CBaseCombatCharacter *pOwner );

	float	GetFireRate( void ) { return 0.1f; }
	int		CapabilitiesGet( void ) { return bits_CAP_WEAPON_RANGE_ATTACK1; }
	int		WeaponRangeAttack2Condition( float flDot, float flDist );

	virtual const Vector& GetBulletSpread( void );
	void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

	DECLARE_ACTTABLE();

};

#endif // WEAPON_ALYXGUN_H
