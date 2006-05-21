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
#include "beam_shared.h"

#ifdef CLIENT_DLL 
	#define CFFMiniTurret C_FFMiniTurret
	#define CAI_BaseNPC C_AI_BaseNPC
	#define CFFMiniTurretLaser C_FFMiniTurretLaser	
/*
	#include "iviewrender_beams.h"	
	#include "beamdraw.h"
	*/
	#include "c_ai_basenpc.h"
#else
	#include "ai_basenpc.h"
#endif

//=============================================================================
//
// Class CFFMiniTurretLaser
//
//=============================================================================

#define FF_MINITURRET_BEAM			"effects/bluelaser1.vmt"
#define FF_MINITURRET_DOT			"sprites/redglow1.vmt"
#define FF_MINITURRET_HALO			"sprites/muzzleflash1.vmt"

class CFFMiniTurretLaser : public CSprite
{
public:
	DECLARE_CLASS( CFFMiniTurretLaser, CSprite );
	DECLARE_NETWORKCLASS();
	DECLARE_DATADESC();

	CFFMiniTurretLaser( void );
	~CFFMiniTurretLaser( void );

	static CFFMiniTurretLaser *Create( const Vector& vecOrigin, CBaseEntity *pOwner = NULL );

	bool IsOn( void ) const	{ return m_bIsOn; }
	void TurnOn( void ) 	{ m_bIsOn = true; RemoveEffects( EF_NODRAW ); }
	void TurnOff( void ) 	{ m_bIsOn = false; AddEffects( EF_NODRAW ); }
	//void Toggle( void ) 	{ m_bIsOn = !m_bIsOn; }
	virtual void			Spawn( void );

	int ObjectCaps( void )	{ return( BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION ) | FCAP_DONT_SAVE; }

#ifdef CLIENT_DLL
	virtual bool			IsTransparent( void ) { return true; }
	virtual RenderGroup_t	GetRenderGroup( void ) { return RENDER_GROUP_TRANSLUCENT_ENTITY; }
	virtual void			OnDataChanged( DataUpdateType_t updateType );
	virtual bool			ShouldDraw( void ) { return IsOn(); }
	virtual void			ClientThink( void );

	/*CHandle< CBeam >*/CBeam *m_hLaser;
#else
	void					OnObjectThink( void );
#endif	

	CNetworkVar( bool, m_bIsOn );
};

//=============================================================================
//
// CFFMiniTurretLaser tables
//
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFMiniTurretLaser, DT_FFMiniTurretLaser ) 

BEGIN_NETWORK_TABLE( CFFMiniTurretLaser, DT_FFMiniTurretLaser ) 
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO( m_bIsOn ) ),
#else
	SendPropInt( SENDINFO( m_bIsOn ) ),
#endif
END_NETWORK_TABLE() 

BEGIN_DATADESC( CFFMiniTurretLaser )
	DEFINE_FIELD( m_bIsOn, FIELD_BOOLEAN ),
#ifdef GAME_DLL
	DEFINE_THINKFUNC( OnObjectThink ),
#endif
END_DATADESC()

LINK_ENTITY_TO_CLASS( env_ffminiturretlaser, CFFMiniTurretLaser );

//=============================================================================
//
//	class CFFMiniTurret
//
//=============================================================================

#define	FF_MINITURRET_MODEL			"models/buildable/respawn_turret/respawn_turret.mdl"
//#define FF_MINITURRET_GLOW_SPRITE	"sprites/glow1.vmt"
#define FF_MINITURRET_BC_YAW		"aim_yaw"
#define FF_MINITURRET_BC_PITCH		"aim_pitch"

#define FF_MINITURRET_PING_TIME		5.0
#define FF_MINITURRET_MAX_WAIT		5.0
#define FF_MINITURRET_RANGE			1024

#define FF_MINITURRET_MAX_PITCH		0.0f
#define FF_MINITURRET_MIN_PITCH		-90.0f

#define FF_MINITURRET_MUZZLE_ATTACHMENT	"barrel01"
#define FF_MINITURRET_EYE_ATTACHMENT	"eyes"

//=============================================================================
//
//	class CFFMiniTurret
//
//=============================================================================
class CFFMiniTurret : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CFFMiniTurret, CAI_BaseNPC );
	DECLARE_NETWORKCLASS();	
	DECLARE_PREDICTABLE();

	// --> Shared code
	CFFMiniTurret( void );
	~CFFMiniTurret( void );

	virtual void Precache( void );
	Class_T	Classify( void ) { return CLASS_TURRET; }

	Vector	EyePosition( void )
	{
		SetupAttachments();
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
		SetupAttachments();
		GetAttachment( m_iMuzzleAttachment, vecOrigin, vecAngles );

		return vecOrigin;
	}
	void	MuzzlePosition( Vector& vecOrigin, QAngle& vecAngles )
	{
		SetupAttachments();
		GetAttachment( m_iMuzzleAttachment, vecOrigin, vecAngles );
	}
	void	LaserPosition( Vector& vecOrigin, QAngle& vecAngles )
	{
		SetupAttachments();
		GetAttachment( m_iLaserAttachment, vecOrigin, vecAngles );
	}

	void	SetupAttachments( void )
	{
		if( m_iMuzzleAttachment == -1 )
			m_iMuzzleAttachment = LookupAttachment( FF_MINITURRET_MUZZLE_ATTACHMENT );
		if( m_iEyeAttachment == -1 )
			m_iEyeAttachment = LookupAttachment( FF_MINITURRET_EYE_ATTACHMENT );
		if( m_iLaserAttachment == -1 )
			m_iLaserAttachment = LookupAttachment( FF_MINITURRET_EYE_ATTACHMENT );
	}

protected:
	int		m_iEyeAttachment;
	int		m_iMuzzleAttachment;
	int		m_iLaserAttachment;
	// <-- Shared code

#ifdef CLIENT_DLL 
	virtual void OnDataChanged( DataUpdateType_t updateType ) 
	{
		BaseClass::OnDataChanged( updateType );

		if( updateType == DATA_UPDATE_CREATED ) 
		{
			SetNextClientThink( CLIENT_THINK_ALWAYS );
		}
	}
#else
	DECLARE_DATADESC();
	
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

	// Inputs
	/*
	void	InputToggle( inputdata_t &inputdata );
	void	InputEnable( inputdata_t &inputdata );
	void	InputDisable( inputdata_t &inputdata );

	bool	IsValidEnemy( CBaseEntity *pEnemy );
	bool	CanBeAnEnemyOf( CBaseEntity *pEnemy );
	*/

	float	MaxYawSpeed( void );

protected:
	void	Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict = false );
	void	DoMuzzleFlash( void );
	void	Ping( void );	
	//void	Toggle( void );
	//void	Enable( void );
	//void	Disable( void );
	void	SpinUp( void );
	void	SpinDown( void );
	bool	UpdateFacing( void );
	void	EnableLaser( void );
	void	DisableLaser( void );

protected:
	// Team the turret is on
	int		m_iTeam;
	int		m_iAmmoType;
	float	m_flNextShell;

	bool	m_bActive;		// Denotes the turret is deployed and looking for targets
	bool	m_bEnabled;		// Denotes whether the turret is able to deploy or not

	float	m_flShotTime;
	float	m_flLastSight;
	//float	m_flPingTime;
	//float	m_flNextActivateSoundTime;

	int		m_iPitchPoseParameter;
	int		m_iYawPoseParameter;
		
	COutputEvent m_OnDeploy;
	COutputEvent m_OnRetire;

	// Aiming
	QAngle	m_vecGoalAngles;

	CHandle< CFFMiniTurretLaser >	m_hLaser;

	//DEFINE_CUSTOM_AI;
#endif // CLIENT_DLL
};

#endif // FF_MINITURRET_H
