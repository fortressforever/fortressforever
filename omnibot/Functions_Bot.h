////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2005-12-25 23:49:22 -0500 (Sun, 25 Dec 2005) $
// $LastChangedRevision: 1092 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __FUNCTIONS_BOT_H__
#define __FUNCTIONS_BOT_H__

#include "Functions_Nav.h"

#include "Omni-Bot.h"
#include "Omni-Bot_Types.h"
#include "Omni-Bot_Events.h"
#include "Functions_Engine.h"

// Title: Functions Bot

// typedef: Bot_EngineFuncs_t
//		This struct defines all the function pointers that the bot will fill in 
//		and give to the interface so that the interface can request the entire
//		suite of functions at once from the bot.
typedef struct 
{	
	omnibot_error (*pfnBotInitialise)(const NavigatorID _navid, const Game_EngineFuncs_t *_pEngineFuncs, int _version);
	void (*pfnBotUpdate)();
	void (*pfnBotShutdown)();
	int (*pfnBotConsoleCommand)(const char *_cmd, int _size);
	void (*pfnBotSendEvent)(int _eid, int _dest, int _source, int _msdelay, BotUserData *_data);
	void (*pfnBotSendGlobalEvent)(int _eid, int _source, int _msdelay, BotUserData *_data);
	void (*pfnBotLog)(const char *_txt);
	void (*pfnBotAddGoal)(const GameEntity _ent, int _goaltype, int _team, const char *_tag, BotUserData *_bud);
	void (*pfnBotSendTrigger)(TriggerInfo *_triggerInfo);
	void (*pfnBotAddBBRecord)(BlackBoard_Key _type, int _posterID, int _targetID, BotUserData *_data);
	void (*pfnBotAddThreatEntity)(GameEntity _ent, EntityInfo *_info);
	int (*pfnBotSetNavFuncs)(Nav_EngineFuncs_t *_pNavFuncs, int _size);
} Bot_EngineFuncs_t;

#endif

