////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-05-08 21:41:37 -0400 (Mon, 08 May 2006) $
// $LastChangedRevision: 1209 $
//
// about: Generic Bot Events
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __OMNIBOT_EVENTS_H__
#define __OMNIBOT_EVENTS_H__

// typedef: EventId
//		Readable identifier for various events that can be sent to the bot
//		and considered for state changes or behavioral modifications.
typedef enum 
{
	EVENT_ID_UNDEFINED = 0,

	SYSTEM_ID_FIRST,
		SYSTEM_INIT,
		SYSTEM_UPDATE,
		SYSTEM_SHUTDOWN,
		SYSTEM_THREAD_CREATED,
		SYSTEM_THREAD_DESTROYED,
	SYSTEM_ID_LAST,

	GAME_ID_FIRST,
		GAME_ID_STARTGAME,
		GAME_ID_ENDGAME,
		GAME_ID_NEWROUND,
		GAME_ID_BOTCONNECTED,
		GAME_ID_BOTDISCONNECTED,
		GAME_ID_CLIENTCONNECTED,
		GAME_ID_CLIENTDISCONNECTED,
	GAME_ID_LAST,

	EVENT_ID_FIRST,
		// Actions
		ACTION_ID_FIRST,
			ACTION_WEAPON_FIRE,
			ACTION_WEAPON_CHANGE,
		ACTION_ID_LAST,

		GOAL_ID_FIRST,
			GOAL_SUCCESS,
			GOAL_FAILED,
			GOAL_ABORTED,
		GOAL_ID_LAST,

		// Messages that are passed around between any objects
		MESSAGE_ID_FIRST,
			MESSAGE_SPAWN,
			MESSAGE_CHANGETEAM,
			MESSAGE_INVALIDTEAM,
			MESSAGE_INVALIDCLASS,
			MESSAGE_CHANGECLASS,
			MESSAGE_NEEDITEM,
			MESSAGE_TRIGGER,		// passed to a "behavior" to indicate it's sensor has triggered
			MESSAGE_DAMAGE,			// inform the object that it has to take damage
			MESSAGE_DEATH,
			MESSAGE_HEAL,
			MESSAGE_CHAT,			// used to send text msg between objects
			MESSAGE_POWERUP,		// power up was picked up
			MESSAGE_ADDWEAPON,		// gives a weapon to the bot, should add to list to be evaluated for use
			MESSAGE_RESETWEAPONS,	// tells the bot to clear out all the weapons
			MESSAGE_SPECTATED,
			MESSAGE_KILLEDSOMEONE,
		MESSAGE_ID_LAST,

		COMMAND_ID_FIRST,
			COMMAND_GOTO,
			COMMAND_DEFEND,			
			COMMAND_ATTACK,
		COMMAND_ID_LAST,

		// Percepts  (senses: feel, see, hear, smell, )
		PERCEPT_ID_FIRST,
			PERCEPT_FEEL_TOUCH_PLAYER,
			PERCEPT_FEEL_PAIN,
			PERCEPT_SEE_ENEMY_FOOT_PRINT,
			PERCEPT_HEAR_SOUND,
			PERCEPT_HEAR_GLOBALVOICEMACRO,
			PERCEPT_HEAR_TEAMVOICEMACRO,
			PERCEPT_HEAR_PRIVATEVOICEMACRO,
			PERCEPT_HEAR_GLOBALCHATMSG,
			PERCEPT_HEAR_TEAMCHATMSG,
			PERCEPT_HEAR_PRIVCHATMSG,
		PERCEPT_ID_LAST,
	EVENT_ID_LAST,
	EVENT_NUM_EVENTS
} EventId;

////////////////////////////////////////////////////////////////////////

// enum: GameMessage
//		General game messages common to all bots/mods
typedef enum
{
	GEN_MSG_NONE = 0,
	GEN_MSG_ISALIVE,
	GEN_MSG_ISRELOADING,
	GEN_MSG_ISREADYTOFIRE,
	GEN_MSG_ISALLIED,
	GEN_MSG_ISHUMAN,

	GEN_MSG_GETPOINTCONTENTS,
	GEN_MSG_GETEQUIPPEDWEAPON,
	GEN_MSG_GETCURRENTCLASS,
	GEN_MSG_GETCURRENTTEAM,
	GEN_MSG_GETHEALTHARMOR,
	GEN_MSG_GETFLAGSTATE,
	GEN_MSG_GAMESTATE,

	GEN_MSG_END
} GEN_GameMessage;

// enum: GEN_EntityProperty
//		General entity properties
typedef enum
{
	PROP_CLASSNAME = 0,
	PROP_TARGETNAME,
	
	PROPS_END
} GEN_EntityProperty;

// enum: BlackBoard_Key
//		Human readable identifiers for blackboard entries.
typedef enum
{
	bbk_All = 0,
	bbk_DelayGoal,

	// This must stay last.
	bbk_LastKey,
} BlackBoard_Key;

#endif
