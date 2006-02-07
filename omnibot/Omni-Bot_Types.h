////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-01-28 12:33:58 -0500 (Sat, 28 Jan 2006) $
// $LastChangedRevision: 1137 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __OMNIBOT_TYPES_H__
#define __OMNIBOT_TYPES_H__

#include "Omni-Bot_BasicTypes.h"

// Errors
typedef enum
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

// typedef: WaypointFlags
//		This is the type waypoint flags, and should be a 64 bit type
typedef obuint64 NavigationFlags;

// enum: obBool
//		Since theres no promise that we're running in C or C++, and therefor no
//		guarantee that bools are supported, lets use an enumerated value instead.
typedef enum
{
	obfalse,
	obtrue
} obBool;

typedef enum
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
	// NOTE: This must be last!
	NUM_BOT_DEBUG_FLAGS = (1<<16)
} BotDebugFlag;

// enum: Helpers
//		A couple enum values for readable values for special case class identifiers
//		Used for the client ChangeClass and ChangeTeam
typedef enum
{
	RANDOM_CLASS				= -1,
	RANDOM_CLASS_IF_NO_CLASS	= -2,
	RANDOM_TEAM					= -1,
	RANDOM_TEAM_IF_NO_TEAM		= -2,
} Helpers;

// enum: obLineTypes
//		For identifying the types of lines the interface should draw. Some are treated differently based on type.
typedef enum
{
	LINE_NONE,
	LINE_NORMAL,
    LINE_WAYPOINT,
	LINE_PATH,
	LINE_BLOCKABLE,
} obLineType;

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

// package: ButtonFlags
//		This enum defines the button press states available
typedef enum eButtonFlags
{
	BOT_BUTTON_NONE		= 0,
	// bit: BOT_BUTTON_ATTACK1
	//		If the bot is pressing primary attack
	BOT_BUTTON_ATTACK1	= (1<<0),
	// bit: BOT_BUTTON_ATTACK2
	//		If the bot is pressing secondary attack
	BOT_BUTTON_ATTACK2	= (1<<1),
	// bit: BOT_BUTTON_JUMP
	//		If the bot is pressing jump
	BOT_BUTTON_JUMP		= (1<<2),
	// bit: BOT_BUTTON_CROUCH
	//		If the bot is pressing crouch
	BOT_BUTTON_CROUCH	= (1<<3),
	// bit: BOT_BUTTON_PRONE
	//		If the bot is pressing prone
	BOT_BUTTON_PRONE	= (1<<4),
	// bit: BOT_BUTTON_WALK
	//		If the bot is pressing walk
	BOT_BUTTON_WALK		= (1<<5),
	// bit: BOT_BUTTON_USE
	//		If the bot is pressing use
	BOT_BUTTON_USE		= (1<<6),
	// bit: BOT_BUTTON_RSTRAFE
	//		If the bot is pressing right strafe
	BOT_BUTTON_RSTRAFE  = (1<<7),
	// bit: BOT_BUTTON_LSTRAFE
	//		If the bot is pressing left strafe
	BOT_BUTTON_LSTRAFE  = (1<<8),
	// bit: BOT_BUTTON_RELOAD
	//		If the bot is pressing reload
	BOT_BUTTON_RELOAD	= (1<<9),
	// bit: BOT_BUTTON_SPRINT
	//		If the bot wants to sprint
	BOT_BUTTON_SPRINT	= (1<<10),
	// bit: BOT_BUTTON_DROP
	//		If the bot wants to drop current item
	BOT_BUTTON_DROP		= (1<<11),
	// bit: BOT_BUTTON_LEANLEFT
	//		If the bot wants to lean left
	BOT_BUTTON_LEANLEFT	= (1<<12),
	// bit: BOT_BUTTON_LEANRIGHT
	//		If the bot wants to lean right
	BOT_BUTTON_LEANRIGHT= (1<<13),
	// bit: BOT_BUTTON_AIM
	//		If the bot wants to drop current item
	BOT_BUTTON_AIM		= (1<<14),

	BOT_BUTTUN_FIRSTUSER= (1<<20)
} ButtonFlags;

// package: GoalType
//		This enum defines the identifiers for goal entity types the game can register with the bot.
typedef enum eGoalType
{
	GOAL_NONE = 0,
	// var: GOAL_AMMO
	//		Ammunition can be located at this goal
	GOAL_AMMO,
	// var: GOAL_ARMOR
	//		Armor can be located at this goal
	GOAL_ARMOR,
	// var: GOAL_HEALTH
	//		Health can be located at this goal
	GOAL_HEALTH,
	// var: GOAL_GOTO
	//		The bot should just go to this goal
	GOAL_GOTO,
	// var: GOAL_DEFEND
	//		The bot should just go to this goal and watch for targets.
	GOAL_DEFEND,
	// var: GOAL_ATTACK
	//		The bot should just go to this goal and hunt for targets.
	GOAL_ATTACK,
	// var: GOAL_SNIPE
	//		The bot should snipe from this point.
	GOAL_SNIPE,
	// var: GOAL_CTF_FLAG
	//		The bot should attempt to grab this flag and return it to a capture point.
	GOAL_CTF_FLAG,
	// var: GOAL_CTF_RETURN_FLAG
	//		The bot should attempt to touch this flag to 'return' it.
	GOAL_CTF_RETURN_FLAG,
	// var: GOAL_CTF_FLAGCAP
	//		The bot should take GOAL_CTF_FLAG to this point.
	GOAL_CTF_FLAGCAP,
	// This must stay last!
	BASE_GOAL_NUM
} GoalType;

// A basic list of goals. There should be one of these for any base Goal class.
typedef enum eETFGoals
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
	goal_base_num
} Goals_Base;

// package: EntityFlags
//		Possible flags that can go on an entity for special behavior/treatment
typedef enum eEntityFlags
{
	// bit: ENT_FLAG_TEAM1
	// bit: ENT_FLAG_TEAM2
	// bit: ENT_FLAG_TEAM3
	// bit: ENT_FLAG_TEAM4
	//		This entity is only available/visible for a certain team.
	ENT_FLAG_TEAM1		= (1<<0),
	ENT_FLAG_TEAM2		= (1<<1),
	ENT_FLAG_TEAM3		= (1<<2),
	ENT_FLAG_TEAM4		= (1<<3),

	// bit: ENT_FLAG_DISABLED
	//		This entity is disabled
	ENT_FLAG_DISABLED	= (1<<4),
	// bit: ENT_FLAG_PRONED
	//		This entity is prone
	ENT_FLAG_PRONED		= (1<<5),
	// bit: ENT_FLAG_CROUCHED
	//		This entity is crouched
	ENT_FLAG_CROUCHED	= (1<<6),
	// bit: ENT_FLAG_CARRYABLE
	//		This entity is carryable(flag, powerup,...)
    ENT_FLAG_CARRYABLE	= (1<<7),
	// bit: ENT_FLAG_DEAD
	//		This entity is dead
	ENT_FLAG_DEAD		= (1<<8),
	// bit: ENT_FLAG_INWATER
	//		This entity is in water
	ENT_FLAG_INWATER	= (1<<9),
	// bit: ENT_FLAG_UNDERWATER
	//		This entity is under water
	ENT_FLAG_UNDERWATER	= (1<<10),
	// bit: ENT_FLAG_ZOOMING
	//		This entity is zooming through scope or binoculars.
	ENT_FLAG_ZOOMING	= (1<<11),
	// bit: ENT_FLAG_LADDER
	//		This entity is on a ladder.
	ENT_FLAG_LADDER		= (1<<11),

	// THIS MUST BE LAST
	ENT_FLAG_FIRST_USER	= (1<<16)
} EntityFlags;

// package: EntityCategory
//		Category flags that categorize this entity type for sensory queries.
typedef enum eEntityCategory
{
	// bit: ENT_CAT_PLAYER
	//		This entity is a player of some sort
	ENT_CAT_PLAYER		= (1<<0),
	// bit: ENT_CAT_PROJECTILE
	//		This entity is a projectile of some sort
	ENT_CAT_PROJECTILE	= (1<<1),
	// bit: ENT_CAT_SHOOTABLE
	//		This entity is shootable
	ENT_CAT_SHOOTABLE	= (1<<2),
	// bit: ENT_CAT_PICKUP
	//		This entity is a pickup/powerup of some sort
	ENT_CAT_PICKUP		= (1<<3),
	// bit: ENT_CAT_TRIGGER
	//		This entity is a trigger of some sort
	ENT_CAT_TRIGGER		= (1<<4),
	// bit: ENT_CAT_MOVER
	//		This entity is a mover of some sort(lift, door,...)
	ENT_CAT_MOVER		= (1<<5),
	// bit: ENT_CAT_AVOID
	//		This entity is something bots should avoid
	ENT_CAT_AVOID		= (1<<6),
	// bit: ENT_CAT_MOUNTEDWEAPON
	//		This entity is something bots can mount and use
	ENT_CAT_MOUNTEDWEAPON= (1<<7),
	// bit: ENT_CAT_MISC
	//		Miscellaneous entity category.
	ENT_CAT_MISC		= (1<<8),
	// bit: ENT_CAT_STATIC
	//		Static entities don't need to be seen. This allows the bot to skip LOS checks.
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

// package: SoundType
//		This enum categorizes generic sound types for the bot. Mods can extend this
//		if they want with more specific sound ids or more categories.
typedef enum eSoundType
{
	SND_NONE,

	// int: SND_JUMP
	//		Sound of jump from another entity
	SND_JUMP,
	// int: SND_FOOTSTEP
	//		Sound of footstep from another entity
	SND_FOOTSTEP,
	// int: SND_TAKEDAMAGE
	//		Sound of another entity taking damage
	SND_TAKEDAMAGE,
	// int: SND_POWERUP_SPAWN
	//		Sound of a powerup respawning
	SND_POWERUP_SPAWN,
	// int: SND_POWERUP_PICKUP
	//		Sound of a powerup being picked up
	SND_POWERUP_PICKUP,
	// int: SND_WEAPON_FIRE
	//		Sound of a weapon firing
	SND_WEAPON_FIRE,
	// int: SND_WEAPON_RELOAD
	//		Sound of a weapon reloading
	SND_WEAPON_RELOAD,
	// int: SND_WEAPON_EMPTY
	//		Sound of a weapon empty
	SND_WEAPON_EMPTY,
	// int: SND_WEAPON_STARTFIRE
	//		Sound of a weapon starting to fire
	SND_WEAPON_STARTFIRE,
	// int: SND_VOICE_TAUNT
	//		Sound of a voice taunt
	SND_VOICE_TAUNT,
	// int: SND_VOICE_TEAM
	//		Sound of a voice team message
	SND_VOICE_TEAM,
	// int: SND_VOICE_ENEMY
	//		Sound of a voice enemy message
	SND_VOICE_ENEMY,

	// THIS MUST BE LAST!
	SND_MAX_SOUNDS
} SoundType;

// package: Contents
//		This enum defines contents representing special properties of a place
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

// package: NavigationID
//		This enum defines the available navigation systems that are usable for path planning.
typedef enum eNavigatorID
{
	NAVID_NONE,
	// int: NAVID_WP
	//		Waypoint-based path planning implementation
	NAVID_WP,
	// int: NAVID_AAS
	//		Implementation of Quake 4 AAS system
	NAVID_Q4_AAS,
	// int: NAVID_HL2_NAVMESH
	//		Implementation of HL2 Navigation Mesh
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

// package: TraceMasks
//		This enum defines the masks used for tracelines.
typedef enum eTraceMasks
{
	// enum: TR_MASK_ALL
	//		This trace should test against everything
	TR_MASK_ALL			= (1<<0),
	// enum: TR_MASK_SOLID
	//		This trace should test against only solids
	TR_MASK_SOLID		= (1<<1),
	// enum: TR_MASK_PLAYER
	//		This trace should test against only players
	TR_MASK_PLAYER		= (1<<2),
	// const: TR_MASK_SHOT
	//		This trace should test as a shot trace
	TR_MASK_SHOT		= (1<<3),
	// const: TR_MASK_OPAQUE
	//		This trace should test against opaque objects only
	TR_MASK_OPAQUE		= (1<<4),
	// const: TR_MASK_WATER
	//		This trace should test against water
	TR_MASK_WATER		= (1<<5),
	// const: TR_MASK_PLAYERCLIP
	//		This trace should test against player clips
	TR_MASK_PLAYERCLIP	= (1<<6),

	// THIS MUST BE LAST!
	TR_MASK_LAST = (1<<16)
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
	int					m_DataType;
	// Easy Constructors for C++
#ifdef __cplusplus
	BotUserData_t() : m_DataType(dtNone) {};
	BotUserData_t(const char * _str) : m_DataType(dtString) { udata.m_String = _str; };
	BotUserData_t(int _int) : m_DataType(dtInt) { udata.m_Int = _int; };
	BotUserData_t(float _float) : m_DataType(dtFloat) { udata.m_Float = _float; };
	BotUserData_t(const GameEntity &_ent) : m_DataType(dtEntity) { udata.m_Entity = _ent; };
	BotUserData_t(float _x, float _y, float _z) : 
		m_DataType(dtVector) 
	{
		udata.m_Vector[0] = _x; 
		udata.m_Vector[1] = _y; 
		udata.m_Vector[2] = _z;
	};
	BotUserData_t(int _0, int _1, int _2) : m_DataType(dt3_4byteFlags)
	{
		udata.m_4ByteFlags[0] = _0; 
		udata.m_4ByteFlags[1] = _1; 
		udata.m_4ByteFlags[2] = _2;
	};
	BotUserData_t(char *_0, char *_1, char *_2) : m_DataType(dt3_Strings)
	{
		udata.m_CharPtrs[0] = _0; 
		udata.m_CharPtrs[1] = _1; 
		udata.m_CharPtrs[2] = _2;
	};
	BotUserData_t(short _0, short _1, short _2, short _3, short _4, short _5) : 
		m_DataType(dt6_2byteFlags)
	{
		udata.m_2ByteFlags[0] = _0; 
		udata.m_2ByteFlags[1] = _1; 
		udata.m_2ByteFlags[2] = _2;
		udata.m_2ByteFlags[3] = _3; 
		udata.m_2ByteFlags[4] = _4; 
		udata.m_2ByteFlags[5] = _5;
	};
	BotUserData_t(char _0, char _1, char _2, char _3, char _4, char _5, char _6, char _7, char _8, char _9, char _10, char _11) : 
		m_DataType(dt12_1byteFlags)
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
	inline bool IsNone() const { return (m_DataType == dtNone); };
	// Function: IsString
	// This <BotUserData> is a char * type
	inline bool IsString() const { return (m_DataType == dtString); };
	// Function: Is3String
	// This <BotUserData> is a array of 3 strings
	inline bool Is3String() const { return (m_DataType == dt3_Strings); };
	// Function: IsInt
	// This <BotUserData> is an int type
	inline bool IsInt() const { return (m_DataType == dtInt); };
	// Function: IsFloat
	// This <BotUserData> is an float type
	inline bool IsFloat() const { return (m_DataType == dtFloat); };
	// Function: IsFloatOrInt
	// This <BotUserData> is an float type or an int type
	inline bool IsFloatOrInt() const { return (m_DataType == dtFloat) || (m_DataType == dtInt); };
	// Function: IsEntity
	// This <BotUserData> is an <GameEntity> type
	inline bool IsEntity() const { return (m_DataType == dtEntity); };
	// Function: IsVector
	// This <BotUserData> is a 3d Vector type
	inline bool IsVector() const { return (m_DataType == dtVector); };
	// Function: Is3_4ByteFlags
	// This <BotUserData> is an array of 3 4-byte values
	inline bool Is3_4ByteFlags() const { return (m_DataType == dt3_4byteFlags); };
	// Function: Is6_2ByteFlags
	// This <BotUserData> is an array of 6 2-byte values
	inline bool Is6_2ByteFlags() const { return (m_DataType == dt6_2byteFlags); };
	// Function: Is12_1ByteFlags
	// This <BotUserData> is an array of 12 1-byte values
	inline bool Is12_1ByteFlags() const { return (m_DataType == dt12_1byteFlags); };

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

// struct: EntityInfo
//		Used to store information about an entity
typedef struct 
{
	// int: m_EntityClass
	//		The specific classification of this entity
	int			m_EntityClass;
	// int: m_EntityFlags
	//		Current flags of this entity, see <EntityFlags>
	int			m_EntityFlags;
	// int: m_EntityCategoty
	//		Current category of this entity, see <EntityCategory>
	int			m_EntityCategory;
	// var: m_UserData
	//		Additional Info
	BotUserData	m_UserData;
} EntityInfo;

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


#endif
