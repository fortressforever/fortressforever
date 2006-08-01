////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-07-20 08:59:11 -0700 (Thu, 20 Jul 2006) $
// $LastChangedRevision: 1234 $
//
// Title: TF Config
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __TF_EVENTS_H__
#define __TF_EVENTS_H__

#include "Omni-Bot_Types.h"
#include "Omni-Bot_Events.h"

// constants: Game constants
//		RADAR_CELL_REQUIREMENT - Number of cells required to use the radar.
//		BUILD_SENTRY_REQUIREMENT - Number of cells required to build a sentry gun.
//		BUILD_DISPENSER_REQUIREMENT - Number of cells required to build a dispenser.
//		MAX_DEMO_PIPETRAPS - Max number of traps to lay at once.
//		MAX_DEMO_PIPES - Max number of pipes a single demo-man can lay.
//		MAX_DEMO_TEAM_PIPES - Max number of pipes available for an entire team.
#define RADAR_CELL_REQUIREMENT 5
#define BUILD_SENTRY_REQUIREMENT 130
#define BUILD_SENTRY_UPGRADE_RADIUS 64.0
#define BUILD_DISPENSER_REQUIREMENT 100
#define MAX_DEMO_PIPETRAPS 3
#define MAX_DEMO_PIPES 6
#define MAX_DEMO_TEAM_PIPES 8

// enumerations: TF_EntityClass
//		TF_CLASS_SCOUT - Scout player class.
//		TF_CLASS_SNIPER - Sniper player class.
//		TF_CLASS_SOLDIER - Soldier player class.
//		TF_CLASS_DEMOMAN - Demo-man player class.
//		TF_CLASS_MEDIC - Medic player class.
//		TF_CLASS_HWGUY - HWGuy player class.
//		TF_CLASS_PYRO - Pyro player class.
//		TF_CLASS_SPY - Spy player class.
//		TF_CLASS_ENGINEER - Engineer player class.
//		TF_CLASS_CIVILIAN - Civilian player class.
//		TF_CLASSEX_SENTRY - Sentry entity.
//		TF_CLASSEX_DISPENSER - Dispenser entity.
//		TF_CLASSEX_BACKPACK - Backpack entity.
//		TF_CLASSEX_DETPACK - Detpack entity.
//		TF_CLASSEX_GRENADE - Grenade entity.
//		TF_CLASSEX_EMP_GRENADE - EMP Grenade entity.
//		TF_CLASSEX_NAIL_GRENADE - Nail Grenade entity.
//		TF_CLASSEX_MIRV_GRENADE - Mirvlet Grenade entity.
//		TF_CLASSEX_MIRVLET_GRENADE - Mirvlet Grenade entity.
//		TF_CLASSEX_NAPALM_GRENADE - Napalm Grenade entity.
//		TF_CLASSEX_GAS_GRENADE - Gas Grenade entity.
//		TF_CLASSEX_CONC_GRENADE - Concussion Grenade entity.
//		TF_CLASSEX_CALTROP - Caltrop Grenade entity.
//		TF_CLASSEX_PIPE - Pipe Grenade entity.
//		TF_CLASSEX_ROCKET - Rocket Grenade entity.
typedef enum eTF_EntityClass
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
	TF_CLASSEX_GLGRENADE,
	TF_CLASSEX_ROCKET,
	TF_CLASSEX_TURRET,

	TF_NUM_CLASSES
} TF_EntityClass;

// enumerations: TF_EntityFlags
//		TF_ENT_FLAG_SAVEME - This entity is has called for medic.
//		TF_ENT_FLAG_ARMORME - This entity has called for armor.
//		TF_ENT_FLAG_BURNING - This entity is on fire.
//		TF_ENT_FLAG_TRANQED - This entity is tranquilized.
//		TF_ENT_SNIPERAIMING - This entity is aiming a scoped weapon.
//		TF_ENT_ASSAULTFIRING - This entity is shooting an assault weapon.
//		TF_ENT_LEGSHOT - This entity is suffering from a leg shot.
//		TF_ENT_CALTROP - This entity is suffering from a caltrop.
//		TF_ENT_RADIOTAGGED - This entity has been radio tagged.
//		TF_ENT_CAN_SABOTAGE - This entity can be sabotaged.
//		TF_ENT_SABOTAGED - This entity has been sabotaged.
//		TF_ENT_SABOTAGING - This entity is sabotaging something.
typedef enum eTF_EntityFlags
{
	TF_ENT_FLAG_SAVEME = (ENT_FLAG_FIRST_USER),	
	TF_ENT_FLAG_ARMORME,
	TF_ENT_FLAG_BURNING,
	TF_ENT_FLAG_TRANQED,
	TF_ENT_FLAG_SNIPERAIMING,
	TF_ENT_FLAG_ASSAULTFIRING,
	TF_ENT_FLAG_LEGSHOT,
	TF_ENT_FLAG_CALTROP,
	TF_ENT_FLAG_RADIOTAGGED,
	TF_ENT_FLAG_CAN_SABOTAGE,
	TF_ENT_FLAG_SABOTAGED,
	TF_ENT_FLAG_SABOTAGING,
	TF_ENT_FLAG_BUILDING_SG,
	TF_ENT_FLAG_BUILDING_DISP,
	TF_ENT_FLAG_BUILDING_DETP,
} TF_EntityFlags;

// enumerations: TF_Powerups
//		TF_PWR_DISGUISE_BLUE - Disguised as blue team.
//		TF_PWR_DISGUISE_RED - Disguised as red team.
//		TF_PWR_DISGUISE_YELLOW - Disguised as yellow team.
//		TF_PWR_DISGUISE_GREEN - Disguised as green team.
//		TF_PWR_DISGUISE_SCOUT - Disguised as scout.
//		TF_PWR_DISGUISE_SNIPER - Disguised as sniper.
//		TF_PWR_DISGUISE_SOLDIER - Disguised as soldier.
//		TF_PWR_DISGUISE_DEMOMAN - Disguised as demo-man.
//		TF_PWR_DISGUISE_MEDIC - Disguised as medic.
//		TF_PWR_DISGUISE_HWGUY - Disguised as hwguy.
//		TF_PWR_DISGUISE_PYRO - Disguised as pyro.
//		TF_PWR_DISGUISE_SPY - Disguised as spy.
//		TF_PWR_DISGUISE_CIVILIAN - Disguised as civilian.
//		TF_PWR_FEIGNED - Entity is feigned.
typedef enum eTF_Powerups
{
	// Team Disguise
	TF_PWR_DISGUISE_BLUE = PWR_FIRST_USER,
	TF_PWR_DISGUISE_RED,
	TF_PWR_DISGUISE_YELLOW,
	TF_PWR_DISGUISE_GREEN,

	// Class Disguise
	TF_PWR_DISGUISE_SCOUT,
	TF_PWR_DISGUISE_SNIPER,
	TF_PWR_DISGUISE_SOLDIER,
	TF_PWR_DISGUISE_DEMOMAN,
	TF_PWR_DISGUISE_MEDIC,
	TF_PWR_DISGUISE_HWGUY,
	TF_PWR_DISGUISE_PYRO,
	TF_PWR_DISGUISE_ENGINEER,
	TF_PWR_DISGUISE_SPY,
	TF_PWR_DISGUISE_CIVILIAN,
	
	// Other powerups
	TF_PWR_FEIGNED,
} TF_Powerups;

// enumerations: TF_Weapon
// 		WP_UMBRELLA - Umbrella.
// 		WP_AXE - Axe.
// 		WP_CROWBAR - Crowbar.
// 		WP_MEDKIT - Med-kit.
// 		WP_KNIFE - Knife.
// 		WP_SPANNER - Spanner/wrench.
// 		WP_SHOTGUN - Shotgun.
// 		WP_SUPERSHOTGUN - Super shotgun.
// 		WP_NAILGUN - Nailgun.
// 		WP_SUPERNAILGUN - Super Nailgun.
// 		WP_GRENADE_LAUNCHER - Grenade Launcher.
// 		WP_ROCKET_LAUNCHER - Rocket Launcher.
// 		WP_SNIPER_RIFLE - Sniper Rifle.
// 		WP_RADIOTAG_RIFLE - Radio Tag Rifle.
// 		WP_RAILGUN - Railgun.
// 		WP_FLAMETHROWER - Flamethrower.
// 		WP_MINIGUN - Minigun/Assault cannon.
// 		WP_AUTORIFLE - Auto-rifle.
// 		WP_DARTGUN - Dart gun.
// 		WP_PIPELAUNCHER - Pipe Launcher.
// 		WP_NAPALMCANNON - Napalm Cannon.
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
	TF_WP_DEPLOY_SG,
	TF_WP_DEPLOY_DISP,
	TF_WP_DEPLOY_DETP,
	TF_WP_FLAG,
	TF_WP_MAX
} TF_Weapon;

// enumerations: TF_AmmoType
//		TF_AMMO_SHELLS - Shotgun shells.
//		TF_AMMO_NAILS - Nails.
//		TF_AMMO_ROCKETS - Rockets.
//		TF_AMMO_CELLS - Cells.
//		TF_AMMO_MEDIKIT - Medkit.
//		TF_AMMO_DETPACK - Detpack.
//		TF_AMMO_RADIOTAG - Radio Tag.
//		TF_AMMO_GRENADE1 - Grenade 1.
//		TF_AMMO_GRENADE2 - Grenade 2.
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

// constants: TF_GrenadeTypes
//		TF_GREN_NORMAL - Normal grenade.
//		TF_GREN_CONCUSS - Concussion grenade.
//		TF_GREN_FLASH - Flash grenade.
//		TF_GREN_FLARE - Flare grenade.
//		TF_GREN_NAIL - Nail grenade.
//		TF_GREN_CLUSTER - Cluster Grenade(Mirv).
//		TF_GREN_CLUSTERSECTION - Cluster Section(Mirvlet).
//		TF_GREN_NAPALM - Napalm/Incendiary grenade.
//		TF_GREN_GAS - Gas/Hallucinogen grenade.
//		TF_GREN_EMP - EMP grenade.
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
	TF_NUM_GRENADES
} TF_GrenadeTypes;

// enumerations: TF_Team
//		TF_TEAM_BLUE - Blue team.
//		TF_TEAM_RED - Red team.
//		TF_TEAM_YELLOW - Yellow team.
//		TF_TEAM_GREEN - Green team.
typedef enum eTF_Team
{
	TF_TEAM_NONE = 0,
	TF_TEAM_BLUE,
	TF_TEAM_RED,
	TF_TEAM_YELLOW,
	TF_TEAM_GREEN,
	TF_TEAM_MAX
} TF_Team;

// typedef: TF_Events
//		Defines the events specific to the TF game, numbered starting at the end of
//		the global events.
typedef enum eTF_Events
{
	TF_MSG_BEGIN = EVENT_NUM_EVENTS,

	// General Events
	TF_MSG_CLASS_DISABLED, // todo: implement this
	TF_MSG_CLASS_NOTAVAILABLE, // todo: implement this
	TF_MSG_CLASS_CHANGELATER, // todo: implement this
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
	TF_MSG_SENTRY_AIMED,
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
} TF_Events;

// typedef: TF_GameMessage
//		Events that allow the bot to query for information from the game.
typedef enum eTF_GameMessage
{
	TF_MSG_START = GEN_MSG_END,

	// Info.
	TF_MSG_ISGUNCHARGING,
	TF_MSG_GETBUILDABLES,

	// Get Info
	TF_MSG_PLAYERPIPECOUNT,
	TF_MSG_TEAMPIPEINFO,

	// Commands
	TF_MSG_DISGUISE,
	TF_MSG_FEIGN,
	TF_MSG_LOCKPOSITION,
	TF_MSG_HUDHINT,
	TF_MSG_HUDMENU,

	TF_MSG_END
} TF_GameMessage;

// typdef: TF_GoalType
//		This enum defines the identifiers for goal entity types the game can register with the bot.
typedef enum eTF_GoalType
{
	TF_GOAL_FLAG = BASE_GOAL_NUM,
	TF_GOAL_BACK_PACK,
	TF_GOAL_CAPPOINT,
	TF_GOAL_SNIPEPOINT,
	TF_GOAL_SENTRYPOINT,
	TF_GOAL_DISPENSERPOINT,
	TF_GOAL_PIPETRAPPOINT,
	TF_GOAL_DETPACK,
} TF_GoalType;

// typedef: TF_Goals
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

// typedef: TF_BuildableStatus
//		Enumerations for TF building status.
typedef enum eTF_BuildableStatus
{
	BUILDABLE_INVALID,
	BUILDABLE_BUILDING,
	BUILDABLE_BUILT,
} TF_BuildableStatus;
#endif
