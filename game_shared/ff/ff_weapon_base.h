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
#include "SoundEmitterSystem/isoundemittersystembase.h"

#ifdef CLIENT_DLL 
	#define CFFWeaponBase C_FFWeaponBase
#endif

class CFFPlayer;

#define MAX_DEPLOY_TIME 0.5f

// These are the names of the ammo types that the weapon script files reference.
// GRENADES and BULLETS have now been removed
#define AMMO_SHELLS				"AMMO_SHELLS"	// shotguns
#define AMMO_NAILS				"AMMO_NAILS"	// nailguns
#define AMMO_ROCKETS			"AMMO_ROCKETS"	// rpg
#define AMMO_CELLS				"AMMO_CELLS"	// for building dispenser, sentry gun
#define AMMO_DETPACK			"AMMO_DETPACK"	// for "building" detpacks
#define AMMO_MANCANNON			"AMMO_MANCANNON"	// for "building" man cannons
#define AMMO_GREN1				"AMMO_GREN1"	// gren1
#define AMMO_GREN2				"AMMO_GREN2"	// gren2

// Bleh I can't think of anything better. Problem is you
// can holster weapons but still fire them which is
// kind of good cause we want the fire to CANCEL the build
// but we don't want the weapon ACTUALLY firing. I'm
// going to put this into the primary attack function
// of any weapon that can be deployed for classes that
// can build.
#ifdef GAME_DLL
#define CANCEL_IF_BUILDING() \
{ \
	CFFPlayer *pFFPlayer = GetPlayerOwner(); \
	if( pFFPlayer && pFFPlayer->IsStaticBuilding() ) \
	{ \
		switch( pFFPlayer->GetCurrentBuild() ) \
		{ \
			case FF_BUILD_DISPENSER: pFFPlayer->Command_BuildDispenser(); break; \
			case FF_BUILD_SENTRYGUN: pFFPlayer->Command_BuildSentryGun(); break; \
			case FF_BUILD_DETPACK: engine->ClientCommand( pFFPlayer->edict(), "detpack 5" ); break; \
			case FF_BUILD_MANCANNON: pFFPlayer->Command_BuildManCannon(); break; \
		} \
		return; \
	} \
}
#endif

#ifdef CLIENT_DLL
#define CANCEL_IF_BUILDING() \
{ \
	CFFPlayer *pFFPlayer = GetPlayerOwner(); \
	if( pFFPlayer && pFFPlayer->IsStaticBuilding() ) \
		return; \
}
#endif

//This isnt used, as far as i can tell - shok
#define ABORT_FUNC_IF_BUILDING() \
{ \
	CFFPlayer *pFFPlayer = GetPlayerOwner(); \
	if( pFFPlayer && pFFPlayer->IsBuilding() ) \
		return; \
}

#ifdef GAME_DLL
#define CANCEL_IF_CLOAKED() \
{ \
	CFFPlayer *pFFPlayer = GetPlayerOwner(); \
	if( !pFFPlayer ) \
		return; \
	if( pFFPlayer->GetClassSlot() == CLASS_SPY ) \
	{ \
		if( pFFPlayer->IsCloaked() ) \
		{ \
			pFFPlayer->Uncloak( true ); \
		} \
	} \
}
#else
#define CANCEL_IF_CLOAKED() \
{ \
}
#endif

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
	FF_WEAPON_SHIELD,
	FF_WEAPON_DEPLOYSHIELD,

	// Sniper specific
	FF_WEAPON_AUTORIFLE, 
	FF_WEAPON_SNIPERRIFLE, 
	FF_WEAPON_LASERRIFLE,

	// Pyro specific
	FF_WEAPON_FLAMETHROWER, 
	FF_WEAPON_IC, 

	// Engineer specific
	FF_WEAPON_RAILGUN, 

	// Scout specific
	FF_WEAPON_JUMPDOWN,
	FF_WEAPON_JUMPUP,

	// Spy specific
	FF_WEAPON_TRANQUILISER, 

	// HWG Specific
	FF_WEAPON_ASSAULTCANNON, 

	// Soldier specific
	FF_WEAPON_RPG, 

	// Medic specific
	FF_WEAPON_GOOPGUN,
	
	// Spy grappling hook gun
	FF_WEAPON_HOOKGUN, 

	// End of normal TFC weapons
	// =========================

	// Civilian specific
	FF_WEAPON_TOMMYGUN,

	// End of physical weapons
	// =======================

	// Buildables
	FF_WEAPON_DEPLOYDISPENSER, 
	FF_WEAPON_DEPLOYSENTRYGUN, 
	FF_WEAPON_DEPLOYDETPACK,
	FF_WEAPON_DEPLOYMANCANNON,

	// Don't put any more weapons down here! 
	// Put the rest above the DEPLOY weapons please.
	// =============================================

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
	~CFFWeaponBase();

	#ifdef GAME_DLL
		DECLARE_DATADESC();
	#endif

	virtual void		Spawn( void );

	// All FF weapons are predicted
	virtual bool		IsPredicted() const { return true; }

	// All predicted weapons need to implement and return true
	virtual FFWeaponID	GetWeaponID() const { AssertMsg(0, "GetWeaponID() not defined for weapon"); return FF_WEAPON_NONE; }
	
	// Get FF weapon specific weapon data.
	CFFWeaponInfo const	&GetFFWpnData() const;

	// Get a pointer to the player that owns this weapon
	CFFPlayer			*GetPlayerOwner() const;

	// Play the correct sounds
	virtual	void		WeaponSound(WeaponSound_t sound_type, float soundtime = 0.0f);
	virtual	void		WeaponSoundLocal( WeaponSound_t sound_type, float soundtime = 0.0f );

	// Ensure that weapons cannot be selected while building
	virtual bool		CanBeSelected();
	
	// Plays recoil on both client & server
	virtual void		WeaponRecoil();

	// This is overloaded with the correct response
	virtual void		Fire() { AssertMsg(0, "Fire() not defined for weapon"); }

	// Death notice name
	virtual char		*GetDeathNoticeName();

	// Override the deploy time to a fixed time
	virtual bool		DefaultDeploy(char *szViewModel, char *szWeaponModel, int iActivity, char *szAnimExt);

	// Default primary attack for non-clip weapons
	virtual void		PrimaryAttack();

	// Modified to take into account different ammo amounts being fired
	virtual void		ItemPostFrame();

	// Overload drop function for our weapons. Makes it so they CANT be touched EVER
	virtual void		Drop( const Vector& vecVelocity );
			void		DropThink( void );
	
	// Should override the fov
	virtual float		GetFOV() { return -1; }

	const char			*GetWorldModel( void ) const;
	int					GetWorldModelIndex( void );

	// Let us multiply the recoil amount by something (like more recoil for a fully charged sniper rifle shot)
	virtual float		GetRecoilMultiplier( void ) { return 1.0f; }
	bool				m_bMuzzleFlash;

private:

	CFFWeaponBase(const CFFWeaponBase &);

#ifdef CLIENT_DLL
	// Some things from HL2MP
	virtual bool	ShouldPredict();
	virtual void	OnDataChanged(DataUpdateType_t type);
	virtual int		DrawModel( int flags );
	virtual RenderGroup_t GetRenderGroup( void );
	virtual ShadowType_t ShadowCastType( void );
#endif
};


#endif // FF_WEAPON_BASE_H
