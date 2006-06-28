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
#include "ff_playeranimstate.h"
#include "ff_shareddefs.h"
#include "ff_playerclass_parse.h"
#include "ff_esp_shared.h"
#include "utlvector.h"
#include "ff_weapon_base.h"
#include "ff_buildableobjects_shared.h"

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
	SE_SNIPERRIFLE,
	SE_ASSAULTCANNON,
	SE_LEGSHOT,
	SE_TRANQ,
	SE_CALTROP,
};

// BEG: Speed Effect class for handling speed impairing effects (caltrop, legshot, etc)
#define NUM_SPEED_EFFECTS 10
struct SpeedEffect
{
	SpeedEffectType type;		// using a type now so we can search through them all
								// to check it doesnt already exist (for non-accumulative)
	float startTime;	// start time
	float endTime;		// end time
	float speed;			// speed to set the player to
	int modifiers;
	bool active;			// whether this speed effect is active or not
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

	// This passes the event to the client's and server's CPlayerAnimState.
	void DoAnimationEvent( PlayerAnimEvent_t event );

	virtual void PreThink();
	virtual void PostThink();
	virtual CBaseEntity *EntSelectSpawnPoint();
	virtual void Spawn();
	virtual void InitialSpawn();
	virtual void Precache();
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles );
	
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

	void CreateRagdollEntity();

	IFFPlayerAnimState *m_PlayerAnimState;

	// ---> FF class stuff (billdoor)
	CNetworkVar(float, m_fArmorType);

	// BEG: Added by Mulchman for armor stuff
public:
	int AddArmor( int iAmount )
	{
		iAmount = min( iAmount, m_iMaxArmor - m_iArmor );
		if (iAmount < 1)
			return 0;

		//m_iArmor.GetForModify() += iAmount;
		m_iArmor += iAmount;

		return iAmount;
	}
	int RemoveArmor( int iAmount )
	{
		int iRemovedAmt = min( iAmount, m_iArmor );

		m_iArmor = clamp( m_iArmor - iAmount, 0, m_iArmor );

		return iRemovedAmt;
	}

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

public:
	bool m_fRandomPC;
	int m_iNextClass;

	void ChangeClass(const char *szNewClassName);
	int ActivateClass( void );
	int GetClassSlot();
	int GetNextClassSlot() { return m_iNextClass; }

	void KillPlayer( void );
	void RemoveItems( void );
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
	void Command_Disguise();
	// ---> end of FF server-side player command handlers

	

    // Beg: Added by Mulchman for building objects and such
	CNetworkHandle( CAI_BaseNPC, m_hDispenser );
	CNetworkHandle( CAI_BaseNPC, m_hSentryGun );
	CNetworkHandle( CAI_BaseNPC, m_hDetpack );

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
	CNetworkVar(FFPlayerGrenadeState, m_iGrenadeState);
	CNetworkVar(float, m_flServerPrimeTime);
	CNetworkVar(int, m_iPrimary);
	CNetworkVar(int, m_iSecondary);
	bool m_bWantToThrowGrenade;			// does the client want to throw this grenade as soon as possible?
	// Backpacks
public:
	void PackDeadPlayerItems( void );
	//------------------------------------------------------------------------


public:
	void AddSpeedEffect(SpeedEffectType type, float duration, float speed, int mod = 0);
	bool IsSpeedEffectSet( SpeedEffectType type );
	void RemoveSpeedEffect(SpeedEffectType type);
	int	ClearSpeedEffects(int mod = 0);

	void Infect( CFFPlayer * );
	void Cure( CFFPlayer * );
	void ApplyBurning( CFFPlayer *hIgniter, float scale = 1.0f );

	bool IsInfected( void ) const		{ return m_bInfected; }
	CBaseEntity *GetInfector( void )	{ return ( m_hInfector == NULL ) ? NULL : ( CBaseEntity * )m_hInfector; }
	int GetInfectorTeam( void ) const	{ return IsInfected() ? m_iInfectedTeam : TEAM_UNASSIGNED; }
	
	bool GetSpecialInfectedDeath( void ) const { return m_bSpecialInfectedDeath; }
	void SetSpecialInfectedDeath( void ) { m_bSpecialInfectedDeath = true; }
private:
	SpeedEffect m_vSpeedEffects[NUM_SPEED_EFFECTS];				// All speed effects impairing the player
	float m_fLastHealTick;									// When the last time the medic was healed
	float m_fLastInfectedTick;							// When the last health tick for infections was at
	// Mulch: wrapping in EHANDLE
	EHANDLE m_hInfector;									// Who infected this player
	bool m_bInfected;												// if this player is infected
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
	virtual const unsigned char *GetEncryptionKey( void );
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
		m_iNewSpyDisguise = iTeam;
		m_iNewSpyDisguise += iClass << 4;
		m_flFinishDisguise = gpGlobals->curtime + 1.0f;
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

	int Heal(float);

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
	void Concuss( float amount, const QAngle *viewjerk = NULL );

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

	float m_flLastGassed;	// Last time we took gas damage, so that gas grens won't be cumulative

	int AddAmmo(const char* ammo, unsigned int amount);
	virtual int GiveAmmo(int iCount, int iAmmoIndex, bool bSuppressSound = false);
	int	GiveAmmo(int iCount, const char *szName, bool bSuppressSound = false);

	int m_iMaxAmmo[MAX_AMMO_TYPES];


	void SetDisguisable(bool in) 
	{
		DevMsg("Disguisable set to %d\n", in);
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

	CNetworkVar( int, m_iSpyDisguise );	// Mulch: Want to tell the client our current disguise
	CNetworkVar(int, m_iSpawnInterpCounter);

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

	virtual void Touch(CBaseEntity *pOther);
	void	InstaSwitch(int iClassNum);
	
	void	SetupClassVariables();

	virtual void FireBullets(const FireBulletsInfo_t &info);
	virtual bool HandleShotImpactingWater(const FireBulletsInfo_t &info, const Vector &vecEnd, ITraceFilter *pTraceFilter, Vector *pVecTracerDest);

	void		SpySabotageThink();
	float		m_flNextSpySabotageThink;
	float		m_flSpySabotageFinish;
	CHandle<CFFBuildableObject>	m_hSabotaging;
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
