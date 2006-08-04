////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-08-03 00:47:21 -0700 (Thu, 03 Aug 2006) $
// $LastChangedRevision: 1241 $
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __FUNCTIONS_BOT_H__
#define __FUNCTIONS_BOT_H__

#include "Functions_Nav.h"

#include "Omni-Bot.h"
#include "Omni-Bot_Types.h"
#include "Omni-Bot_Events.h"
#include "Functions_Engine.h"
#include "MessageHelper.h"

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
	obint32 (*pfnBotAddGoal)(const GameEntity _ent, int _goaltype, int _team, const char *_tag, BotUserData *_bud);
	void (*pfnBotSendTrigger)(TriggerInfo *_triggerInfo);
	void (*pfnBotAddBBRecord)(BlackBoard_Key _type, int _posterID, int _targetID, BotUserData *_data);
	void (*pfnBotAddThreatEntity)(GameEntity _ent, EntityInfo *_info);
	int (*pfnBotSetNavFuncs)(Nav_EngineFuncs_t *_pNavFuncs, int _size);

	// New message stuff.
	/*SubscriberHandle (*pfnSubscribeToMsg)(int _msg, pfnMessageFunction _func);
	void (*pfnUnsubscribe)(const SubscriberHandle _handle);

	MessageHelper (*pfnBeginMessage)(int _msgId, obuint32 _messageSize);
	MessageHelper (*pfnBeginMessageEx)(int _msgId, void *_mem, obuint32 _messageSize);
	void (*pfnEndMessage)(const MessageHelper &_helper);
	void (*pfnEndMessageEx)(const MessageHelper &_helper);*/

} Bot_EngineFuncs_t;

#endif

