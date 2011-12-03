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
	#include "ff_scriptman.h"
	#include "ff_luacontext.h"
	#include "ff_utils.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static int g_iMiniTurretBeam, g_iMiniTurretHalo, g_iMiniTurretDot;

//=============================================================================
//
// Class CFFMiniTurretLaser
//
//=============================================================================

//ConVar	turret_usepvs( "ffdev_spawnturret_usepvs", "0", FCVAR_FF_FFDEV_REPLICATED );
#define TURRET_USEPVS false // turret_usepvs.GetBool()

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
// Purpose: Destructor
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
	RecvPropBool( RECVINFO( m_bActive ) ),
	RecvPropBool( RECVINFO( m_bEnabled ) ),
#else
	SendPropBool( SENDINFO( m_bActive ) ),
	SendPropBool( SENDINFO( m_bEnabled ) ),
#endif
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS( ff_miniturret, CFFMiniTurret );
PRECACHE_REGISTER( ff_miniturret );

#ifdef GAME_DLL

// Debug visualization
//ConVar	miniturret_debug( "ffdev_miniturret_debug", "0", FCVAR_FF_FFDEV );
#define MINITURRET_DEBUG false
//ConVar	miniturret_turnspeed( "ffdev_miniturret_turnspeed", "17.0", FCVAR_FF_FFDEV );
#define MINITURRET_TURNSPEED 17.0f
//ConVar	miniturret_castrate( "ffdev_miniturret_castrate", "0", FCVAR_FF_FFDEV );
#define MINITURRET_CASTRATE false

// Datatable
BEGIN_DATADESC( CFFMiniTurret )

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
// Purpose: Destructor
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

	//SetViewOffset( EyeOffset( ACT_IDLE ) );
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

	SetActivity( ( Activity )ACT_MINITURRET_CLOSED_IDLE );

	//m_Activity = m_IdealActivity;
	//m_nIdealSequence = GetSequence();

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
CBaseEntity *CFFMiniTurret::HackFindEnemy( void )
{
	// Our location - used later
	Vector vecOrigin = GetAbsOrigin();
	CBaseEntity *pTarget = NULL;

	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );

		if( !pPlayer || !pPlayer->IsPlayer() || pPlayer->IsObserver() )
			continue;
		
		// Check if lua will let us target this sentrygun
		if( pPlayer->GetSentryGun() )
		{
			CFFSentryGun *pSentryGun = pPlayer->GetSentryGun();
			//CFFLuaObjectWrapper hValidTarget;
			CFFLuaSC hContext( 1, pSentryGun );
			if( _scriptman.RunPredicates_LUA( this, &hContext, "validtarget" ) )
			{
				if( hContext.GetBool() )
					if( IsTargetVisible(pSentryGun) )
						pTarget = MiniTurret_IsBetterTarget( pTarget, pSentryGun, ( pSentryGun->GetAbsOrigin() - vecOrigin ).LengthSqr() );
			}
		}

		// Check if lua will let us target this dispenser
		if( pPlayer->GetDispenser() )
		{
			CFFDispenser *pDispenser = pPlayer->GetDispenser();
			
			CFFLuaSC hContext( 1, pDispenser );
			if( _scriptman.RunPredicates_LUA( this, &hContext, "validtarget" ) )
			{
				if( hContext.GetBool() )
					if( IsTargetVisible(pDispenser) )
						pTarget = MiniTurret_IsBetterTarget( pTarget, pDispenser, ( pDispenser->GetAbsOrigin() - vecOrigin ).LengthSqr() );
			}
		}
		

		// Check if lua will let us target this jumppad
		if( pPlayer->GetManCannon() )
		{
			CFFManCannon *pManCannon = pPlayer->GetManCannon();
			
			CFFLuaSC hContext( 1, pManCannon );
			if( _scriptman.RunPredicates_LUA( this, &hContext, "validtarget" ) )
			{
				if( hContext.GetBool() )
					if( IsTargetVisible(pManCannon) )
						pTarget = MiniTurret_IsBetterTarget( pTarget, pManCannon, ( pManCannon->GetAbsOrigin() - vecOrigin ).LengthSqr() );
			}
		}

		if( !pPlayer->IsAlive() )
			continue;

		/*
		if( m_iTeam != 0 )
			if( FFGameRules()->IsTeam1AlliedToTeam2( m_iTeam, pPlayer->GetTeamNumber() ) == GR_TEAMMATE )
				continue;
				*/
		
		// Check if lua will let us target this player
		CFFLuaSC hContext( 1, pPlayer );
		if( _scriptman.RunPredicates_LUA( this, &hContext, "validtarget" ) )
		{
			if( hContext.GetBool() )
				if( IsTargetVisible( pPlayer ) )
					pTarget = MiniTurret_IsBetterTarget( pTarget, pPlayer, ( pPlayer->GetAbsOrigin() - vecOrigin ).LengthSqr() );
		}
	}

	return pTarget;
}

//-----------------------------------------------------------------------------
// Purpose: Set our enemy
//-----------------------------------------------------------------------------
void CFFMiniTurret::SetEnemy( CBaseEntity *pEntity )
{
	m_hEnemy = pEntity;
}

//-----------------------------------------------------------------------------
// Purpose: Get our enemy
//-----------------------------------------------------------------------------
CBaseEntity *CFFMiniTurret::GetEnemy( void )
{
	if( m_hEnemy )
		return ( CBaseEntity * )m_hEnemy;

	return NULL;
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
	if( MINITURRET_DEBUG )
		DevMsg( "[MiniTurret] OnAutoSearchThink\n" );

	OnObjectThink();

	SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.1f, 0.3f ) );

	if( GetEnemy() && !GetEnemy()->IsAlive() )
		SetEnemy( NULL );

	if(!GetEnemy())
		SetEnemy(HackFindEnemy());

	if( GetEnemy() )
	{
		// I see you new guy!
		Vector vecSoundOrigin = EyePosition();
		CPASAttenuationFilter filter( vecSoundOrigin );
		EmitSound( filter, entindex(), "RespawnTurret.Alert", &vecSoundOrigin );

		// Get a delay value from LUA
		CFFLuaSC hContext( 1, GetEnemy() );
		if( _scriptman.RunPredicates_LUA( this, &hContext, "deploydelay" ) )
		{
			// Lua function existed, grab the delay (hopefully)

			// Delay deploying!
			SetNextThink( gpGlobals->curtime + hContext.GetFloat() );
		}
		
		SetThink( &CFFMiniTurret::OnDeploy );		
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMiniTurret::OnDeploy( void )
{
	if( MINITURRET_DEBUG )
		DevMsg( "[MiniTurret] OnDeploy\n" );

	OnObjectThink();

	SetNextThink( gpGlobals->curtime );

	if( GetActivity() != ACT_MINITURRET_OPEN )
	{
		m_bActive = true;
		SetActivity( ( Activity )ACT_MINITURRET_OPEN );

		m_OnDeploy.FireOutput( NULL, this );

		Vector vecSoundOrigin = EyePosition();
		CPASAttenuationFilter filter( vecSoundOrigin );
		EmitSound( filter, entindex(), "RespawnTurret.Deploy", &vecSoundOrigin );

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
	if( MINITURRET_DEBUG )
		DevMsg( "[MiniTurret] OnSearchThink\n" );

	OnObjectThink();

	SetNextThink( gpGlobals->curtime + 0.05f );

	SetActivity( ( Activity )ACT_MINITURRET_OPEN_IDLE );

	if( GetEnemy() && !GetEnemy()->IsAlive() )
		SetEnemy( NULL );

	if( !GetEnemy() )
		SetEnemy(HackFindEnemy());

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
	m_vecGoalAngles.y = AngleNormalize( m_vecGoalAngles.y + MaxYawSpeed() );

	UpdateFacing();
	Ping();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMiniTurret::OnActiveThink( void )
{
	if( MINITURRET_DEBUG )
		DevMsg( "[MiniTurret] OnActiveThink\n" );

	OnObjectThink();

	SetNextThink( gpGlobals->curtime + 0.1f );

	// Check lua here too to make sure this guy is still a valid target
	// He might have done something to make him not valid because of
	// the awesomeness of lua
	bool bValidTarget = true;

	CFFLuaSC hContext( 1, GetEnemy() );
	if( _scriptman.RunPredicates_LUA( this, &hContext, "validtarget" ) )
		bValidTarget = hContext.GetBool();

	if( !m_bActive || !GetEnemy() || !bValidTarget )
	{
		SetEnemy( NULL );
		m_flLastSight = gpGlobals->curtime + FF_MINITURRET_MAX_WAIT;
		SetThink( &CFFMiniTurret::OnSearchThink );

		return;
	}

	bool bEnemyVisible = false;

	// Get aiming direction and angles
	Vector vecMuzzle, vecMuzzleDir;
	QAngle vecMuzzleAng;

	MuzzlePosition( vecMuzzle, vecMuzzleAng );
	AngleVectors( vecMuzzleAng, &vecMuzzleDir );
	
	Vector vecMidEnemy = GetEnemy()->BodyTarget( vecMuzzle, false );
	
	bEnemyVisible = IsTargetVisible( GetEnemy() );
	/*
	if( GetEnemy()->IsPlayer() )
	{
		// Enemy is a player
		CFFPlayer *pPlayer = ToFFPlayer( GetEnemy() );
		bEnemyVisible = GetEnemy()->IsAlive() && ( FVisible( pPlayer->GetAbsOrigin() ) || FVisible( pPlayer->GetLegacyAbsOrigin() ) || FVisible( pPlayer->EyePosition() ) );		
	}
	else
	{
		// Enemy is something else
		bEnemyVisible = FVisible( GetEnemy()->GetAbsOrigin() ) || FVisible( GetEnemy()->EyePosition() );
	}
	*/

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
			
			SpinDown();
			return;
		}
	}

	Vector vecDirToEnemy = vecMidEnemy - vecMuzzle;
	VectorNormalizeFast( vecDirToEnemy );	

	if( MINITURRET_DEBUG )
	{
		NDebugOverlay::Line( vecMuzzle, vecMuzzle + ( vecDirToEnemy * 256 ), 255, 0, 0, false, 0.25f );
		NDebugOverlay::Line( vecMuzzle, vecMuzzle + ( vecMuzzleDir * 256 ), 0, 0, 255, false, 0.25f );
	}

	if( m_flShotTime < gpGlobals->curtime )
	{
		// Fire the gun
		if( DotProduct( vecDirToEnemy, vecMuzzleDir ) > DOT_10DEGREE )
		{
			//SetActivity( ACT_RESET );
			//SetActivity( ( Activity )ACT_MINITURRET_FIRE );

			// Fire the weapon
			Shoot( vecMuzzle, vecDirToEnemy );
		} 
	}

	// If we can see our enemy, face it
	if( bEnemyVisible )
	{
		QAngle vecAnglesToEnemy;
		VectorAngles( vecDirToEnemy, vecAnglesToEnemy );

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
	if( MINITURRET_DEBUG )
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

			Vector vecSoundOrigin = EyePosition();
			CPASAttenuationFilter filter( vecSoundOrigin );
			EmitSound( filter, entindex(), "RespawnTurret.Retire", &vecSoundOrigin );
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
// Purpose: See if a target is visible
//-----------------------------------------------------------------------------
bool CFFMiniTurret::IsTargetVisible( CBaseEntity *pTarget )
{
	if( !pTarget )
		return false;

	CFFPlayer *pFFPlayer = ToFFPlayer( pTarget );

	// early out if player's not even alive
	if ( pFFPlayer && !pFFPlayer->IsAlive() )
		return false;

	// Get our aiming position
	Vector vecMuzzle, vecMuzzleDir;
	QAngle vecMuzzleAng;

	MuzzlePosition( vecMuzzle, vecMuzzleAng );
	AngleVectors( vecMuzzleAng, &vecMuzzleDir );

	// Get a position on the target
	Vector vecTarget = pTarget->BodyTarget( vecMuzzle, false );

	/*float flDistToTarget = vecMuzzle.DistTo( vecTarget );

	// Check for out of range
	if( flDistToTarget > SG_RANGE )
		return false;*/

	// Check PVS for early out
	if(TURRET_USEPVS)
	{
		byte pvs[ MAX_MAP_CLUSTERS/8 ];
		int iPVSCluster = engine->GetClusterForOrigin(vecMuzzle);
		int iPVSLength = engine->GetPVSForCluster(iPVSCluster, sizeof(pvs), pvs);
		if(!engine->CheckOriginInPVS(vecTarget, pvs, iPVSLength))
			return false;
	}

	// Can we trace to the target?
	trace_t tr;
	// Using MASK_SHOT instead of MASK_PLAYERSOLID so turrets track through anything they can actually shoot through
	UTIL_TraceLine( EyePosition(), vecTarget, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	//UTIL_TraceLine( vecOrigin, vecTarget, MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr );

	/*if ( TURRET_DEBUG )
	{
		int r = 0, g = 0, b = 0;
		if(tr.fraction < 1.f)
			r = 255;
		else
			g = 255;
		debugoverlay->AddLineOverlay(vecOrigin, vecTarget, r, g, b, false, 0.1f);
	}*/

	// What did our trace hit?
	// Miniturrets honestly don't care about startsolid, they actually do start in a solid
	if( /* tr.startsolid || ( tr.fraction != 1.0f ) ||*/ !tr.m_pEnt || FF_TraceHitWorld( &tr ) )
		return false;

	if(tr.m_pEnt != pTarget)
		return false;

	return true;
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

	if( MINITURRET_DEBUG )
		DevMsg( "[MiniTurret] Current Pitch: %f, Goal Pitch: %f, pitch val: %f\n", vecAngles.x, m_vecGoalAngles.x, flDiff );

	// Get the current yaw value
	float flYaw = GetPoseParameter( m_iYawPoseParameter );

	if( MINITURRET_DEBUG )
		Warning( "[MiniTurret] Current pose yaw: %f, vecAngles yaw: %f, goal yaw: %f\n", flYaw, vecAngles.y, m_vecGoalAngles.y );

	// Update yaw
	flDiff = AngleNormalize( UTIL_ApproachAngle( AngleNormalize( m_vecGoalAngles.y + (flYaw - vecAngles.y) ), AngleNormalize( flYaw ), MaxYawSpeed() ) );
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

	Vector vecSoundOrigin = EyePosition();
	CPASAttenuationFilter filter( vecSoundOrigin );
	EmitSound( filter, entindex(), "RespawnTurret.Ping", &vecSoundOrigin );

	m_flPingTime = gpGlobals->curtime + FF_MINITURRET_PING_TIME;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float CFFMiniTurret::MaxYawSpeed( void )
{
	if( GetEnemy() )
		return MINITURRET_TURNSPEED;

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
	if( MINITURRET_DEBUG )
		info.m_iDamage = 0.0f;
	else
		info.m_iDamage = 70.0f;

	if( !MINITURRET_CASTRATE )
	{
		FireBullets( info );

		Vector vecSoundOrigin = EyePosition();
		CPASAttenuationFilter filter( vecSoundOrigin );
		EmitSound( filter, entindex(), "RespawnTurret.Fire", &vecSoundOrigin );

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

//-----------------------------------------------------------------------------
// Purpose: Sets the activity to the desired activity immediately, skipping any
//			transition sequences.
// Input  : NewActivity - 
//-----------------------------------------------------------------------------
void CFFMiniTurret::SetActivity( Activity NewActivity )
{
	if( m_Activity == NewActivity )
		return;

	// Don't do this if I'm playing a transition, unless it's ACT_RESET.
	if( ( NewActivity != ACT_RESET ) && ( m_Activity == ACT_TRANSITION ) && ( m_IdealActivity != ACT_DO_NOT_DISTURB ) )
		return;

	if( !GetModelPtr() )
		return;

	// In case someone calls this with something other than the ideal activity.
	m_IdealActivity = NewActivity;

	m_nIdealSequence = SelectWeightedSequence( m_IdealActivity );

	if( m_nIdealSequence == ACT_INVALID )
		m_nIdealSequence = 0;

	// Set to the desired anim, or default anim if the desired is not present
	if( m_nIdealSequence > ACTIVITY_NOT_AVAILABLE )
	{
		if( ( GetSequence() != m_nIdealSequence ) || !SequenceLoops() )
			SetCycle( 0 );

		ResetSequence( m_nIdealSequence );
	}
	else
	{
		// Not available try to get default anim
		ResetSequence( 0 );
	}

	// Go ahead and set this so it doesn't keep trying when the anim is not present
	m_Activity = m_IdealActivity;
}

//-----------------------------------------------------------------------------
// Purpose: Returns true if our ideal activity has finished playing.
//-----------------------------------------------------------------------------
bool CFFMiniTurret::IsActivityFinished( void )
{
	return ( IsSequenceFinished() && ( GetSequence() == m_nIdealSequence ) );
}

#endif // GAME_DLL
