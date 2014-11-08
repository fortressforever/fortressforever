//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL1.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ff_player.h"
#include "ff_entity_system.h"	// Entity system
#include "ff_scriptman.h"
#include "ff_luacontext.h"
#include "ff_gamerules.h"
#include "ff_weapon_base.h"
#include "ff_weapon_baseclip.h"
#include "predicted_viewmodel.h"
#include "iservervehicle.h"
#include "viewport_panel_names.h"
#include "EntityFlame.h"
#include "rumble_shared.h"
#include "soundent.h"
#include "nav_mesh.h"

#include "ff_item_flag.h"
#include "ff_utils.h"
#include "ff_grenade_base.h"
#include "ff_buildableobjects_shared.h"
#include "ff_item_backpack.h"

#include "ff_team.h"			// team info
#include "in_buttons.h"			// for in_attack2
#include "ff_projectile_pipebomb.h"
#include "ff_grenade_emp.h"
#include "ff_lualib_constants.h"

#include "client.h"
#include "gib.h"
#include "omnibot_interface.h"
#include "te_effect_dispatch.h"

// added these so I could cast to check for grenades that are not derived from projectile base
// Could probably do it more cleanly but I just went with what was already in place.  -> Defrag
#include "ff_grenade_napalmlet.h"

extern int gEvilImpulse101;
#define FF_PLAYER_MODEL "models/player/demoman/demoman.mdl"

// Oh no a gobal.
int g_iLimbs[CLASS_CIVILIAN + 1][5] = { { 0 } };

#define PIPE_DET_DELAY 0.55 // this is mirrored in ff_projectile_pipebomb.cpp and ff_player_shared.cpp

// grenade information
//ConVar gren_timer("ffdev_gren_timer","3.81",0,"Timer length for all grenades.");
#define GREN_TIMER 3.81f
//ConVar gren_throw_delay("ffdev_throw_delay","0.5",0,"Delay before primed grenades can be thrown.");
#define GREN_THROW_DELAY 0.5f
//ConVar gren_speed("ffdev_gren_speed","500.0",0,"Speed grenades are thrown at.");
//ConVar gren_spawn_ang_x("ffdev_gren_spawn_ang_x","18.5",0,"X axis rotation grenades spawn at.");
#define GREN_SPAWN_ANG_X 18.5f
//ConVar gren_forward_offset("ffdev_gren_forward_offset","8",0,"Forward offset grenades spawn at in front of the player.");


#define BURN_TICK_INTERVAL 1.25f
//ConVar burn_damage_ic("ffdev_burn_damage_ic","7.0",0,"Burn damage of the Incendiary Cannon (per tick)");
//ConVar burn_damage_ng("ffdev_burn_damage_ng","7.0",0,"Burn damage of the Napalm Grenade (per tick)");
//ConVar burn_damage_ft("ffdev_burn_damage_ft","15.0",0,"Burn damage of the Flamethrower (per tick)");
#define BURN_DAMAGE_BASE 13.0f
//ConVar burn_ticks("ffdev_burn_ticks","6",0,"Number of burn ticks for pyro weapons.");
#define BURN_TICKS 6
//ConVar burn_multiplier_3burns("ffdev_burn_multiplier_3burns","5",0,"Burn damage multiplier for all 3 burn types.");
#define BURN_MULTIPLIER_3BURNS 5.0f
//ConVar burn_multiplier_2burns("ffdev_burn_multiplier_2burns","2.5",0,"Burn damage multiplier for 2 burn types.");
#define BURN_MULTIPLIER_2BURNS 2.0f

//ConVar ffdev_flamesize_burn1("ffdev_flamesize_burn1","0.015", FCVAR_FF_FFDEV_REPLICATED, "flame size multiplier for burn level 1");
#define FFDEV_FLAMESIZE_BURN1 0.015f //ffdev_flamesize_burn1.GetFloat()
//ConVar ffdev_flamesize_burn2("ffdev_flamesize_burn2","0.04", FCVAR_FF_FFDEV_REPLICATED, "flame size multiplier for burn level 2");
#define FFDEV_FLAMESIZE_BURN2 0.03f //ffdev_flamesize_burn2.GetFloat()
//ConVar ffdev_flamesize_burn3("ffdev_flamesize_burn3","0.055", FCVAR_FF_FFDEV_REPLICATED, "flame size multiplier for burn level 3");
#define FFDEV_FLAMESIZE_BURN3 0.045f //ffdev_flamesize_burn3.GetFloat()

// [integer] Max distance a player can be from us to be shown
//static ConVar radiotag_distance( "ffdev_radiotag_distance", "1024" );
#define RADIOTAG_DISTANCE 1024

// [float] Time between radio tag updates
//static ConVar radiotag_duration( "ffdev_radiotag_duration", "0.25" );
#define RADIOTAG_DURATION 0.25f

// [integer] Time in seconds you will stay 'tagged' once hit by a radio tag rifle round
//static ConVar radiotag_draw_duration( "ffdev_radiotag_draw_duration", "60" );
#define RADIOTAG_DRAW_DURATION 15

// [float] Time between updating the players location
//static ConVar location_update_frequency( "ffdev_location_update_frequency", "0.5" );
#define LOCATION_UPDATE_FREQUENCY 0.5f

//ConVar ffdev_changeclass_graceperiod( "ffdev_changeclass_graceperiod", "", "5", FCVAR_CHEATS | FCVAR_NOTIFY, "You can only change class once per grace period without getting a 5 second respawn delay" );
#define CHANGECLASS_GRACEPERIOD 5.0f

// status effect
//ConVar ffdev_infect_freq("ffdev_infect_freq","2",0,"Frequency (in seconds) a player loses health from an infection");
#define FFDEV_INFECT_FREQ 2.0f
//ConVar ffdev_infect_damage("ffdev_infect_damage","30",FCVAR_FF_FFDEV_REPLICATED,"Damage of the first infection tick (future ticks are modified by the infect_damagepertick_ vars)");
#define FFDEV_INFECT_DAMAGE 30.0f //ffdev_infect_damage.GetFloat()
//extern ConVar ffdev_infect_numticks; // in ff_player_shared.cpp
#define FFDEV_INFECT_NUMTICKS 10
//ConVar ffdev_infect_damagepertick_mult("ffdev_infect_damagepertick_mult","1",FCVAR_FF_FFDEV_REPLICATED,"Infection tick damage is multiplied by this number each tick");
#define FFDEV_INFECT_DAMAGEPERTICK_MULT 1 //ffdev_infect_damagepertick_mult.GetFloat()
//ConVar ffdev_infect_damagepertick_exp("ffdev_infect_damagepertick_exp",".8",FCVAR_FF_FFDEV_REPLICATED,"Infection tick damage is raised to this power each tick");
#define FFDEV_INFECT_DAMAGEPERTICK_EXP 0.8f //ffdev_infect_damagepertick_exp.GetFloat()
//ConVar ffdev_infect_regaindamage("ffdev_infect_regaindamage","10",FCVAR_FF_FFDEV_REPLICATED,"Amount of damage that is not regained after infection wears off");
#define FFDEV_INFECT_REGAINDAMAGE 10.0f //ffdev_infect_regaindamage.GetFloat()
//ConVar ffdev_regen_freq("ffdev_regen_freq","3",0,"Frequency (in seconds) a player loses health when a medic");
#define FFDEV_REGEN_FREQ 3.0f
//ConVar ffdev_regen_health("ffdev_regen_health","2",0,"Amount of health a player gains while a medic");
#define FFDEV_REGEN_HEALTH 2
//ConVar ffdev_regen_armor("ffdev_regen_armor","4",0,"Amount of armor a player gains while a engy");
#define FFDEV_REGEN_ARMOR 4
//ConVar ffdev_overhealth_freq("ffdev_overhealth_freq","3",0,"Frequency (in seconds) a player loses health when health > maxhealth");
#define FFDEV_OVERHEALTH_FREQ 3.0f

//ConVar ffdev_infection_time( "ffdev_infection_time", "14", FCVAR_FF_FFDEV_REPLICATED, "Amount of time for infection to last( in seconds )" );
#define FFDEV_INFECTION_TIME 14 //ffdev_infection_time.GetFloat()

//ConVar ffdev_immune_time( "ffdev_immune_time", "2", FCVAR_FF_FFDEV_REPLICATED, "Amount of time for immunity to last( in seconds )" );
#define FFDEV_IMMUNE_TIME 2 //ffdev_immune_time.GetFloat()

//ConVar ffdev_dmgforfullslow("ffdev_dmgforfullslow","90",FCVAR_FF_FFDEV_REPLICATED ,"When getting hit and player is moving above run speed, he gets slowed down in proportion to this damage");
#define FFDEV_DMGFORFULLSLOW 90.0f //ffdev_dmgforfullslow.GetFloat()

//ConVar ffdev_dmgforfullslow_sg("ffdev_dmgforfullslow_sg","90",FCVAR_FF_FFDEV_REPLICATED ,"When getting hit by a SG and player is moving above run speed, he gets slowed down in proportion to this damage");
#define FFDEV_DMGFORFULLSLOW_SG 90.0f //ffdev_dmgforfullslow_sg.GetFloat()

//static ConVar jerkmulti( "ffdev_concuss_jerkmulti", "0.0004", 0, "Amount to jerk view on conc" );
#define JERKMULTI 0.0004f

ConVar ffdev_gibdamage("ffdev_gibdamage", "50", FCVAR_FF_FFDEV_REPLICATED, "If a players health is -(ffdev_gibdamage's value) or less after death, then they will gib instead of ragdoll");
#define FFDEV_GIBDAMAGE ffdev_gibdamage.GetFloat()

extern ConVar sv_maxspeed;

//ConVar ffdev_spy_cloakfadespeed( "ffdev_spy_cloaktime", "1", FCVAR_REPLICATED, "Time it takes to cloak (fade out to cloak)" );
#define FFDEV_SPY_CLOAKFADESPEED 1.0f
//ConVar ffdev_spy_scloakfadespeed( "ffdev_spy_scloaktime", ".3", FCVAR_REPLICATED, "Time it takes to silent cloak (fade out to cloak)" );
#define FFDEV_SPY_SCLOAKFADESPEED 0.3f
//ConVar ffdev_spy_speedenforcewait( "ffdev_spy_speedenforcewait", "2", FCVAR_REPLICATED, "Time after cloaking a spys' speed will get enforced to the max cloak speed" );
#define FFDEV_SPY_SPEEDENFORCEWAIT 2.0f
//ConVar ffdev_spy_cloakzvel( "ffdev_spy_cloakzvel", "0.5", FCVAR_REPLICATED, "To tweak z factor of velocity when spy is cloaked" );
#define FFDEV_SPY_CLOAKZVEL 0.5f

//ConVar ffdev_gren_throwspeed( "ffdev_gren_throwspeed", "660", FCVAR_FF_FFDEV_REPLICATED );
#define GREN_THROWSPEED 660.0f

#ifdef _DEBUG
	// --------------------------------------------------------------------------------
	// Purpose: To spawn a model for testing - REMOVE (or disable) for release
	//
	// Bug #0000605: Rebo would like a way to spawn models on the fly in game to test stuff
	// --------------------------------------------------------------------------------
	class CFFModelTemp : public CBaseAnimating
	{
	public:
		DECLARE_CLASS( CFFModelTemp, CBaseAnimating );
		
		CFFModelTemp( void ) { m_hModel = NULL; }
		~CFFModelTemp( void ) {}

		const char *m_hModel;

		void Spawn( void )
		{
			if( m_hModel )
				PrecacheModel( m_hModel );
			BaseClass::Precache();
			if( m_hModel )
				SetModel( m_hModel );
		}

		static CFFModelTemp *Create( const char *szModel, const Vector& vecOrigin, const QAngle& vecAngles )
		{
			CFFModelTemp *pObject = ( CFFModelTemp * )CBaseEntity::Create( "ff_model_temp", vecOrigin, vecAngles );
			pObject->m_hModel = szModel;
			pObject->Spawn();
			return pObject;
		}
	};
	LINK_ENTITY_TO_CLASS( ff_model_temp, CFFModelTemp );

	CON_COMMAND( model_temp, "Spawn a temporary model. Usage: model_temp <model> <distance_in_front_of_player>" )
	{
		if( ( engine->Cmd_Argc() > 2 ) && ( engine->Cmd_Argc() < 4 ) )
		{
			CFFPlayer *pPlayer = ToFFPlayer( UTIL_GetCommandClient() );

			if( !pPlayer )
				return;

			Vector vecOrigin, vecForward, vecRight;
			
			// Get some info from the player...
			vecOrigin = pPlayer->GetAbsOrigin();
			pPlayer->EyeVectors( &vecForward, &vecRight );

			vecForward.z = 0;

			// Normalize
			VectorNormalize( vecForward );

			vecOrigin += ( vecForward * atof( engine->Cmd_Argv(2) ) );
			
			QAngle vecAngles;
			VectorAngles( vecForward, vecAngles );

			CFFModelTemp::Create( engine->Cmd_Argv(1), vecOrigin, vecAngles );
		}
		else
		{
			ClientPrint(UTIL_GetCommandClient(), HUD_PRINTCONSOLE, "Usage: model_temp <model> <distance_in_front_of_player>\n");
		}
	}
#endif

// --------------------------------------------------------------------------------
// Purpose: Kill the player and set a 5 second spawn delay
//			Stolen from client.cpp for easier interfacing w/ CFFPlayer
// --------------------------------------------------------------------------------
void CC_Player_Kill( void )
{
	CFFPlayer *pPlayer = ToFFPlayer( UTIL_GetCommandClient() );
	if (pPlayer)
	{
		// Don't kill if we're a spec or something
		if( pPlayer->GetTeamNumber() < TEAM_BLUE )
			return;

		// Bug #0000578: Suiciding using /kill doesn't cause a respawn delay
		if( pPlayer->IsAlive() )
		{
			CFFLuaSC hPlayerOnKill;
			hPlayerOnKill.Push(pPlayer);
			if(_scriptman.RunPredicates_LUA( NULL, &hPlayerOnKill, "player_onkill" ))
			{
				if(hPlayerOnKill.GetBool() == false)
					return;
			}

			pPlayer->SetRespawnDelay( 5.0f );

			// Bug #0000700: people with infection should give medic kill if they suicide
			if( pPlayer->IsInfected() && pPlayer->GetInfector() )
				pPlayer->SetSpecialInfectedDeath();

			ClientKill( pPlayer->edict() );

			// Call lua player_killed on suicides
			CFFLuaSC hPlayerKilled;
			hPlayerKilled.Push(pPlayer);
			_scriptman.RunPredicates_LUA( NULL, &hPlayerKilled, "player_killed" );
		}
	}
}
static ConCommand kill("kill", CC_Player_Kill, "kills the player");

// -------------------------------------------------------------------------------- //
// Player animation event. Sent to the client when a player fires, jumps, reloads, etc..
// -------------------------------------------------------------------------------- //

class CTEPlayerAnimEvent : public CBaseTempEntity
{
public:
	DECLARE_CLASS( CTEPlayerAnimEvent, CBaseTempEntity );
	DECLARE_SERVERCLASS();

					CTEPlayerAnimEvent( const char *name ) : CBaseTempEntity( name )
					{
					}

	CNetworkHandle( CBasePlayer, m_hPlayer );
	CNetworkVar( int, m_iEvent );
};

IMPLEMENT_SERVERCLASS_ST_NOBASE( CTEPlayerAnimEvent, DT_TEPlayerAnimEvent )
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropInt( SENDINFO( m_iEvent ), Q_log2( PLAYERANIMEVENT_COUNT ) + 1, SPROP_UNSIGNED ),
END_SEND_TABLE()

static CTEPlayerAnimEvent g_TEPlayerAnimEvent( "PlayerAnimEvent" );

void TE_PlayerAnimEvent( CBasePlayer *pPlayer, PlayerAnimEvent_t event )
{
	CPVSFilter filter( (const Vector&)pPlayer->EyePosition() );

	// Mirv: Don't send animation event to self
	filter.RemoveRecipient(pPlayer);
	
	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}

// --------------------------------------------------------------------------------
// Purpose: Team spawn class
// --------------------------------------------------------------------------------
class CFFTeamSpawn : public CPointEntity
{
public:
	DECLARE_CLASS( CFFTeamSpawn, CPointEntity );

	virtual Class_T Classify( void ) { return CLASS_TEAMSPAWN; }
};

LINK_ENTITY_TO_CLASS( info_ff_teamspawn , CFFTeamSpawn );

#ifdef EXTRA_LOCAL_ORIGIN_ACCURACY
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
static void *SendProxy_NonLocalOrigin(const SendProp *pProp, const void *pStruct, const void *pVarData, CSendProxyRecipients *pRecipients, int objectID)
{
	pRecipients->ClearRecipient(objectID - 1);
	return (void *) pVarData;
}
REGISTER_SEND_PROXY_NON_MODIFIED_POINTER(SendProxy_NonLocalOrigin);

BEGIN_SEND_TABLE_NOBASE(CBaseEntity, DT_NonLocalOrigin)
	SendPropVector(SENDINFO(m_vecOrigin), -1,  SPROP_COORD|SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT, SendProxy_Origin),
END_SEND_TABLE()
#endif

// -------------------------------------------------------------------------------- //
// Tables.
// -------------------------------------------------------------------------------- //
LINK_ENTITY_TO_CLASS( player, CFFPlayer );
PRECACHE_REGISTER(player);

BEGIN_SEND_TABLE_NOBASE( CFFPlayer, DT_FFPlayerObserver )
	SendPropFloat(SENDINFO(m_flNextClassSpecificSkill)),
	SendPropFloat(SENDINFO(m_flTrueAimTime)),
	SendPropFloat(SENDINFO(m_flHitTime)),
END_SEND_TABLE()

BEGIN_SEND_TABLE_NOBASE( CFFPlayer, DT_FFLocalPlayerExclusive )

#ifdef EXTRA_LOCAL_ORIGIN_ACCURACY
	SendPropVector(SENDINFO(m_vecOrigin), 32,  SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT),
#endif

	SendPropInt( SENDINFO( m_iShotsFired ), 8, SPROP_UNSIGNED ),

	// Buildables
	SendPropEHandle( SENDINFO( m_hDispenser ) ),
	SendPropEHandle( SENDINFO( m_hSentryGun ) ),
	SendPropEHandle( SENDINFO( m_hDetpack ) ),
	SendPropEHandle( SENDINFO( m_hManCannon ) ),
	SendPropBool( SENDINFO( m_bStaticBuilding ) ),
	SendPropBool( SENDINFO( m_bBuilding ) ),
	SendPropInt( SENDINFO( m_iCurBuild ), 3, SPROP_UNSIGNED ),

	// health/armor	
	SendPropInt(SENDINFO( m_iArmorType ), 4, SPROP_UNSIGNED ),

	// random player class 
	SendPropBool( SENDINFO ( m_fRandomPC ) ),

	SendPropInt(SENDINFO(m_iSkiState)),
	
	SendPropInt( SENDINFO( m_iLastSpyDisguise ), 8, SPROP_UNSIGNED ),   // AfterShock: this only uses the last 2 hex digits, bits 1-4 for team, 5-8 for class

	// Grenade Related
	SendPropInt( SENDINFO( m_iGrenadeState ), 2, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iPrimary ), 4 ),		// Not unsigned because can be -1
	SendPropInt( SENDINFO( m_iSecondary ), 4 ),
	SendPropFloat( SENDINFO( m_flServerPrimeTime ) ),

	// Map guide
	SendPropEHandle( SENDINFO( m_hNextMapGuide ) ),
	SendPropEHandle( SENDINFO( m_hLastMapGuide ) ),
	SendPropFloat( SENDINFO( m_flNextMapGuideTime ) ),

	SendPropFloat(SENDINFO(m_flSlidingTime)),
	SendPropFloat(SENDINFO(m_flSpeedModifier)),

	SendPropInt( SENDINFO( m_iSpyDisguising ), 4 ),

	// Radiotag information the local client needs to know
	SendPropEHandle( SENDINFO( m_hRadioTagData ) ),
	SendPropBool( SENDINFO( m_bCloakable ) ),
	SendPropBool( SENDINFO( m_bDisguisable ) ),
	SendPropQAngles( SENDINFO( m_vecInfoIntermission ), 13 ),
	// Entity at player's current objective (set by Lua)
	SendPropEHandle( SENDINFO( m_hObjectiveEntity ) ),
	// Location of player's current objective (also set by Lua)
	SendPropVector( SENDINFO( m_vecObjectiveOrigin ), SPROP_COORD ),
END_SEND_TABLE( )

IMPLEMENT_SERVERCLASS_ST( CFFPlayer, DT_FFPlayer )
	SendPropExclude( "DT_BaseAnimating", "m_flPoseParameter" ),
	SendPropExclude( "DT_BaseAnimating", "m_flPlaybackRate" ),	
	SendPropExclude( "DT_BaseAnimating", "m_nSequence" ),
	SendPropExclude( "DT_BaseEntity", "m_angRotation" ),
	SendPropExclude( "DT_BaseAnimatingOverlay", "overlay_vars" ),
	
	// playeranimstate and clientside animation takes care of these on the client
	SendPropExclude( "DT_ServerAnimationData" , "m_flCycle" ),	
	SendPropExclude( "DT_AnimTimeMustBeFirst" , "m_flAnimTime" ),

#ifdef EXTRA_LOCAL_ORIGIN_ACCURACY
	SendPropExclude( "DT_BaseEntity" , "m_vecOrigin" ),
#endif

	// Data that only gets sent to the local player.
	SendPropDataTable( "fflocaldata", 0, &REFERENCE_SEND_TABLE(DT_FFLocalPlayerExclusive), SendProxy_SendLocalDataTable ),

#ifdef EXTRA_LOCAL_ORIGIN_ACCURACY
	SendPropDataTable("fforigin", 0, &REFERENCE_SEND_TABLE(DT_NonLocalOrigin), SendProxy_NonLocalOrigin),
#endif
	
	// Data that only gets sent to the player as well as observers of the player
	SendPropDataTable( "ffplayerobserverdata", 0, &REFERENCE_SEND_TABLE(DT_FFPlayerObserver), SendProxy_OnlyToObservers ),

	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11 ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11 ),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),

	SendPropInt( SENDINFO( m_iClassStatus ), 4, SPROP_UNSIGNED ),   // AfterShock: this only uses the last hex digit i.e. 0x0000000F
	SendPropInt( SENDINFO( m_iSpyDisguise ), 8, SPROP_UNSIGNED ),   // AfterShock: this only uses the last 2 hex digits, bits 1-4 for team, 5-8 for class

	SendPropInt(SENDINFO(m_iSpawnInterpCounter), 4),
	
	SendPropFloat(SENDINFO(m_flConcTime)),

	SendPropBool( SENDINFO( m_bSaveMe ) ),
	SendPropBool( SENDINFO( m_bEngyMe ) ),
	SendPropBool( SENDINFO( m_bAmmoMe ) ),
	SendPropBool( SENDINFO( m_bConcussed ) ),
	SendPropBool( SENDINFO( m_bTranqed ) ),
	SendPropBool( SENDINFO( m_bSliding ) ),
	SendPropEHandle( SENDINFO( m_hActiveSlowfield ) ),
	SendPropBool( SENDINFO( m_bInfected ) ),
	SendPropBool( SENDINFO( m_bImmune ) ),
	SendPropInt( SENDINFO( m_iInfectTick ) ),
	SendPropInt( SENDINFO( m_iCloaked ), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iActiveSabotages ), 2, SPROP_UNSIGNED ),
END_SEND_TABLE( )

LINK_ENTITY_TO_CLASS( ff_ragdoll, CFFRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CFFRagdoll, DT_FFRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) ),

	// State of player's limbs
	SendPropInt(SENDINFO(m_fBodygroupState), 5, SPROP_UNSIGNED), //AfterShock: this uses 5 flags for limbs decapped, up to DECAP_RIGHT_LEG which is 1 << 4
	SendPropInt(SENDINFO(m_nSkinIndex), 3, SPROP_UNSIGNED),
END_SEND_TABLE()

// -------------------------------------------------------------------------------- //

void cc_CreatePredictionError_f()
{
	CBaseEntity *pEnt = CBaseEntity::Instance( 1 );
	pEnt->SetAbsOrigin( pEnt->GetAbsOrigin() + Vector( 63, 0, 0 ) );
}

ConCommand cc_CreatePredictionError( "CreatePredictionError", cc_CreatePredictionError_f, "Create a prediction error", FCVAR_CHEAT );

// --------------------------------------------------------------------------------
// Purpose: Constructor!
// --------------------------------------------------------------------------------
CFFPlayer::CFFPlayer()
{
#ifdef FF_BETA_TEST_COMPILE
	CBaseEntity *p = NULL;
	p->Activate();
#else
	m_PlayerAnimState = CreatePlayerAnimState( this, this, LEGANIM_9WAY, true );

	UseClientSideAnimation();
	m_angEyeAngles.Init();

	SetViewOffset( FF_PLAYER_VIEW_OFFSET );

	m_iLocalSkiState = 0;

	// Assume true
	m_bRespawnable = true;

	m_bStaticBuilding = false;
	m_bBuilding = false;
	//m_bCancelledBuild = false;
	m_iWantBuild = FF_BUILD_NONE;
	m_iCurBuild = FF_BUILD_NONE;
	m_hDispenser = NULL;
	m_hSentryGun = NULL;
	m_hDetpack = NULL;
	m_hManCannon = NULL;
	m_flBuildTime = 0.0f;

	m_bRadioTagged = false;
	m_flRadioTaggedStartTime = 0.0f;
	m_flRadioTaggedDuration = RADIOTAG_DRAW_DURATION;

	// Grenade Related
	m_iGrenadeState = FF_GREN_NONE;
	m_flServerPrimeTime = 0;
	m_bEngyGrenWarned = false;
	m_iPrimary = 0;
	m_iSecondary = 0;
	m_bWantToThrowGrenade = false;

	// Status Effects
	m_flNextBurnTick = 0.0;
	m_iBurnTicks = 0;
	m_flBurningDamage = 0.0;

	m_bBurnFlagNG = false; // AfterShock - burning flags for multiplying flames and damage for combos!
	m_bBurnFlagFT = false;
	m_bBurnFlagIC = false;

	m_bACDamageHint = true;  // For triggering the "Pyro takes damage from HWGuy" hint only once
	m_bSGDamageHint = true;	 // For triggering the "Spy takes damage from SG while cloaked" hint only once

	m_bDisguisable = true;

	m_flNextJumpTimeForDouble = 0;

	m_flSpeedModifier = 1.0f;
	m_flSpeedModifierOld = 1.0f;
	m_flSpeedModifierChangeTime = 0;

	for (int i=0; i<NUM_SPEED_EFFECTS; i++)
	{
		RemoveSpeedEffectByIndex( i );
	}
	m_fLastHealTick = 0.0f;
	m_fLastInfectedTick = 0.0f;
	m_bInfected = false;
	m_hInfector = NULL;
	m_bImmune = false;
	m_iInfectedTeam = TEAM_UNASSIGNED;
	m_flImmuneTime = 0.0f;
	m_iInfectTick = 0; //Green Mushy
	m_nNumInfectDamage = 0;
	m_flLastOverHealthTick = 0.0f;
	m_iActiveSabotages = 0;
	m_iSabotagedSentries = 0;
	m_iSabotagedDispensers = 0;
	
	m_iLastSpyDisguise = 0;

	// Map guide stuff
	m_hNextMapGuide = NULL;
	m_flNextMapGuideTime = 0;

	// Class stuff
	m_fRandomPC = false;
	m_iNextClass = 0;

	m_flConcTime = 0;		// Not concussed on creation
	m_bConcussed = false;
	
	m_flTrueAimTime	= 0.0f;
	m_flHitTime	= 0.0f;
	
	m_bTranqed = false;

	m_iClassStatus = 0;		// No class sorted yet

	m_flSlidingTime = 0;		// Not sliding on creation
	m_bSliding = false;

	m_bGassed = false;
	m_hGasser = NULL;
	m_flNextGas = 0;
	m_flGasTime = 0;

	m_hActiveSlowfield = NULL;

	m_Locations.Purge();
	m_iClientLocation = 0;

	SetObjectiveEntity(NULL);

	m_pBuildLastWeapon = NULL;

	m_flJumpTime = m_flFallTime = 0;

	m_iSpawnInterpCounter = 0;

	m_fl_LuaSet_PlayerRespawnDelay = 0.0f;

	m_flLastClassSwitch = 0.0f;

	m_SpawnPointOverride = 0;

	//m_iStatsID = -1;

#endif // FF_BETA_TEST_COMPILE
}

CFFPlayer::~CFFPlayer()
{
	m_PlayerAnimState->Release();
}

// --------------------------------------------------------------------------------
// Purpose: 
// --------------------------------------------------------------------------------
void CFFPlayer::UpdateOnRemove( void )
{
	// Kill off flame & burning sound
	Extinguish();

	BaseClass::UpdateOnRemove();
}

// --------------------------------------------------------------------------------
// Purpose: Set the spawn delay for a player. If the current delay
//			is longer than flDelay then flDelay is ignored and
//			the longer delay is used. It also checks the entity
//			system/mp_respawndelay for any global delay.
// --------------------------------------------------------------------------------
void CFFPlayer::SetRespawnDelay( float flDelay )
{
	// The largest delay is what the player will be told
	float flTmpDelay = max( flDelay, mp_respawndelay.GetFloat() );
	m_flNextSpawnDelay = max( flTmpDelay, m_fl_LuaSet_PlayerRespawnDelay );
}

CFFPlayer *CFFPlayer::CreatePlayer( const char *className, edict_t *ed )
{
	CFFPlayer::s_PlayerEdict = ed;
	return (CFFPlayer*)CreateEntityByName( className );
}

void CFFPlayer::LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles )
{
	BaseClass::LeaveVehicle( vecExitPoint, vecExitAngles );

	//teleport physics shadow too
	// Vector newPos = GetAbsOrigin();
	// QAngle newAng = GetAbsAngles();

	// Teleport( &newPos, &newAng, &vec3_origin );
}

void CFFPlayer::PreThink(void)
{
#ifdef FF_BETA_TEST_COMPILE
	// Crash
	CFFPlayer *p = NULL;
	p->PreThink();
#endif

#ifndef FF_BETA_TEST_COMPILE
	// reset these every frame
	m_fBodygroupState = 0;

	// Update networked current-speed-var for spy cloak mat proxy (sent to all players! :X)
	//Vector vecCloakVelocity = GetLocalVelocity();
	//m_flCloakSpeed = FastSqrt( ( vecCloakVelocity.x * vecCloakVelocity.x ) + 
	//	( vecCloakVelocity.y * vecCloakVelocity.y ) + 
	//	( ( vecCloakVelocity.z * ffdev_spy_cloakzvel.GetFloat() ) * ( vecCloakVelocity.z * ffdev_spy_cloakzvel.GetFloat() ) ) );

	// Has our radio tag expired?
	if( m_bRadioTagged && ( ( m_flRadioTaggedStartTime + m_flRadioTaggedDuration ) < gpGlobals->curtime ) )
	{
		m_bRadioTagged = false;
		m_pWhoTaggedMe = NULL;
	}

	// See if it's time to reset our saveme status
	if( ( m_flSaveMeTime < gpGlobals->curtime ) && ( m_bSaveMe ) )
		m_bSaveMe = false;

	// See if it's time to reset our engyme status
	if( ( m_flSaveMeTime < gpGlobals->curtime ) && ( m_bEngyMe ) )
		m_bEngyMe = false;

	// See if it's time to reset our engyme status
	if( ( m_flSaveMeTime < gpGlobals->curtime ) && ( m_bAmmoMe ) )
		m_bAmmoMe = false;

	// See if it's time to reset our concussion status
	if( ( m_flConcTime < gpGlobals->curtime ) && ( m_bConcussed ) )
		m_bConcussed = false;

	// Update our list of tagged players that the client
	FindRadioTaggedPlayers();

	// Riding a vehicle?
	if( IsInAVehicle() )	
	{
		// make sure we update the client, check for timed damage and update suit even if we are in a vehicle
		UpdateClientData();		
		CheckTimeBasedDamage();

		// Allow the suit to recharge when in the vehicle.
		CheckSuitUpdate();
		
		WaterMove();	
		return;
	}

	// update the grenade status if needed
	if( IsGrenadePrimed() )
		GrenadeThink();

	// Bug #0000459: building on ledge locks you into place.
	if( m_bBuilding )
	{
		// Our origin has changed while building! no!!!!!!!!!!!!!!!!!!!!!!
		if( m_bStaticBuilding && ( m_vecBuildOrigin.DistTo( GetAbsOrigin() ) > 128.0f ) )
		{
			Warning( "[Buildable] Player origin has changed!\n" );
			m_iWantBuild = m_iCurBuild;
			PreBuildGenericThink();
		}
		// Keeping the m_bBuilding line in there in case we cancel the build cause
		// we left the ground in which case m_bBuilding will be false and we'll
		// just skip this.
		// If we're building and we've waited the two seconds or whatever...
		if( m_bBuilding && ( m_flBuildTime < gpGlobals->curtime ) )
			PostBuildGenericThink();
	}

	StatusEffectsThink();	

	// Do some spy stuff
	if (GetClassSlot() == CLASS_SPY)
	{
		// Get horizontal + forward velocity (no vertical velocity)
		//Vector vecVelocity = GetLocalVelocity();
		//float flSpeed = FastSqrt( vecVelocity[ 0 ] * vecVelocity[ 0 ] + vecVelocity[ 1 ] * vecVelocity[ 1 ] );
		//float flSpeed = m_flCloakSpeed;

		// Jiggles: Nope, let's try it this way instead
		Vector vecCloakVelocity = GetLocalVelocity();
		float flSpeed = FastSqrt( ( vecCloakVelocity.x * vecCloakVelocity.x ) + 
		  ( vecCloakVelocity.y * vecCloakVelocity.y ) + 
		  ( ( vecCloakVelocity.z * FFDEV_SPY_CLOAKZVEL ) * ( vecCloakVelocity.z * FFDEV_SPY_CLOAKZVEL ) ) );

		// If going faster than spies walk speed, reset
		if( IsCloaked() && ( flSpeed > 220 /* ffdev_spy_maxcloakspeed */ ) )
		{
			// If it was a regular cloak, verify we haven't JUST cloaked
			// and are still within the ffdev_spy_speedenforcewait period

			bool bUncloak = false;
			
			// When this is false it's a regular cloak that got us cloaked
			if( !m_bCloakFadeType )
			{
				if( gpGlobals->curtime > ( m_flCloakTime +  FFDEV_SPY_SPEEDENFORCEWAIT ) )
					bUncloak = true;
			}
			else
			{
				// We silently cloaked and we're above the speed, uncloak!
				bUncloak = true;
			}

			// Uncloak NOW
			if( bUncloak )
				Uncloak( true );
		}

		// Disguising
		if (m_iNewSpyDisguise && gpGlobals->curtime > m_flFinishDisguise)
			FinishDisguise();

		// Sabotage!!
		SpySabotageThink();

		// Cloak fade thinking
		SpyCloakFadeThink();
	}

	// Do we need to do a class specific skill?
	if( m_afButtonPressed & IN_ATTACK2 )
		ClassSpecificSkill();

	else if (m_afButtonReleased & IN_ATTACK2)
		ClassSpecificSkill_Post();

	if (m_iSabotagedSentries == 0)
		m_iActiveSabotages &= ~2;
	if (m_iSabotagedDispensers == 0)
		m_iActiveSabotages &= ~1;

	BaseClass::PreThink();
#endif // FF_BETA_TEST_COMPILE
}

void CFFPlayer::PostThink()
{
#ifdef FF_BETA_TEST_COMPILE
	// Crash
	CFFPlayer *p = NULL;
	p->PostThink();
#endif

#ifndef FF_BETA_TEST_COMPILE
	BaseClass::PostThink();

	if( GetTeamNumber() < TEAM_BLUE )
	{
		MoveTowardsMapGuide();
	}
	else
	{
		QAngle angles = GetLocalAngles();
		angles[PITCH] = 0;
		SetLocalAngles( angles );

		// Store the eye angles pitch so the client can compute its animation state correctly.
		m_angEyeAngles = EyeAngles();

		m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );
	}
#endif // FF_BETA_TEST_COMPILE
}


void CFFPlayer::Precache()
{
#ifndef FF_BETA_TEST_COMPILE
	PrecacheModel(FF_PLAYER_MODEL);

	// #0000331: impulse 81 not working (weapon_cubemap)
	PrecacheModel("models/shadertest/envballs.mdl");

	// Quick addition to precache all the player models
	PrecacheModel("models/player/scout/scout.mdl");
	PrecacheModel("models/player/sniper/sniper.mdl");
	PrecacheModel("models/player/soldier/soldier.mdl");
	PrecacheModel("models/player/demoman/demoman.mdl");
	PrecacheModel("models/player/medic/medic.mdl");
	PrecacheModel("models/player/hwguy/hwguy.mdl");
	PrecacheModel("models/player/pyro/pyro.mdl");
	PrecacheModel("models/player/spy/spy.mdl");
	PrecacheModel("models/player/engineer/engineer.mdl");
	PrecacheModel("models/player/civilian/civilian.mdl");

	// Sounds
	//PrecacheScriptSound("Grenade.Timer");
	PrecacheScriptSound("Grenade.Prime");
	PrecacheScriptSound("Player.Jump");
	PrecacheScriptSound("Player.SpyFall");
	PrecacheScriptSound("Player.Deathbeep");
	PrecacheScriptSound("Player.Ammotoss");
	PrecacheScriptSound("speech.saveme");
	PrecacheScriptSound("Player.bodysplat");
	PrecacheScriptSound("Item.Toss");
	PrecacheScriptSound("Player.Pain");
	PrecacheScriptSound("Player.Scream");
	PrecacheScriptSound("Player.Flameout");
	PrecacheScriptSound("medical.saveme");
	PrecacheScriptSound("maintenance.saveme");
	PrecacheScriptSound("infected.saveme");
	PrecacheScriptSound("ammo.saveme");
	PrecacheScriptSound("overpressure.explode");
	
	// Precache gib sound -> Defrag
	PrecacheScriptSound("Player.Gib");

	// Special case
	PrecacheScriptSound( EMP_SOUND );

	// Precache sabotage sounds
	PrecacheScriptSound( "Player.Sabotage" );
	PrecacheScriptSound( "Player.SabotageTimedOut" );

	// Flashlight - Bug #0000679: flashlight sound isn't precached
	PrecacheScriptSound( "HL2Player.FlashLightOn" );
	PrecacheScriptSound( "HL2Player.FlashLightOff" );
	PrecacheModel( "sprites/glow01.vmt" );
	
	// Class specific things!
	for (int i = CLASS_SCOUT; i <= CLASS_CIVILIAN; i++)
	{
		char buf[256];
		const char *class_string = Class_IntToString(i);

		// Model gibs
		Q_snprintf(buf, 255, "models/player/%s/%s_head.mdl", class_string, class_string);
		g_iLimbs[i][0] = PrecacheModel(buf);

		Q_snprintf(buf, 255, "models/player/%s/%s_leftarm.mdl", class_string, class_string);
		g_iLimbs[i][1] = PrecacheModel(buf);

		Q_snprintf(buf, 255, "models/player/%s/%s_rightarm.mdl", class_string, class_string);
		g_iLimbs[i][2] = PrecacheModel(buf);

		Q_snprintf(buf, 255, "models/player/%s/%s_leftleg.mdl", class_string, class_string);
		g_iLimbs[i][3] = PrecacheModel(buf);

		Q_snprintf(buf, 255, "models/player/%s/%s_rightleg.mdl", class_string, class_string);
		g_iLimbs[i][4] = PrecacheModel(buf);

		// Saveme shouts
		// 0001318: Get rid of class specific save me sounds
		// Q_snprintf(buf, 255, "%s.saveme", class_string);
		// PrecacheScriptSound(buf);
	}

	for (int i = 0; i < 8; i++)
	{
		PrecacheModel(UTIL_VarArgs("models/gibs/gib%d.mdl", (i + 1)));
	}

	BaseClass::Precache();
#endif // FF_BETA_TEST_COMPILE
}

extern CBaseEntity *g_pLastSpawn; // this is defined somewhere.. i'm using it :)

// this is going to strictly be for the randomizing at the beginning of EntSelectSpawnPoint
// by starting from the last entity when going into the for loop
CBaseEntity *g_pLastSpawnRandomizer = NULL;

void CFFPlayer::SetLastSpawn( CBaseEntity *pEntity )
{
	g_pLastSpawn = pEntity;

	if( !pEntity )
		g_pLastSpawnRandomizer = NULL;
}

CBaseEntity *CFFPlayer::EntSelectSpawnPoint()
{
#ifdef FF_BETA_TEST_COMPILE
	// Return bogus crap
	return NULL;
#endif

#ifndef FF_BETA_TEST_COMPILE
	/*
	CBaseEntity *pSpot, *pGibSpot;
	edict_t		*player;

	player = edict();

	pSpot = g_pLastSpawn;
	// Randomize the start spot
	// NOTE: given a larger range from the default SDK function so that players won't always
	// spawn at the "first" spawn point for their team if the spawns are put in a "bad" order
	for ( int i = random->RandomInt(15,25); i > 0; i-- )
	{
		pSpot = gEntList.FindEntityByClassT( pSpot, CLASS_TEAMSPAWN );
		if ( !pSpot )  // skip over the null point
			pSpot = gEntList.FindEntityByClassT( pSpot, CLASS_TEAMSPAWN );
	}

	CBaseEntity *pFirstSpot = pSpot;
	pGibSpot = pFirstSpot;

	do 
	{
		if ( pSpot )
		{
			// check the scripting system to check if the player is allowed to spawn here
			// but let them spawn here if the name doesn't exist
			if ( !FStrEq(STRING(pSpot->GetEntityName()), "") )
			{
				//CFFLuaObjectWrapper hSpawnSpot;
				CFFLuaSC hAllowed( 1, this );
				if( entsys.RunPredicates_LUA( pSpot, &hAllowed, "validspawn" ) )
				{
					// pSpot:validspawn() exists, check the value returned from it
					if( !hAllowed.GetBool() )
					{
						//DevMsg("[entsys] Skipping spawn for player %s at %s because it fails the check\n", GetPlayerName(), STRING( pSpot->GetEntityName() ) );
						pSpot = gEntList.FindEntityByClassname( pSpot, "info_ff_teamspawn" );
						continue;
					}					
				}
			}
			pGibSpot = pSpot;

			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetLocalOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassT( pSpot, CLASS_TEAMSPAWN );
					continue;
				}

				// if so, go to pSpot
				goto ReturnSpot;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassT( pSpot, CLASS_TEAMSPAWN );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
	pSpot = pGibSpot;
	if ( pSpot )
	{
		CBaseEntity *pList[128];
		// Made minx/maxs be size of actual info_playerstart and included FL_FAKECLIENT for Dr Evil's bots
		int count = UTIL_EntitiesInBox( pList, 128, pSpot->GetAbsOrigin()-Vector( 16, 16, 35 ), pSpot->GetAbsOrigin()+Vector( 16, 16, 35 ), FL_CLIENT|FL_NPC|FL_FAKECLIENT );
		if ( count )
			//Iterate through the list and check the results
			for ( int i = 0; i < count; i++ )
			{
				CBaseEntity *ent = pList[i];
				if( ent )
				{
					if( ent->IsPlayer() )
					{
						// We were doing damage to dead players before
						if( ent->IsAlive() && ( ent->edict() != player ) )
							ent->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
					}					
				}
			}

			goto ReturnSpot;
	}

	// as a last resort, try to find an info_player_start
	//DevMsg("Spawning at info_player_start\n");
	pSpot = gEntList.FindEntityByClassT( NULL, CLASS_TEAMSPAWN );
	if ( pSpot ) 
		goto ReturnSpot;

ReturnSpot:
	if ( !pSpot  )
	{
		Warning( "PutClientInServer: no info_player_start on level\n");
		return CBaseEntity::Instance( INDEXENT( 0 ) );
	}

	//DevMsg("Spawning player %s at spawn '%s'\n", GetPlayerName(), STRING(pSpot->GetEntityName()));

	g_pLastSpawn = pSpot;
	return pSpot;
	*/

	// NOTE: below is some test code I was playing with. It's not perfect
	// yet and it does a UTIL_EntitiesInBox twice (eek?) but I want to commit
	// my shit before someone starts diffing, heh.
	//*
#ifdef _DEBUG
	Warning( "[EntSelectSpawnPoint] Looking for a spawn point!\n" );
#endif

	//////////////////////////////////////////////////////////////////////////
	// Shortcut for spawning bots where we want them.
	if(m_SpawnPointOverride)
		return m_SpawnPointOverride;
	//////////////////////////////////////////////////////////////////////////

// --> Jon: new spawn method

	CBaseEntity *pSpot = NULL, *pGibSpot = NULL, *pFirstSpot = NULL;

	CUtlVector<CBaseEntity*> spawns;
	spawns.EnsureCapacity( FFGameRules()->m_SpawnPoints.Count() );
	spawns.CopyArray( FFGameRules()->m_SpawnPoints.Base(), FFGameRules()->m_SpawnPoints.Count() );

	// loop until there are no more spawn points to loop through
	while( spawns.Count() > 0 )
	{
		// pick a random number
		int iRand = random->RandomInt(0, spawns.Count() - 1);

		// let's check this random spot
		pSpot = spawns[iRand];

		// Jon: this is just for testing purposes
		//Class_T classtype = pSpot->Classify();
		//if (classtype != CLASS_TEAMSPAWN)
		//	classtype = classtype;

		// as long as there's something to check, that is
		if(pSpot)
		{
			// initialize the first spot
			if (!pFirstSpot)
			{
				pFirstSpot = pSpot;
				pGibSpot = pFirstSpot;
			}

			// If we got a spawn spot and we're spec, it's valid
			if( GetTeamNumber() < TEAM_BLUE )
			{
				goto ReturnSpot;
			}

			// is this spot valid according to the game rules?
			if( FFGameRules()->IsSpawnPointValid( pSpot, ( CBasePlayer * )this ) )
			{
				// See if the spot is clear
				if( FFGameRules()->IsSpawnPointClear( pSpot, ( CBasePlayer * )this ) )
				{
					goto ReturnSpot;
				}
				else
				{
					// Not clear, so perhaps later we'll gib the guy here
					pGibSpot = pSpot;
				}
			}
		}

		// remove this spawn point from the list, reducing the count as well
		spawns.FastRemove(iRand);
	}

/*
	// I think this was one of the main sources of the not-so-randomness - Jon
	//pSpot = g_pLastSpawn;
	pSpot = g_pLastSpawnRandomizer;

	// Randomize the start spot
	// NOTE: given a larger range from the default SDK function so that players won't always
	// spawn at the "first" spawn point for their team if the spawns are put in a "bad" order
	// ANOTHER NOTE: This method sucks. - Jon
	// Also, raising it too much will make it skip over common "blocks" of valid teamspawns.
	for( int i = random->RandomInt(1,5); i > 0; i-- )
		pSpot = gEntList.FindEntityByClassT( pSpot, CLASS_TEAMSPAWN );

	// only have to do this when finally out of that for loop instead of every time inside it
	if( !pSpot )  // skip over the null point
		pSpot = gEntList.FindEntityByClassT( pSpot, CLASS_TEAMSPAWN );

	// See, we'll keep track of the last entity used during all this random stuff instead of
	// what's figured out down below, which sometimes gets back to the very first teamspawn.
	// But this still isn't the best solution, because what will probably happen is uh...
	// the "randomness" up there will get past a "block" of lua-based valid spawn points and just
	// get back to the end of the valid block thanks to the do-while loop below...ending up on the
	// same teamspawn that's always picked when the for loop up there goes past the valid block.
	// What we really need to do is collect all lua-based valid spawn points and randomly pick from those.
	// But my dick all hard, so I might do it tomorrow. - Jon
	g_pLastSpawnRandomizer = pSpot;

	CBaseEntity *pFirstSpot = pSpot;
	pGibSpot = pFirstSpot;

	do 
	{
		if( pSpot )
		{
			// If we got a spawn spot and we're spec, it's valid
			if( GetTeamNumber() < TEAM_BLUE )
			{
				goto ReturnSpot;
			}

			if( FFGameRules()->IsSpawnPointValid( pSpot, ( CBasePlayer * )this ) )
			{
				//Vector vecMins = Vector( -16, -16, 0 );
				//Vector vecMaxs = Vector( 16, 16, 72 );
				//Vector vecOrigin = pSpot->GetAbsOrigin();

				// See if the spot is clear
				if( FFGameRules()->IsSpawnPointClear( pSpot, ( CBasePlayer * )this ) )
				{
					goto ReturnSpot;
				}
				else
				{
					// Not clear, so perhaps later we'll gib the guy here
					pGibSpot = pSpot;
				}
			}
		}

		// Increment pSpot
		pSpot = gEntList.FindEntityByClassT( pSpot, CLASS_TEAMSPAWN );
	} 
	while( pSpot != pFirstSpot ); // loop if we're not back to the start
*/
	// <-- Jon: new spawn method

	// At this point, we've checked all ff specific team spawns. If we
	// have a gib spot then kill people in that spot. Otherwise, we'll
	// go and check info_player_starts.
	pSpot = pGibSpot;
	if( pSpot )
	{
// commenting this telefrag stuff to maybe move the player around this spawn location instead
/*
#ifdef _DEBUG
		Warning( "[EntSelectSpawnPoint] All spawns full, going to have to telefrag.\n" );
#endif

		CBaseEntity *pList[ 128 ];
		int count = UTIL_EntitiesInBox( pList, 128, pSpot->GetAbsOrigin() - Vector( 16, 16, 0 ), pSpot->GetAbsOrigin() + Vector( 16, 16, 72 ), FL_CLIENT | FL_NPC | FL_FAKECLIENT );
		if( count )
		{
			// Iterate through the list and check the results
			for( int i = 0; i < count; i++ )
			{
				CBaseEntity *ent = pList[ i ];
				if( ent )
				{
					if( ent->IsPlayer() )
					{
						if( ( ToFFPlayer( ent ) != this ) && ent->IsAlive() )
							ent->TakeDamage( CTakeDamageInfo( GetContainingEntity( INDEXENT( 0 ) ), GetContainingEntity( INDEXENT( 0 ) ), 6969, DMG_GENERIC ) );
					}
					else
					{
						// TODO: Remove objects - buildables/grenades/projectiles - on the spawn point?
					}
				}
			}
		}
*/
		goto ReturnSpot;
	}

#ifdef _DEBUG
	Warning( "[EntSelectSpawnPoint] Picking the info_player_start...\n" );
#endif

	// As a last resort, try to find an info_player_start
	pSpot = gEntList.FindEntityByClassname( NULL, "info_player_start");
	if( pSpot ) 
		goto ReturnSpot;

ReturnSpot:
	if( !pSpot  )
	{
		Warning( "PutClientInServer: no info_player_start on level\n");
		return CBaseEntity::Instance( INDEXENT( 0 ) );
	}
	else
	{
#ifdef _DEBUG
		Warning( "[EntSelectSpawnPoint] Got a valid spawn point!\n" );
#endif
	}

	g_pLastSpawn = pSpot;
	return pSpot;
	//*/
#endif // FF_BETA_TEST_COMPILE
}

// --------------------------------------------------------------------------------
// Purpose: This is called when players are forcibly respawned without being killed
//			to do some stuff that Event_Killed does (clear sounds, reset weapons, etc)
// --------------------------------------------------------------------------------
void CFFPlayer::PreForceSpawn( void )
{
	// If we're already dead, who cares
	if( IsAlive() )
	{
		RumbleEffect( RUMBLE_STOP_ALL, 0, RUMBLE_FLAGS_NONE );
		ClearUseEntity();

		if( GetClassSlot() == CLASS_SPY )
			SpyCloakFadeIn( true );

		Extinguish();

		// This client isn't going to be thinking for a while, so reset the sound until they respawn
		CSound *pSound = CSoundEnt::SoundPointerForIndex( CSoundEnt::ClientSoundIndex( edict() ) );
		if( pSound )
			pSound->Reset();

		if( m_bStaticBuilding )
		{
			CFFBuildableObject *pBuildable = GetBuildable( m_iCurBuild );
			if( pBuildable )
				pBuildable->Cancel();
			m_bBuilding = false;
			m_bStaticBuilding = false;
		}

		// Detonate player's pipes
		CFFProjectilePipebomb::DestroyAllPipes(this, true);

		// Release control of sabotaged structures
		SpySabotageRelease();		

		// Holster the current weapon
		if( GetActiveFFWeapon() )
			GetActiveFFWeapon()->Holster();

		SetActiveWeapon( NULL );

		// Remove all weapons
		for( int i = 0; i < MAX_WEAPONS; i++ )
		{
			if( m_hMyWeapons[ i ] )
			{
				m_hMyWeapons[ i ]->ForceRemove();
				m_hMyWeapons.Set( i, NULL );
			}
		}

		// clear out the suit message cache so we don't keep chattering
		SetSuitUpdate( NULL, false, 0 );

		// reset FOV
		//SetFOV( this, 0 );

		if( FlashlightIsOn() )
			FlashlightTurnOff();

		// only count alive players
		if( m_lastNavArea )
		{
			m_lastNavArea->DecrementPlayerCount( GetTeamNumber() );
			m_lastNavArea = NULL;
		}

		// Clear the deceased's sound channels.(may have been firing or reloading when killed)
		EmitSound( "BaseCombatCharacter.StopWeaponSounds" );
	}
}

// --------------------------------------------------------------------------------
// Purpose: Spawn!
// --------------------------------------------------------------------------------
void CFFPlayer::Spawn( void )
{
#ifdef FF_BETA_TEST_COMPILE
	// Crash
	CFFPlayer *p = NULL;
	p->Spawn();
#endif

#ifndef FF_BETA_TEST_COMPILE
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!
	//  Please don't reinitialise class specific variables in here, but SetupClassVariables instead!
	// !!!!!!!!!!!!!!!!!!!!!!!!!!!

	// First initalise a bunch of player data
	m_pWhoTaggedMe		= NULL;
	m_flNextBurnTick	= 0.0f;
	m_iBurnTicks		= 0.0f;
	m_flBurningDamage	= 0.0f;
	m_fLastHealTick		= 0.0f;
	m_fLastInfectedTick = 0.0f;
	m_bInfected			= false;
	m_hInfector			= NULL;
	m_bImmune			= false;
	m_iInfectedTeam		= TEAM_UNASSIGNED;
	m_hRagdoll			= NULL;
	m_flConcTime		= 0.0f;
	m_flSlidingTime		= 0.0f;
	m_flSpeedModifier	= 1.0f;
	m_flNextClassSpecificSkill = 0.0f;
	m_hActiveSlowfield	= NULL;
	
	m_flSpeedModifierOld		= 1.0f;
	m_flSpeedModifierChangeTime	= 0;

	m_iActiveSabotages = 0;
	m_iSabotagedSentries = 0;
	m_iSabotagedDispensers = 0;

	// If we get spawned, kill any primed grenades!
	m_flServerPrimeTime = 0.0f;
	m_iGrenadeState = FF_GREN_NONE;

	// Fixes water bug
	if (GetWaterLevel() == 3)
	{
		StopSound("Player.AmbientUnderWater");
		SetPlayerUnderwater(false);
		SetWaterLevel(0);
	}

	// Maybe this should go elsewhere
	CSingleUserRecipientFilter user((CBasePlayer *) this);
	user.MakeReliable();

	// This will finish all status icons
	UserMessageBegin(user, "StatusIconUpdate");
		WRITE_BYTE(FF_STATUSICON_MAX);
		WRITE_FLOAT(0.0);
	MessageEnd();

	// Get rid of any fire
	Extinguish();
	
	//AfterShock - flaginfo on spawn (connect doesnt work)
	CFFLuaSC hFlagInfo;
	hFlagInfo.Push(this);
	_scriptman.RunPredicates_LUA(NULL, &hFlagInfo, "flaginfo");

	// Tried to spawn while unassigned (and not in a map guide)
	// Bug #0001767 -- Improved camera stuff when speccing / changing teams / etc.
	if (GetTeamNumber() == TEAM_UNASSIGNED)
	{
		// Update camera stuff while unassigned
		UpdateCamera( true );

		// Show the team menu
		ShowViewPortPanel(PANEL_TEAM, true);

		// Don't allow them to spawn
		return;
	}

	// They tried to spawn with no class and they are not a spectator (or randompc)
	if (GetClassSlot() == 0 && GetTeamNumber() != TEAM_SPECTATOR && !m_fRandomPC)
	{
		// Set stuff we need to make sure the spec cam doesn't vibrate due to prediction errors
		// The way it now works is that when you switch teams, the player's view will move to the
		// intermission / mapguide / map origin rather than staying put.  As a result, we need to 
		// stop any glitching caused by switching between being a regular player and a pseudo-spectator.

		m_iHealth = 1;
		m_lifeState = LIFE_DEAD; 
		pl.deadflag = true;

		//m_afPhysicsFlags |= PFLAG_OBSERVER; 

		SetGroundEntity( (CBaseEntity *)NULL );
		RemoveFlag( FL_DUCKING );
		//SetSolidFlags( FSOLID_NOT_SOLID );
		
		// This is the biggie!
		SetMoveType( MOVETYPE_NONE );

		// Update camera stuff while assigned to a team
		UpdateCamera( false );

		// Don't allow them to spawn
		return;
	}

	// They have spawned as a spectator
	if (GetTeamNumber() == TEAM_SPECTATOR)
	{
		BaseClass::Spawn();

		// HACK: If you spawn as spectator without spawning as a class first then
		// you will end up with a physics object that goes mad if you try to noclip
		// through a wall. So we're just going to brute-destroy that here.
		VPhysicsDestroyObject();

		StartObserverMode(OBS_MODE_ROAMING);
		AddEffects(EF_NODRAW);

		// We have to do this after base spawn because it handly sets it to 0
		SetMaxSpeed(sv_maxspeed.GetFloat());

		// Dexter: if player has clientside model being drawn for deployable, destroy it
		RemoveAllItems( true );
		/*
		CFFWeaponBase *pWeapon = this->GetActiveFFWeapon();
		if(pWeapon)
		{
			int id = pWeapon->GetWeaponID();
			
			// its a buildable herfaderf
			if( id >= FF_WEAPON_DEPLOYDISPENSER && id <= FF_WEAPON_DEPLOYMANCANNON )
			{
				pWeapon->Cleanup();
			}
		}
		*/

		return;
	}

	// Handle spawn stuff in CBasePlayer::PlayerDeathThink

	// They're spawning as a player
	//if( gpGlobals->curtime < ( m_flDeathTime + m_flNextSpawnDelay ) )
	//	return;

	// m_flNextSpawnDelay used for "kill" command wait out
	// and also when need to force a player to spawn
	// after they've chosen team/class for first time
	// Reset spawn delay to 0
	//m_flNextSpawnDelay = 0.0f;

	// Run spawn delays through this function now. It will calculate
	// any LUA/cvar or personal spawn delay and return
	// the appropriate delay.
	SetRespawnDelay();

	// Activate their class stuff, die if we can't
	if (!ActivateClass())
		return;

	StopObserverMode();
	RemoveEffects( EF_NODRAW );

	BaseClass::Spawn();

	// Make sure they don't spawn ducked
	RemoveFlag(FL_DUCKING);

	SetModel( FF_PLAYER_MODEL );
	SetMoveType( MOVETYPE_WALK );
	RemoveSolidFlags( FSOLID_NOT_SOLID );

	SetupClassVariables();
	
	// equip the HEV suit
	EquipSuit();

	// Set on ground
	AddFlag(FL_ONGROUND);

	// Run this after SetupClassVariables in case lua is
	// manipulating the players' inventory
	CFFLuaSC hPlayerSpawn( 1, this );
	_scriptman.RunPredicates_LUA( NULL, &hPlayerSpawn, "player_spawn" );

	//////////////////////////////////////////////////////////////////////////
	while(true) // meh, cheat so i can use break;
	{
		const char *pDefaultWpn = 0;
		const char *pSpawnWpn = 0;
		switch(GetClassSlot())
		{
		case CLASS_SCOUT:
			pDefaultWpn = DEFAULTWEAPON_SCOUT;
			pSpawnWpn = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnweapon_scout");
			break;
		case CLASS_SNIPER:
			pDefaultWpn = DEFAULTWEAPON_SNIPER;
			pSpawnWpn = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnweapon_sniper");
			break;
		case CLASS_SOLDIER:
			pDefaultWpn = DEFAULTWEAPON_SOLDIER;
			pSpawnWpn = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnweapon_soldier");
			break;
		case CLASS_DEMOMAN:
			pDefaultWpn = DEFAULTWEAPON_DEMOMAN;
			pSpawnWpn = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnweapon_demoman");
			break;
		case CLASS_MEDIC:
			pDefaultWpn = DEFAULTWEAPON_MEDIC;
			pSpawnWpn = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnweapon_medic");
			break;
		case CLASS_HWGUY:
			pDefaultWpn = DEFAULTWEAPON_HWGUY;
			pSpawnWpn = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnweapon_hwguy");
			break;
		case CLASS_PYRO:
			pDefaultWpn = DEFAULTWEAPON_PYRO;
			pSpawnWpn = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnweapon_pyro");
			break;
		case CLASS_ENGINEER:
			pDefaultWpn = DEFAULTWEAPON_ENGINEER;
			pSpawnWpn = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnweapon_engineer");
			break;
		case CLASS_SPY:
			pDefaultWpn = DEFAULTWEAPON_SPY;
			pSpawnWpn = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnweapon_spy");
			break;
		case CLASS_CIVILIAN:
			pDefaultWpn = DEFAULTWEAPON_CIVILIAN;
			pSpawnWpn = engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnweapon_civilian");
			break;
		}

		enum { BufferSize = 128 };
		char weaponDefault[BufferSize] = {};
		char weaponSpawn[BufferSize] = {};
		Q_snprintf(weaponDefault, BufferSize, "ff_weapon_%s", pDefaultWpn?pDefaultWpn:"");
		Q_snprintf(weaponSpawn, BufferSize, "ff_weapon_%s", pSpawnWpn?pSpawnWpn:"");

		CBaseCombatWeapon *pSpawnWeapon = Weapon_OwnsThisType(weaponSpawn);
		if(pSpawnWpn && pSpawnWeapon)
		{
			if(Weapon_Switch(pSpawnWeapon))
				break;
		}

		pSpawnWeapon = Weapon_OwnsThisType(weaponDefault);
		if(pDefaultWpn && pSpawnWeapon)
		{
			if(Weapon_Switch(pSpawnWeapon))
				break;
		}

		break;
	}
	//////////////////////////////////////////////////////////////////////////
	
	// Make sure we don't go running around during the intermission
	// AfterShock - Commented out to try to prevent mapload respawn frozen
	/*
	if (FFGameRules()->IsIntermission())
		AddFlag(FL_FROZEN);
	else
		RemoveFlag(FL_FROZEN);
	*/
	// TODO: CHECK OUT WHY THE ABOVE ACTUALLY FIXED IT, WHY DID IT THINK WE'RE IN AN INTERMISSION?!

	// Increment the spawn counter
	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

#endif // FF_BETA_TEST_COMPILE
}

// Mirv: Moved all this out of spawn into here
void CFFPlayer::SetupClassVariables()
{
#ifndef FF_BETA_TEST_COMPILE
	// Reset Engineer stuff
	m_pBuildLastWeapon = NULL;

	//m_hSaveMe = NULL;
	m_flSaveMeTime = 0.0f;

	m_flPipebombShotTime = 0.0f;

	m_bSpecialInfectedDeath = false;

	m_flScreamTime = 0.0f;
	m_flMancannonTime = 0.0f;
	m_flMancannonDetTime = 0.0f;
	// Reset Spy stuff
	m_iCloaked = 0;
	m_flCloakTime = 0.0f;
	m_flNextCloak = 0.0f;
	m_flCloakFadeStart = 0.0f;
	m_flCloakFadeFinish = 0.0f;
	SetCloakable( true );
	m_bCloakFadeType = false; // assume regular cloaking and not silent coaking
	m_iCloakFadeCloaking = 0;
	// Not currently fading in or out
	m_bCloakFadeCloaking = false;
	m_flFinishDisguise = 0;
	m_iSpyDisguise = 0;
	m_iNewSpyDisguise = 0;
	m_iSpyDisguising = 0;
	m_flNextSpySabotageThink = 0.0f;
	m_flSpySabotageFinish = 0.0f;
	m_hSabotaging = NULL;

	m_Locations.Purge();
	m_iClientLocation = 0;

	SetObjectiveEntity(NULL);

	// Class system
	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	ClientPrint( this, HUD_PRINTNOTIFY, pPlayerClassInfo.m_szPrintName );

	m_iHealth		= pPlayerClassInfo.m_iHealth;
	m_iMaxHealth	= pPlayerClassInfo.m_iHealth;
	m_iArmor		= pPlayerClassInfo.m_iInitialArmour;
	m_iMaxArmor		= pPlayerClassInfo.m_iMaxArmour;
	m_iArmorType	= pPlayerClassInfo.m_iArmourType;
	m_iBaseArmorType = m_iArmorType;

	m_flMaxspeed	= pPlayerClassInfo.m_iSpeed;
	m_iPrimary		= pPlayerClassInfo.m_iPrimaryInitial;
	m_iSecondary	= pPlayerClassInfo.m_iSecondaryInitial;

	SetModel( pPlayerClassInfo.m_szModel );
	m_nSkin = GetTeamNumber() - FF_TEAM_BLUE;

	for ( int i = 0; i < pPlayerClassInfo.m_iNumWeapons; i++ )
		GiveNamedItem( pPlayerClassInfo.m_aWeapons[i] );
	// <-- Mirv: Various things to do on spawn

	// Makes life simpler later to store this in an array
	// TODO Use an array in playerclassparse instead	
	m_iMaxAmmo[GetAmmoDef()->Index(AMMO_CELLS)] = pPlayerClassInfo.m_iMaxCells;
	m_iMaxAmmo[GetAmmoDef()->Index(AMMO_NAILS)] = pPlayerClassInfo.m_iMaxNails;
	m_iMaxAmmo[GetAmmoDef()->Index(AMMO_SHELLS)] = pPlayerClassInfo.m_iMaxShells;
	m_iMaxAmmo[GetAmmoDef()->Index(AMMO_ROCKETS)] = pPlayerClassInfo.m_iMaxRockets;
	m_iMaxAmmo[GetAmmoDef()->Index(AMMO_DETPACK)] = pPlayerClassInfo.m_iMaxDetpack;
	m_iMaxAmmo[GetAmmoDef()->Index(AMMO_MANCANNON)] = pPlayerClassInfo.m_iMaxManCannon;

	// Can I get some freakin ammo please?
	// Maybe some sharks with freakin laser beams?
	for(int i = 0; i < pPlayerClassInfo.m_iNumAmmos; i++)
	{
		// UNDONE: per defrag and documentation update - give player the detpack ammo
		// just don't let them lay another detpack while one is out
		//		// Bug #0000452: Detpack is given to player after they've already used a detpack.
		//		if( m_hDetpack.Get() && !Q_strcmp( pPlayerClassInfo.m_aAmmos[i].m_szAmmoType, "AMMO_DETPACK" ) )
		//			continue;

		GiveAmmo(pPlayerClassInfo.m_aAmmos[i].m_iAmount, pPlayerClassInfo.m_aAmmos[i].m_szAmmoType, true);
	}

	ClearSpeedEffects();
#endif // FF_BETA_TEST_COMPILE
}

void CFFPlayer::InitialSpawn( void )
{
#ifndef FF_BETA_TEST_COMPILE
	// Make sure they are dead
	m_lifeState = LIFE_DEAD;
	pl.deadflag = true;

	m_Locations.Purge();
	m_iClientLocation = 0;

	SetObjectiveEntity(NULL);
	
	// Reset to 0
	m_vecInfoIntermission.Init();

	BaseClass::InitialSpawn();

	// Fixes the no model problem
	SetModel( FF_PLAYER_MODEL );

	// Set up their global voice channel
	m_iChannel = 0;

	m_hRadioTagData = ( CFFRadioTagData * )CreateEntityByName( "ff_radiotagdata" );
	Assert( m_hRadioTagData );
	m_hRadioTagData->Spawn();

	// Mulch: I'm wondering if there's a network delay in getting this value, so
	// lets try to get it right from the start so that later on we'll have the
	// value right away (in attempt to fix):
	// Bug #0001217: Instant death changing class to random pc even with cl_classautokill 0
	engine->GetClientConVarValue( engine->IndexOfEdict( edict() ), "cl_classautokill" );

	//DevMsg("CFFPlayer::InitialSpawn");
#endif // FF_BETA_TEST_COMPILE
}

//-----------------------------------------------------------------------------
extern void SendScriptChecksumToClient(CBasePlayer* pPlayer, unsigned long scriptCRC);

bool CFFPlayer::ClientCommand(const char *cmd)
{
	if(strcmp(cmd, "ff_scriptcrc") == 0)
	{
		unsigned long scriptCRC = _scriptman.GetScriptCRC();
		SendScriptChecksumToClient(this, scriptCRC);
		return true;
	}

	return BaseClass::ClientCommand(cmd);
}

//-----------------------------------------------------------------------------
// Purpose: This is the normal, noisey cloak. It used to have a check for
//			velocity but not anymore.
//-----------------------------------------------------------------------------
/*
void CFFPlayer::Command_SpyCloak( void )
{
	Warning( "[Cloak] [S] Time: %f\n", gpGlobals->curtime );

	// Just be on ground
	if (GetGroundEntity() == NULL)
		return;

	// A yell of pain
	if (!IsCloaked())
	{
		EmitSound( "Player.Death" );
	}

	Command_SpySilentCloak();
}
*/

//-----------------------------------------------------------------------------
// Purpose: This doesn't have a maximum speed either.
//			That wasn't really a description or a purpose, I know.
//-----------------------------------------------------------------------------
/*
void CFFPlayer::Command_SpySilentCloak( void )
{
	Warning( "[Silent Cloak] [S] Time: %f\n", gpGlobals->curtime );

	// Must be on ground
	if (GetGroundEntity() == NULL)
		return;

	// Already Cloaked so remove all effects
	if (IsCloaked())
	{
		ClientPrint( this, HUD_PRINTCENTER, "#FF_UNCLOAK" );

		// Yeah we're not Cloaked anymore bud
		m_iCloaked = 0;

		// If we're currently disguising, remove some time (50%)
		if( m_flFinishDisguise > gpGlobals->curtime )
			m_flFinishDisguise -= ( m_flFinishDisguise - gpGlobals->curtime ) * 0.5f;

		CFFRagdoll *pRagdoll = dynamic_cast<CFFRagdoll *> (m_hRagdoll.Get());

		// Remove the ragdoll instantly
		pRagdoll->SetThink(&CBaseEntity::SUB_Remove);
		pRagdoll->SetNextThink(gpGlobals->curtime);

		// Visible and able to move again
		RemoveEffects(EF_NODRAW);
		//RemoveFlag(FL_FROZEN);

		// Redeploy our weapon
		if (GetActiveWeapon() && GetActiveWeapon()->IsWeaponVisible() == false)
		{
			GetActiveWeapon()->Deploy();
			ShowCrosshair(true);
		}

		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent("uncloaked");						
		if(pEvent)
		{
			pEvent->SetInt("userid", this->GetUserID());
			gameeventmanager->FireEvent(pEvent, true);
		}
	}
	// Not already cloakned, so collapse with ragdoll
	else
	{
		ClientPrint( this, HUD_PRINTCENTER, "#FF_CLOAK" );

		m_iCloaked = 1;

		// If we're currently disguising, add on some time (50%)
		if( m_flFinishDisguise > gpGlobals->curtime )
			m_flFinishDisguise += ( m_flFinishDisguise - gpGlobals->curtime ) * 0.5f;

		// Create our ragdoll using this function (we could just c&p it and modify it i guess)
		CreateRagdollEntity();

		CFFRagdoll *pRagdoll = dynamic_cast<CFFRagdoll *> (m_hRagdoll.Get());

		// Make a few things to the ragdoll, such as stopping it from travelling any
		// further and don't allow it to fade out. Actually this velocity change
		// won't work because the client will use the velocity of its local 
		// representation of this player. Dang.
		pRagdoll->m_vecRagdollVelocity = Vector(0, 0, 0);
		pRagdoll->SetThink(NULL);

		// Invisible and unable to move
		AddEffects(EF_NODRAW);
		//AddFlag(FL_FROZEN);

		// Holster our current weapon
		if (GetActiveWeapon())
			GetActiveWeapon()->Holster(NULL);

		CFFLuaSC hOwnerCloak( 1, this );
		// Find any items that we are in control of and let them know we Cloaked
		CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByOwnerAndClassT( NULL, ( CBaseEntity * )this, CLASS_INFOSCRIPT );
		while( pEnt != NULL )
		{
			// Tell the ent that it Cloaked
			_scriptman.RunPredicates_LUA( pEnt, &hOwnerCloak, "onownercloak" );

			// Next!
			pEnt = (CFFInfoScript*)gEntList.FindEntityByOwnerAndClassT( pEnt, ( CBaseEntity * )this, CLASS_INFOSCRIPT );
		}

		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent("cloaked");						
		if(pEvent)
		{
			pEvent->SetInt("userid", this->GetUserID());
			gameeventmanager->FireEvent(pEvent, true);
		}
	}
}
*/

void CFFPlayer::Event_Killed( const CTakeDamageInfo &info )
{
	if( m_bSaveMe )
		m_bSaveMe = false;

	if( m_bEngyMe )
		m_bEngyMe = false;
	
	if( m_bAmmoMe )
		m_bAmmoMe = false;

	if( m_bConcussed )
		m_bConcussed = false;

	if( m_bTranqed )
		m_bTranqed = false;
	
	if( m_bSliding )
		m_bSliding = false;

	m_flSaveMeTime = 0.0f;

	if( GetClassSlot() == CLASS_SPY )
		SpyCloakFadeIn( true );

	// TODO: Take SGs into account here?
	CFFPlayer *pKiller = ToFFPlayer(dynamic_cast<CMultiplayRules *>(g_pGameRules)->GetDeathScorer( info.GetAttacker(), info.GetInflictor() ));

	if ( info.GetInflictor() )
	{
		switch ( info.GetInflictor()->Classify() )
		{
			// Sends a hint to a player killed by an EMP
			case CLASS_GREN_EMP:
				FF_SendHint( this, GLOBAL_EMPDEATH, 3, PRIORITY_NORMAL, "#FF_HINT_GLOBAL_EMPDEATH" );
				break;

			// Sends a hint explaining how to use grenades
			case CLASS_GREN:
			case CLASS_GREN_NAIL:
			case CLASS_GREN_MIRV:
			case CLASS_GREN_MIRVLET:
			case CLASS_GREN_NAPALM:
			case CLASS_GREN_GAS:
				FF_SendHint( this, GLOBAL_GRENDEATH, 3, PRIORITY_NORMAL, "#FF_HINT_GLOBAL_GRENDEATH" );
				break;
		}    
	}

	// Drop any grenades
	if (m_iGrenadeState != FF_GREN_NONE)
	{
		ThrowGrenade(GREN_TIMER - (gpGlobals->curtime - m_flServerPrimeTime), 0.0f);
		m_iGrenadeState = FF_GREN_NONE;
		m_flServerPrimeTime = 0;
		m_bEngyGrenWarned = false;
	}
	
	/* Legshot scoring code - unfinished (ignore)
	if(( IsSpeedEffectSet( SE_LEGSHOT ); ) && (g_pGameRules->PlayerRelationship(this, pKiller) != GR_TEAMMATE)) 
	{
		if( m_pWhoTaggedMe != NULL )
		{
			CFFPlayer *pTagger = GetPlayerWhoTaggedMe();
			CFFPlayer *pKiller2 = ToFFPlayer( pKiller );
			// AfterShock - scoring system: 100 points for anyone fragging your radiotagged player
			// No points if you kill your own radiomarked target
			if ( !(pTagger == pKiller2) )
				if( pKiller2 )
					pKiller2->AddScore( 100, true );
		}
	}
	*/

	ClearSpeedEffects();
	RemoveFlag(FL_FROZEN);

	// a bunch of these, will set up a better way of doing this soon
	CSingleUserRecipientFilter user(this);
	user.MakeReliable();

	// Reset all the view effects under our controller
	UserMessageBegin(user, "FFViewEffect");
	WRITE_BYTE(FF_VIEWEFFECT_MAX);
	MessageEnd();

	// reset their status effects
	m_flNextBurnTick = 0.0;
	m_iBurnTicks = 0;
	m_flBurningDamage = 0.0;
	
	m_bBurnFlagNG = false;
	m_bBurnFlagIC = false;
	m_bBurnFlagFT = false;

	for (int i = 0; i < NUM_SPEED_EFFECTS; i++)
	{
		RemoveSpeedEffectByIndex( i );
	}

	m_fLastHealTick = 0.0f;
	m_fLastInfectedTick = 0.0f;

	// Stop infection
	m_bInfected = false;

	m_iActiveSabotages = 0;
	m_iSabotagedSentries = 0;
	m_iSabotagedDispensers = 0;

	//stop gas
	m_bGassed = false;
	m_flNextGas = 0;
	m_flGasTime = 0;
	m_hGasser = NULL;

	// Beg; Added by Mulchman
	if( m_bStaticBuilding )
	{
		CFFBuildableObject *pBuildable = GetBuildable( m_iCurBuild );
		if( pBuildable )
				pBuildable->Cancel();

		// Unlock the player if he/she got locked
		UnlockPlayer( );

		// Re-initialize
		m_bStaticBuilding = false;
		m_bBuilding = false;
		m_iCurBuild = FF_BUILD_NONE;
		m_iWantBuild = FF_BUILD_NONE;

		// Mirv: Cancel build timer
		CSingleUserRecipientFilter user(this);
		user.MakeReliable();
		UserMessageBegin(user, "FF_BuildTimer");
		WRITE_SHORT(m_iCurBuild);
		WRITE_FLOAT(0);
		MessageEnd();
	}

	// If the tag is still active, award a point
	// to the person who tagged us
	// (So long as you havent been TKed - AfterShock)
	if(( m_bRadioTagged ) && (g_pGameRules->PlayerRelationship(this, pKiller) != GR_TEAMMATE)) 
	{
		if( m_pWhoTaggedMe != NULL )
		{
			CFFPlayer *pTagger = GetPlayerWhoTaggedMe();
			CFFPlayer *pKiller2 = ToFFPlayer( pKiller );
			// AfterShock - scoring system: If the dead player was radio tagged, award 100 points to the tagging player
			// Do not award points to the tagger if the tagger is also the killer.
			if ( !(pTagger == pKiller2) )
			{
				// fixed typo (wasn't compiling).  Also made above comment a bit less ambiguous -> Defrag
				if( pTagger )
					pTagger->AddFortPoints( 50, "#FF_FORTPOINTS_TEAMMATERADIOTAGKILL" );
			}
		}
	}

	// Reset this
	m_pWhoTaggedMe = NULL;

	// Reset the tag...
	m_bRadioTagged = false;

	// EDIT: Let's not use strings if we don't have to...
	// Find any items that we are in control of and drop them
	CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( NULL, CLASS_INFOSCRIPT );

	while( pEnt != NULL )
	{
		// Tell the ent that it died
		pEnt->OnOwnerDied( this );

		// Next!
		pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( pEnt, CLASS_INFOSCRIPT );
	}

	// Kill infection sound

	StopSound( "Player.DrownContinue" );

	// Stop the saveme sounds upon death
	StopSound( "infected.saveme" );
	StopSound( "medical.saveme" );
	StopSound( "maintenance.saveme" );
	StopSound( "ammo.saveme" );

	// --> Mirv: Create backpack moved here to stop crash
	CFFItemBackpack *pBackpack = (CFFItemBackpack *) CBaseEntity::Create( "ff_item_backpack", GetAbsOrigin(), GetAbsAngles());

	if (pBackpack)
	{
		pBackpack->SetOwnerEntity( this );
		pBackpack->SetSpawnFlags(SF_NORESPAWN);

		Vector vel = GetAbsVelocity();

		if (vel.z < 1.0f)
			vel.z = 1.0f;

		float velmag = vel.Length();

		// Bug #0000243: Deaths by explosion send the dropped ammo bag flying unrealistically far
		if (velmag > 300.0f)
			vel *= 300.0f / velmag;

		pBackpack->SetAbsVelocity(vel);
		pBackpack->SetAbsOrigin(GetAbsOrigin());

		// AfterShock: Players always drop the same ammo now

		//for (int i = 1; i < MAX_AMMO_SLOTS; i++)
		//	pBackpack->SetAmmoCount(i, GetAmmoCount(i));

		pBackpack->SetAmmoCount( GetAmmoDef()->Index( AMMO_ROCKETS ), 20 );
		pBackpack->SetAmmoCount( GetAmmoDef()->Index( AMMO_SHELLS ), 100 );
		pBackpack->SetAmmoCount( GetAmmoDef()->Index( AMMO_NAILS ), 100 );
		pBackpack->SetAmmoCount( GetAmmoDef()->Index( AMMO_CELLS ), 30 );

	}
	// <-- Mirv: Create backpack moved here to stop crash

	// Note: since we're dead, it won't draw us on the client, but we don't set EF_NODRAW
	// because we still want to transmit to the clients in our PVS.

	// Detonate player's pipes
	CFFProjectilePipebomb::DestroyAllPipes(this, true);

	// Release control of sabotaged structures
	SpySabotageRelease();

	//prevent weapons from being picked up when dropped (they should get deleted properly later so do not delete them here)
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if ( m_hMyWeapons[i] )
			m_hMyWeapons[i]->SetTouch( NULL ); //touching other peoples private parts is naughty.
	}

	if (!ShouldGib(info))
	{
		CreateRagdollEntity(&info);
		CreateLimbs(m_fBodygroupState);
	}

	// Get rid of fire (after gibbing!)
	Extinguish();

	BaseClass::Event_Killed( info );
}

//-----------------------------------------------------------------------------
// Purpose: This is handled instead by CreateRagdollEntity()!
//-----------------------------------------------------------------------------
bool CFFPlayer::BecomeRagdollOnClient( const Vector &force )
{
	return true;
}

void CFFPlayer::CreateRagdollEntity(const CTakeDamageInfo *info)
{
	// Let's not create ragdolls until we're actually playing
	if( GetTeamNumber() < TEAM_BLUE )
		return;

	// If we already have a ragdoll, don't make another one.
	CFFRagdoll *pRagdoll = dynamic_cast< CFFRagdoll* >( m_hRagdoll.Get() );

	if ( !pRagdoll )
	{
		// create a new one
		pRagdoll = dynamic_cast< CFFRagdoll* >( CreateEntityByName( "ff_ragdoll" ) );

		pRagdoll->m_hPlayer = this;
	}
	else
	{
		// If our ragdoll is already on fire extinguish it
		CEntityFlame *pPrevRagdollFlame = dynamic_cast< CEntityFlame * >( pRagdoll->GetEffectEntity() );
		if( pPrevRagdollFlame )
			pPrevRagdollFlame->Extinguish();
	}

	pRagdoll->m_fBodygroupState = m_fBodygroupState; // Allows decapitation
	pRagdoll->m_nSkinIndex = m_nSkin;
	pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
	pRagdoll->m_nModelIndex = m_nModelIndex;
	pRagdoll->m_nForceBone = m_nForceBone;

	//get flame entity of the player
	CEntityFlame *pPlayerFlame = dynamic_cast< CEntityFlame * >( GetEffectEntity() );
	
	//if the player has flames
	if( pPlayerFlame )
	{
		//set players effect entity to none
		SetEffectEntity( NULL );

		pPlayerFlame->Extinguish();

		CEntityFlame *pRagdollFlame = CEntityFlame::Create(pRagdoll, true, 0.8f);

		//attach the flame to the ragdoll instead
		pRagdollFlame->AttachToEntity( pRagdoll );

		//set the ragdoll's effect entity to the flame
		pRagdoll->SetEffectEntity( pRagdollFlame );

		//set the same lifetime the same as the ragdoll itself!
		pRagdollFlame->SetLifetime( 15.0f );
	}

	// not everything that gets here has an info
	// when we change class we dont have an inflictor either
	if ( info ) 
	{
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_vecForce = info->GetDamageForce();
	}
	else
	{
		//use whatever the players velocity was
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_vecForce = Vector(0, 0, 0);
	}

	// remove the ragdoll after a time
	pRagdoll->SetNextThink( gpGlobals->curtime + 15.0f );
	pRagdoll->SetThink( &CBaseEntity::SUB_Remove );

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

void CFFPlayer::DoAnimationEvent( PlayerAnimEvent_t event )
{
	//Check for jimmyleg cases on the jump event(and speed) -GreenMushy
	if( Q_atoi( engine->GetClientConVarValue( this->entindex(), "cl_jimmyleg_mode" ) ) != 0 && event == PLAYERANIMEVENT_JUMP )
	{
		//check convars or speed!
		Vector vecVelocity = this->GetAbsVelocity();
		int nSpeed = (int) FastSqrt( vecVelocity.x * vecVelocity.x + vecVelocity.y * vecVelocity.y );
		float jcap = (Q_atof( engine->GetClientConVarValue( this->entindex(), "cl_jimmyleg_cap" ) ) );

		switch( (Q_atoi( engine->GetClientConVarValue( this->entindex(), "cl_jimmyleg_mode" ) ) ) )
		{
		case 1:
		case 2:
			//If the player is below their (specified) bunnyhop cap, do a jump animation.
			if( (float)nSpeed < ( this->MaxSpeed() * jcap ) )
			{
				m_PlayerAnimState->DoAnimationEvent( event );
			}
			break;
		default:
			//Always do the animation event if previous stuff wasnt hit
			m_PlayerAnimState->DoAnimationEvent( event );
			break;
		}
	}
	else
	{
		m_PlayerAnimState->DoAnimationEvent( event );
	}
	TE_PlayerAnimEvent( this, event );	// Send to any clients who can see this guy.
}

CFFWeaponBase* CFFPlayer::GetActiveFFWeapon() const
{
	return dynamic_cast< CFFWeaponBase* >( GetActiveWeapon() );
}

void CFFPlayer::CreateViewModel( int index /*=0*/ )
{
	Assert( index >= 0 && index < MAX_VIEWMODELS );

	if ( GetViewModel( index ) )
		return;

	CPredictedViewModel *vm = ( CPredictedViewModel * )CreateEntityByName( "predicted_viewmodel" );
	if ( vm )
	{
		vm->SetAbsOrigin( GetAbsOrigin() );
		vm->SetOwner( this );
		vm->SetIndex( index );
		DispatchSpawn( vm );
		vm->FollowEntity( this, false );
		m_hViewModel.Set( index, vm );
	}
}

void CFFPlayer::CheatImpulseCommands( int iImpulse )
{
	if (iImpulse != 101)
	{
		BaseClass::CheatImpulseCommands( iImpulse );
		return ;
	}

	if(sv_cheats->GetBool() && GetTeamNumber() >= TEAM_BLUE && GetTeamNumber() <= TEAM_GREEN)
	{
		GiveAmmo(300, AMMO_NAILS);
		GiveAmmo(300, AMMO_SHELLS);
		GiveAmmo(300, AMMO_ROCKETS);
		GiveAmmo(300, AMMO_CELLS);
		GiveAmmo(300, AMMO_DETPACK);
		GiveAmmo(300, AMMO_MANCANNON);
		AddPrimaryGrenades( 4 );
		AddSecondaryGrenades( 4 );
		SetHealth(m_iMaxHealth);
		m_iArmor = m_iMaxArmor;
	}
}

bool CFFPlayer::PlayerHasSkillCommand(const char *szCommand)
{
	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	for ( int i = 0; i < pPlayerClassInfo.m_iNumSkills; i++ )
	{
		if( strcmp( pPlayerClassInfo.m_aSkills[i], szCommand ) == 0 )
			return true;
	}

	return false;
}

//Set a player's objective
void CFFPlayer::SetObjectiveEntity( const CBaseEntity *pEntity )
{
	if ( m_hObjectiveEntity )
	{
		m_hObjectiveEntity->m_ObjectivePlayerRefs.FindAndRemove(this);
		m_hObjectiveEntity->DispatchUpdateTransmitState();
	}

	m_hObjectiveEntity = pEntity;

	if ( m_hObjectiveEntity )
	{
		SetObjectiveOrigin( m_hObjectiveEntity->GetAbsOrigin() );
		m_hObjectiveEntity->m_ObjectivePlayerRefs.AddToHead(this);
		m_hObjectiveEntity->DispatchUpdateTransmitState();
	}
	else
	{
		SetObjectiveOrigin( Vector( 0, 0, INVALID_OBJECTIVE_LOCATION ) );
	}
}

//Set a player's location.
void CFFPlayer::SetLocation( int entindex, const char *szNewLocation, int iNewLocationTeam )
{
	//Same as last one? Don't add additional head entries.
	if(m_Locations.Count() && m_Locations[0].entindex == entindex)
		return;

	LocationInfo info;
	info.entindex = entindex;
	Q_strcpy( info.locationname, szNewLocation );
	info.team = iNewLocationTeam;

	m_Locations.AddToHead(info);

	if(m_iClientLocation != entindex)
	{
		CSingleUserRecipientFilter filter( this );
		filter.MakeReliable();	// added
		UserMessageBegin( filter, "SetPlayerLocation" );
		WRITE_STRING( GetLocation() );
		WRITE_SHORT( GetLocationTeam() ); // changed
		MessageEnd();

		m_iClientLocation = entindex;
		Q_strncpy( m_szLastLocation, GetLocation(), sizeof( m_szLastLocation ) );
		m_iLastLocationTeam = GetLocationTeam();
	}
}

//Remove players location.
void CFFPlayer::RemoveLocation( int entindex )
{
	if(entindex <= 0)
		return;

	for ( int i = 0; i < m_Locations.Count(); i++ )
		if(m_Locations[i].entindex == entindex)
			m_Locations.Remove(i);

	//Update location if we have too.
	if(m_Locations.Count() > 0 
		&& m_iClientLocation != m_Locations[0].entindex)
	{
		CSingleUserRecipientFilter filter( this );
		filter.MakeReliable();	// added
		UserMessageBegin( filter, "SetPlayerLocation" );
		WRITE_STRING( GetLocation() );
		WRITE_SHORT( GetLocationTeam() - 1 ); // changed
		MessageEnd();

		m_iClientLocation = m_Locations[0].entindex;
		Q_strncpy( m_szLastLocation, GetLocation(), sizeof( m_szLastLocation ) );
		m_iLastLocationTeam = GetLocationTeam() - 1;
	}
}

void CFFPlayer::Command_TestCommand(void)
{
	ClientPrint(UTIL_GetCommandClient(), HUD_PRINTCONSOLE, "Inside Command_TestCommand for player %s\n", GetPlayerName());
	ClientPrint(UTIL_GetCommandClient(), HUD_PRINTCONSOLE, "Full command line on server was: %s\n", engine->Cmd_Args());
}

void CFFPlayer::Command_MapGuide( void )
{
	// Swap them to spectator class if needed
	if (GetTeamNumber() != TEAM_SPECTATOR)
	{
		KillAndRemoveItems();
		ChangeTeam(TEAM_SPECTATOR);
		Spawn();
	}

	// Start at specified mapguide
	if (engine->Cmd_Argc() > 1)
		m_hLastMapGuide = m_hNextMapGuide = FindMapGuide(MAKE_STRING(engine->Cmd_Argv(1)));
	else
		m_hLastMapGuide = m_hNextMapGuide = FindMapGuide(MAKE_STRING("overview"));

	// Check if mapguide was found
	if (m_hNextMapGuide)
	{
		m_flNextMapGuideTime = 0;
		ClientPrint(this, HUD_PRINTCONSOLE, "Starting, teleporting to guide 0\n");
	}
	else
		ClientPrint(this, HUD_PRINTCONSOLE, "Couldn't start, unable to find first map guide\n");
}

// Set the voice comm channel
void CFFPlayer::Command_SetChannel( void )
{
	if( engine->Cmd_Argc( ) < 2 )
	{
		// no channel specified
		return;
	}

	int iChannel = atoi( engine->Cmd_Argv(1) );

	if( iChannel < 0 || iChannel > 2 )
		return;

	m_iChannel = iChannel;
}

int CFFPlayer::GetClassSlot() const
{
/*	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	return pPlayerClassInfo.m_iSlot;*/

	return GetClassForClient();
}

// When spawned we this is called to handle class changing
int CFFPlayer::ActivateClass()
{
	CFFTeam *pTeam = GetGlobalFFTeam(GetTeamNumber());

	// This shouldn't be called while not on a team
	if (!pTeam)
		return 0;

	// Don't always have to call this
	if (m_iNextClass == GetClassSlot() && !m_fRandomPC)
		return GetClassSlot();

	// If we're being unassigned then go for it.
	if (m_iNextClass == 0 && !m_fRandomPC)
	{
		SetClassForClient(0);
		return 0;
	}

	// Make a random choice for randompcs
	if (m_fRandomPC)
	{
		m_iNextClass = UTIL_PickRandomClass(GetTeamNumber());
	}

	// Check target class is still available
	else if (m_iNextClass != GetClassSlot())
	{
		char cClassesAvailable[10];
		int nClassesAvailable = UTIL_GetClassSpaces(GetTeamNumber(), cClassesAvailable);
		nClassesAvailable;

		int iClassIndex = m_iNextClass - CLASS_SCOUT;
		Assert(iClassIndex >= 0 && iClassIndex <= 10);

		// This class is no longer available, slow-mo
		if (cClassesAvailable[iClassIndex] <= 0)
		{
			m_iNextClass = GetClassSlot();
			ClientPrint(this, HUD_PRINTNOTIFY, "#FF_ERROR_NOLONGERAVAILABLE");
			return GetClassSlot();
		}
	}

	// Do we still need the availability check here? I'm pretty sure we don't.

	// Now lets try reading this class, this shouldn't fail!
	if (!ReadPlayerClassDataFromFileForSlot(filesystem, Class_IntToString(m_iNextClass), &m_hPlayerClassFileInfo, GetEncryptionKey()))
	{
		//if (m_iNextClass != 0)
		AssertMsg(0, "Unable to read class script file, this shouldn't happen");
		return GetClassSlot();
	}

	// Send a player class change event. - finished below!
	IGameEvent *pEvent = gameeventmanager->CreateEvent("player_changeclass");
	if (pEvent)
	{
		pEvent->SetInt("userid", GetUserID());
		pEvent->SetInt("oldclass", GetClassForClient());
	}

	const CFFPlayerClassInfo &pNewPlayerClassInfo = GetFFClassData();
	SetClassForClient(pNewPlayerClassInfo.m_iSlot);

	// finish the class change event - AFTER WE'VE SPAWNED!
	if (pEvent)
	{
		pEvent->SetInt("newclass", GetClassForClient());
		gameeventmanager->FireEvent(pEvent, true);
	}

	// Display our class information
	ClientPrint(this, HUD_PRINTNOTIFY, pNewPlayerClassInfo.m_szPrintName);
	ClientPrint(this, HUD_PRINTNOTIFY, pNewPlayerClassInfo.m_szDescription);

	// Remove all buildable items from the game
	RemoveItems();

	// So the client can keep track
	SetClassForClient(m_iNextClass);


	return GetClassSlot();
}

void CFFPlayer::ChangeTeam(int iTeamNum)
{
	// clear the random player class flag, so player doesnt
	// immediately spawn when changing classes
	m_fRandomPC = false;

	// if we're changing to/from spec, count it as a class change so people can't use that to avoid the suicide respawn delay
	if (iTeamNum == FF_TEAM_SPEC || GetTeamNumber() == FF_TEAM_SPEC)
		m_flLastClassSwitch = gpGlobals->curtime;

	BaseClass::ChangeTeam(iTeamNum);
}

void CFFPlayer::ChangeClass(const char *szNewClassName)
{
	//const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();
	CFFTeam *pTeam = GetGlobalFFTeam( GetTeamNumber() );

	// This shouldn't happen as class changes only allowed while on a team
	if( !pTeam )
		return;

	bool fInstantSwitch = strcmp(engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_classautokill"), "0") != 0;
	bool bWasRandom = m_fRandomPC;

	// They are picking the randompc slot
	if( FStrEq( szNewClassName, "randompc" ) )
	{
		m_fRandomPC = true;

		// #0001554: Random player class doesn't work unless you've already spawned as a 'normal' class -> Defrag
		// Moved KillAndRemoveItems() outside the if( fInstantSwitch ) as this was causing the player to be unable to spawn
		// unless either A: autokill was on B: the player had already played a different class 

		// Edited again because it was killing you instantly even if autokill was off and you chose random PC
		bool bAlive = IsAlive();			

		// This one handles spectators joining a new team
		if( ! bAlive )
		{
			// if we're changing class twice in 5 seconds, then add a respawn delay as if they typed kill in console
			if (gpGlobals->curtime - m_flLastClassSwitch <= CHANGECLASS_GRACEPERIOD)
			{
				SetRespawnDelay( CHANGECLASS_GRACEPERIOD - (gpGlobals->curtime - m_flLastClassSwitch) );
			}

			m_flLastClassSwitch = gpGlobals->curtime;

			// Need to run KillAndRemoveItems() to set the DeathThink properly so you can actually spawn
            KillAndRemoveItems();
			return;
		}
		
		if( fInstantSwitch )
		{
			// if we're changing class twice in 5 seconds, then add a respawn delay as if they typed kill in console
			if (gpGlobals->curtime - m_flLastClassSwitch <= CHANGECLASS_GRACEPERIOD)
			{
				SetRespawnDelay( CHANGECLASS_GRACEPERIOD - (gpGlobals->curtime - m_flLastClassSwitch) );
			}

			m_flLastClassSwitch = gpGlobals->curtime;

			KillAndRemoveItems();
			if( bAlive && (GetClassSlot() != 0) )
			{
				CFFLuaSC hPlayerKilled;
				hPlayerKilled.Push(this);
				_scriptman.RunPredicates_LUA( NULL, &hPlayerKilled, "player_killed" );
			}
		}

		return;
	}
	else
		m_fRandomPC = false;	

	int iClass = Class_StringToInt( szNewClassName );

	// Check that they picked a valid class
	if( !iClass )
	{
		ClientPrint( this, HUD_PRINTNOTIFY, "#FF_ERROR_NOSUCHCLASS" );
		return;
	}

	// Check that they aren't already this class
	//if( Q_strcmp( pPlayerClassInfo.m_szClassName, Class_IntToString( iClass ) ) == 0 )
	//	return;

	// If they're already this class, no class change needed. Just inform the player.
	if (iClass == m_iNextClass && !bWasRandom)
	{
		ClientPrint(this, HUD_PRINTNOTIFY, "#FF_CHANGECLASS_LATER", Class_IntToString(iClass));
		return;
	}

	int iAlreadyThisClass = 0;

    // Count the number of people of this class
	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CFFPlayer *pPlayer = (CFFPlayer *) UTIL_PlayerByIndex( i );

		if( pPlayer && pPlayer->GetTeamNumber() == GetTeamNumber() && pPlayer->GetClassSlot() == iClass )
			iAlreadyThisClass++;
	}

	int class_limit = pTeam->GetClassLimit( iClass );

	// Class is disabled
	if( class_limit == -1 )
	{
		ClientPrint( this, HUD_PRINTNOTIFY, "#FF_ERROR_DISABLED" );
		return;
	}

	// Not enough space for this class
	if( class_limit > 0 && iAlreadyThisClass >= class_limit )
	{
		ClientPrint( this, HUD_PRINTNOTIFY, "#FF_ERROR_NOSPACE" );
		return;
	}
	
	// check if Lua wants to allow it
	CFFLuaSC hPlayerSwitchClass;
	hPlayerSwitchClass.Push(this);
	hPlayerSwitchClass.Push(GetClassSlot());
	hPlayerSwitchClass.Push(iClass);
	if(_scriptman.RunPredicates_LUA( NULL, &hPlayerSwitchClass, "player_switchclass" ))
	{
		if(hPlayerSwitchClass.GetBool() == false)
		{
			ClientPrint( this, HUD_PRINTNOTIFY, "#FF_ERROR_CANTSWITCHCLASS" );
			return;
		}
	}

	// Yup it is okay to change
	// We don't change class instantly, only when we spawn
	m_iNextClass = iClass;

	// Now just need a way to select which one you want
	if (fInstantSwitch || GetClassSlot() == 0)
	{
		bool bAlive = IsAlive();
		
		// if we're changing class twice in 5 seconds, then add a respawn delay as if they typed kill in console
		if (gpGlobals->curtime - m_flLastClassSwitch <= CHANGECLASS_GRACEPERIOD)
		{
			SetRespawnDelay( CHANGECLASS_GRACEPERIOD - (gpGlobals->curtime - m_flLastClassSwitch) );
		}

		m_flLastClassSwitch = gpGlobals->curtime;

		// But for now we do have instant switching
		KillAndRemoveItems();

		if( bAlive && (GetClassSlot() != 0) )
		{
			CFFLuaSC hPlayerKilled;
			hPlayerKilled.Push(this);
			_scriptman.RunPredicates_LUA( NULL, &hPlayerKilled, "player_killed" );
		}		

		// Should call ActivateClass right afterwards too, since otherwise they might
		// miss out on getting their class because somebody else took it during the 5sec
		// death wait
		ActivateClass();
	}
	else
	{
		// They didn't want to kill themselves, so let them know they're changing
		ClientPrint(this, HUD_PRINTNOTIFY, "#FF_CHANGECLASS_LATER", Class_IntToString(iClass));
	}

	// These next things are kind of lame... but when doing bot training
	// maps the player automatically spawns as a team and a class... so
	// we need to kill the vgui menus otherwise it looks bad being spawned
	// and ready to go and all the stuff is still on your screen

	// MULCH: We no longer have a motd panel!
	// Hide motd panel if it's showing
	//ShowViewPortPanel( PANEL_INFO, false );

	// Hide team panel if it's showing
	ShowViewPortPanel( PANEL_TEAM, false );

	// Hide class panel if it's showing
	ShowViewPortPanel( PANEL_CLASS, false );
}

void CFFPlayer::Command_Class(void)
{
	if(engine->Cmd_Argc() < 1)
	{
		// no class specified
		return;
	}

	//DevMsg("CFFPlayer::Command_Class || Changing class to '%s'\n", engine->Cmd_Argv(1));
	ChangeClass(engine->Cmd_Argv(1));
}

void CFFPlayer::Command_Team( void )
{
	int iOldTeam = GetTeamNumber();

	if( engine->Cmd_Argc( ) < 1 )
	{
		// no team specified
		return;
	}

	int iTeam = 0;
	int iTeamNumbers[8] = {0};

    // Count the number of people each team
	for( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CFFPlayer *pPlayer = (CFFPlayer *) UTIL_PlayerByIndex( i );

		if( pPlayer )
			iTeamNumbers[pPlayer->GetTeamNumber()]++;
	}

	// Case insensitive compares for now
	if( Q_stricmp( engine->Cmd_Argv( 1 ), "spec" ) == 0 )
		iTeam = FF_TEAM_SPEC;
	else if( Q_stricmp( engine->Cmd_Argv( 1 ), "blue" ) == 0 )
		iTeam = FF_TEAM_BLUE;
	else if( Q_stricmp( engine->Cmd_Argv( 1 ), "red" ) == 0 )
		iTeam = FF_TEAM_RED;
	else if( Q_stricmp( engine->Cmd_Argv( 1 ), "yellow" ) == 0 )
		iTeam = FF_TEAM_YELLOW;
	else if( Q_stricmp( engine->Cmd_Argv( 1 ), "green" ) == 0 )
		iTeam = FF_TEAM_GREEN;

	// Pick the team with least capacity to join
	else if( Q_stricmp( engine->Cmd_Argv( 1 ), "auto" ) == 0 )
	{
		int iBestTeam = UTIL_PickRandomTeam();

		// Couldn't find a valid team to join (because of limits)
		if( iBestTeam < 0 )
		{
			ClientPrint( this, HUD_PRINTNOTIFY, "#FF_ERROR_NOFREETEAM" );
			return;
		}

		// This is our team then bud
		iTeam = iBestTeam;
	}

	// Couldn't find a team afterall
	if( iTeam == 0 )
	{
		ClientPrint( this, HUD_PRINTNOTIFY, "#FF_ERROR_NOSUCHTEAM" );
		return;
	}

	// Check the team that was picked has space
	CFFTeam *pTeam = GetGlobalFFTeam(iTeam);

	// This should stop teams being picked which aren't active
	if( !pTeam )
	{
		ClientPrint( this, HUD_PRINTNOTIFY, "#FF_ERROR_NOSUCHTEAM" );
		return;
	}

	// Check the team isn't full
	if( pTeam->GetTeamLimits() != 0 && iTeamNumbers[iTeam] >= pTeam->GetTeamLimits() )
	{
		ClientPrint( this, HUD_PRINTNOTIFY, "#FF_ERROR_TEAMFULL" );
		return;
	}

	// [DEBUG] Check what team was picked
	//DevMsg( "%d\n", iTeam );

	// Are we already this team
	if( GetTeamNumber() == iTeam )
	{
		ClientPrint( this, HUD_PRINTNOTIFY, "#FF_ERROR_ALREADYONTHISTEAM" );
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	CFFLuaSC hPlayerSwitchTeam;
	hPlayerSwitchTeam.Push(this);
	hPlayerSwitchTeam.Push(GetTeamNumber());
	hPlayerSwitchTeam.Push(iTeam);
	if(_scriptman.RunPredicates_LUA( NULL, &hPlayerSwitchTeam, "player_switchteam" ))
	{
		if(hPlayerSwitchTeam.GetBool() == false)
		{
			ClientPrint( this, HUD_PRINTNOTIFY, "#FF_ERROR_SWITCHTOOSOON" );
			return;
		}
	}
	//////////////////////////////////////////////////////////////////////////

	// Bug #0000700: people with infection should give medic kill if they suicide	
	if( IsInfected() && GetInfector() )
		SetSpecialInfectedDeath();

	// Bug #0001545: going to spectator mode while cloaked does not turn off cloak shader -- Defrag
	// When changing to spec, the class slot is immediately changed to 0 prior to Event_Killed() being run
	// In Event_Killed(), the uncloak call is only made when the player is a spy, so the uncloak stuff wasn't being called.  Do it here instead.

	if( GetClassSlot() == CLASS_SPY )
	{
		Uncloak( true );
	}

	// set their class to unassigned, so that they don't spawn
	// immediately when changing teams
	//ReadPlayerClassDataFromFileForSlot( filesystem, "unassigned", &m_hPlayerClassFileInfo, GetEncryptionKey() );
	
	SetClassForClient(0);

	RemoveItems();

	// Make sure they don't subsequently drop any grenades
	m_iGrenadeState = 0;

	// Only kill the player if they are alive on a team
	if (IsAlive() && GetTeamNumber() >= TEAM_BLUE)
	{
		KillPlayer();

		// This isn't called when you changeteams
		CFFLuaSC hPlayerKilled;
		hPlayerKilled.Push(this);
		_scriptman.RunPredicates_LUA( NULL, &hPlayerKilled, "player_killed" );
	}
	
	ChangeTeam(iTeam);

	// Bug #0001686: Possible to spectate outside of spectator mode
	// If our previous team was spectator, then stop observer mode.
	if( iOldTeam == FF_TEAM_SPEC )
	{
		StopObserverMode();
	}
	
	// Make sure they don't think they're meant to be spawning as a new class
	m_iNextClass = 0;

	// Cancel map guides
    m_hNextMapGuide = NULL;

	// A forced spawn situation
	m_flNextSpawnDelay = -1;

	// Hide motd panel if it's showing
	ShowViewPortPanel( PANEL_INFO, false );

	// Hide team panel if it's showing
	ShowViewPortPanel( PANEL_TEAM, false );

	Spawn();
/*
	if( GetTeamNumber() != TEAM_SPECTATOR && GetTeamNumber() != TEAM_UNASSIGNED )
	{
		KillAndRemoveItems();
		ChangeTeam( iTeam );

		// Fix - Make sure they don't think they're meant to be spawning as a new class
		m_iNextClass = 0;	
	}
	else
	{
		ChangeTeam( iTeam );
		m_lifeState = LIFE_DISCARDBODY;
		pl.deadflag = true;
		Spawn();

		// Fix - Make sure they don't think they're meant to be spawning as a new class
		m_iNextClass = 0;	
	}

	ChangeTeam( iTeam );*/
}

//-----------------------------------------------------------------------------
// Purpose: Remove buildables
//-----------------------------------------------------------------------------
void CFFPlayer::RemoveBuildables( void )
{
	m_bBuilding = false;
	m_bStaticBuilding = false;
	m_iCurBuild = FF_BUILD_NONE;
	m_iWantBuild = FF_BUILD_NONE;

	// Remove buildables if they exist
	if( GetDispenser() )
		GetDispenser()->Cancel();

	if( GetSentryGun() )
		GetSentryGun()->Cancel();

	if( GetDetpack() )
		GetDetpack()->Cancel();

	if( GetManCannon() )
		GetManCannon()->Cancel();
}

//-----------------------------------------------------------------------------
// Purpose: Remove grenades
//-----------------------------------------------------------------------------
void CFFPlayer::RemoveProjectiles( void )
{
	CBaseEntity *pTemp = NULL;
	CBaseEntity *pEnt = gEntList.FindEntityByOwner(NULL, this);

	while (pEnt != NULL)
	{
		if (dynamic_cast<CFFProjectileBase *>( pEnt ) != NULL)
		{
			pTemp = gEntList.FindEntityByOwner( pEnt, this );
			UTIL_Remove(pEnt);
			pEnt = pTemp;
		}
		// #0001536: napalm and caltrops become hostile after changeteam
		// Begin changes by Defrag.  Probably could use some kind of RTTI for this instead of casting (should be cheaper)
		// but I just went with what was currently used, instead.  This isn't called too often anyway.
		else if(dynamic_cast<CFFGrenadeNapalmlet *>( pEnt ) != NULL )
		{
			pTemp = gEntList.FindEntityByOwner( pEnt, this );
			UTIL_Remove(pEnt);
			pEnt = pTemp;			
		}	// End changes by Defrag		
		else
		{
			pEnt = gEntList.FindEntityByOwner(pEnt, this);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove backpacks
//-----------------------------------------------------------------------------
void CFFPlayer::RemoveBackpacks( void )
{
	CBaseEntity *pTemp = NULL;
	CBaseEntity *pEntity = gEntList.FindEntityByClassT( NULL, CLASS_BACKPACK );

	while( pEntity )
	{
		if( ToFFPlayer( pEntity->GetOwnerEntity() ) == this )
		{
			pTemp = gEntList.FindEntityByClassT( pEntity, CLASS_BACKPACK );
			UTIL_Remove( pEntity );
			pEntity = pTemp;
		}
		else
		{
			pEntity = gEntList.FindEntityByClassT( pEntity, CLASS_BACKPACK );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove some items from the player
//-----------------------------------------------------------------------------
void CFFPlayer::RemoveItems( void )
{
	// Remove buildables
	RemoveBuildables();	

	// Remove any projectiles (grenades, nails, etc)
	RemoveProjectiles();
}

void CFFPlayer::KillPlayer( void )
{
	// Unlock the player if they were building and died
	UnlockPlayer();
	Extinguish();

	// have the player kill themself
	m_iHealth = 0;

	if (IsAlive())
	{
		// Bug #0000700: people with infection should give medic kill if they suicide
		if( GetSpecialInfectedDeath() && IsInfected() && GetInfector() )
		{
			Event_Killed( CTakeDamageInfo( GetInfector(), GetInfector(), 0, DMG_NEVERGIB ) );	// |-- Mirv: pInflictor = NULL so that death message is "x died."
		}
		else
			Event_Killed( CTakeDamageInfo( NULL, this, 0, DMG_NEVERGIB ) );
		Event_Dying();
	}
	else
	{
		SetThink(&CBasePlayer::PlayerDeathThink);
		SetNextThink(gpGlobals->curtime);
	}
}

void CFFPlayer::KillAndRemoveItems( void )
{
	// Not on a team, don't be so hasty
	if (GetTeamNumber() < TEAM_BLUE)
	{
		m_lifeState = LIFE_DISCARDBODY;
		pl.deadflag = true;

		return;
	}

	RemoveItems();
	KillPlayer();
}

void CFFPlayer::LockPlayerInPlace( void )
{
 	SetMoveType( MOVETYPE_NONE );

	// Bug #0000333: Buildable Behavior (non build slot) while building
	SetAbsVelocity( Vector( 0, 0, 0 ) );
}

void CFFPlayer::UnlockPlayer( void )
{
	SetMoveType( MOVETYPE_WALK );
	//SetAbsVelocity( Vector( 0, 0, 0 ) );
}

void CFFPlayer::FindRadioTaggedPlayers( void )
{
	if( !m_hRadioTagData.Get() )
		return;

	// Reset stuff back to zero
	m_hRadioTagData->ClearVisible();

	// Get client count
	int iMaxClients = gpGlobals->maxClients;

	// If we're the only ones we don't care
	if( iMaxClients < 2 )
		return;	

	// My origin
	Vector vecOrigin = GetFeetOrigin();

	// Loop through doing stuff on each player
	for( int i = 1; i <= iMaxClients; i++ )
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );
		
		if( !pPlayer )
			continue;		

		// Skip if not a player
		if( !pPlayer->IsPlayer() )
			continue;

		// Skip if spec
		if( pPlayer->IsObserver() || FF_IsPlayerSpec( pPlayer ) )
			continue;

		// Skip if us
		if( pPlayer == this )
			continue;

		// Skip if not tagged
		if( !pPlayer->IsRadioTagged() )
			continue;

		// Bug #0000517: Enemies see radio tag.
		// Only want to show players whom people on our team have tagged or
		// players whom allies have tagged
		if( g_pGameRules->PlayerRelationship( this, ToFFPlayer( pPlayer->GetPlayerWhoTaggedMe() ) ) != GR_TEAMMATE )
			continue;

		// Get their origin
		Vector vecPlayerOrigin = pPlayer->GetFeetOrigin();

		// Skip if they're out of range
		if( vecOrigin.DistTo( vecPlayerOrigin ) > RADIOTAG_DISTANCE )
			continue;

		// We're left w/ a player who's within range
		// Add player to a list and send off to client

		m_hRadioTagData->Set( pPlayer->entindex(), true, pPlayer->GetClassSlot(), pPlayer->GetTeamNumber(), ( pPlayer->GetFlags() & FL_DUCKING ) ? true : false, vecPlayerOrigin );

		Omnibot::Notify_RadioTagUpdate(this, pPlayer);	
	}
}

void CFFPlayer::Command_WhatTeam( void )
{
}

void CFFPlayer::Command_DispenserText( void )
{
	if( engine->Cmd_Argc( ) < 1 )
	{
		// no text given
		return;
	}

	// Build a string based off of the input
	// Just store the string until we build
	// a dispenser then tell the dispenser 
	// the text

	// Clear out whatever was in there
	m_szCustomDispenserText[ 0 ] = '\0';

	// Get the number of args
	int iArgs = engine->Cmd_Argc( );

	// Running total of the length of the dispenser
	// text string
	int iTotalLen = 0;

	// Start building a string
	for( int i = 1; i < iArgs; i++ )
	{
		// Grab a chunk of text
		char *pszTemp = engine->Cmd_Argv( i );
		// Get the length of said chunk
		int iTempLen = Q_strlen( pszTemp );

		// FF_BUILD_DISP_STRING_LEN - 2 for the null terminator and
		// the space if there is a next word
		if(( iTempLen + iTotalLen ) >= ( FF_BUILD_DISP_STRING_LEN - 2 ))
			iTempLen = ( ( FF_BUILD_DISP_STRING_LEN - 1 ) - iTotalLen );

		// Add iTempLen of the chunk
		Q_strncat( m_szCustomDispenserText, pszTemp, sizeof( m_szCustomDispenserText ), iTempLen );
		// Add the space
		Q_strncat( m_szCustomDispenserText, " ", sizeof(m_szCustomDispenserText) );

		// Increase running len total + 1 to account
		// for the space we just added too
		iTotalLen += ( iTempLen + 1 );
	}

	//DevMsg( "[Dispenser Text] %s\n", m_szCustomDispenserText );

	// Change text on the fly
	if( GetDispenser() )
		GetDispenser()->SetText( m_szCustomDispenserText );
}

void CFFPlayer::Command_BuildDispenser( void )
{
	//m_bCancelledBuild = false;
	m_iWantBuild = FF_BUILD_DISPENSER;
	PreBuildGenericThink();
}

void CFFPlayer::Command_BuildSentryGun( void )
{	
	//m_bCancelledBuild = false;
	m_iWantBuild = FF_BUILD_SENTRYGUN;
	PreBuildGenericThink();
}

void CFFPlayer::Command_BuildDetpack( void )
{
	// Assume 5 second fuse time
	m_iDetpackTime = 5;

	// If there's an argument...
	if( engine->Cmd_Argc() > 1 )
	{
		// Grab the first argument after detpack
		char *pszArg = engine->Cmd_Argv( 1 );

		//DevMsg( "[Detpack Timer] %s\n", pszArg );

		m_iDetpackTime = atoi( pszArg );
		bool bInRange = true;
		if( ( m_iDetpackTime < 5 ) || ( m_iDetpackTime > 60 ) )
		{
			//m_iDetpackTime = clamp( m_iDetpackTime, 5, 60 );
			bInRange = false;
		}

		if(!bInRange)
		{
			char szBuffer[16];
			Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", m_iDetpackTime );

			// EDIT: There's no gcc/g++ itoa apparently?
			//ClientPrint(this, HUD_PRINTCONSOLE, "FF_INVALIDTIMER", itoa((int) m_iDetpackTime, szBuffer, 10));
			ClientPrint(this, HUD_PRINTCENTER, "FF_INVALIDTIMER", szBuffer);
			return;
		}

		// Bug #0000453: Detpack timer can't be anything other than multiples of five
//		if( m_iDetpackTime % 5 != 0 )
//		{
//			// Round it to the nearest multiple of 5
//			int iMultiple = (int)(((float)m_iDetpackTime + 2.5f) / 5.0f);
//			m_iDetpackTime = iMultiple * 5;
//			DevMsg( "[Detpack Timer] Fuse length must be a multiple of 5! Resetting to nearest multiple %d.\n", m_iDetpackTime );
//		}
	}

	//m_bCancelledBuild = false;
	m_iWantBuild = FF_BUILD_DETPACK;
	PreBuildGenericThink();
}

void CFFPlayer::Command_BuildManCannon( void )
{
	m_iWantBuild = FF_BUILD_MANCANNON;
	PreBuildGenericThink();
}

//ConVar sg_buildtime("ffdev_sg_buildtime", "7.0", FCVAR_REPLICATED | FCVAR_CHEAT, "Sentry Gun build time");
#define SG_BUILDTIME 3.0f //sg_buildtime.GetFloat()  //

void CFFPlayer::PreBuildGenericThink( void )
{
	//
	// March 21, 2006 - Re-working entire build process
	//

	if( !m_bBuilding )
	{
		m_bBuilding = true; // Set these immediately in case player tries to do anything elsewhere, presumably?
		m_bStaticBuilding = true;

		// Store the player's current origin
		m_vecBuildOrigin = GetAbsOrigin();

		// See if player is in a no build area first
		// TODO: need to check where the SG is being built, NOT where player is? - AfterShock
		if( IsInNoBuild() && ( (m_iWantBuild == FF_BUILD_DISPENSER) || (m_iWantBuild == FF_BUILD_SENTRYGUN) || (m_iWantBuild == FF_BUILD_MANCANNON) ) )
		{
			Omnibot::Notify_Build_CantBuild(this, m_iWantBuild);

			// Re-initialize
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;
			m_bStaticBuilding = false;
			
			ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_NOBUILD" );

			return;
		}

		/*
		DevMsg( "[Building] Not currently building so lets try to build a: %s" );
		switch( m_iWantBuild )
		{
			case FF_BUILD_DISPENSER: DevMsg( "dispenser\n" ); break;
			case FF_BUILD_SENTRYGUN: DevMsg( "sentrygun\n" ); break;
			case FF_BUILD_DETPACK: DevMsg( "detpack\n" ); break;
			case FF_BUILD_MANCANNON: DevMsg( "mancannon\n" ); break;
		}
		*/

		// See if the user has already built this item
		if( ( (m_iWantBuild == FF_BUILD_DISPENSER) && GetDispenser()) ||
			( (m_iWantBuild == FF_BUILD_SENTRYGUN) && GetSentryGun()) ||
			( (m_iWantBuild == FF_BUILD_DETPACK) && GetDetpack()) ||
			( (m_iWantBuild == FF_BUILD_MANCANNON) && GetManCannon()) )
		{
			Omnibot::Notify_Build_AlreadyBuilt(this, m_iWantBuild);

			switch( m_iWantBuild )
			{
				case FF_BUILD_DISPENSER: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_DISPENSER_ALREADYBUILT" ); break;
				case FF_BUILD_SENTRYGUN: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_SENTRYGUN_ALREADYBUILT" ); break;
				case FF_BUILD_DETPACK: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_DETPACK_ALREADYSET" ); break;
				case FF_BUILD_MANCANNON: 
					// If the Scout right-clicks after has built a jump pad, he'll get the warning, and a message
					// that he can click again to det it; this gives him 2 seconds to do so
					if ( gpGlobals->curtime > m_flMancannonDetTime )
					{
						ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_MANCANNON_ALREADYBUILT" ); 
						m_flMancannonDetTime = gpGlobals->curtime + 2.f;
					}
					else
					{
						CFFManCannon *pJumpPadToDet = GetManCannon();
						if ( pJumpPadToDet )
						{
							pJumpPadToDet->Detonate();
							ClientPrint( this, HUD_PRINTCENTER, "#FF_MANCANNON_DESTROYED" );
						}
					}
					break;
			}

			// Re-initialize
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;
			m_bStaticBuilding = false;

			return;
		}

		// See if the user has appropriate ammo
		if( ( (m_iWantBuild == FF_BUILD_DISPENSER) && (GetAmmoCount( AMMO_CELLS ) < 100) ) ||
			( (m_iWantBuild == FF_BUILD_SENTRYGUN) && (GetAmmoCount( AMMO_CELLS ) < 130) ) ||
			( (m_iWantBuild == FF_BUILD_DETPACK) && (GetAmmoCount( AMMO_DETPACK ) < 1 )) ||
			( (m_iWantBuild == FF_BUILD_MANCANNON) && (GetAmmoCount( AMMO_MANCANNON ) < 1 )) )
		{
			Omnibot::Notify_Build_NotEnoughAmmo(this, m_iWantBuild);

			switch( m_iWantBuild )
			{
				case FF_BUILD_DISPENSER: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_DISPENSER_NOTENOUGHAMMO" ); break;
				case FF_BUILD_SENTRYGUN: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_SENTRYGUN_NOTENOUGHAMMO" ); break;
				case FF_BUILD_DETPACK: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_DETPACK_NOTENOUGHAMMO" ); break;
				case FF_BUILD_MANCANNON: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_MANCANNON_NOTENOUGHAMMO" ); break;
			}
			
			// Re-initialize
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;
			m_bStaticBuilding = false;

			return;
		}

		// See if on ground...
		if( !FBitSet( GetFlags(), FL_ONGROUND ) )
		{
			Omnibot::Notify_Build_MustBeOnGround(this, m_iWantBuild);

			ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_MUSTBEONGROUND" );

			// Re-initialize
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;
			m_bStaticBuilding = false;

			return;
		}

		// Our neat buildable info container
		CFFBuildableInfo hBuildInfo( this, m_iWantBuild );

		// Will we be able to build here?
		if( hBuildInfo.BuildResult() == BUILD_ALLOWED )
		{
			// BUILD! - Create the actual item finally
			m_iCurBuild = m_iWantBuild;	

			switch( m_iCurBuild )
			{
				case FF_BUILD_DISPENSER:
				{					
					FF_SendHint( this, ENGY_BUILDDISP, 3, PRIORITY_NORMAL, "#FF_HINT_ENGY_BUILDDISP" );

					// Changed to building straight on ground (Bug #0000191: Engy "imagines" SG placement, then lifts SG, then back to imagined position.)
					CFFDispenser *pDispenser = CFFDispenser::Create( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles(), this );
					
					// Set custom text
					pDispenser->SetText( m_szCustomDispenserText );

					pDispenser->SetLocation(g_pGameRules->GetChatLocation(true, this));

					// Mirv: Store future ground location + orientation
					pDispenser->SetGroundOrigin( hBuildInfo.GetBuildOrigin() );
					pDispenser->SetGroundAngles( hBuildInfo.GetBuildAngles() );

					// Set network var
					m_hDispenser = pDispenser;

					// Set the time it takes to build
					m_flBuildTime = gpGlobals->curtime + 2.0f;

					// Bug #0001558: exploit to get instant lvl2 SG
					// Moved code to remove cells from CFFDispenser::GoLive() to here.  
					// Leaving the remove armour code in that function as you can't fiddle with armour vals via this exploit -> Defrag
					RemoveAmmo( 100, AMMO_CELLS );

					Omnibot::Notify_DispenserBuilding(this, pDispenser);
					
					//m_bStaticBuilding = false; // AfterShock - Uncomment this for testing drop-and-run SGs / Dispensers! (also need SG_BUILDTIME raising)
				}
				break;

				case FF_BUILD_SENTRYGUN:
				{
					// Jiggles: Start hint code	
					// Event: Player starts building SG
					FF_SendHint( this, ENGY_BUILDSG, 3, PRIORITY_NORMAL, "#FF_HINT_ENGY_BUILDSG" );

					// Notify allied players within 1000 units
					CBaseEntity *ent = NULL;
					for( CEntitySphereQuery sphere( GetAbsOrigin(), 1000 ); ( ent = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
					{
						if( ent->IsPlayer() )
						{
							CFFPlayer *player = ToFFPlayer( ent );
							// Only alive friendly players within 1000 units are sent these hints
							if( player && ( player != this ) && player->IsAlive() && ( g_pGameRules->PlayerRelationship( this, player ) == GR_TEAMMATE ) )
							{
								if( player->GetClassSlot() == CLASS_ENGINEER )
									FF_SendHint( player, ENGY_TEAMSG, 3, PRIORITY_NORMAL, "#FF_HINT_ENGY_TEAMSG" );  // Go help that dude upgrade!
								else
									FF_SendHint( player, GLOBAL_DEFENDSG, 3, PRIORITY_NORMAL, "#FF_HINT_GLOBAL_DEFENDSG" );  // Go protect that dude!
							}
						}
					}			
					// End hint code

					// Changed to building straight on ground (Bug #0000191: Engy "imagines" SG placement, then lifts SG, then back to imagined position.)
					CFFSentryGun *pSentryGun = CFFSentryGun::Create( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles(), this );
				
					pSentryGun->SetLocation(g_pGameRules->GetChatLocation(true, this));

					// Mirv: Store future ground location + orientation
					pSentryGun->SetGroundOrigin( hBuildInfo.GetBuildOrigin() );
					pSentryGun->SetGroundAngles( hBuildInfo.GetBuildAngles() );

					// Set network var
					m_hSentryGun = pSentryGun;

					// Set the time it takes to build
					m_flBuildTime = gpGlobals->curtime + SG_BUILDTIME/*5.0f*/;	// |-- Mirv: Bug #0000127: when building a sentry gun the build finishes before the sound

					// Bug #0001558: exploit to get instant lvl2 SG
					// Moved code to remove cells from CFFSentryGun::GoLive() to here -> Defrag
					RemoveAmmo( 130, AMMO_CELLS );

					Omnibot::Notify_SentryBuilding(this, pSentryGun);

					//m_bStaticBuilding = false; // AfterShock - Uncomment this for testing drop-and-run SGs / Dispensers! (also need SG_BUILDTIME raising)
				}
				break;

				case FF_BUILD_DETPACK:
				{
					// Changed to building straight on ground (Bug #0000191: Engy "imagines" SG placement, then lifts SG, then back to imagined position.)
					CFFDetpack *pDetpack = CFFDetpack::Create( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles(), this );

					pDetpack->SetLocation(g_pGameRules->GetChatLocation(true, this));

					// Set the fuse time
					pDetpack->m_iFuseTime = m_iDetpackTime;

					// Mirv: Store future ground location + orientation
					pDetpack->SetGroundOrigin( hBuildInfo.GetBuildOrigin() );
					pDetpack->SetGroundAngles( hBuildInfo.GetBuildAngles() );

					// Set network var
					m_hDetpack = pDetpack;

					// Set time it takes to build
					m_flBuildTime = gpGlobals->curtime + 3.0f; // mulch: bug 0000337: build time 3 seconds for detpack

					Omnibot::Notify_DetpackBuilding(this, pDetpack);
				}
				break;

				case FF_BUILD_MANCANNON:
				{
					CFFManCannon *pManCannon = CFFManCannon::Create( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles(), this );

					pManCannon->SetLocation( g_pGameRules->GetChatLocation( true, this ) );
					pManCannon->SetGroundOrigin( hBuildInfo.GetBuildOrigin() );
					pManCannon->SetGroundAngles( hBuildInfo.GetBuildAngles() );

					m_hManCannon = pManCannon;
					m_flBuildTime = gpGlobals->curtime + 3.5f; // 3.5 seconds to build?

					// TODO: Omnibot::Notify_ManCannonBuilding( this, pManCannon );
				}
				break;
			}
			
			if ( m_bStaticBuilding == true )
			{
				LockPlayerInPlace(); 
				m_hActiveWeapon = GetActiveFFWeapon();
				if( m_hActiveWeapon )
					m_hActiveWeapon->Holster( NULL );
			}

			// Mirv: Start build timer
			CSingleUserRecipientFilter user( this );
			user.MakeReliable();
			UserMessageBegin( user, "FF_BuildTimer" );
				WRITE_SHORT( m_iCurBuild );
				WRITE_FLOAT( m_flBuildTime - gpGlobals->curtime );
			MessageEnd();
		}
		else
		{
			// Show a message as to why we couldn't build
			hBuildInfo.GetBuildError();

			m_bBuilding = false;
			m_bStaticBuilding = false;
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
		}
	}
	else
	{
		// Player is already in the process of building something so cancel the build if
		// they're trying to build what they're already building otherwise just show a message
		// saying they can't build while building

		if( m_iCurBuild == m_iWantBuild )
		{
			// DevMsg( "[Building] You're currently building this item so cancel the build.\n" );
			Omnibot::Notify_Build_BuildCancelled(this,m_iCurBuild);

			CFFBuildableObject *pBuildable = GetBuildable( m_iCurBuild );
			
			if( pBuildable )
			{
				switch( m_iCurBuild )
				{
				case FF_BUILD_DISPENSER:
					GiveAmmo( 100, AMMO_CELLS, true );
					break;
				case FF_BUILD_SENTRYGUN:
					GiveAmmo( 130, AMMO_CELLS, true );
					break;
				default:
					break;
				}

				pBuildable->Cancel();
			}

				
			// Unlock the player
			UnlockPlayer();			

			// Mirv: Cancel build timer
			CSingleUserRecipientFilter user( this );
			user.MakeReliable();
			UserMessageBegin( user, "FF_BuildTimer" );
				WRITE_SHORT( FF_BUILD_NONE );
				WRITE_FLOAT( 0 );
			MessageEnd();

			// Re-initialize
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;
			m_bStaticBuilding = false;

			
			if( m_hActiveWeapon )
				m_hActiveWeapon->Deploy();
				
		}
		else
		{
			ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_MULTIPLEBUILDS" );
		}
	}
}

void CFFPlayer::PostBuildGenericThink( void )
{
	// If we're building
	if( m_bBuilding )
	{
		FFWeaponID switchToWeapon = FF_WEAPON_NONE;

		// Find out what we're building and drop it to the floor
		switch( m_iCurBuild )
		{
			case FF_BUILD_DISPENSER:
			{
				if( GetDispenser() )
				{
					GetDispenser()->GoLive();

					 switchToWeapon = FF_WEAPON_SPANNER;
					IGameEvent *pEvent = gameeventmanager->CreateEvent( "build_dispenser" );
					if( pEvent )
					{
						pEvent->SetInt( "userid", GetUserID() );
						gameeventmanager->FireEvent( pEvent, true );
					}	
					
					FF_SendHint( this, ENGY_BUILTDISP, 3, PRIORITY_NORMAL, "#FF_HINT_ENGY_BUILTDISP" );


				}
			}
			break;

			case FF_BUILD_SENTRYGUN:
			{
				if( GetSentryGun() )
				{
					GetSentryGun()->GoLive();

					switchToWeapon = FF_WEAPON_SPANNER;
					IGameEvent *pEvent = gameeventmanager->CreateEvent( "build_sentrygun" );
					if( pEvent )
					{
						pEvent->SetInt( "userid", GetUserID() );
						gameeventmanager->FireEvent( pEvent, true );
					}
					
					FF_SendHint( this, ENGY_BUILTSG, 3, PRIORITY_NORMAL, "#FF_HINT_ENGY_BUILTSG" );

				}
			}
			break;

			case FF_BUILD_DETPACK: 
			{
				if( GetDetpack() )
				{
					GetDetpack()->GoLive();

					switchToWeapon = FF_WEAPON_GRENADELAUNCHER;
					IGameEvent *pEvent = gameeventmanager->CreateEvent( "build_detpack" );
					if( pEvent )
					{
						pEvent->SetInt( "userid", GetUserID() );
						gameeventmanager->FireEvent( pEvent, true );
					}
				}
			}
			break;

			case FF_BUILD_MANCANNON:
			{
				if( GetManCannon() )
				{
					GetManCannon()->GoLive();

					// TODO: Change to something
					switchToWeapon = FF_WEAPON_NAILGUN;
					IGameEvent *pEvent = gameeventmanager->CreateEvent( "build_mancannon" );
					if( pEvent )
					{
						pEvent->SetInt( "userid", GetUserID() );
						gameeventmanager->FireEvent( pEvent, true );
					}
				}
			}
			break;
		}

		if ( m_bStaticBuilding) 
		{
			// Unlock the player
			UnlockPlayer();

			// Reset stuff		
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;
			m_bStaticBuilding = false; // this needs to be before deploy weapon, you can't deploy weapons whilst building! - AfterShock

			if( m_hActiveWeapon )
				m_hActiveWeapon->Deploy();

			// Switch to a different weapon if you finished building
			if((switchToWeapon != FF_WEAPON_NONE) && (switchToWeapon < MAX_WEAPONS))
			{
				CFFWeaponBase *weap;
				for (int i = 0; i < MAX_WEAPON_SLOTS; i++)
				{
					weap = dynamic_cast<CFFWeaponBase *>(GetWeapon(i));
					if (weap && weap->GetWeaponID() == switchToWeapon)
					{
						Weapon_Switch(weap);
						break;
					}
				}
			}
		}
		else
		{
			// Reset stuff		
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;
			m_bStaticBuilding = false;
			//m_bCancelledBuild = false;
		}

	}
	else
	{
		// TODO: Something bad happened - Called the post think 
		// and we weren't building - wtf happened!?
		// NOTE: Cancelling a build stops the PostBuildGenericThink
		// from _EVER_ being called

		// Give a message for now
		Warning( "CFFPlayer::PostBuildGenericThink - ERROR!!!\n" );
	}

	// Deploy weapon
	//if( GetActiveWeapon()->GetLastWeapon() )
	//	GetActiveWeapon()->GetLastWeapon()->Deploy();
	//if( m_pBuildLastWeapon )
	//	m_pBuildLastWeapon->Deploy();
}

/**
	FlagInfo
*/
void CFFPlayer::Command_FlagInfo( void )
{	
	CFFLuaSC hFlagInfo( 1, this );
	_scriptman.RunPredicates_LUA(NULL, &hFlagInfo, "flaginfo");
}

/**
	DropItems
*/
void CFFPlayer::Command_DropItems( void )
{	
	CFFLuaSC hDropItemCmd( 1, this );
	//entsys.RunPredicates(NULL, this, "dropitems");
	CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( NULL, CLASS_INFOSCRIPT );

	while( pEnt != NULL )
	{
		if( pEnt->GetOwnerEntity() == ( CBaseEntity * )this )
		{
			// If the function exists, try and drop
			_scriptman.RunPredicates_LUA( pEnt, &hDropItemCmd, "dropitemcmd" );
				//pEnt->Drop(30.0f, 500.0f);
		}

		// Next!
		pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( pEnt, CLASS_INFOSCRIPT );
	}
}

void CFFPlayer::Command_DetPipes( void )
{	
	if( ( GetPipebombShotTime() + PIPE_DET_DELAY ) < gpGlobals->curtime )
		CFFProjectilePipebomb::DestroyAllPipes(this);
}

/**
	Discard
*/

// 2007/08/20
// Tidied up this function, fixed cells bug and also renamed a few variables to make things clearer.
// Removed ubiquitous i counters for more descriptive names.  Old variable name for "bKeepAmmo" was "iDiscardable" which was 
// ... backwards (since we only discarded it if iDiscardable evaluated to false).  Made my head hurt a bit! |---> Defrag

void CFFPlayer::Command_Discard( void )
{
	CFFItemBackpack *pBackpack = NULL;

	// if any of these are flagged as true, then we have a weapon that uses this ammo type and thus retain it (no discard allowed).  
	bool bKeepAmmo[MAX_AMMO_TYPES] = { false };

	// Check we have the ammo to discard first
	if ( GetClassSlot() != CLASS_ENGINEER )
	{
		// get ammo used by our weapons
		for( int iWeaponNum = 0; iWeaponNum < MAX_WEAPON_SLOTS; iWeaponNum++ )
		{
			if( GetWeapon( iWeaponNum ) )
			{
				int ammoid = GetWeapon( iWeaponNum )->GetPrimaryAmmoType();

				if (ammoid > -1)
				{
					// We need this ammo.  Keep it.
					bKeepAmmo[ammoid] = 1;					
				}
			}
		}

		// Add ammo if they have any
		for( int iAmmoNum = 0; iAmmoNum < MAX_AMMO_TYPES; iAmmoNum++ )
		{
			// Only discard ammo that we do not want to keep.  I.e. discard = ! keep
			if ( ! bKeepAmmo[iAmmoNum] && GetAmmoCount( iAmmoNum ) > 0)
			{
				if ( ! pBackpack )
					pBackpack = (CFFItemBackpack *) CBaseEntity::Create( "ff_item_backpack", GetAbsOrigin(), GetAbsAngles() );

				if( pBackpack )
				{
					pBackpack->SetAmmoCount( iAmmoNum, GetAmmoCount( iAmmoNum ));
					RemoveAmmo( GetAmmoCount( iAmmoNum), iAmmoNum );
				}
			}
		}
	}
	// We're an engineer
	else
	{
		int iShells = min(GetAmmoCount(AMMO_SHELLS), 40); // Default 20, 2 per unit
		int iNails = min(GetAmmoCount(AMMO_NAILS), 20); // Default 20, 1 per unit
		int iCells = min(GetAmmoCount(AMMO_CELLS), 20); // Default 10, 2 per unit
		int iRockets = min(GetAmmoCount(AMMO_ROCKETS), 20); // Default 10, 2 per unit

		// We have at least 1 of one type of ammo
		if (iShells || iNails || iCells || iRockets)
		{
			pBackpack = (CFFItemBackpack *) CBaseEntity::Create("ff_item_backpack", GetAbsOrigin(), GetAbsAngles());

			if (pBackpack)
			{
				pBackpack->SetAmmoCount(GetAmmoDef()->Index(AMMO_SHELLS), iShells);
				pBackpack->SetAmmoCount(GetAmmoDef()->Index(AMMO_NAILS), iNails);
				pBackpack->SetAmmoCount(GetAmmoDef()->Index(AMMO_CELLS), iCells);
				pBackpack->SetAmmoCount(GetAmmoDef()->Index(AMMO_ROCKETS), iRockets);

				RemoveAmmo(iShells, AMMO_SHELLS);
				RemoveAmmo(iNails, AMMO_NAILS);
				RemoveAmmo(iCells, AMMO_CELLS);
				RemoveAmmo(iRockets, AMMO_ROCKETS);
			}
		}
	}

	// Now sort out the rest of the backpack stuff if needed
	if (pBackpack)
	{
		pBackpack->SetSpawnFlags(SF_NORESPAWN);
		pBackpack->SetOwnerEntity( this );

		// UNDONE: This is causing the sniper trace to ignore the backpack
		//pBackpack->SetOwnerEntity(this);

		Vector vForward;
		AngleVectors(EyeAngles(), &vForward);

		vForward *= 420.0f;

		// Bugfix: Floating objects
		if (vForward.z < 1.0f)
			vForward.z = 1.0f;

		pBackpack->SetAbsVelocity(vForward);
		pBackpack->SetAbsOrigin(GetAbsOrigin());

		// Play a sound
		EmitSound("Item.Toss");
	}
}

void CFFPlayer::StatusEffectsThink( void )
{
	if( m_bGassed )
	{
		if(m_flGasTime > gpGlobals->curtime)
		{
			if(m_flNextGas < gpGlobals->curtime)
			{
				CFFPlayer *pGasser = GetGasser();
				if( pGasser )
				{
					CTakeDamageInfo info(pGasser, pGasser, vec3_origin, GetAbsOrigin(), 1.0f, DMG_DIRECT);
					info.SetCustomKill(KILLTYPE_GASSED);

					TakeDamage(info);
				}
				else //must be lua set...
				{
					CTakeDamageInfo info(this, this, vec3_origin, GetAbsOrigin(), 1.0f, DMG_DIRECT);
					info.SetCustomKill(KILLTYPE_GASSED);

					TakeDamage(info);
				}

				CSingleUserRecipientFilter user( ( CBasePlayer * )this );
				user.MakeReliable();
				UserMessageBegin( user, "FFViewEffect" );
					WRITE_BYTE( FF_VIEWEFFECT_GASSED );
					// Jiggles: Changed from 6 to 2.5 to better match the 10 sec gas duration (Mantis: 0001222)
					WRITE_FLOAT( 2.5f );
				MessageEnd();

				m_flNextGas = gpGlobals->curtime + 1.0f;
			}
		}
		else
		{
			UnGas();
		}
	}

	if (m_bSliding)
	{
		if (m_flSlidingTime <= gpGlobals->curtime)
		{
			StopSliding();
		}
	}

	// If we jump in water up to waist level, extinguish ourselves
	if (m_iBurnTicks && GetWaterLevel() >= WL_Waist)
		Extinguish();

	// if we're on fire, then do something about it
	if (m_iBurnTicks && (m_flNextBurnTick < gpGlobals->curtime))
	{
		// EmitSound( "General.BurningFlesh" );	// |-- Mirv: Dunno it just sounds odd using the emp sound!

		float damage = m_flBurningDamage / (float)m_iBurnTicks;

		// do damage. If igniter is NULL lets just kill the fire, it
		// means the guy has left the server and just makes bad stuff
		// happen.
		CFFPlayer *pIgniter = GetIgniter();
		if( pIgniter )
		{
			CTakeDamageInfo info( pIgniter, pIgniter, damage, DMG_BURN );

			int iBurnLevel = 0;
			if (m_bBurnFlagNG == true) 
				++iBurnLevel;
			if (m_bBurnFlagFT == true) 
				++iBurnLevel;
			if (m_bBurnFlagIC == true) 
				++iBurnLevel;

			switch(iBurnLevel)
			{
			case 1:info.SetCustomKill(KILLTYPE_BURN_LEVEL1);break;
			case 2:info.SetCustomKill(KILLTYPE_BURN_LEVEL2);break;
			case 3:info.SetCustomKill(KILLTYPE_BURN_LEVEL3);break;
			}

			TakeDamage( info );

			// remove a tick
			m_iBurnTicks--;
			m_flBurningDamage -= damage;

			// schedule the next tick
			m_flNextBurnTick = gpGlobals->curtime + 1.25f;
		}
		else
		{
			Extinguish();
		}		
	}

	// check if the player needs a little health/armor (because they are a medic/engy)
	if ( IsAlive() ) // AfterShock: possible fix for medic crouch bug? Regen health the same tick you die?
	{
		if( ( ( GetClassSlot() == CLASS_MEDIC ) || ( GetClassSlot() == CLASS_ENGINEER ) ) &&
			( gpGlobals->curtime > ( m_fLastHealTick + FFDEV_REGEN_FREQ ) ) )
		{		
			m_fLastHealTick = gpGlobals->curtime;

			if( GetClassSlot() == CLASS_MEDIC )
			{
				// add the regen health
				// Don't call CFFPlayer::TakeHealth as it will clear status effects
				// Bug #0000528: Medics can self-cure being caltropped/tranq'ed
				if( BaseClass::TakeHealth( FFDEV_REGEN_HEALTH, DMG_GENERIC ) )			
				//if( TakeHealth( ffdev_regen_health.GetInt(), DMG_GENERIC ) )
				{				
					// make a sound if we did
					//EmitSound( "medkit.hit" );
				}

				// Give te medic some cells to generate health packs with...!
				GiveAmmo(5, AMMO_CELLS, true);
			}
			else if( GetClassSlot() == CLASS_ENGINEER )
			{
				// add the regen armor
				m_iArmor.GetForModify() = clamp( m_iArmor + FFDEV_REGEN_ARMOR, 0, m_iMaxArmor );
			}
		}
	}

	// Bug #0000485: If you're given beyond 100% health, by a medic, the health doesn't count down back to 100.
	// Reduce health if we're over healthed (health > maxhealth
	if( m_iHealth > m_iMaxHealth )
	{
		if( gpGlobals->curtime > ( m_flLastOverHealthTick + FFDEV_OVERHEALTH_FREQ ) )
		{
			m_flLastOverHealthTick = gpGlobals->curtime;
			m_iHealth = max( m_iHealth - FFDEV_REGEN_HEALTH, m_iMaxHealth );
		}
	}

	// If the player is infected, then take appropriate action
	if( IsInfected() && ( gpGlobals->curtime > ( m_fLastInfectedTick + FFDEV_INFECT_FREQ ) ) )
	{
		bool bIsInfected = true;

		// Need to check to see if the medic who infected us has changed teams
		// or dropped - switching to EHANDLE will handle the drop case
		if( m_hInfector )
		{
			CFFPlayer *pInfector = ToFFPlayer( m_hInfector );
			
			// dexter - reworked this a lil and removed the assert
			//AssertMsg( pInfector, "[Infect] pInfector == NULL\n" );
			if( pInfector )
			{				
				// Medic changed teams / switched class				
				if( pInfector->GetTeamNumber() != m_iInfectedTeam || !pInfector->GetClassSlot() || pInfector->GetClassSlot() != CLASS_MEDIC )
				{
					//DevMsg("Removing infection from %s because infector %s changed class or team\n", this->m_szNetname, pInfector->m_szNetname);
					m_bInfected = false;
				}
			}
			else
			{
				// dexter - this is a ghetto catch all incase the EHANDLE is good but isnt CFFPlayer (how the fuck)
				m_bInfected = false;
			}
		}
		else
		{
			// Player dropped
			m_bInfected = false;
		}

		if( m_iInfectTick >= FFDEV_INFECT_NUMTICKS )// GreenMushy: check to see if the infection should end
		{
			//Will heal for the correct amount
			Cure( NULL );
		}

		// If we were infected but just became uninfected,
		// remove hud effect
		if( bIsInfected && !m_bInfected )
		{
			CSingleUserRecipientFilter user( ( CBasePlayer * )this );
			user.MakeReliable();
			UserMessageBegin( user, "FFViewEffect" );
				WRITE_BYTE( FF_VIEWEFFECT_INFECTED );
				WRITE_FLOAT( 0.0f );
			MessageEnd(); 
		}

		// If we're still infected, cause damage
		if (IsInfected() && IsAlive())	// |-- Mirv: Bug #0000461: Infect sound plays eventhough you are dead
		{
			CFFPlayer *pInfector = ToFFPlayer( m_hInfector );

			// When you change this be sure to change the StopSound above ^^ for bug
			
			EmitSound( "Player.DrownContinue" );	// |-- Mirv: [TODO] Change to something more suitable
			m_fLastInfectedTick = gpGlobals->curtime;
			m_iInfectTick++;

			int iInfectDamage = m_fNextInfectedTickDamage;
			// if infect damage will kill, then bring the player to 1hp instead
			iInfectDamage = min(iInfectDamage, GetHealth() - 1);

			// calc next tick's damage
			// multiply tick damage by the mult, raise to the power of the exp, and round to the nearest whole number
			m_fNextInfectedTickDamage = (int)(pow(m_fNextInfectedTickDamage * FFDEV_INFECT_DAMAGEPERTICK_MULT, FFDEV_INFECT_DAMAGEPERTICK_EXP) + 0.5f);

			m_nNumInfectDamage += iInfectDamage;

			//Msg("Damage done: %d Tick: %d Total damage done: %d\n", iInfectDamage, m_iInfectTick, m_nNumInfectDamage);

			CTakeDamageInfo info( pInfector, pInfector, iInfectDamage, DMG_POISON );
			info.SetCustomKill(KILLTYPE_INFECTION);
			info.SetDamagePosition( GetAbsOrigin() );
			TakeDamage( info );

			CSingleUserRecipientFilter user((CBasePlayer *)this);
			user.MakeReliable();
			UserMessageBegin(user, "StatusIconUpdate");
				WRITE_BYTE(FF_STATUSICON_INFECTION);
				WRITE_FLOAT(2.0f);
			MessageEnd(); 

			/*
			// Bug #0000504: No infection visible effect
			CEffectData data;
			data.m_vOrigin = GetAbsOrigin() - Vector( 0, 0, 16.0f );
			data.m_vStart = GetAbsVelocity();
			data.m_flScale = 1.0f;
			DispatchEffect( "FF_InfectionEffect", data );

			CEffectData data2;
			data2.m_vOrigin = EyePosition() - Vector( 0, 0, 16.0f );
			data2.m_vStart = GetAbsVelocity();
			data2.m_flScale = 1.0f;			
			DispatchEffect( "FF_InfectionEffect", data2 );
			*/

			// Removing friendly infection spreading to stop laming on servers -> Defrag

			/*
			CBaseEntity *ent = NULL;

			// Infect anybody nearby
			for( CEntitySphereQuery sphere( GetAbsOrigin(), 128 ); ( ent = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
			{
				if( ent->IsPlayer() )
				{
					CFFPlayer *player = ToFFPlayer( ent );

					if( player && ( player != this ) && player->IsAlive() )
					{
						trace_t traceHit;
						UTIL_TraceLine( GetAbsOrigin(), player->GetAbsOrigin(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_DEBRIS, &traceHit );

						if( traceHit.fraction != 1.0f )
							continue;

						if( player->GetClassSlot() == CLASS_MEDIC )
							continue;

						// Bug #0000468: Infections transmit to non-teammates
						//if( player->GetTeamNumber() != GetTeamNumber() )
						// Changed to allow infections across allies
						if( g_pGameRules->PlayerRelationship( this, player ) == GR_NOTTEAMMATE )
							continue;

						// Infect this guy
						player->Infect( pInfector );
					}
				}
			}
			*/
		}
	}

	// check if any speed effects are over
	bool recalcspeed = false;
	for (int i=0; i<NUM_SPEED_EFFECTS; i++)
	{
		if (m_vSpeedEffects[i].active && ( m_vSpeedEffects[i].endTime < gpGlobals->curtime ) && ( m_vSpeedEffects[i].duration != -1 ) )
		{
			RemoveSpeedEffectByIndex( i );
			recalcspeed = true;
		}
	}

	// we might need to actually set their speed
	if (recalcspeed)
		RecalculateSpeed();

	// Bug #0000503: "Immunity" is not in the mod
	// See if immunity has worn off
	if( IsImmune() )
	{
		// TODO: Dispatch immune effect!

		if( gpGlobals->curtime > m_flImmuneTime )
			m_bImmune = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: So lua can add effects
//-----------------------------------------------------------------------------
void CFFPlayer::LuaAddEffect( int iEffect, float flEffectDuration, float flIconDuration, float flSpeed )
{
	// Invalid effect
	if( ( iEffect < 0 ) || ( iEffect > ( LUA_EF_MAX_FLAG - 1 ) ) )
		return;

	if( LuaRunEffect( iEffect, NULL, &flEffectDuration, &flIconDuration, &flSpeed ) )
	{
		switch( iEffect )
		{
			case LUA_EF_ONFIRE:
			{
			}
			break;

			case LUA_EF_CONC: Concuss( flEffectDuration, flIconDuration ); break;
			case LUA_EF_GAS: Gas( flEffectDuration, flIconDuration, NULL); break;
			case LUA_EF_INFECT: Infect( this ); break;
			case LUA_EF_RADIOTAG: SetRadioTagged( NULL, gpGlobals->curtime, flEffectDuration ); break;

			case LUA_EF_HEADSHOT:
			{
			}
			break;
			
			case LUA_EF_LEGSHOT: AddSpeedEffect( SE_LEGSHOT, flEffectDuration, flSpeed, SEM_BOOLEAN, FF_STATUSICON_LEGINJURY, flIconDuration, true ); break;
			case LUA_EF_TRANQ: AddSpeedEffect( SE_TRANQ, flEffectDuration, flSpeed, SEM_BOOLEAN | SEM_HEALABLE, FF_STATUSICON_TRANQUILIZED, flIconDuration, true ); break;
			case LUA_EF_ACSPINUP: AddSpeedEffect( SE_ASSAULTCANNON, flEffectDuration, flSpeed, SEM_BOOLEAN, -1, -1.0f, true );  break;
			case LUA_EF_SNIPERRIFLE: AddSpeedEffect( SE_SNIPERRIFLE, flEffectDuration, flSpeed, SEM_BOOLEAN, -1, -1.0f, true ); break;
			case LUA_EF_SPEED_LUA1: AddSpeedEffect( SE_LUA1, flEffectDuration, flSpeed, SEM_ACCUMULATIVE, -1, -1.0f, true ); break;
			case LUA_EF_SPEED_LUA2: AddSpeedEffect( SE_LUA2, flEffectDuration, flSpeed, SEM_ACCUMULATIVE, -1, -1.0f, true );  break;
			case LUA_EF_SPEED_LUA3: AddSpeedEffect( SE_LUA3, flEffectDuration, flSpeed, SEM_ACCUMULATIVE, -1, -1.0f, true ); break;
			case LUA_EF_SPEED_LUA4: AddSpeedEffect( SE_LUA4, flEffectDuration, flSpeed, SEM_ACCUMULATIVE, -1, -1.0f, true ); break;
			case LUA_EF_SPEED_LUA5: AddSpeedEffect( SE_LUA5, flEffectDuration, flSpeed, SEM_ACCUMULATIVE, -1, -1.0f, true ); break;
			case LUA_EF_SPEED_LUA6: AddSpeedEffect( SE_LUA6, flEffectDuration, flSpeed, SEM_ACCUMULATIVE, -1, -1.0f, true ); break;
			case LUA_EF_SPEED_LUA7: AddSpeedEffect( SE_LUA7, flEffectDuration, flSpeed, SEM_ACCUMULATIVE, -1, -1.0f, true ); break;
			case LUA_EF_SPEED_LUA8: AddSpeedEffect( SE_LUA8, flEffectDuration, flSpeed, SEM_ACCUMULATIVE, -1, -1.0f, true ); break;
			case LUA_EF_SPEED_LUA9: AddSpeedEffect( SE_LUA9, flEffectDuration, flSpeed, SEM_ACCUMULATIVE, -1, -1.0f, true ); break;
			case LUA_EF_SPEED_LUA10: AddSpeedEffect( SE_LUA10, flEffectDuration, flSpeed, SEM_ACCUMULATIVE, -1, -1.0f, true ); break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Let's lua see if an effect is active
//-----------------------------------------------------------------------------
bool CFFPlayer::LuaIsEffectActive( int iEffect )
{
	switch( iEffect )
	{
		case LUA_EF_ONFIRE: return ( m_iBurnTicks ? true : false ); break;
		case LUA_EF_CONC: return IsConcussed(); break;
		case LUA_EF_GAS: return IsGassed(); break;
		case LUA_EF_INFECT: return IsInfected(); break;
		case LUA_EF_RADIOTAG: return IsRadioTagged(); break;

		case LUA_EF_HEADSHOT:
		{
		}
		break;

		case LUA_EF_LEGSHOT: IsSpeedEffectSet( SE_LEGSHOT ); break;
		case LUA_EF_TRANQ: IsSpeedEffectSet( SE_TRANQ ); break;
		case LUA_EF_ACSPINUP: IsSpeedEffectSet( SE_ASSAULTCANNON ); break;
		case LUA_EF_SNIPERRIFLE: IsSpeedEffectSet( SE_SNIPERRIFLE ); break;
		case LUA_EF_SPEED_LUA1: IsSpeedEffectSet( SE_LUA1 ); break;
		case LUA_EF_SPEED_LUA2: IsSpeedEffectSet( SE_LUA1 ); break;
		case LUA_EF_SPEED_LUA3: IsSpeedEffectSet( SE_LUA1 ); break;
		case LUA_EF_SPEED_LUA4: IsSpeedEffectSet( SE_LUA1 ); break;
		case LUA_EF_SPEED_LUA5: IsSpeedEffectSet( SE_LUA1 ); break;
		case LUA_EF_SPEED_LUA6: IsSpeedEffectSet( SE_LUA1 ); break;
		case LUA_EF_SPEED_LUA7: IsSpeedEffectSet( SE_LUA1 ); break;
		case LUA_EF_SPEED_LUA8: IsSpeedEffectSet( SE_LUA1 ); break;
		case LUA_EF_SPEED_LUA9: IsSpeedEffectSet( SE_LUA1 ); break;
		case LUA_EF_SPEED_LUA10: IsSpeedEffectSet( SE_LUA1 ); break;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Let's lua remove an effect
//-----------------------------------------------------------------------------
void CFFPlayer::LuaRemoveEffect( int iEffect )
{
	switch( iEffect )
	{
		case LUA_EF_ONFIRE: Extinguish(); break;
		case LUA_EF_CONC: UnConcuss(); break;
		case LUA_EF_GAS: UnGas(); break;
		case LUA_EF_INFECT: Cure( NULL ); break;
		case LUA_EF_RADIOTAG: SetUnRadioTagged(); break;

		case LUA_EF_HEADSHOT:
		{
		}
		break;

		case LUA_EF_LEGSHOT: RemoveSpeedEffect( SE_LEGSHOT, true ); break;
		case LUA_EF_TRANQ: RemoveSpeedEffect( SE_TRANQ, true ); break;
		case LUA_EF_ACSPINUP: RemoveSpeedEffect( SE_ASSAULTCANNON, true ); break;
		case LUA_EF_SNIPERRIFLE: RemoveSpeedEffect( SE_SNIPERRIFLE, true ); break;
		case LUA_EF_SPEED_LUA1: RemoveSpeedEffect( SE_LUA1, true ); break;
		case LUA_EF_SPEED_LUA2: RemoveSpeedEffect( SE_LUA1, true ); break;
		case LUA_EF_SPEED_LUA3: RemoveSpeedEffect( SE_LUA1, true ); break;
		case LUA_EF_SPEED_LUA4: RemoveSpeedEffect( SE_LUA1, true ); break;
		case LUA_EF_SPEED_LUA5: RemoveSpeedEffect( SE_LUA1, true ); break;
		case LUA_EF_SPEED_LUA6: RemoveSpeedEffect( SE_LUA1, true ); break;
		case LUA_EF_SPEED_LUA7: RemoveSpeedEffect( SE_LUA1, true ); break;
		case LUA_EF_SPEED_LUA8: RemoveSpeedEffect( SE_LUA1, true ); break;
		case LUA_EF_SPEED_LUA9: RemoveSpeedEffect( SE_LUA1, true ); break;
		case LUA_EF_SPEED_LUA10: RemoveSpeedEffect( SE_LUA1, true ); break;
	}
}

void CFFPlayer::AddSpeedEffect(SpeedEffectType type, float duration, float speed, int mod, int iIcon, float flIconDuration, bool bLuaAdded)
{
	// find an open slot
	int i = 0;

	// Without boolean we default to accumulative, but warn anyway in case we just forgot
	// boolean = effect is either on or off. Accumulative = the more effect you get, the stronger the effect is (e.g. more caltrops = slower)
	Assert((mod & SEM_BOOLEAN)|(mod & SEM_ACCUMULATIVE));

	if (mod & SEM_BOOLEAN)
	{
		// Search for an already existing one to overwrite
		for( ; i < NUM_SPEED_EFFECTS; i++)
		{
			// we'll overwrite the old one
			if (m_vSpeedEffects[i].type == type)
				break;
		}

		// We didn't overwrite one, so lets find an empty spot!
		if( i == NUM_SPEED_EFFECTS )
		{
			// Gotta reset 0, too.
			i = 0;
			while( m_vSpeedEffects[ i ].active && ( i != NUM_SPEED_EFFECTS ) )
				++i;
		}
	}
	else // Accumulative
	{
		while (m_vSpeedEffects[i].active && (i != NUM_SPEED_EFFECTS))
			i++;
	}

	if (i == NUM_SPEED_EFFECTS)
	{
		Warning( "ERROR: Too many speed effects. Raise NUM_SPEED_EFFECTS\n" );
		return;
	}

	m_vSpeedEffects[i].active = true;
	m_vSpeedEffects[i].type = type;
	m_vSpeedEffects[i].startTime = gpGlobals->curtime;
	m_vSpeedEffects[i].endTime = gpGlobals->curtime + duration;
	m_vSpeedEffects[i].duration = duration;
	m_vSpeedEffects[i].speed = speed;
	m_vSpeedEffects[i].modifiers = mod;
	m_vSpeedEffects[i].bLuaEnforced = bLuaAdded;

	if( iIcon != -1 )
	{
		CSingleUserRecipientFilter user( ( CBasePlayer * )this );
		user.MakeReliable();

		UserMessageBegin( user, "StatusIconUpdate" );
			WRITE_BYTE( iIcon );
			WRITE_FLOAT( flIconDuration );
		MessageEnd();
	}
	
	// Set any player vars
	switch( m_vSpeedEffects[i].type )
	{
		case SE_TRANQ: m_bTranqed = true; break;
	}
	
	RecalculateSpeed();
}

bool CFFPlayer::IsSpeedEffectSet( SpeedEffectType type )
{
	bool bFound = false;

	for( int i = 0; ( i < NUM_SPEED_EFFECTS ) && !bFound; i++ )
	{
		if( ( m_vSpeedEffects[ i ].type == type ) && ( m_vSpeedEffects[ i ].active ) )
			bFound = true;
	}

	return bFound;
}

void CFFPlayer::RemoveSpeedEffect(SpeedEffectType type, bool bLuaAdded)
{
	bool bRemoved = false;
	// Remove speed effects with the certain name
	for (int i=0; i<NUM_SPEED_EFFECTS; i++)
	{
		if (m_vSpeedEffects[i].type == type)
		{
			// If the effect was added by lua...
			if( m_vSpeedEffects[i].bLuaEnforced )
			{
				// ... make sure lua is removing it
				if( bLuaAdded )
				{
					bRemoved = true;
					RemoveSpeedEffectByIndex( i );
				}				
			}
			// Effect not added by lua so remove anyway
			else
			{
				bRemoved = true;
				RemoveSpeedEffectByIndex( i );
			}
		}
	}

	if( !bRemoved )
		return;

	RecalculateSpeed();
}

void CFFPlayer::RemoveSpeedEffectByIndex( int iSpeedEffectIndex )
{
	if( iSpeedEffectIndex < 0 )
		return;
	if( iSpeedEffectIndex >= NUM_SPEED_EFFECTS )
		return;

	// If it's active, do stuff
	if( m_vSpeedEffects[ iSpeedEffectIndex ].active )
	{
		// Turn it off
		m_vSpeedEffects[ iSpeedEffectIndex ].active = false;
		
		// Set any player vars
		switch( m_vSpeedEffects[ iSpeedEffectIndex ].type )
		{
			case SE_TRANQ: m_bTranqed = false; break;
		}

		int iIcon = -1;

		// Get its icon
		switch( m_vSpeedEffects[ iSpeedEffectIndex ].type )
		{
			case SE_TRANQ: iIcon = FF_STATUSICON_TRANQUILIZED; break;
			case SE_LEGSHOT: iIcon = FF_STATUSICON_LEGINJURY; break;
		}

		// Remove its icon
		if( iIcon != -1 )
		{
			// Send message to remove
			CSingleUserRecipientFilter user( ( CBasePlayer * )this );
			user.MakeReliable();

			UserMessageBegin( user, "StatusIconUpdate" );
				WRITE_BYTE( iIcon );
				WRITE_FLOAT( 0.0f );
			MessageEnd();
		}

		int iViewEffect = -1;

		// Get its view effect
		switch( m_vSpeedEffects[ iSpeedEffectIndex ].type )
		{
			case SE_TRANQ: iViewEffect = FF_VIEWEFFECT_TRANQUILIZED; break;
		}

		// Remove its view effect
		if( iViewEffect != -1 )
		{
			// Send message to remove
			CSingleUserRecipientFilter user( ( CBasePlayer * )this );
			user.MakeReliable();

			UserMessageBegin( user, "FFViewEffect" );
				WRITE_BYTE( iViewEffect );
				WRITE_FLOAT( 0.0f );
			MessageEnd();
		}
	}
}

int CFFPlayer::ClearSpeedEffects(int mod)
{
	int iCount = 0;

	if (mod)
	{
		for (int i = 0; i < NUM_SPEED_EFFECTS; i++)
		{
			if (m_vSpeedEffects[i].modifiers & mod)
			{
				// if effect is active and wasn't added by lua, we can remove it
				if( m_vSpeedEffects[i].active && !m_vSpeedEffects[ i ].bLuaEnforced )
				{
					// Wow, I just spent an hour debugging why my speed
					// effect wasn't getting removed and it was because
					// of this i++ line.
					//i++;
					// I'm assuming you meant for it to be a counter... but it
					// was fucking up the loop. Same goes for the else case below.
					// Fixed now.

					iCount++;					
					RemoveSpeedEffectByIndex( i );
				}				
			}
		}
	}
	else
	{
		for (int i = 0; i < NUM_SPEED_EFFECTS; i++)
		{
			// No mod supplied so lets assume we want all reset
			if( m_vSpeedEffects[i].active )
			{
				RemoveSpeedEffectByIndex( i );
				iCount++;
			}
		}
	}

	RecalculateSpeed();

	return iCount;
}

void CFFPlayer::RecalculateSpeed( void )
{
	float flSpeed = 1.0f;

	// need this so we can slow down the player more if using the assault cannon
	bool bAssaultCannonIncluded = false;

	// go apply all speed effects
	for (int i = 0; i < NUM_SPEED_EFFECTS; i++)
	{
		if (m_vSpeedEffects[i].active)
		{
			flSpeed -= (1 - m_vSpeedEffects[i].speed);

			if( m_vSpeedEffects[i].type == SE_ASSAULTCANNON )
				bAssaultCannonIncluded = true;
		}		
	}
	
	// No amount of effects should let you go below 40% speed unless you're firing the AC
	if( bAssaultCannonIncluded ) 
	{
		// May as well clamp this at 0 for safety; dunno if it'll ever happen
		if( flSpeed < 0.0f )
			flSpeed = 0.0f;
	}
	else
	{
		// no AC effect so just clamp @ 40%
		if( flSpeed < 0.4f )
			flSpeed = 0.4f;
	}	

	// If speed has gotten slower then delay the max speed change on the server.
	// This way the client can predict in time that the speed has changed, and warping
	// won't happen.
	// This will break if two speed changes happen in quick succession but unless
	// that turns out to be a problem it's not worth worrying about.
	if (flSpeed < m_flSpeedModifier)
	{
		// Store off the old max speed for the server to use for the next few ms.
		m_flSpeedModifierOld = m_flSpeedModifier;

		// Work our approximately when the client will receive this speed change.
		m_flSpeedModifierChangeTime = gpGlobals->curtime + (0.002f * this->GetPing());
	}
	else
	{
		// Apply the speed change instantly. Warping won't occur because the client
		// is moving slower than the server's max speed.
		m_flSpeedModifierChangeTime = 0;
	}

	// While we've set the max speed now, it won't actually be used by the server
	// movement code until after m_flMaxspeedChangeTime. This allows the client to
	// predict the speed change in time and avoid any warping.
	m_flSpeedModifier = flSpeed;
}

//-----------------------------------------------------------------------------
// Purpose: Reloads clips that might be drained
//-----------------------------------------------------------------------------
void CFFPlayer::ReloadClips( void )
{
	if( IsAlive() )
	{
		for( int i = 0; i < WeaponCount(); i++ )
		{
			CFFWeaponBaseClip *pWeapon = dynamic_cast< CFFWeaponBaseClip * >( GetWeapon( i ) );
			if( pWeapon )
			{
				if( pWeapon->m_iClip1 < pWeapon->GetMaxClip1() )
					pWeapon->m_iClip1 = pWeapon->GetMaxClip1();
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get a player's Steam ID
//-----------------------------------------------------------------------------
const char *CFFPlayer::GetSteamID( void ) const
{
	if( engine )
		return engine->GetPlayerNetworkIDString( edict() );

	return "\0";
}

//-----------------------------------------------------------------------------
// Purpose: Get a player's ping
//-----------------------------------------------------------------------------
int CFFPlayer::GetPing( void ) const
{
	int iPing = 0, iPacketloss = 0;
	UTIL_GetPlayerConnectionInfo( entindex(), iPing, iPacketloss );

	return iPing;
}

//-----------------------------------------------------------------------------
// Purpose: Get a player's packetloss
//-----------------------------------------------------------------------------
int CFFPlayer::GetPacketloss( void ) const
{
	int iPing = 0, iPacketloss = 0;
	UTIL_GetPlayerConnectionInfo( entindex(), iPing, iPacketloss );

	return iPacketloss;
}

bool CFFPlayer::Infect( CFFPlayer *pInfector )
{
	if( !IsInfected() && !IsImmune() )
	{
		if( !FFScriptRunPredicates( this, "oninfect", true ) )
			return false;

		// they aren't infected or immune, so go ahead and infect them
		m_bInfected = true;
		m_iInfectTick = 0;
		m_fLastInfectedTick = gpGlobals->curtime;
		m_fNextInfectedTickDamage = FFDEV_INFECT_DAMAGE;
		m_nNumInfectDamage = 0;
		m_hInfector = pInfector;
		m_iInfectedTeam = pInfector->GetTeamNumber();

		EmitSound( "Player.cough" );	// |-- Mirv: [TODO] Change to something more suitable

		// And now.. an effect
		CSingleUserRecipientFilter user(this);
		user.MakeReliable();

		UserMessageBegin(user, "FFViewEffect");
		WRITE_BYTE(FF_VIEWEFFECT_INFECTED);
		WRITE_FLOAT(999.0f);
		MessageEnd();

		Omnibot::Notify_Infected(this, pInfector);

		return true;
	}

	else if ( !IsInfected() ) // they aren't infected, but they are immune
		FF_SendHint( pInfector, MEDIC_NOINFECT, 3, PRIORITY_NORMAL, "#FF_HINT_MEDIC_NOINFECT" );

	return false;	
}

bool CFFPlayer::Cure( CFFPlayer *pCurer )
{
	bool bCured = false;

	if( IsInfected() )
	{
		// they are infected, so go ahead and cure them
		m_bInfected = false;
		m_fLastInfectedTick = 0.0f;
		m_fNextInfectedTickDamage = 0.0f;
		m_hInfector = NULL;
		// Bug# 0000503: "Immunity" is not in the mod
		m_bImmune = true;
		m_flImmuneTime = gpGlobals->curtime + FFDEV_IMMUNE_TIME;
		m_iInfectTick = 0;

		// Send the status icon to the player
		CSingleUserRecipientFilter user( ( CBasePlayer * )this );
		user.MakeReliable();
		UserMessageBegin( user, "StatusIconUpdate" );
			WRITE_BYTE( FF_STATUSICON_IMMUNE );
			WRITE_FLOAT( 10.0f );
		MessageEnd();

		// Heal for the ammount lost, minus regain damage ( only if you have enough taken damage to subtract from )
		if( m_nNumInfectDamage > FFDEV_INFECT_REGAINDAMAGE )
		{
			AddHealth( m_nNumInfectDamage - FFDEV_INFECT_REGAINDAMAGE ) ;
		}
		
		//Reset the infect damamge counter
		m_nNumInfectDamage = 0;

		// credit the curer with a score
		if( pCurer )
			pCurer->AddFortPoints( 100, "#FF_FORTPOINTS_CUREINFECTION" );

		Omnibot::Notify_Cured(this, pCurer);

		bCured = true;
	}

	// Hack-ish - removing infection effect
	if( !pCurer )
	{
		m_bInfected = false;
		m_fLastInfectedTick = 0.0f;
		m_hInfector = NULL;
		m_iInfectTick = 0;
		m_nNumInfectDamage = 0;
	}

	// Bug #0000528: Medics can self-cure being caltropped/tranq'ed
	if (ClearSpeedEffects( SEM_HEALABLE ) > 0)
		bCured = true;

	if (IsOnFire())
		bCured = true;

	// Wiki says curing removes everything but concussion
	Extinguish();

	return bCured;
}

// scale = damage per tick :: Scale currently ignored - use cvars for weapon damage!
void CFFPlayer::ApplyBurning( CFFPlayer *hIgniter, float scale, float flIconDuration, eBurnType BurnType)
{
	// Okay, now pyros don't catch fire at all
	if (GetClassSlot() == CLASS_PYRO)
	{
		return;
	}

	// send the status icon to be displayed
	CSingleUserRecipientFilter user( (CBasePlayer *)this );
	user.MakeReliable();
	
	// set them on fire
	if (!m_iBurnTicks)
		m_flNextBurnTick = gpGlobals->curtime + BURN_TICK_INTERVAL;
	// multiply damage left to burn by number of remaining ticks, then divide it out among the new 8 ticks
	// This prevents damage being incorrectly multiplied - shok
	// ignore this now - instead we use burn levels and simply reset the timer
	
	//m_flBurningDamage = m_flBurningDamage + scale*((GetClassSlot()==CLASS_PYRO)?8.0:16.0);
	//m_iBurnTicks = (GetClassSlot()==CLASS_PYRO)?4:8;

	m_iBurnTicks = BURN_TICKS;
	int oldburnlevel = 0;
	if (m_bBurnFlagNG == true) 
		++oldburnlevel;
	if (m_bBurnFlagFT == true) 
		++oldburnlevel;
	if (m_bBurnFlagIC == true) 
		++oldburnlevel;

	switch (BurnType)
	{
		case BURNTYPE_NALPALMGRENADE: m_bBurnFlagNG = true; break;
		case BURNTYPE_FLAMETHROWER: m_bBurnFlagFT = true; break;
		case BURNTYPE_ICCANNON: m_bBurnFlagIC= true; break;
	}
	
	int newburnlevel = 0;
	if (m_bBurnFlagNG == true) 
		++newburnlevel;
	if (m_bBurnFlagFT == true) 
		++newburnlevel;
	if (m_bBurnFlagIC == true) 
		++newburnlevel;
	
	if(oldburnlevel != newburnlevel)
		Omnibot::Notify_BurnLevel(this, hIgniter, newburnlevel);

	// each weapons burn damage can only stack once. (else you set them on 999 fire with the FT)
	/** Uncomment this to use different burn damages depending on the weapon - AfterShock
	m_flBurningDamage = 0;
	if (m_bBurnFlagNG) 
		m_flBurningDamage += burn_damage_ng.GetFloat();
	if (m_bBurnFlagFT)
		m_flBurningDamage += burn_damage_ft.GetFloat();
	if (m_bBurnFlagIC)
		m_flBurningDamage += burn_damage_ic.GetFloat();
	*/
	// Else use this single value (from flamethrower) and multiply it by the burn multipliers
	m_flBurningDamage = BURN_DAMAGE_BASE * newburnlevel;
	//m_flBurningDamage = m_flBurningDamage + scale*((GetClassSlot()==CLASS_PYRO)?8.0:16.0);
	
	// if we're on fire from all 3 flame weapons, holy shit BURN! - shok
	if (newburnlevel == 3)
	{
		m_flBurningDamage *= BURN_MULTIPLIER_3BURNS;
		if (gpGlobals->curtime > m_flScreamTime + 1.7f)
		{
			EmitSound("Player.Scream"); // haha
			m_flScreamTime = gpGlobals->curtime;
		}
		if (oldburnlevel == 2) 
		{
			UserMessageBegin(user, "StatusIconUpdate");
				WRITE_BYTE( FF_STATUSICON_BURNING2 );
				WRITE_FLOAT( 0.0f );
			MessageEnd();
		}
		UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE( FF_STATUSICON_BURNING3 );
			WRITE_FLOAT( flIconDuration );
		MessageEnd();

		Ignite( 10.0, false, FFDEV_FLAMESIZE_BURN3, false );
	}
	// if we're on fire from 2 flame weapons, burn a bit more
	else if (newburnlevel == 2)
	{
		m_flBurningDamage *= BURN_MULTIPLIER_2BURNS;
		if (oldburnlevel == 1) 
		{
			UserMessageBegin(user, "StatusIconUpdate");
				WRITE_BYTE( FF_STATUSICON_BURNING1 );
				WRITE_FLOAT( 0.0f );
			MessageEnd();
		}
		UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE( FF_STATUSICON_BURNING2 );
			WRITE_FLOAT( flIconDuration );
		MessageEnd();

		Ignite( 10.0, false, FFDEV_FLAMESIZE_BURN2, false );
	}
	else // burn level 1
	{
		UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE( FF_STATUSICON_BURNING1 );
			WRITE_FLOAT( flIconDuration );
		MessageEnd();

		Ignite( 10.0, false, FFDEV_FLAMESIZE_BURN1, false );
	}

	DevMsg("Burn: %f",m_flBurningDamage);

	m_BurnType = BurnType;

	/*
	this may cause less damage to happen..
	implemented differently above - fryguy 

	// --> Mirv: Pyros safer against fire
	//	# 50% damage reduction when being hurt by fire.
	//	# Once on fire, burn time reduced by 50%.
	if (GetClassSlot() == CLASS_PYRO)
	{
	m_iBurnTicks *= 0.5f;
	m_flBurningDamage *= 0.5f;
	}
	// <-- Mirv: Pyros safer against fire
	*/

	// set up the igniter
	m_hIgniter = hIgniter;
}

// Toggle grenades (requested by defrag)
void CFFPlayer::Command_ToggleOne( void )
{
	if( IsGrenadePrimed() )
		Command_ThrowGren();
	else
		Command_PrimeOne();
}

void CFFPlayer::Command_ToggleTwo( void )
{
	if( IsGrenadePrimed() )
		Command_ThrowGren();
	else
		Command_PrimeTwo();
}

void CFFPlayer::Command_PrimeOne(void)
{
	if (IsGrenadePrimed())
		return;

	if( GetFlags() & FL_FROZEN )
		return;

	// Can't throw grenade while building
	if( m_bStaticBuilding )
		return;

	// Bug #0000366: Spy's cloaking & grenade quirks
	// Spy shouldn't be able to prime grenades when Cloaked
	if (IsCloaked())
		return;

	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	// we have a primary grenade type
	if ( strcmp( pPlayerClassInfo.m_szPrimaryClassName, "None" ) != 0 )
	{
		if(m_iPrimary > 0)
		{
			// ax1
			EmitSoundShared("Grenade.Prime"); // This is mirrored in CC_PrimeOne for client

			// Jiggles: Added lua callback for squeek's training map
			CFFLuaSC hContext( 1, this );
			_scriptman.RunPredicates_LUA( NULL, &hContext, "player_onprimegren1" );
			// End callback

			m_iGrenadeState = FF_GREN_PRIMEONE;
			m_flServerPrimeTime = gpGlobals->curtime;
#ifndef _DEBUG
			m_iPrimary--;
		}
		else
		{
			DevMsg("[Grenades] You are out of primary grenades!\n");
#endif
		}
	}
}

void CFFPlayer::Command_PrimeTwo(void)
{
	if (IsGrenadePrimed())
		return;

	if( GetFlags() & FL_FROZEN )
		return;

	// Can't throw grenade while building
	if( m_bStaticBuilding )
		return;

	// Bug #0000366: Spy's cloaking & grenade quirks
	// Spy shouldn't be able to prime grenades when Cloaked
	if (IsCloaked())
		return;

    const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	// we have a secondary grenade type
	if ( strcmp( pPlayerClassInfo.m_szSecondaryClassName, "None" ) != 0 )
	{
		if(m_iSecondary > 0)
		{
			// ax1
			EmitSoundShared("Grenade.Prime"); // This is mirrored in CC_PrimeTwo for client

			// Jiggles: Added lua callback for squeek's training map
			CFFLuaSC hContext( 1, this );
			_scriptman.RunPredicates_LUA( NULL, &hContext, "player_onprimegren2" );
			// End callback

			// Hint Code
			//Msg("\nSecondary Class Name: %s\n", pPlayerClassInfo.m_szSecondaryClassName );
			//if ( strcmp( pPlayerClassInfo.m_szSecondaryClassName, "ff_grenade_nail" ) == 0 )
			//	FF_SendHint( this, SOLDIER_NAILGREN, 4, PRIORITY_NORMAL, "#FF_HINT_SOLDIER_NAILGREN" );
			//else if ( strcmp( pPlayerClassInfo.m_szSecondaryClassName, "ff_grenade_concussion" ) == 0 )
			//	FF_SendHint( this, SCOUT_CONC1, 1, PRIORITY_NORMAL, "#FF_HINT_SCOUT_CONC1" );

			// Jiggles: Let's try it this way to avoid the above string compares
				switch( GetClassSlot() )
				{
					case CLASS_SOLDIER: 
						FF_SendHint( this, SOLDIER_LASERGREN, 1, PRIORITY_NORMAL, "#FF_HINT_SOLDIER_LASERGREN" );
						break;
					case CLASS_MEDIC:
					case CLASS_SCOUT:
						FF_SendHint( this, SCOUT_CONC1, 1, PRIORITY_NORMAL, "#FF_HINT_SCOUT_CONC1" );
						break;
				}
			// End hint code

			m_iGrenadeState = FF_GREN_PRIMETWO;
			m_flServerPrimeTime = gpGlobals->curtime;
#ifndef _DEBUG
			m_iSecondary--;
		}
		else
		{
			DevMsg("[Grenades] You are out of secondary grenades!\n");
#endif
		}
	}
}

void CFFPlayer::Command_ThrowGren(void)
{
	if (!IsGrenadePrimed())
		return;

	// ted_maul: 0000614: Grenade timer issues
	// release delay
	if(gpGlobals->curtime - m_flServerPrimeTime < GREN_THROW_DELAY)
	{
		// release this grenade at the earliest opportunity
		m_bWantToThrowGrenade = true;
		return;
	}

	bool bThrowGrenade = true;
	float fPrimeTimer = GREN_TIMER - (gpGlobals->curtime - m_flServerPrimeTime);

	// Give lua the chance to override grenade throwing.
	// It should return false to avoid throwing the grenade
	CFFLuaSC hContext( 1, this );
	hContext.Push(1.0f - (fPrimeTimer / GREN_TIMER));
	
	const char *pLuaFn = 0;
	if(m_iGrenadeState == FF_GREN_PRIMEONE)
		pLuaFn = "player_onthrowgren1";
	else if(m_iGrenadeState == FF_GREN_PRIMETWO)
		pLuaFn = "player_onthrowgren2";	
	if( pLuaFn && _scriptman.RunPredicates_LUA( NULL, &hContext, pLuaFn ) )
		bThrowGrenade = hContext.GetBool();

	if(bThrowGrenade)
		ThrowGrenade(fPrimeTimer);
	m_bWantToThrowGrenade = false;
	m_iGrenadeState = FF_GREN_NONE;
	m_flServerPrimeTime = 0.0f;
	m_bEngyGrenWarned = false;
}

int CFFPlayer::GetPrimaryGrenades( void )
{
	return m_iPrimary;
}

int CFFPlayer::GetSecondaryGrenades( void )
{
	return m_iSecondary;
}

void CFFPlayer::SetPrimaryGrenades( int iNewCount )
{
	const CFFPlayerClassInfo &info = GetFFClassData();
	m_iPrimary = clamp(iNewCount, 0, info.m_iPrimaryMax);
}

void CFFPlayer::SetSecondaryGrenades( int iNewCount )
{
	const CFFPlayerClassInfo &info = GetFFClassData();
	m_iSecondary = clamp(iNewCount, 0, info.m_iSecondaryMax);
}

int CFFPlayer::AddPrimaryGrenades( int iNewCount )
{
	const CFFPlayerClassInfo &info = GetFFClassData();
	int ret = clamp(info.m_iPrimaryMax - m_iPrimary, 0, iNewCount);
	m_iPrimary = clamp(m_iPrimary+iNewCount, 0, info.m_iPrimaryMax);
	return ret;
}

int CFFPlayer::AddSecondaryGrenades( int iNewCount )
{
	const CFFPlayerClassInfo &info = GetFFClassData();
	int ret = clamp(info.m_iSecondaryMax - m_iSecondary, 0, iNewCount);
	m_iSecondary = clamp(m_iSecondary+iNewCount, 0, info.m_iSecondaryMax);
	return ret;
}

bool CFFPlayer::IsGrenade1Primed()
{
	return ( ( m_iGrenadeState == FF_GREN_PRIMEONE ) );
}
bool CFFPlayer::IsGrenade2Primed()
{
	return ( ( m_iGrenadeState == FF_GREN_PRIMETWO ) );
}
bool CFFPlayer::IsGrenadePrimed()
{
	return IsGrenade1Primed() || IsGrenade2Primed();
}

void CFFPlayer::GrenadeThink(void)
{
	if (!IsGrenadePrimed())
		return;

	// Bug #0000993: Holding(HHing) an emp bugs the pre-det sound
	// Because the grenade doesn't actually exist yet for the bug,
	// we've gotta play the sound here if applicable
	if( ( GetClassSlot() == CLASS_ENGINEER ) && ( m_iGrenadeState == FF_GREN_PRIMETWO ) )
	{
		if( !m_bEngyGrenWarned && ( gpGlobals->curtime > ( m_flServerPrimeTime + GREN_TIMER - 0.685f ) ) )
		{
			m_bEngyGrenWarned = true;
			EmitSound( EMP_SOUND );
		}
	}	

	if(m_bWantToThrowGrenade && gpGlobals->curtime - m_flServerPrimeTime >= GREN_THROW_DELAY)
	{
		Command_ThrowGren();
		return;
	}

	if ( (m_flServerPrimeTime != 0 ) && ( ( gpGlobals->curtime - m_flServerPrimeTime ) >= GREN_TIMER ) )
	{
		ThrowGrenade(0); // "throw" a grenade that immediately explodes at the player's origin
		m_iGrenadeState = FF_GREN_NONE;
		m_flServerPrimeTime = 0;
		m_bEngyGrenWarned = false;
	}
}

void CFFPlayer::ThrowGrenade(float fTimer, float flSpeed)
{
	if (!IsGrenadePrimed())
		return;

	// This is our player details
	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	// For if we make a grenade
	CFFGrenadeBase *pGrenade = NULL;

	// Check which grenade we have to do
	switch (m_iGrenadeState)
	{
		case FF_GREN_PRIMEONE:
			
			// They don't actually have a primary grenade
			if( Q_strcmp( pPlayerClassInfo.m_szPrimaryClassName, "None" ) == 0 )
				return;

			// Make the grenade
			pGrenade = (CFFGrenadeBase *) CreateEntityByName(pPlayerClassInfo.m_szPrimaryClassName);
			break;

		case FF_GREN_PRIMETWO:

			// They don't actually have a secondary grenade
			if (Q_strcmp( pPlayerClassInfo.m_szSecondaryClassName, "None") == 0)
				return;

			// Make the grenade
			pGrenade = (CFFGrenadeBase *) CreateEntityByName(pPlayerClassInfo.m_szSecondaryClassName);			
			break;
	}

	// So we made a grenade
	if (pGrenade != NULL)
	{
		Vector vecForward, vecSrc, vecVelocity;
		QAngle angAngles;

		EyeVectors(&vecForward);

		// Mirv: Grenade should always come from the waist, tfc-style
		vecSrc = GetAbsOrigin();

		VectorAngles( vecForward, angAngles );
		angAngles.x -= GREN_SPAWN_ANG_X;

		UTIL_SetOrigin(pGrenade, vecSrc);

		// Stationary
		if (fTimer != 0)
		{
			AngleVectors(angAngles, &vecVelocity);
			VectorNormalize(vecVelocity);
			if (flSpeed > 0)
				vecVelocity *= GREN_THROWSPEED; // flSpeed;
			else
				vecVelocity *= flSpeed; // |-- Mirv: So we can drop grenades
		}
		else
			vecVelocity = Vector(0, 0, 0);

		pGrenade->Spawn();
		pGrenade->SetAbsVelocity(vecVelocity);
		pGrenade->SetThrower(this);
		pGrenade->SetOwnerEntity(this);
		pGrenade->ChangeTeam(GetTeamNumber());

		pGrenade->SetDetonateTimerLength( fTimer );
		pGrenade->m_flSpawnTime = gpGlobals->curtime - (3.0f - fTimer);	// This shold be done in a neater way!!
		pGrenade->SetupInitialTransmittedVelocity(vecVelocity);

		// Special case for emps since their explode sound starts before it actually explodes
		if( ( GetClassSlot() == CLASS_ENGINEER ) && ( m_bEngyGrenWarned ) && ( pGrenade->Classify() == CLASS_GREN_EMP ) )
			dynamic_cast< CFFGrenadeEmp * >( pGrenade )->SetWarned();

		if (fTimer > 0)
			pGrenade->m_fIsHandheld = false;

#ifdef GAME_DLL
		if(m_iGrenadeState == FF_GREN_PRIMEONE)
			Omnibot::Notify_PlayerShoot(this, Omnibot::TF_WP_GRENADE1, pGrenade);
		else if(m_iGrenadeState == FF_GREN_PRIMETWO)
			Omnibot::Notify_PlayerShoot(this, Omnibot::TF_WP_GRENADE2, pGrenade);
#endif
	}
}

//-----------------------------------------------------------------------------
const char* CFFPlayer::GetActiveWeaponName() const
{
	const char* szWeaponName = NULL;

	CBaseCombatWeapon *weapon = GetActiveWeapon();
    if(NULL != weapon)
		szWeaponName = weapon->GetName();

	return szWeaponName;
}

//-----------------------------------------------------------------------------
// Purpose: Remove primed grenades
//-----------------------------------------------------------------------------
void CFFPlayer::RemovePrimedGrenades( void )
{
	if( IsGrenadePrimed() )
	{
		m_flServerPrimeTime = 0.0f;
		m_iGrenadeState = FF_GREN_NONE;
		m_bEngyGrenWarned = false;

		// dexter: stop all the timers beepin' away man
		FF_SendStopGrenTimerMessage(this);
	}
}

void CFFPlayer::PackDeadPlayerItems( void )
{
	// Remove everything
	RemoveAllItems( true );
}

// Taken from player.cpp
static float DamageForce( const Vector &size, float damage )
{ 
	float force = damage * ((32 * 32 * 72.0) / (size.x * size.y * size.z)) * 5;

	if ( force > 1000.0) 
	{
		force = 1000.0;
	}

	return force;
}

int CFFPlayer::OnTakeDamage(const CTakeDamageInfo &inputInfo)
{
	// have suit diagnose the problem - ie: report damage type
	int bitsDamage = inputInfo.GetDamageType();
	int fTookDamage;

	CTakeDamageInfo info = inputInfo;

	IServerVehicle *pVehicle = GetVehicle();
	if ( pVehicle )
	{
		// Players don't take blast or radiation damage while in vehicles.
		// The vehicle has to deal it to him
		if ( info.GetDamageType() & (DMG_BLAST|DMG_RADIATION) )
			return 0;

		info.ScaleDamage(pVehicle->DamageModifier(info));
	}

	if ( GetFlags() & FL_GODMODE )
		return 0;

	if ( m_debugOverlays & OVERLAY_BUDDHA_MODE ) 
	{
		if ((m_iHealth - info.GetDamage()) <= 0)
		{
			m_iHealth = 1;
			return 0;
		}
	}

	// Early out if there's no damage
	if ( !info.GetDamage() )
		return 0;

	// Already dead
	if ( !IsAlive() )
		return 0;

	// call script: player_ondamage(player, damageinfo)	
	CFFLuaSC func;
	func.Push(this);
	func.PushRef(info);
	func.CallFunction("player_ondamage");

	// Bug #0000781: Placing a detpack can be interrupted
	if( !m_bStaticBuilding )
	{
		// We need to apply the force first (since you should move a bit when damage is reduced)
		ApplyAbsVelocityImpulse( info.GetDamageForce() );
	}	

	// Don't need this anymore due to Olah's new stuff!
	//entsys.SetVar("info_damage", info.GetDamage());
	//entsys.SetVar("info_attacker", ENTINDEX(info.GetAttacker()));
	//entsys.SetVar("info_classname", info.GetInflictor()->GetClassname());
    //CFFPlayer *player = ToFFPlayer(info.GetInflictor());
    //if (player)
    //{
      //  CBaseCombatWeapon *weapon = player->GetActiveWeapon();
        //if (weapon)
		//	entsys.SetVar("info_classname", weapon->GetName());
	//}

	// go take the damage first
	if ( !g_pGameRules->FCanTakeDamage( this, info.GetAttacker() ) )
	{
        // Refuse the damage
		return 0;
	}

	// Check for radio tag shots
	if( inputInfo.GetInflictor() )
	{
		CFFWeaponBase *pWeapon = dynamic_cast<CFFWeaponBase *>( inputInfo.GetInflictor() );
		if( pWeapon && (pWeapon->GetWeaponID() == FF_WEAPON_SNIPERRIFLE) )
		{
			CFFPlayer *pAttacker = ToFFPlayer( info.GetAttacker() );
			if( pAttacker )
			{
				SetRadioTagged( pAttacker, gpGlobals->curtime, RADIOTAG_DRAW_DURATION );

				// AfterShock - Scoring system: 10 points for a radiotag (if not already tagged)
				// This could be editted later to give points for a renewed tag
				if (!IsRadioTagged())
					pAttacker->AddFortPoints(10,"#FF_FORTPOINTS_RADIOTAG");
			}			
		}
	}

	// check to see if the shield should block this incoming damage

		//Example code for how to use dot product to find angle between things -GreenMushy
		//// get the displacement between the players
		//Vector vDisplacement = pTarget->GetAbsOrigin() - pPlayer->GetAbsOrigin();
		//vDisplacement.z = 0;
		//vDisplacement.NormalizeInPlace();

		//// get the direction the target is facing
		//Vector vFacing;
		//AngleVectors(pTarget->GetLocalAngles(), &vFacing);
		//vFacing.z = 0;
		//vFacing.NormalizeInPlace();

		//// see if they are facing the same direction
		//float angle = vFacing.Dot(vDisplacement);
		//if (angle > ffdev_knife_backstab_angle.GetFloat() )
		//{
	
	//// tag the player if hit by radio tag ammo 
	//if( inputInfo.GetAmmoType() == m_iRadioTaggedAmmoIndex )
	//{
	//	//CFFSentryGun *pSentryGun = FF_ToSentrygun( pObject );
	//	CFFPlayer *pAttacker = ToFFPlayer( info.GetAttacker() );
	//	SetRadioTagged( pAttacker, gpGlobals->curtime, radiotag_draw_duration.GetInt() );
	//	
	//	// AfterShock - Scoring system: 10 points for a radiotag (if not already tagged)
	//	// This could be editted later to give points for a renewed tag
	//	if ((!IsRadioTagged()) && (pAttacker))
	//		pAttacker->AddFortPoints(10,true);
	//}

	// if it's a pyro, they take half damage
	// AfterShock: pyros now take full damage. They can't be lit on fire tho, and thats a big enough differnce!
	/*
	if ( GetClassSlot() == CLASS_PYRO && info.GetDamageType()&DMG_BURN )
	{
		info.SetDamage(info.GetDamage()/1.3); //Instead of taking 50% damage, pyros now take 75% - AfterShock
	}
	*/

	// keep track of amount of damage last sustained
	m_lastDamageAmount = info.GetDamage();

	// Armor. 
	if (!(info.GetDamageType() & (DMG_FALL | DMG_DROWN | DMG_POISON | DMG_RADIATION | DMG_DIRECT)))// armor doesn't protect against fall or drown damage!
	{
		//float flNew = info.GetDamage() * flRatio;
		float fFullDamage = info.GetDamage();

		float fArmorDamage = fFullDamage * (((float)m_iArmorType) / 10.0f); //AfterShock: changing int to float e.g. armor type 8 means 0.8 i.e. 80% damage absorbed by armor
		float fHealthDamage = fFullDamage - fArmorDamage;
		float fArmorLeft = (float) m_iArmor;

		// if the armor damage is greater than the amount of armor remaining, apply the excess straight to health
		if(fArmorDamage > fArmorLeft)
		{
			fHealthDamage += fArmorDamage - fArmorLeft;
			fArmorDamage = fArmorLeft;
			m_iArmor = 0;
		}
		else
		{
			m_iArmor -= (int) fArmorDamage;
		}

		// Set armor lost for hud "damage" message
		m_DmgSave = fArmorDamage;

		info.SetDamage(fHealthDamage);
	}

	// Don't call up the baseclass, it does all this again
	// Call instead the CBaseCombatChracter one which actually applies the damage
	fTookDamage = CBaseCombatCharacter::OnTakeDamage( info );

	// Early out if the base class took no damage
	if ( !fTookDamage )
		return 0;

	// AfterShock - Reset sabotage timer on getting shot
	SpyStopSabotaging();

// COMMENTED - NOT READY YET (not fully tested)
	// AfterShock: slow the player depending on how much damage they took, down to a minimum of standard run speed - mostly useful for slowing bhoppers and concers

	// get damage that they took
	// get their current velocity vector and horizontal speed
	// get their class max run speed
	// get the maxdamage cvar 
	// if their horizontal speed is less than max run speed then skipfunction
	
	if( inputInfo.GetInflictor() )
	{
		float damageForFullSlow;

		CFFBuildableObject *pBuildable = dynamic_cast <CFFBuildableObject *> (info.GetInflictor());
		if( pBuildable && ( pBuildable->Classify() == CLASS_SENTRYGUN ) ) // Buildables e.g. SGs use a different slow value?
		{
			damageForFullSlow = FFDEV_DMGFORFULLSLOW_SG;
		}
		else
		{
			damageForFullSlow = FFDEV_DMGFORFULLSLOW;
		}
		Vector vecVelocity = GetAbsVelocity();
		Vector vecLatVelocity = vecVelocity * Vector(1.0f, 1.0f, 0.0f);
		float flHorizontalSpeed = vecLatVelocity.Length();
		float flMaxSpeed = MaxSpeed();

		// dont slow on team damage or self damage, else we couldnt rocket jump or boost!
		if ( !g_pGameRules->PlayerRelationship( this, ToFFPlayer( info.GetAttacker() ) ) == GR_TEAMMATE ) 
		{
			if (flHorizontalSpeed > flMaxSpeed)
			{
				// excess speed is the speed over the maxrunspeed 
				// e.g. hit for 50 dmg, maxdmg is 100, speed is 1000, maxrun is 300
				// so we want 50/100 = 0.5 * 1000-300 = 350, take that away from current speed = 650 speed after shot
				float fLateral;
				if (m_lastDamageAmount > damageForFullSlow) //
				{
					// just set their speed to max run speed
					fLateral = flMaxSpeed / flHorizontalSpeed;
				}
				else
				{
					// new speed is current speed minus a fraction of the speed above the cap
					fLateral = flHorizontalSpeed - ((flHorizontalSpeed - flMaxSpeed) * (m_lastDamageAmount / damageForFullSlow));
					fLateral = fLateral / flHorizontalSpeed;
				}
				SetAbsVelocity(Vector(vecVelocity.x * fLateral, vecVelocity.y * fLateral, vecVelocity.z));					
			}
		}
	}


	// add to the damage total for clients, which will be sent as a single
	// message at the end of the frame
	// todo: remove after combining shotgun blasts?
	if ( info.GetInflictor() && info.GetInflictor()->edict() )
		m_DmgOrigin = info.GetInflictor()->GetAbsOrigin();

	// Set health lost for hud "damage" message
	m_DmgTake += (int)info.GetDamage();	

	// reset damage time countdown for each type of time based damage player just sustained

	for (int i = 0; i < CDMG_TIMEBASED; i++)
	{
		if (info.GetDamageType() & (DMG_PARALYZE << i))
		{
			m_rgbTimeBasedDamage[i] = 0;
		}
	}

	// Display any effect associate with this damage type
	DamageEffect(info.GetDamage(),bitsDamage);

	// Emit a pain sound but not when we're falling, because that is already handled
	if (IsAlive() && !(info.GetDamageType() & DMG_FALL))
	{
		EmitSound("Player.Pain");
	}
	
	// Send hit indicator to attacker
	CFFPlayer *pAttacker = ToFFPlayer( info.GetAttacker() );
	if( pAttacker && pAttacker != this )
	{
		pAttacker->m_flHitTime = gpGlobals->curtime;
	}

	m_bitsDamageType |= bitsDamage; // Save this so we can report it to the client
	m_bitsHUDDamage = -1;  // make sure the damage bits get resent

	return fTookDamage;
}
// ---> end

//-----------------------------------------------------------------------------
// Purpose: Remove radio tagging
//-----------------------------------------------------------------------------
void CFFPlayer::SetUnRadioTagged( void )
{
	m_bRadioTagged = false;
	m_pWhoTaggedMe = NULL;
	m_flRadioTaggedStartTime = 0.0f;
	m_flRadioTaggedDuration = 0.0f;	

	// Send status icon
	CSingleUserRecipientFilter user( ( CBasePlayer * )this );
	user.MakeReliable();

	UserMessageBegin( user, "StatusIconUpdate" );
	WRITE_BYTE( FF_STATUSICON_RADIOTAG );
	WRITE_FLOAT( 0.0f );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Set a player as being "radio tagged"
//-----------------------------------------------------------------------------
void CFFPlayer::SetRadioTagged( CFFPlayer *pWhoTaggedMe, float flStartTime, float flDuration )
{
	m_bRadioTagged = true;
	m_pWhoTaggedMe = pWhoTaggedMe;
	m_flRadioTaggedStartTime = flStartTime;
	m_flRadioTaggedDuration = flDuration;

	// Send status icon
	CSingleUserRecipientFilter user( ( CBasePlayer * )this );
	user.MakeReliable();

	UserMessageBegin( user, "StatusIconUpdate" );
	WRITE_BYTE( FF_STATUSICON_RADIOTAG );
	WRITE_FLOAT( flDuration );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Get the team number of the player who last radio tagged us
//-----------------------------------------------------------------------------
int CFFPlayer::GetTeamNumOfWhoTaggedMe( void ) const
{
	if( m_pWhoTaggedMe != NULL )
	{
		CBaseEntity *pEntity = ( CBaseEntity * )m_pWhoTaggedMe;
		if( pEntity && pEntity->IsPlayer() )
		{
			CFFPlayer *pPlayer = ToFFPlayer( pEntity );
			if( !pPlayer->IsObserver() )
				return pPlayer->GetTeamNumber();
		}
	}

	return TEAM_UNASSIGNED;
}

//-----------------------------------------------------------------------------
// Purpose: Get the player who tagged us
//-----------------------------------------------------------------------------
CFFPlayer *CFFPlayer::GetPlayerWhoTaggedMe( void )
{
	if( m_pWhoTaggedMe != NULL )
	{
		CBaseEntity *pEntity = ( CBaseEntity * )m_pWhoTaggedMe;
		if( pEntity && pEntity->IsPlayer() )
			return ToFFPlayer( pEntity );
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Get the player who set us on fire!
//-----------------------------------------------------------------------------
CFFPlayer *CFFPlayer::GetIgniter( void )
{
	if( m_hIgniter != NULL )
	{
		CBaseEntity *pEntity = ( CBaseEntity * )m_hIgniter;
		if( pEntity && pEntity->IsPlayer() )
			return ToFFPlayer( pEntity );
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Get the player who last gassed us
//-----------------------------------------------------------------------------
CFFPlayer *CFFPlayer::GetGasser( void )
{
	if( m_hGasser != NULL )
	{
		CBaseEntity *pEntity = ( CBaseEntity * )m_hGasser;
		if( pEntity && pEntity->IsPlayer() )
			return ToFFPlayer( pEntity );
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Overrided in order to let the explosion force actually be of
//			TFC proportions, also lets people lose limbs when needed
//-----------------------------------------------------------------------------
int CFFPlayer::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
	// set damage type sustained
	m_bitsDamageType |= info.GetDamageType();

	// Skip the CBasePlayer one because it sucks
	if (!CBaseCombatCharacter::OnTakeDamage_Alive(info))
		return 0;

	// Don't bother with decaps this time
	if (m_iHealth > 0)
	{
		m_fBodygroupState = 0;
	}
	// Only explosions now okay.
	else if (m_iHealth < 50.0f /*&& info.GetDamageType() & DMG_BLAST*/)
	{
		LimbDecapitation(info);
	}

	CBaseEntity * attacker = info.GetAttacker();

	if (!attacker)
		return 0;
	
	// Jiggles: Hint Code
	//			Event: Pyro takes damage from enemy Hwguy
	if ( m_bACDamageHint )
	{
		CFFPlayer *pAttacker = ToFFPlayer( attacker );
		if ( ( GetClassSlot() == CLASS_PYRO ) && pAttacker && ( pAttacker->GetClassSlot() == CLASS_HWGUY )  )
		{
			FF_SendHint( this, PYRO_ROASTHW, 1, PRIORITY_NORMAL, "#FF_HINT_PYRO_ROASTHW" );
			m_bACDamageHint = false; // Only do this hint once -- we don't want this hint sent every time this function is triggered!
		}
	}

	//	Event: Spy, while cloaked, takes damage from SG
	if ( m_bSGDamageHint && IsCloaked() && ( attacker->Classify() == CLASS_SENTRYGUN )  )
	{
		FF_SendHint( this, SPY_SGCLOAK, 1, PRIORITY_NORMAL, "#FF_HINT_SPY_SGCLOAK" );
		m_bSGDamageHint = false; // Only do this hint once -- we don't want this hint sent every time this function is triggered!
	}
	// End hint code

	// Apply the force needed
	// (commented this out cause we're doing it in the above function)
	//DevMsg("Applying impulse force of: %f\n", info.GetDamageForce().Length());
	//ApplyAbsVelocityImpulse(info.GetDamageForce());

	// fire global game event
	IGameEvent * pEvent = gameeventmanager->CreateEvent("player_hurt");
	if (pEvent)
	{
		pEvent->SetInt("userid", GetUserID());
		pEvent->SetInt("health", (m_iHealth > 0 ? m_iHealth : 0));	// max replaced with this
		if (attacker->IsPlayer())
		{
			CBasePlayer *player = ToBasePlayer(attacker);
			pEvent->SetInt("attacker", player->GetUserID()); // hurt by other player
		}
		else
		{
			pEvent->SetInt("attacker", 0); // hurt by "world"
		}

		gameeventmanager->FireEvent(pEvent, true);
	}

	Omnibot::Notify_Hurt(this, attacker);

	return 1;
}

int CFFPlayer::ObjectCaps( void ) 
{ 
	int iCaps = BaseClass::ObjectCaps();
	if(IsBot())
		 iCaps |= FCAP_IMPULSE_USE;
	return iCaps; 
}

//-----------------------------------------------------------------------------
// Purpose: Depending on the origin, chop stuff off
//			This is a pretty hacky method!
//-----------------------------------------------------------------------------
void CFFPlayer::LimbDecapitation(const CTakeDamageInfo &info)
{
	// Headshot
	if (info.GetCustomKill() == KILLTYPE_HEADSHOT)
	{
		m_fBodygroupState = DECAP_HEAD;
		return;
	}

	// For now the rest of this depends on explosions
	if (!(info.GetDamageType() & DMG_BLAST))
		return;

	// In which direction from the player was the explosion?
	Vector direction = info.GetDamagePosition() - BodyTarget(info.GetDamagePosition(), false);
	VectorNormalize(direction);

	// And which way is the player facing
	Vector	vForward, vRight, vUp;
	EyeVectors(&vForward, &vRight, &vUp);

	// Use rightarm to work out which side of the player it is on and use the
	// absolute upwards direction to work out whether it was above or below
	float dp_rightarm = direction.Dot(vRight);
	float dp_head = direction.Dot(Vector(0.0f, 0.0f, 1.0f));

	// Now check whether the explosion seems to be on the right or left side
	if (dp_rightarm > 0.6f)
	{
		m_fBodygroupState |= DECAP_RIGHT_ARM;
	}
	else if (dp_rightarm < -0.6f)
	{
		m_fBodygroupState |= DECAP_LEFT_ARM;
	}

	// Now check if the explosion seems to be above or below
	if (dp_head > 0.7f)
	{
		m_fBodygroupState |= DECAP_HEAD;
	}
	else if (dp_head < -0.6f)
	{
		// If they lost an arm on one side don't lose the leg on the other
		if (! (m_fBodygroupState & DECAP_RIGHT_ARM))
			m_fBodygroupState |= DECAP_LEFT_LEG;

		if (! (m_fBodygroupState & DECAP_LEFT_ARM))
			m_fBodygroupState |= DECAP_RIGHT_LEG;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Create limbs for bitfield iLimbs
//-----------------------------------------------------------------------------
void CFFPlayer::CreateLimbs(int iLimbs)
{
	// Now spawn any limbs that are needed
	// [TODO] This is not a great way to do it okay!
	for (int i = 0; i <= 4; i++)
	{
		if (iLimbs & (1 << i))
		{
			CFFRagdoll *pRagdoll = dynamic_cast< CFFRagdoll * > (CreateEntityByName("ff_ragdoll"));

			if (!pRagdoll)
			{
				Warning("Couldn't make limb ragdoll!\n");
				return;
			}

			if (pRagdoll)
			{
				Vector vecPosition, vecDirection;
				QAngle angAngle;

				GetAttachment(i, vecPosition, angAngle);
				AngleVectors(angAngle, &vecDirection);

				// Don't associate with a player, this is a quick fix to the ragdoll
				// limbs trying to snatch a player model instance.
				pRagdoll->m_hPlayer = NULL;	
				pRagdoll->m_vecRagdollOrigin = vecPosition;
				pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
				pRagdoll->m_nModelIndex = g_iLimbs[GetClassSlot()][i];
				pRagdoll->m_nForceBone = m_nForceBone;
				pRagdoll->m_vecForce = Vector(0, 0, 0);
				pRagdoll->m_fBodygroupState = 0;
				pRagdoll->m_nSkinIndex = m_nSkin;

				// Remove it after a time
				pRagdoll->SetThink(&CBaseEntity::SUB_Remove);
				pRagdoll->SetNextThink(gpGlobals->curtime + 10.0f);

				// Some blood
				UTIL_BloodSpray(vecPosition, vecDirection, BLOOD_COLOR_RED, 23, FX_BLOODSPRAY_ALL);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get rid of the ear ringing
//-----------------------------------------------------------------------------
void CFFPlayer::OnDamagedByExplosion( const CTakeDamageInfo &info )
{
}

//-----------------------------------------------------------------------------
// Purpose: Gibbing is currently broken (cannot respawn) so keep false
//-----------------------------------------------------------------------------
bool CFFPlayer::ShouldGib( const CTakeDamageInfo &info )
{
	return (GetHealth() <= -FFDEV_GIBDAMAGE);
}

bool CFFPlayer::Event_Gibbed(const CTakeDamageInfo &info)
{
	m_takedamage	= DAMAGE_NO;
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_lifeState		= LIFE_DEAD;
	AddEffects( EF_NODRAW ); // make the model invisible.
	pl.deadflag = true;

	SetMoveType(MOVETYPE_FLYGRAVITY);

	Extinguish();

	SetThink(&CBasePlayer::PlayerDeathThink);
	SetNextThink( gpGlobals->curtime + 0.1f );

	CEffectData data;
  	data.m_vOrigin = GetAbsOrigin();
	data.m_vStart = info.GetDamageForce();
	data.m_nEntIndex = entindex();
	DispatchEffect("Gib", data);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: get this game's encryption key for decoding weapon kv files
// Output : virtual const unsigned char
//-----------------------------------------------------------------------------
const unsigned char *CFFPlayer::GetEncryptionKey( void ) 
{ 
	return g_pGameRules->GetEncryptionKey(); 
}

//----------------------------------------------------------------------------
// Purpose: Get script file playerclass data
//----------------------------------------------------------------------------
const CFFPlayerClassInfo &CFFPlayer::GetFFClassData() const
{
	const CFFPlayerClassInfo *pPlayerClassInfo = GetFilePlayerClassInfoFromHandle( m_hPlayerClassFileInfo ); //&GetClassData();

	Assert(pPlayerClassInfo != NULL);

	return *pPlayerClassInfo;
}

//-----------------------------------------------------------------------------
// Purpose: Un-apply the concussion effect on this player
//-----------------------------------------------------------------------------
void CFFPlayer::UnConcuss( void )
{
	m_bConcussed = false; //ELMO
	m_flConcTime = gpGlobals->curtime - 1;

	// Remove status icon
	CSingleUserRecipientFilter user( ( CBasePlayer * )this );
	user.MakeReliable();

	UserMessageBegin( user, "StatusIconUpdate" );
		WRITE_BYTE( FF_STATUSICON_CONCUSSION );
		WRITE_FLOAT( 0.0f );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Apply the concussion effect on this player
//-----------------------------------------------------------------------------
void CFFPlayer::Concuss(float flDuration, float flIconDuration, const QAngle *viewjerk, float flDistance)
{
	// dexter: note Concuss will now be called on specs! network conc time for specs, also apply the status icon
	if( flDuration == -1 )
		m_flConcTime = flDuration;
	else
		m_flConcTime = gpGlobals->curtime + flDuration;

	// dont do the effect on specs
	if ( !IsObserver() )
		m_bConcussed = true; //ELMO

	// Send the status icon here... makes sense.
	// send the concussion icon to be displayed

	CSingleUserRecipientFilter user( ( CBasePlayer * )this );
	user.MakeReliable();

	UserMessageBegin( user, "StatusIconUpdate" );
		WRITE_BYTE( FF_STATUSICON_CONCUSSION );
		WRITE_FLOAT( flIconDuration );
	MessageEnd();		
	

	if (viewjerk)
	{
		ViewPunch((*viewjerk) * JERKMULTI * flDistance);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Hold some class information for the client
//-----------------------------------------------------------------------------
void CFFPlayer::SetClassForClient( int classnum )
{
	m_iClassStatus &= 0xFFFFFFF0;
	m_iClassStatus |= ( 0x0000000F & classnum );
}

void CFFPlayer::Ignite( float flFlameLifetime, bool bNPCOnly, float flSize, bool bCalledByLevelDesigner )
{
	AddFlag( FL_ONFIRE );

	SetFlameSpritesLifetime(flFlameLifetime, flSize);

	m_OnIgnite.FireOutput( this, this );
}

//-----------------------------------------------------------------------------
// Purpose: Extinguish player flames
//-----------------------------------------------------------------------------
void CFFPlayer::Extinguish( void )
{
	// Only send the network stuff once
	if (IsOnFire())
	{
		CSingleUserRecipientFilter user( this );
		user.MakeReliable();

		UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE(FF_STATUSICON_BURNING1);
			WRITE_FLOAT(0.0);
		MessageEnd();

		UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE( FF_STATUSICON_BURNING2 );
			WRITE_FLOAT( 0.0f );
		MessageEnd();

		UserMessageBegin(user, "StatusIconUpdate");
			WRITE_BYTE( FF_STATUSICON_BURNING3 );
			WRITE_FLOAT( 0.0f );
		MessageEnd();

		RemoveFlag(FL_ONFIRE);

		// Bug #0000969: first person flame effect not extinguished when going in water
		// Remove view effect
		CSingleUserRecipientFilter user2( ( CBasePlayer * )this );
		user2.MakeReliable();

		UserMessageBegin( user2, "FFViewEffect" );
			WRITE_BYTE( FF_VIEWEFFECT_BURNING );
			WRITE_FLOAT( 0.0 );
		MessageEnd();

		// Play sound!
		CSingleUserRecipientFilter hFilter( ( CBasePlayer * )this );
		EmitSound( hFilter, entindex(), "Player.Flameout" );
	}

	// Make sure these are turned off
	m_iBurnTicks = 0;
	m_flBurningDamage = 0;
	m_bBurnFlagNG = false;
	m_bBurnFlagIC = false;
	m_bBurnFlagFT = false;
	SetFlameSpritesLifetime( -1.0f );

	StopSound( "General.BurningFlesh" );
	StopSound( "General.BurningObject" );
	//EmitSound( "General.StopBurning" );
}

//-----------------------------------------------------------------------------
// Purpose: Gas a player
//-----------------------------------------------------------------------------
void CFFPlayer::Gas( float flDuration, float flIconDuration, CFFPlayer *pGasser)
{
	/*
	We always want to apply gas even if they are already gassed, this way if they are killed,
	the last person that gassed them gets the kill.
	*/

	m_bGassed = true;
	m_hGasser = pGasser; 

	if(m_flNextGas < gpGlobals->curtime)
		m_flNextGas = gpGlobals->curtime;

	if(flDuration != -1)
		m_flGasTime = gpGlobals->curtime + flDuration;
	else
		m_flGasTime = gpGlobals->curtime + 99999.0f;//this should last a while.

	// Send status icon
	CSingleUserRecipientFilter user( ( CBasePlayer * )this );
	user.MakeReliable();

	UserMessageBegin( user, "StatusIconUpdate" );
		WRITE_BYTE( FF_STATUSICON_HALLUCINATIONS );
		WRITE_FLOAT( flIconDuration );
	MessageEnd();

	// Send hallucination effect
	// This should probably be done as a HUD message!
	CEffectData data;
	te->DispatchEffect(user, 0.0, data.m_vOrigin, "Hallucination", data);
}

//-----------------------------------------------------------------------------
// Purpose: Un-Gas a player
//-----------------------------------------------------------------------------
void CFFPlayer::UnGas( void )
{
	// Remove gas
	m_bGassed = false;
	m_hGasser = NULL;
	m_flNextGas = 0;
	m_flGasTime = 0;

	CSingleUserRecipientFilter user( ( CBasePlayer * )this );
	user.MakeReliable();

	UserMessageBegin( user, "StatusIconUpdate" );
		WRITE_BYTE( FF_STATUSICON_HALLUCINATIONS );
		WRITE_FLOAT( 0.0f );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Slide a player (overpressure effect, gives low friction/acceleration)
//-----------------------------------------------------------------------------
void CFFPlayer::StartSliding( float flDuration, float flIconDuration )
{
	m_bSliding = true;
	SetFriction( 0.0f );

	if(flDuration != -1)
		m_flSlidingTime = gpGlobals->curtime + flDuration;
	else
		m_flSlidingTime = gpGlobals->curtime + 99999.0f;//this should last a while.

	// Send status icon
	CSingleUserRecipientFilter user( ( CBasePlayer * )this );
	user.MakeReliable();

	UserMessageBegin( user, "StatusIconUpdate" );
		WRITE_BYTE( FF_STATUSICON_SLIDING );
		WRITE_FLOAT( flIconDuration );
	MessageEnd();

	// Send hallucination effect
	// This should probably be done as a HUD message!
	//CEffectData data;
	//te->DispatchEffect(user, 0.0, data.m_vOrigin, "Hallucination", data);
}

//-----------------------------------------------------------------------------
// Purpose: Stop sliding a player (overpressure effect, gives low friction/acceleration)
//-----------------------------------------------------------------------------
void CFFPlayer::StopSliding( void )
{
	m_bSliding = false;
	m_flSlidingTime = 0;
	SetFriction( 1.0f );

	CSingleUserRecipientFilter user( ( CBasePlayer * )this );
	user.MakeReliable();

	UserMessageBegin( user, "StatusIconUpdate" );
		WRITE_BYTE( FF_STATUSICON_SLIDING );
		WRITE_FLOAT( 0.0f );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Heal player above their maximum
//-----------------------------------------------------------------------------
int CFFPlayer::Heal(CFFPlayer *pHealer, float flHealth, bool healToFull)
{
	if (!edict() || m_takedamage < DAMAGE_YES)
		return 0;

	// 150% max specified in the wiki
	if( ( float )m_iHealth >= ( float )( m_iMaxHealth * 1.5f ) )
		return 0;

	int iOriginalHP = m_iHealth;

	// Also medpack boosts health to maximum + then carries on to 150%
	if( ( m_iHealth < m_iMaxHealth ) && ( healToFull == true ) )
		m_iHealth = m_iMaxHealth;
	else if ( healToFull == true)
	{
		// Bug #0000467: Medic can't give over 100% health [just added in the "m_iHealth =" line...]
		m_iHealth = min( ( float )( m_iHealth + flHealth ), ( float )( m_iMaxHealth * 1.5f ) );
	}
	else
		m_iHealth = max( min( ( float )( m_iHealth + flHealth ), ( float )( m_iMaxHealth ) ) , ( float )( m_iHealth ) );

	if (IsInfected())
	{
		//g_StatsLog->AddStat(pHealer->m_iStatsID, m_iStatInfectCures, 1);
		m_bInfected = false;

		// Icon thing, give it just long enough to fade
		CSingleUserRecipientFilter user(this);
		user.MakeReliable();

		UserMessageBegin(user, "FFViewEffect");
			WRITE_BYTE(FF_VIEWEFFECT_INFECTED);
			WRITE_FLOAT(1.0f);
		MessageEnd();

		// [TODO] A better sound
		EmitSound( "medkit.hit" );
	}

	// And removes any adverse speed effects
	ClearSpeedEffects(SEM_HEALABLE);

	if (healToFull != true && m_iHealth - iOriginalHP == 0)
		return 0;

	// show addhealth
	CSingleUserRecipientFilter filter( this );
	filter.MakeReliable();

	// show added/subtracted health
	UserMessageBegin( filter, "PlayerAddHealth" );
		WRITE_SHORT( m_iHealth - iOriginalHP );
	MessageEnd();

	// AfterShock - scoring system: Heal x amount of health +.5*health_given (only if last damage from enemy) 
	// Leaving the 'last damage from enemy' part out until discussion has finished about it.
	pHealer->AddFortPoints( ( (m_iHealth - iOriginalHP) * 0.5 ), "#FF_FORTPOINTS_GIVEHEALTH");

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: Remove speed effects when players get hp
//-----------------------------------------------------------------------------
int CFFPlayer::TakeHealth( float flHealth, int bitsDamageType )
{
	int hp = BaseClass::TakeHealth( flHealth, bitsDamageType );

	// Reverting back to how it was and will add medic regen
	// health specifically when its time to add medic regen health

// Bug reverted back to before "fix"
//	// Bug #0000528: Medics can self-cure being caltropped/tranq'ed
//	// Only have good effects if they got health from this

	// Bug #0000604: Taking health with health kit doesn't fix status effects
	//if (hp && flHealth)
	// Want to remove the effects anyway whether you get health or not
	// because you could walk over a health kit but not get health or something
	// but your effect should still go away. Ie. you shouldn't have to be
	// injured to make a bag/pack remove your status effects
		ClearSpeedEffects(SEM_HEALABLE);

	return hp;
}

// ConVar emp_celldamage("ffdev_emp_celldamagecap", "150", FCVAR_REPLICATED | FCVAR_CHEAT);
#define MAX_CELLDAMAGE 150 // emp_celldamage.GetInt()

//ConVar emp_hwguy_shells_multi("ffdev_emp_hwguy_shells_multi", "0.666", FCVAR_REPLICATED | FCVAR_CHEAT);
#define EMP_HWGUY_SHELLS_MULTI 0.666f // emp_hwguy_shells_multi.GetFloat()

int CFFPlayer::TakeEmp()
{
	// Rockets, shells, and cells explode from EMPs, unless you are an engineer
	// in which case cells do not
	// EMPs also reduce your ammo store by 25%!
	// Values calculated  from TFC

    int ammodmg = 0;

	int iShells = GetAmmoDef()->Index(AMMO_SHELLS);
	int iRockets = GetAmmoDef()->Index(AMMO_ROCKETS);
	int iCells = GetAmmoDef()->Index(AMMO_CELLS);

	float flShellsMultiplier = 1.0f;

	if (GetClassSlot() == CLASS_HWGUY)
		flShellsMultiplier = EMP_HWGUY_SHELLS_MULTI;

	ammodmg += GetAmmoCount(iShells) * 0.5f * flShellsMultiplier;
	SetAmmoCount(GetAmmoCount(iShells) * 0.75f, iShells);

	ammodmg += GetAmmoCount(iRockets) * 1.3f;
	SetAmmoCount(GetAmmoCount(iRockets) * 0.75f, iRockets);

	// phish and I just found out that only the engineer has his cells ignored
	// the pyro just has so many cells, that they destroy him - Jon - 2/9/2007
	if (GetClassSlot() != CLASS_ENGINEER)
	{
		// Jiggles: Actually, we don't really want to insta-gib Pyros...
		ammodmg += clamp( GetAmmoCount(iCells) * 1.3f, 0, MAX_CELLDAMAGE );
		SetAmmoCount(GetAmmoCount(iCells) * 0.75f, iCells);
	}

	return ammodmg;
}

bool CFFPlayer::TakeNamedItem(const char* pszName)
{
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		if (m_hMyWeapons[i] && FClassnameIs(m_hMyWeapons[i], pszName))
		{
			// This is their active weapon
			if (GetActiveWeapon() == m_hMyWeapons[i])
			{
				//ClearActiveWeapon();

				if (!SwitchToNextBestWeapon(GetActiveWeapon()))
				{
					CBaseViewModel *vm = GetViewModel();

					if (vm)
						vm->AddEffects( EF_NODRAW );
				}
				//Weapon_SetLast(NULL);
			}

			// Remove weapon
			m_hMyWeapons[i]->Delete( );
			m_hMyWeapons.Set( i, NULL );

			IGameEvent *pEvent = gameeventmanager->CreateEvent("player_removeitem");						
			if(pEvent)
			{
				pEvent->SetInt("userid", GetUserID());
				pEvent->SetString("name", pszName);
				gameeventmanager->FireEvent(pEvent, true);
			}
		}
	}

	return true;
}

void CFFPlayer::Command_Disguise()
{
	int iTeam = 0, iClass = 0;

	if (!GetDisguisable()) 
	{
		ClientPrint(this, HUD_PRINTTALK, "#FF_SPY_NODISGUISENOW");
		return;
	}

	// if the format is "disguise team class"
	if(engine->Cmd_Argc( ) >= 3)
	{
		const char *szTeam = engine->Cmd_Argv( 1 );
		const char *szClass = engine->Cmd_Argv( 2 );

		// Allow either specify enemy/friendly or an actual team
		if (FStrEq(szTeam, "enemy") || FStrEq(szTeam, "e"))
		{
			int iMyTeam = GetTeamNumber();

			// TODO: Check friendliness of the other team (do a loop methinks)
			//iTeam = ( ( GetTeamNumber() == TEAM_BLUE ) ? TEAM_RED : TEAM_BLUE );

			int iEnemyTeams[8] = {0};
			int iNumTeams = 0;
			for(int i = TEAM_BLUE; i <= TEAM_GREEN; ++i)
			{
				if(i==iMyTeam)
					continue;

				CFFTeam *pTeam = GetGlobalFFTeam(i);
				if(pTeam && pTeam->GetTeamLimits() != -1)
				{
					if(!(pTeam->GetAllies() & iMyTeam))
					{
						iEnemyTeams[iNumTeams++] = i;
					}
				}
			}

			if(iNumTeams>0)
				iTeam = iEnemyTeams[RandomInt(0,iNumTeams-1)];
		}
		else if (FStrEq(szTeam, "friendly") || FStrEq(szTeam, "f"))
		{
			iTeam = GetTeamNumber();
		}
		else
		{
			// iTeam = atoi(szTeam) + (TEAM_BLUE - 1);

			// Since players can enter an integer or string for
			// the team we need to account for both methods.

			if( strlen( szTeam ) == 1 ) // Single character
			{
				if( ( szTeam[ 0 ] >= '1' ) && ( szTeam[ 0 ] <= '4' ) ) // Number from 1-4
					iTeam = atoi( szTeam ) + 1; // TEAM_BLUE = 2
				else
				{
					switch( szTeam[ 0 ] ) // Single letter like b, r, y, g
					{
						case 'b':
						case 'B': iTeam = TEAM_BLUE; break;

						case 'r':
						case 'R': iTeam = TEAM_RED; break;

						case 'y':
						case 'Y': iTeam = TEAM_YELLOW; break;

						case 'g':
						case 'G': iTeam = TEAM_GREEN; break;

						default:
							Warning( "[Disguise] Disguise command must be in the proper format!\n" );
							return;
						break;

					}
				}
			}
			else
			{
				// Some kind of string
				if( !Q_stricmp( szTeam, "blue" ) )
					iTeam = TEAM_BLUE;
				else if( !Q_stricmp( szTeam, "red" ) )
					iTeam = TEAM_RED;
				else if( !Q_stricmp( szTeam, "yellow" ) )
					iTeam = TEAM_YELLOW;
				else if( !Q_stricmp( szTeam, "green" ) )
					iTeam = TEAM_GREEN;
			}
		}
		
		// Now for the class. Allow numbers 1-9 and strings like "soldier".
		// Not allowing single characters representing the first letter of the class.
		if( ( strlen( szClass ) == 1 ) && ( szClass[ 0 ] >= '1' ) && ( szClass[ 0 ] <= '9' ) )
			iClass = atoi( szClass );
		else
			iClass = Class_StringToInt( szClass );
		
	}
	// else if the format is "disguise command"
	else if (engine->Cmd_Argc( ) == 2)
	{
		const char *szCmd = engine->Cmd_Argv( 1 );
		// "disguise last" / "disguise l"
		if ( FStrEq(szCmd, "last") || FStrEq(szCmd, "l") )
		{
			if (HasLastDisguise())
			{
				iTeam = GetLastDisguisedTeam();
				iClass = GetLastDisguisedClass();
			}
			else
			{
				Warning( "[Disguise] No last disguise found\n" );
				return;
			}
		}
	}

	// Bail if we don't have a team yet
	if( !iTeam )
	{
		Warning( "[Disguise] Disguise command must be in the proper format!\n" );
		return;
	}

	// Bail if we don't have a class yet
	if( !iClass )
	{
		Warning( "[Disguise] Disguise command must be in the proper format!\n" );
		return;
	}

	// Make sure class/team limits allow this disguise!
	CFFTeam *pTeam = GetGlobalFFTeam( iTeam );
	if( !pTeam )
	{
		Warning( "[Disguise] Uh, invalid somehow? What?\n" );
		return;
	}

	if( pTeam->GetTeamLimits() == -1 )
	{
		// TODO: Nice hud msg!
		//Warning( "[Disguise] Invalid team for this map!\n" );

		Omnibot::Notify_CantDisguiseAsTeam(this, iTeam);
		return;
	}

	if( pTeam->GetClassLimit( iClass ) == -1 )
	{
		// TODO: Nice hud msg!
		//Warning( "[Disguise] Invalid class for this map!\n" );

		Omnibot::Notify_CantDisguiseAsClass(this, iClass);
		return;
	}

	//Warning( "[Disguise] [Server] Disguise team: %i, Disguise class: %i\n", iTeam, iClass );

	// Now do the actual disguise
	SetDisguise(iTeam, iClass);

	ClientPrint( this, HUD_PRINTTALK, "#FF_SPY_DISGUISING" );

	// TODO: This should probably pass in iTeam & iClass as
	// these two functions won't have the right shit yet?
	Omnibot::Notify_Disguising(this, GetNewDisguisedTeam(), GetNewDisguisedClass());
}

// Server only
int CFFPlayer::GetNewDisguisedTeam( void ) const
{
	// Assumes we're a spy and currently disguising
	return ( m_iNewSpyDisguise & 0x0000000F );
}

// Server only
int CFFPlayer::GetNewDisguisedClass( void ) const
{
	// Assumes we're a spy and currently disguising
	return ( ( m_iNewSpyDisguise & 0x000000F0 ) >> 4 );
}

//-----------------------------------------------------------------------------
// Purpose: Reset the model, skin and any flags
//-----------------------------------------------------------------------------
void CFFPlayer::ResetDisguise()
{
	// If not a spy, abort
	if( GetClassSlot() != CLASS_SPY )
		return;

	// If disguised
	if( IsDisguised() )
		ClientPrint( this, HUD_PRINTTALK, "#FF_SPY_LOSTDISGUISE" );
	else
	{
		// If not disguised...

		// If we're not disguising, abort
		if( m_iNewSpyDisguise == 0 )
			return;

		// So, guy is disguising, need to do a hud msg
		// then change him back to normal
		ClientPrint( this, HUD_PRINTTALK, "#FF_SPY_FORCEDLOSTDISGUISE" );
	}

	// Change back to normal - "reset"
	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	SetModel(pPlayerClassInfo.m_szModel);
	m_nSkin = GetTeamNumber() - FF_TEAM_BLUE;

	m_iNewSpyDisguise = 0;
	m_iSpyDisguise = 0;
	m_iSpyDisguising = 0;

	Omnibot::Notify_DisguiseLost(this);
}

//-----------------------------------------------------------------------------
// Purpose: Disguise has finished, so model and skin needs to be changed
//-----------------------------------------------------------------------------
void CFFPlayer::FinishDisguise()
{
	ClientPrint(this, HUD_PRINTTALK, "#FF_SPY_DISGUISED");

	PLAYERCLASS_FILE_INFO_HANDLE classinfo;

	if (ReadPlayerClassDataFromFileForSlot(filesystem, Class_IntToString( GetNewDisguisedClass() ), &classinfo, GetEncryptionKey()))
	{
		const CFFPlayerClassInfo *pPlayerClassInfo = GetFilePlayerClassInfoFromHandle(classinfo);

		if (pPlayerClassInfo)
		{
			// UNDONE: Don't do this since we're not using a separate model for cloaking now
			// Only set new model & skin if we're not cloaked
			//if( !IsCloaked() )
			//{
			// AfterShock: If we want to do friendly spies showing up as friendly spies, we'd have to have different models for friendly and enemies?
				SetModel(pPlayerClassInfo->m_szModel);
				m_nSkin = GetNewDisguisedTeam() - TEAM_BLUE; // since m_nSkin = 0 is blue
			//}
		}
	}

	m_iSpyDisguise = m_iNewSpyDisguise;
	m_iNewSpyDisguise = 0;
	m_iSpyDisguising = 0;

	// Fire an event.
	IGameEvent *pEvent = gameeventmanager->CreateEvent("disguised");						
	if(pEvent)
	{
		pEvent->SetInt("userid", this->GetUserID());
		pEvent->SetInt("team", GetDisguisedTeam());
		pEvent->SetInt("class", GetDisguisedClass());
		gameeventmanager->FireEvent(pEvent, true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set the disguise
//-----------------------------------------------------------------------------
void CFFPlayer::SetDisguise(int iTeam, int iClass, bool bInstant /* = false */)
{
#ifdef _DEBUG
	//bInstant = true;
#endif

	m_iNewSpyDisguise = iTeam;
	m_iNewSpyDisguise += iClass << 4;

	// set last spy disguise here
	m_iLastSpyDisguise = m_iNewSpyDisguise;

	m_iSpyDisguising++;	// Jiggles: For the client HUD disguise progress bar
	
	// TODO: Time logic
	if (bInstant)
	{
		m_flFinishDisguise = 0;
	}
	else
	{
		m_flFinishDisguise = gpGlobals->curtime + 3.5f;
	}

	// 50% longer when Cloaked
	//if( IsCloaked() )
	//	m_flFinishDisguise += 7.0f * 0.5f;
}

int CFFPlayer::AddHealth(unsigned int amount)
{
	int left = TakeHealth( amount, DMG_GENERIC );
	return left;
}

//-----------------------------------------------------------------------------
// Purpose: Give the player some ammo. LUA, and only LUA, calls this to give
//			ammo to a player.
//-----------------------------------------------------------------------------
int CFFPlayer::LuaAddAmmo( int iAmmoType, int iAmount )
{
	int iDispensed = 0;

	switch( iAmmoType )
	{
		case LUA_AMMO_SHELLS:
		case LUA_AMMO_CELLS:
		case LUA_AMMO_NAILS:
		case LUA_AMMO_ROCKETS:
		case LUA_AMMO_DETPACK:
		case LUA_AMMO_MANCANNON:
			iDispensed = GiveAmmo( iAmount, LookupLuaAmmo( iAmmoType ), true );
		break;

		case LUA_AMMO_GREN1:
			iDispensed = AddPrimaryGrenades( iAmount );
		break;

		case LUA_AMMO_GREN2:
			iDispensed = AddSecondaryGrenades( iAmount );
		break;
	}

	return iDispensed;
}

int CFFPlayer::LuaAddHealth(int iAmount)
{
	if (m_iHealth > m_iMaxHealth) // Dont let LUA give over max health
		return 0;

	iAmount = min( iAmount, m_iMaxHealth - m_iHealth ); // dont give more health than their max
		//if (iAmount == 0)
		//	return 0;
	if (iAmount > 0)
		m_iHealth += iAmount;
	else
	{
		// use direct damage so armor is ignored -squeek
		CTakeDamageInfo info( this, this, -iAmount, DMG_DIRECT );
		//info.SetDamageForce( Vector( 0, 0, -1 ) );
		//info.SetDamagePosition( Vector( 0, 0, 1 ) );
		info.SetCustomKill(KILLTYPE_INFECTION);

		TakeDamage( info );
	}

	if (iAmount != 0)
	{
		CSingleUserRecipientFilter filter( this );
		filter.MakeReliable();

		// show added/subtracted health
		UserMessageBegin( filter, "PlayerAddHealth" );
			WRITE_SHORT( iAmount );
		MessageEnd();
	}

	return iAmount;
}

//-----------------------------------------------------------------------------
// Purpose: Remove ammo from player. LUA, and only LUA, calls this
//-----------------------------------------------------------------------------
void CFFPlayer::LuaRemoveAmmo( int iAmmoType, int iAmount )
{
	switch( iAmmoType )
	{
		case LUA_AMMO_SHELLS:
		case LUA_AMMO_CELLS:
		case LUA_AMMO_NAILS:
		case LUA_AMMO_ROCKETS:
		case LUA_AMMO_DETPACK:
		case LUA_AMMO_MANCANNON:
			RemoveAmmo( iAmount, LookupLuaAmmo( iAmmoType ) );
			break;

		case LUA_AMMO_GREN1:
			AddPrimaryGrenades( -iAmount );
			break;

		case LUA_AMMO_GREN2:
			AddSecondaryGrenades( -iAmount );
			break;
	}

	/*useful here too?
	if(bClipToo)
	{
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			if(m_hMyWeapons[i] && m_hMyWeapons[i]->m_iClip1 != -1)
				m_hMyWeapons[i]->m_iClip1 = 0;
			if(m_hMyWeapons[i] && m_hMyWeapons[i]->m_iClip2 != -1)
				m_hMyWeapons[i]->m_iClip2 = 0;
		}
		SwitchToNextBestWeapon(GetActiveWeapon());
	}*/
}

void CFFPlayer::LuaRemoveAllAmmo(bool bClipToo)
{
	BaseClass::RemoveAllAmmo();
	AddSecondaryGrenades(-4);
	AddPrimaryGrenades(-4);

	if(bClipToo)
	{
		for (int i = 0; i < MAX_WEAPONS; i++)
		{
			if(m_hMyWeapons[i] && m_hMyWeapons[i]->m_iClip1 != -1)
				m_hMyWeapons[i]->m_iClip1 = 0;
			if(m_hMyWeapons[i] && m_hMyWeapons[i]->m_iClip2 != -1)
				m_hMyWeapons[i]->m_iClip2 = 0;
		}
		SwitchToNextBestWeapon(GetActiveWeapon());
	}
}

int CFFPlayer::LuaGetAmmoCount( int iAmmoType )
{
	return GetAmmoCount( GetAmmoDef()->Index(LookupLuaAmmo( iAmmoType )) );
}

bool CFFPlayer::LuaOwnsWeaponType(const char *_name)
{
	return Weapon_OwnsThisType(_name, 0) != NULL;
}

bool CFFPlayer::LuaGiveWeapon(const char *_name, bool _autoselect)
{
	CBaseEntity *pEnt = GiveNamedItem(_name, 0);
	if(pEnt)
	{
		if(_autoselect)
		{
			SelectItem(_name, 0);
		}
		return true;
	}
	return false;
}

void CFFPlayer::LuaRemoveAllWeapons()
{
	RemoveAllItems(false);
}

float CFFPlayer::LuaGetMovementSpeed()
{
	Vector vecVelocity = GetAbsVelocity();
	Vector vecLatVelocity = vecVelocity * Vector(1.0f, 1.0f, 0.0f);
	return vecLatVelocity.Length();
}

int CFFPlayer::GetAmmoInClip()
{
	if (GetActiveFFWeapon())
	{
		return GetActiveFFWeapon()->Clip1();
	}
	return -1;
}

int CFFPlayer::GetAmmoInClip( const char *_name )
{
	if (Weapon_OwnsThisType(_name, 0))
	{
		for (int i=0;i<MAX_WEAPONS;i++) 
		{
			if ( m_hMyWeapons[i].Get() && FClassnameIs(m_hMyWeapons[i], _name) )
			{
				CBaseCombatWeapon *pWeapon = m_hMyWeapons[i].Get();
				return pWeapon->Clip1();
			}
		}
	}
	return -1;
}

bool CFFPlayer::SetAmmoInClip( int iAmount )
{
	if (GetActiveFFWeapon())
	{
		GetActiveFFWeapon()->Clip1( iAmount );
		return true;
	}
	return false;
}

bool CFFPlayer::SetAmmoInClip( const char *_name, int iAmount )
{
	if (Weapon_OwnsThisType(_name, 0))
	{
		for (int i=0;i<MAX_WEAPONS;i++) 
		{
			if ( m_hMyWeapons[i].Get() && FClassnameIs(m_hMyWeapons[i], _name) )
			{
				CBaseCombatWeapon *pWeapon = m_hMyWeapons[i].Get();
				pWeapon->Clip1( iAmount );
				return true;
			}
		}
	}
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Give the player some ammo.
//-----------------------------------------------------------------------------
int CFFPlayer::GiveAmmo(int iCount, int iAmmoIndex, bool bSuppressSound)
{
	if (iCount <= 0)
		return 0;

	if (iAmmoIndex < 0 || iAmmoIndex >= MAX_AMMO_SLOTS)
		return 0;

	int iMax = m_iMaxAmmo[iAmmoIndex];
	int iAdd = min(iCount, iMax - m_iAmmo[iAmmoIndex]);
	if (iAdd < 1)
		return 0;

	// Ammo pickup sound
	if (!bSuppressSound)
		EmitSound("BaseCombatCharacter.AmmoPickup");

	m_iAmmo.Set(iAmmoIndex, m_iAmmo[iAmmoIndex] + iAdd);

	return iAdd;
}

//-----------------------------------------------------------------------------
// Purpose: Give the player some ammo.
//-----------------------------------------------------------------------------
int CFFPlayer::GiveAmmo(int iCount, const char *szName, bool bSuppressSound)
{
	int iAmmoType = GetAmmoDef()->Index(szName);
	if (iAmmoType == -1)
	{
		Msg("ERROR: Attempting to give unknown ammo type (%s)\n", szName);
		return 0;
	}
	return GiveAmmo(iCount, iAmmoType, bSuppressSound);
}

// Find a map guide
CFFMapGuide *CFFPlayer::FindMapGuide(string_t targetname)
{
	CBaseEntity *pent = gEntList.FindEntityByName(NULL, targetname, NULL);

	if (!pent)
		return NULL;

	CFFMapGuide *pMapGuide = dynamic_cast<CFFMapGuide *>(pent);

	return pMapGuide;
}

//-----------------------------------------------------------------------------
// Purpose: Move between map guide points.
//-----------------------------------------------------------------------------
void CFFPlayer::MoveTowardsMapGuide()
{
	// Cancel now if we're no longer eligable
	if (IsHLTV() || GetTeamNumber() > TEAM_SPECTATOR)
	{
		m_hNextMapGuide = NULL;
		return;
	}

	if (!m_hNextMapGuide.Get())
		return;

	Vector vecMapGuideDir = m_hNextMapGuide->GetAbsOrigin() - GetAbsOrigin();

	// We're close enough to the next one and the time has finished here
	if (gpGlobals->curtime > m_flNextMapGuideTime)
	{
		//DevMsg("[MAPGUIDE] Reached guide %s\n", STRING(m_hNextMapGuide->GetEntityName()));

		// Play the narration file for this (but only if speccing)
		if (m_hNextMapGuide->m_iNarrationFile != NULL_STRING && GetTeamNumber() == TEAM_SPECTATOR)
		{
			EmitSound_t ep;
			ep.m_nChannel = CHAN_ITEM;
			ep.m_pSoundName = (char *) STRING(m_hNextMapGuide->m_iNarrationFile);
			ep.m_flVolume = 1.0f;
			ep.m_SoundLevel = SNDLVL_NORM;

			CSingleUserRecipientFilter filter(this);
			EmitSound(filter, entindex(), ep);
		}

		// Now the next one has become the last
		m_hLastMapGuide = m_hNextMapGuide;

		// And we are looking for a new one
		m_hNextMapGuide = FindMapGuide(m_hLastMapGuide->m_iNextMapguide);

		// Only bother if we found one
		// We are going to reach that in x seconds time (including wait time)
		// The extra wait time is clipped when deciding the position.
		if (m_hNextMapGuide)
			m_flNextMapGuideTime = gpGlobals->curtime + m_hLastMapGuide->m_flTime + m_hLastMapGuide->m_flWait;
		
		// We reached the end
		else
		{
			/* // Causing problems with SourceTV
			// Show menu again if we are spectator
			if (GetTeamNumber() == TEAM_SPECTATOR)
				ShowViewPortPanel(PANEL_MAPGUIDE, true);
			*/

			// Start looping again
			m_hLastMapGuide = m_hNextMapGuide = FindMapGuide(MAKE_STRING("overview"));

			if (m_hNextMapGuide)
				m_flNextMapGuideTime = 0;
		}

		// And also let's aim in the right direction
		SnapEyeAngles(m_hLastMapGuide->GetAbsAngles());
	}
	// We're not close enough, so move us closer
	else
	{
		Vector vecNewPos;

		float t = clamp((m_flNextMapGuideTime - gpGlobals->curtime) / m_hLastMapGuide->m_flTime, 0, 1.0f);
		
		// UNDONE: Only do the spline on the eyeangles
		//t = SimpleSpline(t);

		// There've no curve point to worry about
		if (m_hLastMapGuide->m_iCurveEntity == NULL_STRING)
			vecNewPos = t * m_hLastMapGuide->GetAbsOrigin() + (1 - t) * m_hNextMapGuide->GetAbsOrigin();

		// We're curving towards some point
		else
		{
			CBaseEntity *pent = gEntList.FindEntityByName(NULL, m_hLastMapGuide->m_iCurveEntity, NULL);

			if (pent)
			{
				Vector v1, v2;
				v1 = t * m_hLastMapGuide->GetAbsOrigin() + (1 - t) * pent->GetAbsOrigin();
				v2 = t * pent->GetAbsOrigin() + (1 - t) * m_hNextMapGuide->GetAbsOrigin();

				vecNewPos = t * v1 + (1 - t) * v2;
			}
			else
			{
				DevWarning("Could not find entity %s\n", STRING(m_hLastMapGuide->m_iCurveEntity));
				vecNewPos = t * m_hLastMapGuide->GetAbsOrigin() + (1 - t) * m_hNextMapGuide->GetAbsOrigin();
			}
		}

		SetAbsOrigin(vecNewPos);
	}
}

//-----------------------------------------------------------------------------
bool CFFPlayer::HasItem(const char* itemname) const
{
	bool ret = false;

	// get all info_ff_scripts
	CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( NULL, CLASS_INFOSCRIPT );

	while( pEnt && !ret )
	{
		if( ( pEnt->GetOwnerEntity() == ( CBaseEntity * )this ) && FStrEq( STRING(pEnt->GetEntityName()), itemname ) )
			ret = true;

		// Next!
		pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( pEnt, CLASS_INFOSCRIPT );
	}

	return ret;
}

//-----------------------------------------------------------------------------
bool CFFPlayer::IsInNoBuild()
{
	Vector vecForward;
	EyeVectors(&vecForward);
	vecForward.z = 0;
	VectorNormalize( vecForward );
	Vector vecOrigin = GetAbsOrigin() + (vecForward * 88.0f);

#ifdef _DEBUG
	if( !engine->IsDedicatedServer() )
	{
		// draw a green box
		NDebugOverlay::Box( vecOrigin, Vector( -28, -28, -28 ), Vector( 28, 28, 28 ), 0, 255, 0, 100, 5.0f );
	}
#endif

	return !FFScriptRunPredicates( (CBaseEntity*)this, "onbuild", true, vecOrigin, 40.0f );
}


//-----------------------------------------------------------------------------
// Purpose: Is the flashlight on or off, taken from HL2MP
//-----------------------------------------------------------------------------
int CFFPlayer::FlashlightIsOn()
{
	return IsEffectActive(EF_DIMLIGHT);
}


//-----------------------------------------------------------------------------
// Purpose: Turn on the flashlight, taken from HL2MP
//-----------------------------------------------------------------------------
void CFFPlayer::FlashlightTurnOn()
{
	AddEffects(EF_DIMLIGHT);
	EmitSound("HL2Player.FlashLightOn");
}

//-----------------------------------------------------------------------------
// Purpose: Turn off the flashlight, taken from HL2MP
//-----------------------------------------------------------------------------
void CFFPlayer::FlashlightTurnOff()
{
	RemoveEffects(EF_DIMLIGHT);
	EmitSound("HL2Player.FlashLightOff");
}

//-----------------------------------------------------------------------------
// Purpose: Scouts and spys can uncover disguised spies (trackerid: #0000585)
//-----------------------------------------------------------------------------
void CFFPlayer::Touch(CBaseEntity *pOther)
{
	if (GetClassSlot() == CLASS_SCOUT || GetClassSlot() == CLASS_SPY)
	{
		CFFPlayer *ffplayer = dynamic_cast<CFFPlayer *> (pOther);

		// Don't forget allies!
		if (ffplayer && ffplayer->IsDisguised() && g_pGameRules->PlayerRelationship(this, ffplayer) == GR_NOTTEAMMATE)
		{
			ClientPrint(ffplayer, HUD_PRINTTALK, "#FF_SPY_BEENREVEALED");
			ffplayer->ResetDisguise();

			//AfterShock - Scoring System: 100 points for uncovering spy
			AddFortPoints(30, "#FF_FORTPOINTS_UNDISGUISESPY");
			ClientPrint(this, HUD_PRINTTALK, "#FF_SPY_REVEALEDSPY");
			FF_SendHint( ffplayer, SPY_LOSEDISGUISE, -1, PRIORITY_NORMAL, "#FF_HINT_SPY_LOSEDISGUISE" );

			// This the correct func for logs? 
			// No, it's not :P - AfterShock
			//UTIL_LogPrintf("%s just exposed an enemy spy!\n", STRING(ffplayer->pl.netname));

			IGameEvent *pEvent = NULL;
			pEvent = gameeventmanager->CreateEvent( "disguise_lost" );
			if( pEvent )
			{
				pEvent->SetInt( "attackerid", GetUserID() ); // this player just touched and undisguised enemy spy 
				pEvent->SetInt( "userid", ffplayer->GetUserID() ); // enemy spy is the victim
				pEvent->SetInt( "priority", 10 );
				gameeventmanager->FireEvent( pEvent );
				DevMsg("event fired!");
			}

		}

		// Will only let scouts uncloak dudes for the meantime
		if( GetClassSlot() == CLASS_SCOUT )
		{
			if( ffplayer && ffplayer->IsCloaked() && g_pGameRules->PlayerRelationship( this, ffplayer ) == GR_NOTTEAMMATE )
			{
				ClientPrint( ffplayer, HUD_PRINTTALK, "#FF_SPY_BEENREVEALEDCLOAKED" );
				ffplayer->Command_SpyCloak();

				// MULCH: Assign real value here, just copy/pasted from above for
				// Bug #0001444: Scouts do not uncloak spies
				//AfterShock - Scoring System: ??? points for uncovering spy
				AddFortPoints( 30, "#FF_FORTPOINTS_UNCLOAKSPY" );
				ClientPrint( this, HUD_PRINTTALK, "#FF_SPY_REVEALEDCLOAKEDSPY" );
				FF_SendHint( ffplayer, SPY_LOSECLOAK, -1, PRIORITY_NORMAL, "#FF_HINT_SPY_LOSECLOAK" );

				// This the correct func for logs?
				// No, it's not :P - AfterShock
				//UTIL_LogPrintf( "%s just uncloaked an enemy spy!\n", STRING( ffplayer->pl.netname ) );

				IGameEvent *pEvent = NULL;
				pEvent = gameeventmanager->CreateEvent( "cloak_lost" );
				if( pEvent )
				{
					pEvent->SetInt( "attackerid", GetUserID() ); // this player just touched and undisguised enemy spy 
					pEvent->SetInt( "userid", ffplayer->GetUserID() ); // enemy spy is the victim
					pEvent->SetInt( "priority", 10 );
					gameeventmanager->FireEvent( pEvent );
					DevMsg("event fired!");
				}
			}
		}
	}

	BaseClass::Touch(pOther);
}
//-----------------------------------------------------------------------------
// Purpose: An instance switch.
//			This should allow classes to be swapped without killing them.
//-----------------------------------------------------------------------------
void CFFPlayer::InstaSwitch(int iClassNum)
{
	if (iClassNum < CLASS_SCOUT || iClassNum > CLASS_CIVILIAN)
		return;

	if (GetClassSlot() == iClassNum)
		return;

	m_iNextClass = iClassNum;

	// First clean up some (don't remove suit though)
	RemoveAllItems(false);
	RemoveItems();

	// Then apply the class stuff
	ActivateClass();
	SetupClassVariables();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFPlayer::SpyCloakFadeIn( bool bInstant )
{
	//Warning( "[Spy Cloak Fade] Start fading in!\n" );

	/*
	// Assume we're not disguised
	int iClass = CLASS_SPY;

	// Set correct model
	if( IsDisguised() )
		iClass = GetDisguisedClass();

	// HEY, be nice man.
	Assert( ( iClass > CLASS_NONE ) && ( iClass <= CLASS_CIVILIAN ) );

	// Come up with the path to the correct model we're as (whether disguised or not)
	const char *pszClass = Class_IntToString( iClass );

	char szClass[ MAX_PATH ];
	Q_snprintf( szClass, sizeof( szClass ), "models/player/%s/%s.mdl", pszClass, pszClass );

	// Set the correct model
	SetModel( szClass );

	// Set correct skin
	int iTeam = GetTeamNumber();

	if( IsDisguised() )
		iTeam = GetDisguisedTeam();

	// Set the correct team color for us
	m_nSkin = iTeam - FF_TEAM_BLUE;
	*/

	// NOTE NOTE: Can't seem to fade in!? You can fade out but not in...
	// Valve has probably hacked something to fade out... but I can't figure
	// out why you can't just change a players' render mode to something low
	// and start ramping up the alpha. So, we'll force instant switching
	// until I can figure it out.

	// Forcing instant switching until fade in can be resolved
	bInstant = true;

	// Find out when we'll finish the cloak fade
	m_flCloakFadeStart = gpGlobals->curtime;
	m_flCloakFadeFinish = bInstant ? gpGlobals->curtime : gpGlobals->curtime + ( m_bCloakFadeType ? FFDEV_SPY_SCLOAKFADESPEED : FFDEV_SPY_CLOAKFADESPEED );

	// If instant, set alpha back to normal and bail
	if( bInstant )
	{
		// Make sure normal drawing
		SetRenderMode( ( RenderMode_t )kRenderNormal );
		SetRenderColorA( 255 );

		// No need for cloak think, we're already done
		m_bCloakFadeCloaking = false;
	}
	else
	{
		// Make sure we're transparent and faded out so
		// we can be faded in
		//SetRenderMode( ( RenderMode_t )kRenderTransTexture );
		SetRenderColorA( 110 );

		// Un-cloaking
		m_iCloakFadeCloaking = 2;

		// Need to do stuff in fade think
		m_bCloakFadeCloaking = true;
	}	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFPlayer::SpyCloakFadeOut( bool bInstant )
{
	//Warning( "[Spy Cloak Fade] Start fading out!\n" );

	// Do instant fades until we delay setting
	// the forcedoverride material
	bInstant = true;

	m_flCloakFadeStart = gpGlobals->curtime;
	m_flCloakFadeFinish = bInstant ? gpGlobals->curtime : gpGlobals->curtime + ( m_bCloakFadeType ? FFDEV_SPY_SCLOAKFADESPEED : FFDEV_SPY_CLOAKFADESPEED );

	// Change render mode so we can start fading out
	//SetRenderMode( ( RenderMode_t )kRenderTransTexture );
	SetRenderColorA( 255 );	

	// Cloaking
	m_iCloakFadeCloaking = 1;

	// Need to do stuff in fade think
	m_bCloakFadeCloaking = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFPlayer::SpyCloakFadeThink( void )
{
	if( m_bCloakFadeCloaking )
	{
		float flFadeTimeLeft = m_flCloakFadeFinish - gpGlobals->curtime;
		if( flFadeTimeLeft <= 0.0f )
		{
			// Done fading, set the new skin
			m_bCloakFadeCloaking = false;			
			
			switch( m_iCloakFadeCloaking )
			{
				case 2:
					// Reset this, cloak shader handles the rest (when cloaking)
					//SetRenderMode( ( RenderMode_t )kRenderNormal );
					SetRenderColorA( 255 );
				break;

				// Done fading out from the cloak, so set refract skin
				// and let mat proxy take over
				case 1:
					//SetRenderMode( ( RenderMode_t )kRenderTransTexture );
					SetRenderColorA( 110 );
					//SetRenderMode( ( RenderMode_t )kRenderTransTexture );
					//SetModel( "models/player/predator/predator.mdl" );
				break;
			}
		}
		else
		{
			// Find percentage of fade completed
			float flPercFade = ( gpGlobals->curtime - m_flCloakFadeStart ) / ( m_flCloakFadeFinish - m_flCloakFadeStart );

			float flMinFade = 110.0f;
			float flMaxFade = 255.0f;

			float flFadeDiff = flMaxFade - flMinFade;

			float flNewAlpha = 0.0f;

			// Not done fading, adjust values more. Map percentage
			// faded to a range of alpha values
			switch( m_iCloakFadeCloaking )
			{
				// Un-Cloaking, so increase alpha
				case 2:
					flNewAlpha = flMinFade + ( flFadeDiff * flPercFade );
				break;

				// Cloaking, so decrease alpha
				case 1:
					flNewAlpha = flMaxFade - ( flFadeDiff * flPercFade );
				break;

				default:
					Assert( 0 );
				break;
			}

			//Warning( "[Spy Fade] Percentage faded: %f%%, New Alpha: %f\n", flPercFade * 100.0f, flNewAlpha );

			SetRenderColorA( flNewAlpha );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Attempts to sabotage the entity the player is looking at.
//			Handles both the timers and the actual sabotage itself.
//-----------------------------------------------------------------------------
void CFFPlayer::SpySabotageThink()
{
	// We don't want dead spies sabotaging
	if (!IsAlive() )
		return;

	if (m_flNextSpySabotageThink > gpGlobals->curtime)
		return;

	// No sabotaging while cloaked
	if( IsCloaked() )
		return;

	m_flNextSpySabotageThink = gpGlobals->curtime + 0.2f;

	// We have to be under a particular speed to sabotage
	if (GetAbsVelocity().LengthSqr() > 100 * 100)
	{
		SpyStopSabotaging();
		return;
	}

	// Traceline to see what we are looking at
	Vector vecForward;
	AngleVectors(EyeAngles(), &vecForward);

	trace_t tr;
	UTIL_TraceLine(EyePosition(), EyePosition() + vecForward * 100.0f, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);

	CFFBuildableObject *pBuildable = dynamic_cast<CFFBuildableObject *> (tr.m_pEnt);

	// Our sabotage status has changed
	if (pBuildable != m_hSabotaging)
	{
		// If it's not a valid thing to sabotage
		if ( pBuildable == NULL || !pBuildable->CanSabotage() || pBuildable->GetTeamNumber() == GetTeamNumber() || FFGameRules()->IsTeam1AlliedToTeam2( pBuildable->GetTeamNumber(), GetDisguisedTeam() ) != GR_TEAMMATE )
		{
			// Not something we can sabotage, stop 
			SpyStopSabotaging();
			return;
		}

		// Reset the time left until the sabotage is finished
		m_flSpySabotageFinish = gpGlobals->curtime + 3.0f;

		// Now send off the timer
		SpyStartSabotaging(pBuildable);
	}
	// Sabotage state has not changed
	else
	{
		// If we are sabotaging then check if we've timed out
		if (m_hSabotaging && m_flSpySabotageFinish <= gpGlobals->curtime)
		{
			CSingleUserRecipientFilter filter(this);
			CFFEntitySystemHelper* pHelperInst = CFFEntitySystemHelper::GetInstance();
			if(pHelperInst)
				pHelperInst->EmitSound( filter, pHelperInst->entindex(), "Player.Sabotage" );

			// Fire an event.
			IGameEvent *pEvent = NULL;
			if(m_hSabotaging->Classify() == CLASS_SENTRYGUN)
			{
				ClientPrint(this, HUD_PRINTCENTER, "#FF_SENTRYSABOTAGEREADY");
				pEvent = gameeventmanager->CreateEvent("sentry_sabotaged");
				m_iActiveSabotages |= 2;	// Magic numbers, I know!
				m_iSabotagedSentries++;
			}
			else if(m_hSabotaging->Classify() == CLASS_DISPENSER)
			{
				ClientPrint(this, HUD_PRINTCENTER, "#FF_DISPENSERSABOTAGEREADY");
				pEvent = gameeventmanager->CreateEvent("dispenser_sabotaged");
				m_iActiveSabotages |= 1;
				m_iSabotagedDispensers++;
			}
			if(pEvent)
			{
				CFFPlayer *pOwner = NULL;
				if( m_hSabotaging )
					pOwner = ToFFPlayer( m_hSabotaging->m_hOwner.Get() );
					
				if( pOwner )
				{					
					pEvent->SetInt("userid", pOwner->GetUserID());
					pEvent->SetInt("saboteur", GetUserID());

					// caes: fire the sentry/dispenser sabotaged events
					gameeventmanager->FireEvent( pEvent );
					// caes
				}				
			}

			m_hSabotaging->Sabotage(this);
			m_hSabotaging = NULL;
			SetCloakable( true );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: sends the start sabotage timer message
//-----------------------------------------------------------------------------
void CFFPlayer::SpyStartSabotaging(CFFBuildableObject *pBuildable)
{
	if (m_hSabotaging)
	{
		SpyStopSabotaging();
		return;
	}

	int iBuildableType;

	// Determine the correct item that'll be shown on the menu
	if (pBuildable->Classify() == CLASS_SENTRYGUN)
		iBuildableType = FF_BUILD_SENTRYGUN;
	else
		iBuildableType = FF_BUILD_DISPENSER;

	CSingleUserRecipientFilter user(this);
	user.MakeReliable();
	UserMessageBegin(user, "FF_BuildTimer");
	WRITE_SHORT(iBuildableType);
	WRITE_FLOAT(3.0f);
	MessageEnd();

	// Now remember what we're sabotaging
	m_hSabotaging = pBuildable;

	SetCloakable( false );
}

//-----------------------------------------------------------------------------
// Purpose: sends the start sabotage timer message
//-----------------------------------------------------------------------------
void CFFPlayer::SpyStopSabotaging()
{
	if (!m_hSabotaging)
		return;

	CSingleUserRecipientFilter user(this);
	user.MakeReliable();
	UserMessageBegin(user, "FF_BuildTimer");
	WRITE_SHORT(0);
	WRITE_FLOAT(0);
	MessageEnd();

	// Remember that we aren't sabotaging
	m_hSabotaging = NULL;

	SetCloakable( true );
}

//-----------------------------------------------------------------------------
// Purpose: Return the weapon in a slot
//-----------------------------------------------------------------------------
CBaseCombatWeapon *CFFPlayer::GetWeaponForSlot(int iSlot)
{
	Assert((iSlot >= 0) && (iSlot < MAX_WEAPON_SLOTS));

	for (int iWeap = 0; iWeap < MAX_WEAPONS; iWeap++)
	{
		CBaseCombatWeapon *pWeapon = GetWeapon(iWeap);

		if (pWeapon && pWeapon->GetSlot() == iSlot)
			return pWeapon;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Add armour, ensure that armour type is reset
//-----------------------------------------------------------------------------
int CFFPlayer::AddArmor( int iAmount )
{
	// Boost up their armour type again
	// AfterShock: not needed now disp quiet sabotage is canned
	//m_iArmorType = m_iBaseArmorType;

	iAmount = min( iAmount, m_iMaxArmor - m_iArmor );
	if (iAmount <= 0)
		return 0;

	m_iArmor += iAmount;

	if (iAmount != 0)
	{
		CSingleUserRecipientFilter filter( this );
		filter.MakeReliable();

		// show added/subtracted health
		UserMessageBegin( filter, "PlayerAddArmor" );
			WRITE_SHORT( iAmount );
		MessageEnd();
	}

	return iAmount;
}

//-----------------------------------------------------------------------------
// Purpose: Remove armour, yadda yadda yadda. Moved out of the header file
//-----------------------------------------------------------------------------
int CFFPlayer::RemoveArmor( int iAmount )
{
	int iRemovedAmt = min( iAmount, m_iArmor );

	m_iArmor = clamp( m_iArmor - iAmount, 0, m_iArmor );

	return iRemovedAmt;
}

//-----------------------------------------------------------------------------
// Purpose: Reduce armour class to level below normal, this is only really
//			used by the sabotaged dispenser
//-----------------------------------------------------------------------------
/* AfterShock: removing quiet disp sabotage
void CFFPlayer::ReduceArmorClass()
{
	if (m_flBaseArmorType == 0.8f)
		m_flArmorType = 0.5f;
	else if (m_flBaseArmorType == 0.5f)
		m_flArmorType = 0.3f;
}
*/
//-----------------------------------------------------------------------------
// Purpose: Find all sentry guns that have been sabotaged by this player and 
//			turn them on the enemy.
//-----------------------------------------------------------------------------
void CFFPlayer::Command_SabotageSentry()
{
	CFFSentryGun *pSentry = NULL; 

	ClientPrint(this, HUD_PRINTCONSOLE, "> Send spike... ... ... ... ... ...Spike sent.\n");

	while ((pSentry = (CFFSentryGun *) gEntList.FindEntityByClassT(pSentry, CLASS_SENTRYGUN)) != NULL) 
	{
		if (pSentry->IsSabotaged() && pSentry->m_hSaboteur == this) 
			pSentry->MaliciouslySabotage(this);
	}
	m_iActiveSabotages &= ~2;
}

//-----------------------------------------------------------------------------
// Purpose: Find all dispensers that have been sabotaged by this player and 
//			detonate them
//-----------------------------------------------------------------------------
void CFFPlayer::Command_SabotageDispenser()
{
	CFFDispenser *pDispenser = NULL; 

	while ((pDispenser = (CFFDispenser *) gEntList.FindEntityByClassT(pDispenser, CLASS_DISPENSER)) != NULL)  
	{
		if (pDispenser->IsSabotaged() && pDispenser->m_hSaboteur == this) 
			pDispenser->MaliciouslySabotage(this);
	}
	m_iActiveSabotages &= ~1;
}

//-----------------------------------------------------------------------------
// Purpose: Release control of any sabotaged thing
//-----------------------------------------------------------------------------
void CFFPlayer::SpySabotageRelease()
{

	// Jiggles: Let's try it without Strings
	CFFDispenser *pDispenser = (CFFDispenser*)gEntList.FindEntityByClassT( NULL, CLASS_DISPENSER );
	// Release any dispensers
	while( pDispenser != NULL )
	{
		if (pDispenser->m_hSaboteur == this) 
		{
			pDispenser->RemoveSaboteur(true);
		}
		// Next!
		pDispenser = (CFFDispenser*)gEntList.FindEntityByClassT( pDispenser, CLASS_DISPENSER );
	}

	// Jiggles: Again, let's try it without Strings
	CFFSentryGun *pSentry = (CFFSentryGun*)gEntList.FindEntityByClassT( NULL, CLASS_SENTRYGUN );
	// Release any sentry guns
	while( pSentry != NULL )
	{
		// Jiggles: Don't remove the Saboteur if he's already triggered Malicious Sabotage mode!
		//          The SG will die after the "shooting teammates" period ends anyway.
		if ( (pSentry->m_hSaboteur == this) && !pSentry->m_bMaliciouslySabotaged ) 
		{
			pSentry->RemoveSaboteur(true);
		}
		// Next!
		pSentry = (CFFSentryGun*)gEntList.FindEntityByClassT( pSentry, CLASS_SENTRYGUN );
	}

}

//-----------------------------------------------------------------------------
// Purpose: A more TFC-style bodytarget
//			Have decreased some of the variance as well to make rocket
//			jumping more consistent
//-----------------------------------------------------------------------------
Vector CFFPlayer::BodyTarget(const Vector &posSrc, bool bNoisy)
{ 
	if (IsInAVehicle())
	{
		return GetVehicle()->GetVehicleEnt()->BodyTarget(posSrc, bNoisy);
	}

	if (bNoisy)
	{
		return GetAbsOrigin() + (Vector(0, 0, 28) * random->RandomFloat(0.5f, 1.1f));
	}
	else
	{
		return GetAbsOrigin(); //return EyePosition(); 
	}
};

//-----------------------------------------------------------------------------
// Purpose: Run "effects" and "speed effects" through here first before giving 
//			the player the actual effect.
// Input  :	iEffect - the effect
// Input  : szEffectSuffix - the effect suffix, like "conc", or "gas", etc. It
//			corresponds to a player_on<SUFFIX> function in Lua.
// Output : Function returns true if LUA allowed us to conc the person. Values
//			may or may not have been modified.
//-----------------------------------------------------------------------------
bool CFFPlayer::LuaRunEffect( int iEffect, CBaseEntity *pEffector, float *pflDuration, float *pflIconDuration, float *pflSpeed )
{
	// Invalid effect
	if( ( iEffect < 0 ) || ( iEffect > ( LUA_EF_MAX_FLAG - 1 ) ) )
		return false;

	float flDuration = 0.0f, flIconDuration = 0.0f, flSpeed = 0.0f;
	
	if( pflDuration )
		flDuration = *pflDuration;
	if( pflIconDuration )
		flIconDuration = *pflIconDuration;
	if( pflSpeed )
		flSpeed = *pflSpeed;

	// Generate our lua function name...
	char szLuaFunc[ 64 ], *pszEffectSuffix = 0;

	switch( iEffect )
	{
		case LUA_EF_ONFIRE: pszEffectSuffix = "fire"; break;
		case LUA_EF_CONC: pszEffectSuffix = "conc"; break;
		case LUA_EF_GAS: pszEffectSuffix = "gas"; break;
		case LUA_EF_INFECT: pszEffectSuffix = "infect"; break;
		case LUA_EF_RADIOTAG: pszEffectSuffix = "radiotag"; break;
		case LUA_EF_HEADSHOT: pszEffectSuffix = "headshot"; break;
		case LUA_EF_LEGSHOT: pszEffectSuffix = "legshot"; break;
		case LUA_EF_TRANQ: pszEffectSuffix = "tranq"; break;
		case LUA_EF_ACSPINUP: pszEffectSuffix = "acspinup"; break;
		case LUA_EF_SNIPERRIFLE: pszEffectSuffix = "sniperrifle"; break;
		case LUA_EF_SPEED_LUA1: pszEffectSuffix = "speedlua1"; break;
		case LUA_EF_SPEED_LUA2: pszEffectSuffix = "speedlua2"; break;
		case LUA_EF_SPEED_LUA3: pszEffectSuffix = "speedlua3"; break;
		case LUA_EF_SPEED_LUA4: pszEffectSuffix = "speedlua4"; break;
		case LUA_EF_SPEED_LUA5: pszEffectSuffix = "speedlua5"; break;
		case LUA_EF_SPEED_LUA6: pszEffectSuffix = "speedlua6"; break;
		case LUA_EF_SPEED_LUA7: pszEffectSuffix = "speedlua7"; break;
		case LUA_EF_SPEED_LUA8: pszEffectSuffix = "speedlua8"; break;
		case LUA_EF_SPEED_LUA9: pszEffectSuffix = "speedlua9"; break;
		case LUA_EF_SPEED_LUA10: pszEffectSuffix = "speedlua10"; break;
	}

	Q_snprintf( szLuaFunc, sizeof( szLuaFunc ), "player_on%s", pszEffectSuffix );

	// Set up the lua vars that get set
	char szLuaDuration[ 64 ];
	char szLuaIconDuration[ 64 ];
	char szLuaSpeed[ 64 ];

	Q_snprintf( szLuaDuration, sizeof( szLuaDuration ), "%s_duration", pszEffectSuffix );
	Q_snprintf( szLuaIconDuration, sizeof( szLuaIconDuration ), "%s_iconduration", pszEffectSuffix );
	Q_snprintf( szLuaSpeed, sizeof( szLuaSpeed ), "%s_speed", pszEffectSuffix );

	// Set the vars in lua - duration and icon duration
	switch( iEffect )
	{
		case LUA_EF_ONFIRE:
		case LUA_EF_CONC:
		case LUA_EF_GAS:
		case LUA_EF_INFECT:
		case LUA_EF_RADIOTAG:
		case LUA_EF_LEGSHOT:
		case LUA_EF_TRANQ:
			_scriptman.SetVar( szLuaDuration, flDuration );
			_scriptman.SetVar( szLuaIconDuration, flIconDuration );
			break;
	}

	// Set the vars in lua - speed
	switch( iEffect )
	{
		case LUA_EF_LEGSHOT:
		case LUA_EF_TRANQ:
		case LUA_EF_ACSPINUP:
		case LUA_EF_SNIPERRIFLE:
		case LUA_EF_SPEED_LUA1:
		case LUA_EF_SPEED_LUA2:
		case LUA_EF_SPEED_LUA3:
		case LUA_EF_SPEED_LUA4:
		case LUA_EF_SPEED_LUA5:
		case LUA_EF_SPEED_LUA6:
		case LUA_EF_SPEED_LUA7:
		case LUA_EF_SPEED_LUA8:
		case LUA_EF_SPEED_LUA9:
		case LUA_EF_SPEED_LUA10:
			_scriptman.SetVar( szLuaSpeed, flSpeed );
			break;
	}

	CFFLuaSC hContext( 2, this, pEffector );
	if( _scriptman.RunPredicates_LUA( NULL, &hContext, szLuaFunc ) )
	{
		// LUA function was found and run.

		// LUA function returned false. This means the player
		// should not take the effect.
		if( !hContext.GetBool() )
			return false;
		else
		{
			// Pick up the vars that could have been set.

			// Get the vars in lua - duration and icon duration
			switch( iEffect )
			{
				case LUA_EF_ONFIRE:
				case LUA_EF_CONC:
				case LUA_EF_GAS:
				case LUA_EF_INFECT:
				case LUA_EF_RADIOTAG:
				case LUA_EF_LEGSHOT:
				case LUA_EF_TRANQ:
					flDuration = _scriptman.GetFloat( szLuaDuration );
					flIconDuration = _scriptman.GetFloat( szLuaIconDuration );
					break;
			}

			// Get the vars in lua - speed
			switch( iEffect )
			{
				case LUA_EF_LEGSHOT:
				case LUA_EF_TRANQ:
				case LUA_EF_ACSPINUP:
				case LUA_EF_SNIPERRIFLE:
				case LUA_EF_SPEED_LUA1:
				case LUA_EF_SPEED_LUA2:
				case LUA_EF_SPEED_LUA3:
				case LUA_EF_SPEED_LUA4:
				case LUA_EF_SPEED_LUA5:
				case LUA_EF_SPEED_LUA6:
				case LUA_EF_SPEED_LUA7:
				case LUA_EF_SPEED_LUA8:
				case LUA_EF_SPEED_LUA9:
				case LUA_EF_SPEED_LUA10:
					flSpeed = _scriptman.GetFloat( szLuaSpeed );
					break;
			}

			if( flDuration < 0 )
				flDuration = -1;
			if( flIconDuration < 0 )
				flIconDuration = -1;
			if( flSpeed < 0 )
				flSpeed = -1;

			if( pflDuration )
				*pflDuration = flDuration;
			if( pflIconDuration )
				*pflIconDuration = flIconDuration;
			if( pflSpeed )
				*pflSpeed = flSpeed;

			return true;
		}
	}

	// Lua function didn't exist so function as normal
	return true;
}

//------------------------------------------------------------------------------
// Purpose : Do some kind of damage effect for the type of damage
//------------------------------------------------------------------------------
void CFFPlayer::DamageEffect(float flDamage, int fDamageType)
{
	if (fDamageType & DMG_POISON)
	{
		// Green damage indicator
		color32 green = {32, 64, 0, 50};
		UTIL_ScreenFade(this, green, 2.0f, 0.1f, FFADE_IN);

		ViewPunch(QAngle(random->RandomFloat(-1.0f, 1.0f), random->RandomFloat(-1.0f, 1.0f), random->RandomFloat(-1.0f, 1.0f)));
	}
	else if (fDamageType & DMG_BURN && GetClassSlot() != CLASS_PYRO)
	{
		CSingleUserRecipientFilter user(this);
		user.MakeReliable();

		UserMessageBegin(user, "FFViewEffect");
		WRITE_BYTE(FF_VIEWEFFECT_BURNING);
		WRITE_BYTE(min(30.0f * flDamage, 255));
		MessageEnd();

		ViewPunch(QAngle(random->RandomFloat(-1.0f, 1.0f), random->RandomFloat(-1.0f, 1.0f), random->RandomFloat(-1.0f, 1.0f)));
	}
	else
	{
		BaseClass::DamageEffect(flDamage, fDamageType);
	}
}

//-----------------------------------------------------------------------------
// Purpose: A function to handle all the flame stuff rather than having it
//			strewed throughout the code.
//-----------------------------------------------------------------------------
void CFFPlayer::SetFlameSpritesLifetime(float flLifeTime, float flFlameSize)
{
	CEntityFlame *pFlame = dynamic_cast <CEntityFlame *> (GetEffectEntity());

	// if no flame currently exists
	if (!pFlame )
	{
		if (flLifeTime <= 0.0f)
		{
			return;
		}

		pFlame = CEntityFlame::Create(this, true, flFlameSize);
		SetEffectEntity(pFlame);
	}
	else
	{
		if (flLifeTime <= 0.0f) //if this is part of the player extinguish then just return immediately
		{
			pFlame->Extinguish();
			return;
		}

		// this lame calculation copied from CEntityFlame::Create

		float xSize = CollisionProp()->OBBMaxs().x - CollisionProp()->OBBMins().x;
		float ySize = CollisionProp()->OBBMaxs().y - CollisionProp()->OBBMins().y;
		float size = ( xSize + ySize ) * 0.5f;
		size = size * flFlameSize;

		if ( size != pFlame->m_flSize )
		{
			//pFlame->SetSize(size); // i wish this would work and just grow the existing flame
			pFlame->Extinguish(); // kill the old smaller flame
			pFlame = CEntityFlame::Create(this, true, flFlameSize); // make a brand new bigger flame
			SetEffectEntity(pFlame);
		}
			

	}

	Assert(pFlame);

	if (!pFlame)
		return;

	// If we're reducing the lifecycle in order to remove the flame then check
	// that the flame is still going.
	// If it's not already going then we should return without doing anything otherwise
	// we can end up in a loop when the flame keeps notifying the player that it has
	// run out and this function is called.
	if (flLifeTime <= 0.0f)
	{
		 //&& pFlame->m_flLifetime > gpGlobals->curtime)
		pFlame->Extinguish();
	}
	else if (flLifeTime > 0.0f)
	{
		pFlame->SetLifetime(flLifeTime);

		// Take effect immediately
		pFlame->FlameThink();
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates camera position & mapguide stuff (called from CFFPlayer::Spawn()
// Fixes bug #0001767
//-----------------------------------------------------------------------------
void CFFPlayer::UpdateCamera( bool bUnassigned )
{
	Vector position( 0, 0, 0 );
	QAngle angles( 0, 0, 0 );

	// Unassigned logic.  Should start the mapguide if there's multiple points.  If there's only one, use its angles.  
	// Failing that, use the intermission.  If that fails, then set default stuff.
	if( bUnassigned )
	{
		// Start a new map overview guide
		if (!m_hNextMapGuide)
			m_hLastMapGuide = m_hNextMapGuide = FindMapGuide(MAKE_STRING("overview"));
		
		// m_hNextMapGuide = FindMapGuide(m_hLastMapGuide->m_iNextMapguide);

		// Check if mapguide was found & whether there is more than one
		if( m_hNextMapGuide )
		{
			CFFMapGuide *pNextOne = FindMapGuide( m_hNextMapGuide->m_iNextMapguide );
			
			// If there's no next one, then sod it!  Just use the overview mapguide's position & angles and chill.
			if( ! pNextOne )
			{
				CFFMapGuide *pCurrent = dynamic_cast< CFFMapGuide* >( m_hNextMapGuide.Get() );	
				
				if( pCurrent )
				{
					// use the angles
					SnapEyeAngles( pCurrent->GetAbsAngles() );
					SetAbsOrigin( pCurrent->GetAbsOrigin() );
					
					m_vecInfoIntermission = pCurrent->GetAbsAngles();
				}		

				// Don't use mapguides any more (since there's only one = static view).
				m_hLastMapGuide = m_hNextMapGuide = NULL;
			}	
			else
			{
				// we're ready to rock.  Mapguide shall proceed as ordered, guv'nor!
				m_flNextMapGuideTime = 0;
			}
		}
		else	// no mapguide found -- put the player at the intermission spot
		{			
			// this function will return a decent fallback value so just use what it sets regardless of whether it succeeds
			UTIL_GetIntermissionData( &position, &angles );
			SetAbsOrigin( position );				
			SnapEyeAngles( angles );

			// Send this to client so the client will actually look at what we want it to!
			m_vecInfoIntermission = angles;
		}
			
	}	// END if( unassigned )
	
	else	// this is the regular logic that is used most often
	{
		// If intermission data was found, use it.
		if( UTIL_GetIntermissionData( &position, &angles ) )
		{
			SetAbsOrigin( position );
			SnapEyeAngles( angles );

			// Don't need to bother with this if we're not unassigned afaik
			// m_vecInfoIntermission = angles;
		}
		else	// no intermission so use the mapguide overview point to start with
		{
			CFFMapGuide *pMapGuide = FindMapGuide(MAKE_STRING("overview"));

			if( pMapGuide )
			{				
				SnapEyeAngles( pMapGuide->GetAbsAngles() );
				SetAbsOrigin( pMapGuide->GetAbsOrigin() );
			}
			else	// no mapguide found; just use the default position & angles 
			{
				SetAbsOrigin( position );
				SnapEyeAngles( angles );
			}
		}				
	}
}
