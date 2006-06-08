// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_buildableobjects_shared.h
// @author Patrick O'Leary (Mulchman)
// @date 06/08/2005
// @brief Shared code for buildable objects
//
// REVISIONS
// ---------
// 06/08/2005, Mulchman: 
//		This file First created
// 22/01/2006, Mirv:
//		Rewritten a lot of this
//
//	05/10/2006,	Mulchman:
//		Adding radio tags to dispenser

#ifndef FF_BUILDABLEOBJECTS_SHARED_H
#define FF_BUILDABLEOBJECTS_SHARED_H

#ifdef _WIN32
	#pragma once
#endif

#include "debugoverlay_shared.h"

#ifdef CLIENT_DLL
	#include "c_ff_player.h"

	#define CAI_BaseNPC C_AI_BaseNPC
	#include "c_ai_basenpc.h"	
	
	#define CFFBuildableInfo C_FFBuildableInfo
	#define CFFBuildableObject C_FFBuildableObject
	#define CFFDispenser C_FFDispenser
	#define CFFSentryGun C_FFSentryGun
	#define CFFDetpack C_FFDetpack
	#define CFFSevTest C_FFSevTest
#else
	#include "ff_player.h"
	#include "ai_basenpc.h"
#endif

#define FF_DISPENSER_MODEL					"models/buildable/dispenser/dispenser.mdl"
#define FF_DISPENSER_GIB01_MODEL			"models/gibs/dispenser/gib1.mdl"
#define FF_DISPENSER_GIB02_MODEL			"models/gibs/dispenser/gib2.mdl"
#define FF_DISPENSER_GIB03_MODEL			"models/gibs/dispenser/gib3.mdl"
#define FF_DISPENSER_GIB04_MODEL			"models/gibs/dispenser/gib4.mdl"
#define FF_DISPENSER_BUILD_SOUND			"Dispenser.Build"
#define FF_DISPENSER_UNBUILD_SOUND			"Dispenser.unbuild"
#define FF_DISPENSER_EXPLODE_SOUND			"Dispenser.Explode"

#define FF_DETPACK_MODEL					"models/buildable/detpack/detpack.mdl"
#define FF_DETPACK_BUILD_SOUND				"Detpack.Build"
#define FF_DETPACK_EXPLODE_SOUND			"Detpack.Explode"

#define FF_SENTRYGUN_MODEL					"models/buildable/sg/sg_lvl1.mdl"
#define FF_SENTRYGUN_MODEL_LVL2				"models/buildable/sg/sg_lvl2.mdl"
#define FF_SENTRYGUN_MODEL_LVL3				"models/buildable/sg/sg_lvl3.mdl"
#define FF_SENTRYGUN_BUILD_SOUND			"Sentry.One"
#define FF_SENTRYGUN_UNBUILD_SOUND			"Sentry.unbuild"
#define FF_SENTRYGUN_EXPLODE_SOUND			"Sentry.Explode"

//#define FF_SENTRYGUN_AIMSPHERE_MODEL		"models/buildable/sg/sentrygun_aimsphere.mdl"

#define FF_SEVTEST_MODEL					"models/weapons/w_missile.mdl"

#define FF_BUILDALBE_GENERIC_GIB_MODEL_01	"models/gibs/random/randGib1.mdl"
#define FF_BUILDALBE_GENERIC_GIB_MODEL_02	"models/gibs/random/randGib2.mdl"
#define FF_BUILDALBE_GENERIC_GIB_MODEL_03	"models/gibs/random/randGib3.mdl"
#define FF_BUILDALBE_GENERIC_GIB_MODEL_04	"models/gibs/random/randGib4.mdl"
#define FF_BUILDALBE_GENERIC_GIB_MODEL_05	"models/gibs/random/randGib5.mdl"

#define FF_BUILDABLE_TIMER_BUILD_STRING		"FF_Building"
#define FF_BUILDABLE_TIMER_DETPACK_STRING	"FF_Detpack_Primed"

#define FF_BUILD_NONE		0
#define FF_BUILD_DISPENSER	1
#define FF_BUILD_SENTRYGUN	2
#define FF_BUILD_DETPACK	3

#define FF_BUILD_DISP_BUILD_DIST	36.0f
#define FF_BUILD_DISP_RAISE_VAL		16.0f
#define FF_BUILD_SG_BUILD_DIST		64.0f //54.0f
#define FF_BUILD_SG_RAISE_VAL		16.0f
#define FF_BUILD_DET_BUILD_DIST		42.0f
#define FF_BUILD_DET_RAISE_VAL		48.0f

#define FF_BUILD_DISP_STRING_LEN	256

// 30 Degrees
#define FF_BUILD_EPSILON			0.13f

// Using this value based off of the mins/maxs
#define FF_BUILD_DISP_HALF_WIDTH	12.0f

#define FF_DISPENSER_MINS	Vector( -12, -12, 0 )
#define FF_DISPENSER_MAXS	Vector( 12, 12, 48 )

#define FF_SENTRYGUN_MINS	Vector( -24, -21, 0 )
#define FF_SENTRYGUN_MAXS	Vector( 24, 21, 43 )

#define FF_DETPACK_MINS		Vector( -14, -14, 0 )
#define FF_DETPACK_MAXS		Vector( 14, 14, 11 )

#define FF_SOUND_BUILD		0	// Don't change these two values
#define FF_SOUND_EXPLODE	1

// Radius of the aimsphere model
//#define FF_SENTRYGUN_AIMSPHERE_RADIUS	128.0f
//#define FF_SENTRYGUN_AIMSPHERE_VISIBLE	(FF_SENTRYGUN_AIMSPHERE_RADIUS + 64.0f)

// Currently only the server uses these...
#ifdef CLIENT_DLL 
#else
	extern const char *g_pszFFDispenserModels[ ];
	extern const char *g_pszFFDispenserGibModels[ ];
	extern const char *g_pszFFDispenserSounds[ ];

	extern const char *g_pszFFDetpackModels[ ];
	extern const char *g_pszFFDetpackGibModels[ ];
	extern const char *g_pszFFDetpackSounds[ ];

	extern const char *g_pszFFSentryGunModels[ ];
	extern const char *g_pszFFSentryGunGibModels[ ];
	extern const char *g_pszFFSentryGunSounds[ ];

	extern const char *g_pszFFSevTestModels[ ];
	extern const char *g_pszFFSevTestGibModels[ ];
	extern const char *g_pszFFSevTestSounds[ ];

	extern const char *g_pszFFGenGibModels[ ];
#endif

enum BuildInfoResult_t
{
	BUILD_ALLOWED = 0,

	BUILD_NOROOM,
	BUILD_NOPLAYER,
	BUILD_TOOSTEEP,
	BUILD_TOOFAR,
	BUILD_INVALIDGROUND,

	BUILD_ERROR
};

class CFFBuildableInfo
{
private:
	CFFBuildableInfo( void ) {}

public:
	CFFBuildableInfo( CFFPlayer *pPlayer, int iBuildObject, float flBuildDist, float m_flRaiseVal );
	~CFFBuildableInfo( void ) {}

	// Returns true if the area in front of the player is
	// able to be built on (for the particular object trying
	// to be built)
	BuildInfoResult_t BuildResult( void ) const { return m_BuildResult; }

	Vector	GetBuildAirOrigin( void ) const { return m_vecBuildAirOrigin; }
	QAngle	GetBuildAirAngles( void ) const { return m_angBuildAirAngles; }

	Vector	GetBuildGroundOrigin( void ) const { return m_vecBuildGroundOrigin; }
	QAngle	GetBuildGroundAngles( void ) const { return m_angBuildGroundAngles; }

	//int OrientBuildableToGround( CBaseEntity *pEntity, int iBuildObject );

	// Tells the player why they couldn't build
	void	GetBuildError( void );

protected:
	// If the area in front of the player is able to be built on
	//bool	m_bBuildAreaClear;
	// Type of object we're trying to build
	int		m_iBuildObject;

	float	m_flBuildDist;
	float	m_flRaiseVal;
	float	m_flTestDist;

	// Player's info
	CFFPlayer *m_pPlayer;
	Vector	m_vecPlayerForward;
	Vector	m_vecPlayerRight;
	Vector	m_vecPlayerOrigin;

	// Since we spawn the object in the air when building
	// then drop it we need to have these
	Vector	m_vecBuildAirOrigin;
	QAngle	m_angBuildAirAngles;

	// Final position/angle on the ground of the object (if it can be built, of course)
	Vector	m_vecBuildGroundOrigin;
	QAngle	m_angBuildGroundAngles;

	// For the error parsing
	//int		m_iBuildError;

	BuildInfoResult_t	m_BuildResult;

protected:
	bool				IsGeometryInTheWay();
	//bool				IsGroundTooSteep();
	BuildInfoResult_t	CanOrientToGround();

};

//=============================================================================
//
//	class CFFBuildableObject / C_FFBuildableObject
//
//=============================================================================
class CFFBuildableObject : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CFFBuildableObject, CAI_BaseNPC )

#ifdef CLIENT_DLL
	DECLARE_CLIENTCLASS();
#else
	DECLARE_SERVERCLASS();
#endif

	// --> shared
	CFFBuildableObject();
	~CFFBuildableObject();
	
	virtual bool IsAlive( void ) { return true; }
	virtual bool IsPlayer( void ) const { return false; }
	virtual bool BlocksLOS( void ) { return true; }
	virtual int	BloodColor( void ) { return BLOOD_COLOR_MECH; } // |-- Mirv: Don't bleed
	bool IsBuilt( void	) const { return m_bBuilt; }
 
	CNetworkHandle( CBaseEntity, m_hOwner );

	int GetHealthPercent( void );
	unsigned int GetAmmoPercent( void ) { return m_iAmmoPercent; }

protected:
	CNetworkVarForDerived( unsigned int, m_iAmmoPercent );
	// <-- shared

#ifdef CLIENT_DLL
public:
	virtual void OnDataChanged( DataUpdateType_t updateType );

	virtual int	GetHealth( void ) const { return m_iHealth; }
	virtual int	GetMaxHealth( void ) const { return m_iMaxHealth; }

	bool CheckForOwner( void ) { return ( m_hOwner.Get() ); }

	bool m_bBuilt;	
#else
public:
	virtual void Spawn( void ); 
	virtual void Precache( void );
	
	void GoLive( void );
	void Detonate( void );
	void RemoveQuietly( void );
	
	virtual void Cancel( void ) 
	{
		// Stop the build sound
		StopSound( m_ppszSounds[ 0 ] );
		RemoveQuietly(); 
	}
	
	bool CheckForOwner( void )
	{
		if( !m_hOwner.Get() )
		{
			RemoveQuietly();
			return false;
		}

		return true;
	}
	//void EyeVectors( )

	// NOTE: Super class handles touch function
	// void OnObjectTouch( CBaseEntity *pOther );
	void OnObjectThink( void );

	virtual int OnTakeDamage( const CTakeDamageInfo &info );

	virtual void Event_Killed( const CTakeDamageInfo &info );

	virtual bool ShouldSavePhysics( void ) { return false; }
	virtual int TakeEmp( void ) { return 0; }

	// Mirv: Store in advance the ground position
	virtual void SetGroundAngles(const QAngle &ang) { m_angGroundAngles = ang; }
	virtual void SetGroundOrigin(const Vector &vec) { m_vecGroundOrigin = vec; }

private:
	// NOTE: Don't call the CFFBuildableObject::Create function
	static CFFBuildableObject *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

public:
	// So weapons (like the railgun) don't effect building
	virtual int VPhysicsTakeDamage( const CTakeDamageInfo &info );

protected:
	void Explode( void );
	void SpawnGib( const char *szGibModel, bool bFlame = true, bool bDieGroundTouch = false );
	void DoExplosion( void );

	virtual void SendStatsToBot() {};
protected:

	// Mirv: Store in advance the ground position
	QAngle m_angGroundAngles;
	Vector m_vecGroundOrigin;

	// Pointer to array of char *'s of model names
	const char **m_ppszModels;
	// Pointer to array of char *'s of gib model names
	const char **m_ppszGibModels;
	// Pointer to array of char *'s of sounds
	const char **m_ppszSounds;

	// For the explosion function

	// Explosion magnitude (int)
	int		m_iExplosionMagnitude;
	// Explosion magnitude (float)
	float	m_flExplosionMagnitude;
	// Explosion radius (float -> 3.5*magnitude)
	float	m_flExplosionRadius;
	// Explosion radius (int -> 3.5*magnitude)
	int		m_iExplosionRadius;
	// Explosion force
	float	m_flExplosionForce;
	// Explosion damage (for radius damage - same as flExplosion force)
	float	m_flExplosionDamage;
	// Explosion duration (duration of screen shaking)
	float	m_flExplosionDuration;
	// Explosion fireball scale
	int		m_iExplosionFireballScale;

	// Time (+ gpGlobals->curtime) that we will think (update network vars)
	float	m_flThinkTime;// = 0.2f;

	// Shockwave texture
	int		m_iShockwaveExplosionTexture;
	// Draw shockwaves
	bool	m_bShockWave;

	// Object is live and in color (not being built)
	CNetworkVar( bool, m_bBuilt );
	// Object takes damage once it is built
	bool	m_bTakesDamage;

	// Object has sounds associated with it
	bool	m_bHasSounds;

	// Whether or not the model is translucent
	// while building
	bool	m_bTranslucent;

	// If true we should be using physics
	bool	m_bUsePhysics;

#endif

};

//=============================================================================
//
//	class CFFSevTest / C_FFSevTest
//
//=============================================================================
class CFFSevTest : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFSevTest, CFFBuildableObject )

#ifdef CLIENT_DLL 
	DECLARE_CLIENTCLASS()
#else
	DECLARE_SERVERCLASS()
	DECLARE_DATADESC()
#endif

	// --> shared
	CFFSevTest( void );
	~CFFSevTest( void );
	virtual bool BlocksLOS( void ) { return false; }
	// <-- shared

#ifdef CLIENT_DLL
	virtual void OnDataChanged( DataUpdateType_t updateType );
#else
	void OnObjectThink( void );
	void Spawn( void );
	void GoLive( void );	

	static CFFSevTest *Create( const Vector &vecOrigin, const QAngle &vecAngles, edict_t *pentOwner = NULL );

protected:
	float	m_flSpawnTime;
#endif

};

//=============================================================================
//
//	class CFFDetpack / C_FFDetpack
//
//=============================================================================
class CFFDetpack : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFDetpack, CFFBuildableObject )

#ifdef CLIENT_DLL 
	
	DECLARE_CLIENTCLASS()
#else
	DECLARE_SERVERCLASS()
	DECLARE_DATADESC()
#endif

	// --> shared
	CFFDetpack( void );
	~CFFDetpack( void );

	virtual bool	BlocksLOS( void ) { return false; }
	virtual Class_T Classify( void ) { return CLASS_DETPACK; }
	// <-- shared

#ifdef CLIENT_DLL
	virtual void OnDataChanged( DataUpdateType_t updateType );

	// Creates a client side ONLY detpack - used for the build slot
	static C_FFDetpack *CreateClientSideDetpack( const Vector& vecOrigin, const QAngle& vecAngles );	
#else
	virtual void Spawn( void );
	void GoLive( void );

	void OnObjectTouch( CBaseEntity *pOther );
	void OnObjectThink( void );
	void SendStartTimerMessage( void );
	void SendStopTimerMessage( void );
	//void OnEmpExplosion( void );
	virtual int TakeEmp( void );

	static CFFDetpack *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

	//bool	m_bLive;
	int		m_iFuseTime;
	float	m_flDetonateTime;
	bool	m_bFiveSeconds;
#endif

};

//=============================================================================
//
//	class CFFDispenser / C_FFDispenser
//
//=============================================================================
class CFFDispenser : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFDispenser, CFFBuildableObject )

#ifdef CLIENT_DLL 
	DECLARE_CLIENTCLASS()
#else
	DECLARE_SERVERCLASS()
	DECLARE_DATADESC()
#endif

	// --> shared
	CFFDispenser( void );
	~CFFDispenser( void );

	virtual Class_T Classify( void ) { return CLASS_DISPENSER; }

public:
	// Network variables
	CNetworkVar( int, m_iCells );
	CNetworkVar( int, m_iShells );
	CNetworkVar( int, m_iNails );
	CNetworkVar( int, m_iRockets );
	CNetworkVar( int, m_iArmor );
	CNetworkVar( int, m_iRadioTags );

	int NeedsHealth( void ) { return m_iMaxHealth - m_iHealth; }
	int NeedsArmor( void ) { return m_iMaxArmor - m_iArmor; }
	int NeedsCells( void ) { return m_iMaxCells - m_iCells; }
	int NeedsShells( void ) { return m_iMaxShells - m_iShells; }
	int NeedsNails( void ) { return m_iMaxNails - m_iNails; }
	int NeedsRockets( void ) { return m_iMaxRockets - m_iRockets; }
	int NeedsRadioTags( void ) { return m_iMaxRadioTags - m_iRadioTags; }

protected:
	int		m_iMaxCells;
	int		m_iGiveCells;
	int		m_iMaxShells;
	int		m_iGiveShells;
	int		m_iMaxNails;
	int		m_iGiveNails;
	int		m_iMaxRockets;
	int		m_iGiveRockets;
	int		m_iMaxArmor;
	int		m_iGiveArmor;
	int		m_iMaxRadioTags;
	int		m_iGiveRadioTags;
	// <-- shared

public:

#ifdef CLIENT_DLL 
	virtual void OnDataChanged( DataUpdateType_t updateType );

	// Creates a client side ONLY dispenser - used for build slot
	static C_FFDispenser *CreateClientSideDispenser( const Vector& vecOrigin, const QAngle& vecAngles );
#else
	virtual void Spawn( void );
	void GoLive( void );

	void OnObjectTouch( CBaseEntity *pOther );
	void OnObjectThink( void );
	virtual void Event_Killed( const CTakeDamageInfo &info );

	CNetworkVar( unsigned int, m_iAmmoPercent );

	// Generic function to send hud messages to players
	void SendMessageToPlayer( CFFPlayer *pPlayer, const char *pszMessage, bool bDispenserText = false );
	void AddAmmo( int iArmor, int iCells, int iShells, int iNails, int iRockets, int iRadioTags );

	// Some functions for the custom dispenser text
	void SetText( const char *szCustomText ) { Q_strcpy( m_szCustomText, szCustomText ); }
	const char *GetText( void ) const { return m_szCustomText; }

	static CFFDispenser *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

protected:
	void SendStatsToBot( void );

	// Custom dispenser text string thing
	char		m_szCustomText[ FF_BUILD_DISP_STRING_LEN ];
	CFFPlayer	*m_pLastTouch;
	float		m_flLastTouch;

	// Actually give a player stuff
	void Dispense( CFFPlayer *pPlayer );

	// Calculates an adjustment to be made to the explosion
	// based on how much stuff is in the dispenser
	void CalcAdjExplosionVal( void );
	float	m_flOrigExplosionMagnitude;

	void UpdateAmmoPercentage( void );
#endif

};

//=============================================================================
//
//	class CFFSentryGun / C_FFSentryGun
//
//=============================================================================
class CFFSentryGun : public CFFBuildableObject
{
public:
	DECLARE_CLASS( CFFSentryGun, CFFBuildableObject )

#ifdef CLIENT_DLL 
	DECLARE_CLIENTCLASS()
#else
	DECLARE_SERVERCLASS()
	DECLARE_DATADESC()
#endif

	// --> shared
	CFFSentryGun( void );
	~CFFSentryGun( void );
	int GetRockets( void );

	virtual Class_T Classify( void ) { return CLASS_SENTRYGUN; }

public:
	// Network variables
	CNetworkVar( float, m_flRange );
	CNetworkVar( int, m_iLevel );
	CNetworkVar( int, m_iShells );
	CNetworkVar( int, m_iRockets );
	// <-- shared

#ifdef CLIENT_DLL 
	virtual void OnDataChanged( DataUpdateType_t updateType );

	// Creates a client side ONLY sentrygun - used for build slot
	static C_FFSentryGun *CreateClientSideSentryGun( const Vector& vecOrigin, const QAngle& vecAngles );

	// Mulch: now this is in buildableobject to extend to disp & sg
	// Mirv: Just going to store the ammo percentage here, with the msb
	// holding the rocket state
	//unsigned int m_iAmmoPercent;
#else
	virtual void Precache( void );
	virtual void Spawn( void );
	void GoLive( void );

	int TakeEmp( void );
	int NeedsHealth( void ) { return m_iMaxHealth - m_iHealth; }
	int NeedsShells( void ) { return m_iMaxShells - m_iShells; }
	int NeedsRockets( void ) { return m_iMaxRockets - m_iRockets; }

	void SetFocusPoint(Vector &origin);

	void OnObjectThink( void ); // NOTE: Not an actual think function but called during every think function
	void OnSearchThink( void );
	void OnActiveThink( void );

	void HackFindEnemy( void );

	float MaxYawSpeed( void );
	float MaxPitchSpeed( void );

protected:
	void Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict = false );
	void Ping( void );	
	void SpinUp( void );
	void SpinDown( void );
	bool UpdateFacing( void );

	void SendStatsToBot( void );

public:
	virtual void Event_Killed( const CTakeDamageInfo &info );

	const char *GetTracerType( void ) { return "SGTracer"; }

	virtual Vector EyePosition( void );
	Vector MuzzlePosition( void );
	Vector EyeOffset( Activity nActivity ) { return Vector( 0, 0, 64 ); }

	// Generic function to send hud messages to players
	void SendMessageToPlayer( CFFPlayer *pPlayer, const char *pszMessage );

	int GetLevel( void ) const { return m_iLevel; }
	void Upgrade( bool bUpgradeLevel = false, int iCells = 0, int iShells = 0, int iRockets = 0 );

	static CFFSentryGun *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

	virtual void DoMuzzleFlash( void );

public:
	CNetworkVar(unsigned int, m_iAmmoPercent);

	// Level-specific values
	int		m_iMaxShells;
	int		m_iMaxRockets;

	int		m_iShellDamage;

	float	m_flRocketCycleTime;
	float	m_flShellCycleTime;

	float	m_flTurnSpeed;
	float	m_flPitchSpeed;
	float	m_flLockTime;


	// Ammo definition for shells
	int		m_iAmmoType;
	float	m_flNextShell;
	float	m_flNextRocket;

	float	m_flLastSight;
	float	m_flPingTime;
	float	m_flNextActivateSoundTime;

	int		m_iEyeAttachment;
	int		m_iMuzzleAttachment;

	int m_iPitchPoseParameter;
	int m_iYawPoseParameter;

	//
	// Level 3 only stuff
	//
	// Which barrel to fire from (when level 3)
	bool m_bLeftBarrel;
	bool m_bRocketLeftBarrel;
	int m_iLBarrelAttachment;
	int m_iRBarrelAttachment;

	// Aiming
	QAngle	m_angGoal;
	QAngle	m_angAimBase;
	QAngle	m_angAiming;
#endif

};

#endif // FF_BUILDABLEOBJECTS_SHARED_H
