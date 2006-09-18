//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for HL1.
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ff_player.h"
#include "ff_entity_system.h"	// Entity system
#include "ff_luacontext.h"
#include "ff_gamerules.h"
#include "ff_weapon_base.h"
#include "predicted_viewmodel.h"
#include "iservervehicle.h"
#include "viewport_panel_names.h"
#include "EntityFlame.h"

#include "ff_item_flag.h"
#include "ff_utils.h"
#include "ff_grenade_base.h"
#include "ff_buildableobjects_shared.h"
#include "ff_item_backpack.h"

#include "ff_team.h"			// team info
#include "in_buttons.h"			// for in_attack2
#include "ff_projectile_pipebomb.h"
#include "ff_grenade_emp.h"

#include "client.h"

#include <vector>
#include <algorithm>

// Re-adding this after STL to fix linux server issues
#undef MINMAX_H
#include "minmax.h"		

#include "ff_statslog.h"

#include "gib.h"

#include "omnibot_interface.h"

#include "te_effect_dispatch.h"

extern int gEvilImpulse101;
#define FF_PLAYER_MODEL "models/player/demoman/demoman.mdl"

// Oh no a gobal.
int g_iLimbs[CLASS_CIVILIAN + 1][5] = { { 0 } };

// grenade information
ConVar gren_timer("ffdev_gren_timer","4.0",0,"Timer length for all grenades.");
ConVar gren_throw_delay("ffdev_throw_delay","0.5",0,"Delay before primed grenades can be thrown.");
//ConVar gren_speed("ffdev_gren_speed","500.0",0,"Speed grenades are thrown at.");
ConVar gren_spawn_ang_x("ffdev_gren_spawn_ang_x","18.5",0,"X axis rotation grenades spawn at.");
//ConVar gren_forward_offset("ffdev_gren_forward_offset","8",0,"Forward offset grenades spawn at in front of the player.");

// For testing purposes
// [integer] Number of cells it takes to perform the "radar" command
static ConVar radar_num_cells( "ffdev_radar_num_cells", "5" );

// [integer] Distance of the "radar" pulse - ie. max distance someone
// can be from us when doing a "radar" command so that the player
// will show up on our screen
static ConVar radar_radius_distance( "ffdev_radar_radius_distance", "1024" );

// [integer] Time [in seconds] you have to wait before doing another "radar" command
static ConVar radar_wait_time( "ffdev_radar_wait_time", "5" );

// [integer] Max distance a player can be from us to be shown
static ConVar radiotag_distance( "ffdev_radiotag_distance", "1024" );

// [float] Time between radio tag updates
static ConVar radiotag_duration( "ffdev_radiotag_duration", "0.25" );

// [integer] Time in seconds you will stay 'tagged' once hit by a radio tag rifle round
static ConVar radiotag_draw_duration( "ffdev_radiotag_draw_duration", "60" );

// [float] Time between updating the players location
static ConVar location_update_frequency( "ffdev_location_update_frequency", "0.5" );

// status effect
ConVar ffdev_infect_freq("ffdev_infect_freq","2",0,"Frequency (in seconds) a player loses health from an infection");
ConVar ffdev_infect_damage("ffdev_infect_damage","8",0,"Amount of health a player loses while infected");
ConVar ffdev_regen_freq("ffdev_regen_freq","3",0,"Frequency (in seconds) a player loses health when a medic");
ConVar ffdev_regen_health("ffdev_regen_health","2",0,"Amount of health a player gains while a medic");
ConVar ffdev_regen_armor("ffdev_regen_armor","4",0,"Amount of armor a player gains while a engy");
ConVar ffdev_overhealth_freq("ffdev_overhealth_freq","3",0,"Frequency (in seconds) a player loses health when health > maxhealth");

static ConVar jerkmulti( "ffdev_concuss_jerkmulti", "0.0004", 0, "Amount to jerk view on conc" );

static ConVar ffdev_gibdamage("ffdev_gibdamage", "100");

extern ConVar sv_maxspeed;

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

// Bug #0000605: Rebo would like a way to spawn models on the fly in game to test stuff
LINK_ENTITY_TO_CLASS( ff_model_temp, CFFModelTemp );

// Bug #0000605: Rebo would like a way to spawn models on the fly in game to test stuff
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
			pPlayer->SetRespawnDelay( 5.0f );

		// Bug #0000700: people with infection should give medic kill if they suicide
		if( pPlayer->IsInfected() && pPlayer->GetInfector() )
			pPlayer->SetSpecialInfectedDeath();

		ClientKill( pPlayer->edict() );

		// Call lua player_killed on suicides
		entsys.SetVar( "killer", ENTINDEX( pPlayer ) );
		CFFLuaSC hPlayerKilled( 1, pPlayer );
		entsys.RunPredicates_LUA( NULL, &hPlayerKilled, "player_killed" );
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

BEGIN_SEND_TABLE_NOBASE( CFFPlayer, DT_FFLocalPlayerExclusive )

#ifdef EXTRA_LOCAL_ORIGIN_ACCURACY
	SendPropVector(SENDINFO(m_vecOrigin), 32,  SPROP_CHANGES_OFTEN, 0.0f, HIGH_DEFAULT),
#endif

	SendPropInt( SENDINFO( m_iShotsFired ), 8, SPROP_UNSIGNED ),

	// Buildables
	SendPropEHandle( SENDINFO( m_hDispenser ) ),
	SendPropEHandle( SENDINFO( m_hSentryGun ) ),
	SendPropEHandle( SENDINFO( m_hDetpack ) ),
	SendPropInt( SENDINFO( m_bBuilding ) ),
	SendPropInt( SENDINFO( m_iCurBuild ) ),

	// health/armor	
	SendPropFloat(SENDINFO(m_flArmorType)),

	SendPropInt(SENDINFO(m_iSkiState)),

	// Grenade Related
	SendPropInt( SENDINFO( m_iGrenadeState ), 2, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iPrimary ), 4 ),		// Not unsigned because can be -1
	SendPropInt( SENDINFO( m_iSecondary ), 4 ),
	SendPropFloat( SENDINFO( m_flServerPrimeTime ) ),

	// Map guide
	SendPropEHandle( SENDINFO( m_hNextMapGuide ) ),
	SendPropEHandle( SENDINFO( m_hLastMapGuide ) ),
	SendPropFloat( SENDINFO( m_flNextMapGuideTime ) ),

	SendPropFloat(SENDINFO(m_flConcTime)),

	SendPropFloat(SENDINFO(m_flSpeedModifier)),
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
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11 ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11 ),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),

	SendPropInt( SENDINFO( m_iClassStatus ) ),
	SendPropInt( SENDINFO( m_iSpyDisguise ) ), 

	SendPropInt(SENDINFO(m_iSpawnInterpCounter), 4),

	SendPropInt( SENDINFO( m_iSaveMe ), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_iEngyMe ), 1, SPROP_UNSIGNED ),
	SendPropBool( SENDINFO( m_bInfected ) ),
END_SEND_TABLE( )

class CFFRagdoll : public CBaseAnimatingOverlay
{
public:
	DECLARE_CLASS( CFFRagdoll, CBaseAnimatingOverlay );
	DECLARE_SERVERCLASS();

	// Transmit ragdolls to everyone.
	virtual int UpdateTransmitState()
	{
		return SetTransmitState( FL_EDICT_ALWAYS );
	}

public:
	// In case the client has the player entity, we transmit the player index.
	// In case the client doesn't have it, we transmit the player's model index, origin, and angles
	// so they can create a ragdoll in the right place.
	CNetworkHandle( CBaseEntity, m_hPlayer );	// networked entity handle 
	CNetworkVector( m_vecRagdollVelocity );
	CNetworkVector( m_vecRagdollOrigin );

	// State of player's limbs
	CNetworkVar(int, m_fBodygroupState);
	
	// Network this separately now that previous method broken
	CNetworkVar(int, m_nSkinIndex);
};

LINK_ENTITY_TO_CLASS( ff_ragdoll, CFFRagdoll );

IMPLEMENT_SERVERCLASS_ST_NOBASE( CFFRagdoll, DT_FFRagdoll )
	SendPropVector( SENDINFO(m_vecRagdollOrigin), -1,  SPROP_COORD ),
	SendPropEHandle( SENDINFO( m_hPlayer ) ),
	SendPropModelIndex( SENDINFO( m_nModelIndex ) ),
	SendPropInt		( SENDINFO(m_nForceBone), 8, 0 ),
	SendPropVector	( SENDINFO(m_vecForce), -1, SPROP_NOSCALE ),
	SendPropVector( SENDINFO( m_vecRagdollVelocity ) ),

	// State of player's limbs
	SendPropInt(SENDINFO(m_fBodygroupState)),
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
	m_PlayerAnimState = CreatePlayerAnimState( this, this, LEGANIM_9WAY, true );

	UseClientSideAnimation();
	m_angEyeAngles.Init();

	SetViewOffset( FF_PLAYER_VIEW_OFFSET );

	m_iLocalSkiState = 0;

	m_bBuilding = false;
	//m_bCancelledBuild = false;
	m_iWantBuild = FF_BUILD_NONE;
	m_iCurBuild = FF_BUILD_NONE;
	m_hDispenser = NULL;
	m_hSentryGun = NULL;
	m_hDetpack = NULL;	
	m_flBuildTime = 0.0f;

	m_flLastScoutRadarUpdate = 0.0f;

	m_bRadioTagged = false;
	m_flRadioTaggedStartTime = 0.0f;
	m_flRadioTaggedDuration = radiotag_draw_duration.GetInt();

	m_flLastRadioTagUpdate = 0.0f;

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
	m_flLastOverHealthTick = 0.0f;

	// Map guide stuff
	m_hNextMapGuide = NULL;
	m_flNextMapGuideTime = 0;

	// Class stuff
	m_fRandomPC = false;
	m_iNextClass = 0;

	m_flConcTime = 0;		// Not concussed on creation
	m_iClassStatus = 0;		// No class sorted yet

	m_flLastGassed = 0;

	m_Locations.RemoveAll();
	m_iClientLocation = 0;

	m_pBuildLastWeapon = NULL;

	m_flJumpTime = m_flFallTime = 0;

	m_iSpawnInterpCounter = 0;

	m_fl_LuaSet_PlayerRespawnDelay = 0.0f;
}

CFFPlayer::~CFFPlayer()
{
	m_PlayerAnimState->Release();
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
	// reset these every frame
	m_fBodygroupState = 0;	

	// Has our radio tag expired?
	if( m_bRadioTagged && ( ( m_flRadioTaggedStartTime + m_flRadioTaggedDuration ) < gpGlobals->curtime ) )
	{
		m_bRadioTagged = false;
	}

	// See if it's time to reset our saveme status
	if( ( m_flSaveMeTime < gpGlobals->curtime ) && ( m_iSaveMe != 0 ) )
		m_iSaveMe = 0;

	// See if it's time to reset our engyme status
	if( ( m_flEngyMeTime < gpGlobals->curtime ) && ( m_iEngyMe != 0 ) )
		m_iEngyMe = 0;

	// Update our list of tagged players that the client
	// should be "seeing" if it's time to do another update
	if( ( m_flLastRadioTagUpdate + radiotag_duration.GetFloat( ) ) < gpGlobals->curtime )
	{
		FindRadioTaggedPlayers();
	}

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
		// Players can jump while building now
		/*
		// Need to stop building because player somehow came off the ground
		if( !FBitSet( GetFlags(), FL_ONGROUND ) )
		{
			Warning( "[Buildable] Player building and came off the ground! Need to cancel build.\n" );

			// Send back through build process *should* cancel it correctly
			m_iWantBuild = m_iCurBuild;
			PreBuildGenericThink();
		}
		*/

		// Our origin has changed while building! no!!!!!!!!!!!!!!!!!!!!!!
		if( m_vecBuildOrigin.DistTo( GetAbsOrigin() ) > 128.0f )
		{
			Warning( "[Buildable] Player origin has changed!\n" );
			m_iWantBuild = m_iCurBuild;
			PreBuildGenericThink();
		}

		// Keeping the m_bBulding line in there in case we cancel the build cause
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
		// Disguising
		if (m_iNewSpyDisguise && gpGlobals->curtime > m_flFinishDisguise)
			FinishDisguise();

		// Sabotage!!
		SpySabotageThink();
	}

	// Do we need to do a class specific skill?
	if( m_afButtonPressed & IN_ATTACK2 )
		ClassSpecificSkill();

	else if (m_afButtonReleased & IN_ATTACK2)
		ClassSpecificSkill_Post();

	BaseClass::PreThink();
}

void CFFPlayer::PostThink()
{
	BaseClass::PostThink();

	MoveTowardsMapGuide();
 
	QAngle angles = GetLocalAngles();
	angles[PITCH] = 0;
	SetLocalAngles( angles );
	
	// Store the eye angles pitch so the client can compute its animation state correctly.
	m_angEyeAngles = EyeAngles();

    m_PlayerAnimState->Update( m_angEyeAngles[YAW], m_angEyeAngles[PITCH] );	
}


void CFFPlayer::Precache()
{
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
	PrecacheScriptSound("Grenade.Timer");
	PrecacheScriptSound("Grenade.Prime");
	PrecacheScriptSound("Player.Jump");
	PrecacheScriptSound("Player.Ammotoss");
	PrecacheScriptSound("speech.saveme");
	PrecacheScriptSound("radar.single_shot");
	PrecacheScriptSound("Player.bodysplat");
	PrecacheScriptSound("Item.Toss");
	PrecacheScriptSound("Player.Pain");
	PrecacheScriptSound("Player.Flameout");
	// Special case
	PrecacheScriptSound( EMP_SOUND );

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
		Q_snprintf(buf, 255, "%s.saveme", class_string);
		PrecacheScriptSound(buf);
	}

	for (int i = 0; i < 8; i++)
	{
		PrecacheModel(UTIL_VarArgs("models/gibs/gib%d.mdl", (i + 1)));
	}

	BaseClass::Precache();
}

extern CBaseEntity *g_pLastSpawn; // this is defined somewhere.. i'm using it :)

void CFFPlayer::SetLastSpawn( CBaseEntity *pEntity )
{
	g_pLastSpawn = pEntity;
}

CBaseEntity *CFFPlayer::EntSelectSpawnPoint()
{
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

	CBaseEntity *pSpot = NULL, *pGibSpot = NULL;

	pSpot = g_pLastSpawn;
	// Randomize the start spot
	// NOTE: given a larger range from the default SDK function so that players won't always
	// spawn at the "first" spawn point for their team if the spawns are put in a "bad" order
	for( int i = random->RandomInt(15,25); i > 0; i-- )
	{
		pSpot = gEntList.FindEntityByClassT( pSpot, CLASS_TEAMSPAWN );
		if( !pSpot )  // skip over the null point
			pSpot = gEntList.FindEntityByClassT( pSpot, CLASS_TEAMSPAWN );
	}

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
				Vector vecMins = -Vector( 16, 16, 0 );
				Vector vecMaxs = Vector( 16, 16, 70 );
				Vector vecOrigin = pSpot->GetAbsOrigin();

				// See if the spot is clear
				if( FFGameRules()->IsSpawnPointClear( pSpot, ( CBasePlayer * )this ) )
				{
					/*
#ifdef _DEBUG
					// Some debug listenserver visualization
					if( !engine->IsDedicatedServer() )
					{
						NDebugOverlay::Box( vecOrigin, vecMins, vecMaxs, 0, 0, 255, 100, 10.0f );
						NDebugOverlay::Line( vecOrigin, vecOrigin + Vector( 0, 0, 70 ), 0, 0, 255, false, 10.0f );
					}
#endif
					*/

					goto ReturnSpot;
				}
				else
				{
					/*
#ifdef _DEBUG
					// Some debug listenserver visualization
					if( !engine->IsDedicatedServer() )
					{
						NDebugOverlay::Box( vecOrigin, vecMins, vecMaxs, 0, 0, 255, 100, 10.0f );
						NDebugOverlay::Line( vecOrigin, vecOrigin + Vector( 0, 0, 70 ), 255, 0, 0, false, 10.0f );
					}
#endif
					*/
					// Not clear, so perhaps later we'll gib the guy here
					pGibSpot = pSpot;
				}
			}
		}

		// Increment pSpot
		pSpot = gEntList.FindEntityByClassT( pSpot, CLASS_TEAMSPAWN );
	} 
	while( pSpot != pFirstSpot ); // loop if we're not back to the start

	// At this point, we've checked all ff specific team spawns. If we
	// have a gib spot then kill people in that spot. Otherwise, we'll
	// go and check info_player_starts.

	pSpot = pGibSpot;
	if( pSpot )
	{
#ifdef _DEBUG
		Warning( "[EntSelectSpawnPoint] All spawns full, going to have to telefrag.\n" );
#endif

		CBaseEntity *pList[ 128 ];
		int count = UTIL_EntitiesInBox( pList, 128, pSpot->GetAbsOrigin() - Vector( 16, 16, 0 ), pSpot->GetAbsOrigin() + Vector( 16, 16, 70 ), FL_CLIENT | FL_NPC | FL_FAKECLIENT );
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
							ent->TakeDamage( CTakeDamageInfo( GetContainingEntity( INDEXENT( 0 ) ), GetContainingEntity( INDEXENT( 0 ) ), 400, DMG_GENERIC ) );
					}
					else
					{
						// TODO: Remove objects - buildables/grenades/projectiles - on the spawn point?
					}
				}
			}
		}

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
}

void CFFPlayer::Spawn()
{
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
	m_flSpeedModifier	= 1.0f;
	
	m_flSpeedModifierOld		= 1.0f;
	m_flSpeedModifierChangeTime	= 0;

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

	// Tried to spawn while unassigned (and not in a map guide)
	if (GetTeamNumber() == TEAM_UNASSIGNED)
	{
		// Start a new map overview guide
		if (!m_hNextMapGuide)
			m_hLastMapGuide = m_hNextMapGuide = FindMapGuide(MAKE_STRING("overview"));

		// Check if mapguide was found
		if (m_hNextMapGuide)
			m_flNextMapGuideTime = 0;

		// Set us in the intermission location instead
		else
		{
			// Find an info_player_start place to spawn observer mode at
			CBaseEntity *pSpawnSpot = gEntList.FindEntityByClassname( NULL, "info_intermission");

			// We could find one
			if (pSpawnSpot)
			{
				SetLocalOrigin(pSpawnSpot->GetLocalOrigin() + Vector(0, 0, 1));
				SetLocalAngles(pSpawnSpot->GetLocalAngles());
			}
			// We couldn't find one
			else
			{
				SetAbsOrigin(Vector( 0, 0, 0 ));
				SetAbsAngles(QAngle( 0, 0, 0 ));
			}
		}

		// Show the team menu
		ShowViewPortPanel(PANEL_TEAM, true);

		// Don't allow them to spawn
		return;
	}

	// They tried to spawn with no class and they are not a spectator (or randompc)
	if (GetClassSlot() == 0 && GetTeamNumber() != TEAM_SPECTATOR && !m_fRandomPC)
	{
		// Start a new map overview guide
		if (!m_hNextMapGuide)
			m_hLastMapGuide = m_hNextMapGuide = FindMapGuide(MAKE_STRING("overview"));

		// Check if mapguide was found
		if (m_hNextMapGuide)
			m_flNextMapGuideTime = 0;

		// TODO set something if no map guides on this map

		// Show the class select menu	
		// Bug #0000584: Switch teams results in the class menu appearing twice.
		//ShowViewPortPanel( PANEL_CLASS, true );

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

	SetModel( FF_PLAYER_MODEL );
	SetMoveType( MOVETYPE_WALK );
	RemoveSolidFlags( FSOLID_NOT_SOLID );

	SetupClassVariables();

	// Run this after SetupClassVariables in case lua is
	// manipulating the players' inventory
	CFFLuaSC hPlayerSpawn( 1, this );
	entsys.RunPredicates_LUA( NULL, &hPlayerSpawn, "player_spawn" );

	for (int i=0; i<NUM_SPEED_EFFECTS; i++)
		RemoveSpeedEffectByIndex( i );

	// equip the HEV suit
	EquipSuit();

	// Set on ground
	AddFlag(FL_ONGROUND);

	// Increment the spawn counter
	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;

	// get our stats id, just in case.
	m_iStatsID = g_StatsLog->GetPlayerID(engine->GetPlayerNetworkIDString(this->edict()), GetClassSlot(), GetTeamNumber(), engine->GetPlayerUserId(this->edict()), GetName());
}

// Mirv: Moved all this out of spawn into here
void CFFPlayer::SetupClassVariables()
{
	// Reset Engineer stuff
	m_pBuildLastWeapon = NULL;

	//m_hSaveMe = NULL;
	m_flSaveMeTime = 0.0f;
	m_flEngyMeTime = 0.0f;

	m_flPipebombShotTime = 0.0f;

	m_bSpecialInfectedDeath = false;

	// Reset Spy stuff
	m_fFeigned = false;
	m_flFinishDisguise = 0;
	m_iSpyDisguise = 0;
	m_iNewSpyDisguise = 0;
	m_flNextSpySabotageThink = 0.0f;
	m_flSpySabotageFinish = 0.0f;
	m_hSabotaging = NULL;

	m_Locations.RemoveAll();
	m_iClientLocation = 0;

	// Class system
	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	ClientPrint( this, HUD_PRINTNOTIFY, pPlayerClassInfo.m_szPrintName );

	m_iHealth		= pPlayerClassInfo.m_iHealth;
	m_iMaxHealth	= pPlayerClassInfo.m_iHealth;
	m_iArmor		= pPlayerClassInfo.m_iInitialArmour;
	m_iMaxArmor		= pPlayerClassInfo.m_iMaxArmour;
	m_flArmorType	= pPlayerClassInfo.m_flArmourType;
	m_flBaseArmorType = m_flArmorType;

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
	m_iMaxAmmo[GetAmmoDef()->Index(AMMO_RADIOTAG)] = pPlayerClassInfo.m_iMaxRadioTag;

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
}

void CFFPlayer::InitialSpawn( void )
{
	// Make sure they are dead
	m_lifeState = LIFE_DEAD;
	pl.deadflag = true;

	BaseClass::InitialSpawn( );

	m_iRadioTaggedAmmoIndex = GetAmmoDef( )->Index( AMMO_RADIOTAG );

	// Fixes the no model problem
	SetModel( FF_PLAYER_MODEL );

	// Set up their global voice channel
	m_iChannel = 0;

	// I'm putting this here, I'm not sure if it's the best place though
	// I wanted to make sure that the statslog was created or I would've put
	// it in the constructor - FryGuy
	m_iStatDeath = g_StatsLog->GetStatID("deaths");
	m_iStatTeamKill = g_StatsLog->GetStatID("teamkills");
	m_iStatKill = g_StatsLog->GetStatID("kills");
	m_iStatInfections = g_StatsLog->GetStatID("infections");
	m_iStatCures = g_StatsLog->GetStatID("cures");
	m_iStatHeals = g_StatsLog->GetStatID("heals");
	m_iStatHealHP = g_StatsLog->GetStatID("healhp");
	m_iStatCritHeals = g_StatsLog->GetStatID("critheals");
	m_iStatInfectCures = g_StatsLog->GetStatID("infectcures");

	//DevMsg("CFFPlayer::InitialSpawn");
}

//-----------------------------------------------------------------------------
// Purpose: This is the normal, noisey feign. It used to have a check for
//			velocity but not anymore.
//-----------------------------------------------------------------------------
void CFFPlayer::SpyFeign( void )
{
	// Just be on ground
	if (GetGroundEntity() == NULL)
		return;

	// A yell of pain
	if (!m_fFeigned)
	{
		EmitSound( "Player.Death" );
	}

	SpySilentFeign();
}

//-----------------------------------------------------------------------------
// Purpose: This doesn't have a maximum speed either.
//			That wasn't really a description of a purpose, I know.
//-----------------------------------------------------------------------------
void CFFPlayer::SpySilentFeign( void )
{
	// Must be on ground
	if (GetGroundEntity() == NULL)
		return;

	// Already feigned so remove all effects
	if (m_fFeigned)
	{
		// Yeah we're not feigned anymore bud
		m_fFeigned = false;

		CFFRagdoll *pRagdoll = dynamic_cast<CFFRagdoll *> (m_hRagdoll.Get());

		// Remove the ragdoll instantly
		pRagdoll->SetThink(&CBaseEntity::SUB_Remove);
		pRagdoll->SetNextThink(gpGlobals->curtime);

		// Visible and able to move again
		RemoveEffects(EF_NODRAW);
		RemoveFlag(FL_FROZEN);

		// Redeploy our weapon
		if (GetActiveWeapon() && GetActiveWeapon()->IsWeaponVisible() == false)
		{
			GetActiveWeapon()->Deploy();
			ShowCrosshair(true);
		}

		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent("unfeigned");						
		if(pEvent)
		{
			pEvent->SetInt("userid", this->GetUserID());
			gameeventmanager->FireEvent(pEvent, true);
		}
	}
	// Not already feigned, so collapse with ragdoll
	else
	{
		m_fFeigned = true;

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
		AddFlag(FL_FROZEN);

		// Holster our current weapon
		if (GetActiveWeapon())
			GetActiveWeapon()->Holster(NULL);

		CFFLuaSC hOwnerFeign( 1, this );
		// Find any items that we are in control of and let them know we feigned
		CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( NULL, "info_ff_script" );
		while( pEnt != NULL )
		{
			// Tell the ent that it died
			if (pEnt->GetOwnerEntity() == this)
				entsys.RunPredicates_LUA( pEnt, &hOwnerFeign, "ownerfeign" );

			// Next!
			pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( pEnt, "info_ff_script" );
		}

		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent("feigned");						
		if(pEvent)
		{
			pEvent->SetInt("userid", this->GetUserID());
			gameeventmanager->FireEvent(pEvent, true);
		}
	}
}

void CFFPlayer::Event_Killed( const CTakeDamageInfo &info )
{
	/*
	if( m_hSaveMe )
	{
		// This will kill it
		m_hSaveMe->SetLifeTime( gpGlobals->curtime );
	}
	*/
	if( m_iSaveMe )
		m_iSaveMe = 0;

	if( m_iEngyMe )
		m_iEngyMe = 0;

	// Log the death to the stats engine
	g_StatsLog->AddStat(m_iStatsID, m_iStatDeath, 1);

	// TODO: Take SGs into account here?
	CFFPlayer *pKiller = dynamic_cast<CFFPlayer *> (info.GetAttacker());
	
	// Log the correct stat for the killer
	if (pKiller)
	{
		if (g_pGameRules->PlayerRelationship(this, pKiller) == GR_TEAMMATE)
			g_StatsLog->AddStat(pKiller->m_iStatsID, m_iStatTeamKill, 1);
		else
			g_StatsLog->AddStat(pKiller->m_iStatsID, m_iStatKill, 1);
	}

	// Drop any grenades
	if (m_iGrenadeState != FF_GREN_NONE)
	{
		ThrowGrenade(gren_timer.GetFloat() - (gpGlobals->curtime - m_flServerPrimeTime), 0.0f);
		m_iGrenadeState = FF_GREN_NONE;
		m_flServerPrimeTime = 0;
		m_bEngyGrenWarned = false;
	}

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
	for (int i=0; i<NUM_SPEED_EFFECTS; i++)
	{
		RemoveSpeedEffectByIndex( i );
	}
	m_fLastHealTick = 0.0f;
	m_fLastInfectedTick = 0.0f;

	// Stop infection
	m_bInfected = false;

	// Beg; Added by Mulchman
	if( m_bBuilding )
	{
		switch( m_iCurBuild )
		{
			case FF_BUILD_DISPENSER:
			{
				if( m_hDispenser.Get() )
					( ( CFFDispenser * )m_hDispenser.Get() )->Cancel();
			}
			break;

			case FF_BUILD_SENTRYGUN:
			{
				if( m_hSentryGun.Get() )
					( ( CFFSentryGun * )m_hSentryGun.Get() )->Cancel();
			}
			break;

			case FF_BUILD_DETPACK: 
			{
				if( m_hDetpack.Get() )
					( ( CFFDetpack * )m_hDetpack.Get() )->Cancel();
			}
			break;
		}

		// Unlock the player if he/she got locked
		UnlockPlayer( );

		// Re-initialize
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
	if( m_bRadioTagged )
	{
		if( m_pWhoTaggedMe != NULL )
		{
			CFFPlayer *pPlayer = GetPlayerWhoTaggedMe();
			if( pPlayer )
				pPlayer->AddPoints( 2, true );
		}
	}

	// Reset this
	m_pWhoTaggedMe = NULL;

	// Reset the tag...
	m_bRadioTagged = false;

	// Get rid of fire
	Extinguish();	

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
	// Bug #0000461: Infect sound plays eventhough you are dead
	StopSound( "Player.DrownContinue" );

	// --> Mirv: Create backpack moved here to stop crash
	CFFItemBackpack *pBackpack = (CFFItemBackpack *) CBaseEntity::Create("ff_item_backpack", GetAbsOrigin(), GetAbsAngles());

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

		for (int i = 1; i < MAX_AMMO_SLOTS; i++)
			pBackpack->SetAmmoCount(i, GetAmmoCount(i));
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

	BaseClass::Event_Killed( info );

	// Set view angle + positions
	SetViewOffset(VEC_DEAD_VIEWHEIGHT);
	AddFlag(FL_DUCKING);
	QAngle eyeAngles = EyeAngles();
	eyeAngles.z = 50.0f;
	SnapEyeAngles(eyeAngles);

	if (!ShouldGib(info))
	{
		CreateRagdollEntity(&info);
		CreateLimbs(m_fBodygroupState);
	}
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
	}

	if ( pRagdoll )
	{
		pRagdoll->m_hPlayer = this;
		pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
		pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
		pRagdoll->m_nModelIndex = m_nModelIndex;
		pRagdoll->m_nForceBone = m_nForceBone;
		pRagdoll->m_vecForce = Vector(0, 0, 0);

		if (info && info->GetDamageType() & DMG_BLAST)
			pRagdoll->m_vecForce = 100.0f * info->GetDamageForce();

		// remove it after a time
		pRagdoll->SetThink( &CBaseEntity::SUB_Remove );
		pRagdoll->SetNextThink( gpGlobals->curtime + 30.0f );

		// Allows decapitation to continue into ragdoll state
		//DevMsg( "Createragdoll: %d\n", LastHitGroup() );
		pRagdoll->m_fBodygroupState = m_fBodygroupState;

		// Bugfix: #0000574: On death, ragdolls change to the blue team's skin
		pRagdoll->m_nSkinIndex = m_nSkin;
	}

	// ragdolls will be removed on round restart automatically
	m_hRagdoll = pRagdoll;
}

void CFFPlayer::DoAnimationEvent( PlayerAnimEvent_t event )
{
	m_PlayerAnimState->DoAnimationEvent( event );
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
		GiveAmmo(200, AMMO_NAILS);
		GiveAmmo(200, AMMO_SHELLS);
		GiveAmmo(200, AMMO_ROCKETS);
		GiveAmmo(200, AMMO_CELLS);
		GiveAmmo(200, AMMO_RADIOTAG);
		GiveAmmo(200, AMMO_DETPACK);
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
		WRITE_SHORT( GetLocationTeam() - 1 ); // changed
		MessageEnd();

		m_iClientLocation = entindex;
		Q_strncpy( m_szLastLocation, GetLocation(), sizeof( m_szLastLocation ) );
		m_iLastLocationTeam = GetLocationTeam() - 1;
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

int CFFPlayer::GetClassSlot()
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

	int iClasses[11] = {0};

	// Build up an array of all in-use classes
	for (int i = 1; i <= gpGlobals->maxClients; i++)
	{
		CFFPlayer *pPlayer = (CFFPlayer *) UTIL_PlayerByIndex(i);

		// Check player classes on this player's team
		if (pPlayer && pPlayer->GetTeamNumber() == GetTeamNumber())
		{
			int playerclass = pPlayer->GetClassSlot();

			if (playerclass > 0 && playerclass <= 10)
				iClasses[playerclass]++;
		}
	}

	std::vector<int> vecAvailable;

	// Build up a vector array of available classes
	for (int j = 1; j <= 10; j++)
	{
		int class_limit = pTeam->GetClassLimit(j);

		// This class is available
		if (class_limit == 0 || iClasses[j] < class_limit)
			vecAvailable.push_back(j);
	}

	// Make a random choice for randompcs
	if (m_fRandomPC)
	{
		int iNumAvail = (int) vecAvailable.size();

		// No available classes
		if (iNumAvail == 0)
			return 0;


		// Loop until we get a new class if there's more than one available
		while ((m_iNextClass = vecAvailable[(rand() % iNumAvail) ]) == GetClassSlot() && iNumAvail != 1);

		//DevMsg("randompc: Tried to select %d out of %d options\n", m_iNextClass, (int) vecAvailable.size());
	}
	// Check target class is still available
	else if (m_iNextClass != GetClassSlot())
	{
		// It's not available anymore, slowmow
		if (std::find(vecAvailable.begin(), vecAvailable.end(), m_iNextClass) == vecAvailable.end())
		{
			m_iNextClass = GetClassSlot();
			ClientPrint(this, HUD_PRINTNOTIFY, "#FF_ERROR_NOLONGERAVAILABLE");
			return GetClassSlot();
		}
	}

	// It's not available anymore(server changed class limits?)
	if (std::find(vecAvailable.begin(), vecAvailable.end(), m_iNextClass) == vecAvailable.end())
	{
		m_iNextClass = 0;
		ClientPrint(this, HUD_PRINTNOTIFY, "#FF_ERROR_NOLONGERAVAILABLE");
		return GetClassSlot();
	}

	// Now lets try reading this class, this shouldn't fail!
	if (!ReadPlayerClassDataFromFileForSlot(filesystem, Class_IntToString(m_iNextClass), &m_hPlayerClassFileInfo, GetEncryptionKey()))
	{
		//if (m_iNextClass != 0)
		AssertMsg(0, "Unable to read class script file, this shouldn't happen");
		return GetClassSlot();
	}

	const CFFPlayerClassInfo &pNewPlayerClassInfo = GetFFClassData();
	SetClassForClient(pNewPlayerClassInfo.m_iSlot);

	// Display our class information
	ClientPrint(this, HUD_PRINTNOTIFY, pNewPlayerClassInfo.m_szPrintName);
	ClientPrint(this, HUD_PRINTNOTIFY, pNewPlayerClassInfo.m_szDescription);

	// Remove all buildable items from the game
	RemoveItems();

	// Send a player class change event.
	IGameEvent *pEvent = gameeventmanager->CreateEvent("player_changeclass");
	if (pEvent)
	{
		pEvent->SetInt("userid", GetUserID());
		pEvent->SetInt("oldclass", GetClassForClient());
		pEvent->SetInt("newclass", m_iNextClass);
		gameeventmanager->FireEvent(pEvent, true);
	}

	// So the client can keep track
	SetClassForClient(m_iNextClass);


	return GetClassSlot();
}

void CFFPlayer::ChangeTeam(int iTeamNum)
{
	// clear the random player class flag, so player doesnt
	// immediately spawn when changing classes
	m_fRandomPC = false;
	BaseClass::ChangeTeam(iTeamNum);
}

void CFFPlayer::ChangeClass(const char *szNewClassName)
{
	//const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();
	CFFTeam *pTeam = GetGlobalFFTeam( GetTeamNumber() );

	// They are picking the randompc slot
	if( FStrEq( szNewClassName, "randompc" ) )
	{
		m_fRandomPC = true;
		KillAndRemoveItems();
		return;
	}
	else
		m_fRandomPC = false;

	// This shouldn't happen as class changes only allowed while on a team
	if( !pTeam )
		return;

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

	// Make sure they aren't already this class
	if (iClass == GetClassSlot())
		return;

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

	// Yup it is okay to change
	// We don't change class instantly, only when we spawn
	m_iNextClass = iClass;

	bool fInstantSwitch = strcmp(engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_classautokill"), "0") != 0;

	// Now just need a way to select which one you want
	if (fInstantSwitch || GetClassSlot() == 0)
	{
		// But for now we do have instant switching
		KillAndRemoveItems();

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

	// Hide motd panel if it's showing
	ShowViewPortPanel( PANEL_INFO, false );

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
		int iBestTeam = -1;
		float flBestCapacity = 9999.0f;

		// Loop from blue to green
		for( int iTeamToCheck = FF_TEAM_BLUE; iTeamToCheck <= FF_TEAM_GREEN; iTeamToCheck++ )
		{
			CFFTeam *pTeam = GetGlobalFFTeam(iTeamToCheck);

			// Don't bother with non-existant teams
			if( !pTeam )
				continue;

			// Take team limits into account when calculating the best team to join
			float flTeamCapacity = (float)iTeamNumbers[iTeamToCheck] / ( pTeam->GetTeamLimits() == 0 ? 32 : pTeam->GetTeamLimits() );

			//DevMsg( "Team %d: %d (%f)\n", iTeamToCheck, iTeamNumbers[iTeamToCheck], flTeamCapacity );

			// Is this the best team to join so far (and there is space on it)
			if( flTeamCapacity < flBestCapacity && ( pTeam->GetTeamLimits() == 0 || iTeamNumbers[iTeamToCheck] < pTeam->GetTeamLimits() ) )
			{
				flBestCapacity = flTeamCapacity;
				iBestTeam = iTeamToCheck;
			}
		}

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

	// Bug #0000700: people with infection should give medic kill if they suicide	
	if( IsInfected() && GetInfector() )
		SetSpecialInfectedDeath();

	// set their class to unassigned, so that they don't spawn
	// immediately when changing teams
	//ReadPlayerClassDataFromFileForSlot( filesystem, "unassigned", &m_hPlayerClassFileInfo, GetEncryptionKey() );
	SetClassForClient(0);

	RemoveItems();

	// Only kill the player if they are alive on a team
	if (IsAlive() && GetTeamNumber() >= TEAM_BLUE)
	{
		KillPlayer();
	}
	
	ChangeTeam(iTeam);
	
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
	m_iCurBuild = FF_BUILD_NONE;
	m_iWantBuild = FF_BUILD_NONE;

	// Remove buildables if they exist
	if( m_hDispenser.Get() )
		( ( CFFDispenser * )m_hDispenser.Get() )->Cancel();

	if( m_hSentryGun.Get() )
		( ( CFFSentryGun * )m_hSentryGun.Get() )->Cancel();

	if( m_hDetpack.Get() )
	{
		( ( CFFDetpack * )m_hDetpack.Get() )->Cancel();
	}
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

	if( m_bBuilding )
	{
		// Holster our current weapon
		// Holster our current weapon
		//if( GetActiveWeapon() )
			//GetActiveWeapon()->Holster( NULL );
		m_pBuildLastWeapon = GetActiveFFWeapon();
		if( m_pBuildLastWeapon )
			m_pBuildLastWeapon->Holster( NULL );
	}
}

void CFFPlayer::UnlockPlayer( void )
{
	SetMoveType( MOVETYPE_WALK );
	SetAbsVelocity( Vector( 0, 0, 0 ) );
}

void CFFPlayer::FindRadioTaggedPlayers( void )
{
	// Reset
	m_hRadioTaggedList.RemoveAll();

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
		//if( ToFFPlayer( pPlayer->m_pWhoTaggedMe )->GetTeamNumber() != GetTeamNumber() )
		//{
		if( g_pGameRules->PlayerRelationship( this, ToFFPlayer( pPlayer->m_pWhoTaggedMe ) ) != GR_TEAMMATE )
			continue;
		//}

//		// Skip if they're not someone we can hurt
//		if( !g_pGameRules->FPlayerCanTakeDamage( pPlayer, this ) )
//			continue;

		// Get their origin
		Vector vecPlayerOrigin = pPlayer->GetFeetOrigin();

		// Skip if they're out of range
		if( vecOrigin.DistTo( vecPlayerOrigin ) > radiotag_distance.GetInt() )
			continue;

		// We're left w/ a player who's within range
		// Add player to a list and send off to client

		// Create a single object
		ESP_Shared_s hObject;
		hObject.m_iEntIndex = pPlayer->entindex() - 1;
		hObject.m_iClass = pPlayer->GetClassSlot();
		hObject.m_iTeam = pPlayer->GetTeamNumber();
		hObject.m_bDucked = ( pPlayer->GetFlags() & FL_DUCKING ) ? true : false;
		hObject.m_vecOrigin = vecPlayerOrigin;
		hObject.m_vecVel = pPlayer->GetAbsVelocity(); //+ Vector( 0, 0, 6 );

		// Add object to radio tagged array
		m_hRadioTaggedList.AddToTail( hObject );

		// Omni-bot: Notify the bot he has detected someone.
		if(IsBot())
		{
			Omnibot::Notify_RadioTagUpdate(this, pPlayer->edict());							
		}
	}

	int iCount = m_hRadioTaggedList.Count();
	if( iCount > 0 )
	{
		// Only send this message to the local player	
		CSingleUserRecipientFilter user( ( CBasePlayer * )this );
		user.MakeReliable();

		// Start the message block
		UserMessageBegin( user, "RadioTagUpdate" );

		WRITE_SHORT( iCount );

		for( int i = 0; i < iCount; i++ )
		{
			int iInfo = m_hRadioTaggedList[ i ].m_iTeam;
			iInfo += m_hRadioTaggedList[ i ].m_iClass << 4;

			WRITE_SHORT( m_hRadioTaggedList[ i ].m_iEntIndex );
			WRITE_WORD( iInfo );
			WRITE_BYTE( m_hRadioTaggedList[ i ].m_bDucked ? ( byte )1 : ( byte )0 );
			WRITE_VEC3COORD( m_hRadioTaggedList[ i ].m_vecOrigin );
			WRITE_VEC3COORD( m_hRadioTaggedList[ i ].m_vecVel );
		}
			
		// End the message block
		MessageEnd();		

		// Doing an update now...
		m_flLastRadioTagUpdate = gpGlobals->curtime;
	}
}

void CFFPlayer::Command_WhatTeam( void )
{
	//DevMsg( "[What Team] You are currently on team: %i\n", GetTeamNumber() );
	//DevMsg( "[What Team] Dispenser Text: %s\n", m_szCustomDispenserText );

	//char szBuffer[128];
	//Q_snprintf(szBuffer, 127, "[What Team] m_iSpyDisguise: %i, Disguised? %s, Team: %i, Class: %i, My Team: %i\n", m_iSpyDisguise, IsDisguised() ? "yes" : "no", GetDisguisedTeam(), GetDisguisedClass(), GetTeamNumber());
	//ClientPrint(UTIL_GetCommandClient(), HUD_PRINTCONSOLE, szBuffer);
	Warning( "[Player %s] IsAlive(): %s, H: %i, A: %i, LIFE_STATE: %i\n", GetPlayerName(), IsAlive() ? "Yes" : "No", GetHealth(), GetArmor(), m_lifeState );
}

void CFFPlayer::Command_HintTest( void )
{	
	//ShowViewPortPanel( PANEL_HINT, true );
	//UTIL_HudHintText( this, "#FF_HELLO" );
	FF_HudHint( this, 0, 1, "#FF_HELLO" );
	// ted - HL2 Hint Text buffer overrun PoC, don't enable... doesn't affect FF hints!
	/*
	char buf[254];
	for(unsigned int i = 0; i < sizeof(buf); i++)
	{
		buf[i] = 'A' + i % 26;
	}
	buf[sizeof(buf) - 1] = '\0';
	UTIL_HudHintText(this, buf);
	*/
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
	if( m_hDispenser.Get() )
		( ( CFFDispenser * )m_hDispenser.Get() )->SetText( m_szCustomDispenserText );
}

void CFFPlayer::Command_Radar( void )
{
	// Player issued the command "radar"
	// Can only do it every CVAR seconds (atm)
	// Cost is CVAR cells (atm)
	if( gpGlobals->curtime > ( m_flLastScoutRadarUpdate + ( float )radar_wait_time.GetInt() ) )
	{
		// See if the player has enough ammo
		if( GetAmmoCount( AMMO_CELLS ) >= radar_num_cells.GetInt() )
		{
			// Bug #0000531: Everyone hears radar
			//CPASAttenuationFilter sndFilter;
			//sndFilter.RemoveAllRecipients();
			//sndFilter.AddRecipient( ( CBasePlayer * )this );
			CSingleUserRecipientFilter sndFilter( ( CBasePlayer * )this );
			EmitSound( sndFilter, entindex(), "radar.single_shot");

			// Remove ammo
			RemoveAmmo( radar_num_cells.GetInt(), AMMO_CELLS );

			CUtlVector< ScoutRadar_s > hRadarInfo;

			Vector vecOrigin = GetFeetOrigin();

			for( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );
				if( pPlayer && ( pPlayer != this ) )
				{
					// Bug #0000497: The scout radar picks up on people who are observing/spectating.
					// If the player isn't alive
					if( !pPlayer->IsAlive() )
						continue;

					// Bug #0000497: The scout radar picks up on people who are observing/spectating.
					// If the player is a spectator
					if( pPlayer->IsObserver() )
						continue;

					// Bug #0000497: The scout radar picks up on people who are observing/spectating.
					// If the player is a spectator
					if( FF_IsPlayerSpec( pPlayer ) )
						return;

					Vector vecPlayerOrigin = pPlayer->GetFeetOrigin();
					float flDist = vecOrigin.DistTo( vecPlayerOrigin );

					if( flDist <= ( float )radar_radius_distance.GetInt() )
					{
						int iInfo = pPlayer->GetTeamNumber();
						iInfo += pPlayer->GetClassSlot() << 4;

						if( ( g_pGameRules->PlayerRelationship( this, pPlayer ) == GR_NOTTEAMMATE ) &&
							( pPlayer->IsDisguised() ) )
						{
							iInfo = pPlayer->GetDisguisedTeam();
							iInfo += pPlayer->GetDisguisedClass() << 4;
						}

						ScoutRadar_s hInfo( iInfo, ( pPlayer->GetFlags() & FL_DUCKING ) ? ( byte )1 : ( byte )0, vecPlayerOrigin );
						hRadarInfo.AddToTail( hInfo );

						// Omni-bot: Notify the bot he has detected someone.
						if(IsBot())
						{
							Omnibot::Notify_RadarDetectedEnemy(this, pPlayer->edict());							
						}
					}
				}
			}

			int iCount = hRadarInfo.Count();
			if( iCount >= 0 )
			{
				// Only send this message to the local player	
				CSingleUserRecipientFilter user( ( CBasePlayer * )this );
				user.MakeReliable();

				// Start the message block
				UserMessageBegin( user, "RadarUpdate" );

				// Tell client how much to expect
				WRITE_SHORT( iCount );

				for( int i = 0; i < iCount; i++ )
				{
					WRITE_WORD( hRadarInfo[ i ].m_iInfo );
					WRITE_BYTE( hRadarInfo[ i ].m_bDucking );
					WRITE_VEC3COORD( hRadarInfo[ i ].m_vecOrigin );
				}

				// End the message block
				MessageEnd();
			}

			// Update our timer
			m_flLastScoutRadarUpdate = gpGlobals->curtime;
		}
		else
		{
			ClientPrint(this, HUD_PRINTCONSOLE, "#FF_RADARCELLS");
		}
	}
	else
	{
		ClientPrint(this, HUD_PRINTCONSOLE, "#FF_RADARTOOSOON");
	}
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
			m_iDetpackTime = clamp( m_iDetpackTime, 0, 60 );
			bInRange = false;
		}

		if(!bInRange)
		{
			char szBuffer[16];
			Q_snprintf( szBuffer, sizeof( szBuffer ), "%i", m_iDetpackTime );

			// EDIT: There's no gcc/g++ itoa apparently?
			//ClientPrint(this, HUD_PRINTCONSOLE, "FF_INVALIDTIMER", itoa((int) m_iDetpackTime, szBuffer, 10));
			ClientPrint(this, HUD_PRINTCONSOLE, "FF_INVALIDTIMER", szBuffer);
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

void CFFPlayer::PreBuildGenericThink( void )
{
	// This is associated with
	// Bug #0000333: Buildable Behavior (non build slot) while building
	bool bDeployHack = false;

	//
	// March 21, 2006 - Re-working entire build process
	//

	if( !m_bBuilding )
	{
		m_bBuilding = true;

		// Store the player's current origin
		m_vecBuildOrigin = GetAbsOrigin();

		// See if player is in a no build area first
		if( !FFScriptRunPredicates( this, "onbuild", true ) && ( ( m_iWantBuild == FF_BUILD_DISPENSER ) || ( m_iWantBuild == FF_BUILD_SENTRYGUN ) ) )
		{
			// Notify the bot: convert this to an event?
			if(IsBot())
			{
				Omnibot::Notify_Build_CantBuild(this, m_iWantBuild);
			}

			// Re-initialize
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;

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
		}
		*/

		// See if the user has already built this item
		if( ( ( m_iWantBuild == FF_BUILD_DISPENSER ) && ( m_hDispenser.Get() ) ) ||
			( ( m_iWantBuild == FF_BUILD_SENTRYGUN ) && ( m_hSentryGun.Get() ) ) ||
			( ( m_iWantBuild == FF_BUILD_DETPACK ) && ( m_hDetpack.Get() ) ) )
		{
			// Notify the bot: convert this to an event?
			if(IsBot())
			{
				Omnibot::Notify_Build_AlreadyBuilt(this, m_iWantBuild);
			}

			switch( m_iWantBuild )
			{
				case FF_BUILD_DISPENSER: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_DISPENSER_ALREADYBUILT" ); break;
				case FF_BUILD_SENTRYGUN: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_SENTRYGUN_ALREADYBUILT" ); break;
				case FF_BUILD_DETPACK: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_DETPACK_ALREADYSET" ); break;
			}

			// Re-initialize
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;

			return;
		}

		// See if the user has appropriate ammo
		if( ( ( m_iWantBuild == FF_BUILD_DISPENSER ) && ( GetAmmoCount( AMMO_CELLS ) < 100 ) ) ||
			( ( m_iWantBuild == FF_BUILD_SENTRYGUN ) && ( GetAmmoCount( AMMO_CELLS ) < 130 ) ) ||
			( ( m_iWantBuild == FF_BUILD_DETPACK ) && ( GetAmmoCount( AMMO_DETPACK ) < 1 ) ) )
		{
			// Notify the bot: convert this to an event?
			if(IsBot())
			{
				Omnibot::Notify_Build_NotEnoughAmmo(this, m_iWantBuild);
			}

			switch( m_iWantBuild )
			{
				case FF_BUILD_DISPENSER: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_DISPENSER_NOTENOUGHAMMO" ); break;
				case FF_BUILD_SENTRYGUN: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_SENTRYGUN_NOTENOUGHAMMO" ); break;
				case FF_BUILD_DETPACK: ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_DETPACK_NOTENOUGHAMMO" ); break;
			}
			
			// Re-initialize
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;

			return;
		}

		// See if on ground...
		if( !FBitSet( GetFlags(), FL_ONGROUND ) )
		{
			// Notify the bot: convert this to an event?
			if(IsBot())
			{
				Omnibot::Notify_Build_MustBeOnGround(this, m_iWantBuild);				
			}

			ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_MUSTBEONGROUND" );

			// Re-initialize
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;

			return;
		}

		// Our neat buildable info container
		CFFBuildableInfo hBuildInfo( this, m_iWantBuild );

		// Will we be able to build here?
		if( hBuildInfo.BuildResult() == BUILD_ALLOWED )
		{
			// BUILD! - Create the actual item finally
			m_iCurBuild = m_iWantBuild;
			
			LockPlayerInPlace();

			switch( m_iCurBuild )
			{
				case FF_BUILD_DISPENSER:
				{
					// Changed to building straight on ground (Bug #0000191: Engy "imagines" SG placement, then lifts SG, then back to imagined position.)
					CFFDispenser *pDispenser = CFFDispenser::Create( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles(), this );
					
					// Set custom text
					pDispenser->SetText( m_szCustomDispenserText );					

					// Mirv: Store future ground location + orientation
					pDispenser->SetGroundOrigin( hBuildInfo.GetBuildOrigin() );
					pDispenser->SetGroundAngles( hBuildInfo.GetBuildAngles() );

					// Set network var
					m_hDispenser = pDispenser;

					// Set the time it takes to build
					m_flBuildTime = gpGlobals->curtime + 2.0f;

					if(IsBot())
					{
						Omnibot::Notify_DispenserBuilding(this, pDispenser->edict());
					}
				}
				break;

				case FF_BUILD_SENTRYGUN:
				{
					// Changed to building straight on ground (Bug #0000191: Engy "imagines" SG placement, then lifts SG, then back to imagined position.)
					CFFSentryGun *pSentryGun = CFFSentryGun::Create( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles(), this );
				
					// Mirv: Store future ground location + orientation
					pSentryGun->SetGroundOrigin( hBuildInfo.GetBuildOrigin() );
					pSentryGun->SetGroundAngles( hBuildInfo.GetBuildAngles() );

					// Set network var
					m_hSentryGun = pSentryGun;

					// Set the time it takes to build
					m_flBuildTime = gpGlobals->curtime + 5.0f;	// |-- Mirv: Bug #0000127: when building a sentry gun the build finishes before the sound

					if(IsBot())
					{
						Omnibot::Notify_SentryBuilding(this, pSentryGun->edict());
					}
				}
				break;

				case FF_BUILD_DETPACK:
				{
					// Changed to building straight on ground (Bug #0000191: Engy "imagines" SG placement, then lifts SG, then back to imagined position.)
					CFFDetpack *pDetpack = CFFDetpack::Create( hBuildInfo.GetBuildOrigin(), hBuildInfo.GetBuildAngles(), this );

					// Set the fuse time
					pDetpack->m_iFuseTime = m_iDetpackTime;

					// Mirv: Store future ground location + orientation
					pDetpack->SetGroundOrigin( hBuildInfo.GetBuildOrigin() );
					pDetpack->SetGroundAngles( hBuildInfo.GetBuildAngles() );

					// Set network var
					m_hDetpack = pDetpack;

					// Set time it takes to build
					m_flBuildTime = gpGlobals->curtime + 3.0f; // mulch: bug 0000337: build time 3 seconds for detpack

					if(IsBot())
					{
						Omnibot::Notify_DetpackBuilding(this, pDetpack->edict());
					}
				}
				break;
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

			CBaseAnimating *pEntity = NULL;

			// Cancel the build
			switch( m_iCurBuild )
			{
				case FF_BUILD_DISPENSER: pEntity = m_hDispenser.Get(); break;
				case FF_BUILD_SENTRYGUN: pEntity = m_hSentryGun.Get(); break;
				case FF_BUILD_DETPACK:   pEntity = m_hDetpack.Get(); break;
			}

			// If object exists (was crashing ff_restartround), stop building
			if( pEntity )
			{
				CFFBuildableObject *pBuildable = dynamic_cast< CFFBuildableObject * >( pEntity );
				if( pBuildable )
					pBuildable->Cancel();
			}

			// Unlock the player
			UnlockPlayer();

			// Mirv: Cancel build timer
			CSingleUserRecipientFilter user( this );
			user.MakeReliable();
			UserMessageBegin( user, "FF_BuildTimer" );
				WRITE_SHORT( m_iCurBuild );
				WRITE_FLOAT( 0 );
			MessageEnd();

			// Re-initialize
			m_iCurBuild = FF_BUILD_NONE;
			m_iWantBuild = FF_BUILD_NONE;
			m_bBuilding = false;

			// This is associated with
			// Bug #0000333: Buildable Behavior (non build slot) while building
			bDeployHack = true;
		}
		else
		{
			ClientPrint( this, HUD_PRINTCENTER, "#FF_BUILDERROR_MULTIPLEBUILDS" );
		}
	}

	// This is associated with
	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( bDeployHack )
	{
		// Need to set m_bBuilding false before bringing out a new weapon...
		if( m_pBuildLastWeapon )
			m_pBuildLastWeapon->Deploy();
	}
}

void CFFPlayer::PostBuildGenericThink( void )
{
	// If we're building
	if( m_bBuilding )
	{
		// Find out what we're building and drop it to the floor
		switch( m_iCurBuild )
		{
			case FF_BUILD_DISPENSER:
			{
				if( m_hDispenser.Get() )
				{
					( ( CFFDispenser * )m_hDispenser.Get() )->GoLive();

					IGameEvent *pEvent = gameeventmanager->CreateEvent( "build_dispenser" );
					if( pEvent )
					{
						pEvent->SetInt( "userid", GetUserID() );
						gameeventmanager->FireEvent( pEvent, true );
					}					
				}
			}
			break;

			case FF_BUILD_SENTRYGUN:
			{
				if( m_hSentryGun.Get() )
				{
					( ( CFFSentryGun * )m_hSentryGun.Get() )->GoLive();

					IGameEvent *pEvent = gameeventmanager->CreateEvent( "build_sentrygun" );
					if( pEvent )
					{
						pEvent->SetInt( "userid", GetUserID() );
						gameeventmanager->FireEvent( pEvent, true );
					}
				}
			}
			break;

			case FF_BUILD_DETPACK: 
			{
				if( m_hDetpack.Get() )
				{
					// Remove what it cost to build. Do it here to fix
					// bug #0000327
					RemoveAmmo( 1, AMMO_DETPACK );

					( ( CFFDetpack * )m_hDetpack.Get( ) )->GoLive();

					IGameEvent *pEvent = gameeventmanager->CreateEvent( "build_detpack" );
					if( pEvent )
					{
						pEvent->SetInt( "userid", GetUserID() );
						gameeventmanager->FireEvent( pEvent, true );
					}
				}
			}
			break;

		}

		// Unlock the player
		UnlockPlayer();

		// Reset stuff		
		m_iCurBuild = FF_BUILD_NONE;
		m_iWantBuild = FF_BUILD_NONE;
		m_bBuilding = false;
		//m_bCancelledBuild = false;
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
	if( m_pBuildLastWeapon )
		m_pBuildLastWeapon->Deploy();
}
// Sev's test animation thing
void CFFPlayer::Command_SevTest( void )
{
/* STOLEN!
	Vector vecOrigin = GetAbsOrigin( );
	vecOrigin.z += 48;

	CFFSevTest *pSevTest = CFFSevTest::Create( vecOrigin, GetAbsAngles( ) );
	if( pSevTest )
		pSevTest->GoLive( );

	// BASTARD! :P
	if (engine->Cmd_Argc() == 4)
	{
		Vector vPush(atof(engine->Cmd_Argv(1)),atof(engine->Cmd_Argv(2)),atof(engine->Cmd_Argv(3)));
		ApplyAbsVelocityImpulse(vPush);
	}

	STOLEN AGAIN. NINJA!
*/

	if (engine->Cmd_Argc() > 1)
		entsys.DoString(engine->Cmd_Args());
}

/**
	FlagInfo
*/
void CFFPlayer::Command_FlagInfo( void )
{	
	CFFLuaSC hFlagInfo( 1, this );
	entsys.RunPredicates_LUA(NULL, &hFlagInfo, "flaginfo");
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
			entsys.RunPredicates_LUA( pEnt, &hDropItemCmd, "dropitemcmd" );
				//pEnt->Drop(30.0f, 500.0f);
		}

		// Next!
		pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( pEnt, CLASS_INFOSCRIPT );
	}
}

/**
	Discard
*/
void CFFPlayer::Command_Discard( void )
{
	CFFItemBackpack *pBackpack = NULL;

	int iDroppableAmmo[MAX_AMMO_TYPES] = {0};

	// Check we have the ammo to discard first
		if (GetClassSlot() != CLASS_ENGINEER)
	{
		// get ammo used by our weapons
		for (int i = 0; i < MAX_WEAPON_SLOTS; i++)
		{
			if (GetWeapon(i))
			{
				int ammoid = GetWeapon(i)->GetPrimaryAmmoType();

				if (ammoid > -1)
				{
					iDroppableAmmo[ammoid] = 1;
				}
			}
		}

		// Add ammo if they have any
		for (int i = 0; i < MAX_AMMO_TYPES; i++)
		{
			if (!iDroppableAmmo[i] && GetAmmoCount(i) > 0)
			{
				if (!pBackpack)
					pBackpack = (CFFItemBackpack *) CBaseEntity::Create("ff_item_backpack", GetAbsOrigin(), GetAbsAngles());

				// Check again in case we failed to make one
				if (pBackpack)
				{
					pBackpack->SetAmmoCount(i, GetAmmoCount(i));
					RemoveAmmo(GetAmmoCount(i), i);
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

		// UNDONE: This is causing the sniper trace to ignore the backpack
		//pBackpack->SetOwnerEntity(this);

		Vector vForward;
		AngleVectors(EyeAngles(), &vForward);

		vForward *= 420.0f;

		// Bugfix: Floating objects
		if (vForward.z < 1.0f)
			vForward.z = 1.0f;

		pBackpack->SetAbsVelocity(vForward);
		pBackpack->SetAbsOrigin(GetLegacyAbsOrigin());

		// Play a sound
		EmitSound("Item.Toss");
	}
}

void CFFPlayer::Command_SaveMe( void )
{
	if( m_flSaveMeTime < gpGlobals->curtime )
	{
		m_iSaveMe = 1;

		// Set the time we can do another saveme at
		m_flSaveMeTime = gpGlobals->curtime + 7.0f;
	}

	// Do the actual sound always... cause spamming the sound is fun
	CPASAttenuationFilter sndFilter( this );

	// Remove people not allied to us (or not on our team)
	for( int i = TEAM_BLUE; i <= TEAM_GREEN; i++ )
	{
		if( FFGameRules()->IsTeam1AlliedToTeam2( GetTeamNumber(), i ) == GR_NOTTEAMMATE )
			sndFilter.RemoveRecipientsByTeam( GetGlobalFFTeam( i ) );
	}

	// Compose the sound
	char szBuf[ 64 ];
	Q_snprintf( szBuf, sizeof( szBuf ), "%s.saveme", Class_IntToString( GetClassSlot() ) );
	EmitSound( sndFilter, entindex(), szBuf );

	/*
	// MEDIC!
	if( !m_hSaveMe )
	{
		// Spawn the glyph
		m_hSaveMe = ( CFFSaveMe * )CreateEntityByName( "ff_saveme" );

		if( m_hSaveMe )
		{		
			m_hSaveMe->SetOwnerEntity( this );
			m_hSaveMe->Spawn();
			m_hSaveMe->SetLifeTime( gpGlobals->curtime + FF_SAVEME_LIFETIME );
			m_hSaveMe->FollowEntity(this, true);
		}
	}

	// play the sound
	CPASAttenuationFilter sndFilter( this );

	// remove all other teams except our own
	for( int iTeamToCheck = FF_TEAM_BLUE; iTeamToCheck <= FF_TEAM_GREEN; iTeamToCheck++ )
	{
		if (iTeamToCheck != GetTeamNumber() || ( FFGameRules()->IsTeam1AlliedToTeam2( iTeamToCheck, GetTeamNumber() ) == GR_NOTTEAMMATE ) )
			sndFilter.RemoveRecipientsByTeam( GetGlobalFFTeam( iTeamToCheck ) );
	}
	
	// BugID #0000332: PrecacheScriptSound 'speech.saveme' failed, no such sound script entry
	char buf[64];
	Q_snprintf(buf, 63, "%s.saveme", Class_IntToString(GetClassSlot()));
	EmitSound(sndFilter, entindex(), buf);
	*/
}

void CFFPlayer::Command_EngyMe( void )
{
	if( m_flEngyMeTime < gpGlobals->curtime )
	{
		m_iEngyMe = 1;

		// Set the time we can do another engyme at
		m_flEngyMeTime = gpGlobals->curtime + 7.0f;
	}

	// Do the actual sound always... cause spamming the sound is fun
	CPASAttenuationFilter sndFilter( this );

	// Remove people not allied to us (or not on our team)
	for( int i = TEAM_BLUE; i <= TEAM_GREEN; i++ )
	{
		if( FFGameRules()->IsTeam1AlliedToTeam2( GetTeamNumber(), i ) == GR_NOTTEAMMATE )
			sndFilter.RemoveRecipientsByTeam( GetGlobalFFTeam( i ) );
	}

	// Compose the sound
	char szBuf[ 64 ];
	Q_snprintf( szBuf, sizeof( szBuf ), "%s.engyme", Class_IntToString( GetClassSlot() ) );
	EmitSound( sndFilter, entindex(), szBuf );
}

void CFFPlayer::StatusEffectsThink( void )
{
	if( m_bGassed )
	{
		// Yeah... this needs work.
		m_bGassed = false;
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
	if( ( ( GetClassSlot() == CLASS_MEDIC ) || ( GetClassSlot() == CLASS_ENGINEER ) ) &&
		( gpGlobals->curtime > ( m_fLastHealTick + ffdev_regen_freq.GetFloat() ) ) )
	{		
		m_fLastHealTick = gpGlobals->curtime;

		if( GetClassSlot() == CLASS_MEDIC )
		{
			// add the regen health
			// Don't call CFFPlayer::TakeHealth as it will clear status effects
			// Bug #0000528: Medics can self-cure being caltropped/tranq'ed
			if( BaseClass::TakeHealth( ffdev_regen_health.GetInt(), DMG_GENERIC ) )			
			//if( TakeHealth( ffdev_regen_health.GetInt(), DMG_GENERIC ) )
			{				
				// make a sound if we did
				//EmitSound( "medkit.hit" );
			}
		}
		else if( GetClassSlot() == CLASS_ENGINEER )
		{
			// add the regen armor
			m_iArmor.GetForModify() = clamp( m_iArmor + ffdev_regen_armor.GetInt(), 0, m_iMaxArmor );
		}
	}

	// Bug #0000485: If you're given beyond 100% health, by a medic, the health doesn't count down back to 100.
	// Reduce health if we're over healthed (health > maxhealth
	if( m_iHealth > m_iMaxHealth )
	{
		if( gpGlobals->curtime > ( m_flLastOverHealthTick + ffdev_overhealth_freq.GetFloat() ) )
		{
			m_flLastOverHealthTick = gpGlobals->curtime;
			m_iHealth = max( m_iHealth - ffdev_regen_health.GetInt(), m_iMaxHealth );
		}
	}

	// If the player is infected, then take appropriate action
	if( IsInfected() && ( gpGlobals->curtime > ( m_fLastInfectedTick + ffdev_infect_freq.GetFloat() ) ) )
	{
		bool bIsInfected = true;

		// Need to check to see if the medic who infected us has changed teams
		// or dropped - switching to EHANDLE will handle the drop case
		if( m_hInfector )
		{
			CFFPlayer *pInfector = ToFFPlayer( m_hInfector );

			AssertMsg( pInfector, "[Infect] pInfector == NULL\n" );
		
			// Medic changed teams
			if( pInfector->GetTeamNumber() != m_iInfectedTeam )
				m_bInfected = false;
		}
		else
		{
			// Player dropped
			m_bInfected = false;
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
			CTakeDamageInfo info( pInfector, pInfector, ffdev_infect_damage.GetInt(), DMG_POISON );
			//info.SetDamageForce( Vector( 0, 0, -1 ) );
			//info.SetDamagePosition( Vector( 0, 0, 1 ) );
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
			data.m_vOrigin = GetLegacyAbsOrigin() - Vector( 0, 0, 16.0f );
			data.m_vStart = GetAbsVelocity();
			data.m_flScale = 1.0f;
			DispatchEffect( "FF_InfectionEffect", data );

			CEffectData data2;
			data2.m_vOrigin = EyePosition() - Vector( 0, 0, 16.0f );
			data2.m_vStart = GetAbsVelocity();
			data2.m_flScale = 1.0f;			
			DispatchEffect( "FF_InfectionEffect", data2 );
			*/

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
	if( m_bImmune )
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
			case LUA_EF_GAS: Gas( flEffectDuration, flIconDuration ); break;
			case LUA_EF_INFECT: Infect( this ); break;
			case LUA_EF_RADIOTAG: SetRadioTagged( NULL, gpGlobals->curtime, flEffectDuration ); break;

			case LUA_EF_HEADSHOT:
			{
			}
			break;
			
			case LUA_EF_LEGSHOT: AddSpeedEffect( SE_LEGSHOT, flEffectDuration, flSpeed, SEM_BOOLEAN, FF_STATUSICON_LEGINJURY, flIconDuration, true ); break;
			case LUA_EF_TRANQ: AddSpeedEffect( SE_TRANQ, flEffectDuration, flSpeed, SEM_BOOLEAN | SEM_HEALABLE, FF_STATUSICON_TRANQUILIZED, flIconDuration, true ); break;
			case LUA_EF_CALTROP: AddSpeedEffect( SE_CALTROP, flEffectDuration, flSpeed, SEM_ACCUMULATIVE | SEM_HEALABLE, FF_STATUSICON_CALTROPPED, flIconDuration, true ); break;
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
		case LUA_EF_CONC: return (( m_flConcTime == -1 ) || ( m_flConcTime > gpGlobals->curtime )); break;
		case LUA_EF_GAS: return IsGassed(); break;
		case LUA_EF_INFECT: return IsInfected(); break;
		case LUA_EF_RADIOTAG: return IsRadioTagged(); break;

		case LUA_EF_HEADSHOT:
		{
		}
		break;

		case LUA_EF_LEGSHOT: IsSpeedEffectSet( SE_LEGSHOT ); break;
		case LUA_EF_TRANQ: IsSpeedEffectSet( SE_TRANQ ); break;
		case LUA_EF_CALTROP: IsSpeedEffectSet( SE_CALTROP ); break;
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
		case LUA_EF_CALTROP: RemoveSpeedEffect( SE_CALTROP, true ); break;
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
	Assert((mod & SEM_BOOLEAN)|(mod & SEM_ACCUMULATIVE));

	if (mod & SEM_BOOLEAN)
	{
		for (int i = 0; i < NUM_SPEED_EFFECTS; i++)
		{
			// we'll overwrite the old one
			if (m_vSpeedEffects[i].type == type)
				break;
		}
	}
	else
	{
		while (m_vSpeedEffects[i].active && (i != NUM_SPEED_EFFECTS))
			i++;
	}

	if (i == NUM_SPEED_EFFECTS)
	{
		Warning("ERROR: Too many speed effects. Raise NUM_SPEED_EFFECTS");
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

		int iIcon = -1;

		// Get its icon
		switch( m_vSpeedEffects[ iSpeedEffectIndex ].type )
		{
			case SE_TRANQ: iIcon = FF_STATUSICON_TRANQUILIZED; break;
			case SE_CALTROP: iIcon = FF_STATUSICON_CALTROPPED; break;
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

	// go apply all speed effects
	for (int i = 0; i < NUM_SPEED_EFFECTS; i++)
	{
		if (m_vSpeedEffects[i].active)
			flSpeed *= m_vSpeedEffects[i].speed;
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

void CFFPlayer::Infect( CFFPlayer *pInfector )
{
	if( !IsInfected() && !m_bImmune )
	{
		// they aren't infected or immune, so go ahead and infect them
		m_bInfected = true;
		m_fLastInfectedTick = gpGlobals->curtime;
		m_hInfector = pInfector;
		m_iInfectedTeam = pInfector->GetTeamNumber();

		EmitSound( "Player.DrownStart" );	// |-- Mirv: [TODO] Change to something more suitable

		g_StatsLog->AddStat(pInfector->m_iStatsID, m_iStatInfections, 1);

		// And now.. an effect
		CSingleUserRecipientFilter user(this);
		user.MakeReliable();

		UserMessageBegin(user, "FFViewEffect");
		WRITE_BYTE(FF_VIEWEFFECT_INFECTED);
		WRITE_FLOAT(999.0f);
		MessageEnd();
	}
}
void CFFPlayer::Cure( CFFPlayer *pCurer )
{
	if( IsInfected() )
	{
		// they are infected, so go ahead and cure them
		m_bInfected = false;
		m_fLastInfectedTick = 0.0f;
		m_hInfector = NULL;
		// Bug# 0000503: "Immunity" is not in the mod
		m_bImmune = true;
		m_flImmuneTime = gpGlobals->curtime + 10.0f;

		// credit the curer with a score
		pCurer->IncrementFragCount( 1 );

		// Log this in the stats
		g_StatsLog->AddStat(pCurer->m_iStatsID, m_iStatCures, 1);
	}

	// Hack-ish - removing infection effect
	if( !pCurer )
	{
		m_bInfected = false;
		m_fLastInfectedTick = 0.0f;
		m_hInfector = NULL;
	}

	// Bug #0000528: Medics can self-cure being caltropped/tranq'ed
	ClearSpeedEffects( SEM_HEALABLE );

	// Wiki says curing removes everything but concussion
	Extinguish();
}

void CFFPlayer::ApplyBurning( CFFPlayer *hIgniter, float scale, float flIconDuration )
{
	// Okay, now pyros don't catch fire at all
	if (GetClassSlot() == CLASS_PYRO)
	{
		return;
	}

	// send the status icon to be displayed
	CSingleUserRecipientFilter user( (CBasePlayer *)this );
	user.MakeReliable();
	UserMessageBegin(user, "StatusIconUpdate");
		WRITE_BYTE( FF_STATUSICON_BURNING );
		WRITE_FLOAT( flIconDuration );
	MessageEnd();

	// set them on fire
	if (!m_iBurnTicks)
		m_flNextBurnTick = gpGlobals->curtime + 1.25;
	m_iBurnTicks = (GetClassSlot()==CLASS_PYRO)?4:8;
	m_flBurningDamage = 0.75f*m_flBurningDamage + scale*((GetClassSlot()==CLASS_PYRO)?8.0:16.0);

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

	Ignite( 10.0, false, 8, false );

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
	if( m_bBuilding )
		return;

	// Bug #0000366: Spy's cloaking & grenade quirks
	// Spy shouldn't be able to prime grenades when feigned
	if (m_fFeigned)
		return;

	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	// we have a primary grenade type
	if ( strcmp( pPlayerClassInfo.m_szPrimaryClassName, "None" ) != 0 )
	{
		if(m_iPrimary > 0)
		{
			// ax1
			EmitSound("Grenade.Prime");

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
	if( m_bBuilding )
		return;

	// Bug #0000366: Spy's cloaking & grenade quirks
	// Spy shouldn't be able to prime grenades when feigned
	if (m_fFeigned)
		return;

    const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	// we have a secondary grenade type
	if ( strcmp( pPlayerClassInfo.m_szSecondaryClassName, "None" ) != 0 )
	{
		if(m_iSecondary > 0)
		{
			// ax1
			EmitSound("Grenade.Prime");

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
	if(gpGlobals->curtime - m_flServerPrimeTime < gren_throw_delay.GetFloat())
	{
		// release this grenade at the earliest opportunity
		m_bWantToThrowGrenade = true;
		return;
	}

	ThrowGrenade(gren_timer.GetFloat() - (gpGlobals->curtime - m_flServerPrimeTime));
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

bool CFFPlayer::IsGrenadePrimed(void)
{
	return ( ( m_iGrenadeState == FF_GREN_PRIMEONE ) || ( m_iGrenadeState == FF_GREN_PRIMETWO ) );
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
		if( !m_bEngyGrenWarned && ( gpGlobals->curtime > ( m_flServerPrimeTime + gren_timer.GetFloat() - 0.685f ) ) )
		{
			m_bEngyGrenWarned = true;
			EmitSound( EMP_SOUND );
		}
	}	

	if(m_bWantToThrowGrenade && gpGlobals->curtime - m_flServerPrimeTime >= gren_throw_delay.GetFloat())
	{
		Command_ThrowGren();
		return;
	}

	if ( (m_flServerPrimeTime != 0 ) && ( ( gpGlobals->curtime - m_flServerPrimeTime ) >= gren_timer.GetFloat() ) )
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
		vecSrc = GetLegacyAbsOrigin();

		VectorAngles( vecForward, angAngles );
		angAngles.x -= gren_spawn_ang_x.GetFloat();

		UTIL_SetOrigin(pGrenade, vecSrc);

		// Stationary
		if (fTimer != 0)
		{
			AngleVectors(angAngles, &vecVelocity);
			VectorNormalize(vecVelocity);
			vecVelocity *= flSpeed; //gren_speed.GetFloat();	// |-- Mirv: So we can drop grenades
		}
		else
			vecVelocity = Vector(0, 0, 0);

		pGrenade->Spawn();
		pGrenade->SetAbsVelocity(vecVelocity);
		pGrenade->SetThrower(this);
		pGrenade->SetOwnerEntity(this);
		pGrenade->ChangeTeam(GetTeamNumber());

		pGrenade->SetDetonateTimerLength( fTimer );
		pGrenade->SetupInitialTransmittedVelocity(vecVelocity);

		// Special case for emps since their explode sound starts before it actually explodes
		if( ( GetClassSlot() == CLASS_ENGINEER ) && ( m_bEngyGrenWarned ) && ( pGrenade->Classify() == CLASS_GREN_EMP ) )
			dynamic_cast< CFFGrenadeEmp * >( pGrenade )->SetWarned();

		if (fTimer > 0)
			pGrenade->m_fIsHandheld = false;
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

	// Bug #0000781: Placing a detpack can be interrupted
	if( !m_bBuilding )
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

	// call script: player_ondamage(player, damageinfo)	
	CFFLuaSC func;
	func.Push(this);
	func.Push(&info);
	func.CallFunction("player_ondamage");

	// go take the damage first
	if ( !g_pGameRules->FPlayerCanTakeDamage( this, info.GetAttacker() ) )
	{
        // Refuse the damage
		return 0;
	}

	// tag the player if hit by radio tag ammo
	if( inputInfo.GetAmmoType() == m_iRadioTaggedAmmoIndex )
	{
		SetRadioTagged( ToFFPlayer( info.GetAttacker() ), gpGlobals->curtime, radiotag_draw_duration.GetInt() );
	}

	// if it's a pyro, they take half damage
	if ( GetClassSlot() == CLASS_PYRO && info.GetDamageType()&DMG_BURN )
	{
		info.SetDamage(info.GetDamage()/2);
	}

	// keep track of amount of damage last sustained
	m_lastDamageAmount = info.GetDamage();

	// Armor. 
	if (!(info.GetDamageType() & (DMG_FALL | DMG_DROWN | DMG_POISON | DMG_RADIATION | DMG_DIRECT)))// armor doesn't protect against fall or drown damage!
	{
		//float flNew = info.GetDamage() * flRatio;
		float fFullDamage = info.GetDamage();

		float fArmorDamage = fFullDamage * m_flArmorType;
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

		info.SetDamage(fHealthDamage);
	}

	// Don't call up the baseclass, it does all this again
	// Call instead the CBaseCombatChracter one which actually applies the damage
	fTookDamage = CBaseCombatCharacter::OnTakeDamage( info );

	// Early out if the base class took no damage
	if ( !fTookDamage )
		return 0;

	// add to the damage total for clients, which will be sent as a single
	// message at the end of the frame
	// todo: remove after combining shotgun blasts?
	if ( info.GetInflictor() && info.GetInflictor()->edict() )
		m_DmgOrigin = info.GetInflictor()->GetAbsOrigin();

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
	if (!(info.GetDamageType() & DMG_FALL))
	{
		EmitSound("Player.Pain");
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
	return (GetHealth() <= -ffdev_gibdamage.GetFloat());
}

bool CFFPlayer::Event_Gibbed(const CTakeDamageInfo &info)
{
	m_takedamage	= DAMAGE_NO;
	AddSolidFlags( FSOLID_NOT_SOLID );
	m_lifeState		= LIFE_DEAD;
	AddEffects( EF_NODRAW ); // make the model invisible.
	pl.deadflag = true;

	SetMoveType(MOVETYPE_FLYGRAVITY);

	SetFlameSpritesLifetime(-1.0f);

	SetThink(&CBasePlayer::PlayerDeathThink);
	SetNextThink( gpGlobals->curtime + 0.1f );

	CEffectData data;
	data.m_nEntIndex = entindex();
	data.m_vOrigin = GetAbsOrigin();
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
	if( flDuration == -1 )
		m_flConcTime = flDuration;
	else
		m_flConcTime = gpGlobals->curtime + flDuration;

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
		ViewPunch((*viewjerk) * jerkmulti.GetFloat() * flDistance);
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


	SetFlameSpritesLifetime(flFlameLifetime);

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
			WRITE_BYTE(FF_STATUSICON_BURNING);
			WRITE_FLOAT(0.0);
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

	SetFlameSpritesLifetime(-1.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Gas a player
//-----------------------------------------------------------------------------
void CFFPlayer::Gas( float flDuration, float flIconDuration )
{
	// The gas effect is limited to once per second
	if (m_flLastGassed + 1.0f > gpGlobals->curtime)
		return;

	m_bGassed = true;

	m_flLastGassed = gpGlobals->curtime;	

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
	m_bGassed = false;

	// Remove gas
	CSingleUserRecipientFilter user( ( CBasePlayer * )this );
	user.MakeReliable();

	UserMessageBegin( user, "StatusIconUpdate" );
		WRITE_BYTE( FF_STATUSICON_CONCUSSION );
		WRITE_FLOAT( 0.0f );
	MessageEnd();
}

//-----------------------------------------------------------------------------
// Purpose: Heal player above their maximum
//-----------------------------------------------------------------------------
int CFFPlayer::Heal(CFFPlayer *pHealer, float flHealth)
{
	if (!edict() || m_takedamage < DAMAGE_YES)
		return 0;

	// 150% max specified in the wiki
	if( ( float )m_iHealth >= ( float )( m_iMaxHealth * 1.5f ) )
		return 0;

	int iOriginalHP = m_iHealth;

	// Also medpack boosts health to maximum + then carries on to 150%
	if( m_iHealth < m_iMaxHealth )
		m_iHealth = m_iMaxHealth;
	else
		// Bug #0000467: Medic can't give over 100% health [just added in the "m_iHealth =" line...]
		m_iHealth = min( ( float )( m_iHealth + flHealth ), ( float )( m_iMaxHealth * 1.5f ) );

	// Log the added health
	g_StatsLog->AddStat(pHealer->m_iStatsID, m_iStatHeals, 1);
	g_StatsLog->AddStat(pHealer->m_iStatsID, m_iStatHealHP, m_iHealth - iOriginalHP);
	
	// Critical heal is when they are <= 15hp
	if (iOriginalHP <= 15)
		g_StatsLog->AddStat(pHealer->m_iStatsID, m_iStatCritHeals, 1);

	if (IsInfected())
	{
		g_StatsLog->AddStat(pHealer->m_iStatsID, m_iStatInfectCures, 1);
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

int CFFPlayer::TakeEmp()
{
	// Only rockets and shells explode from EMPs, unless you are a pyro
	// in which case cells will do too!
	// EMPs also reduce your ammo store by 25%!
	// Values calculated  from TFC
    int ammodmg = 0;

	int iShells = GetAmmoDef()->Index(AMMO_SHELLS);
	int iRockets = GetAmmoDef()->Index(AMMO_ROCKETS);
	int iCells = GetAmmoDef()->Index(AMMO_CELLS);

	ammodmg += GetAmmoCount(iShells) * 0.5f;
	SetAmmoCount(GetAmmoCount(iShells) * 0.75f, iShells);

	ammodmg += GetAmmoCount(iRockets) * 1.3f;
	SetAmmoCount(GetAmmoCount(iRockets) * 0.75f, iRockets);

	if (GetClassSlot() == CLASS_PYRO)
	{
		ammodmg += GetAmmoCount(iCells) * 1.3f;
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
	if(engine->Cmd_Argc( ) < 3)
		return;

	if (!m_bDisguisable) 
	{
		ClientPrint(this, HUD_PRINTTALK, "#FF_SPY_NODISGUISENOW");
		return;
	}

	const char *szTeam = engine->Cmd_Argv( 1 );
	const char *szClass = engine->Cmd_Argv( 2 );
	int iTeam = 0, iClass = 0;

	// Allow either specify enemy/friendly or an actual team
	if (FStrEq(szTeam, "enemy"))
		// TODO: Check friendliness of the other team (do a loop methinks)
		iTeam = ( ( GetTeamNumber() == TEAM_BLUE ) ? TEAM_RED : TEAM_BLUE );
	else if (FStrEq(szTeam, "friendly"))
		iTeam = GetTeamNumber();
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
	
	// Bail if we don't have a team yet
	if( !iTeam )
	{
		Warning( "[Disguise] Disguise command must be in the proper format!\n" );
		return;
	}


	// Now for the class. Allow numbers 1-9 and strings like "soldier".
	// Not allowing single characters representing the first letter of the class.
	if( ( strlen( szClass ) == 1 ) && ( szClass[ 0 ] >= '1' ) && ( szClass[ 0 ] <= '9' ) )
		iClass = atoi( szClass );
	else
		iClass = Class_StringToInt( szClass );

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
		Warning( "[Disguise] Invalid team for this map!\n" );

		Omnibot::Notify_CantDisguiseAsTeam(this, iTeam);
		return;
	}

	if( pTeam->GetClassLimit( iClass ) == -1 )
	{
		// TODO: Nice hud msg!
		Warning( "[Disguise] Invalid class for this map!\n" );

		Omnibot::Notify_CantDisguiseAsClass(this, iClass);
		return;
	}

	Warning( "[Disguise] [Server] Disguise team: %i, Disguise class: %i\n", iTeam, iClass );

	// Now do the actual disguise
	SetDisguise(iTeam, iClass);

	ClientPrint( this, HUD_PRINTTALK, "#FF_SPY_DISGUISING" );

	// Notify the bot: convert this to an event?
	if(IsBot())
	{
		// TODO: This should probably pass in iTeam & iClass as
		// these two functions won't have the right shit yet?
		Omnibot::Notify_Disguising(this, GetNewDisguisedTeam(), GetNewDisguisedClass());
	}
}

bool CFFPlayer::IsDisguised( void )
{
	return ( GetClassSlot() == CLASS_SPY ) && ( m_iSpyDisguise != 0 );
}

int CFFPlayer::GetDisguisedTeam( void )
{
	if( IsDisguised() )	
		return ( m_iSpyDisguise & 0x0000000F );

	return 0;
}

int CFFPlayer::GetDisguisedClass( void )
{
	if( IsDisguised() )
		return ( ( m_iSpyDisguise & 0xFFFFFFF0 ) >> 4 );

	return 0;
}

// Server only
int CFFPlayer::GetNewDisguisedTeam( void )
{
	// Assumes we're a spy and currently disguising
	return ( m_iNewSpyDisguise & 0x0000000F );
}

// Server only
int CFFPlayer::GetNewDisguisedClass( void )
{
	// Assumes we're a spy and currently disguising
	return ( ( m_iNewSpyDisguise & 0xFFFFFFF0 ) >> 4 );
}

//-----------------------------------------------------------------------------
// Purpose: Reset the model, skin and any flags
//-----------------------------------------------------------------------------
void CFFPlayer::ResetDisguise()
{
	if (!IsDisguised())
		return;

	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();

	SetModel(pPlayerClassInfo.m_szModel);
	m_nSkin = GetTeamNumber() - FF_TEAM_BLUE;

	m_iNewSpyDisguise = 0;
	m_iSpyDisguise = 0;

	ClientPrint(this, HUD_PRINTTALK, "#FF_SPY_LOSTDISGUISE");

	// Notify the bot: convert this to an event?
	if(IsBot())
	{
		Omnibot::Notify_DisguiseLost(this);
	}
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
			SetModel(pPlayerClassInfo->m_szModel);
			m_nSkin = GetNewDisguisedTeam() - TEAM_BLUE; // since m_nSkin = 0 is blue
		}
	}

	m_iSpyDisguise = m_iNewSpyDisguise;
	m_iNewSpyDisguise = 0;

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
	bInstant = true;
#endif

	m_iNewSpyDisguise = iTeam;
	m_iNewSpyDisguise += iClass << 4;
	
	// TODO: Time logic
	if (bInstant)
	{
		m_flFinishDisguise = 0;
	}
	else
	{
		m_flFinishDisguise = gpGlobals->curtime + 7.0f;
	}
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
		case LUA_AMMO_RADIOTAG:
		case LUA_AMMO_DETPACK:
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
		case LUA_AMMO_RADIOTAG:
		case LUA_AMMO_DETPACK:
			RemoveAmmo( iAmount, LookupLuaAmmo( iAmmoType ) );
			break;

		case LUA_AMMO_GREN1:
			AddPrimaryGrenades( -iAmount );
			break;

		case LUA_AMMO_GREN2:
			AddSecondaryGrenades( -iAmount );
			break;
	}
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
	if (GetTeamNumber() > TEAM_SPECTATOR)
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
			// Show menu again if we are spectator
			if (GetTeamNumber() == TEAM_SPECTATOR)
				ShowViewPortPanel(PANEL_MAPGUIDE, true);

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
		t = SimpleSpline(t);

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
bool CFFPlayer::IsInNoBuild() const
{
	return !FFScriptRunPredicates( (CBaseEntity*)this, "onbuild", true );
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
			
			ClientPrint(this, HUD_PRINTTALK, "#FF_SPY_REVEALEDSPY");

			// This the correct func for logs?
			UTIL_LogPrintf("%s just exposed an enemy spy!\n", STRING(ffplayer->pl.netname));
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
// Purpose: Attempts to sabotage the entity the player is looking at.
//			Handles both the timers and the actual sabotage itself.
//-----------------------------------------------------------------------------
void CFFPlayer::SpySabotageThink()
{
	if (m_flNextSpySabotageThink > gpGlobals->curtime)
		return;

	m_flNextSpySabotageThink = gpGlobals->curtime + 0.2f;

	// We have to be under a particular speed to sabotage
	if (GetAbsVelocity().LengthSqr() > 100 * 100)
	{
		// Cancel anything currently going on
		if (m_hSabotaging)
		{
			CSingleUserRecipientFilter user(this);
			user.MakeReliable();
			UserMessageBegin(user, "FF_BuildTimer");
			WRITE_SHORT(0);
			WRITE_FLOAT(0);
			MessageEnd();
		}

		m_hSabotaging = NULL;
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
		if (pBuildable == NULL || !pBuildable->CanSabotage() || pBuildable->GetTeamNumber() == GetTeamNumber() || GetDisguisedTeam() != pBuildable->GetTeamNumber())
		{
			// Not something we can sabotage, stop 
			if (m_hSabotaging)
			{
				CSingleUserRecipientFilter user(this);
				user.MakeReliable();
				UserMessageBegin(user, "FF_BuildTimer");
				WRITE_SHORT(0);
				WRITE_FLOAT(0);
				MessageEnd();
			}

			// Remember that we aren't sabotaging
			m_hSabotaging = NULL;

			return;
		}

		int iBuildableType;

		// Determine the correct item that'll be shown on the menu
		if (pBuildable->Classify() == CLASS_SENTRYGUN)
			iBuildableType = FF_BUILD_SENTRYGUN;
		else
			iBuildableType = FF_BUILD_DISPENSER;

		// Reset the time left until the sabotage is finished
		m_flSpySabotageFinish = gpGlobals->curtime + 3.0f;

		// Now send off the timer
		CSingleUserRecipientFilter user(this);
		user.MakeReliable();
		UserMessageBegin(user, "FF_BuildTimer");
		WRITE_SHORT(iBuildableType);
		WRITE_FLOAT(3.0f);
		MessageEnd();

		// Now remember what we're sabotaging
		m_hSabotaging = pBuildable;
	}
	// Sabotage state has not changed
	else
	{
		// If we are sabotaging then check if we've timed out
		if (m_hSabotaging && m_flSpySabotageFinish <= gpGlobals->curtime)
		{
			ClientPrint(this, HUD_PRINTCENTER, "#FF_BUILDINGSABOTAGED");

			// Fire an event.
			IGameEvent *pEvent = NULL;
			if(m_hSabotaging->Classify() == CLASS_SENTRYGUN)
				pEvent = gameeventmanager->CreateEvent("sentry_sabotaged");
			else if(m_hSabotaging->Classify() == CLASS_DISPENSER)
				pEvent = gameeventmanager->CreateEvent("dispenser_sabotaged");
			if(pEvent)
			{
				CFFPlayer *pOwner = NULL;
				if( m_hSabotaging )
					pOwner = ToFFPlayer( m_hSabotaging->m_hOwner.Get() );
					
				if( pOwner )
				{					
					pEvent->SetInt("userid", pOwner->GetUserID());
					pEvent->SetInt("saboteur", GetUserID());

					// WHY? WHY? WHY?
					// Why set up the event and never send it? 
					// DrEvil - I think you for the
					// "gameeventmanager->FireEvent( pEvent );" 
					// line. <3
				}				
			}

			m_hSabotaging->Sabotage(this);
			m_hSabotaging = NULL;
		}
	}
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
	m_flArmorType = m_flBaseArmorType;

	iAmount = min( iAmount, m_iMaxArmor - m_iArmor );
	if (iAmount < 1)
		return 0;

	m_iArmor += iAmount;

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
void CFFPlayer::ReduceArmorClass()
{
	if (m_flBaseArmorType == 0.8f)
		m_flArmorType = 0.5f;
	else if (m_flBaseArmorType == 0.5f)
		m_flArmorType = 0.3f;
}

//-----------------------------------------------------------------------------
// Purpose: Find all sentry guns that have been sabotaged by this player and 
//			turn them on the enemy.
//-----------------------------------------------------------------------------
void CFFPlayer::Command_SabotageSentry()
{
	CFFSentryGun *pSentry = NULL; 

	while ((pSentry = (CFFSentryGun *) gEntList.FindEntityByClassname(pSentry, "FF_SentryGun")) != NULL) 
	{
		if (pSentry->IsSabotaged() && pSentry->m_hSaboteur == this) 
			pSentry->MaliciousSabotage(this);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Find all dispensers that have been sabotaged by this player and 
//			detonate them
//-----------------------------------------------------------------------------
void CFFPlayer::Command_SabotageDispenser()
{
	CFFDispenser *pDispenser = NULL; 

	while ((pDispenser = (CFFDispenser *) gEntList.FindEntityByClassname(pDispenser, "FF_Dispenser")) != NULL) 
	{
		if (pDispenser->IsSabotaged() && pDispenser->m_hSaboteur == this) 
			pDispenser->MaliciousSabotage(this);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Release control of any sabotaged thing
//-----------------------------------------------------------------------------
void CFFPlayer::SpySabotageRelease()
{
	CFFDispenser *pDispenser = NULL; 

	// Detonate any pipes belonging to us
	while ((pDispenser = (CFFDispenser *) gEntList.FindEntityByClassname(pDispenser, "FF_Dispenser")) != NULL) 
	{
		if (pDispenser->m_hSaboteur == this) 
		{
			pDispenser->m_hSaboteur = NULL;
			pDispenser->m_flSabotageTime = 0;
		}
	}

	CFFSentryGun *pSentry = NULL; 

	// Detonate any pipes belonging to us
	while ((pSentry = (CFFSentryGun *) gEntList.FindEntityByClassname(pSentry, "FF_SentryGun")) != NULL) 
	{
		if (pSentry->m_hSaboteur == this) 
		{
			pSentry->m_hSaboteur = NULL;
			pSentry->m_flSabotageTime = 0;
		}
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
		return GetLegacyAbsOrigin() + (Vector(0, 0, 28) * random->RandomFloat(0.5f, 1.1f));
	}
	else
	{
		return GetLegacyAbsOrigin(); //return EyePosition(); 
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
		case LUA_EF_CALTROP: pszEffectSuffix = "caltrop"; break;
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
		case LUA_EF_CALTROP:
			entsys.SetVar( szLuaDuration, flDuration );
			entsys.SetVar( szLuaIconDuration, flIconDuration );
			break;
	}

	// Set the vars in lua - speed
	switch( iEffect )
	{
		case LUA_EF_LEGSHOT:
		case LUA_EF_TRANQ:
		case LUA_EF_CALTROP:
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
			entsys.SetVar( szLuaSpeed, flSpeed );
			break;
	}

	CFFLuaSC hContext( 2, this, pEffector );
	if( entsys.RunPredicates_LUA( NULL, &hContext, szLuaFunc ) )
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
				case LUA_EF_CALTROP:
					flDuration = entsys.GetFloat( szLuaDuration );
					flIconDuration = entsys.GetFloat( szLuaIconDuration );
					break;
			}

			// Get the vars in lua - speed
			switch( iEffect )
			{
				case LUA_EF_LEGSHOT:
				case LUA_EF_TRANQ:
				case LUA_EF_CALTROP:
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
					flSpeed = entsys.GetFloat( szLuaSpeed );
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
void CFFPlayer::SetFlameSpritesLifetime(float flLifeTime)
{
	CEntityFlame *pFlame = dynamic_cast <CEntityFlame *> (GetEffectEntity());

	// If there is no flame then only create one if necessary
	if (!pFlame)
	{
		if (flLifeTime <= 0.0f)
			return;

		pFlame = CEntityFlame::Create(this);
		SetEffectEntity(pFlame);
	}

	Assert(pFlame);

	if (!pFlame)
		return;

	// If we're reducing the lifecycle in order to remove the flame then check
	// that the flame is still going.
	// If it's not already going then we should return without doing anything otherwise
	// we can end up in a loop when the flame keeps notifying the player that it has
	// run out and this function is called.
	if (flLifeTime <= 0.0f && pFlame->m_flLifetime > gpGlobals->curtime)
	{
		pFlame->SetLifetime(-1.0f);
	}
	else if (flLifeTime > 0.0f)
	{
		pFlame->SetLifetime(flLifeTime);
	}
	else
		return;

	// Take effect immediately
	pFlame->FlameThink();
}