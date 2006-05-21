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
//
//	5/19/2006,	Mulchman:
//		Fixed some yaw code. Beginning to add the laser effect.


#include "cbase.h"
#include "ff_miniturret.h"
//#include "debugoverlay_shared.h"

#ifdef CLIENT_DLL 	
	#include "c_ff_player.h"
#else	
	#include "ammodef.h"
	#include "ff_gamerules.h"
	#include "ff_buildableobjects_shared.h"
	#include "te_effect_dispatch.h"
	#include "ff_player.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static int g_iMiniTurretBeam, g_iMiniTurretHalo, g_iMiniTurretDot;

//=============================================================================
//
// Class CFFMiniTurretLaser
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Create a laser
//-----------------------------------------------------------------------------
CFFMiniTurretLaser::CFFMiniTurretLaser( void )
{
#ifdef CLIENT_DLL
	m_hLaser = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Create a laser
//-----------------------------------------------------------------------------
CFFMiniTurretLaser::~CFFMiniTurretLaser( void )
{
#ifdef CLIENT_DLL
	if( m_hLaser )
	{
		m_hLaser->Remove();		
		//UTIL_Remove( m_hLaser );
		m_hLaser = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Create a laser
//-----------------------------------------------------------------------------
CFFMiniTurretLaser *CFFMiniTurretLaser::Create( const Vector& vecOrigin, CBaseEntity *pOwner ) 
{
#ifdef GAME_DLL
	CFFMiniTurretLaser *pLaser = ( CFFMiniTurretLaser * )CBaseEntity::Create( "env_ffminiturretlaser", vecOrigin, QAngle( 0, 0, 0 ) );

	if( !pLaser )
		return NULL;

	pLaser->SetRenderMode( ( RenderMode_t )9 );

	pLaser->SetMoveType( MOVETYPE_NONE );
	pLaser->AddSolidFlags( FSOLID_NOT_SOLID );
	pLaser->AddEffects( EF_NOSHADOW );
	UTIL_SetSize( pLaser, vec3_origin, vec3_origin );

	pLaser->SetOwnerEntity( pOwner );

	pLaser->AddEFlags( EFL_FORCE_CHECK_TRANSMIT );

	pLaser->SpriteInit( FF_MINITURRET_DOT, vecOrigin );
	pLaser->SetName( AllocPooledString( "FF_MINITURRET_LASER" ) );
	pLaser->SetTransparency( kRenderWorldGlow, 255, 255, 255, 255, kRenderFxNoDissipation );
	pLaser->SetScale( 0.25f );
	//pLaser->SetOwnerEntity( pOwner );
	pLaser->SetSimulatedEveryTick( true );

	return pLaser;
#else
	return NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Spawn function
//-----------------------------------------------------------------------------
void CFFMiniTurretLaser::Spawn( void )
{
	BaseClass::Spawn();

#ifdef CLIENT_DLL
	// Create the laser beam
	//Vector vecOrigin = GetAbsOrigin();
	//m_hLaser = CBeam::BeamCreate( FF_MINITURRET_BEAM, 1.0f );
	//m_hLaser->PointsInit( vecOrigin, vecOrigin );

	//SetThink( &CFFMiniTurretLaser::OnObjectThink );
	
#else
	SetThink( &CFFMiniTurretLaser::OnObjectThink );
	SetNextThink( gpGlobals->curtime );
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Draw our sprite
//-----------------------------------------------------------------------------
void CFFMiniTurretLaser::OnObjectThink( void )
{
	SetNextThink( gpGlobals->curtime );

	if( !IsOn() )
		return;

	CFFMiniTurret *pOwner = dynamic_cast< CFFMiniTurret * >( GetOwnerEntity() );
	if( !pOwner )
	{
		AssertMsg( false, "Mini Turret laser has no owner!" );
		return;
	}

	Vector vecOrigin, vecForward, vecEndPos;
	QAngle vecAngles;	
	bool bDrawDot = true;

	pOwner->LaserPosition( vecOrigin, vecAngles );	
	AngleVectors( vecAngles, &vecForward );

	VectorNormalizeFast( vecForward );

	trace_t tr;
	//UTIL_TraceLine( vecOrigin, vecOrigin + ( vecForward * MAX_TRACE_LENGTH ), MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_HITBOX, pOwner, COLLISION_GROUP_NONE, &tr );
	UTIL_TraceLine( vecOrigin, vecOrigin + ( vecForward * MAX_TRACE_LENGTH ), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );

//	if( tr.DidHit() )
//	{
//		if( tr.m_pEnt )
//		{
//			Warning( "[Mini Turret Laser] Hit %s", tr.m_pEnt->GetClassname() );
//			if( tr.m_pEnt->IsPlayer() )
//				Warning( " (%s)\n", ToFFPlayer( tr.m_pEnt )->GetPlayerName() );
//			else
//				Warning( "\n" );
//		}
//	}

	vecEndPos = tr.endpos + ( tr.plane.normal * 1.0f );

	//debugoverlay->AddBoxOverlay( vecOrigin, -Vector( 5, 5, 5 ), Vector( 5, 5, 5 ), vecAngles, 255, 255, 255, 255, 2 );
	//debugoverlay->AddBoxOverlay( vecEndPos, -Vector( 5, 5, 5 ), Vector( 5, 5, 5 ), vecAngles, 255, 255, 255, 255, 2 );

	if( tr.surface.flags & SURF_SKY )
		bDrawDot = false;

	if( !bDrawDot )
	{
		if( IsOn() )
			TurnOff();
	}
	else
	{
		if( !IsOn() )
			TurnOn();
		SetAbsOrigin( vecEndPos );
	}
}
#endif // GAME_DLL

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Client think function
//-----------------------------------------------------------------------------
void CFFMiniTurretLaser::ClientThink( void )
{
	// Draw beams on the client since beam drawn on server
	// doesn't get predicted so it doesn't line up with the
	// laser dot. But, since the trace doesn't work on the
	// client I'm making the laser fade out after x units
	// so it could go back to being drawn on the server
	// just fine...


	if( !IsOn() )
		return;

	// Get the owner
	CFFMiniTurret *pOwner = dynamic_cast< CFFMiniTurret * >( GetOwnerEntity() );
	if( !pOwner )
	{
		AssertMsg( false, "Mini Turret laser has no owner!" );
		return;
	}

	// Get the origin & angles for the laser beam
	Vector vecOrigin, vecForward, vecEndPos;
	QAngle vecAngles;

	pOwner->LaserPosition( vecOrigin, vecAngles );	
	AngleVectors( vecAngles, &vecForward );

	// If the laser beam exists... update its position
	if( m_hLaser )
	{
		VectorNormalizeFast( vecForward );

		// THIS SHIT DOES NOT HIT PLAYERS FOR WHATEVER REASON... MAKES NO SENSE
		/*
		trace_t tr;
		//UTIL_TraceLine( vecOrigin, vecOrigin + ( vecForward * MAX_TRACE_LENGTH ), MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_HITBOX, pOwner, COLLISION_GROUP_NONE, &tr );
		UTIL_TraceLine( vecOrigin, vecOrigin + ( vecForward * MAX_TRACE_LENGTH ), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );

		if( tr.DidHit() )
		{
			if( tr.m_pEnt )
			{
				Warning( "[Mini Turret Laser] [Client] Hit %s", tr.m_pEnt->GetClassname() );
				if( tr.m_pEnt->IsPlayer() )
					Warning( " (%s)\n", ToFFPlayer( tr.m_pEnt )->GetPlayerName() );
				else
					Warning( "\n" );
			}
		}

		vecEndPos = tr.endpos + ( tr.plane.normal * 1.0f );
		*/

		// Since the trace doesn't work, make the laser fade out
		vecEndPos = vecOrigin + ( vecForward * 48.0f );
		m_hLaser->SetFadeLength( 32.0f );

		m_hLaser->SetAbsStartPos( vecOrigin );
		m_hLaser->SetAbsEndPos( vecEndPos );
		m_hLaser->RelinkBeam();
	}
	else
	{
		// Create laser beam
		m_hLaser = CBeam::BeamCreate( FF_MINITURRET_BEAM, 1.0f );
		if( !m_hLaser )
			Warning( "[Mini Turret Laser] Failed to create laser beam\n" );
		else
		{
			m_hLaser->PointsInit( vecOrigin, vecEndPos );
			m_hLaser->SetColor( 255, 0, 0 );
			m_hLaser->SetBrightness( 255 );
			m_hLaser->SetNoise( 0.0f );
			m_hLaser->SetEndWidth( 0.0f );
			m_hLaser->SetScrollRate( 0.0f );
		}
	}	

	BaseClass::ClientThink();
}

//-----------------------------------------------------------------------------
// Purpose: Setup our sprite reference
//-----------------------------------------------------------------------------
void CFFMiniTurretLaser::OnDataChanged( DataUpdateType_t updateType ) 
{
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED ) 
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}	
}
#endif

//=============================================================================
//
// CFFMiniTurretLaser tables
//
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFMiniTurret, DT_FFMiniTurret ) 

BEGIN_NETWORK_TABLE( CFFMiniTurret, DT_FFMiniTurret ) 
END_NETWORK_TABLE() 

BEGIN_PREDICTION_DATA( CFFMiniTurret ) 
END_PREDICTION_DATA()

LINK_ENTITY_TO_CLASS( ff_miniturret, CFFMiniTurret );
PRECACHE_REGISTER( ff_miniturret );

#ifdef GAME_DLL

// Debug visualization
ConVar	miniturret_debug( "ffdev_miniturret_debug", "0" );
ConVar	miniturret_turnspeed( "ffdev_miniturret_turnspeed", "17.0" );
ConVar	miniturret_castrate( "ffdev_miniturret_castrate", "0" );

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

#endif // GAME_DLL

//=============================================================================
//
//	class CFFMiniTurret
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFMiniTurret::CFFMiniTurret( void )
{
#ifdef GAME_DLL
	m_bActive = false;
	m_iAmmoType = -1;
	//m_flPingTime = 0;
	//m_flNextActivateSoundTime = 0;
	m_flShotTime = 0;
	m_flLastSight = 0;
	//m_bBlinkState = false;
	m_bEnabled = false;

	m_hLaser = NULL;
#endif // GAME_DLL

	m_iEyeAttachment = -1;
	m_iMuzzleAttachment = -1;
	m_iLaserAttachment = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CFFMiniTurret::~CFFMiniTurret( void )
{
#ifdef GAME_DLL
	if( m_hLaser != NULL )
	{
		UTIL_Remove( m_hLaser );
		m_hLaser = NULL;
	}
#endif
}
//-----------------------------------------------------------------------------
// Purpose: Precache assets
//-----------------------------------------------------------------------------
void CFFMiniTurret::Precache( void )
{
#ifdef GAME_DLL
	PrecacheModel( FF_MINITURRET_MODEL );

	// Activities
	ADD_CUSTOM_ACTIVITY( CFFMiniTurret, ACT_MINITURRET_OPEN );
	ADD_CUSTOM_ACTIVITY( CFFMiniTurret, ACT_MINITURRET_CLOSE );
	ADD_CUSTOM_ACTIVITY( CFFMiniTurret, ACT_MINITURRET_CLOSED_IDLE );
	ADD_CUSTOM_ACTIVITY( CFFMiniTurret, ACT_MINITURRET_OPEN_IDLE );
	ADD_CUSTOM_ACTIVITY( CFFMiniTurret, ACT_MINITURRET_FIRE );
#endif

	BaseClass::Precache();

	g_iMiniTurretDot = PrecacheModel( FF_MINITURRET_DOT );
	g_iMiniTurretBeam = PrecacheModel( FF_MINITURRET_BEAM );
	g_iMiniTurretHalo = PrecacheModel( FF_MINITURRET_HALO );	
}

#ifdef GAME_DLL

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

	//m_iMuzzleAttachment = LookupAttachment( FF_MINITURRET_MUZZLE_ATTACHMENT );
	//m_iEyeAttachment = LookupAttachment( FF_MINITURRET_EYE_ATTACHMENT );
	SetupAttachments();

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

		EnableLaser();
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

		DisableLaser();
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
// Purpose: Creates the laser
//-----------------------------------------------------------------------------
void CFFMiniTurret::EnableLaser( void )
{
	if( m_hLaser )
		return;

	Vector vecOrigin;
	QAngle vecAngles;

	LaserPosition( vecOrigin, vecAngles );

	m_hLaser = CFFMiniTurretLaser::Create( vecOrigin, this );

	AssertMsg( m_hLaser, "Failed to create Mini Turret laser!" );

	m_hLaser->TurnOn();
}

//-----------------------------------------------------------------------------
// Purpose: Destroys the laser
//-----------------------------------------------------------------------------
void CFFMiniTurret::DisableLaser( void )
{
	if( m_hLaser )
	{
		m_hLaser->TurnOff();
		UTIL_Remove( m_hLaser );
		m_hLaser = NULL;
	}
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

	if( !miniturret_castrate.GetBool() )
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

#endif // GAME_DLL
