#include "cbase.h"
#include "ai_basenpc.h"

#include "omnibot_interface.h"
#include "omnibot_eventhandler.h"
#include "ff_utils.h"

#include <filesystem.h>

KeyValues *g_pGameEvents = NULL;
KeyValues *g_pEngineEvents = NULL;
KeyValues *g_pModEvents = NULL;

ConVar g_BotShowEvents( "omnibot_showevents", "", 0, "Echo Recieved Events to console." );

using namespace Omnibot;

omnibot_eventhandler *g_pEventHandler = NULL;

typedef void (*pfnEventCallback)(IGameEvent *_event);

typedef struct  
{
	const char *		m_EventName;
	pfnEventCallback	m_Callback;
} EventCallback;

void event_Chat(IGameEvent *_event);
void event_Class(IGameEvent *_event);
void event_Team(IGameEvent *_event);
void event_Hurt(IGameEvent *_event);
void event_Death(IGameEvent *_event);
void event_Spawn(IGameEvent *_event);
void event_Connect(IGameEvent *_event);
void event_Connect(IGameEvent *_event);
void event_Disconnect(IGameEvent *_event);
void event_SentryBuilt(IGameEvent *_event);
void event_DispenserBuilt(IGameEvent *_event);
void event_DetpackBuilt(IGameEvent *_event);
void event_DetpackDetonated(IGameEvent *_event);
void event_SentryKilled(IGameEvent *_event);
void event_DispenserKilled(IGameEvent *_event);
void event_SentryUpgraded(IGameEvent *_event);
void event_DisguiseFinished(IGameEvent *_event);
void event_DisguiseLost(IGameEvent *_event);
void event_SpyUnFeigned(IGameEvent *_event);
void event_SpyFeigned(IGameEvent *_event);
void event_DispenserEnemyUsed(IGameEvent *_event);
void event_DispenserSabotaged(IGameEvent *_event);
void event_SentrySabotaged(IGameEvent *_event);
void event_DispenserDetonated(IGameEvent *_event);
void event_DispenserDismantled(IGameEvent *_event);
void event_SentryDetonated(IGameEvent *_event);
void event_SentryDismantled(IGameEvent *_event);

const EventCallback EVENT_CALLBACKS[] =
{
	{ "player_chat", event_Chat },
	{ "player_changeclass", event_Class },
	{ "player_team", event_Team },
	{ "player_hurt", event_Hurt },
	{ "player_death", event_Death },
	{ "player_spawn", event_Spawn },
	{ "player_connect", event_Connect },
	{ "player_disconnect", event_Disconnect },
	{ "build_dispenser", event_DispenserBuilt },
	{ "build_sentrygun", event_SentryBuilt },
	{ "build_detpack", event_DetpackBuilt },
	{ "detpack_detonated", event_DetpackDetonated },	
	{ "sentrygun_killed", event_SentryKilled },
	{ "dispenser_killed", event_DispenserKilled },
	{ "sentrygun_upgraded", event_SentryUpgraded },

	{ "disguised", event_DisguiseFinished },
	{ "disguise_lost", event_DisguiseLost },
	{ "unfeigned", event_SpyUnFeigned },
	{ "feigned", event_SpyFeigned },
	{ "dispenser_enemyused", event_DispenserEnemyUsed },		
	{ "dispenser_detonated", event_DispenserDetonated },
	{ "dispenser_dismantled", event_DispenserDismantled },
	{ "sentry_detonated", event_SentryDetonated },
	{ "sentry_dismantled", event_SentryDismantled },
	{ "dispenser_sabotaged", event_DispenserSabotaged },
	{ "sentry_sabotaged", event_SentrySabotaged },

	// sabotaged?
};

//---------------------------------------------------------------------------------
// Purpose: called when an event is fired
//---------------------------------------------------------------------------------

void omnibot_eventhandler::RegisterEvents()
{
	int iNumCallbacks = sizeof(EVENT_CALLBACKS) / sizeof(EVENT_CALLBACKS[0]);
	for(int i = 0; i < iNumCallbacks; ++i)
	{
		gameeventmanager->AddListener(this, EVENT_CALLBACKS[i].m_EventName, true);
	}
}

void omnibot_eventhandler::UnRegisterEvents()
{
	gameeventmanager->RemoveListener(this);
}

void omnibot_eventhandler::FireGameEvent( IGameEvent *_event )
{
	// do we want to display game events on the fly ?
	if(g_BotShowEvents.GetBool())
		PrintEvent(_event);

	// Dispatch game events as bot messages where needed
	if(_event)
	{
		int iNumCallbacks = sizeof(EVENT_CALLBACKS) / sizeof(EVENT_CALLBACKS[0]);
		for(int i = 0; i < iNumCallbacks; ++i)
		{
			if(!Q_strcmp(_event->GetName(), EVENT_CALLBACKS[i].m_EventName))
			{
				EVENT_CALLBACKS[i].m_Callback(_event);
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////

void omnibot_eventhandler::ExtractEvents()
{
	// read the game events
	g_pGameEvents = new KeyValues("GameEventsFile");
	if (g_pGameEvents->LoadFromFile(filesystem, "resource/GameEvents.res", "GAME"))
	{
		// loop through all events in this file and record ourselves as listener for each
		for (KeyValues *pEvent = g_pGameEvents->GetFirstSubKey(); pEvent; pEvent = pEvent->GetNextKey())
		{
			Msg("Event: %s\n", pEvent->GetName());
			gameeventmanager->AddListener(this, pEvent->GetName(), true);
		}
	}
	else
		Msg ("Unable to read game events from file\n");
	// read the engine events
	g_pEngineEvents = new KeyValues("EngineEventsFile");
	if (g_pEngineEvents->LoadFromFile(filesystem, "resource/ServerEvents.res", "GAME"))
	{
		// loop through all events in this file and record ourselves as listener for each
		for (KeyValues *pEvent = g_pEngineEvents->GetFirstSubKey(); pEvent; pEvent = pEvent->GetNextKey())
		{
			Msg("Event: %s\n", pEvent->GetName());
			gameeventmanager->AddListener(this, pEvent->GetName(), true);
		}
	}
	else
		Msg ("Unable to read engine events from file\n");
	// read the MOD events
	g_pModEvents = new KeyValues("ModEventsFile");
	if (g_pModEvents->LoadFromFile(filesystem, "resource/ModEvents.res", "MOD"))
	{
		// loop through all events in this file and record ourselves as listener for each
		for (KeyValues *pEvent = g_pModEvents->GetFirstSubKey(); pEvent; pEvent = pEvent->GetNextKey())
		{
			Msg("Event: %s\n", pEvent->GetName());
			gameeventmanager->AddListener(this, pEvent->GetName(), true);
		}
	}
	else
		Msg ("Unable to read MOD events from file\n");
}

void omnibot_eventhandler::PrintEvent(IGameEvent *pEvent)
{
	if(!pEvent)
		return;

	KeyValues *pEventAsKey;
	KeyValues *pKey;
	Msg ("Got event \"%s\"\n", pEvent->GetName ()); // event name
	Msg ("{\n"); // print the open brace
	// find the key/value corresponding to this event
	pEventAsKey = NULL;
	// look in the game events first...
	if (pEventAsKey == NULL)
		for (pEventAsKey = g_pGameEvents->GetFirstSubKey (); pEventAsKey; pEventAsKey = pEventAsKey->GetNextKey ())
			if (strcmp (pEventAsKey->GetName (), pEvent->GetName ()) == 0)
				break;
	// if not found, then look in the engine events...
	if (pEventAsKey == NULL)
		for (pEventAsKey = g_pEngineEvents->GetFirstSubKey (); pEventAsKey; pEventAsKey = pEventAsKey->GetNextKey ())
			if (strcmp (pEventAsKey->GetName (), pEvent->GetName ()) == 0)
				break;
	// and finally look for it in the mod events
	if (pEventAsKey == NULL)
		for (pEventAsKey = g_pModEvents->GetFirstSubKey (); pEventAsKey; pEventAsKey = pEventAsKey->GetNextKey ())
			if (strcmp (pEventAsKey->GetName (), pEvent->GetName ()) == 0)
				break;
	// display the whole key/value tree for this event
	for (pKey = pEventAsKey->GetFirstSubKey (); pKey; pKey = pKey->GetNextKey ())
	{
		// given the data type, print out the data
		if (strcmp (pKey->GetString (), "none") == 0)
			Msg ("   \"%s\" = no value (TYPE_NONE)\n", pKey->GetName ());
		else if (strcmp (pKey->GetString (), "string") == 0)
			Msg ("   \"%s\" = \"%s\" (TYPE_STRING)\n", pKey->GetName (), pEvent->GetString (pKey->GetName ()));
		else if (strcmp (pKey->GetString (), "bool") == 0)
			Msg ("   \"%s\" = %s (TYPE_BOOL)\n", pKey->GetName (), (pEvent->GetBool (pKey->GetName ()) ? "true" : "false"));
		else if (strcmp (pKey->GetString (), "byte") == 0)
			Msg ("   \"%s\" = %d (TYPE_BYTE)\n", pKey->GetName (), pEvent->GetInt (pKey->GetName ()));
		else if (strcmp (pKey->GetString (), "short") == 0)
			Msg ("   \"%s\" = %d (TYPE_SHORT)\n", pKey->GetName (), pEvent->GetInt (pKey->GetName ()));
		else if (strcmp (pKey->GetString (), "long") == 0)
			Msg ("   \"%s\" = %d (TYPE_LONG)\n", pKey->GetName (), pEvent->GetInt (pKey->GetName ()));
		else if (strcmp (pKey->GetString (), "float") == 0)
			Msg ("   \"%s\" = %f (TYPE_FLOAT)\n", pKey->GetName (), pEvent->GetFloat (pKey->GetName ()));
	}
	Msg ("}\n");
}

void omnibot_eventhandler::PrintEventStructure(KeyValues *pEventAsKey)
{
	KeyValues *pKey;
	Msg ("Event \"%s\"\n", pEventAsKey->GetName ()); // event name
	Msg ("{\n"); // print the open brace
	// display the whole key/value tree for this event
	for (pKey = pEventAsKey->GetFirstSubKey (); pKey; pKey = pKey->GetNextKey ())
	{
		// given the data type, print out the data
		if (strcmp (pKey->GetString (), "none") == 0)
			Msg ("   \"%s\", no value (TYPE_NONE)\n", pKey->GetName ());
		else if (strcmp (pKey->GetString (), "string") == 0)
			Msg ("   \"%s\" (TYPE_STRING)\n", pKey->GetName ());
		else if (strcmp (pKey->GetString (), "bool") == 0)
			Msg ("   \"%s\" (TYPE_BOOL)\n", pKey->GetName ());
		else if (strcmp (pKey->GetString (), "byte") == 0)
			Msg ("   \"%s\" (TYPE_BYTE)\n", pKey->GetName ());
		else if (strcmp (pKey->GetString (), "short") == 0)
			Msg ("   \"%s\" (TYPE_SHORT)\n", pKey->GetName ());
		else if (strcmp (pKey->GetString (), "long") == 0)
			Msg ("   \"%s\" (TYPE_LONG)\n", pKey->GetName ());
		else if (strcmp (pKey->GetString (), "float") == 0)
			Msg ("   \"%s\" (TYPE_FLOAT)\n", pKey->GetName ());
	}
	Msg ("}\n"); // print the closing brace
	return;
}

CON_COMMAND( listevt, "List all the events matching a given pattern" )
{
	KeyValues *pEventAsKey;
	int event_count;
	char arg1[128]; // no way, I hate pointers...
	Q_snprintf(arg1, 128, engine->Cmd_Argv (1)); // get any argument to the command
	// tell people what we are going to do
	if (arg1[0] == 0)
		Msg ("Printing out ALL game, server and MOD events...\n");
	else
		Msg ("Printing out game, server and MOD events matching \"%s\"...\n", arg1);
	Msg ("Game events:\n");
	event_count = 0;
	if(g_pGameEvents)
	{
		for(pEventAsKey = g_pGameEvents->GetFirstSubKey (); pEventAsKey; pEventAsKey = pEventAsKey->GetNextKey ())
		{
			// if we want a particular pattern AND this event does not match
			if ((arg1[0] != 0) && (strstr (pEventAsKey->GetName (), arg1) == NULL))
				continue; // then skip it
			g_pEventHandler->PrintEventStructure (pEventAsKey); // display this event's structure
			event_count++;
		}
	}	
	Msg ("%d game event%s.\n", event_count, (event_count > 1 ? "s" : ""));
	Msg ("Engine events:\n");
	event_count = 0;
	if(g_pEngineEvents)
	{
		for(pEventAsKey = g_pEngineEvents->GetFirstSubKey (); pEventAsKey; pEventAsKey = pEventAsKey->GetNextKey ())
		{
			// if we want a particular pattern AND this event does not match
			if ((arg1[0] != 0) && (strstr (pEventAsKey->GetName (), arg1) == NULL))
				continue; // then skip it
			g_pEventHandler->PrintEventStructure (pEventAsKey); // display this event's structure
			event_count++;
		}
	}	
	Msg ("%d engine event%s.\n", event_count, (event_count > 1 ? "s" : ""));
	Msg ("MOD events:\n");
	event_count = 0;
	if(g_pModEvents)
	{
		for(pEventAsKey = g_pModEvents->GetFirstSubKey (); pEventAsKey; pEventAsKey = pEventAsKey->GetNextKey ())
		{
			// if we want a particular pattern AND this event does not match
			if ((arg1[0] != 0) && (strstr (pEventAsKey->GetName (), arg1) == NULL))
				continue; // then skip it
			g_pEventHandler->PrintEventStructure (pEventAsKey); // display this event's structure
			event_count++;
		}
	}	
	Msg ("%d MOD event%s.\n", event_count, (event_count > 1 ? "s" : ""));
}

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

void event_Chat(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer)
	{
		int iGameId = pPlayer->entindex()-1;
		bool bTeamOnly = _event->GetBool("teamonly");
		BotUserData bud( _event->GetString("text", ""));
		if(bTeamOnly)
		{
			int iSourceTeam = pPlayer->GetTeamNumber();

			for(int i = 1; i <= gpGlobals->maxClients; ++i)
			{
				CBaseEntity *pEntity = CBaseEntity::Instance(i);				
				if(pEntity && (iSourceTeam == pEntity->GetTeamNumber()))
				{
					omnibot_interface::Bot_Interface_SendEvent(PERCEPT_HEAR_TEAMCHATMSG, i+1, iGameId, 0, &bud);
				}
			}
		}
		else
		{
			omnibot_interface::Bot_Interface_SendGlobalEvent(PERCEPT_HEAR_GLOBALCHATMSG, iGameId, 0, &bud);
		}
	}		
}

void event_Class(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_ChangedClass(pPlayer, _event->GetInt("oldclass"), _event->GetInt("newclass"));
	}
}

void event_Team(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_ChangedTeam(pPlayer, _event->GetInt("team"));
	}
}

void event_Hurt(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CBasePlayer *pAttacker = UTIL_PlayerByUserId(_event->GetInt("attacker"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_Hurt(pPlayer, pAttacker ? pAttacker->edict() : 0);
	}	
}

void event_Death(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CBasePlayer *pAttacker = UTIL_PlayerByUserId(_event->GetInt("attacker"));
	//const char *pWeapon = _event->GetInt("weapon"); TODO:
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_Death(pPlayer, pAttacker->edict());
	}	
}

void event_Spawn(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_Spawned(pPlayer);
	}
}

void event_Connect(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer)
	{
		Omnibot::Notify_ClientConnected(pPlayer, _event->GetBool("bot"));
	}	
}

void event_Disconnect(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer)
	{
		Omnibot::Notify_ClientDisConnected(pPlayer);
	}	
}

void event_SentryBuilt(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CFFPlayer *pFFPlayer = ToFFPlayer(pPlayer);
 	if(pFFPlayer && pFFPlayer->IsBot())
	{
		// Get the sentry edict.
		CAI_BaseNPC *pBuildable = pFFPlayer->m_hSentryGun.Get();
		if(pBuildable)
		{
			Omnibot::Notify_SentryBuilt(pPlayer, pBuildable->edict());			
		}
	}
}

void event_DispenserBuilt(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CFFPlayer *pFFPlayer = ToFFPlayer(pPlayer);
	if(pFFPlayer && pFFPlayer->IsBot())
	{
		// Get the sentry edict.
		CAI_BaseNPC *pBuildable = pFFPlayer->m_hDispenser.Get();
		if(pBuildable)
		{
			Omnibot::Notify_DispenserBuilt(pPlayer, pBuildable->edict());
		}
	}
}

void event_DetpackBuilt(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CFFPlayer *pFFPlayer = ToFFPlayer(pPlayer);
	if(pFFPlayer && pFFPlayer->IsBot())
	{
		// Get the sentry edict.
		CAI_BaseNPC *pBuildable = pFFPlayer->m_hDetpack.Get();
		if(pBuildable)
		{
			Omnibot::Notify_DetpackBuilt(pPlayer, pBuildable->edict());
		}
	}
}

void event_SentryKilled(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CBasePlayer *pAttacker = UTIL_PlayerByUserId(_event->GetInt("attacker"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_SentryDestroyed(pPlayer, pAttacker ? pAttacker->edict() : 0);
	}
}

void event_DispenserKilled(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CBasePlayer *pAttacker = UTIL_PlayerByUserId(_event->GetInt("attacker"));

	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_DispenserDestroyed(pPlayer, pAttacker ? pAttacker->edict() : 0);	
	}
}

void event_SentryUpgraded(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_SentryUpgraded(pPlayer, _event->GetInt("level"));	
	}
}

void event_DisguiseFinished(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_Disguising(pPlayer, _event->GetInt("team"), _event->GetInt("class"));
	}
}

void event_DisguiseLost(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_DisguiseLost(pPlayer);
	}
}

void event_SpyUnFeigned(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_UnFeigned(pPlayer);
	}
}

void event_SpyFeigned(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_Feigned(pPlayer);
	}
}

void event_DispenserEnemyUsed(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CBasePlayer *pEnemy = UTIL_PlayerByUserId(_event->GetInt("saboteur"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_DispenserEnemyUsed(pPlayer, pEnemy ? pEnemy->edict() : 0);
	}
}

void event_DispenserSabotaged(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CBasePlayer *pEnemy = UTIL_PlayerByUserId(_event->GetInt("saboteur"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_DispenserSabotaged(pPlayer, pEnemy ? pEnemy->edict() : 0);
	}
}

void event_SentrySabotaged(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CBasePlayer *pEnemy = UTIL_PlayerByUserId(_event->GetInt("saboteur"));
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_SentrySabotaged(pPlayer, pEnemy ? pEnemy->edict() : 0);
	}
}

void event_DispenserDetonated(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));	
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_DispenserDetonated(pPlayer);
	}
}

void event_DispenserDismantled(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));	
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_DispenserDismantled(pPlayer);
	}
}

void event_SentryDetonated(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));	
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_SentryDetonated(pPlayer);
	}
}

void event_SentryDismantled(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));	
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_SentryDismantled(pPlayer);
	}
}

void event_DetpackDetonated(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));	
	if(pPlayer && pPlayer->IsBot())
	{
		Omnibot::Notify_DetpackDetonated(pPlayer);
	}
}
