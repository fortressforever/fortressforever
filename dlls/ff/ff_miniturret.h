// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_turret.h
// @author Patrick O'Leary (Mulchman) 
// @date 6/1/2006
// @brief Turret class (for respawn turrets)
//
// REVISIONS
// ---------
//	6/1/2006, Mulchman: 
//		First created

#ifndef FF_MINITURRET_H
#define FF_MINITURRET_H

#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc.h"

#define	FF_MINITURRET_MODEL			"models/buildable/respawn_turret/respawn_turret.mdl"
#define FF_MINITURRET_GLOW_SPRITE	"sprites/glow1.vmt"
#define FF_MINITURRET_BC_YAW		"aim_yaw"
#define FF_MINITURRET_BC_PITCH		"aim_pitch"
#define FF_MINITURRET_YAW			"aim_yaw"
#define FF_MINITURRET_PITCH			"aim_pitch"

#define FF_MINITURRET_PING_TIME		5.0
#define FF_MINITURRET_MAX_WAIT		5.0
#define FF_MINITURRET_RANGE			1024

#define MAX_PITCH					90.0f
#define MIN_PITCH					-90.0f
#define SCAN_HALFWIDTH				40.0f

//=============================================================================
//
//	class CFFMiniTurret
//
//=============================================================================
class CFFMiniTurret : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CFFMiniTurret, CAI_BaseNPC );
	DECLARE_DATADESC();

	CFFMiniTurret( void );
	~CFFMiniTurret( void );

	void	Precache( void );
	void	Spawn( void );
	//void	Activate( void );
	//bool	CreateVPhysics( void );
	//virtual int	OnTakeDamage( const CTakeDamageInfo &info );
	//int		VPhysicsTakeDamage( const CTakeDamageInfo &info );

	int		GetTeamNumber( void ) { return m_iTeam; }
	bool	IsPlayer( void ) const { return false; }
	bool	IsAlive( void ) { return true; }
	bool	IsActive( void ) { return m_bActive; }
	bool	BlocksLOS( void ) { return false; }
	int		BloodColor( void ) { return BLOOD_COLOR_MECH; }
	bool	ShouldSavePhysics( void ) { return false; }
	const char *GetTracerType( void ) { return "AR2Tracer"; }

	// Think functions
	//void	OnObjectThink( void );	// Not a think function
	//void	OnRetire( void );
	//void	OnDeploy( void );
	//void	OnActiveThink( void );
	//void	OnSearchThink( void );
	//void	OnAutoSearchThink( void );
	//void	OnDisabledThink( void );
	//void	HackFindEnemy( void );

	//float	GetAttackDamageScale( CBaseEntity *pVictim );
	//Vector	GetAttackSpread( CBaseCombatWeapon *pWeapon, CBaseEntity *pTarget ) ;

	// Inputs
	/*
	void	InputToggle( inputdata_t &inputdata );
	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );

	bool	IsValidEnemy( CBaseEntity *pEnemy );
	bool	CanBeAnEnemyOf( CBaseEntity *pEnemy );
	*/

	//float	MaxYawSpeed( void );
	//float	MaxPitchSpeed( void );

	Class_T	Classify( void ) { return CLASS_TURRET; }

	Vector	EyePosition( void )
	{
		Vector vecOrigin;
		QAngle vecAngles;

		GetAttachment( m_iEyeAttachment, vecOrigin, vecAngles );

		return vecOrigin;
	}

	Vector	MuzzlePosition( void )
	{
		Vector vecOrigin;
		QAngle vecAngles;

		GetAttachment( m_iMuzzleAttachment, vecOrigin, vecAngles );

		return vecOrigin;
	}

	//Vector	EyeOffset( Activity nActivity ) { return Vector( 0, 0, 58 ); }

protected:
	////bool	PreThink( turretState_e state );
	//void	Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict = false );
	////void	SetEyeState( eyeState_t state );
	//void	Ping( void );	
	//void	Toggle( void );
	//void	Enable( void );
	//void	Disable( void );
	//void	SpinUp( void );
	//void	SpinDown( void );
	//bool	UpdateFacing( void );

protected:
	// Team the turret is on
	int		m_iTeam;
	int		m_iAmmoType;
	float	m_flNextShell;

	//bool	m_bAutoStart;
	bool	m_bActive;		// Denotes the turret is deployed and looking for targets
	bool	m_bBlinkState;
	bool	m_bEnabled;		// Denotes whether the turret is able to deploy or not

	float	m_flShotTime;
	float	m_flLastSight;
	float	m_flPingTime;
	float	m_flNextActivateSoundTime;

	int		m_iPitchPoseParameter;
	int		m_iYawPoseParameter;
	
	//QAngle	m_vecGoalAngles;

	int						m_iEyeAttachment;
	int						m_iMuzzleAttachment;
	//eyeState_t				m_iEyeState;
	//CHandle<CSprite>		m_hEyeGlow;
	//CHandle<CTurretTipController>	m_pMotionController;

	//Vector	m_vecEnemyLKP;

	// physics influence
	//CHandle<CBasePlayer>	m_hPhysicsAttacker;
	//float					m_flLastPhysicsInfluenceTime;

	//static const char		*m_pShotSounds[];

	COutputEvent m_OnDeploy;
	COutputEvent m_OnRetire;

	// Aiming
	QAngle	m_angGoal;
	QAngle	m_angAimBase;
	QAngle	m_angAiming;

	//DEFINE_CUSTOM_AI;
};

#endif // FF_MINITURRET_H
