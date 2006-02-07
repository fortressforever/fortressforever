////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-01-28 12:33:58 -0500 (Sat, 28 Jan 2006) $
// $LastChangedRevision: 1137 $
//
// Title: TF Config
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __TF_EVENTS_H__
#define __TF_EVENTS_H__

#include "Omni-Bot_Types.h"
#include "Omni-Bot_Events.h"

// typedef: TF_Events
//		Defines the events specific to the TF game, numbered starting at the end of
//		the global events.
typedef enum eTF_Events
{
	TF_MESSAGE_BEGIN = EVENT_NUM_EVENTS,

	// General Events

	// Scout
	TF_MESSAGE_RADAR_DETECT_ENEMY,

	// Sniper

	// Soldier

	// Demo-man
	TF_MESSAGE_DETPIPES,		// The bot has detected the desire to det pipes.
	TF_MESSAGE_DETPIPESNOW,		// Configurable delayed message for the actual detting.

	// Medic
	TF_MESSAGE_CALLFORMEDIC,

	// HW-Guy

	// Pyro

	// Spy
	TF_MESSAGE_AGENT_START,
		TF_MESSAGE_AGENT_INFO,
	TF_MESSAGE_AGENT_END,

	// Engineer
	TF_MESSAGE_SENTRY_START,
		TF_MESSAGE_SENTRY_BUILDING,
		TF_MESSAGE_SENTRY_BUILT,
		TF_MESSAGE_SENTRY_DESTROYED,
		TF_MESSAGE_SENTRY_SPOTENEMY,
		TF_MESSAGE_SENTRY_DAMAGED,
		TF_MESSAGE_SENTRY_STATS,
	TF_MESSAGE_SENTRY_END,

	TF_MESSAGE_DISPENSER_START,
		TF_MESSAGE_DISPENSER_BUILDING,
		TF_MESSAGE_DISPENSER_BUILT,
		TF_MESSAGE_DISPENSER_DESTROYED,
		TF_MESSAGE_DISPENSER_ENEMYUSED,
		TF_MESSAGE_DISPENSER_DAMAGED,
		TF_MESSAGE_DISPENSER_TOUCHED,
		TF_MESSAGE_DISPENSER_STATS,
	TF_MESSAGE_DISPENSER_END,

	TF_MESSAGE_DISPENSER_DETONATE,
	TF_MESSAGE_DETPACK_BUILT,

	// Civilian

	TF_MESSAGE_END
} FF_Events;

// typedef: TF_GameMessage
//		Events that allow the bot to query for information from the game.
typedef enum eTF_GameMessage
{
	TF_MSG_START = GEN_MSG_END,

	// Info.
	TF_MSG_ISGUNCHARGING,
	//TF_MSG_ISINVISIBLE,
	//TF_MSG_ISDISGUISED,
	TF_MSG_ISBUILDING,
	TF_MSG_GETSENTRY,
	TF_MSG_GETDISPENSER,

	/*TF_MSG_GETSENTRYSTATS,
	TF_MSG_GETDISPENSERSTATS,*/

	// Effects
	/*TF_MSG_ISCONCED,
	TF_MSG_ISONFIRE,
	TF_MSG_ISINFECTED,
	TF_MSG_ISGASSED,
	TF_MSG_ISTRANQED,
	TF_MSG_ISBLIND,*/

	// Powerups
	/*TF_MSG_HASQUAD,
	TF_MSG_HASBATTLESUIT,
	TF_MSG_HASHASTE,
	TF_MSG_HASINVIS,
	TF_MSG_HASREGEN,
	TF_MSG_HASFLIGHT,
	TF_MSG_HASINVULN,
	TF_MSG_HASAQUALUNG,*/

	// Get Info
	TF_MSG_GETPLAYERPIPECOUNT,
	TF_MSG_GETTEAMPIPEINFO,

	TF_MSG_END
} TF_GameMessage;

// typedef: TF_PlayerClass_enum
//		The available classes for this gametype
typedef enum eTF_PlayerClass
{
	TF_CLASS_NONE = 0,
	TF_CLASS_SCOUT,
	TF_CLASS_SNIPER,
	TF_CLASS_SOLDIER,
	TF_CLASS_DEMOMAN,
	TF_CLASS_MEDIC,
	TF_CLASS_HWGUY,
	TF_CLASS_PYRO,
	TF_CLASS_SPY,
	TF_CLASS_ENGINEER,
	TF_CLASS_CIVILIAN,
	TF_CLASS_MAX,

	// Other values to identify the "class"
	TF_CLASSEX_SENTRY,
	TF_CLASSEX_DISPENSER,
	TF_CLASSEX_BACKPACK,
	TF_CLASSEX_DETPACK,
	TF_CLASSEX_GRENADE,
	TF_CLASSEX_PIPE,
	TF_CLASSEX_ROCKET,

	TF_NUM_CLASSES
} TF_PlayerClass;

typedef enum eTF_EntityFlags
{
	// bit: TF_ENT_FLAG_DISGUISED
	//		This entity is disguised
	TF_ENT_FLAG_DISGUISED	= (ENT_FLAG_FIRST_USER<<0),	
	// bit: TF_ENT_FLAG_INVISIBLE
	//		This entity is invisible
	TF_ENT_FLAG_INVISIBLE	= (ENT_FLAG_FIRST_USER<<1),
	// bit: FF_ENT_SAVEME
	//		This entity is has called for medic
	TF_ENT_FLAG_SAVEME		= (ENT_FLAG_FIRST_USER<<2),
	// bit: FF_ENT_ARMORME
	//		This entity has called for armor
	TF_ENT_FLAG_ARMORME	= (ENT_FLAG_FIRST_USER<<3),
	// bit: FF_ENT_BURNING
	//		This entity is on fire
	TF_ENT_FLAG_BURNING	= (ENT_FLAG_FIRST_USER<<4),
	// bit: FF_ENT_TRANQED
	//		This entity is tranquilized
	TF_ENT_FLAG_TRANQED	= (ENT_FLAG_FIRST_USER<<5),
} FF_EntityFlags;

// typedef: TF_Weapon
//		The available weapons for this gametype
typedef enum eTF_Weapon
{
	TF_WP_NONE = 0,
	TF_WP_UMBRELLA,
	TF_WP_AXE,
	TF_WP_MEDKIT,
	TF_WP_KNIFE,
	TF_WP_SPANNER,
	TF_WP_SHOTGUN,
	TF_WP_SUPERSHOTGUN,
	TF_WP_NAILGUN,
	TF_WP_SUPERNAILGUN,
	TF_WP_GRENADE_LAUNCHER,
	TF_WP_ROCKET_LAUNCHER,
	TF_WP_SNIPER_RIFLE,
	TF_WP_RAILGUN,
	TF_WP_FLAMETHROWER,
	TF_WP_MINIGUN,
	TF_WP_AUTORIFLE,
	TF_WP_DARTGUN,
	TF_WP_PIPELAUNCHER,
	TF_WP_NAPALMCANNON,
	TF_WP_MAX
} TF_Weapon;

// typedef: TF_AmmoType
//		The available ammo types for this gametype
typedef enum eTF_AmmoType
{
	TF_AMMO_SHELLS,
	TF_AMMO_NAILS,
	TF_AMMO_ROCKETS,
	TF_AMMO_CELLS,
	TF_AMMO_MEDIKIT,
	TF_AMMO_DETPACK,
	TF_AMMO_RADIOTAG,
	TF_AMMO_GRENADE1,
	TF_AMMO_GRENADE2,
	TF_AMMO_MAX
} TF_AmmoType;

// typedef: TF_GrenadeTypes
//		The available grenade types in TF
typedef enum eTF_GrenadeTypes
{
	TF_GREN_NONE,
	TF_GREN_NORMAL,
	TF_GREN_CONCUSS,
	TF_GREN_FLASH,
	TF_GREN_FLARE,
	TF_GREN_NAIL,
	TF_GREN_CLUSTER,
	TF_GREN_CLUSTERSECTION,
	TF_GREN_NAPALM,
	TF_GREN_GAS,
	TF_GREN_EMP,
	TF_GREN_CHARGE,
	FF_NUM_GRENADES
} TF_GrenadeTypes;

// typedef: TF_Team
//		The available teams for this gametype
typedef enum eTF_Team
{
	TF_TEAM_NONE = 0,
	TF_TEAM_BLUE,
	TF_TEAM_RED,
	TF_TEAM_YELLOW,
	TF_TEAM_GREEN,
	TF_TEAM_MAX
} TF_Team;

// typdef: GoalType
//		This enum defines the identifiers for goal entity types the game can register with the bot.
typedef enum eTF_GoalType
{
	TF_GOAL_FLAG = BASE_GOAL_NUM,
	TF_GOAL_CAPPOINT,
	TF_GOAL_SNIPEPOINT,
	TF_GOAL_SENTRYPOINT,
	TF_GOAL_DISPENSERPOINT,
	TF_GOAL_PIPETRAPPOINT,
	TF_GOAL_DETPACK,
} TF_GoalType;

// typedef: Goals_TF
//		Enumerations for TF specific goals.
typedef enum eTF_Goals
{
	goal_tf_sentry = goal_base_num,
	goal_tf_ss,
	goal_tf_getarmor,
	goal_tf_gethealth,
	goal_tf_getammo,
	goal_tf_buildsentry,
	goal_tf_upgradesentry,
	goal_tf_buildss,
	goal_tf_pipetrap,
	goal_tf_laypipetrap,
	goal_tf_watchpipetrap,
	goal_tf_givehealth,
	goal_tf_givearmor,
	goal_tf_giveammo,

	goal_tf_num
} TF_Goals;

//////////////////////////////////////////////////////////////////////////
// Message Helpers
typedef struct  
{
	obint32		m_NumTeamPipes;
	obint32		m_NumTeamPipers;
	obint32		m_MaxPipesPerPiper;
} TF_TeamPipeInfo;

typedef enum
{
	BUILDING_NONE,
	BUILDING_DISPENSER,
	BUILDING_SENTRY,
	BUILDING_DETPACK,
} TF_BuildingStatus;

#endif
