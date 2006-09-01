//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for FF Game
//
// $NoKeywords: $
//=============================================================================//

#ifndef FF_PLAYER_H
#define FF_PLAYER_H
#pragma once

#include "player.h"
#include "server_class.h"

#undef MINMAX_H
#include "minmax.h"

#include "ff_playeranimstate.h"
#include "ff_shareddefs.h"
#include "ff_playerclass_parse.h"
#include "ff_esp_shared.h"
#include "utlvector.h"
#include "ff_weapon_base.h"
#include "ff_buildableobjects_shared.h"
#include "ff_modelglyph.h"
#include "in_buttons.h"

class CFFBuildableObject;
class CFFDetpack;
class CFFDispenser;
class CFFSentryGun;
class CFFSevTest;

#include "ff_mapguide.h"	// |-- Mirv: Map guides

#define FF_BUILD_DISP_STRING_LEN	256

// BEG: Added by Mulchman for team junk
#define FF_TEAM_UNASSIGNED	0
#define FF_TEAM_SPEC		1
#define FF_TEAM_BLUE		2
#define FF_TEAM_RED			3
#define FF_TEAM_YELLOW		4
#define FF_TEAM_GREEN		5
// END: Added by Mulchman for team junk

// Speed effect type
enum SpeedEffectType
{
	SE_SNIPERRIFLE = 0,
	SE_ASSAULTCANNON,
	SE_LEGSHOT,
	SE_TRANQ,
	SE_CALTROP,
	SE_LUA1,	// a speed effect that lua can set
	SE_LUA2,	// a speed effect that lua can set
	SE_LUA3,	// a speed effect that lua can set
	SE_LUA4,	// a speed effect that lua can set
	SE_LUA5,	// a speed effect that lua can set
	SE_LUA6,	// a speed effect that lua can set
	SE_LUA7,	// a speed effect that lua can set
	SE_LUA8,	// a speed effect that lua can set
	SE_LUA9,	// a speed effect that lua can set
	SE_LUA10,	// a speed effect that lua can set
};

// Scout radar struct
struct ScoutRadar_s
{
	int		m_iInfo;
	byte	m_bDucking;
	Vector	m_vecOrigin;

	ScoutRadar_s( int iInfo, byte bDucking, const Vector& vecOrigin )
	{
		m_iInfo = iInfo;
		m_bDucking = bDucking;
		m_vecOrigin = vecOrigin;
	}
};

// BEG: Speed Effect class for handling speed impairing effects (caltrop, legshot, etc)
#define NUM_SPEED_EFFECTS 48
struct SpeedEffect
{
	SpeedEffectType type;		// using a type now so we can search through them all
								// to check it doesnt already exist (for non-accumulative)
	float startTime;	// start time
	float endTime;		// end time
	float speed;			// speed to set the player to
	int modifiers;
	bool active;			// whether this speed effect is active or not
	float duration;
	bool bLuaEnforced;		// this is for speed effects set by lua that should not
							// be removed until lua says it can be removed (like when
							// you've got a speed effect on you and you get health
							// we don't want lua enforced effects to be removed then)
};

struct LocationInfo
{
	int entindex;
	char locationname[1024];
	int team;
};
// END: Speed Effect class

#define SEM_BOOLEAN			(1 << 0)
#define SEM_ACCUMULATIVE		(1 << 1)
#define SEM_HEALABLE			(1 << 2)

//=============================================================================
// >> FF Game player
//=============================================================================
class CFFPlayer : public CBasePlayer, public IFFPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( CFFPlayer, CBasePlayer );
	DECLARE_SERVERCLASS();
	DECLARE_PREDICTABLE();

	CFFPlayer();
	~CFFPlayer();

	static CFFPlayer *CreatePlayer( const char *className, edict_t *ed );
	static CFFPlayer* Instance( int iEnt );

	virtual int ObjectCaps( void );

	// This passes the event to the client's and server's CPlayerAnimState.
	void DoAnimationEvent( PlayerAnimEvent_t event );

	virtual void PreThink();
	virtual void PostThink();
	virtual CBaseEntity *EntSelectSpawnPoint();
	virtual void Spawn();
	virtual void InitialSpawn();
	virtual void Precache();
	
	virtual void Event_Killed(const CTakeDamageInfo &info);
	virtual bool Event_Gibbed(const CTakeDamageInfo &info);
	virtual bool BecomeRagdollOnClient(const Vector &force);

	virtual void LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles );
	
	// returns the name of the active weapons. returns null if there
	// there is no active weapon
	const char* GetActiveWeaponName() const;

	CFFWeaponBase* GetActiveFFWeapon() const;
	virtual void	CreateViewModel( int viewmodelindex = 0 );

	virtual void	CheatImpulseCommands( int iImpulse );

	CNetworkQAngle( m_angEyeAngles );	// Copied from EyeAngles() so we can send it to the client.
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 

// In shared code.
public:
	// IFFPlayerAnimState overrides.
	virtual CFFWeaponBase* FFAnim_GetActiveWeapon();
	virtual bool FFAnim_CanMove();
	

	void FireBullet( 
		Vector vecSrc, 
		const QAngle &shootAngles, 
		float vecSpread, 
		float flDamage,		// |-- Mirv: Float
		int iBulletType,
		CBaseEntity *pevAttacker,
		bool bDoEffects,
		float x,
		float y,
		float flSniperRifleCharge = 0.0 ); // added by Mulchman 9/20/2005
											// |-- Mirv: modified a bit
	// --> Mirv: Proper sound effects
	void PlayJumpSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol );
	void PlayFallSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol );
	void PlayStepSound( Vector &vecOrigin, surfacedata_t *psurface, float fvol, bool force );

	float	m_flJumpTime;
	float	m_flFallTime;
	// <-- Mirv: Proper sound effects

	// --> mulch
	int GetHealthPercentage( void );
	int GetArmorPercentage( void );
	// <--

private:

	void CreateRagdollEntity(const CTakeDamageInfo *info = NULL);

	IFFPlayerAnimState *m_PlayerAnimState;

	// ---> FF class stuff (billdoor)
	CNetworkVar(float, m_flArmorType);
	float m_flBaseArmorType;

	// BEG: Added by Mulchman for armor stuff
public:
	int		AddArmor( int iAmount );
	int		RemoveArmor( int iAmount );
	void	ReduceArmorClass();	// Bit of a one hit wonder, this

	int GetMaxShells( void ) const { return GetFFClassData().m_iMaxShells; }
	int GetMaxCells( void ) const { return GetFFClassData().m_iMaxCells; }
	int GetMaxNails( void ) const { return GetFFClassData().m_iMaxNails; }
	int GetMaxRockets( void ) const { return GetFFClassData().m_iMaxRockets; }
	int GetMaxDetpack( void ) const { return GetFFClassData().m_iMaxDetpack; }
	int GetMaxRadioTag( void ) const { return GetFFClassData().m_iMaxRadioTag; }

	// These "needs" functions will return however much the player needs
	// of the item to reach the max capacity. It'll return 0 if they don't
	// need anything.
	int NeedsArmor( void ) const { return GetMaxArmor() - GetArmor(); }
	int NeedsShells( void ) const { return GetMaxShells() - GetAmmoCount( AMMO_SHELLS ); }
	int NeedsCells( void ) const { return GetMaxCells() - GetAmmoCount( AMMO_CELLS ); }
	int NeedsNails( void ) const { return GetMaxNails() - GetAmmoCount( AMMO_NAILS ); }
	int NeedsRockets( void ) const { return GetMaxRockets() - GetAmmoCount( AMMO_ROCKETS ); }
	int NeedsDetpack( void ) const { return GetMaxDetpack() - GetAmmoCount( AMMO_DETPACK ); }
	int NeedsRadioTag( void ) const { return GetMaxRadioTag() - GetAmmoCount( AMMO_RADIOTAG ); }
	// END: Added by Mulchman for armor stuff

	void	SetLastSpawn( CBaseEntity *pEntity );

public:
	bool m_fRandomPC;
	int m_iNextClass;

	void ChangeTeam(int iTeamNum);
	void ChangeClass(const char *szNewClassName);
	int ActivateClass( void );
	int GetClassSlot();
	int GetNextClassSlot() { return m_iNextClass; }

	void KillPlayer( void );
	void RemoveItems( void );
	void RemoveBuildables( void );	// dispenser / sg / detpack
	void RemoveProjectiles( void );	// greandes & projectiles
	void RemoveBackpacks( void );	// Remove player thrown backpacks
	void KillAndRemoveItems( void );
	
	bool PlayerHasSkillCommand(const char *szCommand);
	virtual int OnTakeDamage(const CTakeDamageInfo &inputInfo);

	// ---> end

	// --> Mirv: Damage & force stuff
	virtual int OnTakeDamage_Alive( const CTakeDamageInfo &info );
	virtual void OnDamagedByExplosion( const CTakeDamageInfo &info );

	virtual bool ShouldGib( const CTakeDamageInfo &info );
	virtual bool HasHumanGibs( void ) { return true; }

	void LimbDecapitation( const CTakeDamageInfo &info );
	void CreateLimbs(int iLimbs);

	int m_fBodygroupState;
	// <-- Mirv: Damage & force stuff

	bool HasItem(const char* szItemName) const;
	bool IsInNoBuild() const;
	bool IsUnderWater() const { return (GetWaterLevel() == WL_Eyes); }
	bool IsWaistDeepInWater() const { return (GetWaterLevel() == WL_Waist); }
	bool IsFeetDeepInWater() const { return (GetWaterLevel() == WL_Feet); }

private:
	// ---> FF movecode stuff (billdoor)
	friend class CFFGameMovement;	// |-- Mirv: a class key must be used when declaring a friend!
	void StartSkiing(void) { if(m_iSkiState == 0) m_iSkiState = 1; m_iLocalSkiState = 1; };
	void StopSkiing(void) { if(m_iSkiState == 1) m_iSkiState = 0; m_iLocalSkiState = 0; };
	int GetSkiState(void) { return m_iSkiState.Get(); };
	CNetworkVar(int, m_iSkiState);
	// this version of the ski state is not sent over the network, but is altered only by the movecode for the local player
	int m_iLocalSkiState;
	// ---> end

public:
	// ---> FF server-side player command handlers
	void Command_TestCommand(void);
	void Command_Class(void);
	void Command_Team( void );
	void Command_WhatTeam( void ); // for testing purposes
	void Command_BuildDispenser( void );
	void Command_BuildSentryGun( void );
	void Command_BuildDetpack( void );
	void Command_Radar( void );	
	void Command_HintTest( void );
	void Command_DispenserText( void );	// to set custom dispenser text messages on the server
	void Command_PrimeOne(void); // prime primary grenade
	void Command_PrimeTwo(void); // prime secondary grenade
	void Command_ThrowGren(void); // throw currently primed grenade
	void Command_ToggleOne( void );
	void Command_ToggleTwo( void );	
	void Command_SevTest( void ); // Sev's test animation thing
	void Command_FlagInfo( void ); // flaginfo
	void Command_DropItems( void );
	void Command_Discard( void );	
	void Command_SaveMe( void );
	void Command_EngyMe( void );
	void Command_Disguise();

	void Command_SabotageSentry();
	void Command_SabotageDispenser();
	// ---> end of FF server-side player command handlers

    // Beg: Added by Mulchman for building objects and such
	CNetworkHandle( CAI_BaseNPC, m_hDispenser );
	CNetworkHandle( CAI_BaseNPC, m_hSentryGun );
	CNetworkHandle( CAI_BaseNPC, m_hDetpack );

	bool IsBuilding( void ) { return m_bBuilding; }
	// Used for seeing if a player is currently
	// trying to build a detpack, dispenser, or sentry gun
	CNetworkVar( bool, m_bBuilding );
	// Tells us what we are currently building
	CNetworkVar( int, m_iCurBuild );
	// Tells us what we want to build
	int m_iWantBuild;
	// Tells us if the player cancelled building
	//CNetworkVar( bool, m_bCancelledBuild );
	// Tells us when we can call the postbuildgenericthink
    float m_flBuildTime;

	// Origin of where we started to build at
	Vector m_vecBuildOrigin;

	CFFWeaponBase *m_pBuildLastWeapon;

	void PreBuildGenericThink( void );	// *** NOT AN ACTUAL THINK FUNCTION ***
	void PostBuildGenericThink( void );	// *** NOT AN ACTUAL THINK FUNCTION ***
	// End: Added by Mulchman for building objects and such

public:
	// Can we update our location yet?
	void SetLocation(int entindex, const char *szNewLocation, int iNewLocationTeam);
	void RemoveLocation( int entindex );
	const char *GetLocation( void ) 
	{ 
		if(m_Locations.Count() > 0)
			return const_cast< char * >( m_Locations[0].locationname );
		else
			return m_szLastLocation;
	}
	int GetLocationTeam( void ) 
	{ 
		if(m_Locations.Count() > 0)
			return m_Locations[0].team; 
		else
			return m_iLastLocationTeam;
	}
private:
	char m_szLastLocation[1024];
	int	m_iLastLocationTeam;
	int	m_iClientLocation;
	CUtlVector<LocationInfo> m_Locations;

public:
	// Set the spawn delay for a player. If the current delay
	// is longer than flDelay then flDelay is ignored and
	// the longer delay is used. It also checks the entity
	// system & mp_spawndelay for any global delays.
    void SetRespawnDelay( float flDelay = 0.0f );

	// Only for LUA to use to set player specific spawn delays
	void LUA_SetPlayerRespawnDelay( float flDelay ) { m_fl_LuaSet_PlayerRespawnDelay = flDelay; SetRespawnDelay(); }
private:
	float m_fl_LuaSet_PlayerRespawnDelay;

	//-- Added by L0ki -------------------------------------------------------
	// Grenade related
public:
	int GetPrimaryGrenades( void );
	int GetSecondaryGrenades( void );
	void SetPrimaryGrenades( int iNewCount );
	void SetSecondaryGrenades( int iNewCount );
	int AddPrimaryGrenades( int iNewCount );
	int AddSecondaryGrenades( int iNewCount );
private:
	bool IsGrenadePrimed(void);
	void GrenadeThink(void);
	void ThrowGrenade(float fTimer, float speed = 630.0f);		// |-- Mirv: So we can drop grens
public:
	void RemovePrimedGrenades( void );
private:
	CNetworkVar(FFPlayerGrenadeState, m_iGrenadeState);
	CNetworkVar(float, m_flServerPrimeTime);
	CNetworkVar(int, m_iPrimary);
	CNetworkVar(int, m_iSecondary);
	bool m_bWantToThrowGrenade;			// does the client want to throw this grenade as soon as possible?
	bool m_bEngyGrenWarned;
	// Backpacks
public:
	void PackDeadPlayerItems( void );
	//------------------------------------------------------------------------


public:
	// These 3 handle lua settings speed effects and other effects
	void LuaAddEffect( int iEffect, float flEffectDuration = 0.0f, float flIconDuration = 0.0f, float flSpeed = 0.0f );
	bool LuaIsEffectActive( int iEffect );
	void LuaRemoveEffect( int iEffect );
 
	void AddSpeedEffect(SpeedEffectType type, float duration, float speed, int mod = 0, int iIcon = -1, float flIconDuration = -1, bool bLuaAdded = false);
	bool IsSpeedEffectSet( SpeedEffectType type );
	void RemoveSpeedEffect(SpeedEffectType type, bool bLuaAdded = false);
	int	ClearSpeedEffects(int mod = 0);
protected:
	void RemoveSpeedEffectByIndex( int iSpeedEffectIndex );

public:
	void Infect( CFFPlayer *pPlayer );
	void Cure( CFFPlayer *pPlayer );
	void ApplyBurning( CFFPlayer *hIgniter, float scale = 1.0f, float flIconDuration = 10.0f );
	bool IsBurning( void ) const;

	void Gas( float flDuration, float flIconDuration );
	bool IsGassed( void ) const { return m_bGassed; }
	float m_flLastGassed;	// Last time we took gas damage, so that gas grens won't be cumulative
protected:
	void UnGas( void );
	bool m_bGassed;

public:	
	bool IsInfected( void ) const		{ return m_bInfected; }
	CBaseEntity *GetInfector( void )	{ return ( m_hInfector == NULL ) ? NULL : ( CBaseEntity * )m_hInfector; }
	int GetInfectorTeam( void ) const	{ return IsInfected() ? m_iInfectedTeam : TEAM_UNASSIGNED; }
	
	bool GetSpecialInfectedDeath( void ) const { return m_bSpecialInfectedDeath; }
	void SetSpecialInfectedDeath( void ) { m_bSpecialInfectedDeath = true; }
private:
	SpeedEffect m_vSpeedEffects[NUM_SPEED_EFFECTS];				// All speed effects impairing the player
	float m_fLastHealTick;							// When the last time the medic was healed
	float m_fLastInfectedTick;						// When the last health tick for infections was at
	// Mulch: wrapping in EHANDLE
	EHANDLE m_hInfector;							// Who infected this player
	bool m_bInfected;								// if this player is infected
	bool m_bImmune;	// Mulch: immunity
	float m_flImmuneTime; // Mulch: immunity: time in the future of when the immunity ends
	int m_iInfectedTeam;	// Mulch: team the medic who infected us was on
    float m_flLastOverHealthTick; // Mulch: last time we took health cause health > maxhealth
	
	// A hack flag to put on a player who was infected and tried to
	// evade dying by infection by changing teams or typing kill. We
	// note this so we can acredit the guy who infected him when we
	// get to gamerules. If we didn't have to note this properly in
	// a hud death msg (and logs) it'd be a lot easier.
	bool m_bSpecialInfectedDeath; 
public:
	bool IsImmune( void ) const { return m_bImmune; }

private:

	CFFPlayer *m_hIgniter;
	 float m_flNextBurnTick;   // when the next burn tick should fire
	int m_iBurnTicks;         // how many more ticks are left to fire
	float m_flBurningDamage;  // how much total damage is left to take

	void StatusEffectsThink( void );
	void RecalculateSpeed( );

private:
	// --> Mirv: Player class script files
	virtual const unsigned char *GetEncryptionKey();
	PLAYERCLASS_FILE_INFO_HANDLE	m_hPlayerClassFileInfo;

public:
	CFFPlayerClassInfo const &GetFFClassData() const;
	// <-- Mirv: Player class script files

public:
	// Added by Mulchman - two overrides
	virtual void LockPlayerInPlace( void );
	virtual void UnlockPlayer( void );

public:
	// Beg: Added by Mulchman for scout radar
	float m_flLastScoutRadarUpdate;
	// End: Added by Mulchman for scout radar

	// BEG: Added by Mulchman for radar tagging
	bool IsRadioTagged( void ) const { return m_bRadioTagged; }
	void SetRadioTagged( CFFPlayer *pWhoTaggedMe, float flStartTime, float flDuration );
	void SetUnRadioTagged( void );
	int GetTeamNumOfWhoTaggedMe( void ) const;
	CFFPlayer *GetPlayerWhoTaggedMe( void );
protected:
	bool m_bRadioTagged;
	float m_flRadioTaggedStartTime;
	float m_flRadioTaggedDuration;
	int m_iRadioTaggedAmmoIndex;

	// This is here so that when someone tags us w/ a radiotag and
	// we die while tagged by that person, we can award that player 
	// their extra point
	EHANDLE	m_pWhoTaggedMe;

	CUtlVector< ESP_Shared_s > m_hRadioTaggedList;
	float m_flLastRadioTagUpdate;

protected:
	void FindRadioTaggedPlayers( void );
	// END: Added by Mulchman for radar tagging

	// TODO: REMOVE ME REMOVE ME
	// this is here so i can easily make the bot
	// set his dispenser text to something so
	// i can test!
public:
	void Bot_SetDispenserText( const char *pszString ) { Q_strcpy( m_szCustomDispenserText, pszString ); }
	void Bot_SetDetpackTimer( int iTime ) { m_iDetpackTime = iTime; }
	void Bot_Disguise( int iTeam, int iClass )
	{
		SetDisguise(iTeam, iClass);
	}
	
protected:
	// Added by Mulchman - here temporarily I guess
	// client has to send the text to the server
	// and storing it in the player is an easy way
	// for the client to send it "whenever"
	char m_szCustomDispenserText[ FF_BUILD_DISP_STRING_LEN ];

	// Added by Mulchman - the detpack fuse time
	int m_iDetpackTime;

public:
	// --> Mirv: Various things
	void Command_SetChannel( void );
	int m_iChannel;
    
	bool m_fFeigned;
	void SpyFeign( void );
	void SpySilentFeign( void );
	bool IsFeigned( void ) const { return m_fFeigned; }

	int Heal(CFFPlayer *, float);

	// Keeping these seperate for now
	CNetworkHandle( CFFMapGuide, m_hNextMapGuide );
	CNetworkHandle( CFFMapGuide, m_hLastMapGuide );
	CNetworkVar( float, m_flNextMapGuideTime );

	CFFMapGuide *FindMapGuide(string_t targetname);
	void MoveTowardsMapGuide();

	void Command_MapGuide();

	void ClassSpecificSkill();
	void ClassSpecificSkill_Post();

	float m_flNextClassSpecificSkill;

	CNetworkVar( float, m_flConcTime );
	void UnConcuss( void );
	void Concuss(float flDuration, float flIconDuration, const QAngle *viewjerk = NULL, float flDistance = 0.0f);

	CNetworkVar( int, m_iClassStatus );
	int GetClassForClient() { return (0x0000000F & m_iClassStatus); }

	// Use this to directly set a player class.
	// 0 is unassigned. Yar.
	void SetClassForClient( int classnum );

	virtual void Extinguish();

	int AddHealth(unsigned int amount);
	virtual int				TakeHealth( float flHealth, int bitsDamageType );

	// Moving to CBasePlayer for use with "kill" command
	// and also force spawning after joining a map for the first time
	//float m_flNextSpawnDelay;

	virtual int TakeEmp();
	virtual void Ignite( float flFlameLifetime, bool bNPCOnly, float flSize, bool bCalledByLevelDesigner );

	virtual bool TakeNamedItem(const char* szName);	

public:
	int LuaAddAmmo( int iAmmoType, int iAmount );
	void LuaRemoveAmmo( int iAmmoType, int iAmount );

	virtual int GiveAmmo(int iCount, int iAmmoIndex, bool bSuppressSound = false);
	int	GiveAmmo(int iCount, const char *szName, bool bSuppressSound = false);

	int m_iMaxAmmo[MAX_AMMO_TYPES];

	// For pipebomb delay
	void SetPipebombShotTime( float flShotTime ) { m_flPipebombShotTime = flShotTime; }
	float GetPipebombShotTime( void ) const { return m_flPipebombShotTime; }
protected:
	float m_flPipebombShotTime;

public:
	void SetDisguisable(bool in) 
	{
		m_bDisguisable = in;
		if (!in) 
			ResetDisguise();
	}
	bool GetDisguisable() const { return m_bDisguisable; }
private:
	bool m_bDisguisable;

public:	
	int GetDisguisedClass( void );
	int GetDisguisedTeam( void );
	bool IsDisguised( void );
	void SetDisguise(int iTeam, int iClass, bool bInstant = false);

	CNetworkVar( int, m_iSpyDisguise );	// Mulch: Want to tell the client our current disguise
	CNetworkVar(int, m_iSpawnInterpCounter);

	// **********************************
	// SaveMe stuffs
public:
	bool IsInSaveMe( void ) const { return m_iSaveMe != 0; }
protected:
	//CHandle< CFFSaveMe >	m_hSaveMe;
	CNetworkVar( unsigned int, m_iSaveMe );
	float m_flSaveMeTime;
	// **********************************

	// **********************************
	// EngyMe stuffs
public:
	bool IsInEngyMe( void ) const { return m_iEngyMe != 0; }
protected:
	CNetworkVar( unsigned int, m_iEngyMe );
	float m_flEngyMeTime;
	// **********************************

public:
	// Some luabind functions
	bool IsInAttack1( void ) const	{ return ( m_nButtons & IN_ATTACK ) ? true : false; }
	bool IsInAttack2( void ) const	{ return ( m_nButtons & IN_ATTACK2 ) ? true : false; }
	bool IsInUse( void ) const		{ return ( m_nButtons & IN_USE ) ? true : false; }
	bool IsInJump( void ) const		{ return ( m_nButtons & IN_JUMP ) ? true : false; }
	bool IsInForward( void ) const	{ return ( m_nButtons & IN_FORWARD ) ? true : false; }
	bool IsInBack( void ) const		{ return ( m_nButtons & IN_BACK ) ? true : false; }
	bool IsInMoveLeft( void ) const	{ return ( m_nButtons & IN_MOVELEFT ) ? true : false; }
	bool IsInMoveRight( void ) const	{ return ( m_nButtons & IN_MOVERIGHT ) ? true : false; }
	bool IsInLeft( void ) const		{ return ( m_nButtons & IN_LEFT ) ? true : false; }
	bool IsInRight( void ) const	{ return ( m_nButtons & IN_RIGHT ) ? true : false; }
	bool IsInRun( void ) const		{ return ( m_nButtons & IN_RUN ) ? true : false; }
	bool IsInReload( void ) const	{ return ( m_nButtons & IN_RELOAD ) ? true : false; }
	bool IsInSpeed( void ) const	{ return ( m_nButtons & IN_SPEED ) ? true : false; }
	bool IsInWalk( void ) const		{ return ( m_nButtons & IN_WALK ) ? true : false; }
	bool IsInZoom( void ) const		{ return ( m_nButtons & IN_ZOOM ) ? true : false; }
	bool IsDucking( void ) const	{ return ( GetFlags() & FL_DUCKING ) ? true : false; }
	bool IsOnGround( void ) const	{ return ( GetFlags() & FL_ONGROUND ) ? true : false; }
	bool IsInAir( void ) const		{ return !IsOnGround(); }

	const char *GetSteamID( void ) const;
	int GetPing( void ) const;
	int GetPacketloss( void ) const;

private:
	int GetNewDisguisedClass( void );
	int GetNewDisguisedTeam( void );
	int m_iNewSpyDisguise;
	float m_flFinishDisguise;

public:
	void FinishDisguise();
	void ResetDisguise();

	int		FlashlightIsOn( void );
	void	FlashlightTurnOn( void );
	void	FlashlightTurnOff( void );

	// <-- Mirv: Various things

	// Mirv: In TFC the AbsOrigin is midway up the model. We need to take this into
	// account for various things. 
	Vector GetLegacyAbsOrigin();
	Vector GetWaistOrigin( void );
	Vector GetFeetOrigin( void );

	virtual void Touch(CBaseEntity *pOther);
	void	InstaSwitch(int iClassNum);
	
	void	SetupClassVariables();

	virtual void FireBullets(const FireBulletsInfo_t &info);
	virtual bool HandleShotImpactingWater(const FireBulletsInfo_t &info, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest);

	void		SpySabotageThink();
	void		SpySabotageRelease();


	float		m_flNextSpySabotageThink;
	float		m_flSpySabotageFinish;
	CHandle<CFFBuildableObject>	m_hSabotaging;

	CBaseCombatWeapon *GetWeaponForSlot(int iSlot);

	float		m_flIdleTime;

	Vector		BodyTarget(const Vector &posSrc, bool bNoisy);

	// TODO: Possibly network this?
	float		m_flNextJumpTimeForDouble;
	bool		m_bCanDoubleJump;

	CNetworkVar(float, m_flSpeedModifier);
	float		m_flSpeedModifierOld;
	float		m_flSpeedModifierChangeTime;

	// stats stuff
	int			m_iStatsID;
	int			m_iStatDeath;
	int			m_iStatTeamKill;
	int			m_iStatKill;
	int			m_iStatInfections;
	int			m_iStatCures;
	int			m_iStatHeals;
	int			m_iStatHealHP;
	int			m_iStatCritHeals;
	int			m_iStatInfectCures;

public:
	// Run "effects" and "speed effects" through here first
	// before giving the player the actual effect.
	bool LuaRunEffect( int iEffect, CBaseEntity *pEffector = NULL, float *pflDuration = NULL, float *pflIconDuration = NULL, float *pflSpeed = NULL );

	void DamageEffect(float flDamage, int fDamageType);
};


inline CFFPlayer *ToFFPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast<CFFPlayer*>( pEntity ) != 0 );
#endif
	return static_cast< CFFPlayer* >( pEntity );
}

#endif	// FF_PLAYER_H
