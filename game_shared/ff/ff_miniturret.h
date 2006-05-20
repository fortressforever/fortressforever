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
//
//	5/19/2006,	Mulchman:
//		Fixed some yaw code. Beginning to add the laser effect.

#ifndef FF_MINITURRET_H
#define FF_MINITURRET_H

#ifdef _WIN32
#pragma once
#endif

#include "Sprite.h"

#ifdef CLIENT_DLL 
	#define CFFMiniTurretLaser C_FFMiniTurretLaser
	#include "iviewrender_beams.h"
	#include "beam_shared.h"
	#include "beamdraw.h"
#else
	#include "ai_basenpc.h"
#endif

#define FF_MINITURRET_BEAM			"effects/bluelaser1.vmt"
#define FF_MINITURRET_DOT			"sprites/redglow1.vmt"

//=============================================================================
//
// Class CFFMiniTurretLaser
//
//=============================================================================

class CFFMiniTurretLaser : public CSprite
{
public:
	DECLARE_CLASS( CFFMiniTurretLaser, CSprite );
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	CFFMiniTurretLaser( void ) {}
	~CFFMiniTurretLaser( void ) {}

	static CFFMiniTurretLaser *Create( const Vector& vecOrigin, CBaseEntity *pOwner = NULL );

	void SetLaserPosition( const Vector& vecOrigin );

	bool IsOn( void ) const	{ return m_bIsOn; }

	void TurnOn( void ) 	{ m_bIsOn = true; }
	void TurnOff( void ) 	{ m_bIsOn = false; }
	void Toggle( void ) 	{ m_bIsOn = !m_bIsOn; }

	int ObjectCaps( void )	{ return( BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION ) | FCAP_DONT_SAVE; }

#ifdef CLIENT_DLL
	virtual bool			IsTransparent( void ) { return true; }
	virtual RenderGroup_t	GetRenderGroup( void ) { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual int				DrawModel( int flags );
	virtual void			OnDataChanged( DataUpdateType_t updateType );
	virtual bool			ShouldDraw( void ) { return( IsEffectActive( EF_NODRAW) == false ); }
#endif

	CNetworkVar( float, m_flStartTime );

protected:
	bool				m_bIsOn;
};

//=============================================================================
//
// CFFMiniTurretLaser tables
//
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFMiniTurretLaser, DT_FFMiniTurretLaser ) 

BEGIN_NETWORK_TABLE( CFFMiniTurretLaser, DT_FFMiniTurretLaser ) 
#ifdef CLIENT_DLL
	RecvPropFloat( RECVINFO( m_flStartTime ) ) 
#else
	SendPropFloat( SENDINFO( m_flStartTime ) ) 
#endif
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS( env_ffminiturretlaser, CFFMiniTurretLaser );

BEGIN_DATADESC( CFFMiniTurretLaser ) 
	DEFINE_FIELD( m_bIsOn, FIELD_BOOLEAN ), 
END_DATADESC() 

//=============================================================================
//
//	class CFFMiniTurret
//
//=============================================================================

#ifdef GAME_DLL
#define	FF_MINITURRET_MODEL			"models/buildable/respawn_turret/respawn_turret.mdl"
#define FF_MINITURRET_GLOW_SPRITE	"sprites/glow1.vmt"
#define FF_MINITURRET_BC_YAW		"aim_yaw"
#define FF_MINITURRET_BC_PITCH		"aim_pitch"

#define FF_MINITURRET_PING_TIME		5.0
#define FF_MINITURRET_MAX_WAIT		5.0
#define FF_MINITURRET_RANGE			1024

#define FF_MINITURRET_MAX_PITCH		0.0f
#define FF_MINITURRET_MIN_PITCH		-90.0f

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
	virtual int	OnTakeDamage( const CTakeDamageInfo &info ) { return 0; }
	int		VPhysicsTakeDamage( const CTakeDamageInfo &info ) { return 0; }

	int		GetTeamNumber( void ) { return m_iTeam; }
	bool	IsPlayer( void ) const { return false; }
	bool	IsAlive( void ) { return true; }
	bool	IsActive( void ) { return m_bActive; }
	bool	BlocksLOS( void ) { return false; }
	int		BloodColor( void ) { return BLOOD_COLOR_MECH; }
	bool	ShouldSavePhysics( void ) { return false; }
	const char *GetTracerType( void ) { return "AR2Tracer"; }

	// Think functions
	void	OnObjectThink( void );	// Not a think function
	void	OnRetire( void );
	void	OnDeploy( void );
	void	OnActiveThink( void );
	void	OnSearchThink( void );
	void	OnAutoSearchThink( void );
	void	HackFindEnemy( void );

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

	float	MaxYawSpeed( void );

	Class_T	Classify( void ) { return CLASS_TURRET; }

	Vector	EyePosition( void )
	{
		/*
		Vector vecOrigin;
		QAngle vecAngles;

		GetAttachment( m_iEyeAttachment, vecOrigin, vecAngles );

		return vecOrigin - Vector( 0, 0, 48 );
		*/
		return GetAbsOrigin() - Vector( 0, 0, 16 );
	}

	Vector	MuzzlePosition( void )
	{
		Vector vecOrigin;
		QAngle vecAngles;

		GetAttachment( m_iMuzzleAttachment, vecOrigin, vecAngles );

		return vecOrigin;
	}
	void	MuzzlePosition( Vector &vecOrigin, QAngle &vecAngles )
	{
		GetAttachment( m_iMuzzleAttachment, vecOrigin, vecAngles );
	}

	//Vector	EyeOffset( Activity nActivity ) { return -Vector( 0, 0, 48 ); }

protected:
	void	Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict = false );
	void	DoMuzzleFlash( void );
	////void	SetEyeState( eyeState_t state );
	void	Ping( void );	
	//void	Toggle( void );
	//void	Enable( void );
	//void	Disable( void );
	void	SpinUp( void );
	void	SpinDown( void );
	bool	UpdateFacing( void );

protected:
	// Team the turret is on
	int		m_iTeam;
	int		m_iAmmoType;
	float	m_flNextShell;

	bool	m_bActive;		// Denotes the turret is deployed and looking for targets
	//bool	m_bBlinkState;
	bool	m_bEnabled;		// Denotes whether the turret is able to deploy or not

	float	m_flShotTime;
	float	m_flLastSight;
	//float	m_flPingTime;
	//float	m_flNextActivateSoundTime;

	int		m_iPitchPoseParameter;
	int		m_iYawPoseParameter;
	
	int						m_iEyeAttachment;
	int						m_iMuzzleAttachment;
	//eyeState_t				m_iEyeState;
	//CHandle<CSprite>		m_hEyeGlow;
	
	COutputEvent m_OnDeploy;
	COutputEvent m_OnRetire;

	// Aiming
	QAngle	m_vecGoalAngles;

	//DEFINE_CUSTOM_AI;
};

#endif // FF_MINITURRET_H

#endif // GAME_DLL
