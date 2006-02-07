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
void event_SentryKilled(IGameEvent *_event);
void event_DispenserKilled(IGameEvent *_event);

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
	{ "sentry_killed", event_SentryKilled },
	{ "dispenser_killed", event_DispenserKilled },
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

		BotUserData bud(_event->GetString("text"));
		bool bTeamOnly = _event->GetBool("teamonly");
		if(bTeamOnly)
		{
			int iSourceTeam = pPlayer->GetTeamNumber();

			for(int i = 1; i <= gpGlobals->maxClients; ++i)
			{
				CBaseEntity *pEntity = CBaseEntity::Instance(i);				
				if(pEntity && (iSourceTeam == pEntity->GetTeamNumber()))
				{
					omnibot_interface::Bot_Interface_SendEvent(PERCEPT_HEAR_CHATMSG, i+1, iGameId, 0, &bud);
				}
			}
		}
		else
		{
			// Global chat msg, send to everyone
			omnibot_interface::Bot_Interface_SendGlobalEvent(PERCEPT_HEAR_CHATMSG, iGameId, 0, &bud);
		}
	}		
}

void event_Class(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		int iGameId = pPlayer->entindex()-1;
		BotUserData bud(obUtilGetBotClassFromGameClass(_event->GetInt("newclass")));
		omnibot_interface::Bot_Interface_SendEvent(MESSAGE_CHANGECLASS, iGameId, 0, 0, &bud);
	}
}

void event_Team(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		int iGameId = pPlayer->entindex()-1;

		BotUserData bud(_event->GetInt("team"));
		omnibot_interface::Bot_Interface_SendEvent(MESSAGE_CHANGETEAM, iGameId, 0, 0, &bud);
	}
}

void event_Hurt(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		int iGameId = pPlayer->entindex()-1;
		CBasePlayer *pAttackerPlayer = UTIL_PlayerByUserId(_event->GetInt("attacker"));

		BotUserData bud((GameEntity)(pAttackerPlayer ? pAttackerPlayer->edict() : 0));
		omnibot_interface::Bot_Interface_SendEvent(PERCEPT_FEEL_PAIN, iGameId, 0, 0, &bud);
	}	
}

void event_Death(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		int iGameId = pPlayer->entindex()-1;

		BotUserData bud(_event->GetInt("attacker"));
		omnibot_interface::Bot_Interface_SendEvent(MESSAGE_DEATH, iGameId, 0, 0, &bud);
	}	
}

void event_Spawn(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer && pPlayer->IsBot())
	{
		int iGameId = pPlayer->entindex()-1;
		omnibot_interface::Bot_Interface_SendEvent(MESSAGE_SPAWN, iGameId, 0, 0, 0);
	}
}

void event_Connect(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer)
	{
		int iGameId = pPlayer->entindex()-1;
		if (_event->GetBool("bot"))
			omnibot_interface::Bot_Interface_SendGlobalEvent(GAME_ID_BOTCONNECTED, iGameId, 100, NULL);
		else
			omnibot_interface::Bot_Interface_SendGlobalEvent(GAME_ID_CLIENTCONNECTED, iGameId, 100, NULL);
	}	
}

void event_Disconnect(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	if(pPlayer)
	{
		int iGameId = pPlayer->entindex()-1;
		omnibot_interface::Bot_Interface_SendGlobalEvent(GAME_ID_CLIENTDISCONNECTED, iGameId, 0, NULL);	
	}	
}

void event_SentryBuilt(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(pPlayer);
 	if(pFFPlayer && pFFPlayer->IsBot())
	{
		// Get the sentry edict.
		CAI_BaseNPC *pBuildable = pFFPlayer->m_hSentryGun.Get();
		if(pBuildable)
		{
			int iGameId = pPlayer->entindex()-1;

			Omnibot::BotUserData bud(pBuildable->edict());
			Omnibot::omnibot_interface::Bot_Interface_SendEvent(
				Omnibot::TF_MESSAGE_SENTRY_BUILT,
				iGameId, 0, 0, &bud);
		}
	}
}

void event_DispenserBuilt(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(pPlayer);
	if(pFFPlayer && pFFPlayer->IsBot())
	{
		// Get the sentry edict.
		CAI_BaseNPC *pBuildable = pFFPlayer->m_hDispenser.Get();
		if(pBuildable)
		{
			int iGameId = pPlayer->entindex()-1;

			Omnibot::BotUserData bud(pBuildable->edict());
			Omnibot::omnibot_interface::Bot_Interface_SendEvent(
				Omnibot::TF_MESSAGE_DISPENSER_BUILT,
				iGameId, 0, 0, &bud);
		}
	}
}

void event_DetpackBuilt(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("userid"));
	CFFPlayer *pFFPlayer = static_cast<CFFPlayer*>(pPlayer);
	if(pFFPlayer && pFFPlayer->IsBot())
	{
		// Get the sentry edict.
		CAI_BaseNPC *pBuildable = pFFPlayer->m_hDetpack.Get();
		if(pBuildable)
		{
			int iGameId = pPlayer->entindex()-1;

			Omnibot::BotUserData bud(pBuildable->edict());
			Omnibot::omnibot_interface::Bot_Interface_SendEvent(
				Omnibot::TF_MESSAGE_DETPACK_BUILT,
				iGameId, 0, 0, &bud);
		}
	}
}

void event_SentryKilled(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("ownerid"));
	int iAttackerId = _event->GetInt("attackerid");
	CBasePlayer *pAttacker = iAttackerId != 0 ? UTIL_PlayerByUserId(iAttackerId) : 0;

	if(pPlayer && pPlayer->IsBot())
	{
		int iGameId = pPlayer->entindex()-1;

		Omnibot::BotUserData bud;
		if(pAttacker)
		{
			bud.m_DataType = Omnibot::BotUserData::dtEntity;
			bud.udata.m_Entity = pAttacker->edict();
		}

		Omnibot::omnibot_interface::Bot_Interface_SendEvent(
			TF_MESSAGE_SENTRY_DESTROYED,
			iGameId, 0, 0, &bud);		
	}
}

void event_DispenserKilled(IGameEvent *_event)
{
	CBasePlayer *pPlayer = UTIL_PlayerByUserId(_event->GetInt("ownerid"));
	int iAttackerId = _event->GetInt("attackerid");
	CBasePlayer *pAttacker = iAttackerId != 0 ? UTIL_PlayerByUserId(iAttackerId) : 0;

	if(pPlayer && pPlayer->IsBot())
	{
		int iGameId = pPlayer->entindex()-1;

		Omnibot::BotUserData bud;
		if(pAttacker)
		{
			bud.m_DataType = Omnibot::BotUserData::dtEntity;
			bud.udata.m_Entity = pAttacker->edict();
		}

		Omnibot::omnibot_interface::Bot_Interface_SendEvent(
			TF_MESSAGE_DISPENSER_DESTROYED,
			iGameId, 0, 0, &bud);		
	}
}

