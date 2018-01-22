// wrapper for discord rich presence api
// they provide static libs, dlls and headers to use directly, but to no
// surprise, having to use ancient vc2005 ABI, we cant even link against
// modern DLL stub libs. Also the discord header includes stdint which vc2005 
// doesn't even have (lol), so its mostly just manually pulling in the structs
// and loading the library functions manually at runtime.

#include "cbase.h"
#include "ff_discordman.h"
#include "c_ff_player.h"
#include "ff_utils.h"
#include <windows.h>
#include <igameresources.h>
#include <inetchannelinfo.h>
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

static pDiscord_Initialize Discord_Initialize = NULL;
static pDiscord_Shutdown Discord_Shutdown = NULL;
static pDiscord_RunCallbacks Discord_RunCallbacks = NULL;
static pDiscord_UpdatePresence Discord_UpdatePresence = NULL;


// this is here instead of a member because including windows.h on the class
// causes C_FFPlayer to get compile errors. wew lawd
static HINSTANCE hDiscordDLL;

CFFDiscordManager::CFFDiscordManager()
{
	Q_memset(m_szLatchedMapname, 0, MAX_MAP_NAME);
	m_bApiReady = false;
	m_bInitializeRequested = false;
}

CFFDiscordManager::~CFFDiscordManager()
{
	m_bApiReady = false;
	if (hDiscordDLL)
	{
		// blocks :(
		//if (m_bApiReady)
		//	Discord_Shutdown();

		// i mean really, we could just let os handle since our dtor is called at game exit but
		FreeLibrary(hDiscordDLL);
	}
	hDiscordDLL = NULL;
	Discord_Initialize = NULL;
	Discord_Shutdown = NULL;
	Discord_RunCallbacks = NULL;
	Discord_UpdatePresence = NULL;
}

void CFFDiscordManager::Init()
{
	hDiscordDLL = LoadLibrary(DISCORD_LIBRARY_DLL);
	if (!hDiscordDLL)
	{
		m_bErrored = true;
		Warning("failed to load discord DLL, ensure %s exists\n", DISCORD_LIBRARY_DLL);
		return;
	}
	
	Discord_Initialize = (pDiscord_Initialize) GetProcAddress(hDiscordDLL, "Discord_Initialize");
	Discord_Shutdown = (pDiscord_Shutdown) GetProcAddress(hDiscordDLL, "Discord_Shutdown");
	Discord_RunCallbacks = (pDiscord_RunCallbacks) GetProcAddress(hDiscordDLL, "Discord_RunCallbacks");
	Discord_UpdatePresence = (pDiscord_UpdatePresence) GetProcAddress(hDiscordDLL, "Discord_UpdatePresence");

	InitializeDiscord();
	m_bInitializeRequested = true;

	// make sure to call this after game system initialized
	gameeventmanager->AddListener(this, "server_spawn", false );
}

void CFFDiscordManager::RunFrame()
{
	if (m_bErrored)
		return;

	// NOTE: we want to run this even if they have use_discord off, so we can clear
	// any previous state that may have already been sent
	UpdateRichPresence();

	// always run this, otherwise we will chicken & egg waiting for ready
	if (Discord_RunCallbacks)
		Discord_RunCallbacks();
}

void CFFDiscordManager::OnReady()
{
	DevMsg("discord ready");
	_discord.m_bApiReady = true;
	_discord.Reset();
}

void CFFDiscordManager::OnDiscordError(int errorCode, const char *szMessage)
{
	_discord.m_bApiReady = false;
	_discord.m_bErrored = true;
	char buff[1024];
	Q_snprintf(buff, 1024, "Discord init failed. code %d - error: %s\n", errorCode, szMessage);
	Warning(buff);
}

void CFFDiscordManager::InitializeDiscord()
{
	if (!Discord_Initialize)
		return;

	DiscordEventHandlers handlers;
	Q_memset(&handlers, 0, sizeof(handlers));
	handlers.ready = &CFFDiscordManager::OnReady;
	handlers.errored = &CFFDiscordManager::OnDiscordError;
	Discord_Initialize(DISCORD_APP_ID, &handlers, 1, STEAM_ID);
}

bool CFFDiscordManager::NeedToUpdate()
{
	if (!m_bApiReady || m_bErrored || m_szLatchedMapname[0] == '\0')
		return false;

	return gpGlobals->curtime >= m_flLastUpdatedTime + DISCORD_UPDATE_RATE;
}

void CFFDiscordManager::Reset()
{
	if (m_bApiReady)
	{
		Q_memset(&m_sDiscordRichPresence, 0, sizeof(m_sDiscordRichPresence));
		m_sDiscordRichPresence.state = "in menus";
		Discord_UpdatePresence(&m_sDiscordRichPresence);
	}
}

void CFFDiscordManager::UpdatePlayerInfo()
{
	// FF_NumPlayers() from utils doesnt work client side because it doesnt use game resources :)
	IGameResources *pGR = GameResources();
	if (!pGR)
		return;

	int maxPlayers = gpGlobals->maxClients;
	int curPlayers = 0;
	const char *className = NULL;
	int teamNum = 0;
	const char *playerName = NULL;
	int frags = 0;
	int deaths = 0;
	int points = 0;
	int teamPoints = 0;

	for (int i = 1; i < maxPlayers; i++)
	{
		if (pGR->IsConnected(i))
		{
			
			curPlayers++;
			if (pGR->IsLocalPlayer(i))
			{
				playerName = pGR->GetPlayerName(i);
				frags = pGR->GetFrags(i);
				deaths = pGR->GetDeaths(i);
				teamNum = pGR->GetTeam(i);
				points = pGR->GetFortPoints(i);
				teamPoints = pGR->GetTeamScore(teamNum);
				// dont call Class_IntToPrintString as spec, its noisy
				if (teamNum != TEAM_SPECTATOR)
				{
					// this will always return our hardcoded english string,
					// we dont want to send a localized to discord (right?)
					int classNum = pGR->GetClass(i);
					className = Class_IntToPrintString(classNum);
				}
			}
		}
	}

	// state is top line, details is box below
	//const ConVar *hostnameCvar = cvar->FindVar("hostname");
	//const char *szHostname = hostnameCvar->GetString();
	if (m_szLatchedHostname[0] != '\0')
	{
		Q_snprintf(m_szServerInfo, DISCORD_FIELD_SIZE, "[%d/%d] %s", curPlayers, maxPlayers, m_szLatchedHostname);
		DevMsg("[Discord] sending state of '%s'\n", m_szServerInfo);
		m_sDiscordRichPresence.state = m_szServerInfo;
	}

	const char *teamStr = NULL;

	switch (teamNum) 
	{
		// NOTE: this should eventually respect using lua teamnames
		case TEAM_RED: teamStr = "Red"; break;
		case TEAM_BLUE: teamStr = "Blue"; break;
		case TEAM_YELLOW: teamStr = "Yellow"; break;
		case TEAM_GREEN: teamStr = "Green"; break;
	}
	
	if (!teamStr || Q_strlen(className) < 1)
	{
		Q_snprintf(m_szDetails, DISCORD_FIELD_SIZE, "Spectating");
	}
	else
	{
		Q_snprintf(m_szDetails, DISCORD_FIELD_SIZE, "%s (%d pts) - %s %d pts %d:%d K:D", teamStr, teamPoints, className, points, frags, deaths);
	}

	DevMsg("[Discord] sending details of '%s'\n", m_szDetails);
	m_sDiscordRichPresence.details = m_szDetails;
}

void CFFDiscordManager::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( Q_strcmp(type, "server_spawn") == 0 )
	{
		// copy from event, it is freed after
		Q_strncpy( m_szLatchedHostname, event->GetString("hostname"), 255 );
	}
}

void CFFDiscordManager::UpdateRichPresence()
{
	if (!NeedToUpdate())
		return;

	m_flLastUpdatedTime = gpGlobals->curtime;

	if (!use_discord.GetBool())
	{
		// Reset to clear any previous state
		Reset();
		return;
	}

	// we cant use time() due to relative timestamps for VCR mode
	// dont bother with elapsed timer. kinda pointless
	// discordPresence.startTimestamp = //time(0);
	m_sDiscordRichPresence.largeImageKey = m_szLatchedMapname;
	m_sDiscordRichPresence.largeImageText = m_szLatchedMapname;

	UpdatePlayerInfo();
	UpdateNetworkInfo();

	m_sDiscordRichPresence.instance = 1;
	Discord_UpdatePresence(&m_sDiscordRichPresence);
}


void CFFDiscordManager::UpdateNetworkInfo()
{
	INetChannelInfo *ni = engine->GetNetChannelInfo();
	// set ip address as our cookie so if someone really wanted,
	// they could connect directly through discord

	// even in a private server (then needs password) this doesnt need to be secret,
	// just set the address so other clients can get it in join request
	m_sDiscordRichPresence.joinSecret = ni->GetAddress();
}

void CFFDiscordManager::LevelInit(const char *szMapname)
{
	Reset();
	// we cant update our presence here, because if its the first map a client loaded,
	// discord api may not yet be loaded, so latch
	Q_strcpy(m_szLatchedMapname, szMapname);
	// important, clear last update time as well
	m_flLastUpdatedTime = max(0, gpGlobals->curtime - DISCORD_UPDATE_RATE);
}
