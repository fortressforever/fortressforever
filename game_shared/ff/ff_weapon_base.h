/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_weapon_base.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December ##, 2004
/// @brief All weapons derived from here
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation


#ifndef FF_WEAPON_BASE_H
#define FF_WEAPON_BASE_H
#ifdef _WIN32
#pragma once
#endif

#include "ff_playeranimstate.h"
#include "ff_weapon_parse.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponBase C_FFWeaponBase
#endif

class CFFPlayer;

#define MAX_DEPLOY_TIME 0.75f

// These are the names of the ammo types that the weapon script files reference.
// GRENADES and BULLETS have now been removed
#define AMMO_SHELLS				"AMMO_SHELLS"	// shotguns
#define AMMO_NAILS				"AMMO_NAILS"	// nailguns
#define AMMO_ROCKETS			"AMMO_ROCKETS"	// rpg
#define AMMO_CELLS				"AMMO_CELLS"	// for building dispenser, sentry gun
#define AMMO_DETPACK			"AMMO_DETPACK"	// for "building" detpacks
#define AMMO_RADIOTAG			"AMMO_RADIOTAG"	// for radio tagging

// Weapon IDs for all FF Game weapons
typedef enum
{
	FF_WEAPON_NONE = 0, 

	// Melee
	FF_WEAPON_CROWBAR, 
	FF_WEAPON_KNIFE, 
	FF_WEAPON_MEDKIT, 
	FF_WEAPON_SPANNER, 
	FF_WEAPON_UMBRELLA, 
	FF_WEAPON_FLAG, 

	// Shutguns
	FF_WEAPON_SHOTGUN, 
	FF_WEAPON_SUPERSHOTGUN, 

	// Nailguns
	FF_WEAPON_NAILGUN, 
	FF_WEAPON_SUPERNAILGUN, 

	// Demomen Specific
	FF_WEAPON_GRENADELAUNCHER, 
	FF_WEAPON_PIPELAUNCHER, 

	// Sniper specific
	FF_WEAPON_AUTORIFLE, 
	FF_WEAPON_SNIPERRIFLE, 
	FF_WEAPON_RADIOTAGRIFLE, 

	// Pyro specific
	FF_WEAPON_FLAMETHROWER, 
	FF_WEAPON_IC, 

	// Engineer specific
	FF_WEAPON_RAILGUN, 

	// Spy specific
	FF_WEAPON_TRANQUILISER, 

	// HWG Specific
	FF_WEAPON_ASSAULTCANNON, 

	// Soldier specific
	FF_WEAPON_RPG, 

	// Buildables
	FF_WEAPON_DEPLOYDISPENSER, 
	FF_WEAPON_DEPLOYSENTRYGUN, 
	FF_WEAPON_DEPLOYDETPACK, 
	
	FF_WEAPON_MAX, 		// number of weapons weapon index

} FFWeaponID;

typedef enum
{
	Primary_Mode = 0, 
	Secondary_Mode, 
} FFWeaponMode;

int AliasToWeaponID(const char *alias);
const char *WeaponIDToAlias(int id);

//=============================================================================
// CFFWeaponBase
//=============================================================================

class CFFWeaponBase : public CBaseCombatWeapon
{
public:
	DECLARE_CLASS(CFFWeaponBase, CBaseCombatWeapon);
	DECLARE_NETWORKCLASS(); 
	DECLARE_PREDICTABLE();

	CFFWeaponBase();

	#ifdef GAME_DLL
		DECLARE_DATADESC();
	#endif

	// All predicted weapons need to implement and return true
	virtual bool	IsPredicted() const { return true; }
	virtual FFWeaponID GetWeaponID() const { return FF_WEAPON_NONE; }
	
	// Get FF weapon specific weapon data.
	CFFWeaponInfo const	&GetFFWpnData() const;

	// Get a pointer to the player that owns this weapon
	CFFPlayer * GetPlayerOwner() const;

	// override to play custom empty sounds
	virtual bool PlayEmptySound();

	// override these with default firing code to save a lot of replication
	virtual void PrimaryAttack();
	virtual bool Deploy();
	virtual bool Reload();
	virtual void WeaponIdle();
	virtual bool CanBeSelected( void );
	
	// this is where our actual projectile spawning code goes for primary attack
	virtual void Fire();

	// this just loads the recoil
	virtual void WeaponRecoil();

	virtual char *GetDeathNoticeName();

#ifdef GAME_DLL
	virtual void SendReloadEvents();
#endif

	virtual bool DefaultDeploy(char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt);

private:
	CFFWeaponBase(const CFFWeaponBase &);

#ifdef CLIENT_DLL
	float m_flNextReloadAttempt;
#endif

	CNetworkVar(int, m_fInSpecialReload);

	// So we don't spam when trying to kill
	// buildables (by spam I mean stopping it
	// then starting it right away then stopping
	// it right away etc. because of holding
	// down the mouse button. We want one
	// click of mouse to kill the build.)
	float m_flNextBuildKill;

	// We need to cock some guns
	bool m_bNeedsCock;

	// Jerky anim fix
#ifdef CLIENT_DLL
	virtual bool	ShouldPredict();
	virtual void	OnDataChanged( DataUpdateType_t type );
#endif
};


#endif // FF_WEAPON_BASE_H
