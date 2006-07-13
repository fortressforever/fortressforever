////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-04-30 17:19:23 -0400 (Sun, 30 Apr 2006) $
// $LastChangedRevision: 1200 $
//
// Title: TF Config
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __TF_EVENTS_H__
#define __TF_EVENTS_H__

#include "Omni-Bot_Types.h"
#include "Omni-Bot_Events.h"

// These values should be verified to be correct with the game.
#define RADAR_CELL_REQUIREMENT 5
#define BUILD_SENTRY_REQUIREMENT 130
#define BUILD_DISPENSER_REQUIREMENT 100

// typedef: TF_Events
//		Defines the events specific to the TF game, numbered starting at the end of
//		the global events.
typedef enum eTF_Events
{
	TF_MSG_BEGIN = EVENT_NUM_EVENTS,

	// General Events
	TF_CLASS_DISABLED, // todo: implement this
	TF_CLASS_NOTAVAILABLE, // todo: implement this
	TF_CLASS_CHANGELATER, // todo: implement this
	TF_MSG_BUILD_MUSTBEONGROUND,

	// Scout
	TF_MSG_SCOUT_START,
		// Game Events
		TF_MSG_RADAR_DETECT_ENEMY,
		// Internal Events.
	TF_MSG_SCOUT_END,

	// Sniper
	TF_MSG_SNIPER_START,
		// Game Events
		TF_MSG_RADIOTAG_UPDATE,
		// Internal Events
	TF_MSG_SNIPER_END,

	// Soldier
	TF_MSG_SOLDIER_START,
		// Game Events
		// Internal Events
	TF_MSG_SOLDIER_END,

	// Demo-man
	TF_MSG_DEMOMAN_START,
		// Game Events
		TF_MSG_DETPACK_BUILDING,
		TF_MSG_DETPACK_BUILT,
		TF_MSG_DETPACK_NOTENOUGHAMMO,
		TF_MSG_DETPACK_CANTBUILD,
		TF_MSG_DETPACK_ALREADYBUILT,
		TF_MSG_DETPACK_DETONATED,
		// Internal Events
		TF_MSG_DETPIPES,		// The bot has detected the desire to det pipes.
		TF_MSG_DETPIPESNOW,		// Configurable delayed message for the actual detting.
	TF_MSG_DEMOMAN_END,
	
	// Medic
	TF_MSG_MEDIC_START,
		// Game Events
		TF_MSG_CALLFORMEDIC,
		// Internal Events
	TF_MSG_MEDIC_END,

	// HW-Guy
	TF_MSG_HWGUY_START,
		// Game Events	
		// Internal Events
	TF_MSG_HWGUY_END,

	// Pyro
	TF_MSG_PYRO_START,
		// Game Events	
		// Internal Events
	TF_MSG_PYRO_END,

	// Spy
	TF_MSG_AGENT_START,	
		// Game Events
		TF_MSG_DISGUISING,
		TF_MSG_DISGUISED,
		TF_MSG_DISGUISE_LOST,
		TF_MSG_CANT_FEIGN,
		TF_MSG_FEIGNED,
		TF_MSG_UNFEIGNED,
		TF_MSG_SABOTAGED_SENTRY,
		TF_MSG_SABOTAGED_DISPENSER,
		// Internal Events
	TF_MSG_AGENT_END,

	// Engineer
	TF_MSG_ENGINEER_START,
		TF_MSG_SENTRY_START,
			// Game Events
			TF_MSG_SENTRY_NOTENOUGHAMMO,
			TF_MSG_SENTRY_ALREADYBUILT,
			TF_MSG_SENTRY_CANTBUILD,
			TF_MSG_SENTRY_BUILDING,
			TF_MSG_SENTRY_BUILT,
			TF_MSG_SENTRY_DESTROYED,
			TF_MSG_SENTRY_SPOTENEMY,
			TF_MSG_SENTRY_DAMAGED,
			TF_MSG_SENTRY_STATS,
			TF_MSG_SENTRY_UPGRADED,
			TF_MSG_SENTRY_DETONATED,
			TF_MSG_SENTRY_DISMANTLED,
			// Internal Events
		TF_MSG_SENTRY_END,

		TF_MSG_DISPENSER_START,
			// Game Events
			TF_MSG_DISPENSER_NOTENOUGHAMMO,
			TF_MSG_DISPENSER_ALREADYBUILT,
			TF_MSG_DISPENSER_CANTBUILD,			
			TF_MSG_DISPENSER_BUILDING,
			TF_MSG_DISPENSER_BUILT,
			TF_MSG_DISPENSER_DESTROYED,
			TF_MSG_DISPENSER_ENEMYUSED,
			TF_MSG_DISPENSER_DAMAGED,
			TF_MSG_DISPENSER_STATS,
			TF_MSG_DISPENSER_DETONATED,
			TF_MSG_DISPENSER_DISMANTLED,
			// Internal Events
			TF_MSG_DISPENSER_BLOWITUP,
		TF_MSG_DISPENSER_END,
	TF_MSG_ENGINEER_END,

	// Civilian
	TF_MSG_CIVILIAN_START,
		// Game Events	
		// Internal Events
	TF_MSG_CIVILIAN_END,

	TF_MSG_END_EVENTS
} FF_Events;

// typedef: TF_GameMessage
//		Events that allow the bot to query for information from the game.
typedef enum eTF_GameMessage
{
	TF_MSG_START = GEN_MSG_END,

	// Info.
	TF_MSG_ISGUNCHARGING,
	TF_MSG_ISBUILDING,
	TF_MSG_GETBUILDABLES,

	// Get Info
	TF_MSG_PLAYERPIPECOUNT,
	TF_MSG_TEAMPIPEINFO,

	// Commands
	TF_MSG_DISGUISE,
	TF_MSG_FEIGN,
	TF_MSG_HUDHINT,

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
	TF_CLASSEX_EMP_GRENADE,
	TF_CLASSEX_NAIL_GRENADE,
	TF_CLASSEX_MIRV_GRENADE,
	TF_CLASSEX_MIRVLET_GRENADE,
	TF_CLASSEX_NAPALM_GRENADE,
	TF_CLASSEX_GAS_GRENADE,
	TF_CLASSEX_CONC_GRENADE,
	TF_CLASSEX_CALTROP,
	TF_CLASSEX_PIPE,
	TF_CLASSEX_ROCKET,

	TF_NUM_CLASSES
} TF_PlayerClass;

typedef enum eTF_EntityFlags
{
	// bit: FF_ENT_SAVEME
	//		This entity is has called for medic
	TF_ENT_FLAG_SAVEME		= (ENT_FLAG_FIRST_USER<<0),
	// bit: FF_ENT_ARMORME
	//		This entity has called for armor
	TF_ENT_FLAG_ARMORME		= (ENT_FLAG_FIRST_USER<<1),
	// bit: FF_ENT_BURNING
	//		This entity is on fire
	TF_ENT_FLAG_BURNING		= (ENT_FLAG_FIRST_USER<<2),
	// bit: FF_ENT_TRANQED
	//		This entity is tranquilized
	TF_ENT_FLAG_TRANQED		= (ENT_FLAG_FIRST_USER<<3),

	TF_ENT_SNIPERAIMING		= (ENT_FLAG_FIRST_USER<<4),
	TF_ENT_ASSAULTFIRING	= (ENT_FLAG_FIRST_USER<<5),
	TF_ENT_LEGSHOT			= (ENT_FLAG_FIRST_USER<<6),
	TF_ENT_CALTROP			= (ENT_FLAG_FIRST_USER<<8),
	TF_ENT_RADIOTAGGED		= (ENT_FLAG_FIRST_USER<<9),
	
	TF_ENT_CAN_SABOTAGE		= (ENT_FLAG_FIRST_USER<<10),
	TF_ENT_SABOTAGED		= (ENT_FLAG_FIRST_USER<<11),
	TF_ENT_SABOTAGING		= (ENT_FLAG_FIRST_USER<<12),

} FF_EntityFlags;

typedef enum eFF_Powerups
{
	// Team Disguise
	TF_PW_DISGUISE_BLUE		= (1<<0),
	TF_PW_DISGUISE_RED		= (1<<1),
	TF_PW_DISGUISE_YELLOW	= (1<<2),
	TF_PW_DISGUISE_GREEN	= (1<<3),

	// Class Disguise
	TF_PW_DISGUISE_SCOUT	= (1<<4),
	TF_PW_DISGUISE_SNIPER	= (1<<5),
	TF_PW_DISGUISE_SOLDIER	= (1<<6),
	TF_PW_DISGUISE_DEMOMAN	= (1<<7),
	TF_PW_DISGUISE_MEDIC	= (1<<8),
	TF_PW_DISGUISE_HWGUY	= (1<<9),
	TF_PW_DISGUISE_PYRO		= (1<<10),
	TF_PW_DISGUISE_ENGINEER	= (1<<11),
	TF_PW_DISGUISE_SPY		= (1<<12),
	TF_PW_DISGUISE_CIVILIAN	= (1<<13),
	
	// Other powerups
	TF_PW_FEIGNED			= (1<<14),

} FF_Powerups;

// typedef: TF_Weapon
//		The available weapons for this gametype
typedef enum eTF_Weapon
{
	TF_WP_NONE = 0,
	TF_WP_UMBRELLA,
	TF_WP_AXE,
	TF_WP_CROWBAR,
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
	TF_WP_RADIOTAG_RIFLE,
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
	goal_tf_detpack,
	goal_tf_pipetrap,
	goal_tf_laypipetrap,
	goal_tf_watchpipetrap,
	goal_tf_givehealth,
	goal_tf_givearmor,
	goal_tf_giveammo,

	goal_tf_num
} TF_Goals;

typedef enum eTF_BuildingStatus
{
	BUILDING_NONE,
	BUILDING_DISPENSER,
	BUILDING_SENTRY,
	BUILDING_DETPACK,
} TF_BuildingStatus;

#endif
