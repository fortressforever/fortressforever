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
#include "IEffects.h"

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
//ConVar	sg_debug( "ffdev_sg_debug", "1", FCVAR_CHEAT );
//#define SG_DEBUG sg_debug.GetBool()
ConVar	sg_usepvs( "ffdev_sg_usepvs", "0", FCVAR_FF_FFDEV_REPLICATED );
#define SG_USEPVS false // sg_usepvs.GetBool()
ConVar	sg_turnspeed( "ffdev_sg_turnspeed", "5.5", FCVAR_FF_FFDEV_REPLICATED );
#define SG_TURNSPEED sg_turnspeed.GetFloat() // 5.5f
ConVar	sg_pitchspeed( "ffdev_sg_pitchspeed", "4.0", FCVAR_FF_FFDEV_REPLICATED );
#define SG_PITCHSPEED sg_pitchspeed.GetFloat() // 4.0f
ConVar  sg_range( "ffdev_sg_range", "1050.0", FCVAR_FF_FFDEV_REPLICATED );
#define SG_RANGE sg_range.GetFloat() // 1050.0f
ConVar  sg_range_untarget( "ffdev_sg_range_untarget", "1155.0", FCVAR_FF_FFDEV_REPLICATED );
#define SG_RANGE_UNTARGET sg_range_untarget.GetFloat() // 1155.0f

ConVar  sg_range_cloakmulti( "ffdev_sg_range_cloakmulti", "0.666", FCVAR_FF_FFDEV_REPLICATED );
#define SG_RANGE_CLOAKMULTI sg_range_cloakmulti.GetFloat() // 0.666f

ConVar  sg_cloaksonar_interval_near( "ffdev_sg_cloaksonar_interval_near", "2.02", FCVAR_FF_FFDEV_REPLICATED );
#define SG_CLOAKSONAR_INTERVAL_NEAR sg_cloaksonar_interval_near.GetFloat() // 2.02f
ConVar  sg_cloaksonar_interval_far( "ffdev_sg_cloaksonar_interval_far", "3.03", FCVAR_FF_FFDEV_REPLICATED );
#define SG_CLOAKSONAR_INTERVAL_FAR sg_cloaksonar_interval_far.GetFloat() // 3.03f

ConVar  sg_cloaksonar_pitch_near( "ffdev_sg_cloaksonar_pitch_near", "92", FCVAR_FF_FFDEV_REPLICATED );
#define SG_CLOAKSONAR_PITCH_NEAR sg_cloaksonar_pitch_near.GetInt() // 92
ConVar  sg_cloaksonar_pitch_far( "ffdev_sg_cloaksonar_pitch_far", "108", FCVAR_FF_FFDEV_REPLICATED );
#define SG_CLOAKSONAR_PITCH_FAR sg_cloaksonar_pitch_far.GetInt() // 108

ConVar sg_explosiondamage_base("ffdev_sg_explosiondamage_base", "51.0", FCVAR_FF_FFDEV_REPLICATED, "Base damage for the SG explosion");
#define SG_EXPLOSIONDAMAGE_BASE sg_explosiondamage_base.GetFloat() // 51.0f 
ConVar ffdev_sg_bulletpush("ffdev_sg_bulletpush", "4.0", FCVAR_FF_FFDEV_REPLICATED, "SG bullet push force");
#define SG_BULLETPUSH ffdev_sg_bulletpush.GetFloat() // 4.0f
// Jiggles: NOT a cheat for now so the betas can test it, but make it a cheat before release!!!
ConVar ffdev_sg_groundpush_multiplier_lvl1("ffdev_sg_groundpush_multiplier_lvl1", "7.0", FCVAR_FF_FFDEV_REPLICATED, "SG level 1 ground bullet push multiplier");
#define SG_GROUNDPUSH_MULTIPLIER_LVL1 ffdev_sg_groundpush_multiplier_lvl1.GetFloat() // 7.0f
ConVar ffdev_sg_groundpush_multiplier_lvl2("ffdev_sg_groundpush_multiplier_lvl2", "4.0", FCVAR_FF_FFDEV_REPLICATED, "SG level 2 ground bullet push multiplier");
#define SG_GROUNDPUSH_MULTIPLIER_LVL2 ffdev_sg_groundpush_multiplier_lvl2.GetFloat() // SG_GROUNDPUSH_MULTIPLIER_LVL1
ConVar ffdev_sg_groundpush_multiplier_lvl3("ffdev_sg_groundpush_multiplier_lvl3", "4.0", FCVAR_FF_FFDEV_REPLICATED, "SG level 3 ground bullet push multiplier");
#define SG_GROUNDPUSH_MULTIPLIER_LVL3 ffdev_sg_groundpush_multiplier_lvl3.GetFloat() // SG_GROUNDPUSH_MULTIPLIER_LVL1
ConVar ffdev_sg_bulletdamage("ffdev_sg_bulletdamage", "14", FCVAR_FF_FFDEV_REPLICATED, "SG bullet damage");
#define SG_BULLETDAMAGE ffdev_sg_bulletdamage.GetInt() // 14.0f

// AfterShock; These values will be rounded by the ActiveThink time (at time of writing 0.01), so 0.125 = 0.13
ConVar sg_shotcycletime_lvl1("ffdev_sg_shotcycletime_lvl1", "0.2", FCVAR_FF_FFDEV_REPLICATED, "Level 1 SG time between shots");
#define SG_SHOTCYCLETIME_LVL1 0.2f //sg_shotcycletime_lvl1.GetFloat()
ConVar sg_shotcycletime_lvl2("ffdev_sg_shotcycletime_lvl2", "0.129", FCVAR_FF_FFDEV_REPLICATED, "Level 2 SG time between shots");
#define SG_SHOTCYCLETIME_LVL2 0.129f //sg_shotcycletime_lvl2.GetFloat()
ConVar sg_shotcycletime_lvl3("ffdev_sg_shotcycletime_lvl3", "0.1", FCVAR_FF_FFDEV_REPLICATED, "Level 3 SG time between shots");
#define SG_SHOTCYCLETIME_LVL3 0.1f //sg_shotcycletime_lvl3.GetFloat()

//ConVar sg_warningshots_delay("ffdev_sg_warningshots_delay", "0.2", FCVAR_FF_FFDEV_REPLICATED, "Time between warning shots");
//#define SG_WARNINGSHOTS_DELAY sg_warningshots_delay.GetFloat()
//ConVar sg_warningshots_angle("ffdev_sg_warningshots_angle", "0.985", FCVAR_FF_FFDEV_REPLICATED, "Dotproduct angle where SG will start firing warning shots. 5deg=0.996, 10deg=0.985");
//#define SG_WARNINGSHOTS_ANGLE sg_warningshots_angle.GetFloat()
//ConVar sg_shoot_angle("ffdev_sg_shoot_angle", "30.0", FCVAR_FF_FFDEV_REPLICATED, "Angle where SG will start firing.");
//#define SG_SHOOT_ANGLE sg_shoot_angle.GetFloat()
//ConVar sg_shoot_angle_distmult("ffdev_sg_shoot_angle_distmult", "0.008", FCVAR_FF_FFDEV_REPLICATED, "The angle required to shoot the target is multiplied by (distance to target * this)");
//#define SG_SHOOT_ANGLE_DISTMULT sg_shoot_angle_distmult.GetFloat()


ConVar sg_health_lvl1("ffdev_sg_health_lvl1", "145", FCVAR_FF_FFDEV_REPLICATED, "Level 1 SG health");
#define SG_HEALTH_LEVEL1 sg_health_lvl1.GetInt() // 160
ConVar sg_health_lvl2("ffdev_sg_health_lvl2", "180", FCVAR_FF_FFDEV_REPLICATED, "Level 2 SG health");
#define SG_HEALTH_LEVEL2 sg_health_lvl2.GetInt() // 180
ConVar sg_health_lvl3("ffdev_sg_health_lvl3", "200", FCVAR_FF_FFDEV_REPLICATED, "Level 3 SG health");
#define SG_HEALTH_LEVEL3 sg_health_lvl3.GetInt() // 200

ConVar sg_lockontime_lvl1("ffdev_sg_lockontime_lvl1", "0.20", FCVAR_FF_FFDEV_REPLICATED, "Level 1 SG lock on time");
#define SG_LOCKONTIME_LVL1 sg_lockontime_lvl1.GetFloat() // 0.2f
ConVar sg_lockontime_lvl2("ffdev_sg_lockontime_lvl2", "0.20", FCVAR_FF_FFDEV_REPLICATED, "Level 2 SG lock on time");
#define SG_LOCKONTIME_LVL2 sg_lockontime_lvl2.GetFloat() // SG_LOCKONTIME_LVL1
ConVar sg_lockontime_lvl3("ffdev_sg_lockontime_lvl3", "0.20", FCVAR_FF_FFDEV_REPLICATED, "Level 3 SG lock on time");
#define SG_LOCKONTIME_LVL3 sg_lockontime_lvl3.GetFloat() // SG_LOCKONTIME_LVL1

//ConVar sg_lagbehindmul("ffdev_sg_lagbehindmul", "10", FCVAR_FF_FFDEV_REPLICATED, "% of player speed to lag behind");
//#define SG_LAGBEHINDMUL sg_lagbehindmul.GetFloat()

//ConVar sg_timetoreachfullturnspeed("ffdev_sg_timetoreachfullturnspeed", "0.0", FCVAR_FF_FFDEV_REPLICATED, "How many seconds should the SG take to accelerate up to full turnspeed when changing from idle to locked");
//#define SG_TIMETOREACHFULLTURNSPEED sg_timetoreachfullturnspeed.GetFloat()
//ConVar sg_timetoreachfullfirespeed("ffdev_sg_timetoreachfullfirespeed", "0.0", FCVAR_FF_FFDEV_REPLICATED, "How many seconds should the SG take to reach full fire rate when changing from idle to locked");
//#define SG_TIMETOREACHFULLFIRESPEED sg_timetoreachfullfirespeed.GetFloat()
//ConVar sg_lowfirespeed("ffdev_sg_lowfirespeed", "0.1", FCVAR_FF_FFDEV_REPLICATED, "This time is added to normal cycletime at beginning of a lock when it ramps up");
//#define SG_LOWFIRESPEED sg_lowfirespeed.GetFloat()

ConVar sg_returntoidletime("ffdev_sg_returntoidletime", "1.0", FCVAR_FF_FFDEV_REPLICATED, "How many seconds should the SG stay focused after losing a lock, in case the enemy re-appears");
#define SG_RETURNTOIDLETIME sg_returntoidletime.GetFloat() // 1.0f
ConVar sg_returntoidlespeed("ffdev_sg_returntoidlespeed", "0.1", FCVAR_FF_FFDEV_REPLICATED, "Speed the SG turns when it's just lost a lock, should be slower than scan speed (1.0)");
#define SG_TURNSPEED_AFTERLOCK sg_returntoidlespeed.GetFloat() // 0.1f

ConVar sg_empdmg_base("ffdev_sg_empdmg_base", "100", FCVAR_FF_FFDEV_REPLICATED, "Base damage a sentry takes from an emp.");
#define SG_EMPDMG_BASE sg_empdmg_base.GetFloat() // 100.0f
ConVar sg_empdmg_shells_multi("ffdev_sg_empdmg_shells_multi", "0.5", FCVAR_FF_FFDEV_REPLICATED, "Base emp damage plus the sentry's shell count times this");
#define SG_EMPDMG_SHELLS_MULTI sg_empdmg_shells_multi.GetFloat() // 0.5f
ConVar sg_empdmg_rockets_multi("ffdev_sg_empdmg_rockets_multi", "0.9", FCVAR_FF_FFDEV_REPLICATED, "Base emp damage plus the sentry's rocket count times this");
#define SG_EMPDMG_ROCKETS_MULTI sg_empdmg_rockets_multi.GetFloat() // 0.9f

ConVar sg_acknowledge_sabotage_delay("ffdev_sg_acknowledge_sabotage_delay", "2.5", FCVAR_FF_FFDEV_REPLICATED, "Sentry won't spot a maliciously sabotaged sentry for this long");
#define SG_ACKNOWLEDGE_SABOTAGE_DELAY sg_acknowledge_sabotage_delay.GetFloat() // 2.5f

// caes: limit angular acceleration of SG
ConVar sg_accel_yaw("ffdev_sg_accel_yaw", "0.75", FCVAR_FF_FFDEV_REPLICATED, "Maximum angular acceleration of SG in yaw");
#define SG_ANGULAR_ACCEL_YAW sg_accel_yaw.GetFloat() // 0.75f
ConVar sg_accel_pitch("ffdev_sg_accel_pitch", "1.5", FCVAR_FF_FFDEV_REPLICATED, "Maximum angular acceleration of SG in pitch");
#define SG_ANGULAR_ACCEL_PITCH sg_accel_pitch.GetFloat() // 1.5f
ConVar sg_accel_distmult("ffdev_sg_accel_distmult", "0.008", FCVAR_FF_FFDEV_REPLICATED, "Multiplier of distance taken into account on turn accel (smaller value makes SG better at tracking 'weaving' ppl)");
#define SG_ACCELDISTANCEMULT sg_accel_distmult.GetFloat() // 0.008f
ConVar sg_accel_fricmult("ffdev_sg_accel_fricmult", "2.0", FCVAR_FF_FFDEV_REPLICATED, "Multiplier of maximum angular acceleration when slowing down");
#define SG_ACCELFRICTIONMULT sg_accel_fricmult.GetFloat() // 2.0f
// caes

IMPLEMENT_SERVERCLASS_ST(CFFSentryGun, DT_FFSentryGun) 
	SendPropInt( SENDINFO( m_iAmmoPercent), 8, SPROP_UNSIGNED ), 
	//SendPropFloat( SENDINFO( m_flRange ) ), //AfterShock: surely the client knows it's range?
	SendPropInt( SENDINFO( m_iLevel ), 2, SPROP_UNSIGNED ), //AfterShock: max level 3
	SendPropInt( SENDINFO( m_iShells ), 8, SPROP_UNSIGNED ), //AfterShock: max 150 shells for level 3
	SendPropInt( SENDINFO( m_iRockets ), 5, SPROP_UNSIGNED ), //AfterShock: max 20 rockets for level 3
	SendPropInt( SENDINFO( m_iMaxShells ) ), //AfterShock: this should be inferred from level
	SendPropInt( SENDINFO( m_iMaxRockets ) ), //AfterShock: this should be inferred from level
END_SEND_TABLE() 

LINK_ENTITY_TO_CLASS( FF_SentryGun, CFFSentryGun );
PRECACHE_REGISTER( FF_SentryGun );

// Datatable
BEGIN_DATADESC(CFFSentryGun) 
	DEFINE_THINKFUNC( OnActiveThink ), 
	DEFINE_THINKFUNC( OnSearchThink ), 
END_DATADESC() 

extern const char *g_pszFFSentryGunModels[];
extern const char *g_pszFFSentryGunGibModels[];
extern const char *g_pszFFSentryGunSounds[];

extern const char *g_pszFFGenGibModels[];

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CFFSentryGun::CFFSentryGun() 
{
	m_ppszModels = g_pszFFSentryGunModels;
	m_ppszGibModels = g_pszFFSentryGunGibModels;
	m_ppszSounds = g_pszFFSentryGunSounds;

	// Set lvl1 range
	//m_flRange = 1024.0f;

	// Set level - keep it < 0 until we GoLive the first time
	m_iLevel = 0;
	m_flThinkTime = 0.1f;

	m_flPingTime = 0;
	m_flNextActivateSoundTime = 0;
	m_flAcknowledgeSabotageTime = 0.0f;
	m_flStartLockTime = 0.0f;
	m_flEndLockTime = 0.0f;
	m_flNextShell = 0;
	m_flShotAccumulator = 0;
	m_flNextRocket = 0;
	m_flLastSight = 0;
	m_iMaxShells = 200; // TODO: Get Number
	m_iMaxRockets = 0;
	m_iRockets = 0;
	m_iShellDamage = 15;
	m_bLeftBarrel = true;
	m_bRocketLeftBarrel = true;

	m_angGoal.Init();

	m_bSendNailGrenHint = true;

	m_flLastCloakSonarSound = 0.0f;
	m_flCloakDistance = 65536.0f;

	m_flNextSparkTime = 0;
	m_flLastClientUpdate = 0;
	m_iLastState = 0;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFFSentryGun::~CFFSentryGun( void ) 
{
}

void CFFSentryGun::UpdateOnRemove( void ) 
{
	BaseClass::UpdateOnRemove();
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

	SetViewOffset(EyeOffset(ACT_IDLE));

	AddFlag( FL_AIMTARGET );
	AddEFlags( EFL_NO_DISSOLVE );

	SetPoseParameter( SG_BC_YAW, 0 );
	SetPoseParameter( SG_BC_PITCH, 0) ;

	// Change this to neuter the sg
	m_iAmmoType = GetAmmoDef()->Index( AMMO_SHELLS );

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

	// caes: set angular speeds to 0
	m_angSpeed_yaw = 0.0;
	m_angSpeed_pitch = 0.0;
	// caes
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

	// CFFPlayer *pOwner = static_cast< CFFPlayer * >( m_hOwner.Get() );
	
	// Bug #0000244: Building L1 sg doesn't take away cells
	
	// Bug #0001558: exploit to get instant lvl2 SG.
	// Cells are now taken when build starts and returned if build is cancelled -> Defrag
	
	//if( pOwner ) 
	//	pOwner->RemoveAmmo( 130, AMMO_CELLS );

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
	m_bMaliciouslySabotaged = false;
	m_iSaboteurTeamNumber = TEAM_UNASSIGNED;

	// Figure out if we're under water or not
	if( UTIL_PointContents( GetAbsOrigin() + Vector( 0, 0, 48 ) ) & CONTENTS_WATER )
		SetWaterLevel( WL_Eyes );
	else if( UTIL_PointContents( GetAbsOrigin() + Vector( 0, 0, 24 ) ) & CONTENTS_WATER )
		SetWaterLevel( WL_Waist );
	else if( UTIL_PointContents( GetAbsOrigin() ) & CONTENTS_WATER )
		SetWaterLevel( WL_Feet );
}

//-----------------------------------------------------------------------------
// Purpose: Generic think function
//-----------------------------------------------------------------------------
void CFFSentryGun::OnObjectThink( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::OnObjectThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CheckForOwner();

	// Animate
	StudioFrameAdvance();

	// Spark if at 50% or less health
	if ( m_iHealth <= ( m_iMaxHealth / 2 ) && gpGlobals->curtime >= m_flNextSparkTime )
	{
		// Emit some sparks from a random location hopefully inside the SG's bounds :)
		Vector vecUp(0, 0, 1.0f);
		g_pEffects->Sparks( GetAbsOrigin() + Vector(random->RandomFloat( -16, 16 ), random->RandomFloat( -16, 16 ), random->RandomFloat( 5, 32 )), 2, 2, &vecUp );
		EmitSound( "DoSpark" );
		m_flNextSparkTime = gpGlobals->curtime + random->RandomFloat( 0.5, 2.5 );
	}

	if ( m_flAcknowledgeSabotageTime != 0.0f && m_flAcknowledgeSabotageTime + 0.5f <= gpGlobals->curtime )
		m_flAcknowledgeSabotageTime = 0.0f;

	// We've just finished being maliciously sabotaged, so remove enemy here
	if (m_bMaliciouslySabotaged && m_flSabotageTime <= gpGlobals->curtime)
	{
		// Jiggles: No real point in playing this since we're blowing up the sg now
		//EmitSound( "Sentry.SabotageFinish" );

		// Explode SG on sabotage finish
		// TODO: create custom death message for it
		if ( m_hSaboteur )
			TakeDamage( CTakeDamageInfo( this, m_hSaboteur, 10000, DMG_GENERIC ) );
		else	// Jiggles: I'm not sure what would happen if the Saboteur leaves the server before this
		{
			CFFPlayer *pOwner = ToFFPlayer( m_hOwner.Get() );
			if ( pOwner )
				TakeDamage( CTakeDamageInfo( this, pOwner, 10000, DMG_GENERIC ) );
		}
	}

	// Run base class thinking
	CFFBuildableObject::OnObjectThink();
}

//-----------------------------------------------------------------------------
// Purpose: Target doesn't exist or has eluded us, so search for one
//-----------------------------------------------------------------------------
void CFFSentryGun::OnSearchThink( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::OnSearchThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	OnObjectThink();

	SetNextThink( gpGlobals->curtime + 0.029f ); // Just less than 1 tick (33 tickrate) or just less than 2 ticks (66 tick) or just less than 3 (100 tick)

	if( GetEnemy() && !GetEnemy()->IsAlive() ) 
		SetEnemy( NULL );

	if( !GetEnemy() ) 
		SetEnemy(HackFindEnemy());

	// hlstriker: Added to make sure sentry doesn't fire at ghost buildables
	CFFBuildableObject *pBuildable = dynamic_cast <CFFBuildableObject *> (GetEnemy());
	if( pBuildable && FF_IsBuildableObject(pBuildable) && !pBuildable->IsBuilt() )
		SetEnemy( NULL );

	if( GetEnemy() )
	{
		// Pause for the lock-on time
		m_flNextShell = gpGlobals->curtime + m_flLockTime;

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

//-----------------------------------------------------------------------------
// Purpose: Allows the turret to fire on targets if they're visible
//-----------------------------------------------------------------------------
void CFFSentryGun::OnActiveThink( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::OnActiveThink", VPROF_BUDGETGROUP_FF_BUILDABLE );

	OnObjectThink();

	// Update our think time
	SetNextThink( gpGlobals->curtime + 0.029f ); // slightly less than 1 tick (33 tick), 

	CBaseEntity *enemy = GetEnemy();

	// Jiggles: Hint that tells Soldiers to use nail grens on SGs
	CFFPlayer *pFFPlayer = ToFFPlayer( enemy );
 	if( m_bSendNailGrenHint && pFFPlayer && ( pFFPlayer->GetClassSlot() == CLASS_SOLDIER ) )
	{
		FF_SendHint( pFFPlayer, SOLDIER_SENTRY, 3, PRIORITY_NORMAL, "#FF_HINT_SOLDIER_SENTRY" );
		m_bSendNailGrenHint = false;
	}
	// End hint code

	// We've just finished being maliciously sabotaged, so remove enemy here
	if (m_bMaliciouslySabotaged && m_flSabotageTime <= gpGlobals->curtime)
		enemy = NULL;

	// Enemy is no longer targettable // hlstriker: Crashing bug when sg loses sight of enemy buildable is somewhere in this if statement/nest
	if( !enemy 
			|| ( !FVisible( enemy ) && ( pFFPlayer && !FVisible( pFFPlayer->GetLegacyAbsOrigin() ) ) /*&& !FVisible( pFFPlayer->GetAbsOrigin() ) && !FVisible( pFFPlayer->EyePosition() )*/  )
			|| !enemy->IsAlive()
			|| ( WorldSpaceCenter().DistTo( enemy->GetAbsOrigin() ) > SG_RANGE_UNTARGET ) )
			// || ( WorldSpaceCenter().DistTo( enemy->GetAbsOrigin() ) > ( SG_RANGE_UNTARGET * SG_RANGE_CLOAKMULTI ) && pFFPlayer && pFFPlayer->IsCloaked() ) )
	{
		if ( enemy && enemy->IsAlive() )
		{
			// AfterShock: if we lost track of our target, and they are still alive, 
			// and we're looking the right way, then pause to see if our target comes back
			Vector vecAiming, vecGoal;
			AngleVectors( m_angAiming, &vecAiming );
			AngleVectors( m_angGoal, &vecGoal );
			bool bCanFire = vecAiming.Dot( vecGoal ) > DOT_7DEGREE;
			if ( bCanFire )			
				m_flEndLockTime = gpGlobals->curtime; 

			// Tell player they aren't locked on any more, and remove the status icon
			if ( enemy->IsPlayer() )
			{
				CSingleUserRecipientFilter user( ToBasePlayer( enemy ) );
				user.MakeReliable();

				UserMessageBegin(user, "StatusIconUpdate");
					WRITE_BYTE(FF_STATUSICON_LOCKEDON);
					WRITE_FLOAT(0.0);
				MessageEnd();
			}
		}

		SetEnemy( NULL );
		SetThink( &CFFSentryGun::OnSearchThink );
		SpinDown();
		return; // No target, do nothing!
	}

	// If we're targeting a buildable, and a player is a better target, change.
	if(enemy && FF_IsBuildableObject(enemy))
	{
		CBaseEntity *pNewTarget = HackFindEnemy();
		if(pNewTarget && pNewTarget->IsPlayer())
		{
			// Tell player they're locked on, and give the status icon
			CSingleUserRecipientFilter user( ToBasePlayer( pNewTarget ) );
			user.MakeReliable();

			UserMessageBegin(user, "StatusIconUpdate");
				WRITE_BYTE(FF_STATUSICON_LOCKEDON);
				WRITE_FLOAT(-1.0); //forever
			MessageEnd();

			SetEnemy(pNewTarget);
		}
	}

	// Get the approximate distance that we're firing
	float flDistance = (enemy->GetAbsOrigin() - WorldSpaceCenter()).Length();

	// Get our shot positions
	Vector vecMid = MuzzlePosition();

	// Actually we're a bit too close, so move further back
	if (flDistance < 120.0f)
	{
		int iAttachment = GetLevel() > 2 ? (m_bLeftBarrel ? m_iLBarrelAttachment : m_iRBarrelAttachment) : m_iMuzzleAttachment;

		Vector vecTemp;
		QAngle angTemp;
		GetAttachment(iAttachment, vecTemp, angTemp);
		AngleVectors(angTemp, &vecTemp);

		vecMid -= vecTemp * 50.0f;
	}

	Vector vecMidEnemy = GetEnemy()->BodyTarget( vecMid, false ); // false: not 'noisey', so no random z added on

	/* AfterShock: Don't hit targets moving fast across our vision. 
	// Commented for now! Intend to replace with 
	QAngle vecAngles = GetEnemy()->EyeAngles();
	Vector vecForward;
	AngleVectors( vecAngles, &vecForward );

	vecMidEnemy -= vecForward * SG_LAGBEHINDMUL;
	*/

	// Update our goal directions
	Vector vecDirToEnemy = vecMidEnemy - vecMid;

	/*if ( SG_DEBUG )
	{
		debugoverlay->AddLineOverlay(vecMid, vecMidEnemy, 255, 0, 255, false, 0.1f);
	}*/

	// Actually we're pretty close, and we'll wobble unless we use something a 
	//bit more static as the source
	/*if (vecDirToEnemy.LengthSqr() < 10000)
	{
		vecMid = GetAbsOrigin() + Vector(0, 0, 40.0f);
		vecDirToEnemy = vecMidEnemy - vecMid;
	}

	if ( SG_DEBUG )
	{
		debugoverlay->AddLineOverlay(vecMid, vecMidEnemy, 255, 255, 0, false, 0.1f);
	}*/

	VectorNormalize( vecDirToEnemy );
	VectorAngles( vecDirToEnemy, m_angGoal );

	// Update angles now, otherwise we'll always be lagging behind
	UpdateFacing();

	Vector vecAiming, vecGoal;
	AngleVectors( m_angAiming, &vecAiming );
	AngleVectors( m_angGoal, &vecGoal );

	// Are we rotated enough to where we can fire?
	bool bCanFire = vecAiming.Dot( vecGoal ) > DOT_5DEGREE; 

	//bool bCanAlmostFire = vecAiming.Dot( vecGoal ) > SG_WARNINGSHOTS_ANGLE;

	//bool bCanFire = vecAiming.Dot( vecGoal ) > cosf( (SG_SHOOT_ANGLE / ( flDistance * SG_SHOOT_ANGLE_DISTMULT )) / 58.0f ); // scale the angle required by target distance
	//bool bCanAlmostFire = vecAiming.Dot( vecGoal ) > cosf( (SG_WARNINGSHOTS_ANGLE / ( flDistance * SG_SHOOT_ANGLE_DISTMULT )) / 59.0f );

	// Did we fire
	bool bFired = false;

	if( bCanFire )
	{
		// Fire rockets
		if( GetLevel() >= 3 )
		{
			if( ( gpGlobals->curtime > m_flNextRocket ) && ( m_iRockets > 0 ) && ( gpGlobals->curtime > m_flNextShell ) )
			{
				ShootRockets( RocketPosition(), vecAiming, true );

				m_flNextRocket = gpGlobals->curtime + m_flRocketCycleTime;
				bFired = true;
			}
		}

		// Fire shells
		if( ( gpGlobals->curtime > m_flNextShell ) && ( m_iShells > 0 ) ) 
		{
			Vector vecOrigin;
			QAngle vecAngles;
			GetAttachment( m_iMuzzleAttachment, vecOrigin, vecAngles );
			//AngleVectors(vecAngles, &vecAiming);

			Shoot( MuzzlePosition(), vecAiming, true );

			//m_flNextShell = gpGlobals->curtime + m_flShellCycleTime;

			// AfterShock: ramp up fire rate starting from SG_LOWFIRESPEED and reaching normal fire rate at time SG_TIMETOREACHFULLFIRESPEED
			/* Actually, this is too complex, removing
			float prop;
			if ( ( gpGlobals->curtime - m_flStartLockTime ) >= SG_TIMETOREACHFULLFIRESPEED )
			{
				prop = 1.0f;
			}
			else
				prop = ( gpGlobals->curtime - m_flStartLockTime ) / SG_TIMETOREACHFULLFIRESPEED;

			prop = 1.0f - prop;
			m_flNextShell = gpGlobals->curtime + m_flShellCycleTime + SG_LOWFIRESPEED * prop;

			*/
			m_flNextShell = gpGlobals->curtime + m_flShellCycleTime;

			bFired = true;
		}		
	}	
	/*
	else if ( bCanAlmostFire )
	{
		// Fire warning shots
		if( ( gpGlobals->curtime > m_flNextShell + SG_WARNINGSHOTS_DELAY ) && ( m_iShells > 0 ) ) 
		{
			Vector vecOrigin;
			QAngle vecAngles;
			GetAttachment( m_iMuzzleAttachment, vecOrigin, vecAngles );
			//AngleVectors(vecAngles, &vecAiming);

			Shoot( MuzzlePosition(), vecAiming, true );

			m_flNextShell = gpGlobals->curtime + m_flShellCycleTime;
			bFired = true;
		}	
	} */

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
	{
		lastdistance = distance;
		return latest;
	}

	// A player is always preferable to a buildable
	if( latest->IsPlayer() && !cur->IsPlayer() ) 
	{
		lastdistance = distance;
		return latest;
	}

	// If we're on a player already, don't consider non players
	// Do we want to do this?
	if(cur->IsPlayer() && !latest->IsPlayer())
		return cur;

	// Check radio tagged players
	if( latest->IsPlayer() && cur->IsPlayer() )
	{
		if( ToFFPlayer( latest )->IsRadioTagged() && !ToFFPlayer( cur )->IsRadioTagged() )
		{
			lastdistance = distance;
			return latest;
		}
		else if( !ToFFPlayer( latest )->IsRadioTagged() && ToFFPlayer( cur )->IsRadioTagged() )
			return cur;
	}

	// Go for the nearest
	if( distance < lastdistance ) 
	{
		lastdistance = distance;
		return latest;
	}

	return cur;
}

//-----------------------------------------------------------------------------
// Purpose: The turret doesn't run base AI properly, which is a bad decision.
//			As a result, it has to manually find enemies.
//-----------------------------------------------------------------------------
CBaseEntity *CFFSentryGun::HackFindEnemy( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::HackFindEnemy", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Find our owner
	CFFPlayer *pOwner = ToFFPlayer( m_hOwner.Get() );

	if( !pOwner ) 
	{
		Warning( "[SentryGun] Can't find our owner!\n" );
		return 0;
	}

	// Our location - used later
	Vector vecOrigin = GetAbsOrigin();
	CBaseEntity *target = NULL;

	// reset every single time through
	m_flCloakDistance = 65536.0f;

	for( int i = 1; i <= gpGlobals->maxClients; i++ ) 
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex(i) );
		if( !pPlayer )
			continue;
		if( pPlayer->IsObserver() )
			continue;

		bool bIsSentryVisible = false;
		bool bIsSentryMaliciouslySabotaged = false;
		CFFSentryGun *pSentryGun = pPlayer->GetSentryGun();
		if( IsTargetVisible( pSentryGun ) ) //Yes this does null pointer check
		{
				bIsSentryVisible = true;
				if ( pSentryGun->IsMaliciouslySabotaged() && g_pGameRules->PlayerRelationship( pOwner, pSentryGun->m_hSaboteur ) != GR_TEAMMATE )
				{
					// wait a few seconds before spotting a maliciously sabotaged sentry
					if ( m_flAcknowledgeSabotageTime == 0.0f )
						m_flAcknowledgeSabotageTime = gpGlobals->curtime + SG_ACKNOWLEDGE_SABOTAGE_DELAY;
					else if ( m_flAcknowledgeSabotageTime <= gpGlobals->curtime )
						bIsSentryMaliciouslySabotaged = true;
				}
		}

		// Mirv: If we are maliciously sabotaged, then shoot teammates instead.
		int iTypeToTarget = IsMaliciouslySabotaged() ? GR_TEAMMATE : GR_NOTTEAMMATE;

		// Changed a line for
		// Bug #0000526: Sentry gun stays locked onto teammates if mp_friendlyfire is changed
		// Don't bother
		if( g_pGameRules->PlayerRelationship(pOwner, pPlayer) != iTypeToTarget )
		{
			// if a teammate's maliciously sabotaged sentry is spotted, try to target it
			if ( !bIsSentryMaliciouslySabotaged )
				continue;
		}

		if ( pPlayer->IsCloaked() )
		{
			// the player won't be visible, but m_flCloakDistance may change and cause the sonar sound to emit
			IsTargetVisible( pPlayer );
			continue;
		}

		// Spy check - but don't let valid radio tagged targets sneak by!
		if( pPlayer->IsDisguised() ) // && !pPlayer->IsCloaked() )
		{
			// Spy disguised as owners team
			if( ( pPlayer->GetDisguisedTeam() == pOwner->GetTeamNumber() ) && ( !IsPlayerRadioTagTarget( pPlayer, pOwner->GetTeamNumber() ) ) )
				continue;

			// Spy disguised as allied team
			//if( pOwnerTeam->GetAllies() & ( 1 << pPlayer->GetDisguisedTeam() ) )
			if( FFGameRules()->IsTeam1AlliedToTeam2( pOwner->GetTeamNumber(), pPlayer->GetDisguisedTeam() ) && !IsPlayerRadioTagTarget( pPlayer, pOwner->GetTeamNumber() ) )
				continue;
		}

		// IsTargetVisible checks for NULL so these are all safe...

		if( IsTargetVisible( pPlayer ) && !bIsSentryMaliciouslySabotaged )
			target = SG_IsBetterTarget( target, pPlayer, ( pPlayer->GetAbsOrigin() - vecOrigin ).LengthSqr() );

		if( bIsSentryVisible )
		{
			if ( !( pSentryGun->IsMaliciouslySabotaged() && g_pGameRules->PlayerRelationship( pSentryGun->m_hSaboteur, m_hSaboteur ) == GR_TEAMMATE ) )
				target = SG_IsBetterTarget( target, pSentryGun, ( pSentryGun->GetAbsOrigin() - vecOrigin ).LengthSqr() );
		}

		CFFDispenser *pDispenser = pPlayer->GetDispenser();
		if( IsTargetVisible( pDispenser ) && !bIsSentryMaliciouslySabotaged )
		{
			if ( !( pDispenser->IsMaliciouslySabotaged() && g_pGameRules->PlayerRelationship( pDispenser->m_hSaboteur, m_hSaboteur ) == GR_TEAMMATE ) )
				target = SG_IsBetterTarget( target, pDispenser, ( pDispenser->GetAbsOrigin() - vecOrigin ).LengthSqr() );
		}
		
		CFFManCannon *pManCannon = pPlayer->GetManCannon();
		if( IsTargetVisible( pManCannon ) && !bIsSentryMaliciouslySabotaged )
		{
			target = SG_IsBetterTarget( target, pManCannon, ( pManCannon->GetAbsOrigin() - vecOrigin ).LengthSqr() );
		}

		/*
		// Check a couple more locations to check as technically they could be visible whereas others wouldn't be
		if( ( FVisible( pPlayer->GetAbsOrigin() ) || FVisible( pPlayer->GetLegacyAbsOrigin() ) || FVisible( pPlayer->EyePosition() ) ) && ( vecOrigin.DistTo( pPlayer->GetAbsOrigin() ) <= SG_RANGE ) ) 
			target = SG_IsBetterTarget( target, pPlayer, ( pPlayer->GetAbsOrigin() - vecOrigin ).LengthSqr() );

		// Add sentry guns
		if( pPlayer->GetSentryGun() )
		{
			CFFSentryGun *pSentryGun = pPlayer->GetSentryGun();
			if( pSentryGun != this )
			{
				if( ( FVisible( pSentryGun->GetAbsOrigin() ) || FVisible( pSentryGun->EyePosition() ) ) && ( vecOrigin.DistTo( pSentryGun->GetAbsOrigin() ) <= SG_RANGE ) )
					target = SG_IsBetterTarget( target, pSentryGun, ( pSentryGun->GetAbsOrigin() - vecOrigin ).LengthSqr() );
			}
		}

		// Add dispensers
		if( pPlayer->GetDispenser() )
		{
			CFFDispenser *pDispenser = pPlayer->GetDispenser();
			if( ( FVisible( pDispenser->GetAbsOrigin() ) || FVisible( pDispenser->EyePosition() ) ) && ( vecOrigin.DistTo( pDispenser->GetAbsOrigin() ) <= SG_RANGE ) )
				target = SG_IsBetterTarget( target, pDispenser, ( pDispenser->GetAbsOrigin() - vecOrigin ).LengthSqr() );
		}
		*/
	}

	if ( m_flCloakDistance < 65536.0f )
	{
		float flPercent = clamp( m_flCloakDistance / ( SG_RANGE_UNTARGET * SG_RANGE_CLOAKMULTI ), 0.0f, 1.0f );
		float flInterval = SG_CLOAKSONAR_INTERVAL_NEAR + ( ( SG_CLOAKSONAR_INTERVAL_FAR - SG_CLOAKSONAR_INTERVAL_NEAR ) * flPercent );

		if ( m_flLastCloakSonarSound + flInterval <= gpGlobals->curtime )
		{
			CSoundParameters params;
			if ( GetParametersForSound( "Sentry.CloakSonar", params, NULL ) )
			{
				CPASAttenuationFilter filter( this, params.soundlevel );

				EmitSound_t ep;
				ep.m_nChannel = params.channel;
				ep.m_pSoundName = params.soundname;
				ep.m_SoundLevel = params.soundlevel;
				ep.m_pOrigin = &GetAbsOrigin();

				ep.m_flVolume = params.volume;
				ep.m_nPitch = SG_CLOAKSONAR_PITCH_FAR + ( ( SG_CLOAKSONAR_PITCH_NEAR - SG_CLOAKSONAR_PITCH_FAR ) * flPercent ); // far and near are swapped right here

				EmitSound( filter, entindex(), ep );

				Vector vecUp;
				QAngle qAngles = GetAbsAngles();
				AngleVectors(qAngles, NULL, NULL, &vecUp);
				g_pEffects->EnergySplash(GetAbsOrigin(), vecUp, false);

				m_flLastCloakSonarSound = gpGlobals->curtime;
			}
		}
	}

	return target;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the speed at which the turret can face a target
//-----------------------------------------------------------------------------
float CFFSentryGun::MaxYawSpeed( void ) const 
{
	VPROF_BUDGET( "CFFSentryGun::MaxYawSpeed", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if( GetEnemy() )
	{
		/* AfterShock: no more initial accel on turnspeed
		float prop;
		if ( ( gpGlobals->curtime - m_flStartLockTime ) >= SG_TIMETOREACHFULLTURNSPEED )
		{
			prop = 1.0f;
		}
		else
			prop = ( gpGlobals->curtime - m_flStartLockTime ) / SG_TIMETOREACHFULLTURNSPEED;

		return SG_TURNSPEED * prop;
		*/
		return SG_TURNSPEED;
	}
	else if ( m_flEndLockTime > ( gpGlobals->curtime - SG_RETURNTOIDLETIME ) ) // just lost my target, so i'm gonna stay focused for a sec in case he reappears
	{
		return SG_TURNSPEED_AFTERLOCK; // slower than scan speed
	}
	else
		return 1.0f; // Scan speed
}

//-----------------------------------------------------------------------------
// Purpose: Returns the speed at which the turret can pitch to a target
//-----------------------------------------------------------------------------
float CFFSentryGun::MaxPitchSpeed( void ) const
{
	VPROF_BUDGET( "CFFSentryGun::MaxPitchSpeed", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if( GetEnemy() )
	{
		/*
		float prop;
		if ( ( gpGlobals->curtime - m_flStartLockTime ) >= SG_TIMETOREACHFULLTURNSPEED )
		{
			prop = 1.0f;
		}
		else
			prop = ( gpGlobals->curtime - m_flStartLockTime ) / SG_TIMETOREACHFULLTURNSPEED;

		return SG_PITCHSPEED * prop;
		*/
		return SG_PITCHSPEED;
	}
	else if ( m_flEndLockTime > ( gpGlobals->curtime - SG_RETURNTOIDLETIME ) ) // just lost my target, so i'm gonna stay focused for a sec in case he reappears
	{
		return SG_TURNSPEED_AFTERLOCK; // slower than scan speed
	}
	else
		return 2.0f; // Scan speed

}

//-----------------------------------------------------------------------------
// Purpose: See if a target is visible
//-----------------------------------------------------------------------------
bool CFFSentryGun::IsTargetVisible( CBaseEntity *pTarget )
{
	if( !pTarget )
		return false;

	CFFPlayer *pFFPlayer = ToFFPlayer( pTarget );

	// early out if player's not even alive
	if ( pFFPlayer && !pFFPlayer->IsAlive() )
		return false;

	// Get our aiming position
	Vector vecOrigin = WorldSpaceCenter();

	// Get a position on the target
	Vector vecTarget = pTarget->BodyTarget( vecOrigin, false );

	float flDistToTarget = vecOrigin.DistTo( vecTarget );

	// Check for out of range
	if( flDistToTarget > SG_RANGE )
		return false;

	// Check PVS for early out
	if(SG_USEPVS)
	{
		byte pvs[ MAX_MAP_CLUSTERS/8 ];
		int iPVSCluster = engine->GetClusterForOrigin(vecOrigin);
		int iPVSLength = engine->GetPVSForCluster(iPVSCluster, sizeof(pvs), pvs);
		if(!engine->CheckOriginInPVS(vecTarget, pvs, iPVSLength))
			return false;
	}

	// Can we trace to the target?
	trace_t tr;
	// Using MASK_SHOT instead of MASK_PLAYERSOLID so SGs track through anything they can actually shoot through
	UTIL_TraceLine( vecOrigin, vecTarget, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr );
	//UTIL_TraceLine( vecOrigin, vecTarget, MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr );

	/*if ( SG_DEBUG )
	{
		int r = 0, g = 0, b = 0;
		if(tr.fraction < 1.f)
			r = 255;
		else
			g = 255;
		debugoverlay->AddLineOverlay(vecOrigin, vecTarget, r, g, b, false, 0.1f);
	}*/

	// What did our trace hit?
	if( tr.startsolid || /*( tr.fraction != 1.0f ) ||*/ !tr.m_pEnt || FF_TraceHitWorld( &tr ) )
		return false;

	if(tr.m_pEnt != pTarget)
		return false;

	// Is the trace hit entity a valid sg target
	if( !IsTargetClassTValid( tr.m_pEnt->Classify() ) )
		return false;

	if ( pFFPlayer && pFFPlayer->IsCloaked() )
	{
		if (flDistToTarget <= ( SG_RANGE * SG_RANGE_CLOAKMULTI ) && flDistToTarget < m_flCloakDistance )
			m_flCloakDistance = flDistToTarget;

		return false;
	}
	
	// Finally, is that target even in our aim ellipse?
	return IsTargetInAimingEllipse( vecTarget );
}

//-----------------------------------------------------------------------------
// Purpose: See if a target is in our aim ellipse
//-----------------------------------------------------------------------------
bool CFFSentryGun::IsTargetInAimingEllipse( const Vector& vecTarget ) const
{
	// The SG can't see that well behind it so we plug
	// in the target origin to a math equation to see
	// if the target origin is inside our "aim ellipse"

	// TODO:

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: SG can target these classes
//-----------------------------------------------------------------------------
bool CFFSentryGun::IsTargetClassTValid( Class_T cT ) const
{
	return ( ( cT == CLASS_PLAYER ) || ( cT == CLASS_SENTRYGUN ) || ( cT == CLASS_DISPENSER ) || ( cT == CLASS_MANCANNON ) );
}

//-----------------------------------------------------------------------------
// Purpose: Fire Bullets!
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
	info.m_iTracerFreq = 0;	// Mirv: Doing tracers down below now
	info.m_iShots = 1;
	// Jiggles: We want to fire more than 1 shot if the SG's think rate is too slow to keep up with the cycle time value
	//			Since we can't fire partial bullets, we have to accumulate them
	// But don't do it if it has been longer than the SG's think time
	float flLastShellDelta = gpGlobals->curtime - m_flNextShell;
	if ( flLastShellDelta <= 0.1f )
	{
		m_flShotAccumulator += (flLastShellDelta / m_flShellCycleTime);
		if ( m_flShotAccumulator >= 1 )
		{
			info.m_iShots += m_flShotAccumulator;
			m_flShotAccumulator -= (info.m_iShots - 1); // Remove the whole numbers (shots); leave the remainder
		}
	}
	info.m_pAttacker = this;
	info.m_vecSpread = VECTOR_CONE_PRECALCULATED;
	info.m_flDistance = MAX_COORD_RANGE;
	info.m_iAmmoType = m_iAmmoType;
	info.m_iDamage = m_iShellDamage;
	info.m_flDamageForceScale = SG_BULLETPUSH;
	// Jiggles: A HACK to address the fact that it takes a lot more force to push players around on the ground than in the air
	CFFPlayer *pEnemyTarget = ToFFPlayer( GetEnemy() );
	if ( pEnemyTarget && (pEnemyTarget->GetFlags() & FL_ONGROUND) )
	{
		switch (m_iLevel)
		{
		case 1:
			info.m_flDamageForceScale *= SG_GROUNDPUSH_MULTIPLIER_LVL1;
			break;
		case 2:
			info.m_flDamageForceScale *= SG_GROUNDPUSH_MULTIPLIER_LVL2;
			break;
		case 3:
			info.m_flDamageForceScale *= SG_GROUNDPUSH_MULTIPLIER_LVL3;
			break;
		}
	}
		

	// Introduce quite a big spread now if sabotaged
	// but not if we're in malicious mode
	//if (IsSabotaged()&& !IsMaliciouslySabotaged())
	//	info.m_vecSpread = VECTOR_CONE_10DEGREES;

	/*if ( SG_DEBUG )
	{
		debugoverlay->AddLineOverlay(vecSrc, vecSrc+vecDir*1024.f, 0, 255, 0, false, 0.2f);
	}*/

	FireBullets( info );
	EmitSound( "Sentry.Fire" );

	QAngle vecAngles;
	VectorAngles( vecDir, vecAngles );

	int iAttachment = GetLevel() > 2 ? (m_bLeftBarrel ? m_iLBarrelAttachment : m_iRBarrelAttachment) : m_iMuzzleAttachment;

	trace_t tr;
	// Using MASK_SHOT instead of MASK_PLAYERSOLID so SGs track through anything they can actually shoot through
	UTIL_TraceLine(vecSrc, vecSrc + vecDir * 4096.0f, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	//UTIL_TraceLine(vecSrc, vecSrc + vecDir * 4096.0f, MASK_PLAYERSOLID, this, COLLISION_GROUP_PLAYER, &tr);

	CEffectData data;
	data.m_vStart = vecSrc;
	data.m_vOrigin = data.m_vStart;
	data.m_nEntIndex = GetBaseAnimating()->entindex();
	data.m_flScale = GetLevel() > 1 ? 8.0f : 1.0f;
	data.m_nAttachmentIndex = iAttachment;
	data.m_fFlags = MUZZLEFLASH_TYPE_DEFAULT;

	DispatchEffect("MuzzleFlash", data);

	CEffectData data2;
	data2.m_vStart = vecSrc;
	data2.m_vOrigin = tr.endpos;
	data2.m_nEntIndex = GetBaseAnimating()->entindex();
	data2.m_flScale = 0.0f;
	data2.m_nAttachmentIndex = iAttachment;
	DispatchEffect(GetLevel() == 1 ? "Tracer" : "AR2Tracer", data2);

	// Change barrel
	m_bLeftBarrel = !m_bLeftBarrel;	

	m_iShells--;
}

//-----------------------------------------------------------------------------
// Purpose: Fire Rockets!
//-----------------------------------------------------------------------------
void CFFSentryGun::ShootRockets( const Vector &vecSrc, const Vector &vecDirToEnemy, bool bStrict ) 
{
	VPROF_BUDGET( "CFFSentryGun::ShootRockets", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if( m_iRockets <= 0 )
		return;

	Vector vecDir = vecDirToEnemy;

	// Shoot in direction we're facing or shoot directly at enemy?
	if( !bStrict && GetEnemy() ) 
	{
		AssertMsg( 0, "Rockets - Do you really want to hit enemy regardless?" );
		vecDir = GetEnemy()->BodyTarget( vecSrc, false ) - vecSrc;
	}

	QAngle vecAngles;
	VectorAngles( vecDir, vecAngles );

	/*if ( SG_DEBUG )
	{
		debugoverlay->AddLineOverlay(vecSrc, vecSrc+vecDir*1024.f, 0, 255, 0, false, 0.2f);
	}*/

	const int iSentryDamage = 102;
	const int iSentryDamageRadius = 102;
	CFFProjectileRocket::CreateRocket( this, vecSrc, vecAngles, this, iSentryDamage, iSentryDamageRadius, 900.0f );

	EmitSound( "Sentry.RocketFire" );
	//DoRocketMuzzleFlash( ( m_bRocketLeftBarrel ? m_iRocketLAttachment : m_iRocketRAttachment ), vecSrc, vecAngles );

	// Rockets weren't being decremented
	m_iRockets--;

	// Flip which barrel to come out of next
	m_bRocketLeftBarrel = !m_bRocketLeftBarrel;
}

//-----------------------------------------------------------------------------
// Purpose: Bullet muzzle flash
//-----------------------------------------------------------------------------
void CFFSentryGun::DoMuzzleFlash( int iAttachment, const Vector& vecOrigin, const QAngle& vecAngles ) 
{
	VPROF_BUDGET( "CFFSentryGun::DoMuzzleFlash", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CEffectData data;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = iAttachment;
	data.m_vOrigin = vecOrigin;
	data.m_vAngles = vecAngles;
	//data.m_hEntity = this;
	data.m_fFlags = MUZZLEFLASH_TYPE_DEFAULT;
	DispatchEffect( "MuzzleFlash", data );
}

//-----------------------------------------------------------------------------
// Purpose: Rocket muzzle flash
//-----------------------------------------------------------------------------
void CFFSentryGun::DoRocketMuzzleFlash( int iAttachment, const Vector& vecOrigin, const QAngle& vecAngles ) 
{
	VPROF_BUDGET( "CFFSentryGun::DoRocketMuzzleFlash", VPROF_BUDGETGROUP_FF_BUILDABLE );

	CEffectData data;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = iAttachment;
	data.m_vOrigin = vecOrigin;
	data.m_vAngles = vecAngles;
	//data.m_hEntity = this;
	data.m_fFlags = MUZZLEFLASH_RPG;
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

	m_flStartLockTime = gpGlobals->curtime;

	EmitSound("Sentry.Spot");

	if ( GetEnemy() && GetEnemy()->Classify() == CLASS_PLAYER )
	{
		CSingleUserRecipientFilter user( ToBasePlayer( GetEnemy() ) );
		user.MakeReliable();

		UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE(FF_STATUSICON_LOCKEDON);
			WRITE_FLOAT(-1.0); //forever
		MessageEnd();
	}
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


// caes: limit angular acceleration of SG in yaw
	// find how much the SG wants to turn by
	float delta_yaw = new_yaw - cur_yaw;
	// allow for wrapping around
	if( delta_yaw > 180.0 )
	{
		delta_yaw -= 360.0;
	}
	else if( delta_yaw < -180.0 )
	{
		delta_yaw += 360.0;
	}

	// linearly decrease max accel as distance to enemy increases
	float accelYaw;
	if ( GetEnemy() )
		accelYaw = SG_ANGULAR_ACCEL_YAW / ( WorldSpaceCenter().DistTo( GetEnemy()->GetAbsOrigin() ) * SG_ACCELDISTANCEMULT );
	else
		accelYaw = SG_ANGULAR_ACCEL_YAW;

	// allow more accel if slowing down (friction)
	if( ( m_angSpeed_yaw < 0.0 && delta_yaw > m_angSpeed_yaw ) || ( m_angSpeed_yaw > 0.0 && delta_yaw < m_angSpeed_yaw ) )
		accelYaw = SG_ANGULAR_ACCEL_YAW * SG_ACCELFRICTIONMULT;

	// limit the amount it can turn according to its current angular speed and the max angular accel allowed
	if( delta_yaw > m_angSpeed_yaw + accelYaw )
	{
		delta_yaw = m_angSpeed_yaw + accelYaw;
	}
	else if( delta_yaw < m_angSpeed_yaw - accelYaw )
	{
		delta_yaw = m_angSpeed_yaw - accelYaw;
	}
	// update the angular speed for next time
	m_angSpeed_yaw = delta_yaw;
	// calc new aim angle and make sure it's in the range -180 < x <= 180
	new_yaw = cur_yaw + delta_yaw;
	if( new_yaw > 180.0 )
	{
		new_yaw -= 360.0;
	}
	else if( new_yaw <= -180.0 )
	{
		new_yaw += 360.0;
	}
// caes


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
	//if( SG_DEBUG && !engine->IsDedicatedServer()) 
	//{
	//	NDebugOverlay::Line(EyePosition(), EyePosition() + dir * 300.0f, 40, 40, 40, false, 0.05f);
	//	NDebugOverlay::Line(EyePosition(), EyePosition() + vecBaseUp * 300.0f, 110, 110, 110, false, 0.05f);
	//	NDebugOverlay::Line(EyePosition(), EyePosition() + cross * 300.0f, 180, 180, 180, false, 0.05f);
	//	NDebugOverlay::Line(EyePosition(), EyePosition() + fwd * 300.0f, 255, 255, 255, false, 0.05f);
	//}
#endif

	// Target pitch = constrained goal pitch - current pitch
	float new_pitch = UTIL_Approach( clamp( dst_pitch, SG_MIN_PITCH, SG_MAX_PITCH ), cur_pitch, MaxPitchSpeed() );


// caes: limit angular acceleration of SG in pitch
	// find how much the SG wants to turn by
	float delta_pitch = new_pitch - cur_pitch;

	// linearly decrease max accel as distance to enemy increases
	float accelPitch;
	if ( GetEnemy() )
		accelPitch = SG_ANGULAR_ACCEL_PITCH / ( WorldSpaceCenter().DistTo( GetEnemy()->GetAbsOrigin() ) * SG_ACCELDISTANCEMULT );
	else
		accelPitch = SG_ANGULAR_ACCEL_PITCH;

	// allow more accel if slowing down (friction)
	if( ( m_angSpeed_pitch < 0.0 && delta_pitch > m_angSpeed_pitch ) || ( m_angSpeed_pitch > 0.0 && delta_pitch < m_angSpeed_pitch ) )
		accelPitch = SG_ANGULAR_ACCEL_PITCH * SG_ACCELFRICTIONMULT;

	// limit the amount it can turn according to its current angular speed and the max angular accel allowed
	if( delta_pitch > m_angSpeed_pitch + accelPitch )
	{
		delta_pitch = m_angSpeed_pitch + accelPitch;
	}
	else if( delta_pitch < m_angSpeed_pitch - accelPitch )
	{
		delta_pitch = m_angSpeed_pitch - accelPitch;
	}
	// update the angular speed for next time
	m_angSpeed_pitch = delta_pitch;
	// calc new aim angle
	new_pitch = cur_pitch + delta_pitch;
	// check if we've hit the end stops
	if( new_pitch <= SG_MIN_PITCH )
	{
		new_pitch = SG_MIN_PITCH;
		m_angSpeed_pitch = 0.0;
	}
	else if( new_pitch >= SG_MAX_PITCH )
	{
		new_pitch = SG_MAX_PITCH;
		m_angSpeed_pitch = 0.0;
	}
// caes


	SetPoseParameter( m_iPitchPoseParameter, ( clamp( new_pitch, SG_MIN_ANIMATED_PITCH, SG_MAX_PITCH ) / 2.0f ) ); //AfterShock: (90 + 33) / 45.. bad bad hack, seems to work tho. sgs can only look down about 33 degrees.

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


	CBaseEntity *enemy = GetEnemy();

	if ( enemy && enemy->IsPlayer() )
	{
		CSingleUserRecipientFilter user( ToBasePlayer( enemy ) );
		user.MakeReliable();

		UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE(FF_STATUSICON_LOCKEDON);
			WRITE_FLOAT(0.0);
		MessageEnd();
	}

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

Vector CFFSentryGun::RocketPosition( void )
{
	VPROF_BUDGET( "CFFSentryGun::RocketPosition", VPROF_BUDGETGROUP_FF_BUILDABLE );

	Vector vecOrigin;
	QAngle vecAngles;

	// In case someone calls this on a non lvl3 sg
	vecOrigin.Init();

	if( m_bRocketLeftBarrel )
		GetAttachment( m_iRocketLAttachment, vecOrigin, vecAngles );
	else
		GetAttachment( m_iRocketRAttachment, vecOrigin, vecAngles );

	return vecOrigin;
}

//-----------------------------------------------------------------------------
// Purpose: Upgrade the SG
//-----------------------------------------------------------------------------
bool CFFSentryGun::Upgrade( bool bUpgradeLevel, int iCells, int iShells, int iRockets ) 
{
	VPROF_BUDGET( "CFFSentryGun::Update", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// Returns true if we upgrade a level
	bool bUpgraded = false;
	bool bRetval = false;

	if( bUpgradeLevel ) 
	{
		if( m_iLevel < 3 ) 
		{
			bUpgraded = true;
			bRetval = true;
			m_iLevel++;
		}

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
			m_iShellDamage = SG_BULLETDAMAGE;

			//m_flShellCycleTime = 0.2f;
			m_flShellCycleTime = SG_SHOTCYCLETIME_LVL1;

			m_iMaxHealth = SG_HEALTH_LEVEL1;
			m_iHealth = SG_HEALTH_LEVEL1;

			m_flLockTime = SG_LOCKONTIME_LVL1;
			//m_flTurnSpeed = 4.0f;
			m_flTurnSpeed = SG_TURNSPEED;

			break;

		case 2:
			SetModel( FF_SENTRYGUN_MODEL_LVL2 );
			SetSolid( SOLID_VPHYSICS );
			EmitSound( sndFilter, entindex(), "Sentry.Two" );

			m_iMaxShells = 125;
			m_iMaxRockets = 0;
			m_iShellDamage = SG_BULLETDAMAGE;

			//m_flShellCycleTime = 0.1f;
			m_flShellCycleTime = SG_SHOTCYCLETIME_LVL2;

			m_iMaxHealth = SG_HEALTH_LEVEL2;
			m_iHealth = SG_HEALTH_LEVEL2;

			m_flLockTime = SG_LOCKONTIME_LVL2;
			//m_flTurnSpeed = 7.0f;
			m_flTurnSpeed = SG_TURNSPEED;

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
			m_iShellDamage = SG_BULLETDAMAGE;

			//m_flShellCycleTime = 0.1f;
			m_flShellCycleTime = SG_SHOTCYCLETIME_LVL3;
			m_flRocketCycleTime = 3.0f;

			m_iMaxHealth = SG_HEALTH_LEVEL3;
			m_iHealth = SG_HEALTH_LEVEL3;

			m_flLockTime = SG_LOCKONTIME_LVL3;
			//m_flTurnSpeed = 7.0f;
			m_flTurnSpeed = SG_TURNSPEED;
			
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

/*
	if(bUpgraded)
	{
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "sentrygun_upgraded" );
		if( pEvent && m_hOwner.Get() )
		{
			CFFPlayer *pOwner = static_cast<CFFPlayer*>( m_hOwner.Get() );
			pEvent->SetInt( "userid", pOwner->GetUserID() );
			pEvent->SetInt( "level", m_iLevel );
			gameeventmanager->FireEvent( pEvent, true );
		}
	}
*/

	// Recalculate ammo percentage, 7 bits for shells + 1 bit for no rockets
	m_iAmmoPercent = 100.0f * (float)m_iShells / (float)m_iMaxShells;
	if( m_iMaxRockets && !m_iRockets ) 
		m_iAmmoPercent += 128;

	return bRetval;
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
	//if( SG_DEBUG && !engine->IsDedicatedServer()) 
	//	NDebugOverlay::Line( EyePosition(), origin, 255, 0, 255, false, 5.0f );
#endif

	CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
	if(pOwner)
	{
		Omnibot::Notify_SentryAimed(pOwner, this, dir);
	}
}

// How much damage should be taken from an emp explosion
int CFFSentryGun::TakeEmp( void ) 
{
	VPROF_BUDGET( "CFFSentryGun::TakeEmp", VPROF_BUDGETGROUP_FF_BUILDABLE );

	// This base damage matches up the total damage with tfc
	int ammodmg = SG_EMPDMG_BASE;

	// These values are from tfc.
	ammodmg += m_iShells * SG_EMPDMG_SHELLS_MULTI;
	ammodmg += m_iRockets * SG_EMPDMG_ROCKETS_MULTI;

	return ammodmg;
}

//-----------------------------------------------------------------------------
// Purpose: Sabotaged results in SG doing less damage (to simulate being
//			less accurate). Need to keep track of saboteur so that they can
//			trigger the malicious sabotage via their menu.
//-----------------------------------------------------------------------------
void CFFSentryGun::Sabotage(CFFPlayer *pSaboteur)
{
	VPROF_BUDGET( "CFFSentryGun::Sabotage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	if ( !pSaboteur )
		return;

	m_flSabotageTime = gpGlobals->curtime + FF_BUILD_SABOTAGE_TIMEOUT;
	m_hSaboteur = pSaboteur;
	m_iSaboteurTeamNumber = m_hSaboteur->GetTeamNumber();

	// AfterShock - scoring system: 100 points for sabotage SG
	pSaboteur->AddFortPoints(100, "#FF_FORTPOINTS_SABOTAGESG");

	Warning("SG sabotaged\n");
}

//-----------------------------------------------------------------------------
// Purpose: This turns the sentry on its own team for 10 seconds.
//			To differentiate between normal and malicious sabotage, we're
//			just going to set m_hSaboteur to NULL (cheap, I know)
//-----------------------------------------------------------------------------
void CFFSentryGun::MaliciouslySabotage(CFFPlayer *pSaboteur)
{
	VPROF_BUDGET( "CFFSentryGun::MaliciousSabotage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	BaseClass::MaliciouslySabotage( pSaboteur );

	if ( !pSaboteur )
		return;

	EmitSound( "Sentry.SabotageActivate" );

	// Cancel target so it searchs for a new (friendly one)
	if ( GetEnemy() && GetEnemy()->IsPlayer() )
	{
		// Tell player they aren't locked on any more, and remove the status icon
		CSingleUserRecipientFilter user( ToBasePlayer( GetEnemy() ) );
		user.MakeReliable();

		UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE(FF_STATUSICON_LOCKEDON);
			WRITE_FLOAT(0.0);
		MessageEnd();
	}
	SetEnemy(NULL);

	m_nSkin = clamp( pSaboteur->GetTeamNumber() - TEAM_BLUE, 0, 3 );

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
		if (m_hOwner.Get())
		{
			CFFPlayer *pOwner = static_cast<CFFPlayer*>(m_hOwner.Get());
			pEvent->SetInt("userid", pOwner->GetUserID());
			pEvent->SetInt("level", GetLevel());
			gameeventmanager->FireEvent(pEvent, true);
		}
	}

	CBaseEntity *enemy = GetEnemy();

	if ( enemy && enemy->IsPlayer() )
	{
		CSingleUserRecipientFilter user( ToBasePlayer( enemy ) );
		user.MakeReliable();

		UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE(FF_STATUSICON_LOCKEDON);
			WRITE_FLOAT(0.0);
		MessageEnd();
	}

	CFFBuildableObject::Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: Carry out the radius damage for this buildable
//-----------------------------------------------------------------------------
void CFFSentryGun::DoExplosionDamage()
{
	VPROF_BUDGET( "CFFSentryGun::DoExplosionDamage", VPROF_BUDGETGROUP_FF_BUILDABLE );

	float flDamage = SG_EXPLOSIONDAMAGE_BASE * m_iLevel  + (m_iRockets * 1.4f);
	// COmmented out for testing explosion damage - AfterShock
	//flDamage = min(280, flDamage);
	
	if (m_hOwner.Get())
	{
		CTakeDamageInfo info(this, m_hOwner, vec3_origin, GetAbsOrigin() + Vector(0, 0, 32.0f), flDamage, DMG_BLAST);
		info.SetCustomKill( KILLTYPE_SENTRYGUN_DET );
		RadiusDamage(info, GetAbsOrigin(), flDamage * 2.0f, CLASS_NONE, NULL);

		UTIL_ScreenShake(GetAbsOrigin(), flDamage * 0.0125f, 150.0f, m_flExplosionDuration, 620.0f, SHAKE_START);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Carry out the radius damage for this buildable
//-----------------------------------------------------------------------------
void CFFSentryGun::PhysicsSimulate()
{
	BaseClass::PhysicsSimulate();

	// Update the client every 0.2 seconds
	if (gpGlobals->curtime > m_flLastClientUpdate + 0.2f)
	{
		m_flLastClientUpdate = gpGlobals->curtime;

		CFFPlayer *pPlayer = dynamic_cast<CFFPlayer *> (m_hOwner.Get());

		if (!pPlayer)
			return;

		int iHealth = (int) (100.0f * GetHealth() / GetMaxHealth());
		int iAmmo = (int) (100.0f * (float) m_iShells / m_iMaxShells);

		// Last bit of ammo signifies whether the SG needs rockets
		if (m_iMaxRockets && !m_iRockets) 
			m_iAmmoPercent += 128;

		// If things haven't changed then do nothing more
		int iState = iHealth + (iAmmo << 8);
		if (m_iLastState == iState)
			return;

		CSingleUserRecipientFilter user(pPlayer);
		user.MakeReliable();

		UserMessageBegin(user, "SentryMsg");
		WRITE_BYTE(iHealth);
		WRITE_BYTE(iAmmo);
		WRITE_BYTE(GetLevel());
		MessageEnd();

		m_iLastState = iState;
	}
}