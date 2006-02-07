////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2005-12-25 23:49:22 -0500 (Sun, 25 Dec 2005) $
// $LastChangedRevision: 1092 $
//
// about: Exported function definitions
//		In order for the game to call functions from the bot, we must export
//		the functions to the game itself and allow it to call them. 
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __OMNIBOT_H__
#define __OMNIBOT_H__

#include "Functions_Bot.h"
#include "Functions_Nav.h"
#include "Functions_Engine.h"
#include "Omni-Bot_Types.h"
#include "Omni-Bot_Events.h"

// function: Bot_Initialise
//		Initializes the bot library and sets the bot up with the callbacks to 
//		the game in the form of function pointers to functions within the game.
omnibot_error BotInitialise(const NavigatorID _navid, const Game_EngineFuncs_t *_pEngineFuncs, int _version);
// function: Bot_Shutdown
//		Shuts down and frees memory from the bot system
void BotShutdown();
// function: Bot_Update
//		Called regularly by the game in order for the bots to perform their "thinking"
void BotUpdate();
// function: Bot_ConsoleCommand
//		Any time commands from the game are executed, this will get called
//		to allow the bot to process it and perform any necessary actions.
int BotConsoleCommand(const char *_cmd, int _size);
// fucntion: Bot_Log
//		Allows the game to print text messages into the bots logs or other output.
void BotLog(const char *_txt);
// function: Bot_AddGoal
//		Allows the game to register a goal with the bot that the bots can use
void BotAddGoal(const GameEntity _ent, int _goaltype, int _team, const char *_tag, BotUserData *_bud);
// function: Bot_AddBBRecord
//		Allows the game to enter blackboard records into the bots knowledge database.
void BotAddBBRecord(BlackBoard_Key _type, int _posterID, int _targetID, BotUserData *_data);
// function: Bot_AddTargetEntity
//		This adds the provided entity to the bots threat list. 
//		This could be other bots/clients/projectiles
void BotAddThreatEntity(GameEntity _ent, EntityInfo *_info);
// function: Bot_SendEvent
//		Allows the game to send generic events to the bots to signal game events
void BotSendEvent(int _eid, int _dest, int _source, int _msdelay, BotUserData *_data);
// function: Bot_SendGlobalEvent
//		Allows the game to send generic events to everyone
void BotSendGlobalEvent(int _eid, int _source, int _msdelay, BotUserData *_data);
// function: Bot_SendTrigger
//		Allows the game to notify the bot of triggered events.
void BotSendTrigger(TriggerInfo *_triggerInfo);
// function: BotSetNavFuncs
//		Allows the game to give the bot functions to call for an external navigation system.
int BotSetNavFuncs(Nav_EngineFuncs_t *_pNavFuncs, int _size);

#endif
