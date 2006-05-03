// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_turret.cpp
// @author Patrick O'Leary (Mulchman) 
// @date 6/1/2006
// @brief Turret class (for respawn turrets)
//
// REVISIONS
// ---------
//	6/1/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "ff_miniturret.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
//	class CFFMiniTurret
//
//=============================================================================

// Quick conversions from angles to pitchparamter style
#define TO_PITCH(x) ((x) > 180 ? (360 - (x)) : ((x) * -1)) 
#define FROM_PITCH(x) ((x) > 0 ? (360 - (x)) : ((x) * -1)) 

#define TO_YAW(x) ((x) < -180 ? ((x) + 360) : ((x) > 180) ? ((x) - 360) : (x)) 

// Debug visualization
ConVar	miniturret_debug( "ffdev_miniturret_debug", "1" );
ConVar	miniturret_turnspeed( "ffdev_miniturret_turnspeed", "16.0" );
ConVar	miniturret_pitchspeed( "ffdev_miniturret_pitchspeed", "10.0" );

LINK_ENTITY_TO_CLASS( ff_miniturret, CFFMiniTurret );
PRECACHE_REGISTER( ff_miniturret );

// Datatable
BEGIN_DATADESC( CFFMiniTurret )

	DEFINE_FIELD( m_iAmmoType, FIELD_INTEGER ),
	DEFINE_FIELD( m_iTeam, FIELD_INTEGER ),
	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bBlinkState, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bEnabled, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_flShotTime,	FIELD_TIME ),
	DEFINE_FIELD( m_flLastSight, FIELD_TIME ),
	DEFINE_FIELD( m_flPingTime,	FIELD_TIME ),
	DEFINE_FIELD( m_flNextActivateSoundTime, FIELD_TIME ),

	//DEFINE_FIELD( m_vecGoalAngles,FIELD_VECTOR ),
	DEFINE_FIELD( m_iEyeAttachment,	FIELD_INTEGER ),
	DEFINE_FIELD( m_iMuzzleAttachment,	FIELD_INTEGER ),
	//DEFINE_FIELD( m_iEyeState,		FIELD_INTEGER ),
	//DEFINE_FIELD( m_hEyeGlow,		FIELD_EHANDLE ),

//	DEFINE_THINKFUNC( OnRetire ),
//	DEFINE_THINKFUNC( OnDeploy ),
//	DEFINE_THINKFUNC( OnActiveThink ),
//	DEFINE_THINKFUNC( OnSearchThink ),
//	DEFINE_THINKFUNC( OnAutoSearchThink ),
//	DEFINE_THINKFUNC( OnDisabledThink ),

	// Inputs
//	DEFINE_INPUTFUNC( FIELD_VOID, "Toggle", InputToggle ),
//	DEFINE_INPUTFUNC( FIELD_VOID, "Enable", InputEnable ),
//	DEFINE_INPUTFUNC( FIELD_VOID, "Disable", InputDisable ),

	DEFINE_OUTPUT( m_OnDeploy, "OnDeploy" ),
	DEFINE_OUTPUT( m_OnRetire, "OnRetire" ),

END_DATADESC()

// Activities
int ACT_MINITURRET_OPEN;
int ACT_MINITURRET_CLOSE;
int ACT_MINITURRET_OPEN_IDLE;
int ACT_MINITURRET_CLOSED_IDLE;
int ACT_MINITURRET_FIRE;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFMiniTurret::CFFMiniTurret( void )
{
	m_bActive = false;
	m_iAmmoType = -1;
	m_flPingTime = 0;
	m_flNextActivateSoundTime = 0;
	m_flShotTime = 0;
	m_flLastSight = 0;
	m_bBlinkState = false;
	m_bEnabled = false;

	//m_vecGoalAngles.Init();
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CFFMiniTurret::~CFFMiniTurret( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Precache assets
//-----------------------------------------------------------------------------
void CFFMiniTurret::Precache( void )
{
	PrecacheModel( FF_MINITURRET_MODEL );

	// Activities
	ADD_CUSTOM_ACTIVITY( CFFMiniTurret, ACT_MINITURRET_OPEN );
	ADD_CUSTOM_ACTIVITY( CFFMiniTurret, ACT_MINITURRET_CLOSE );
	ADD_CUSTOM_ACTIVITY( CFFMiniTurret, ACT_MINITURRET_CLOSED_IDLE );
	ADD_CUSTOM_ACTIVITY( CFFMiniTurret, ACT_MINITURRET_OPEN_IDLE );
	ADD_CUSTOM_ACTIVITY( CFFMiniTurret, ACT_MINITURRET_FIRE );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn a turret
//-----------------------------------------------------------------------------
void CFFMiniTurret::Spawn( void )
{
	Precache();

	SetSolid( SOLID_BBOX );
	SetMoveType( MOVETYPE_NONE );

	SetModel( FF_MINITURRET_MODEL );	

	BaseClass::Spawn();

	SetBlocksLOS( false );

	//m_HackedGunPos	= Vector( 0, 0, 12.75 );
	//SetViewOffset( EyeOffset( ACT_IDLE ) );
	m_flFieldOfView	= 0.4f; // 60 degrees
	m_takedamage	= DAMAGE_EVENTS_ONLY;
	m_iHealth		= 100;
	m_iMaxHealth	= 100;

	AddFlag( FL_AIMTARGET );
	AddEFlags( EFL_NO_DISSOLVE );

	SetPoseParameter( FF_MINITURRET_BC_YAW, 0 );
	SetPoseParameter( FF_MINITURRET_BC_PITCH, 0 );

	m_iAmmoType = GetAmmoDef()->Index( "SHELLS" );

	m_iMuzzleAttachment = LookupAttachment( "barrel01" );
	m_iEyeAttachment = LookupAttachment( "eyes" );

	m_iPitchPoseParameter = LookupPoseParameter( FF_MINITURRET_BC_PITCH );
	m_iYawPoseParameter = LookupPoseParameter( FF_MINITURRET_BC_YAW );

	// Set our state
	m_bEnabled = true;

	//SetThink( &CFFMiniTurret::OnDisabledThink );
	//SetEyeState( TURRET_EYE_DISABLED );

	// Stagger our starting times
	SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.1f, 0.3f ) );
	//CreateVPhysics();
}
