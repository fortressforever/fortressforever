//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_FFBASE_H
#define WEAPON_FFBASE_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_playeranimstate.h"
#include "ff_weapon_parse.h"

#if defined( CLIENT_DLL )
	#define CWeaponFFBase C_WeaponFFBase
#endif

class CFFPlayer;

// These are the names of the ammo types that the weapon script files reference.
#define AMMO_BULLETS			"AMMO_BULLETS"
#define AMMO_ROCKETS			"AMMO_ROCKETS"
#define AMMO_GRENADE			"AMMO_GRENADE"

//--------------------------------------------------------------------------------------------------------
//
// Weapon IDs for all FF Game weapons
//
typedef enum
{
	WEAPON_NONE = 0,

	WEAPON_MP5,
	WEAPON_SHOTGUN,
	WEAPON_GRENADE,
	
	WEAPON_MAX,		// number of weapons weapon index
} FFWeaponID;

typedef enum
{
	Primary_Mode = 0,
	Secondary_Mode,
} FFWeaponMode;

const char *WeaponIDToAlias( int id );

class CWeaponFFBase : public CBaseCombatWeapon
{
public:
	DECLARE_CLASS( CWeaponFFBase, CBaseCombatWeapon );
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CWeaponFFBase();

	#ifdef GAME_DLL
		DECLARE_DATADESC();
	#endif

	// All predicted weapons need to implement and return true
	virtual bool	IsPredicted() const { return true; }
	virtual FFWeaponID GetWeaponID( void ) const { return WEAPON_NONE; }
	
	// Get FF weapon specific weapon data.
	CFFWeaponInfo const	&GetFFWpnData() const;

	// Get a pointer to the player that owns this weapon
	CFFPlayer* GetPlayerOwner() const;

	// override to play custom empty sounds
	virtual bool PlayEmptySound();

#ifdef GAME_DLL
	virtual void SendReloadEvents();
#endif

private:
	CWeaponFFBase( const CWeaponFFBase & );
};


#endif // WEAPON_FFBASE_H
