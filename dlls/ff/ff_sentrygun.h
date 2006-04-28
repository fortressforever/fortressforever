// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_sentrygun.h
// @author Patrick O'Leary (Mulchman)
// @date 12/28/2005
// @brief SentryGun class
//
// REVISIONS
// ---------
//	12/28/2005, Mulchman: 
//		First created
//
//	05/09/2005, Mulchman:
//		Tons of little updates here and there
//
//	05/17/2005, Mulchman:
//		Starting to make it animated and such
//
//	06/01/2005, Mulchman:
//		Noticed I had dates wrong... *cough* and
//		still working on making the SG animated
//		and such.
//
//	06/08/2005, Mulchman:
//		Decided the SG needs to inherit from the
//		AI base class and not the buildable class.
//		Some easy stuff will give it the same base
//		buildable attributes while inheriting all
//		of the AI stuff that it so desperately needs!

#ifndef FF_SENTRYGUN_H
#define FF_SENTRYGUN_H

#ifdef _WIN32
#pragma once
#endif

//#include "cbase.h"
//#include "ai_basenpc.h"
#include "utlvector.h"
#include "ff_buildableobjects_shared.h"

#define	FLOOR_TURRET_MODEL			"models/combine_turrets/floor_turret.mdl"
#define FLOOR_TURRET_GLOW_SPRITE	"sprites/glow1.vmt"
#define FLOOR_TURRET_BC_YAW			"aim_yaw"
#define FLOOR_TURRET_BC_PITCH		"aim_pitch"
#define FF_BUILD_SG_YAW				"aim_yaw"
#define FF_BUILD_SG_PITCH			"aim_pitch"
#define	FLOOR_TURRET_RANGE			1200
#define	FLOOR_TURRET_MAX_WAIT		5
#define FLOOR_TURRET_SHORT_WAIT		2.0		// Used for FAST_RETIRE spawnflag
#define	FLOOR_TURRET_PING_TIME		10.0f	// LPB!!

#define MAX_PITCH					90.0f
#define MIN_PITCH					-90.0f
#define SCAN_HALFWIDTH				40.0f

//Spawnflags
// BUG: These all stomp Base NPC spawnflags. Any Base NPC code called by this
//		this class may have undesired side effects due to these being set.
#define SF_FLOOR_TURRET_AUTOACTIVATE		0x00000020
#define SF_FLOOR_TURRET_STARTINACTIVE		0x00000040
#define SF_FLOOR_TURRET_FASTRETIRE			0x00000080

/*
class CFFSentryGun : public CFFBuildableObject
{
	DECLARE_CLASS( CFFSentryGun, CFFBuildableObject )
	DECLARE_SERVERCLASS()
	DECLARE_DATADESC()

public:
	CFFSentryGun( void );
	~CFFSentryGun( void );

	virtual void Precache( void );
	virtual void Spawn( void );
	void GoLive( void );

	//void Detonate( void );
	//void RemoveQuietly( void );
	//void Cancel( void ) { RemoveQuietly( ); }

	int TakeEmp();

	void SetFocusPoint(Vector &origin);

	void OnObjectThink( void ); // NOTE: Not an actual think function but called during every think function
	void OnSearchThink( void );
	void OnActiveThink( void );

	void HackFindEnemy( void );

	float MaxYawSpeed();
	float MaxPitchSpeed();

	Vector GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget );

protected:
	void Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict = false );
	void Ping( void );	
	void SpinUp( void );
	void SpinDown( void );
	bool UpdateFacing( void );
	bool OnSide( void );

	void SendStatsToBot();
public:
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual int	BloodColor( void ) { return BLOOD_COLOR_MECH; }

	const char *GetTracerType( void ) { return "SGTracer"; }

	virtual Class_T	Classify( void ) { return CLASS_SENTRYGUN; }
	virtual Vector EyePosition( void );
	Vector	EyeOffset( Activity nActivity ) { return Vector( 0, 0, 64 ); }

	// Generic function to send hud messages to players
	void SendMessageToPlayer( CFFPlayer *pPlayer, const char *pszMessage );

	int GetLevel( void ) const { return m_iLevel; }
	void Upgrade( bool bUpgradeLevel = false, int iCells = 0, int iShells = 0, int iRockets = 0 );

	static CFFSentryGun *Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner = NULL );

	virtual void DoMuzzleFlash( void );

public:
	// Network variables and shiz
	int		m_iShells;
	int		m_iRockets;

	CNetworkVar(unsigned int, m_iAmmoPercent);

	CNetworkVar( float, m_flRange );	// Range (radius) in world units
	CNetworkVar( int, m_iLevel );	// Level of the SG

	// Level-specific values
	int		m_iMaxShells;
	int		m_iMaxRockets;

	int		m_iShellDamage;

	float	m_flRocketCycleTime;
	float	m_flShellCycleTime;

    float	m_flTurnSpeed;
	float	m_flLockTime;


	// Ammo definition for shells
	int		m_iAmmoType;

//	bool	m_bAutoStart;
//	bool	m_bActive;		// Denotes the turret is deployed and looking for targets
//	bool	m_bEnabled;		// Denotes whether the turret is able to deploy or not

	float	m_flNextShell;
	float	m_flNextRocket;


	float	m_flLastSight;
	float	m_flPingTime;
	float	m_flNextActivateSoundTime;


	int		m_iEyeAttachment;
	int		m_iMuzzleAttachment;

	int m_iPitchPoseParameter;
	int m_iYawPoseParameter;

	static const char		*m_pShotSounds[ ];

	// Aiming
	QAngle	m_angGoal;
	QAngle	m_angAimBase;
	QAngle	m_angAiming;
};

*/

#endif // FF_SENTRYGUN_H
