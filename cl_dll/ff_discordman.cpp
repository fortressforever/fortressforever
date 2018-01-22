// wrapper for discord rich presence api
// they provide static libs, dlls and headers to use directly, but to no
// surprise, having to use ancient vc2005 ABI, we cant even link against
// modern DLL stub libs. Also the discord header includes stdint which vc2005 
// doesn't even have (lol), so its mostly just manually pulling in the structs
// and loading the library functions manually at runtime.

#include "cbase.h"
#include "ff_discordman.h"
#include <windows.h>
#include <igameresources.h>
#include "c_ff_player.h"
#include "ff_utils.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#define DISCORD_LIBRARY_DLL "discord-rpc.dll"
#define DISCORD_APP_ID "404135974801637378"
#define STEAM_ID "253530"

// update once every 10 seconds. discord has an internal rate limiter of 15 seconds as well
#define DISCORD_UPDATE_RATE 10.0f

ConVar use_discord("cl_discord", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE | FCVAR_USERINFO, 
				   "Enable discord rich presence integration (current, server, map etc)");

CFFDiscordManager _discord;

// runtime entry point nastiness we will hide in here
typedef void (*pDiscord_Initialize)(const char* applicationId,
                                       DiscordEventHandlers* handlers,
                                       int autoRegister,
                                       const char* optionalSteamId);

typedef void (*pDiscord_Shutdown)(void);
typedef void (*pDiscord_RunCallbacks)(void);
typedef void (*pDiscord_UpdatePresence)(const DiscordRichPresence* presence);

pDiscord_Initialize Discord_Initialize = NULL;
pDiscord_Shutdown Discord_Shutdown = NULL;
pDiscord_RunCallbacks Discord_RunCallbacks = NULL;
pDiscord_UpdatePresence Discord_UpdatePresence = NULL;

static HINSTANCE hDiscordDLL;

// NOTE: can not call discord from main menu thread. it will block

CFFDiscordManager::CFFDiscordManager()
{
	hDiscordDLL = LoadLibrary(DISCORD_LIBRARY_DLL);
	if (!hDiscordDLL)
	{
		return;
	}
	
	Discord_Initialize = (pDiscord_Initialize) GetProcAddress(hDiscordDLL, "Discord_Initialize");
	Discord_Shutdown = (pDiscord_Shutdown) GetProcAddress(hDiscordDLL, "Discord_Shutdown");
	Discord_RunCallbacks = (pDiscord_RunCallbacks) GetProcAddress(hDiscordDLL, "Discord_RunCallbacks");
	Discord_UpdatePresence = (pDiscord_UpdatePresence) GetProcAddress(hDiscordDLL, "Discord_UpdatePresence");

	// dont run this yet. it will hang client
	// InitializeDiscord();
	Q_memset(m_szLatchedMapname, 0, MAX_MAP_NAME);
	m_bApiReady = false;
}

CFFDiscordManager::~CFFDiscordManager()
{
	if (hDiscordDLL) 
	{
		// dont bother with clean exit, it will block. 
		// Discord_Shutdown();
		// i mean really, we could just let os handle since our dtor is called at game exit but
		FreeLibrary(hDiscordDLL);
	}
	hDiscordDLL = NULL;
}

void CFFDiscordManager::RunFrame()
{
	if (!use_discord.GetBool())
		return;

	if (!m_bApiReady)
	{
		if (!m_bInitializeRequested)
		{
			InitializeDiscord();
			m_bInitializeRequested = true;
		}
	}

	//gpGlobals->curtime
	UpdateRichPresence();

	// always run this, otherwise we will chicken & egg waiting for ready
	if (Discord_RunCallbacks)
		Discord_RunCallbacks();
}

void CFFDiscordManager::OnReady()
{
	DevMsg("discord ready");
	_discord.m_bApiReady = true;
}

void CFFDiscordManager::OnDiscordError(int errorCode, const char *szMessage)
{
	_discord.m_bApiReady = false;
	if (Q_strlen(szMessage) < 1024)
	{
		char buff[1024];
		Q_snprintf(buff, 1024, "Discord init failed. code %d - error: %s\n", errorCode, szMessage);
		Warning(buff);
	}
}

void CFFDiscordManager::InitializeDiscord()
{
	DiscordEventHandlers handlers;
	Q_memset(&handlers, 0, sizeof(handlers));
	handlers.ready = &CFFDiscordManager::OnReady;
	handlers.errored = &CFFDiscordManager::OnDiscordError;
//	handlers.disconnected = handleDiscordDisconnected;
//	handlers.joinGame = handleDiscordJoinGame;
//	handlers.spectateGame = handleDiscordSpectateGame;
//	handlers.joinRequest = handleDiscordJoinRequest;
	Discord_Initialize(DISCORD_APP_ID, &handlers, 1, STEAM_ID);
}

void CFFDiscordManager::UpdateRichPresence() 
{
	if (!Discord_UpdatePresence || !m_bApiReady)
		return;

	if (!m_szLatchedMapname || m_szLatchedMapname[0] == '\0')
	{
		return;
	}

	if (gpGlobals->curtime < m_flLastUpdatedTime + DISCORD_UPDATE_RATE)
		return;

	m_flLastUpdatedTime = gpGlobals->curtime;

	struct DiscordRichPresence discordPresence;
	Q_memset(&discordPresence, 0, sizeof(discordPresence));

	
	// we cant use time() due to relative timestamps for VCR mode
	// dont bother with elapsed timer. kinda pointless
	// discordPresence.startTimestamp = //time(0);
	discordPresence.largeImageKey = m_szLatchedMapname;
	discordPresence.largeImageText = m_szLatchedMapname;

	// FF_NumPlayers doesnt work client side because it doesnt use game resources :)

	IGameResources *pGR = GameResources();
	if (pGR)
	{
		//int curPlayers = FF_NumPlayers();
		int maxPlayers = gpGlobals->maxClients;
		int curPlayers = 0;
		char *className = NULL;
		int teamNum = 0;

		for (int i = 1; i < maxPlayers; i++)
		{
			if (pGR->IsConnected(i))
			{
				curPlayers++;
				if (pGR->IsLocalPlayer(i)) 
				{
					// this will always return our hardcoded english string,
					// we dont want to send a localized to discord (right?)
					className = const_cast<char *>(Class_IntToPrintString(pGR->GetClass(i)));
					teamNum = pGR->GetTeam(i);
				}
			}
		}

		// abuse 'party' to show server limits
		discordPresence.partySize = curPlayers;
		discordPresence.partyMax = maxPlayers;

		const ConVar *hostnameCvar = cvar->FindVar( "hostname" );
		const char *szHostname = hostnameCvar->GetString();
		char hostTrimmed[128];
		// discord max for header is 128 bytes, so game mode would probably be better
		Q_snprintf(hostTrimmed, 128, "Server: %s", szHostname); //, curPlayers, maxPlayers);
		discordPresence.state = hostTrimmed;
		
		char details[128];
		char *teamStr = NULL;
		switch (teamNum) 
		{
			case TEAM_RED: teamStr = "red"; break;
			case TEAM_BLUE: teamStr = "blue"; break;
			case TEAM_YELLOW: teamStr = "yellow"; break;
			case TEAM_GREEN: teamStr = "green"; break;
		}

		if (!teamStr || Q_strlen(className) < 1)
		{
			Q_snprintf(details, 128, "Spectating");
		}
		else 
		{
			Q_snprintf(details, 128, "Playing a %s %s", teamStr, className);// m_szLatchedMapname);
		}
		discordPresence.details = details;
	
	}
	
	discordPresence.instance = 1;
	Discord_UpdatePresence(&discordPresence);
}

void CFFDiscordManager::LevelInit(const char *szMapname)
{
	// we cant update our presence here, because if its the first map a client loaded,
	// discord api may not yet be loaded, so latch
	Q_strcpy(m_szLatchedMapname, szMapname);
	// important, clear last update time as well
	m_flLastUpdatedTime = max(0, gpGlobals->curtime - DISCORD_UPDATE_RATE);
}
