// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_sentrygun.cpp
// @author Patrick O'Leary(Mulchman) 
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
//		Noticed I had dates wrong... *cough * and
//		still working on making the SG animated
//		and such.
//
//	06/08/2005, Mulchman:
//		Decided the SG needs to inherit from the
//		AI base class and not the buildable class.
//		Some easy stuff will give it the same base
//		buildable attributes while inheriting all
//		of the AI stuff that it so desperately needs!
//
// 22/01/2006, Mirv:
//		Sentry now has ground pos & angles pre-stored for when it goes live
//
// 26/01/2006 Mirv:
//		A lot of this has been rewritten so support some future stuff. I've not yet
//		finished all of it but its in a state where it can be committed now.
//
//	05/10/2006, Mulchman:
//		Cleaned this up A LOT. SG still doesn't factor in radiotagged targets, though.

#include "cbase.h"
#include "ammodef.h"
#include "ff_sentrygun.h"
#include "te_effect_dispatch.h" 
#include "ff_projectile_rocket.h"
#include "ff_utils.h"
#include "ff_gamerules.h"

#include "omnibot_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
//	class CFFSentryGun
//
//=============================================================================

// Quick conversions from angles to pitchparamter style
#define TO_PITCH(x) ((x) > 180 ? (360 - (x)) : ((x) * -1)) 
#define FROM_PITCH(x) ((x) > 0 ? (360 - (x)) : ((x) * -1)) 

#define TO_YAW(x) ((x) < -180 ? ((x) + 360) : ((x) > 180) ? ((x) - 360) : (x)) 

// Debug visualization
ConVar	sg_debug( "ffdev_sg_debug", "1" );
ConVar	sg_turnspeed( "ffdev_sg_turnspeed", "16.0" );
ConVar	sg_pitchspeed( "ffdev_sg_pitchspeed", "10.0" );
ConVar  sg_range( "ffdev_sg_range", "1152.0" );
ConVar	sg_attachments( "ffdev_sg_attachments", "0" );

IMPLEMENT_SERVERCLASS_ST(CFFSentryGun, DT_FFSentryGun) 
	SendPropInt( SENDINFO( m_iAmmoPercent), 8, SPROP_UNSIGNED ), 
	SendPropFloat( SENDINFO( m_flRange ) ), 
	SendPropInt( SENDINFO( m_iLevel ) ), 
	SendPropInt( SENDINFO( m_iShells ) ),
	SendPropInt( SENDINFO( m_iRockets ) ),
END_SEND_TABLE() 

LINK_ENTITY_TO_CLASS( FF_SentryGun, CFFSentryGun );
PRECACHE_REGISTER( FF_SentryGun );

// Datatable
BEGIN_DATADESC(CFFSentryGun) 
	DEFINE_THINKFUNC( OnActiveThink ), 
	DEFINE_THINKFUNC( OnSearchThink ), 
END_DATADESC() 

// Array of char *'s to sentrygun models
const char *g_pszFFSentryGunModels[] =
{
	FF_SENTRYGUN_MODEL, 
	FF_SENTRYGUN_MODEL_LVL2, 
	FF_SENTRYGUN_MODEL_LVL3, 
	NULL
};

// Array of char *'s to gib models
const char *g_pszFFSentryGunGibModels[] =
{
	NULL
};

// Array of char *'s to sounds
const char *g_pszFFSentryGunSounds[] =
{
	FF_SENTRYGUN_BUILD_SOUND, 
	FF_SENTRYGUN_EXPLODE_SOUND, 
	"Sentry.Fire", 
	"Sentry.Spot", 
	"Sentry.Scan", 
	"Sentry.Two", 
	"Sentry.Three", 
	"Sentry.Aim",
	FF_SENTRYGUN_UNBUILD_SOUND,
	"Spanner.HitSG",
	NULL
};

/*
static Vector g_ffdev_sg_mins = FF_SENTRYGUN_MINS;
static Vector g_ffdev_sg_maxs = FF_SENTRYGUN_MAXS;

CON_COMMAND( ffdev_setsgsize, "set the sg's size" )
{
	g_ffdev_sg_mins.x = -atof( engine->Cmd_Argv( 1 ) );
	g_ffdev_sg_mins.y = -atof( engine->Cmd_Argv( 2 ) );
	g_ffdev_sg_mins.z = -atof( engine->Cmd_Argv( 3 ) );

	g_ffdev_sg_maxs.x = atof( engine->Cmd_Argv( 4 ) );
	g_ffdev_sg_maxs.y = atof( engine->Cmd_Argv( 5 ) );
	g_ffdev_sg_maxs.z = atof( engine->Cmd_Argv( 6 ) );

	Warning( "[Mins/Maxs] %f, %f, %f, - %f, %f, %f\n", g_ffdev_sg_mins.x, g_ffdev_sg_mins.y, g_ffdev_sg_mins.z, g_ffdev_sg_maxs.x, g_ffdev_sg_maxs.y, g_ffdev_sg_maxs.z );
}
*/

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CFFSentryGun::CFFSentryGun() 
{
	m_ppszModels = g_pszFFSentryGunModels;
	m_ppszGibModels = g_pszFFSentryGunGibModels;
	m_ppszSounds = g_pszFFSentryGunSounds;

	// Set lvl1 range
	m_flRange = 1024.0f;

	// Set level - keep it < 0 until we GoLive the first time
	m_iLevel = 0;
	m_flThinkTime = 0.1f;

	m_flPingTime = 0;
	m_flNextActivateSoundTime = 0;
	m_flNextShell = 0;
	m_flNextRocket = 0;
	m_flLastSight = 0;
	m_iMaxShells = 200; // TODO: Get Number
	m_iMaxRockets = 0;
	m_iRockets = 0;
	m_iShellDamage = 8;
	m_bLeftBarrel = true;
	m_bRocketLeftBarrel = true;

	m_angGoal.Init();

	m_flSabotageTime = 0;
	m_hSaboteur = NULL;
	m_bShootingTeammates = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFFSentryGun::~CFFSentryGun( void ) 
{
}

//-----------------------------------------------------------------------------
// Purpose: Precache
//-----------------------------------------------------------------------------
void CFFSentryGun::Precache( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::Precache", VPROF_BUDGETGROUP_FF_BUILDABLE );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the entity
//-----------------------------------------------------------------------------
void CFFSentryGun::Spawn( void ) 
{ 
	VPROF_BUDGET( "CFFSentryGun::Spawn", VPROF_BUDGETGROUP_FF_BUILDABLE );

	Precache();

	BaseClass::Spawn();

	// set skin
	CFFPlayer *pOwner = static_cast< CFFPlayer * >( m_hOwner.Get() );
	if( pOwner ) 
		m_nSkin = clamp( pOwner->GetTeamNumber() - TEAM_BLUE, 0, 3 );	// |-- Mirv: BUG #0000118: SGs are always red	
	// This is dumb as spawn gets called twice (once as the "constructor" and
	// once when the game code actually runs it) so it fails once and gives 
	// this misleading msg in the console
	//else
	//	Warning( "Unable to find sg owner!\n" ); 

	m_HackedGunPos	= Vector(0, 0, 12.75);
	SetViewOffset(EyeOffset(ACT_IDLE));
	m_flFieldOfView	= 0.4f; // 60 degrees

	AddFlag( FL_AIMTARGET );
	AddEFlags( EFL_NO_DISSOLVE );

	SetPoseParameter( SG_BC_YAW, 0 );
	SetPoseParameter( SG_BC_PITCH, 0) ;

	// Change this to neuter the sg
	m_iAmmoType = GetAmmoDef()->Index( "AMMO_SHELLS" );

	m_iMuzzleAttachment = LookupAttachment( "barrel01" );
	m_iEyeAttachment = LookupAttachment( "eyes" );	

	m_iPitchPoseParameter = LookupPoseParameter( SG_BC_PITCH );
	m_iYawPoseParameter = LookupPoseParameter( SG_BC_YAW );
	
	// Set initial direction
	Vector vecBaseForward, vecBaseRight, vecBaseUp;
	GetVectors( &vecBaseForward, &vecBaseRight, &vecBaseUp) ;

	// Set angles so it begins pointing in its starting position
	VectorAngles( vecBaseForward, m_angAiming );
	m_angAimBase = m_angAiming;
	m_angGoal.y = m_angAimBase.y - SG_SCAN_HALFWIDTH;
}

//-----------------------------------------------------------------------------
// Purpose: Make the object ready to react to the world
//-----------------------------------------------------------------------------
void CFFSentryGun::GoLive( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::GoLive", VPROF_BUDGETGROUP_FF_BUILDABLE );	

	BaseClass::GoLive();

	// Upgrade to level 1
	Upgrade( true );

	// Object is now built
	m_bBuilt = true;

	// Now use our stored ground location + orientation
	SetAbsOrigin( m_vecGroundOrigin );
	SetAbsAngles( m_angGroundAngles );

	// start thinking
	SetThink( &CFFSentryGun::OnSearchThink );

	// Stagger our starting times
	SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.1f, 0.3f ) );

	CFFPlayer *pOwner = static_cast< CFFPlayer * >( m_hOwner.Get() );
	
	// Bug #0000244: Building L1 sg doesn't take away cells
	if( pOwner ) 
		pOwner->RemoveAmmo( 130, AMMO_CELLS );

	// Create our flickerer
	m_pFlickerer = ( CFFBuildableFlickerer * )CreateEntityByName( "ff_buildable_flickerer" );
	if( !m_pFlickerer )
	{
		Warning( "[Sentrygun] Failed to create flickerer!\n" );
		m_pFlickerer = NULL;
	}
	else
	{		
		m_pFlickerer->SetBuildable( this );
		m_pFlickerer->Spawn();
	}

	m_flSabotageTime = 0;
	m_hSaboteur = NULL;
	m_bShootingTeammates = false;
}

//-----------------------------------------------------------------------------
// Purpose: Generic think function
//-----------------------------------------------------------------------------
void CFFSentryGun::OnObjectThink( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::OnObjectThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	/*
	bool bValid = true;
	for ( int i=0 ; i<3 ; i++ )
	{
		if (  g_ffdev_sg_mins[i] >  g_ffdev_sg_maxs[i] )
		{
			bValid = false;			
		}
	}

	if( bValid )
		UTIL_SetSize( this, g_ffdev_sg_mins, g_ffdev_sg_maxs );
		*/

	// For FC
	if( sg_attachments.GetBool() && !engine->IsDedicatedServer() && ( GetLevel() == 3 ) )
	{
		// Barrels
		{
			for( int i = 0; i < 2; i++ )
			{
				Vector vecOrigin;
				QAngle vecAngles;

				if( i == 0 )
					GetAttachment( m_iLBarrelAttachment, vecOrigin, vecAngles );
				else
					GetAttachment( m_iRBarrelAttachment, vecOrigin, vecAngles );

				Vector vecForward;
				AngleVectors( vecAngles, &vecForward );

				NDebugOverlay::Line( vecOrigin, vecOrigin + ( vecForward * 256.0f ), 0, 0, 255, false, 5.0f );
			}		
		}

		// Rockets
		{
			for( int i = 0; i < 2; i++ )
			{
				Vector vecOrigin;
				QAngle vecAngles;

				if( i == 0 )
					GetAttachment( m_iRocketLAttachment, vecOrigin, vecAngles );
				else
					GetAttachment( m_iRocketRAttachment, vecOrigin, vecAngles );

				Vector vecForward;
				AngleVectors( vecAngles, &vecForward );

				NDebugOverlay::Line( vecOrigin, vecOrigin + ( vecForward * 256.0f ), 255, 0, 0, false, 5.0f );
			}			
		}

		// Try rockets w/ bones
		{
			int iBones[ 2 ];

			//iBones[ 0 ] = LookupBone( "bone_sgRocket1" );
			//iBones[ 1 ] = LookupBone( "bone_sgRocket2" );

			for( int i = 0; i < 2; i++ )
			{
				Vector vecOrigin;
				QAngle vecAngles;

				//GetBonePosition( iBones[ i ], vecOrigin, vecAngles );
				if( i == 0 )
					GetBonePosition( 8, vecOrigin, vecAngles );
				else
					GetBonePosition( 18, vecOrigin, vecAngles );

				Vector vecForward;
				AngleVectors( vecAngles, &vecForward );

				NDebugOverlay::Line( vecOrigin, vecOrigin + ( vecForward * 256.0f ), 255, 255, 255, false, 5.0f );
			}				
		}
	}

	CheckForOwner();

	// Animate
	StudioFrameAdvance();

	// Run base class thinking
	CFFBuildableObject::OnObjectThink();
}

//-----------------------------------------------------------------------------
// Purpose: Target doesn't exist or has eluded us, so search for one
//-----------------------------------------------------------------------------
void CFFSentryGun::OnSearchThink( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::OnOSearchThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	OnObjectThink();

	SetNextThink( gpGlobals->curtime + 0.05f );

	if( GetEnemy() && !GetEnemy()->IsAlive() ) 
		SetEnemy( NULL );

	if( !GetEnemy() ) 
		HackFindEnemy();

	if( GetEnemy() )
	{
		// Pause for the lock-on time
		m_flNextShell = m_flNextRocket = gpGlobals->curtime + m_flLockTime;

		m_flLastSight = 0;
		SetThink( &CFFSentryGun::OnActiveThink );

		SpinUp();

		if( gpGlobals->curtime > m_flNextActivateSoundTime ) 
		{
			//EmitSound("NPC_FloorTurret.Activate");
			m_flNextActivateSoundTime = gpGlobals->curtime + 3.0;
		}

		return;
	}

	// Check if we've eached the end of this scan and need to swap to the other dir
	int da = UTIL_AngleDistance( m_angGoal.y, m_angAiming.y );
	if( ( da > -1.0f ) && ( da < 1.0f ) )
	{
		if( m_angGoal.y < m_angAimBase.y ) 
			m_angGoal.y = m_angAimBase.y + SG_SCAN_HALFWIDTH;
		else
			m_angGoal.y = m_angAimBase.y - SG_SCAN_HALFWIDTH;
	}

	m_angGoal.x = m_angAimBase.x;

	// Turn and ping
	UpdateFacing();
	Ping();
}

static ConVar sg_pos( "ffdev_sg_pos", "34", FCVAR_ARCHIVE | FCVAR_CHEAT );

//-----------------------------------------------------------------------------
// Purpose: Allows the turret to fire on targets if they're visible
//-----------------------------------------------------------------------------
void CFFSentryGun::OnActiveThink( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::OnActiveThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	OnObjectThink();

	// Update our think time
	SetNextThink( gpGlobals->curtime + 0.1f );

	CBaseEntity *enemy = GetEnemy();

	// We've just finished being maliciously sabotaged, so remove enemy here
	if (m_bShootingTeammates && m_flSabotageTime <= gpGlobals->curtime)
	{
		m_bShootingTeammates = false;
		enemy = NULL;
	}

	// Enemy is no longer targettable
	if( !enemy || !FVisible( enemy ) || !enemy->IsAlive() /* || ( ( enemy->GetAbsOrigin() - GetAbsOrigin() ).LengthSqr() > ( SG_RANGE * SG_RANGE ) )*/ )
	{
		SetEnemy( NULL );
		SetThink( &CFFSentryGun::OnSearchThink );
		SpinDown();
		return;
	}

	// Get our shot positions
	Vector vecMid = MuzzlePosition();
	Vector vecMidEnemy = GetEnemy()->BodyTarget( vecMid, false ); // false: not 'noisey', so no random z added on

	// Update our goal directions
	Vector vecDirToEnemy = vecMidEnemy - vecMid;

	// Actually we're pretty close, and we'll wobble unless we use something a 
	//bit more static as the source
	if (vecDirToEnemy.LengthSqr() < 10000)
	{
		vecMid = GetAbsOrigin() + Vector(0, 0, 40.0f);
		vecDirToEnemy = vecMidEnemy - vecMid;
	}

	VectorNormalize( vecDirToEnemy );
	VectorAngles( vecDirToEnemy, m_angGoal );

	// Update angles now, otherwise we'll always be lagging behind
	UpdateFacing();

	Vector vecAiming, vecGoal;
	AngleVectors( m_angAiming, &vecAiming );
	AngleVectors( m_angGoal, &vecGoal );

	bool bFired = false;

	// Fire shells
	if( ( gpGlobals->curtime > m_flNextShell ) && ( m_iShells > 0 ) ) 
	{
		m_flNextShell = gpGlobals->curtime + m_flShellCycleTime;

		if( vecAiming.Dot( vecGoal ) > DOT_5DEGREE )
			Shoot( MuzzlePosition(), vecAiming, true );

		bFired = true;
	}

	// Fire rockets
	if( ( gpGlobals->curtime > m_flNextRocket ) && ( m_iRockets > 0 ) )
	{
		m_flNextRocket = gpGlobals->curtime + m_flRocketCycleTime;

		// Something is REALLY wrong with the attachments. The angle
		// and origin is way off where they're supposed to be... It's
		// gotta be something in the model...
		
		Vector vecOrigin;
		QAngle vecAngles;

		//vecOrigin.Init();
		//vecAngles.Init();

		// This does not get the correct location from the attachment
		// and I don't know why...
		if( m_bRocketLeftBarrel )
			GetAttachment( m_iRocketLAttachment, vecOrigin, vecAngles );
		else
			GetAttachment( m_iRocketRAttachment, vecOrigin, vecAngles );

		// So, since the attachment grabbing isn't working, hacking in a
		// position for dustbowl playtest

		//vecOrigin.z -= sg_pos.GetFloat();

		//vecDirToEnemy = GetEnemy()->BodyTarget( vecOrigin, false ) - vecOrigin;
		//VectorNormalize( vecDirToEnemy );

		//QAngle vecDirToEnemyAngles;
		//VectorAngles( vecDirToEnemy, vecDirToEnemyAngles );

		//NDebugOverlay::Line( vecOrigin, vecOrigin + ( vecDirToEnemy * 256.0f ), 255, 0, 0, false, 5.0f );

		// TODO: Need to factor in a dot product for aiming as the sg can be not facing
		// someone but its time to fire a rocket so rocket comes out of the back of sg 
		// or something that looks silly

		// Bug #0000583: Dying to the rockets for the sentry gun doesn't accredit kills.
		//CFFProjectileRocket::CreateRocket(this, vecOrigin, vecDirToEnemyAngles, this, 102, 900.0f );
		CFFProjectileRocket::CreateRocket(this, vecOrigin, vecAngles, this, 102, 900.0f );

		// Rockets weren't being decremented
		m_iRockets--;

		bFired = true;

		// Flip which barrel to come out of next
		m_bRocketLeftBarrel = !m_bRocketLeftBarrel;
	}

	if( bFired ) 
	{
		// Recalculate ammo percentage, 7 bits for shells + 1 bit for no rockets
		m_iAmmoPercent = 100.0f * (float) m_iShells / m_iMaxShells;
		if( m_iMaxRockets && !m_iRockets ) 
			m_iAmmoPercent += 128;

		SendStatsToBot();
	}
}

// Decide whether this new target is better than the current one
CBaseEntity *SG_IsBetterTarget( CBaseEntity *cur, CBaseEntity *latest, float distance ) 
{
	static float lastdistance = 0;

	if( !latest ) 
		return cur;

	if( !cur ) 
		return latest;

	// A player is always preferable to a buildable
	if( latest->IsPlayer() && !cur->IsPlayer() ) 
		return latest;

	// Check radio tagged players
	if( latest->IsPlayer() && cur->IsPlayer() )
	{
		if( ToFFPlayer( latest )->IsRadioTagged() && !ToFFPlayer( cur )->IsRadioTagged() )
			return latest;
		else if( !ToFFPlayer( latest )->IsRadioTagged() && ToFFPlayer( cur )->IsRadioTagged() )
			return cur;
	}

	// Go for the nearest
	if( distance < lastdistance ) 
		return latest;

	return cur;
}

//-----------------------------------------------------------------------------
// Purpose: The turret doesn't run base AI properly, which is a bad decision.
//			As a result, it has to manually find enemies.
//-----------------------------------------------------------------------------
void CFFSentryGun::HackFindEnemy( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::HackFindEnemy", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Find our owner
	CFFPlayer *pOwner = ToFFPlayer( m_hOwner.Get() );

	if( !pOwner ) 
	{
		Warning( "[SentryGun] Can't find our owner!\n" );
		return;
	}

	// Our location - used later
	Vector vecOrigin = GetAbsOrigin();
	CBaseEntity *target = NULL;

	for( int i = 1; i <= gpGlobals->maxClients; i++ ) 
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex(i) );

		// Mirv: If we are maliciously sabotaged, then shoot teammates instead.
		int iTypeToTarget = IsShootingTeammates() ? GR_TEAMMATE : GR_NOTTEAMMATE;

		// Changed a line for
		// Bug #0000526: Sentry gun stays locked onto teammates if mp_friendlyfire is changed
		// Don't bother
		if( !pPlayer || !pPlayer->IsPlayer() || !pPlayer->IsAlive() || pPlayer->IsObserver() || /*pPlayer == pOwner ||*/
			( g_pGameRules->PlayerRelationship(pOwner, pPlayer) != iTypeToTarget ) )
			continue;

		// Spy check - but don't let valid radio tagged targets sneak by!
		if( pPlayer->IsDisguised() )
		{
			// Spy disguised as owners team
			if( ( pPlayer->GetDisguisedTeam() == pOwner->GetTeamNumber() ) && ( !IsPlayerRadioTagTarget( pPlayer, pOwner->GetTeamNumber() ) ) )
				continue;

			// Spy disguised as allied team
			//if( pOwnerTeam->GetAllies() & ( 1 << pPlayer->GetDisguisedTeam() ) )
			if( FFGameRules()->IsTeam1AlliedToTeam2( pOwner->GetTeamNumber(), pPlayer->GetDisguisedTeam() ) && !IsPlayerRadioTagTarget( pPlayer, pOwner->GetTeamNumber() ) )
				continue;
		}

		// Added stuff for Bug #0000669: SG can currently lock on to anybody at any range

		// Check a couple more locations to check as technically they could be visible whereas others wouldn't be
		if( ( FVisible( pPlayer->GetAbsOrigin() ) || FVisible( pPlayer->GetLegacyAbsOrigin() ) || FVisible( pPlayer->EyePosition() ) ) && ( vecOrigin.DistTo( pPlayer->GetAbsOrigin() ) <= sg_range.GetFloat() ) ) 
			target = SG_IsBetterTarget( target, pPlayer, ( pPlayer->GetAbsOrigin() - vecOrigin ).LengthSqr() );

		// Add sentry guns
		if( pPlayer->m_hSentryGun.Get() )
		{
			CFFSentryGun *pSentryGun = static_cast< CFFSentryGun * >( pPlayer->m_hSentryGun.Get() );
			if( pSentryGun != this )
			{
				if( ( FVisible( pSentryGun->GetAbsOrigin() ) || FVisible( pSentryGun->EyePosition() ) ) && ( vecOrigin.DistTo( pSentryGun->GetAbsOrigin() ) <= sg_range.GetFloat() ) )
					target = SG_IsBetterTarget( target, pSentryGun, ( pSentryGun->GetAbsOrigin() - vecOrigin ).LengthSqr() );
			}
		}

		// Add dispensers
		if( pPlayer->m_hDispenser.Get() )
		{
			CFFDispenser *pDispenser = static_cast< CFFDispenser * >( pPlayer->m_hDispenser.Get() );
			if( ( FVisible( pDispenser->GetAbsOrigin() ) || FVisible( pDispenser->EyePosition() ) ) && ( vecOrigin.DistTo( pDispenser->GetAbsOrigin() ) <= sg_range.GetFloat() ) )
				target = SG_IsBetterTarget( target, pDispenser, ( pDispenser->GetAbsOrigin() - vecOrigin ).LengthSqr() );
		}
	}

	SetEnemy( target );
}

//-----------------------------------------------------------------------------
// Purpose: Returns the speed at which the turret can face a target
//-----------------------------------------------------------------------------
float CFFSentryGun::MaxYawSpeed( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::MaxYawSpeed", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if( GetEnemy() )
		return sg_turnspeed.GetFloat();
		
	return 1.0f;
}

float CFFSentryGun::MaxPitchSpeed( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::MaxPitchSpeed", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if( GetEnemy() )
		return sg_pitchspeed.GetFloat();

	return 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: Fire!
//-----------------------------------------------------------------------------
void CFFSentryGun::Shoot( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict ) 
{
	VPROF_BUDGET( "CFFSentryGun::Shoot", VPROF_BUDGETGROUP_FF_BUILDABLE );

	FireBulletsInfo_t info;
	Vector vecDir;

	if( m_iShells <= 0 ) 
		return;

	// Shoot in direction we're facing or shoot directly at enemy?
	if( !bStrict && GetEnemy() ) 
	{
		AssertMsg( 0, "Do you really want to hit enemy regardless?" );
		vecDir = GetEnemy()->GetAbsOrigin() - EyePosition();
	}
	else
		vecDir = vecDirToEnemy;

	info.m_vecSrc = vecSrc;
	info.m_vecDirShooting = vecDir;
	info.m_iTracerFreq = 1;
	info.m_iShots = 1;
	info.m_pAttacker = this;
	info.m_vecSpread = VECTOR_CONE_PRECALCULATED;
	info.m_flDistance = MAX_COORD_RANGE;
	info.m_iAmmoType = m_iAmmoType;
	info.m_iDamage = m_iShellDamage;

	// Introduce quite a big spread now if sabotaged
	// but not if we're in malicious mode
	if (IsSabotaged()&& !IsShootingTeammates())
		info.m_vecSpread = VECTOR_CONE_10DEGREES;

	FireBullets( info );
	EmitSound( "Sentry.Fire" );
	DoMuzzleFlash();

	// Change barrel
	m_bLeftBarrel = !m_bLeftBarrel;	

	m_iShells--;
}

void CFFSentryGun::DoMuzzleFlash( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::DoMuzzleFlash", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CEffectData data;

	data.m_nAttachmentIndex = m_iMuzzleAttachment;

	if( m_iLevel > 2 )
	{
		if( m_bLeftBarrel )
			data.m_nAttachmentIndex = m_iLBarrelAttachment;
		else
			data.m_nAttachmentIndex = m_iRBarrelAttachment;
	}

	data.m_nEntIndex = entindex();
	DispatchEffect( "MuzzleFlash", data );
}

//-----------------------------------------------------------------------------
// Purpose: Make a pinging noise so the player knows where we are
//-----------------------------------------------------------------------------
void CFFSentryGun::Ping( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::Ping", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// See if it's time to ping again
	if( m_flPingTime > gpGlobals->curtime ) 
		return;

	// Ping!
	EmitSound( "Sentry.Scan" );

	m_flPingTime = gpGlobals->curtime + SG_PING_TIME;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFSentryGun::SpinUp( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::SpinUp", VPROF_BUDGETGROUP_FF_BUILDABLE );

	EmitSound("Sentry.Spot");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFSentryGun::SpinDown( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::SpinDown", VPROF_BUDGETGROUP_FF_BUILDABLE );
}

//-----------------------------------------------------------------------------
// Purpose: Causes the turret to face its desired angles
//-----------------------------------------------------------------------------
bool CFFSentryGun::UpdateFacing( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::UpdateFacing", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Get vectors for sg base
	Vector vecBaseForward, vecBaseRight, vecBaseUp;
	GetVectors( &vecBaseForward, &vecBaseRight, &vecBaseUp );

	// Calculate new yaw first
	float src_yaw = GetAbsAngles().y;
	float dst_yaw = m_angGoal.y - src_yaw;
	float cur_yaw = m_angAiming.y - src_yaw;
	float new_yaw = UTIL_ApproachAngle( dst_yaw, cur_yaw, MaxYawSpeed() );

	m_angAiming.y = new_yaw + src_yaw;

	SetPoseParameter( m_iYawPoseParameter, TO_YAW( new_yaw ) );

	// Calculate the real pitch target, this depends on the angle at the point we are at now
	Vector vecMuzzle;
	QAngle angMuzzle;
	GetAttachment( (m_iLevel == 3 ? m_iEyeAttachment : m_iMuzzleAttachment), vecMuzzle, angMuzzle );

	// Get orientation pitch at intended orientation
	Vector dir; 
	AngleVectors( QAngle( 0, m_angAiming.y, 0 ), &dir );

	Vector cross = CrossProduct( dir, vecBaseUp );
	Vector fwd = CrossProduct( vecBaseUp, cross );
	QAngle angFwd; VectorAngles( fwd, angFwd );

	float src_pitch = TO_PITCH( angFwd.x );
	float cur_pitch = TO_PITCH( m_angAiming.x ) - src_pitch;
	float dst_pitch = TO_PITCH( m_angGoal.x ) - src_pitch;

#ifdef _DEBUG
	/* VOOGRU: I debug with dedicated server, and I don't want srcds to throw 
		util.cpp (552) : Assertion Failed: !"UTIL_GetListenServerHost" */
	//if( sg_debug.GetBool() && !engine->IsDedicatedServer()) 
	//{
	//	NDebugOverlay::Line(EyePosition(), EyePosition() + dir * 300.0f, 40, 40, 40, false, 0.05f);
	//	NDebugOverlay::Line(EyePosition(), EyePosition() + vecBaseUp * 300.0f, 110, 110, 110, false, 0.05f);
	//	NDebugOverlay::Line(EyePosition(), EyePosition() + cross * 300.0f, 180, 180, 180, false, 0.05f);
	//	NDebugOverlay::Line(EyePosition(), EyePosition() + fwd * 300.0f, 255, 255, 255, false, 0.05f);
	//}
#endif

	// Target pitch = constrained goal pitch - current pitch
	float new_pitch = UTIL_Approach( clamp( dst_pitch, SG_MIN_PITCH, SG_MAX_PITCH ), cur_pitch, MaxPitchSpeed() );
	SetPoseParameter( m_iPitchPoseParameter, new_pitch );

	m_angAiming.x = FROM_PITCH( new_pitch + src_pitch );

	InvalidateBoneCache();
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Called when the object's health < 0
//-----------------------------------------------------------------------------
void CFFSentryGun::Event_Killed( const CTakeDamageInfo &info ) 
{
	VPROF_BUDGET( "CFFSentryGun::Event_Killed", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if( m_hOwner.Get() )
		ClientPrint( ToFFPlayer( m_hOwner.Get() ), HUD_PRINTCENTER, "#FF_SENTRYGUN_DESTROYED" );

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: Gets the position of the eyes
//-----------------------------------------------------------------------------
Vector CFFSentryGun::EyePosition( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::EyePosition", VPROF_BUDGETGROUP_FF_BUILDABLE );

	Vector vecOrigin;
	QAngle vecAngles;

	GetAttachment( (m_iLevel == 3 ? m_iEyeAttachment : m_iMuzzleAttachment), vecOrigin, vecAngles );

	return vecOrigin;
}
Vector CFFSentryGun::MuzzlePosition( void )
{
	VPROF_BUDGET( "CFFSentryGun::MuzzlePosition", VPROF_BUDGETGROUP_FF_BUILDABLE );

	Vector vecOrigin;
	QAngle vecAngles;
	GetAttachment( m_iMuzzleAttachment, vecOrigin, vecAngles );
	if( m_iLevel > 2 )
	{
		if( m_bLeftBarrel )
			GetAttachment( m_iLBarrelAttachment, vecOrigin, vecAngles );
		else
			GetAttachment( m_iRBarrelAttachment, vecOrigin, vecAngles );
	}
	return vecOrigin;
}

//-----------------------------------------------------------------------------
// Purpose: Upgrade the SG
//-----------------------------------------------------------------------------
void CFFSentryGun::Upgrade( bool bUpgradeLevel, int iCells, int iShells, int iRockets ) 
{
	VPROF_BUDGET( "CFFSentryGun::Update", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if( bUpgradeLevel ) 
	{
		if( m_iLevel < 3 ) 
			m_iLevel++;

		float flAimPitch = GetPoseParameter( SG_BC_PITCH );
		float flAimYaw = GetPoseParameter( SG_BC_YAW );

		CPASAttenuationFilter sndFilter( this );

		switch( m_iLevel ) 
		{
		case 1:
			SetModel( FF_SENTRYGUN_MODEL );
			SetSolid( SOLID_VPHYSICS );
			
			m_iShells = 20;

			m_iMaxShells = 100;
			m_iMaxRockets = 0;
			m_iShellDamage = 15;

			m_flShellCycleTime = 0.2f;

			m_iMaxHealth = 150;
			m_iHealth = 150;

			m_flLockTime = 1.0f;
			//m_flTurnSpeed = 4.0f;
			m_flTurnSpeed = sg_turnspeed.GetFloat();

			break;

		case 2:
			SetModel( FF_SENTRYGUN_MODEL_LVL2 );
			SetSolid( SOLID_VPHYSICS );
			EmitSound( sndFilter, entindex(), "Sentry.Two" );

			m_iMaxShells = 125;
			m_iMaxRockets = 0;
			m_iShellDamage = 15;

			m_flShellCycleTime = 0.1f;

			m_iMaxHealth = 180;
			m_iHealth = 180;

			m_flLockTime = 0.5f;
			//m_flTurnSpeed = 7.0f;
			m_flTurnSpeed = sg_turnspeed.GetFloat();

			// Update attachments
			m_iMuzzleAttachment = LookupAttachment( "barrel01" );
			m_iEyeAttachment = LookupAttachment( "eyes" );

			break;

		case 3:
			SetModel( FF_SENTRYGUN_MODEL_LVL3 );
			SetSolid( SOLID_VPHYSICS );
			EmitSound( sndFilter, entindex(), "Sentry.Three" );

			m_iMaxShells = 150;
			m_iMaxRockets = 20;
			m_iShellDamage = 15;

			m_flShellCycleTime = 0.1f;
			m_flRocketCycleTime = 3.0f;

			m_iMaxHealth = 216;
			m_iHealth = 216;

			m_flLockTime = 0.5f;
			//m_flTurnSpeed = 7.0f;
			m_flTurnSpeed = sg_turnspeed.GetFloat();
			
			m_iEyeAttachment = LookupAttachment( "eyes" );
			m_iLBarrelAttachment = LookupAttachment( "barrel01" );
			m_iRBarrelAttachment = LookupAttachment( "barrel02" );

			m_iRocketLAttachment = LookupAttachment( "rocket01" );
			m_iRocketRAttachment = LookupAttachment( "rocket02" );

			break;
		}

		SetPoseParameter( m_iPitchPoseParameter, flAimPitch );
		SetPoseParameter( m_iYawPoseParameter, flAimYaw );

		// Re-adjust size
		UTIL_SetSize( this, FF_SENTRYGUN_MINS, FF_SENTRYGUN_MAXS );
	}
	else
	{
		m_iHealth = clamp( m_iHealth + iCells * 3.5f, 0, m_iMaxHealth );
		m_iShells = clamp( m_iShells + iShells, 0, m_iMaxShells );
		m_iRockets = clamp( m_iRockets + iRockets, 0, m_iMaxRockets );

		// Bug #0000238: Repairing sg doesn't remove damage decals
		if( iCells > 0 )
			RemoveAllDecals();
	}

	SendStatsToBot();

	IGameEvent *pEvent = gameeventmanager->CreateEvent( "sentrygun_upgraded" );
	if( pEvent )
	{
		CFFPlayer *pOwner = static_cast<CFFPlayer*>( m_hOwner.Get() );
		pEvent->SetInt( "userid", pOwner->GetUserID() );
		pEvent->SetInt( "level", m_iLevel );
		gameeventmanager->FireEvent( pEvent, true );
	}

	// Recalculate ammo percentage, 7 bits for shells + 1 bit for no rockets
	m_iAmmoPercent = 100.0f * (float) m_iShells / m_iMaxShells;
	if( m_iMaxRockets && !m_iRockets ) 
		m_iAmmoPercent += 128;
}

//-----------------------------------------------------------------------------
// Purpose: Creates the object
//-----------------------------------------------------------------------------
CFFSentryGun *CFFSentryGun::Create( const Vector &vecOrigin, const QAngle &vecAngles, CBaseEntity *pentOwner ) 
{
	// Create the object
	CFFSentryGun *pObject = ( CFFSentryGun * )CBaseEntity::Create( "FF_SentryGun", vecOrigin, vecAngles, NULL );

	// Set our faux owner - see CFFBuildable::Create for the reason why
	pObject->m_hOwner = pentOwner;

	//pObject->VPhysicsInitNormal( SOLID_VPHYSICS, pObject->GetSolidFlags(), true );

	// Spawn the object
	pObject->Spawn();

	return pObject;
}

// Player-set aim focus point!
void CFFSentryGun::SetFocusPoint( Vector &origin ) 
{
	VPROF_BUDGET( "CFFSentryGun::SetFocusPoint", VPROF_BUDGETGROUP_FF_BUILDABLE );

	QAngle newangle;
	Vector dir = origin - EyePosition();
	VectorNormalize( dir );

	VectorAngles( dir, newangle );
    
	// Shift the goal along by the change in angle
	m_angGoal.y += newangle.y - m_angAimBase.y;

	m_angAimBase = newangle;

	// Bug #0000427: Sound needed for Sentrygun aim feature
	// Play aim sound
	CPASAttenuationFilter sndFilter( this );
	EmitSound( sndFilter, entindex(), "Sentry.Aim" );    

#ifdef _DEBUG
	/* VOOGRU: I debug with dedicated server, and I don't want srcds to throw 
		util.cpp (552) : Assertion Failed: !"UTIL_GetListenServerHost" */
	//if( sg_debug.GetBool() && !engine->IsDedicatedServer()) 
	//	NDebugOverlay::Line( EyePosition(), origin, 255, 0, 255, false, 5.0f );
#endif

	CFFPlayer *pOwner = static_cast<CFFPlayer *>( m_hOwner.Get() );
	if(pOwner && pOwner->IsBot())
	{
		Omnibot::Notify_SentryAimed(pOwner);
	}
}

// How much damage should be taken from an emp explosion
int CFFSentryGun::TakeEmp( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::TakeEmp", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// This base damage matches up the total damage with tfc
	int ammodmg = 100;

	// These values are from tfc.
	ammodmg += m_iShells * 0.5f;
	ammodmg += m_iRockets * 1.3f;

	return ammodmg;
}

void CFFSentryGun::SendStatsToBot( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::SendStatsTobot", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CFFPlayer *pOwner = static_cast<CFFPlayer *>( m_hOwner.Get() );
	if (pOwner && pOwner->IsBot()) 
	{
		Omnibot::BotUserData bud;
		bud.DataType = Omnibot::BotUserData::dt6_2byteFlags;
		bud.udata.m_2ByteFlags[0] = m_iHealth;
		bud.udata.m_2ByteFlags[1] = m_iMaxHealth;

		bud.udata.m_2ByteFlags[2] = m_iShells;
		bud.udata.m_2ByteFlags[3] = m_iMaxShells;

		bud.udata.m_2ByteFlags[4] = m_iRockets | (m_iMaxRockets << 16);
		bud.udata.m_2ByteFlags[5] = m_iLevel;

		int iGameId = pOwner->entindex() -1;
		Omnibot::omnibot_interface::Bot_Interface_SendEvent(
			Omnibot::TF_MSG_SENTRY_STATS, 
			iGameId, 0, 0, &bud);
	}
}

//-----------------------------------------------------------------------------
// Purpose: If already sabotaged then don't try and sabotage again
//-----------------------------------------------------------------------------
bool CFFSentryGun::CanSabotage()
{
	VPROF_BUDGET( "CFFSentryGun::CanSabotage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if (!m_bBuilt)
		return false;

	return !IsSabotaged();
}

//-----------------------------------------------------------------------------
// Purpose: Is this building in level 1 sabotage
//-----------------------------------------------------------------------------
bool CFFSentryGun::IsSabotaged()
{
	VPROF_BUDGET( "CFFSentryGun::IsSabotaged", VPROF_BUDGETGROUP_FF_BUILDABLE );

	return (m_hSaboteur && m_flSabotageTime > gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: Is the SG in level 2 sabotage (shooting teammates mode)
//-----------------------------------------------------------------------------
bool CFFSentryGun::IsShootingTeammates()
{
	VPROF_BUDGET( "CFFSentryGun::IsShootingTeammates", VPROF_BUDGETGROUP_FF_BUILDABLE );

	return (m_hSaboteur && m_bShootingTeammates && m_flSabotageTime > gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: Sabotaged results in SG doing less damage (to simulate being
//			less accurate). Need to keep track of saboteur so that they can
//			trigger the malicious sabotage via their menu.
//-----------------------------------------------------------------------------
void CFFSentryGun::Sabotage(CFFPlayer *pSaboteur)
{
	VPROF_BUDGET( "CFFSentryGun::Sabotage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	m_flSabotageTime = gpGlobals->curtime + 120.0f;
	m_hSaboteur = pSaboteur;
	m_bShootingTeammates = false;


	Warning("SG sabotaged\n");
}

//-----------------------------------------------------------------------------
// Purpose: This turns the sentry on its own team for 10 seconds.
//			To differentiate between normal and malicious sabotage, we're
//			just going to set m_hSaboteur to NULL (cheap, I know)
//-----------------------------------------------------------------------------
void CFFSentryGun::MaliciousSabotage(CFFPlayer *pSaboteur)
{
	VPROF_BUDGET( "CFFSentryGun::MaliciousSabotage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	m_flSabotageTime = gpGlobals->curtime + 10.0f;
	m_bShootingTeammates = true;

	// Cancel target so it searchs for a new (friendly one)
	SetEnemy(NULL);

	Warning("SG maliciously sabotaged\n");
}

//-----------------------------------------------------------------------------
// Purpose: Overridden just to fire the appropriate event.
//-----------------------------------------------------------------------------
void CFFSentryGun::Detonate()
{
	VPROF_BUDGET( "CFFSentryGun::Detonate", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Fire an event.
	IGameEvent *pEvent = gameeventmanager->CreateEvent("sentry_detonated");						
	if(pEvent)
	{
		CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
		pEvent->SetInt("userid", pOwner->GetUserID());
		gameeventmanager->FireEvent(pEvent, true);
	}

	CFFBuildableObject::Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: Carry out the radius damage for this buildable
//-----------------------------------------------------------------------------
void CFFSentryGun::DoExplosionDamage()
{
	VPROF_BUDGET( "CFFSentryGun::DoExplosionDamage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	float flDamage = 2 * (m_iRockets * 1.4f + m_iShells * 1.0f);
	flDamage = min(280, flDamage);
	
	CTakeDamageInfo info(this, m_hOwner, vec3_origin, GetAbsOrigin() + Vector(0, 0, 32.0f), flDamage, DMG_BLAST);
	RadiusDamage(info, GetAbsOrigin(), flDamage * 2.0f, CLASS_NONE, NULL);

	UTIL_ScreenShake(GetAbsOrigin(), flDamage * 0.0125f, 150.0f, m_flExplosionDuration, 620.0f, SHAKE_START);
}