//========= Copyright � 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose:		Player for HL1.
//
// $NoKeywords: $
//=============================================================================

#include "cbase.h"
#include "ff_player.h"
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

#include "client.h"

#include <vector>
#include <algorithm>

// Re-adding this after STL to fix linux server issues
#undef MINMAX_H
#include "minmax.h"		

#include "ff_entity_system.h"	// Entity system
#include "ff_luaobject_wrapper.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"

#include "ff_statslog.h"

#include "gib.h"

#include "omnibot_interface.h"

#include "te_effect_dispatch.h"

extern int gEvilImpulse101;
#define FF_PLAYER_MODEL "models/player/demoman/demoman.mdl"

// Decapitation code
// These need to be mirrored in c_ff_player.cpp
#define DECAP_HEAD			( 1 << 0 )
#define DECAP_LEFT_ARM		( 1 << 1 )
#define DECAP_RIGHT_ARM		( 1 << 2 )
#define DECAP_LEFT_LEG		( 1 << 3 )
#define DECAP_RIGHT_LEG		( 1 << 4 )

// Oh no a gobal.
int g_iLimbs[CLASS_CIVILIAN + 1][5] = { { 0 } };

// grenade information
ConVar gren_timer("ffdev_gren_timer","4.0",0,"Timer length for all grenades.");
ConVar gren_throw_delay("ffdev_throw_delay","1.0",0,"Delay before primed grenades can be thrown.");
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

static ConVar jerkmulti( "ffdev_concuss_jerkmulti", "0.1", 0, "Amount to jerk view on conc" );

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
#ifdef _DEBUG
		if ( engine->Cmd_Argc() > 1	)
#else
		if ( engine->Cmd_Argc() > 1 && !g_pGameRules->IsMultiplayer() )
#endif
		{
			// Find the matching netname
			for ( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex(i) );
				if ( pPlayer )
				{
					if ( Q_strstr( pPlayer->GetPlayerName(), engine->Cmd_Argv(1)) )
					{
						// Bug #0000578: Suiciding using /kill doesn't cause a respawn delay
						if( pPlayer->IsAlive() )
							pPlayer->SetRespawnDelay( 5.0f );

						// Bug #0000700: people with infection should give medic kill if they suicide
						if( pPlayer->IsInfected() && pPlayer->GetInfector() )
							pPlayer->SetSpecialInfectedDeath();

                        ClientKill( pPlayer->edict() );
					}
				}
			}
		}
		else
		{
			// Bug #0000578: Suiciding using /kill doesn't cause a respawn delay
			if( pPlayer->IsAlive() )
				pPlayer->SetRespawnDelay( 5.0f );

			// Bug #0000700: people with infection should give medic kill if they suicide
			if( pPlayer->IsInfected() && pPlayer->GetInfector() )
				pPlayer->SetSpecialInfectedDeath();

			ClientKill( pPlayer->edict() );
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
	
	g_TEPlayerAnimEvent.m_hPlayer = pPlayer;
	g_TEPlayerAnimEvent.m_iEvent = event;
	g_TEPlayerAnimEvent.Create( filter, 0 );
}

// -------------------------------------------------------------------------------- //
// Tables.
// -------------------------------------------------------------------------------- //

// attach the info_ff_teamspawn entity to something too.. might as well put it here
LINK_ENTITY_TO_CLASS( info_ff_teamspawn , CPointEntity);
LINK_ENTITY_TO_CLASS( player, CFFPlayer );
PRECACHE_REGISTER(player);

BEGIN_SEND_TABLE_NOBASE( CFFPlayer, DT_FFLocalPlayerExclusive )
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

	SendPropFloat( SENDINFO( m_flConcTime ) ),
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

	// Data that only gets sent to the local player.
	SendPropDataTable( "fflocaldata", 0, &REFERENCE_SEND_TABLE(DT_FFLocalPlayerExclusive), SendProxy_SendLocalDataTable ),

	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 0), 11 ),
	SendPropAngle( SENDINFO_VECTORELEM(m_angEyeAngles, 1), 11 ),
	SendPropEHandle( SENDINFO( m_hRagdoll ) ),

	SendPropInt( SENDINFO( m_iClassStatus ) ),
	SendPropInt( SENDINFO( m_iSpyDisguise ) ), 

	SendPropInt(SENDINFO(m_iSpawnInterpCounter), 4),
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
	m_iPrimary = 0;
	m_iSecondary = 0;
	m_bWantToThrowGrenade = false;

	// Status Effects
	m_flNextBurnTick = 0.0;
	m_iBurnTicks = 0;
	m_flBurningDamage = 0.0;

	m_bDisguisable = true;

	for (int i=0; i<NUM_SPEED_EFFECTS; i++)
	{
		m_vSpeedEffects[i].active = false;
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

	//teleoprt physics shadow too
	Vector newPos = GetAbsOrigin();
	QAngle newAng = GetAbsAngles();

	Teleport( &newPos, &newAng, &vec3_origin );
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

	// Update our list of tagged players that the client
	// should be "seeing" if it's time to do another update
	if( ( m_flLastRadioTagUpdate + radiotag_duration.GetFloat( ) ) < gpGlobals->curtime )
	{
		FindRadioTaggedPlayers();
	}

	// Riding a vehicle?
	if( IsInAVehicle( ) )	
	{
		// make sure we update the client, check for timed damage and update suit even if we are in a vehicle
		UpdateClientData( );		
		CheckTimeBasedDamage( );

		// Allow the suit to recharge when in the vehicle.
		CheckSuitUpdate( );
		
		WaterMove( );	
		return;
	}

	// update the grenade status if needed
	if( IsGrenadePrimed( ) )
		GrenadeThink( );

	// Bug #0000459: building on ledge locks you into place.
	if( m_bBuilding )
	{
		// Need to stop building because player somehow came off the ground
		if( !FBitSet( GetFlags(), FL_ONGROUND ) )
		{
			Warning( "[Buildable] Player building and came off the ground! Need to cancel build.\n" );

			// Send back through build process *should* cancel it correctly
			m_iWantBuild = m_iCurBuild;
			PreBuildGenericThink();
		}

		// Our origin has changed while building! no!!!!!!!!!!!!!!!!!!!!!!
		if( m_vecBuildOrigin != GetAbsOrigin() )
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

	BaseClass::Precache();
}

extern CBaseEntity *g_pLastSpawn; // this is defined somewhere.. i'm using it :)
CBaseEntity *CFFPlayer::EntSelectSpawnPoint()
{
	CBaseEntity *pSpot, *pGibSpot;
	edict_t		*player;

	player = edict();

	pSpot = g_pLastSpawn;
	// Randomize the start spot
	// NOTE: given a larger range from the default SDK function so that players won't always
	// spawn at the "first" spawn point for their team if the spawns are put in a "bad" order
	for ( int i = random->RandomInt(15,25); i > 0; i-- )
	{
		pSpot = gEntList.FindEntityByClassname( pSpot, "info_ff_teamspawn" );
		if ( !pSpot )  // skip over the null point
			pSpot = gEntList.FindEntityByClassname( pSpot, "info_ff_teamspawn" );
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
				luabind::adl::object hSpawnSpot;
				if( entsys.RunPredicates_LUA( pSpot, this, "validspawn", hSpawnSpot ) )
				{
					// pSpot:validspawn() exists, check the value returned from it
					if( !GetBool( hSpawnSpot ) )
					{
						//DevMsg("[entsys] Skipping spawn for player %s at %s because it fails the check\n", GetPlayerName(), STRING( pSpot->GetEntityName() ) );
						pSpot = gEntList.FindEntityByClassname( pSpot, "info_ff_teamspawn" );
						continue;
					}					
				}

				//DevMsg("[entsys] Found valid spawn for %s at %s\n", GetPlayerName(), STRING( pSpot->GetEntityName() ) );
			}
			pGibSpot = pSpot;

			// check if pSpot is valid
			if ( g_pGameRules->IsSpawnPointValid( pSpot, this ) )
			{
				if ( pSpot->GetLocalOrigin() == vec3_origin )
				{
					pSpot = gEntList.FindEntityByClassname( pSpot, "info_ff_teamspawn" );
					continue;
				}

				// if so, go to pSpot
				goto ReturnSpot;
			}
		}
		// increment pSpot
		pSpot = gEntList.FindEntityByClassname( pSpot, "info_ff_teamspawn" );
	} while ( pSpot != pFirstSpot ); // loop if we're not back to the start

	// we haven't found a place to spawn yet,  so kill any guy at the first spawn point and spawn there
	pSpot = pGibSpot;
	if ( pSpot )
	{
		CBaseEntity *pList[128];
		int count = UTIL_EntitiesInBox( pList, 128, pSpot->GetAbsOrigin()-Vector( 24, 24, 48 ), pSpot->GetAbsOrigin()+Vector( 24, 24, 48 ), FL_CLIENT|FL_NPC );
		if ( count )
			//Iterate through the list and check the results
			for ( int i = 0; i < count; i++ )
			{
				CBaseEntity *ent = pList[i];
				if ( ent && ent->IsPlayer() && !(ent->edict() == player) )
					ent->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );

			}

		/*
		CBaseEntity *ent = NULL;
		for ( CEntitySphereQuery sphere( pSpot->GetAbsOrigin(), 32 ); (ent = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity() )
		{
			// if ent is a client, kill em (unless they are ourselves)
			if ( ent->IsPlayer() && !(ent->edict() == player) )
				ent->TakeDamage( CTakeDamageInfo( GetContainingEntity(INDEXENT(0)), GetContainingEntity(INDEXENT(0)), 300, DMG_GENERIC ) );
		}
		*/

		goto ReturnSpot;
	}

	// as a last resort, try to find an info_player_start
	//DevMsg("Spawning at info_player_start\n");
	pSpot = gEntList.FindEntityByClassname( NULL, "info_player_start");
	if ( pSpot ) 
		goto ReturnSpot;
	pSpot = gEntList.FindEntityByClassname( NULL, "info_player_deathmatch");
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
	WRITE_BYTE(FF_NUMICONS);
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
		StartObserverMode(OBS_MODE_ROAMING);
		AddEffects(EF_NODRAW);

		BaseClass::Spawn();
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
	entsys.RunPredicates_LUA( NULL, this, "player_spawn" );

	for (int i=0; i<NUM_SPEED_EFFECTS; i++)
		m_vSpeedEffects[i].active = false;

	// equip the HEV suit
	EquipSuit();

	// Set on ground
	AddFlag(FL_ONGROUND);

	// Increment the spawn counter
	m_iSpawnInterpCounter = (m_iSpawnInterpCounter + 1) % 8;
}

// Mirv: Moved all this out of spawn into here
void CFFPlayer::SetupClassVariables()
{
	// Reset Engineer stuff
	m_pBuildLastWeapon = NULL;

	m_hSaveMe = NULL;

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

	// Select the correct weapon
	int iSpawnSlot = atoi(engine->GetClientConVarValue(engine->IndexOfEdict(edict()), "cl_spawnslot"));

	if (iSpawnSlot > 0 && iSpawnSlot <= MAX_WEAPON_SLOTS)
	{
		CBaseCombatWeapon *pWeapon = GetWeaponForSlot(iSpawnSlot - 1);
		Weapon_Switch(pWeapon);
	}

	// Load class configs
	engine->ClientCommand(edict(), "exec %.10s.cfg", pPlayerClassInfo.m_szClassName);
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

	//DevMsg("CFFPlayer::InitialSpawn");
}

void CFFPlayer::SpyFeign( void )
{
	Vector relativeVel = GetAbsVelocity();

	if (GetGroundEntity())
		relativeVel.z = 0;

	// A yell of pain if we're feigning [need to check feign will be allowed too]
	if( !m_fFeigned && relativeVel.LengthSqr() <= 25.0f )
		EmitSound( "Player.Death" );

	SpySilentFeign();
}

void CFFPlayer::SpySilentFeign( void )
{
	// Already feigned
	if( m_fFeigned )
	{
		// Yeah we're not feigned anymore bud
		m_fFeigned = false;

		// Remove the ragdoll
		CFFRagdoll *pRagdoll = dynamic_cast< CFFRagdoll* >( m_hRagdoll.Get() );
		pRagdoll->SetThink( &CBaseEntity::SUB_Remove );
		pRagdoll->SetNextThink( gpGlobals->curtime );

		// Invisible and stuff
		RemoveEffects( EF_NODRAW );
		RemoveFlag( FL_FROZEN );

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
	else
	{
		Vector relativeVel = GetAbsVelocity();

		if (GetGroundEntity())
			relativeVel.z = 0;

		// Need to be stationary to feign
		if( relativeVel.LengthSqr() > 25.0f )
		{
			ClientPrint( this, HUD_PRINTTALK, "#FF_SPY_CANTFEIGNSPEED" );
			
			// Notify the bot: convert this to an event?
			if(IsBot())
			{
				Omnibot::Notify_CantFeign(this);
			}
			return;
		}

		m_fFeigned = true;

		// Create our ragdoll using this function (we could just c&p it and modify it i guess)
		CreateRagdollEntity();

		// Quick get that ragdoll and change a few things
		CFFRagdoll *pRagdoll = dynamic_cast< CFFRagdoll* >( m_hRagdoll.Get() );
		pRagdoll->m_vecRagdollVelocity = Vector( 0, 0, 0 );
		pRagdoll->SetThink( NULL );

		// Invisible and stuff
		AddEffects( EF_NODRAW );
		AddFlag( FL_FROZEN );

		// Holster our current weapon
		if (GetActiveWeapon())
			GetActiveWeapon()->Holster(NULL);

		// Find any items that we are in control of and let them know we feigned
		CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( NULL, "info_ff_script" );
		while( pEnt != NULL )
		{
			// Tell the ent that it died
			if (pEnt->GetOwnerEntity() == this)
				entsys.RunPredicates_LUA( pEnt, this, "ownerfeign" );

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
	//Possible fix: 0000807: When using cl_autoreload 1 players randomly cannot respawn
	//engine->ClientCommand( edict(), "-reload" ); //VOOGRU: Let's do this on the client instead.

	if( m_hSaveMe )
	{
		// This will kill it
		m_hSaveMe->SetLifeTime( gpGlobals->curtime );
	}

	// Log the death to the stats engine
	g_StatsLog.AddToCount(this, STAT_DEATHS);

	// TODO: Take SGs into account here?
	CFFPlayer *pKiller = dynamic_cast<CFFPlayer *> (info.GetAttacker());
	
	// Log the correct stat for the killer
	if (pKiller)
	{
		if (g_pGameRules->PlayerRelationship(this, pKiller) == GR_TEAMMATE)
			g_StatsLog.AddToCount(pKiller, STAT_TEAMKILLS);
		else
			g_StatsLog.AddToCount(pKiller, STAT_KILLS);
	}

	// Drop any grenades
	if (m_iGrenadeState != FF_GREN_NONE)
	{
		ThrowGrenade(gren_timer.GetFloat() - (gpGlobals->curtime - m_flServerPrimeTime), 0.0f);
		m_iGrenadeState = FF_GREN_NONE;
		m_flServerPrimeTime = 0;
	}

	ClearSpeedEffects();
	RemoveFlag(FL_FROZEN);

	// reset their status effects
	m_flNextBurnTick = 0.0;
	m_iBurnTicks = 0;
	m_flBurningDamage = 0.0;
	for (int i=0; i<NUM_SPEED_EFFECTS; i++)
	{
		m_vSpeedEffects[i].active = false;
	}
	m_fLastHealTick = 0.0f;
	m_fLastInfectedTick = 0.0f;

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

	// run the player_died event in lua
	//DevMsg("Running player_killed\n");
	entsys.SetVar("killer", ENTINDEX(info.GetAttacker()));
	entsys.RunPredicates_LUA( NULL, this, "player_killed" );

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

	// Get rid of fire
	Extinguish();

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

	BaseClass::Event_Killed( info );

	CreateRagdollEntity(&info);
}

void CFFPlayer::CreateRagdollEntity(const CTakeDamageInfo *info)
{
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

	// Set class in stats engine
	g_StatsLog.SetClass(entindex(), m_iNextClass);

	// So the client can keep track
	SetClassForClient(m_iNextClass);

	return GetClassSlot();
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

	KillAndRemoveItems();
	ChangeTeam(iTeam);
	
	// Make sure they don't think they're meant to be spawning as a new class
	m_iNextClass = 0;

	// Cancel map guides
    m_hNextMapGuide = NULL;

	// A forced spawn situation
	m_flNextSpawnDelay = -1;

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
	CBaseEntity *pEnt = gEntList.FindEntityByOwner(NULL, this);

	while (pEnt != NULL)
	{
		if (dynamic_cast<CFFProjectileBase *>( pEnt ) != NULL)
			UTIL_Remove(pEnt);

		pEnt = gEntList.FindEntityByOwner(pEnt, this);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove backpacks
//-----------------------------------------------------------------------------
void CFFPlayer::RemoveBackpacks( void )
{
	CBaseEntity *pEntity = gEntList.FindEntityByClassT( NULL, CLASS_BACKPACK );

	while( pEntity )
	{
		if( ToFFPlayer( pEntity->GetOwnerEntity() ) == this )
			UTIL_Remove( pEntity );

		pEntity = gEntList.FindEntityByClassT( pEntity, CLASS_BACKPACK );
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

	// TODO: Other classes need to check
	// if (m_bBuilding == True)
	// ie. grenades, weapons, etc

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
	if( iMaxClients < 1 )
		return;

	// My origin
	Vector vecOrigin = GetAbsOrigin();

	// Loop through doing stuff on each player
	for( int i = 0; i < iMaxClients; i++ )
	{
		CBasePlayer *pBasePlayer = UTIL_PlayerByIndex( i );

		// Skip if NULL
		if( !pBasePlayer )
			continue;		

		// Skip if not a player
		if( !pBasePlayer->IsPlayer() )
			continue;

		// Skip if spec
		if( pBasePlayer->IsObserver() )
			continue;

		CFFPlayer *pPlayer = ToFFPlayer( pBasePlayer );

		// Skip if us
		if( pPlayer == this )
			continue;

		// Skip if not tagged
		if( !pPlayer->m_bRadioTagged )
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
		Vector vecPlayerOrigin = pPlayer->GetAbsOrigin();

		// Skip if they're out of range
		if( vecOrigin.DistTo( vecPlayerOrigin ) > radiotag_distance.GetInt() )
			continue;

		// We're left w/ a player who's within range
		// Add player to a list and send off to client

		// Create a single object
		ESP_Shared_s hObject;
		hObject.m_iClass = pPlayer->GetClassSlot();
		hObject.m_iTeam = pPlayer->GetTeamNumber() - 1;
		hObject.m_bDucked = ( pPlayer->GetFlags() & FL_DUCKING ) ? true : false;
		hObject.m_vecOrigin = vecPlayerOrigin;

		// Add object to radio tagged array
		m_hRadioTaggedList.AddToTail( hObject );
	}

	if( m_hRadioTaggedList.Count() )
	{
		// Only send this message to the local player	
		CSingleUserRecipientFilter user( this );
		user.MakeReliable();

		// Start the message block
		UserMessageBegin( user, "RadioTagUpdate" );

			// send block	
			// - team (int 1-4) [to color the silhouettes elitely] adjusted value
			// - class (int)
			// - origin (float[3])
			// team = 99 terminates

			for( int i = 0; i < m_hRadioTaggedList.Count(); i++ )
			{
				int iInfo = m_hRadioTaggedList[ i ].m_iTeam;
				iInfo += m_hRadioTaggedList[ i ].m_iClass << 4;

				WRITE_WORD( iInfo );
				WRITE_BYTE( m_hRadioTaggedList[ i ].m_bDucked ? 1 : 0 );
				WRITE_VEC3COORD( m_hRadioTaggedList[ i ].m_vecOrigin );
			}
			
			// We're done sending the HUD message
			WRITE_WORD( 0 );

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

	char szBuffer[128];
	Q_snprintf(szBuffer, 127, "[What Team] m_iSpyDisguise: %i, Disguised? %s, Team: %i, Class: %i, My Team: %i\n", m_iSpyDisguise, IsDisguised() ? "yes" : "no", GetDisguisedTeam(), GetDisguisedClass(), GetTeamNumber());
	ClientPrint(UTIL_GetCommandClient(), HUD_PRINTCONSOLE, szBuffer);
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
		Q_strcat( m_szCustomDispenserText, " " );

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
		if( GetAmmoCount( "AMMO_CELLS" ) >= radar_num_cells.GetInt( ) )
		{
			// Bug #0000531: Everyone hears radar
			CPASAttenuationFilter sndFilter;
			sndFilter.RemoveAllRecipients();
			sndFilter.AddRecipient( this );
			EmitSound( sndFilter, entindex(), "radar.single_shot");

			// Remove ammo
			RemoveAmmo( radar_num_cells.GetInt(), "AMMO_CELLS" );

			// Only send this message to the local player	
			CSingleUserRecipientFilter user( this );
			user.MakeReliable();

			// Start the message block
			UserMessageBegin( user, "RadarUpdate" );

			// Send our radar/esp to the client
			Vector vecOrigin = GetAbsOrigin();

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
					if( pPlayer->GetTeamNumber() < TEAM_BLUE )
						continue;

					Vector vecPlayerOrigin = pPlayer->GetAbsOrigin();
					float flDist = vecOrigin.DistTo( vecPlayerOrigin );

					if( flDist <= ( float )radar_radius_distance.GetInt() )
					{
						int iInfo = pPlayer->GetTeamNumber() - 1;
						iInfo += pPlayer->GetClassSlot() << 4;

						if( ( g_pGameRules->PlayerRelationship( this, pPlayer ) == GR_NOTTEAMMATE ) &&
							( pPlayer->IsDisguised() ) )
						{
							iInfo = pPlayer->GetDisguisedTeam() - 1;
							iInfo += pPlayer->GetDisguisedClass() << 4;
						}

						WRITE_WORD( iInfo );
						WRITE_BYTE( ( ( pPlayer->GetFlags() & FL_DUCKING ) ? 1 : 0 ) );
						WRITE_VEC3COORD( vecPlayerOrigin );				// Origin in 3d space

						// Omni-bot: Notify the bot he has detected someone.
						if(IsBot())
						{
							Omnibot::Notify_RadarDetectedEnemy(this, pPlayer->edict());							
						}
					}
				}
			}

			// We're done sending the HUD message
			WRITE_WORD( 0 );

			// End the message block
			MessageEnd();

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

		float flRaiseVal = 0.0f, flBuildDist = 0.0f;

		// Set us up the bomb
		switch( m_iWantBuild )
		{
			case FF_BUILD_DISPENSER: flRaiseVal = FF_BUILD_DISP_RAISE_VAL; flBuildDist = FF_BUILD_DISP_BUILD_DIST; break;
			case FF_BUILD_SENTRYGUN: flRaiseVal = FF_BUILD_SG_RAISE_VAL; flBuildDist = FF_BUILD_SG_BUILD_DIST; break;
			case FF_BUILD_DETPACK: flRaiseVal = FF_BUILD_DET_RAISE_VAL; flBuildDist = FF_BUILD_DET_BUILD_DIST; break;
		}
		
		// Drop the detpack down a little if we're ducking so it's not hovering above us
		if( ( GetFlags() & FL_DUCKING ) && ( m_iWantBuild == FF_BUILD_DETPACK ) )
			flRaiseVal /= 2.0f;

		// Our neat buildable info container
		CFFBuildableInfo hBuildInfo( this, m_iWantBuild, flBuildDist, flRaiseVal );

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
					CFFDispenser *pDispenser = CFFDispenser::Create( hBuildInfo.GetBuildGroundOrigin(), hBuildInfo.GetBuildGroundAngles(), this );
					
					// Set custom text
					pDispenser->SetText( m_szCustomDispenserText );					

					// Mirv: Store future ground location + orientation
					pDispenser->SetGroundOrigin( hBuildInfo.GetBuildGroundOrigin() );
					pDispenser->SetGroundAngles( hBuildInfo.GetBuildGroundAngles() );

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
					CFFSentryGun *pSentryGun = CFFSentryGun::Create( hBuildInfo.GetBuildGroundOrigin(), hBuildInfo.GetBuildGroundAngles(), this );
				
					// Mirv: Store future ground location + orientation
					pSentryGun->SetGroundOrigin( hBuildInfo.GetBuildGroundOrigin() );
					pSentryGun->SetGroundAngles( hBuildInfo.GetBuildGroundAngles() );

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
					CFFDetpack *pDetpack = CFFDetpack::Create( hBuildInfo.GetBuildGroundOrigin(), hBuildInfo.GetBuildGroundAngles(), this );

					// Set the fuse time
					pDetpack->m_iFuseTime = m_iDetpackTime;

					// Mirv: Store future ground location + orientation
					pDetpack->SetGroundOrigin( hBuildInfo.GetBuildGroundOrigin() );
					pDetpack->SetGroundAngles( hBuildInfo.GetBuildGroundAngles() );

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

			// Cancel the build
			switch( m_iCurBuild )
			{
				case FF_BUILD_DISPENSER: ( ( CFFDispenser * )( m_hDispenser.Get() ) )->Cancel(); break;
				case FF_BUILD_SENTRYGUN: ( ( CFFSentryGun * )( m_hSentryGun.Get() ) )->Cancel(); break;
				case FF_BUILD_DETPACK:   ( ( CFFDetpack * )( m_hDetpack.Get() ) )->Cancel(); break;
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
	entsys.RunPredicates_LUA(NULL, this, "flaginfo");
}

/**
	DropItems
*/
void CFFPlayer::Command_DropItems( void )
{	
	//entsys.RunPredicates(NULL, this, "dropitems");
	CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( NULL, CLASS_INFOSCRIPT );

	while( pEnt != NULL )
	{
		if( pEnt->GetOwnerEntity() == ( CBaseEntity * )this )
		{
			// If the function exists, try and drop
			entsys.RunPredicates_LUA( pEnt, this, "dropitemcmd" );
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
}

void CFFPlayer::StatusEffectsThink( void )
{
	// If we jump in water up to waist level, extinguish ourselves
	if (m_iBurnTicks && GetWaterLevel() >= WL_Waist)
		Extinguish();

	// if we're on fire, then do something about it
	if (m_iBurnTicks && (m_flNextBurnTick < gpGlobals->curtime))
	{
		// EmitSound( "General.BurningFlesh" );	// |-- Mirv: Dunno it just sounds odd using the emp sound!

		float damage = m_flBurningDamage / (float)m_iBurnTicks;

		// do damage
		CTakeDamageInfo info(m_hIgniter,m_hIgniter,damage,DMG_BURN);
		TakeDamage(info);

		// remove a tick
		m_iBurnTicks--;
		m_flBurningDamage -= damage;

		// schedule the next tick
		m_flNextBurnTick = gpGlobals->curtime + 1.25f;
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
	if( m_bInfected && ( gpGlobals->curtime > ( m_fLastInfectedTick + ffdev_infect_freq.GetFloat() ) ) )
	{
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

		// If we're still infected, cause damage
		if (m_bInfected && IsAlive())	// |-- Mirv: Bug #0000461: Infect sound plays eventhough you are dead
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
				WRITE_BYTE(FF_ICON_INFECTED);
				WRITE_FLOAT(2.0f);
			MessageEnd(); 

#if 1
			// Bug #0000504: No infection visible effect
			CEffectData data;
			data.m_vOrigin = GetLegacyAbsOrigin();
			data.m_flScale = 1.0f;
			//DispatchEffect( "InfectCloud", data );
			DispatchEffect( "bloodimpact", data );

			CEffectData data2;
			data2.m_vOrigin = EyePosition();
			data2.m_flScale = 1.0f;
			//DispatchEffect( "InfectCloud", data );
			DispatchEffect( "bloodimpact", data2 );
#endif

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
		if (m_vSpeedEffects[i].active && m_vSpeedEffects[i].endTime < gpGlobals->curtime )
		{
			m_vSpeedEffects[i].active = false;
			recalcspeed = true;
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
	// These are hardcoded values in base.lua...
	if( ( iEffect >= 0 ) && ( iEffect <= 3 ) )
	{
		// REGULAR EFFECTS

		// These are in lua as:
		// EF_ONFIRE = 0
		// EF_CONCUSS = 1
		// EF_GAS = 2
		// EF_INFECT = 3
		switch( iEffect )
		{
			// This is really sloppy

			case 0:
			{
				// This will do the icon too and set us
				// on fire. Using "this" just so it's not
				// NULL.
				ApplyBurning( this, 1.0f, flIconDuration );
			}				
			break;

			case 1:
			{
				// Set concuss'd
				Concuss( flEffectDuration );

				// Send the concussion icon
				CSingleUserRecipientFilter user( this );
				user.MakeReliable();

				UserMessageBegin( user, "StatusIconUpdate" );
					WRITE_BYTE( FF_ICON_CONCUSSION );
					WRITE_FLOAT( flIconDuration );
				MessageEnd();
			}
			break;

			case 2:
			{
				// EH, need to make gas like fire or something
				// where you've got gasticks counting down and
				// taking damage every x seconds and what not
				// TODO: Make the player take gas damage
			}
			break;

			case 3:
			{
				// Passing "this" just so it's got something
				// besides NULL for the time being
				Infect( this );
			}
			break;
		}
	}
	else if( ( iEffect >= 4 ) && ( iEffect <= 16 ) )
	{
		// SPEED EFFECTS

		// These are in lua as:
		// EF_LEGSHOT = 4
		// EF_TRANQ = 5
		// EF_CALTROP = 6 
 		// EF_SPEED_LUA1 = 7
		// EF_SPEED_LUA2 = 8
		// EF_SPEED_LUA3 = 9
		// EF_SPEED_LUA4 = 10
		// EF_SPEED_LUA5 = 11
		// EF_SPEED_LUA6 = 12
		// EF_SPEED_LUA7 = 13
		// EF_SPEED_LUA8 = 14
		// EF_SPEED_LUA9 = 15
		// EF_SPEED_LUA10 = 16

		bool bUseIcon = false;

		byte bIcon = 0x0;
		int mod = SEM_ACCUMULATIVE;

		SpeedEffectType sEffect;
		switch( iEffect )
		{
			case 4: sEffect = SE_LEGSHOT; mod = SEM_BOOLEAN; bUseIcon = true; bIcon = FF_ICON_LEGSHOT; break;
			case 5: sEffect = SE_TRANQ; mod = SEM_BOOLEAN | SEM_HEALABLE; bUseIcon = true; bIcon = FF_ICON_TRANQ; break;
			case 6: sEffect = SE_CALTROP; mod |= SEM_HEALABLE; bUseIcon = true; bIcon = FF_ICON_CALTROP; break;
			case 7: sEffect = SE_LUA1; break;
			case 8: sEffect = SE_LUA2; break;
			case 9: sEffect = SE_LUA3; break;
			case 10: sEffect = SE_LUA4; break;
			case 11: sEffect = SE_LUA5; break;
			case 12: sEffect = SE_LUA6; break;
			case 13: sEffect = SE_LUA7; break;
			case 14: sEffect = SE_LUA8; break;
			case 15: sEffect = SE_LUA9; break;
			case 16: sEffect = SE_LUA10; break;
			default: return; break;
		}

		if( bUseIcon )
		{
			CSingleUserRecipientFilter user( this );
			user.MakeReliable();

			UserMessageBegin( user, "StatusIconUpdate" );
				WRITE_BYTE( bIcon );
				WRITE_FLOAT( flIconDuration );
			MessageEnd();
		}

		AddSpeedEffect( sEffect, flEffectDuration, flSpeed, mod );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Let's lua see if an effect is active
//-----------------------------------------------------------------------------
bool CFFPlayer::LuaIsEffectActive( int iEffect )
{
	// These are hardcoded values in base.lua...
	if( ( iEffect >= 0 ) && ( iEffect <= 3 ) )
	{
		// REGULAR EFFECTS

		// These are in lua as:
		// EF_ONFIRE = 0
		// EF_CONCUSS = 1
		// EF_GAS = 2
		// EF_INFECT = 3
		switch( iEffect )
		{
			// This is really sloppy

			case 0: return ( m_iBurnTicks ? true : false ); break;
			case 1: return m_flConcTime > gpGlobals->curtime; break;

			case 2:
			{
				// TODO: something for this...?
			}
			break;

			case 3: return m_bInfected; break;
			default: return false; break;
		}
	}
	else if( ( iEffect >= 4 ) && ( iEffect <= 16 ) )
	{
		// SPEED EFFECTS

		// These are in lua as:
		// EF_LEGSHOT = 4
		// EF_TRANQ = 5
		// EF_CALTROP = 6 
		// EF_SPEED_LUA1 = 7
		// EF_SPEED_LUA2 = 8
		// EF_SPEED_LUA3 = 9
		// EF_SPEED_LUA4 = 10
		// EF_SPEED_LUA5 = 11
		// EF_SPEED_LUA6 = 12
		// EF_SPEED_LUA7 = 13
		// EF_SPEED_LUA8 = 14
		// EF_SPEED_LUA9 = 15
		// EF_SPEED_LUA10 = 16

		SpeedEffectType sEffect;
		switch( iEffect )
		{
			case 4: sEffect = SE_LEGSHOT; break;
			case 5: sEffect = SE_TRANQ; break;
			case 6: sEffect = SE_CALTROP; break;
			case 7: sEffect = SE_LUA1; break;
			case 8: sEffect = SE_LUA2; break;
			case 9: sEffect = SE_LUA3; break;
			case 10: sEffect = SE_LUA4; break;
			case 11: sEffect = SE_LUA5; break;
			case 12: sEffect = SE_LUA6; break;
			case 13: sEffect = SE_LUA7; break;
			case 14: sEffect = SE_LUA8; break;
			case 15: sEffect = SE_LUA9; break;
			case 16: sEffect = SE_LUA10; break;
			default: return false; break;
		}

		return IsSpeedEffectSet( sEffect );
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Let's lua remove an effect
//-----------------------------------------------------------------------------
void CFFPlayer::LuaRemoveEffect( int iEffect )
{
	// These are hardcoded values in base.lua...
	if( ( iEffect >= 0 ) && ( iEffect <= 3 ) )
	{
		// REGULAR EFFECTS

		// These are in lua as:
		// EF_ONFIRE = 0
		// EF_CONCUSS = 1
		// EF_GAS = 2
		// EF_INFECT = 3

		byte bIcon = 0x0;

		switch( iEffect )
		{
			// This is really sloppy

			case 0: 
			{
				Extinguish();
				bIcon = FF_ICON_ONFIRE;
			}
			break;

			case 1: 
			{
				m_flConcTime = gpGlobals->curtime - 1;
				bIcon = FF_ICON_CONCUSSION;
			}
			break;

			case 2:
			{
				// TODO: something for this...?
				bIcon = FF_ICON_GAS;
			}
			break;

			case 3:
			{
				// TODO: Do something
				// Cure() is not really what we want to do
				bIcon = FF_ICON_INFECTED;
			}
			break;

			default: return; break;
		}

		// See if this really removes the icon, heh
		if( bIcon )
		{
			CSingleUserRecipientFilter user( this );
			user.MakeReliable();

			UserMessageBegin( user, "StatusIconUpdate" );
				WRITE_BYTE( bIcon );
				WRITE_FLOAT( gpGlobals->curtime - 1 );
			MessageEnd();
		}
	}
	else if( ( iEffect >= 4 ) && ( iEffect <= 16 ) )
	{
		// SPEED EFFECTS

		// These are in lua as:
		// EF_LEGSHOT = 4
		// EF_TRANQ = 5
		// EF_CALTROP = 6 
		// EF_SPEED_LUA1 = 7
		// EF_SPEED_LUA2 = 8
		// EF_SPEED_LUA3 = 9
		// EF_SPEED_LUA4 = 10
		// EF_SPEED_LUA5 = 11
		// EF_SPEED_LUA6 = 12
		// EF_SPEED_LUA7 = 13
		// EF_SPEED_LUA8 = 14
		// EF_SPEED_LUA9 = 15
		// EF_SPEED_LUA10 = 16

		SpeedEffectType sEffect;
		switch( iEffect )
		{
			case 4: sEffect = SE_LEGSHOT; break;
			case 5: sEffect = SE_TRANQ; break;
			case 6: sEffect = SE_CALTROP; break;
			case 7: sEffect = SE_LUA1; break;
			case 8: sEffect = SE_LUA2; break;
			case 9: sEffect = SE_LUA3; break;
			case 10: sEffect = SE_LUA4; break;
			case 11: sEffect = SE_LUA5; break;
			case 12: sEffect = SE_LUA6; break;
			case 13: sEffect = SE_LUA7; break;
			case 14: sEffect = SE_LUA8; break;
			case 15: sEffect = SE_LUA9; break;
			case 16: sEffect = SE_LUA10; break;
			default: return; break;
		}

		return RemoveSpeedEffect( sEffect );
	}
}

//void CFFPlayer::AddLuaSpeedEffect( int iSpeedEffect, float flDuration, float flSpeed )
//{
//	bool bIcon = false;
//
//	byte icon = 0x0;
//	int mod = SEM_ACCUMULATIVE;
//
//	SpeedEffectType sEffect;
//	switch( iSpeedEffect )
//	{
//		case 0: sEffect = SE_SNIPERRIFLE; mod = SEM_BOOLEAN; break;
//		case 1: sEffect = SE_ASSAULTCANNON; mod = SEM_BOOLEAN; break;
//		case 2: sEffect = SE_LEGSHOT; mod |= SEM_HEALABLE; bIcon = true; break;
//		case 3: sEffect = SE_TRANQ; mod = SEM_BOOLEAN | SEM_HEALABLE; bIcon = true; break;
//		case 4: sEffect = SE_CALTROP; mod |= SEM_HEALABLE; bIcon = true; break;
//		case 5: sEffect = SE_LUA1; break;
//		case 6: sEffect = SE_LUA2; break;
//		case 7: sEffect = SE_LUA3; break;
//		case 8: sEffect = SE_LUA4; break;
//		case 9: sEffect = SE_LUA5; break;
//		case 10: sEffect = SE_LUA6; break;
//		case 11: sEffect = SE_LUA7; break;
//		case 12: sEffect = SE_LUA8; break;
//		case 13: sEffect = SE_LUA9; break;
//		case 14: sEffect = SE_LUA10; break;
//		default: return; break;
//	}
//
//	// Fire an icon for appropriate effects
//	if( bIcon )
//	{
//		CSingleUserRecipientFilter filter( this );
//		filter.MakeReliable();
//		UserMessageBegin( filter, "StatusIconUpdate" );
//			WRITE_BYTE( icon );
//			WRITE_FLOAT( flDuration );
//		MessageEnd();
//	}
//	
//	AddSpeedEffect( sEffect, flDuration, flSpeed, mod );
//}
//
////-----------------------------------------------------------------------------
//// Purpose: So lua can add speed effects
////-----------------------------------------------------------------------------
//bool CFFPlayer::IsLuaSpeedEffectSet( int iSpeedEffect )
//{
//	SpeedEffectType sEffect;
//	switch( iSpeedEffect )
//	{
//		case 0: sEffect = SE_SNIPERRIFLE; break;
//		case 1: sEffect = SE_ASSAULTCANNON; break;
//		case 2: sEffect = SE_LEGSHOT; break;
//		case 3: sEffect = SE_TRANQ; break;
//		case 4: sEffect = SE_CALTROP; break;
//		case 5: sEffect = SE_LUA1; break;
//		case 6: sEffect = SE_LUA2; break;
//		case 7: sEffect = SE_LUA3; break;
//		case 8: sEffect = SE_LUA4; break;
//		case 9: sEffect = SE_LUA5; break;
//		case 10: sEffect = SE_LUA6; break;
//		case 11: sEffect = SE_LUA7; break;
//		case 12: sEffect = SE_LUA8; break;
//		case 13: sEffect = SE_LUA9; break;
//		case 14: sEffect = SE_LUA10; break;
//		default: return; break;
//	}
//
//	return IsSpeedEffectSet( sEffect );
//}
//
////-----------------------------------------------------------------------------
//// Purpose: So lua can add speed effects
////-----------------------------------------------------------------------------
//void CFFPlayer::RemoveLuaSpeedEffect( int iSpeedEffect )
//{
//	SpeedEffectType sEffect;
//	switch( iSpeedEffect )
//	{
//		case 0: sEffect = SE_SNIPERRIFLE; break;
//		case 1: sEffect = SE_ASSAULTCANNON; break;
//		case 2: sEffect = SE_LEGSHOT; break;
//		case 3: sEffect = SE_TRANQ; break;
//		case 4: sEffect = SE_CALTROP; break;
//		case 5: sEffect = SE_LUA1; break;
//		case 6: sEffect = SE_LUA2; break;
//		case 7: sEffect = SE_LUA3; break;
//		case 8: sEffect = SE_LUA4; break;
//		case 9: sEffect = SE_LUA5; break;
//		case 10: sEffect = SE_LUA6; break;
//		case 11: sEffect = SE_LUA7; break;
//		case 12: sEffect = SE_LUA8; break;
//		case 13: sEffect = SE_LUA9; break;
//		case 14: sEffect = SE_LUA10; break;
//		default: return; break;
//	}
//
//	RemoveSpeedEffect( sEffect );
//}

void CFFPlayer::AddSpeedEffect(SpeedEffectType type, float duration, float speed, int mod)
{
	// find an open slot
	int i = 0;

	// Without boolean we default to accumulative, but warn anyway in case we just forgot
	Assert((mod & SEM_BOOLEAN)|(mod & SEM_ACCUMULATIVE));

	if (mod & SEM_BOOLEAN)
		for (int i = 0; i < NUM_SPEED_EFFECTS; i++)
		{
			// we'll overwrite the old one
			if (m_vSpeedEffects[i].type == type)
				break;
		}
	else
		while (m_vSpeedEffects[i].active == true && i != NUM_SPEED_EFFECTS) 
			i++;

	if (i == NUM_SPEED_EFFECTS)
	{
		Warning("ERROR: Too many speed effects. Raise NUM_SPEED_EFFECTS");
		return;
	}

	m_vSpeedEffects[i].active = true;
	m_vSpeedEffects[i].type = type;
	m_vSpeedEffects[i].startTime = gpGlobals->curtime;
	m_vSpeedEffects[i].endTime = gpGlobals->curtime + duration;
	m_vSpeedEffects[i].speed = speed;
	m_vSpeedEffects[i].modifiers = mod;
	
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

void CFFPlayer::RemoveSpeedEffect(SpeedEffectType type)
{
	// Remove speed effects with the certain name
	for (int i=0; i<NUM_SPEED_EFFECTS; i++)
	{
		if (m_vSpeedEffects[i].type == type)
		{
			m_vSpeedEffects[i].active = false;
		}
	}

	RecalculateSpeed();
}

int CFFPlayer::ClearSpeedEffects(int mod)
{
	int i = 0;

	if (mod)
	{
		for (int i = 0; i < NUM_SPEED_EFFECTS; i++)
		{
			if (m_vSpeedEffects[i].modifiers & mod)
			{
				// Keep track of any we've turned off
				if (m_vSpeedEffects[i].active)
					i++;

				m_vSpeedEffects[i].active = false;
			}
		}
	}
	else
	{
		for (int i = 0; i < NUM_SPEED_EFFECTS; i++)
		{
			// Keep track of any we've turned off
			if (m_vSpeedEffects[i].active)
				i++;

			m_vSpeedEffects[i].active = false;
		}
	}

	RecalculateSpeed();

	return i;
}

void CFFPlayer::RecalculateSpeed( void )
{
	//DevMsg("[SpeedEffect] Start\n");
	// start off with the class base speed
	const CFFPlayerClassInfo &pPlayerClassInfo = GetFFClassData();
	float speed = (float)pPlayerClassInfo.m_iSpeed;

	// go apply all speed effects
	for (int i=0; i<NUM_SPEED_EFFECTS; i++)
		if (m_vSpeedEffects[i].active)
			speed *= m_vSpeedEffects[i].speed;

	// set the player speed
	SetMaxSpeed((int)speed);

	//DevMsg("[SpeedEffect] Resetting speed to %d\n", (int)speed);
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
	if( !m_bInfected && !m_bImmune )
	{
		// they aren't infected or immune, so go ahead and infect them
		m_bInfected = true;
		m_fLastInfectedTick = gpGlobals->curtime;
		m_hInfector = pInfector;
		m_iInfectedTeam = pInfector->GetTeamNumber();

		EmitSound( "Player.DrownStart" );	// |-- Mirv: [TODO] Change to something more suitable

		g_StatsLog.AddToCount(pInfector, STAT_INFECTIONS, 1);
	}
}
void CFFPlayer::Cure( CFFPlayer *pCurer )
{
	if( m_bInfected )
	{
		// they are infected, so go ahead and cure them
		m_bInfected = false;
		m_fLastInfectedTick = 0;
		m_hInfector = NULL;
		// Bug# 0000503: "Immunity" is not in the mod
		m_bImmune = true;
		m_flImmuneTime = gpGlobals->curtime + 10.0f;

		// credit the curer with a score
		pCurer->IncrementFragCount( 1 );

		// Log this in the stats
		g_StatsLog.AddToCount(pCurer, STAT_CURES, 1);
	}

	// Bug #0000528: Medics can self-cure being caltropped/tranq'ed
	ClearSpeedEffects( SEM_HEALABLE );
}

void CFFPlayer::ApplyBurning( CFFPlayer *hIgniter, float scale, float flIconDuration )
{
	// send the status icon to be displayed
	//DevMsg("[Grenade Debug] Sending status icon\n");
	CSingleUserRecipientFilter user( (CBasePlayer *)this );
	user.MakeReliable();
	UserMessageBegin(user, "StatusIconUpdate");
	WRITE_BYTE(FF_ICON_ONFIRE);
	WRITE_FLOAT(flIconDuration);
	MessageEnd();
	//DevMsg("[Grenade Debug] setting player on fire\n");

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
	switch(m_iGrenadeState)
	{
		case FF_GREN_PRIMEONE:
			
			// They don't actually have a primary grenade
			if( Q_strcmp( pPlayerClassInfo.m_szPrimaryClassName, "None" ) == 0 )
				return;

			// Make the grenade
			pGrenade = (CFFGrenadeBase *)CreateEntityByName( pPlayerClassInfo.m_szPrimaryClassName );
			// Set the grenades team (for use later)
			pGrenade->ChangeTeam( GetTeamNumber() );
			break;

		case FF_GREN_PRIMETWO:

			// They don't actually have a secondary grenade
			if( Q_strcmp( pPlayerClassInfo.m_szSecondaryClassName, "None" ) == 0 )
				return;

			// Make the grenade
			pGrenade = (CFFGrenadeBase *)CreateEntityByName( pPlayerClassInfo.m_szSecondaryClassName );
			// Set the grenades team (for use later)
			pGrenade->ChangeTeam( GetTeamNumber() );
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

#ifdef GAME_DLL
		pGrenade->SetDetonateTimerLength( fTimer );
		pGrenade->SetupInitialTransmittedVelocity(vecVelocity);

		if (fTimer > 0)
			pGrenade->m_fIsHandheld = false;
#endif
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

	entsys.SetVar("info_damage", info.GetDamage());
	entsys.SetVar("info_attacker", ENTINDEX(info.GetAttacker()));
	entsys.SetVar("info_classname", info.GetInflictor()->GetClassname());
    CFFPlayer *player = ToFFPlayer(info.GetInflictor());
    if (player)
    {
        CBaseCombatWeapon *weapon = player->GetActiveWeapon();
        if (weapon)
			entsys.SetVar("info_classname", weapon->GetName());
	}
	entsys.RunPredicates_LUA(NULL, this, "player_ondamage");
	info.SetDamage(entsys.GetFloat("info_damage"));

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
		//m_bRadioTagged = true;
		//m_flRadioTaggedStartTime = gpGlobals->curtime;

		// Setting time to max
		//m_flRadioTaggedDuration = 3 * inputInfo.GetSniperRifleCharge( );
		//m_flRadioTaggedDuration = radiotag_draw_duration.GetInt();

		// Keep track of who's radio tagged us to award them each a point when we die
		//m_hWhoTaggedMeList.AddToTail( ToFFPlayer( info.GetAttacker( ) ) );
		//m_pWhoTaggedMe = ToFFPlayer( info.GetAttacker() );
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

	EmitSound("Player.Pain");	// |-- Mirv: Emit some pain sound why don't you.

	m_bitsDamageType |= bitsDamage; // Save this so we can report it to the client
	m_bitsHUDDamage = -1;  // make sure the damage bits get resent

	return fTookDamage;
}
// ---> end

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
	else if (m_iHealth < 50.0f && info.GetDamageType() & DMG_BLAST)
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

	// Now spawn any limbs that are needed
	// [TODO] This is not a great way to do it okay!
	for (int i = 0; i <= 4; i++)
	{
		if (m_fBodygroupState & (1 << i))
		{
			CFFRagdoll *pRagdoll = dynamic_cast< CFFRagdoll * > (CreateEntityByName("ff_ragdoll"));

			if (!pRagdoll)
			{
				Warning("Couldn't make limb ragdoll!\n");
				return;
			}

			if (pRagdoll)
			{
				pRagdoll->m_hPlayer = this;
				pRagdoll->m_vecRagdollOrigin = GetAbsOrigin();
				pRagdoll->m_vecRagdollVelocity = GetAbsVelocity();
				pRagdoll->m_nModelIndex = g_iLimbs[GetClassSlot() ][i];
				pRagdoll->m_nForceBone = m_nForceBone;
				pRagdoll->m_vecForce = Vector(0, 0, 0);
				pRagdoll->m_fBodygroupState = 0;

				// remove it after a time
				pRagdoll->SetThink(&CBaseEntity::SUB_Remove);
				pRagdoll->SetNextThink(gpGlobals->curtime + 10.0f);
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
	// Some sort of HP check here
	if( false )
		return true;
	else
		return false;
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
// Purpose: Apply the concussion effect on this player
//-----------------------------------------------------------------------------
void CFFPlayer::Concuss( float amount, const QAngle *viewjerk )
{
	if( amount > 0 )
		m_flConcTime = gpGlobals->curtime + amount;

	if( viewjerk )
		ViewPunch( (*viewjerk) * jerkmulti.GetFloat() );
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

	CEntityFlame *pFlame = NULL;

	// Extend the flames for however long
	if (GetEffectEntity())
	{
		pFlame = dynamic_cast<CEntityFlame *> (GetEffectEntity());

		// We shouldn't have any other ones
		Assert(pFlame);
	}
	else
	{
		pFlame = CEntityFlame::Create( this );
		SetEffectEntity(pFlame);

		// We should now have one
		Assert(pFlame);
	}

	// There isn't already a flame
	pFlame->SetLifetime(flFlameLifetime);


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
		WRITE_BYTE(FF_ICON_ONFIRE);
		WRITE_FLOAT(0.0);
		MessageEnd();

		RemoveFlag(FL_ONFIRE);
	}

	// Make sure these are turned off
	m_iBurnTicks = 0;
	m_flBurningDamage = 0;

	CEntityFlame *pFlame = (CEntityFlame *) GetEffectEntity();

	if (!pFlame)
		return;

	// When we kill the flame it calls this again so we'll do this to avoid infinite loop
	if (pFlame->m_flLifetime > gpGlobals->curtime)
	{
		// Bug #0000162: Switching class while on fire, keeps playing burn sound
		pFlame->SetLifetime(-1);
		pFlame->FlameThink();
	}
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
	g_StatsLog.AddToCount(pHealer, STAT_HEALS, 1);
	g_StatsLog.AddToCount(pHealer, STAT_HPHEALED, m_iHealth - iOriginalHP);
	
	// Critical heal is when they are <= 15hp
	if (iOriginalHP <= 15)
		g_StatsLog.AddToCount(pHealer, STAT_CRITICALHEALS, 1);

	if (m_bInfected)
	{
		m_bInfected = false;

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
	if (hp && flHealth)
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

	Warning( "[Disguise] [Server] Disguise team: %i, Disguise class: %i\n", iTeam, iClass );

	m_iNewSpyDisguise = iTeam;
	m_iNewSpyDisguise += iClass << 4;

	m_flFinishDisguise = gpGlobals->curtime + 1.0f;

	ClientPrint( this, HUD_PRINTTALK, "#FF_SPY_DISGUISING" );

	// Notify the bot: convert this to an event?
	if(IsBot())
	{
		Omnibot::Notify_Disguising(this, GetDisguisedTeam(), GetDisguisedClass());
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

int CFFPlayer::AddHealth(unsigned int amount)
{
	int left = TakeHealth( amount, DMG_GENERIC );
    return left;		
}

//-----------------------------------------------------------------------------
int CFFPlayer::AddAmmo(const char* ammo, unsigned int amount)
{
	int dispensed = 0;

	if (FStrEq(ammo, "AMMO_GREN1"))
		dispensed = AddPrimaryGrenades( amount );
	else if (FStrEq(ammo, "AMMO_GREN2"))
		dispensed = AddSecondaryGrenades( amount );
	else
		dispensed = GiveAmmo( amount, ammo, true );

	return dispensed;
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
//			Currently no splining is done.
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
			m_hSabotaging->Sabotage(this);
			m_hSabotaging = NULL;

			ClientPrint(this, HUD_PRINTCENTER, "#FF_BUILDINGSABOTAGED");

			// Fire an event.
			IGameEvent *pEvent = NULL;
			if(m_hSabotaging->Classify() == CLASS_SENTRYGUN)
				pEvent = gameeventmanager->CreateEvent("sentry_sabotaged");
			else if(m_hSabotaging->Classify() == CLASS_DISPENSER)
				pEvent = gameeventmanager->CreateEvent("dispenser_sabotaged");
			if(pEvent)
			{
				CFFPlayer *pPlayer = ToFFPlayer(GetOwnerEntity());
				pEvent->SetInt("userid", pPlayer->GetUserID());
				pEvent->SetInt("saboteur", this->GetUserID());				
			}
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
		return GetLegacyAbsOrigin() + (Vector(0, 0, 28) * random->RandomFloat(0.8f, 1.1f));
	}
	else
	{
		return GetLegacyAbsOrigin(); //return EyePosition(); 
	}
};