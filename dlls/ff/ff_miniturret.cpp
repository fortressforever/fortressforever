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
#include "ff_gamerules.h"
#include "ff_buildableobjects_shared.h"
#include "te_effect_dispatch.h" 

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
//	class CFFMiniTurret
//
//=============================================================================

// Debug visualization
ConVar	miniturret_debug( "ffdev_miniturret_debug", "0" );
ConVar	miniturret_turnspeed( "ffdev_miniturret_turnspeed", "17.0" );

LINK_ENTITY_TO_CLASS( ff_miniturret, CFFMiniTurret );
PRECACHE_REGISTER( ff_miniturret );

// Datatable
BEGIN_DATADESC( CFFMiniTurret )

	DEFINE_KEYFIELD( m_iTeam, FIELD_INTEGER, "team" ),

	DEFINE_FIELD( m_iAmmoType, FIELD_INTEGER ),
	DEFINE_FIELD( m_bActive, FIELD_BOOLEAN ),
	//DEFINE_FIELD( m_bBlinkState, FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bEnabled, FIELD_BOOLEAN ),

	DEFINE_FIELD( m_flShotTime,	FIELD_TIME ),
	DEFINE_FIELD( m_flLastSight, FIELD_TIME ),
	//DEFINE_FIELD( m_flPingTime,	FIELD_TIME ),
	//DEFINE_FIELD( m_flNextActivateSoundTime, FIELD_TIME ),

	DEFINE_FIELD( m_vecGoalAngles, FIELD_VECTOR ),
	DEFINE_FIELD( m_iEyeAttachment,	FIELD_INTEGER ),
	DEFINE_FIELD( m_iMuzzleAttachment,	FIELD_INTEGER ),
	//DEFINE_FIELD( m_iEyeState,		FIELD_INTEGER ),
	//DEFINE_FIELD( m_hEyeGlow,		FIELD_EHANDLE ),

	DEFINE_THINKFUNC( OnRetire ),
	DEFINE_THINKFUNC( OnDeploy ),
	DEFINE_THINKFUNC( OnActiveThink ),
	DEFINE_THINKFUNC( OnSearchThink ),
	DEFINE_THINKFUNC( OnAutoSearchThink ),

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
	//m_flPingTime = 0;
	//m_flNextActivateSoundTime = 0;
	m_flShotTime = 0;
	m_flLastSight = 0;
	//m_bBlinkState = false;
	m_bEnabled = false;
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
	//SetMoveType( MOVETYPE_NONE );

	SetModel( FF_MINITURRET_MODEL );	

	BaseClass::Spawn();

	SetBlocksLOS( false );

	SetViewOffset( EyeOffset( ACT_IDLE ) );
	m_flFieldOfView	= 0.4f; // 60 degrees
	m_takedamage	= DAMAGE_EVENTS_ONLY;
	m_iHealth		= 100;
	m_iMaxHealth	= 100;

	AddFlag( FL_AIMTARGET );
	AddEFlags( EFL_NO_DISSOLVE );

	SetPoseParameter( FF_MINITURRET_BC_YAW, 0 );
	SetPoseParameter( FF_MINITURRET_BC_PITCH, 0 );

	m_iAmmoType = GetAmmoDef()->Index( "AMMO_SHELLS" );

	m_iMuzzleAttachment = LookupAttachment( "barrel01" );
	m_iEyeAttachment = LookupAttachment( "eyes" );

	m_iPitchPoseParameter = LookupPoseParameter( FF_MINITURRET_BC_PITCH );
	m_iYawPoseParameter = LookupPoseParameter( FF_MINITURRET_BC_YAW );

	// Set our state
	m_bEnabled = true;

	SetThink( &CFFMiniTurret::OnAutoSearchThink );

	// Stagger our starting times
	SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.1f, 0.3f ) );

	// For whatever reason, it seems like the yaw has to be set to 180
	// or the yaw for the aiming will be off. The trepids might complain
	// that setting the angles in hammer has no effect, so we might
	// need to compensate for this later.
	SetAbsAngles( QAngle( 0, 180, 0 ) );

	m_vecGoalAngles.Init();

	//DevMsg( "[MiniTurret] On team: %i\n", m_iTeam - 1 );
}

//-----------------------------------------------------------------------------
// Purpose: Decided whether this new target is better than the current one
//-----------------------------------------------------------------------------
CBaseEntity *MiniTurret_IsBetterTarget( CBaseEntity *cur, CBaseEntity *latest, float distance ) 
{
	static float lastdistance = 0;

	if( !latest ) 
		return cur;

	if( !cur ) 
		return latest;

	// A player is always preferable to a buildable
	if( latest->IsPlayer() && !cur->IsPlayer() ) 
		return latest;

	// Go for the nearest
	if( distance < lastdistance ) 
		return latest;

	return cur;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMiniTurret::HackFindEnemy( void )
{
	// Our location - used later
	Vector vecOrigin = GetAbsOrigin();
	CBaseEntity *pTarget = NULL;

	for( int i = 0; i < gpGlobals->maxClients; i++ )
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );

		if( !pPlayer || !pPlayer->IsPlayer() || pPlayer->IsObserver() )
			continue;

		if( !pPlayer->IsAlive() )
			continue;

		if( m_iTeam != 0 )
			if( FFGameRules()->IsTeam1AlliedToTeam2( m_iTeam, pPlayer->GetTeamNumber() ) == GR_TEAMMATE )
				continue;

		if( FVisible( pPlayer->GetAbsOrigin() ) )
			pTarget = MiniTurret_IsBetterTarget( pTarget, pPlayer, ( pPlayer->GetAbsOrigin() - vecOrigin ).LengthSqr() );

		if( pPlayer->m_hSentryGun.Get() )
		{
			CFFSentryGun *pSentryGun = static_cast< CFFSentryGun * >( pPlayer->m_hSentryGun.Get() );
			if( FVisible( pSentryGun->GetAbsOrigin() ) )
				pTarget = MiniTurret_IsBetterTarget( pTarget, pSentryGun, ( pSentryGun->GetAbsOrigin() - vecOrigin ).LengthSqr() );
		}

		if( pPlayer->m_hDispenser.Get() )
		{
			CFFDispenser *pDispenser = static_cast< CFFDispenser * >( pPlayer->m_hDispenser.Get() );
			if( FVisible( pDispenser->GetAbsOrigin() ) )
				pTarget = MiniTurret_IsBetterTarget( pTarget, pDispenser, ( pDispenser->GetAbsOrigin() - vecOrigin ).LengthSqr() );
		}
	}

	SetEnemy( pTarget );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMiniTurret::OnObjectThink( void )
{
	// Animate
	StudioFrameAdvance();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMiniTurret::OnAutoSearchThink( void )
{
	if( miniturret_debug.GetBool() )
		DevMsg( "[MiniTurret] OnAutoSearchThink\n" );

	OnObjectThink();

	SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.1f, 0.3f ) );

	if( GetEnemy() && !GetEnemy()->IsAlive() )
		SetEnemy( NULL );

	if( !GetEnemy() )
	{
		HackFindEnemy();
	}

	if( GetEnemy() )
	{
		SetThink( &CFFMiniTurret::OnDeploy );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMiniTurret::OnDeploy( void )
{
	if( miniturret_debug.GetBool() )
		DevMsg( "[MiniTurret] OnDeploy\n" );

	OnObjectThink();

	//m_vecGoalAngles = GetAbsAngles();

	SetNextThink( gpGlobals->curtime );

	if( GetActivity() != ACT_MINITURRET_OPEN )
	{
		m_bActive = true;
		SetActivity( ( Activity )ACT_MINITURRET_OPEN );

		m_OnDeploy.FireOutput( NULL, this );
	}

	if( IsActivityFinished() )
	{
		SetActivity( ( Activity )ACT_MINITURRET_OPEN_IDLE );

		m_flShotTime = gpGlobals->curtime + 1.0f;
		SetThink( &CFFMiniTurret::OnSearchThink );
	}

	m_flLastSight = gpGlobals->curtime + FF_MINITURRET_MAX_WAIT;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMiniTurret::OnSearchThink( void )
{
	if( miniturret_debug.GetBool() )
		DevMsg( "[MiniTurret] OnSearchThink\n" );

	OnObjectThink();

	SetNextThink( gpGlobals->curtime + 0.05f );

	SetActivity( ( Activity )ACT_MINITURRET_OPEN_IDLE );

	if( GetEnemy() && !GetEnemy()->IsAlive() )
		SetEnemy( NULL );

	if( !GetEnemy() )
		HackFindEnemy();

	if( GetEnemy() )
	{
		m_flShotTime = gpGlobals->curtime + 0.1f;
		m_flLastSight = 0;
		SetThink( &CFFMiniTurret::OnActiveThink );
		SpinUp();
		return;
	}

	// Time to retract?
	if( gpGlobals->curtime > m_flLastSight )
	{
		m_flLastSight = 0;
		SetThink( &CFFMiniTurret::OnRetire );
		return;
	}

	m_vecGoalAngles.x = 10.0f;
	m_vecGoalAngles.y = m_vecGoalAngles.y + QAngle( 0, MaxYawSpeed(), 0 ).y;

	UpdateFacing();
	Ping();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMiniTurret::OnActiveThink( void )
{
	if( miniturret_debug.GetBool() )
		DevMsg( "[MiniTurret] OnActiveThink\n" );

	OnObjectThink();

	SetNextThink( gpGlobals->curtime + 0.1f );

	if( !m_bActive || !GetEnemy() )
	{
		Warning( "[MiniTurret] ActiveThink - !m_bActive || !GetEnemy()\n" );

		SetEnemy( NULL );
		m_flLastSight = gpGlobals->curtime + FF_MINITURRET_MAX_WAIT;
		SetThink( &CFFMiniTurret::OnSearchThink );
		//m_vecGoalAngles = GetAbsAngles();
		return;
	}

	Vector vecMuzzle = MuzzlePosition();
	Vector vecMidEnemy = GetEnemy()->GetAbsOrigin();

	bool bEnemyVisible = FVisible( GetEnemy() ) && GetEnemy()->IsAlive();

	Vector vecDirToEnemy = vecMidEnemy - vecMuzzle;
	//float flDistToEnemy = VectorNormalize( vecDirToEnemy );

	Vector vecDirToEnemyEyes = GetEnemy()->WorldSpaceCenter() - vecMuzzle;
	VectorNormalize( vecDirToEnemyEyes );

	QAngle vecAnglesToEnemy;
	VectorAngles( vecDirToEnemyEyes, vecAnglesToEnemy );

	// Draw debug info
	if( miniturret_debug.GetBool() )
	{
		NDebugOverlay::Line( EyePosition(), EyePosition() + ( vecDirToEnemyEyes * 256 ), 0, 0, 255, false, 2.0f );

		NDebugOverlay::Cross3D( vecMuzzle, -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, false, 0.05 );
		NDebugOverlay::Cross3D( GetEnemy()->WorldSpaceCenter(), -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, false, 0.05 );
		NDebugOverlay::Line( vecMuzzle, GetEnemy()->WorldSpaceCenter(), 0, 255, 0, false, 0.05 );

		NDebugOverlay::Cross3D( vecMuzzle, -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, false, 0.05 );
		NDebugOverlay::Cross3D( vecMidEnemy, -Vector(2,2,2), Vector(2,2,2), 0, 255, 0, false, 0.05 );
		NDebugOverlay::Line( vecMuzzle, vecMidEnemy, 0, 255, 0, false, 0.05f );
	}	

	// Current enemy is not visible
	if( !bEnemyVisible /*|| ( flDistToEnemy > FF_MINITURRET_RANGE )*/ )
	{
		Warning( "[MiniTurret] ActiveThink - !bEnemyVisible\n" );

		if( m_flLastSight )
		{
			m_flLastSight = gpGlobals->curtime + 0.5f;
		}
		else if( gpGlobals->curtime > m_flLastSight )
		{
			// Should we look for a new target?
			SetEnemy( NULL );
			m_flLastSight = gpGlobals->curtime + FF_MINITURRET_MAX_WAIT;
			SetThink( &CFFMiniTurret::OnSearchThink );
			//m_vecGoalAngles = GetAbsAngles();
			SpinDown();
			return;
		}
	}

	// Get aiming direction and angles
	Vector vecMuzzleDir;
	QAngle vecMuzzleAng;

	MuzzlePosition( vecMuzzle, vecMuzzleAng );
	AngleVectors( vecMuzzleAng, &vecMuzzleDir );

	if( miniturret_debug.GetBool() )
	{
		NDebugOverlay::Line( vecMuzzle, vecMuzzle + ( vecMuzzleDir * 128 ), 255, 0, 0, false, 2.0f );
	}

	if( m_flShotTime < gpGlobals->curtime )
	{
		// Fire the gun
		if( DotProduct( vecDirToEnemy, vecMuzzleDir ) >= DOT_10DEGREE ) // 10 degree slop
		{
			//SetActivity( ACT_RESET );
			//SetActivity( ( Activity )ACT_MINITURRET_FIRE );

			// Fire the weapon
			//Shoot( vecMuzzle, vecMuzzleDir );
			if( miniturret_debug.GetBool() )
				DevMsg( "[MiniTurret] SHOOT!\n" );

			Shoot( vecMuzzle, vecMuzzleDir );
		} 
	}
	else
	{
		//SetActivity( ( Activity )ACT_MINITURRET_OPEN_IDLE );
	}

	// If we can see our enemy, face it
	if( bEnemyVisible )
	{
		m_vecGoalAngles.x = vecAnglesToEnemy.x;
		m_vecGoalAngles.y = vecAnglesToEnemy.y;		
	}

	UpdateFacing();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFMiniTurret::OnRetire( void )
{
	if( miniturret_debug.GetBool() )
		DevMsg( "[MiniTurret] OnRetire\n" );

	OnObjectThink();

	m_vecGoalAngles = GetAbsAngles();
	SetNextThink( gpGlobals->curtime );

	if( GetActivity() != ACT_MINITURRET_CLOSE )
	{
		SetActivity( ( Activity )ACT_MINITURRET_OPEN_IDLE );

		if( !UpdateFacing() )
		{
			SetActivity( ( Activity )ACT_MINITURRET_CLOSE );

			m_OnRetire.FireOutput( NULL, this );
		}
	}
	else if( IsActivityFinished() )
	{
		m_bActive = false;
		m_flLastSight = 0;

		SetActivity( ( Activity )ACT_MINITURRET_CLOSED_IDLE );

		SetThink( &CFFMiniTurret::OnAutoSearchThink );
		SetNextThink( gpGlobals->curtime + 0.05f );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFFMiniTurret::UpdateFacing( void )
{
	bool bMoved = false;

	Vector vecOrigin;
	QAngle vecAngles;
	MuzzlePosition( vecOrigin, vecAngles );

	float flYaw = GetPoseParameter( m_iYawPoseParameter );

	if( miniturret_debug.GetBool() )
		Warning( "[MiniTurret] Current pose yaw: %f, vecAngles yaw: %f, goal yaw: %f\n", flYaw, vecAngles.y, m_vecGoalAngles.y );

	// Update yaw
	float flDiff = AngleNormalize( UTIL_ApproachAngle( AngleNormalize( m_vecGoalAngles.y ), AngleNormalize( flYaw ), MaxYawSpeed() ) );
	SetPoseParameter( m_iYawPoseParameter, flDiff );

	// Update pitch
	flDiff = -m_vecGoalAngles.x;
	SetPoseParameter( m_iPitchPoseParameter, flDiff );

	if( miniturret_debug.GetBool() )
		DevMsg( "[MiniTurret] Current Pitch: %f, Goal Pitch: %f, pitch val: %f\n", vecAngles.x, m_vecGoalAngles.x, flDiff );

	InvalidateBoneCache();

	return bMoved;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFMiniTurret::SpinUp( void )
{
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFMiniTurret::SpinDown( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMiniTurret::Ping( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CFFMiniTurret::MaxYawSpeed( void )
{
	if( GetEnemy() )
		return miniturret_turnspeed.GetFloat();

	return 3.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Shoot
//-----------------------------------------------------------------------------
void CFFMiniTurret::Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict )
{
	FireBulletsInfo_t info;

	Vector vecDir = vecDirToEnemy;
	//Vector vecDir = GetEnemy()->GetAbsOrigin() - MuzzlePosition();

	info.m_vecSrc = vecSrc;
	info.m_vecDirShooting = vecDir;
	info.m_iTracerFreq = 1;
	info.m_iShots = 1;
	info.m_pAttacker = this;
	info.m_vecSpread = VECTOR_CONE_PRECALCULATED;
	info.m_flDistance = MAX_COORD_RANGE;
	info.m_iAmmoType = m_iAmmoType;
	info.m_iDamage = 70.0f;

	FireBullets( info );
	DoMuzzleFlash();
}

//-----------------------------------------------------------------------------
// Purpose: Muzzle flash effect
//-----------------------------------------------------------------------------
void CFFMiniTurret::DoMuzzleFlash( void ) 
{
	CEffectData data;

	data.m_nAttachmentIndex = m_iMuzzleAttachment;
	data.m_nEntIndex = entindex();
	DispatchEffect( "MuzzleFlash", data );
}
