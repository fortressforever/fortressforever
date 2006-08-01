////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-07-20 08:59:11 -0700 (Thu, 20 Jul 2006) $
// $LastChangedRevision: 1234 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __OMNIBOT_TYPES_H__
#define __OMNIBOT_TYPES_H__

#include "Omni-Bot_BasicTypes.h"

// constants: Omni-bot Errors
//		BOT_ERROR_NONE - No error
//		BOT_ERROR_CANTLOADDLL - Can't load DLL
//		BOT_ERROR_CANTGETBOTFUNCTIONS - Unable to get functions from bot
//		BOT_ERROR_CANTINITBOT - Unable to init bot
//		BOT_ERROR_BAD_INTERFACE - Bad interface passed to bot
//		BOT_ERROR_WRONGVERSION - Version mismatch between interface and bot
typedef enum eomnibot_error
{
	BOT_ERROR_NONE,
	BOT_ERROR_CANTLOADDLL,
	BOT_ERROR_CANTGETBOTFUNCTIONS,
	BOT_ERROR_CANTINITBOT,
	BOT_ERROR_BAD_INTERFACE,
	BOT_ERROR_WRONGVERSION,

	// Keep this last
	BOT_NUM_ERRORS
} omnibot_error;

// typedef: GameEntity
//		Represents an entity to the bot for every game.
typedef obvoidp GameEntity;

// typedef: GameId
//		A numeric value for an entities game id. Usually an array index of some sort.
typedef int GameId;

// typedef: NavigationFlags
//		This is the type waypoint flags, and should be a 64 bit type
typedef obuint64 NavigationFlags;

// enum: obBool
//		Since theres no promise that we're running in C or C++, and therefor no
//		guarantee that bools are supported, lets use an enumerated value instead.
typedef enum eobBool
{
	Invalid = -1,
	False,
	True
} obBool;

// enumerations: obResult
//		Success - Successful.
//		OutOfPVS - Out of PVS(Potential Visibility Set).
//		UnableToAddBot - Unable to add bot for some reason.
//		InvalidEntity - Invalid entity parameter.
//		InvalidParameter - Invalid parameter.
typedef enum eobResult
{
	Success = 0,
	OutOfPVS,
	UnableToAddBot,
	InvalidEntity,
	InvalidParameter
} obResult;

#ifdef __cplusplus
inline int SUCCESS(obResult _res)
{
	return (_res == Success) ? 1 : 0;
}
#else
#define SUCCESS(res) ((res) == Success ? 1 : 0)
#endif
//#define SUCCESS(x) ((x)==Success ? true : false)

// enumerations: BotDebugFlag
//		BOT_DEBUG_LOG - Debug log for this bot.
//		BOT_DEBUG_MOVEVEC - Draw the move vector.
//		BOT_DEBUG_AIMPOINT - Draw a line to the aim point.
//		BOT_DEBUG_GOALS - Output info about the bot goals.
//		BOT_DEBUG_SENSORY - Draw lines to sensed entities.
//		BOT_DEBUG_BRAIN - Output info from the bot brain.
//		BOT_DEBUG_WEAPON - Output info about weapon system.
//		BOT_DEBUG_SCRIPT - Output info about bot script events/signals.
//		BOT_DEBUG_EVENTS - Output Event info.
//		BOT_DEBUG_FPINFO - Output first person info.
typedef enum eBotDebugFlag
{
	BOT_DEBUG_LOG		= (1<<0),
	BOT_DEBUG_MOVEVEC	= (1<<1),
	BOT_DEBUG_AIMPOINT	= (1<<2),
	BOT_DEBUG_GOALS		= (1<<3),
	BOT_DEBUG_SENSORY	= (1<<4),
	BOT_DEBUG_BRAIN		= (1<<5),
	BOT_DEBUG_WEAPON	= (1<<6),
	BOT_DEBUG_SCRIPT	= (1<<7),
	BOT_DEBUG_EVENTS	= (1<<8),
	BOT_DEBUG_FPINFO	= (1<<9),
	BOT_DEBUG_PLANNER	= (1<<10),

	// THIS MUST STAY LAST
	NUM_BOT_DEBUG_FLAGS = (1<<16)
} BotDebugFlag;

// enumerations: Helpers
//		RANDOM_CLASS - Pick a random class.
//		RANDOM_CLASS_IF_NO_CLASS - Pick a random class if we don't already have a class.
//		RANDOM_TEAM - Pick a random team.
//		RANDOM_TEAM_IF_NO_TEAM -  - Pick a random team if we don't already have a team.
typedef enum eHelpers
{
	RANDOM_CLASS				= -1,
	RANDOM_CLASS_IF_NO_CLASS	= -2,
	RANDOM_TEAM					= -1,
	RANDOM_TEAM_IF_NO_TEAM		= -2,
} Helpers;

// enumerations: obLineTypes
//		LINE_NONE - Null line
//		LINE_NORMAL - Normal line.
//		LINE_WAYPOINT - Waypoint line.
//		LINE_PATH - Path line.
//		LINE_BLOCKABLE - Blockable line.
typedef enum eLineType
{
	LINE_NONE,
	LINE_NORMAL,
    LINE_WAYPOINT,
	LINE_PATH,
	LINE_BLOCKABLE,
} LineType;

// typedef: AABB
//		Represents the axis aligned bounding box of an object
typedef struct AABB_t
{
	float	m_Mins[3];
	float	m_Maxs[3];
#ifdef __cplusplus
	void CenterPoint(float _out[3])
	{
		_out[0] = (m_Mins[0] + m_Maxs[0]) * 0.5f;
		_out[1] = (m_Mins[1] + m_Maxs[1]) * 0.5f;
		_out[2] = (m_Mins[2] + m_Maxs[2]) * 0.5f;
	}
	AABB_t(float _mins[3], float _maxs[3])
	{
		for(int i = 0; i < 3; ++i)
		{
			m_Mins[i] = _mins[i];
			m_Maxs[i] = _maxs[i];
		}
	}
	AABB_t(float _center[3])
	{
		for(int i = 0; i < 3; ++i)
		{
			m_Mins[i] = _center[i];
			m_Maxs[i] = _center[i];
		}
	}
	AABB_t()
	{
	}
#endif
} AABB;

// Helper macro so the game can check the status of bot buttons.
#define BOT_CHECK_BUTTON(button, flag) (((button) & (1<<(flag))) != 0)

// enumerations: ButtonFlags
//		BOT_BUTTON_ATTACK1 - If the bot is pressing primary attack.
//		BOT_BUTTON_ATTACK2 - If the bot is pressing secondary attack.
//		BOT_BUTTON_JUMP - If the bot is pressing jump.
//		BOT_BUTTON_CROUCH - If the bot is pressing crouch.
//		BOT_BUTTON_PRONE - If the bot is pressing prone.
//		BOT_BUTTON_WALK - If the bot is pressing walk.
//		BOT_BUTTON_USE - If the bot is pressing use.
//		BOT_BUTTON_FWD - If the bot is pressing the forward key.
//		BOT_BUTTON_BACK - If the bot is pressing the backward key.
//		BOT_BUTTON_RSTRAFE - If the bot is pressing right strafe.
//		BOT_BUTTON_LSTRAFE - If the bot is pressing left strafe.
//		BOT_BUTTON_RELOAD - If the bot is pressing reload.
//		BOT_BUTTON_SPRINT - If the bot wants to sprint.
//		BOT_BUTTON_DROP - If the bot wants to drop current item.
//		BOT_BUTTON_LEANLEFT - If the bot wants to lean left.
//		BOT_BUTTON_LEANRIGHT - If the bot wants to lean right.
//		BOT_BUTTON_AIM - If the bot wants to drop current item.
typedef enum eButtonFlags
{	
	BOT_BUTTON_ATTACK1,
	BOT_BUTTON_ATTACK2,
	BOT_BUTTON_JUMP,
	BOT_BUTTON_CROUCH,
	BOT_BUTTON_PRONE,
	BOT_BUTTON_WALK,
	BOT_BUTTON_USE,
	BOT_BUTTON_FWD,
	BOT_BUTTON_BACK,
	BOT_BUTTON_RSTRAFE,
	BOT_BUTTON_LSTRAFE,
	BOT_BUTTON_RELOAD,
	BOT_BUTTON_SPRINT,
	BOT_BUTTON_DROP,
	BOT_BUTTON_LEANLEFT,
	BOT_BUTTON_LEANRIGHT,
	BOT_BUTTON_AIM,

	// THIS MUST BE LAST
	BOT_BUTTUN_FIRSTUSER
} ButtonFlags;

// enumerations: GoalType
//		GOAL_AMMO - Ammunition can be located at this goal
//		GOAL_ARMOR - Armor can be located at this goal
//		GOAL_HEALTH - Health can be located at this goal
//		GOAL_GOTO - The bot should just go to this goal
//		GOAL_DEFEND - The bot should just go to this goal and watch for targets.
//		GOAL_ATTACK - The bot should just go to this goal and hunt for targets.
//		GOAL_SNIPE - The bot should snipe from this point.
//		GOAL_CTF_FLAG - The bot should attempt to grab this flag and return it to a capture point.
//		GOAL_CTF_RETURN_FLAG - The bot should attempt to touch this flag to 'return' it.
//		GOAL_CTF_FLAGCAP - The bot should take GOAL_CTF_FLAG to this point.
//		GOAL_SCRIPT - A script should be ran for this goal.
typedef enum eGoalType
{
	GOAL_NONE = 0,
	GOAL_AMMO,
	GOAL_ARMOR,
	GOAL_HEALTH,
	GOAL_GOTO,
	GOAL_DEFEND,
	GOAL_ATTACK,
	GOAL_SNIPE,
	GOAL_CTF_FLAG,
	GOAL_CTF_RETURN_FLAG,
	GOAL_CTF_FLAGCAP,
	GOAL_SCRIPT,

	// THIS MUST BE LAST
	BASE_GOAL_NUM = 1000
} GoalType;

// A basic list of goals. There should be one of these for any base Goal class.
typedef enum eBasicGoals
{
	goal_none,
	goal_goto,
	goal_explore,
	goal_getflag,
	goal_capflag,
	goal_returnflag,
	goal_gotonode,
	goal_gethealth,
	goal_getarmor,
	goal_getammo,
	goal_snipe,
	goal_usedoor,
	goal_jumpgap,
	goal_negotiate_lift,
	goal_teleport,
	goal_script,
	goal_defend,
	goal_attack,
	goal_ride_movable,
	
	goal_hunt_target,
	/*goal_think,
	goal_explore,
	goal_arrive_at_position,
	goal_seek_to_position,
	goal_follow_path,
	goal_traverse_edge,
	goal_move_to_position,
	goal_get_shotgun,
	goal_get_rocket_launcher,
	goal_get_railgun,
	goal_wander,

	goal_attack_target,
	
	goal_strafe,
	goal_adjust_range,
	goal_say_phrase*/

	// THIS MUST BE LAST
	goal_base_num = 1000
} Goals_Base;

// enumerations: EntityFlag
//		ENT_FLAG_TEAM1 - This entity is only available/visible for team 1
//		ENT_FLAG_TEAM2 - This entity is only available/visible for team 2
//		ENT_FLAG_TEAM3 - This entity is only available/visible for team 3
//		ENT_FLAG_TEAM4 - This entity is only available/visible for team 4
//		ENT_FLAG_DISABLED - Entity is disabled
//		ENT_FLAG_PRONED - This entity is prone
//		ENT_FLAG_CROUCHED - This entity is crouched
//		ENT_FLAG_CARRYABLE - This entity is carryable(flag, powerup,...)
//		ENT_FLAG_DEAD - This entity is dead
//		ENT_FLAG_INWATER - This entity is in water
//		ENT_FLAG_UNDERWATER - This entity is under water
//		ENT_FLAG_ZOOMING - This entity is zooming through scope or binoculars.
//		ENT_FLAG_LADDER - This entity is on a ladder.
typedef enum eEntityFlag
{
	ENT_FLAG_NONE = 0,	
	ENT_FLAG_TEAM1,
	ENT_FLAG_TEAM2,
	ENT_FLAG_TEAM3,
	ENT_FLAG_TEAM4,	
	ENT_FLAG_DISABLED,	
	ENT_FLAG_PRONED,	
	ENT_FLAG_CROUCHED,	
    ENT_FLAG_CARRYABLE,	
	ENT_FLAG_DEAD,	
	ENT_FLAG_INWATER,	
	ENT_FLAG_UNDERWATER,	
	ENT_FLAG_ZOOMING,	
	ENT_FLAG_LADDER,

	// THIS MUST BE LAST
	ENT_FLAG_FIRST_USER	= 32
} EntityFlag;

// enumerations: Powerups
//		PW_INVINCIBLE - The entity is invincible.
typedef enum ePowerups
{
	PWR_NONE = 0,
	PWR_INVINCIBLE,

	// THIS MUST BE LAST	
	PWR_FIRST_USER		= 16,
} Powerups;

// enumerations: EntityCategory
//		ENT_CAT_PLAYER - This entity is a player of some sort.
//		ENT_CAT_PROJECTILE - This entity is a projectile of some sort.
//		ENT_CAT_SHOOTABLE - This entity is shootable.
//		ENT_CAT_PICKUP - This entity is a pickup/powerup of some sort.
//		ENT_CAT_TRIGGER - This entity is a trigger of some sort.
//		ENT_CAT_MOVER - This entity is a mover of some sort(lift, door,...).
//		ENT_CAT_AVOID - This entity is something bots should avoid.
//		ENT_CAT_MOUNTEDWEAPON - This entity is something bots can mount and use.
//		ENT_CAT_MISC - Miscellaneous entity category.
//		ENT_CAT_STATIC - Static entities don't need to be seen. This allows the bot to skip LOS checks.
typedef enum eEntityCategory
{
	ENT_CAT_PLAYER		= (1<<0),
	ENT_CAT_PROJECTILE	= (1<<1),
	ENT_CAT_SHOOTABLE	= (1<<2),
	ENT_CAT_PICKUP		= (1<<3),
	ENT_CAT_TRIGGER		= (1<<4),
	ENT_CAT_MOVER		= (1<<5),
	ENT_CAT_AVOID		= (1<<6),
	ENT_CAT_MOUNTEDWEAPON= (1<<7),
	ENT_CAT_MISC		= (1<<8),
	ENT_CAT_STATIC		= (1<<9),

	// THIS MUST BE LAST
	ENT_CAT_MAX			= (1<<16),
} EntityCategory;

// package: EntityClassGeneric
//		Class values for generic entities.
typedef enum eEntityClassGeneric
{
	ENT_CLASS_GENERIC_START = 10000,
	ENT_CLASS_GENERIC_BUTTON,
	ENT_CLASS_GENERIC_HEALTH,
	ENT_CLASS_GENERIC_AMMO,
	ENT_CLASS_GENERIC_ARMOR,
} EntityClassGeneric;

// enumerations: SoundType
//		SND_JUMP - Sound of jump from another entity.
//		SND_FOOTSTEP - Sound of footstep from another entity.
//		SND_TAKEDAMAGE - Sound of another entity taking damage.
//		SND_POWERUP_SPAWN - Sound of a powerup respawning.
//		SND_POWERUP_PICKUP - Sound of a powerup being picked up.
//		SND_WEAPON_FIRE - Sound of a weapon firing.
//		SND_WEAPON_RELOAD - Sound of a weapon reloading.
//		SND_WEAPON_EMPTY - Sound of a weapon empty.
//		SND_WEAPON_STARTFIRE - Sound of a weapon starting to fire.
//		SND_VOICE_TAUNT - Sound of a voice taunt.
//		SND_VOICE_TEAM - Sound of a voice team message.
//		SND_VOICE_ENEMY - Sound of a voice enemy message.
typedef enum eSoundType
{
	SND_NONE,
	SND_JUMP,
	SND_FOOTSTEP,
	SND_TAKEDAMAGE,
	SND_POWERUP_SPAWN,
	SND_POWERUP_PICKUP,
	SND_WEAPON_FIRE,
	SND_WEAPON_RELOAD,
	SND_WEAPON_EMPTY,
	SND_WEAPON_STARTFIRE,
	SND_VOICE_TAUNT,
	SND_VOICE_TEAM,
	SND_VOICE_ENEMY,

	// THIS MUST BE LAST!
	SND_MAX_SOUNDS
} SoundType;

// enumerations: Contents
//		CONT_SOLID - Solid object.
//		CONT_WATER - In water.
//		CONT_SLIME - In slime.
//		CONT_FOG - In fog.
//		CONT_TELEPORTER - In teleporter.
//		CONT_MOVER - In mover.
//		CONT_TRIGGER - In trigger.
//		CONT_LAVA - In lava.
typedef enum eContents
{
	CONT_SOLID		= (1<<0),
	CONT_WATER		= (1<<1),
	CONT_SLIME		= (1<<2),
	CONT_FOG		= (1<<3),
	CONT_TELEPORTER	= (1<<4),
	CONT_MOVER		= (1<<5),
	CONT_TRIGGER	= (1<<6),
	CONT_LAVA		= (1<<7),

	// THIS MUST BE LAST!
	CONT_START_USER = (1<<24)
} Contents;

// enumerations: SkeletonBone
//		BONE_TORSO - Torso bone
//		BONE_PELVIS - Pelvis bone
//		BONE_HEAD - Head bone
//		BONE_RIGHTARM - Right arm bone
//		BONE_LEFTARM - Left arm bone
//		BONE_RIGHTHAND - Right hand bone
//		BONE_LEFTHAND - Left hand bone
//		BONE_RIGHTLEG - Right leg bone
//		BONE_LEFTLEG - Left leg bone
//		BONE_RIGHTFOOT - Right foot bone
//		BONE_LEFTFOOT - Left foot bone
typedef enum eSkeletonBone
{
	BONE_TORSO,
	BONE_PELVIS,
	BONE_HEAD,
	BONE_RIGHTARM,
	BONE_LEFTARM,
	BONE_RIGHTHAND,
	BONE_LEFTHAND,
	BONE_RIGHTLEG,
	BONE_LEFTLEG,
	BONE_RIGHTFOOT,
	BONE_LEFTFOOT,

	// THIS MUST BE LAST!
	BONE_LAST_BONE = 1000
} SkeletonBone;

// enumerations: NavigationID
//		NAVID_WP - Waypoint-based path planning implementation.
//		NAVID_NAVMESH - Navigation mesh path planning implementation.
//		NAVID_AAS - Implementation of Quake 4 AAS system.
//		NAVID_HL2_NAVMESH - Implementation of HL2 Navigation Mesh.
typedef enum eNavigatorID
{
	NAVID_NONE,	
	NAVID_WP,	
	NAVID_NAVMESH,	
	NAVID_Q4_AAS,	
	NAVID_HL2_NAVMESH,

	// THIS MUST BE LAST!
	NAVID_MAX
} NavigatorID;

// class: BotTraceResult
//		This file defines all the common structures used by the game and bot alike.
typedef struct
{
	// float: m_Fraction
	//		0.0 - 1.0 how far the trace went
	float		m_Fraction;
	// float: m_Normal
	//		The plane normal that was struck
	float		m_Normal[3];
	// float: m_Endpos
	//		The end point the trace ended at
	float		m_Endpos[3];
	// var: m_HitEntity
	//		The entity that was hit by the trace
	GameEntity	m_HitEntity;
	// int: m_StartSolid
	//		Did the trace start inside a solid?
	int			m_StartSolid;
	// int: m_iUser1
	//		Extra user info from the trace
	int			m_iUser1;
	// int: m_iUser2
	//		Extra user info from the trace
	int			m_iUser2;
} BotTraceResult;

// enumerations: TraceMasks
//		TR_MASK_ALL - This trace should test against everything
//		TR_MASK_SOLID - This trace should test against only solids
//		TR_MASK_PLAYER - This trace should test against only players
//		TR_MASK_SHOT - This trace should test as a shot trace
//		TR_MASK_OPAQUE - This trace should test against opaque objects only
//		TR_MASK_WATER - This trace should test against water
//		TR_MASK_PLAYERCLIP - This trace should test against player clips
//		TR_MASK_SMOKEBOMB - This trace should test against player clips
typedef enum eTraceMasks
{
	TR_MASK_ALL			= (1<<0),
	TR_MASK_SOLID		= (1<<1),
	TR_MASK_PLAYER		= (1<<2),
	TR_MASK_SHOT		= (1<<3),
	TR_MASK_OPAQUE		= (1<<4),
	TR_MASK_WATER		= (1<<5),
	TR_MASK_PLAYERCLIP	= (1<<6),
	TR_MASK_SMOKEBOMB	= (1<<7),

	// THIS MUST BE LAST!
	TR_MASK_LAST		= (1<<16)
} TraceMasks;

// struct: ClientInput
//		Generic data structure representing the bots input and movement states
//		Game is responsible for translating this into a format suitable for use
//		by the game.
typedef struct
{
	// float: m_Facing
	//		The direction the bot is facing/aiming
	float	m_Facing[3];
	// float: m_MoveDir
	//		The direction the bot is moving
	float	m_MoveDir[3];
	// int: m_ButtonFlags
	//		32 bit int of bits representing bot keypresses, see <ButtonFlags>
	int		m_ButtonFlags;
	// int: m_CurrentWeapon
	//		The current weapon Id this bot wants to use.
	int		m_CurrentWeapon;
} ClientInput;

// struct: BotUserData
//		Generic data structure that uses a union to hold various types
//		of information.
typedef struct BotUserData_t
{
	// enum: DataType
	//		This allows a small level of type safety with the messages
	//		that send BotUserData parameters. It is a good idea to use the
	//		m_DataType parameter so signal which element of the union
	//		is currently being used.
	enum { dtNone = 0, dtVector, dtString, dtInt, dtFloat, dtEntity,
		dt3_4byteFlags, dt3_Strings, dt6_2byteFlags, dt12_1byteFlags } DataType;
	union udatatype
	{
		float			m_Vector[3];
		void *			m_VoidPtrs[3];
		char *			m_CharPtrs[3];
		const char *	m_String;
		int				m_Int;
		float			m_Float;
		GameEntity		m_Entity;
		int				m_4ByteFlags[3];
		short			m_2ByteFlags[6];
		char			m_1ByteFlags[12];
	} udata;
	// Easy Constructors for C++
#ifdef __cplusplus
	BotUserData_t() : DataType(dtNone) {};
	BotUserData_t(const char * _str) : DataType(dtString) { udata.m_String = _str; };
	BotUserData_t(int _int) : DataType(dtInt) { udata.m_Int = _int; };
	BotUserData_t(float _float) : DataType(dtFloat) { udata.m_Float = _float; };
	BotUserData_t(const GameEntity &_ent) : DataType(dtEntity) { udata.m_Entity = _ent; };
	BotUserData_t(float _x, float _y, float _z) : 
		DataType(dtVector) 
	{
		udata.m_Vector[0] = _x; 
		udata.m_Vector[1] = _y; 
		udata.m_Vector[2] = _z;
	};
	BotUserData_t(int _0, int _1, int _2) : DataType(dt3_4byteFlags)
	{
		udata.m_4ByteFlags[0] = _0; 
		udata.m_4ByteFlags[1] = _1; 
		udata.m_4ByteFlags[2] = _2;
	};
	BotUserData_t(char *_0, char *_1, char *_2) : DataType(dt3_Strings)
	{
		udata.m_CharPtrs[0] = _0; 
		udata.m_CharPtrs[1] = _1; 
		udata.m_CharPtrs[2] = _2;
	};
	BotUserData_t(short _0, short _1, short _2, short _3, short _4, short _5) : 
		DataType(dt6_2byteFlags)
	{
		udata.m_2ByteFlags[0] = _0; 
		udata.m_2ByteFlags[1] = _1; 
		udata.m_2ByteFlags[2] = _2;
		udata.m_2ByteFlags[3] = _3; 
		udata.m_2ByteFlags[4] = _4; 
		udata.m_2ByteFlags[5] = _5;
	};
	BotUserData_t(char _0, char _1, char _2, char _3, char _4, char _5, char _6, char _7, char _8, char _9, char _10, char _11) : 
		DataType(dt12_1byteFlags)
	{
		udata.m_1ByteFlags[0] = _0; 
		udata.m_1ByteFlags[1] = _1; 
		udata.m_1ByteFlags[2] = _2;
		udata.m_1ByteFlags[3] = _3; 
		udata.m_1ByteFlags[4] = _4; 
		udata.m_1ByteFlags[5] = _5;
		udata.m_1ByteFlags[6] = _6; 
		udata.m_1ByteFlags[7] = _7; 
		udata.m_1ByteFlags[8] = _8;
		udata.m_1ByteFlags[9] = _9; 
		udata.m_1ByteFlags[10] = _10; 
		udata.m_1ByteFlags[11] = _11;
	};
	// Function: IsNone
	// This <BotUserData> has no type specified
	inline bool IsNone() const { return (DataType == dtNone); };
	// Function: IsString
	// This <BotUserData> is a char * type
	inline bool IsString() const { return (DataType == dtString); };
	// Function: Is3String
	// This <BotUserData> is a array of 3 strings
	inline bool Is3String() const { return (DataType == dt3_Strings); };
	// Function: IsInt
	// This <BotUserData> is an int type
	inline bool IsInt() const { return (DataType == dtInt); };
	// Function: IsFloat
	// This <BotUserData> is an float type
	inline bool IsFloat() const { return (DataType == dtFloat); };
	// Function: IsFloatOrInt
	// This <BotUserData> is an float type or an int type
	inline bool IsFloatOrInt() const { return (DataType == dtFloat) || (DataType == dtInt); };
	// Function: IsEntity
	// This <BotUserData> is an <GameEntity> type
	inline bool IsEntity() const { return (DataType == dtEntity); };
	// Function: IsVector
	// This <BotUserData> is a 3d Vector type
	inline bool IsVector() const { return (DataType == dtVector); };
	// Function: Is3_4ByteFlags
	// This <BotUserData> is an array of 3 4-byte values
	inline bool Is3_4ByteFlags() const { return (DataType == dt3_4byteFlags); };
	// Function: Is6_2ByteFlags
	// This <BotUserData> is an array of 6 2-byte values
	inline bool Is6_2ByteFlags() const { return (DataType == dt6_2byteFlags); };
	// Function: Is12_1ByteFlags
	// This <BotUserData> is an array of 12 1-byte values
	inline bool Is12_1ByteFlags() const { return (DataType == dt12_1byteFlags); };

	inline const char *GetString() const { return udata.m_String; };
	inline int GetInt() const { return udata.m_Int; };
	inline float GetFloat() const { return udata.m_Float; };
	inline GameEntity GetEntity() const { return udata.m_Entity; };
	inline const char *GetStrings(int _index) const { return udata.m_CharPtrs[_index]; };
	inline const float *GetVector() const { return udata.m_Vector; };
	inline const int *Get4ByteFlags() const { return udata.m_4ByteFlags; };
	inline const short *Get2ByteFlags() const { return udata.m_2ByteFlags; };
	inline const char *Get1ByteFlags() const { return udata.m_1ByteFlags; };
	inline float GetNumAsFloat() const 
	{ 
		if(IsFloat()) 
			return GetFloat(); 
		if(IsInt()) 
			return (float)GetInt();
		return 0.0f;
	};
	inline int GetNumAsInt() const 
	{ 
		if(IsFloat()) 
			return (int)GetFloat(); 
		if(IsInt()) 
			return GetInt(); 
		return 0;
	};

	
#endif
} BotUserData;

// struct: TriggerInfo
typedef struct TriggerInfo_t
{
	// ptr: m_TagName
	//		The tagname of this trigger, usually a name given by the mapper.
	const char *m_TagName;
	// ptr: m_Action
	//		The name of the action this trigger is performing, mod specific
	const char *m_Action;
	// ptr: m_Entity
	//		The entity of this trigger, if available
	GameEntity	m_Entity;
	// ptr: m_Activator
	//		The entity that activated this trigger
	GameEntity	m_Activator;
	// ptr: m_UserData
	//		Extra info.
	BotUserData	m_UserData;
#ifdef __cplusplus
	TriggerInfo_t() : m_TagName(0), m_Action(0), m_Entity(0), m_Activator(0) {}
	TriggerInfo_t(const char *_name, const char *_action, GameEntity _ent, GameEntity _activator) : 
		m_TagName(_name), m_Action(_action), m_Entity(_ent), m_Activator(_activator) {}
#endif
} TriggerInfo;

// Generic Enumerations

// typedef: PlayerState
//		Enumerations for a players current game state.
typedef enum ePlayerState
{
	S_PLAYER_INVALID,				// Player doesn't exist
	S_PLAYER_SPECTATOR,			// Player is in spectator mode.
	S_PLAYED_WAITING_TEAM,		// Player waiting on team selection.
	S_PLAYED_WAITING_CLASS,		// Player waiting on class selection.
	S_PLAYED_WAITING_NEXTROUND,	// Player waiting on on the next round. Died or something.
	S_PLAYER_PLAYING				// Player is good to go, and fully joined.
} PlayerState;

// typedef: FlagState
//		Enumerations for the status of a flag type entity.
typedef enum eFlagState
{
	S_FLAG_AT_BASE,
	S_FLAG_DROPPED,
	S_FLAG_CARRIED
} FlagState;

typedef enum eGameState
{
	GAME_STATE_INVALID,
	GAME_STATE_PLAYING,
	GAME_STATE_WARMUP,
	GAME_STATE_WARMUP_COUNTDOWN,
	GAME_STATE_INTERMISSION,
	GAME_STATE_WAITINGFORPLAYERS,
	GAME_STATE_PAUSED,
} GameState;

#endif
