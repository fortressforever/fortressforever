////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2007-06-07 09:51:52 -0700 (Thu, 07 Jun 2007) $
// $LastChangedRevision: 1973 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __FUNCTIONS_BOT_H__
#define __FUNCTIONS_BOT_H__

#include "Omni-Bot.h"
#include "Omni-Bot_Types.h"
#include "Omni-Bot_Events.h"
#include "MessageHelper.h"
#include "IEngineInterface.h"

// Title: Functions Bot

// typedef: Bot_EngineFuncs_t
//		This struct defines all the function pointers that the bot will fill in 
//		and give to the interface so that the interface can request the entire
//		suite of functions at once from the bot.
typedef struct 
{	
	omnibot_error (*pfnBotInitialise)(IEngineInterface *_pEngineFuncs, int _version);
	void (*pfnBotUpdate)();
	void (*pfnBotShutdown)();
	int (*pfnBotConsoleCommand)(const char *_cmd, int _size);
	void (*pfnBotAddGoal)(const GameEntity _ent, int _goaltype, int _team, const char *_tag, obUserData *_bud);
	void (*pfnBotSendTrigger)(TriggerInfo *_triggerInfo);
	void (*pfnBotAddBBRecord)(BlackBoard_Key _type, int _posterID, int _targetID, obUserData *_data);
	
	void (*pfnBotEntityAdded)(GameEntity _ent, EntityInfo *_info);
	
	// New message stuff.
	void (*pfnBotSendEvent)(int _dest, const MessageHelper &_message);
	void (*pfnBotSendGlobalEvent)(const MessageHelper &_message);
} Bot_EngineFuncs_t;

#endif

