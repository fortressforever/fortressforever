////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-07-20 08:59:11 -0700 (Thu, 20 Jul 2006) $
// $LastChangedRevision: 1234 $
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
		GAME_ID_ENDROUND,
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
			PERCEPT_FEEL_PLAYER_USE,
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

// enumerations: GameMessage
//		GEN_MSG_NONE - Invalid message reserved as 0.
//		GEN_MSG_ISALIVE - Is the entity alive?
//		GEN_MSG_ISRELOADING - Is the entity reloading?
//		GEN_MSG_ISREADYTOFIRE - Is the entity ready to fire?
//		GEN_MSG_ISALLIED - Is the entity allied with another?
//		GEN_MSG_ISHUMAN - Is the entity a human player?
//		GEN_MSG_GETPOINTCONTENTS - Get the point content id for a location.
//		GEN_MSG_GETEQUIPPEDWEAPON - Get the currently equipped weapon id for an entity.
//		GEN_MSG_GETHEALTHARMOR - Get health and armor for an entity.
//		GEN_MSG_GETMAXSPEED - Get the max speed of the entity.
//		GEN_MSG_GETFLAGSTATE - Get the current state of the flag.
//		GEN_MSG_GAMESTATE - Get the current state of the game.
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
	GEN_MSG_GETHEALTHARMOR,
	GEN_MSG_GETMAXSPEED,
	GEN_MSG_GETFLAGSTATE,
	GEN_MSG_GAMESTATE,

	// This must stay last.
	GEN_MSG_END
} GEN_GameMessage;

// enumerations: BlackBoard_Key
//		bbk_All - Special identifier for ALL keys.
//		bbk_DelayGoal - Goal delayed for the duration of this blackboard entry.
typedef enum
{
	bbk_All = 0,
	bbk_DelayGoal,

	// This must stay last.
	bbk_LastKey,
} BlackBoard_Key;

#endif
