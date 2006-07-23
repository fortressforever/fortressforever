// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_turret.cpp
// @author Patrick O'Leary (Mulchman) 
// @date 5/1/2006
// @brief Turret class (for respawn turrets)
//
// REVISIONS
// ---------
//	5/1/2006, Mulchman: 
//		First created
//
//	5/19/2006,	Mulchman:
//		Fixed some yaw code. Beginning to add the laser effect.
//
//	5/21/2006,	Mulchman:
//		Added laser beam and laser dot.

#include "cbase.h"
#include "ff_miniturret.h"

#ifdef CLIENT_DLL 	
#else	
	#include "ammodef.h"
	#include "ff_gamerules.h"
	#include "ff_buildableobjects_shared.h"
	#include "te_effect_dispatch.h"
	#include "ff_entity_system.h"
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
// Purpose: Create the laser dot
//-----------------------------------------------------------------------------
CFFMiniTurretLaserDot *CFFMiniTurretLaserDot::Create( const Vector& vecOrigin, CBaseEntity *pOwner ) 
{
#ifdef GAME_DLL
	CFFMiniTurretLaserDot *pLaser = ( CFFMiniTurretLaserDot * )CBaseEntity::Create( "env_ffminiturretlaserdot", vecOrigin, QAngle( 0, 0, 0 ) );

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
	pLaser->SetName( AllocPooledString( "FF_MINITURRET_LASERDOT" ) );
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
void CFFMiniTurretLaserDot::Spawn( void )
{
	BaseClass::Spawn();

#ifdef GAME_DLL
	SetThink( &CFFMiniTurretLaserDot::OnObjectThink );
	SetNextThink( gpGlobals->curtime );
#endif
}

#ifdef GAME_DLL
//-----------------------------------------------------------------------------
// Purpose: Draw the sprite
//-----------------------------------------------------------------------------
void CFFMiniTurretLaserDot::OnObjectThink( void )
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

	// Let's do muzzle position instead so the dot will be where the barrel
	// is pointing and not where the laser is pointing (will hit players
	// better this way [w/ the dot]). Before, the dot was projected from the
	// laser origin so it would never be on the exact spot the barrel was
	// pointing at.
	// pOwner->LaserPosition( vecOrigin, vecAngles );	
	pOwner->MuzzlePosition( vecOrigin, vecAngles );
	AngleVectors( vecAngles, &vecForward );

	VectorNormalizeFast( vecForward );

	trace_t tr;
	//UTIL_TraceLine( vecOrigin, vecOrigin + ( vecForward * MAX_TRACE_LENGTH ), MASK_SOLID | CONTENTS_DEBRIS | CONTENTS_HITBOX, pOwner, COLLISION_GROUP_NONE, &tr );
	UTIL_TraceLine( vecOrigin, vecOrigin + ( vecForward * MAX_TRACE_LENGTH ), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr );

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
// Purpose: Setup our sprite reference
//-----------------------------------------------------------------------------
void CFFMiniTurretLaserDot::OnDataChanged( DataUpdateType_t updateType ) 
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
// Class CFFMiniTurretBeam
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFMiniTurretLaserBeam::CFFMiniTurretLaserBeam( void )
{
#ifdef CLIENT_DLL
	m_pBeam = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CFFMiniTurretLaserBeam::~CFFMiniTurretLaserBeam( void )
{
#ifdef CLIENT_DLL
	if( m_pBeam )
	{
		m_pBeam->Remove();
		m_pBeam = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Create the laser beam
//-----------------------------------------------------------------------------
CFFMiniTurretLaserBeam *CFFMiniTurretLaserBeam::Create( const Vector& vecOrigin, CBaseEntity *pOwner ) 
{
#ifdef GAME_DLL
	CFFMiniTurretLaserBeam *pObject = ( CFFMiniTurretLaserBeam * )CBaseEntity::Create( "env_ffminiturretlaserbeam", vecOrigin, QAngle( 0, 0, 0 ) );

	if( !pObject )
		return NULL;	

	pObject->SetMoveType( MOVETYPE_NONE );
	pObject->AddSolidFlags( FSOLID_NOT_SOLID );
	pObject->AddEffects( EF_NOSHADOW );
	UTIL_SetSize( pObject, vec3_origin, vec3_origin );

	pObject->AddEFlags( EFL_FORCE_CHECK_TRANSMIT );
	pObject->SetOwnerEntity( pOwner );
	pObject->SetName( AllocPooledString( "FF_MINITURRET_LASERBEAM" ) );

	return pObject;
#else
	return NULL;
#endif
}

#ifdef CLIENT_DLL
//-----------------------------------------------------------------------------
// Purpose: Setup our sprite reference
//-----------------------------------------------------------------------------
void CFFMiniTurretLaserBeam::OnDataChanged( DataUpdateType_t updateType ) 
{
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED ) 
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}	
}

//-----------------------------------------------------------------------------
// Purpose: Draw the beam
//-----------------------------------------------------------------------------
void CFFMiniTurretLaserBeam::ClientThink( void )
{
	// Update laser beam position

	if( !IsOn() )
		return;

	CFFMiniTurret *pOwner = dynamic_cast< CFFMiniTurret * >( GetOwnerEntity() );
	if( !pOwner )
	{
		AssertMsg( false, "Mini Turret laser has no owner!" );
		return;
	}

	Vector vecOrigin, vecForward;
	QAngle vecAngles;

	pOwner->LaserPosition( vecOrigin, vecAngles );	
	AngleVectors( vecAngles, &vecForward );

	VectorNormalizeFast( vecForward );

	if( !m_pBeam )
	{
		// Create laser beam
		m_pBeam = CBeam::BeamCreate( FF_MINITURRET_BEAM, 1.0f );
		if( !m_pBeam )
			Warning( "[Mini Turret Laser Beam] Failed to create laser beam\n" );
		else
		{
			m_pBeam->PointsInit( vecOrigin, vecOrigin + ( vecForward * 48.0f ) );
			m_pBeam->SetColor( 255, 0, 0 );
			m_pBeam->SetBrightness( 255 );
			m_pBeam->SetNoise( 0.0f );
			m_pBeam->SetEndWidth( 0.0f );
			m_pBeam->SetScrollRate( 0.0f );
		}
	}
	else
	{
		m_pBeam->SetFadeLength( 32.0f );
		m_pBeam->SetAbsStartPos( vecOrigin );
		m_pBeam->SetAbsEndPos( vecOrigin + ( vecForward * 48.0f ) );
		m_pBeam->RelinkBeam();
	}
}
#endif // CLIENT_DLL

//=============================================================================
//
// Class CFFMiniTurret tables
//
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED( FFMiniTurret, DT_FFMiniTurret ) 

BEGIN_NETWORK_TABLE( CFFMiniTurret, DT_FFMiniTurret )
#ifdef CLIENT_DLL 
	//RecvPropInt( RECVINFO( m_iTeam ) ),
	RecvPropInt( RECVINFO( m_bActive ) ),
	RecvPropInt( RECVINFO( m_bEnabled ) ),
#else
	//SendPropInt( SENDINFO( m_iTeam ) ),
	SendPropInt( SENDINFO( m_bActive ) ),
	SendPropInt( SENDINFO( m_bEnabled ) ),
#endif
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS( ff_miniturret, CFFMiniTurret );
PRECACHE_REGISTER( ff_miniturret );

#ifdef GAME_DLL

// Debug visualization
ConVar	miniturret_debug( "ffdev_miniturret_debug", "0" );
ConVar	miniturret_turnspeed( "ffdev_miniturret_turnspeed", "17.0" );
ConVar	miniturret_castrate( "ffdev_miniturret_castrate", "0" );

// Datatable
BEGIN_DATADESC( CFFMiniTurret )

	//DEFINE_KEYFIELD( m_iTeam, FIELD_INTEGER, "team" ),

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

// Mini Turret sounds to cache
const char *g_ppszMiniTurretSounds[ ] =
{
	"RespawnTurret.Deploy",
	"RespawnTurret.Retire",
	"RespawnTurret.Fire",
	"RespawnTurret.Ping",
	"RespawnTurret.Alert",
	//"RespawnTurret.Spin",
	NULL
};

#endif // GAME_DLL

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

	m_hLaserDot = NULL;
	m_hLaserBeam = NULL;
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
	if( m_hLaserDot != NULL )
	{
		UTIL_Remove( m_hLaserDot );
		m_hLaserDot = NULL;
	}

	if( m_hLaserBeam != NULL )
	{
		UTIL_Remove( m_hLaserBeam );
		m_hLaserBeam = NULL;
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Get the eye position (shared)
//-----------------------------------------------------------------------------
Vector CFFMiniTurret::EyePosition( void )
{
	// Return a position underneath the turret and not up inside
	// somewhere where when we're not deployed we'll never have
	// LOS on anything
	SetupAttachments();
	return GetAbsOrigin() - Vector( 0, 0, 16 );
}

//-----------------------------------------------------------------------------
// Purpose: Get the muzzle position (shared)
//-----------------------------------------------------------------------------
Vector CFFMiniTurret::MuzzlePosition( void )
{
	Vector vecOrigin;
	QAngle vecAngles;
	SetupAttachments();
	GetAttachment( m_iMuzzleAttachment, vecOrigin, vecAngles );

	return vecOrigin;
}
//-----------------------------------------------------------------------------
// Purpose: Get the muzzle position (shared)
//-----------------------------------------------------------------------------
void CFFMiniTurret::MuzzlePosition( Vector& vecOrigin, QAngle& vecAngles )
{
	SetupAttachments();
	GetAttachment( m_iMuzzleAttachment, vecOrigin, vecAngles );
}

//-----------------------------------------------------------------------------
// Purpose: Get the point where the laser eminates from (shared)
//-----------------------------------------------------------------------------
void CFFMiniTurret::LaserPosition( Vector& vecOrigin, QAngle& vecAngles )
{
	SetupAttachments();
	GetAttachment( m_iLaserAttachment, vecOrigin, vecAngles );
}

//-----------------------------------------------------------------------------
// Purpose: Setup attachments (shared)
//-----------------------------------------------------------------------------
void CFFMiniTurret::SetupAttachments( void )
{
	if( m_iMuzzleAttachment == -1 )
		m_iMuzzleAttachment = LookupAttachment( FF_MINITURRET_MUZZLE_ATTACHMENT );
	if( m_iEyeAttachment == -1 )
		m_iEyeAttachment = LookupAttachment( FF_MINITURRET_EYE_ATTACHMENT );
	if( m_iLaserAttachment == -1 )
		m_iLaserAttachment = LookupAttachment( FF_MINITURRET_EYE_ATTACHMENT );
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

	// Precache sounds
	int iCount = 0;
	while( g_ppszMiniTurretSounds[ iCount ] != NULL )
	{		
		PrecacheScriptSound( g_ppszMiniTurretSounds[ iCount ] );
		iCount++;
	}
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
	SetModel( FF_MINITURRET_MODEL );	

	BaseClass::Spawn();

	SetBlocksLOS( false );

	SetViewOffset( EyeOffset( ACT_IDLE ) );
	m_takedamage	= DAMAGE_EVENTS_ONLY;
	m_iHealth		= 100;
	m_iMaxHealth	= 100;

	AddFlag( FL_AIMTARGET );
	AddEFlags( EFL_NO_DISSOLVE );

	SetPoseParameter( FF_MINITURRET_BC_YAW, 0 );
	SetPoseParameter( FF_MINITURRET_BC_PITCH, 0 );

	m_iAmmoType = GetAmmoDef()->Index( AMMO_SHELLS );

	SetupAttachments();

	m_iPitchPoseParameter = LookupPoseParameter( FF_MINITURRET_BC_PITCH );
	m_iYawPoseParameter = LookupPoseParameter( FF_MINITURRET_BC_YAW );

	// Set our state
	m_bEnabled = true;
	m_bActive = false; // don't start deployed ever
	m_flPingTime = gpGlobals->curtime;

	SetThink( &CFFMiniTurret::OnAutoSearchThink );

	// Stagger our starting times
	SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.1f, 0.3f ) );

	// For whatever reason, it seems like the yaw has to be set to 180
	// or the yaw for the aiming will be off. The trepids might complain
	// that setting the angles in hammer has no effect, so we might
	// need to compensate for this later.
	SetAbsAngles( QAngle( 0, 180, 0 ) );

	m_vecGoalAngles.Init();

	//ChangeTeam( m_iTeam );
}

//-----------------------------------------------------------------------------
// Purpose: Change a turrets team
//-----------------------------------------------------------------------------
/*
void CFFMiniTurret::ChangeTeam( int iTeamNum )
{
	AssertMsg( ( iTeamNum >= TEAM_UNASSIGNED ) && ( iTeamNum <= TEAM_GREEN ), "Invalid ChangeTeam for MiniTurret\n" );

	Warning( "[MiniTurret] Changing team to: %i\n", iTeamNum );

	m_iTeam = iTeamNum;
	SetEnemy( NULL );
}
*/

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

	for( int i = 1; i < gpGlobals->maxClients; i++ )
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );

		if( !pPlayer || !pPlayer->IsPlayer() || pPlayer->IsObserver() )
			continue;

		if( !pPlayer->IsAlive() )
			continue;

		/*
		if( m_iTeam != 0 )
			if( FFGameRules()->IsTeam1AlliedToTeam2( m_iTeam, pPlayer->GetTeamNumber() ) == GR_TEAMMATE )
				continue;
				*/
		
		// Check if lua will let us target this player
		if( FVisible( pPlayer->GetLegacyAbsOrigin() ) || FVisible( pPlayer->GetAbsOrigin() ) || FVisible( pPlayer->EyePosition() ) )
		{
			//DevMsg( "[MiniTurret] %s About to run predicates for player: %s ", GetEntityName(), pPlayer->GetPlayerName() );
			if( entsys.RunPredicates_LUA( this, pPlayer, "validtarget" ) )
			{
				//DevMsg( "Success!\n" );
				pTarget = MiniTurret_IsBetterTarget( pTarget, pPlayer, ( pPlayer->GetAbsOrigin() - vecOrigin ).LengthSqr() );
			}
			else
			{
				//DevMsg( "Failure!\n" );
			}
			//DevMsg( "\n" );
		}

		// Check if lua will let us target this sentrygun
		if( pPlayer->m_hSentryGun.Get() )
		{
			CFFSentryGun *pSentryGun = static_cast< CFFSentryGun * >( pPlayer->m_hSentryGun.Get() );
			if( FVisible( pSentryGun->GetAbsOrigin() ) || FVisible( pSentryGun->EyePosition() ) )
			{
				if( entsys.RunPredicates_LUA( this, pSentryGun, "validtarget" ) )
					pTarget = MiniTurret_IsBetterTarget( pTarget, pSentryGun, ( pSentryGun->GetAbsOrigin() - vecOrigin ).LengthSqr() );
			}
		}

		// Check if lua will let us target this dispenser
		if( pPlayer->m_hDispenser.Get() )
		{
			CFFDispenser *pDispenser = static_cast< CFFDispenser * >( pPlayer->m_hDispenser.Get() );
			if( FVisible( pDispenser->GetAbsOrigin() ) || FVisible( pDispenser->EyePosition() ) )
			{
				if( entsys.RunPredicates_LUA( this, pDispenser, "validtarget" ) )
					pTarget = MiniTurret_IsBetterTarget( pTarget, pDispenser, ( pDispenser->GetAbsOrigin() - vecOrigin ).LengthSqr() );
			}
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

		EmitSound( "RespawnTurret.Deploy" );

		EnableLaserDot();
		EnableLaserBeam();
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
		//EmitSound( "RespawnTurret.Alert" );
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
	m_vecGoalAngles.y = AngleNormalize( m_vecGoalAngles.y + MaxYawSpeed() );

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

	// Check lua here too to make sure this guy is still a valid target
	// He might have done something to make him not valid because of
	// the awesomeness of lua
	if( !m_bActive || !GetEnemy() || !entsys.RunPredicates_LUA( this, GetEnemy(), "validtarget" ) )
	{
		SetEnemy( NULL );
		m_flLastSight = gpGlobals->curtime + FF_MINITURRET_MAX_WAIT;
		SetThink( &CFFMiniTurret::OnSearchThink );
		//m_vecGoalAngles = GetAbsAngles();
		return;
	}

	bool bEnemyVisible = false;
	Vector vecMuzzle = MuzzlePosition();	
	Vector vecMidEnemy = BodyTarget( vecMuzzle );
	
	if( GetEnemy()->IsPlayer() )
	{
		// Enemy is a player
		CFFPlayer *pPlayer = ToFFPlayer( GetEnemy() );
		bEnemyVisible = GetEnemy()->IsAlive() && ( FVisible( pPlayer->GetAbsOrigin() ) || FVisible( pPlayer->GetLegacyAbsOrigin() ) || FVisible( pPlayer->EyePosition() ) );		
	}
	else
	{
		// Enemy is a buildable
		bEnemyVisible = FVisible( GetEnemy()->GetAbsOrigin() ) || FVisible( GetEnemy()->EyePosition() );
	}

	Vector vecDirToEnemy = vecMidEnemy - vecMuzzle;

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
		if( DotProduct( vecDirToEnemy, vecMuzzleDir ) <= DOT_3DEGREE )
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

			EmitSound( "RespawnTurret.Retire" );
		}
	}
	else if( IsActivityFinished() )
	{
		m_bActive = false;
		m_flLastSight = 0;

		SetActivity( ( Activity )ACT_MINITURRET_CLOSED_IDLE );

		SetThink( &CFFMiniTurret::OnAutoSearchThink );
		SetNextThink( gpGlobals->curtime + 0.05f );

		DisableLaserDot();
		DisableLaserBeam();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CFFMiniTurret::UpdateFacing( void )
{
	bool bMoved = false;

	// Get current orientation of the muzzle attachment
	Vector vecOrigin;
	QAngle vecAngles;
	MuzzlePosition( vecOrigin, vecAngles );

	// Update pitch
	float flDiff = -m_vecGoalAngles.x;
	SetPoseParameter( m_iPitchPoseParameter, flDiff );

	if( miniturret_debug.GetBool() )
		DevMsg( "[MiniTurret] Current Pitch: %f, Goal Pitch: %f, pitch val: %f\n", vecAngles.x, m_vecGoalAngles.x, flDiff );

	// Get the current yaw value
	float flYaw = GetPoseParameter( m_iYawPoseParameter );

	if( miniturret_debug.GetBool() )
		Warning( "[MiniTurret] Current pose yaw: %f, vecAngles yaw: %f, goal yaw: %f\n", flYaw, vecAngles.y, m_vecGoalAngles.y );

	// Update yaw
	flDiff = AngleNormalize( UTIL_ApproachAngle( AngleNormalize( m_vecGoalAngles.y ), AngleNormalize( flYaw ), MaxYawSpeed() ) );
	SetPoseParameter( m_iYawPoseParameter, flDiff );

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
// Purpose: Creates the laser dot
//-----------------------------------------------------------------------------
void CFFMiniTurret::EnableLaserDot( void )
{
	if( m_hLaserDot )
		return;

	Vector vecOrigin;
	QAngle vecAngles;

	LaserPosition( vecOrigin, vecAngles );

	m_hLaserDot = CFFMiniTurretLaserDot::Create( vecOrigin, this );

	AssertMsg( m_hLaserDot, "Failed to create Mini Turret laser dot!" );

	m_hLaserDot->TurnOn();
}

//-----------------------------------------------------------------------------
// Purpose: Destroys the laser dot
//-----------------------------------------------------------------------------
void CFFMiniTurret::DisableLaserDot( void )
{
	if( m_hLaserDot )
	{
		m_hLaserDot->TurnOff();
		UTIL_Remove( m_hLaserDot );
		m_hLaserDot = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates the laser beam
//-----------------------------------------------------------------------------
void CFFMiniTurret::EnableLaserBeam( void )
{
	if( m_hLaserBeam )
		return;

	Vector vecOrigin;
	QAngle vecAngles;

	LaserPosition( vecOrigin, vecAngles );

	m_hLaserBeam = CFFMiniTurretLaserBeam::Create( vecOrigin, this );

	AssertMsg( m_hLaserBeam, "Failed to create Mini Turret laser beam!" );

	m_hLaserBeam->TurnOn();
}

//-----------------------------------------------------------------------------
// Purpose: Destroys the laser beam
//-----------------------------------------------------------------------------
void CFFMiniTurret::DisableLaserBeam( void )
{
	if( m_hLaserBeam )
	{
		m_hLaserBeam->TurnOff();
		UTIL_Remove( m_hLaserBeam );
		m_hLaserBeam = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMiniTurret::Ping( void )
{
	if( m_flPingTime > gpGlobals->curtime )
		return;

	EmitSound( "RespawnTurret.Ping" );

	m_flPingTime = gpGlobals->curtime + FF_MINITURRET_PING_TIME;
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

	//Vector vecDir = vecDirToEnemy;

	/*
	Vector vecDir, vecOrigin;
	
	if( GetEnemy()->IsPlayer() )
		vecOrigin = ToFFPlayer( GetEnemy() )->GetLegacyAbsOrigin();
	else
		vecOrigin = GetEnemy()->GetAbsOrigin() + Vector( 0, 0, 48.0f );

	vecDir = vecOrigin - MuzzlePosition();
	*/

	info.m_vecSrc = vecSrc;
	info.m_vecDirShooting = vecDirToEnemy;
	info.m_iTracerFreq = 1;
	info.m_iShots = 1;
	info.m_pAttacker = this;
	info.m_vecSpread = VECTOR_CONE_PRECALCULATED;
	info.m_flDistance = MAX_COORD_RANGE;
	info.m_iAmmoType = m_iAmmoType;
	info.m_iDamage = 70.0f;

	if( !miniturret_castrate.GetBool() )
	{
		FireBullets( info );
		EmitSound( "RespawnTurret.Fire" );
		DoMuzzleFlash();
	}
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
