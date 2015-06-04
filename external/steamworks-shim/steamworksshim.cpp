#define STEAM_API_NODLL
#include "steamworks_sdk/public/steam/steam_api.h"
#include "SteamworksShim.h"
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

HMODULE hSteamApiModule = NULL;
// Steam User interface
ISteamUser *m_pSteamUser = NULL;
// Steam UserStats interface
ISteamUserStats *m_pSteamUserStats = NULL;

typedef bool (*pfnSteamAPI_Init)();
typedef ISteamUser* (*pfnSteamUser)();
typedef ISteamUserStats* (*pfnSteamUserStats)();

DllExport int SteamworksShim_StartUp()
{
	hSteamApiModule = LoadLibrary("steam_api_new.dll");
	// this is not in the version of steam api 2007 has :-(
	/*if ( SteamAPI_RestartAppIfNecessary( 0 ) )
	{
		OutputDebugString( "SteamAPI_RestartAppIfNecessary\n" );
		return -1;
	}*/

	if (!hSteamApiModule)
		return -1;
	
	pfnSteamAPI_Init pfnInit = (pfnSteamAPI_Init) GetProcAddress(hSteamApiModule, "SteamAPI_Init");
	if (!pfnInit)
		return -1;

	/*bool bInitRet = pfnInit();
	if ( !bInitRet )
	{
		OutputDebugString( "SteamAPI_Init() failed\n" );
		return -2;
	}*/

	// set our debug handler
	//SteamClient()->SetWarningMessageHook( &SteamAPIDebugTextHook );

	// Ensure that the user has logged into Steam. This will always return true if the game is launched
	// from Steam, but if Steam is at the login prompt when you run your game from the debugger, it
	// will return false.
	//if ( !SteamUser()->BLoggedOn() )
	//{
	//	OutputDebugString( "Steam user is not logged in\n" );
	//	//Alert( "Fatal Error", "Steam user must be logged in to play this game (SteamUser()->BLoggedOn() returned false).\n" );
	//	return -3;
	//}

	//// TESTING
	////CSteamID steamId = SteamUser()->GetSteamID();

	pfnSteamUser pfnGetSteamUser = (pfnSteamUser) GetProcAddress(hSteamApiModule, "SteamUser");
	pfnSteamUserStats pfnGetSteamUserStats = (pfnSteamUserStats) GetProcAddress(hSteamApiModule, "SteamUserStats");
	if (!pfnGetSteamUser || !pfnGetSteamUserStats)
		return -4;
	m_pSteamUser		= pfnGetSteamUser();
	m_pSteamUserStats	= pfnGetSteamUserStats();

	return 1;
}


DllExport void SteamworksShim_Shutdown()
{
	SteamAPI_Shutdown();
	if (hSteamApiModule)
		FreeLibrary(hSteamApiModule);
	hSteamApiModule = NULL;
}


DllExport void SteamworksShim_SetSteamApiDebugMsgHandler( pfnSteamAPIWarningMsgHandler handler )
{
	if ( !handler )
		return;
	SteamAPIWarningMessageHook_t msgHook = reinterpret_cast<SteamAPIWarningMessageHook_t>(handler);
	if (msgHook)
		SteamClient()->SetWarningMessageHook( msgHook );
}

DllExport void SteamworksShim_SetStat(const char *statName, unsigned int val)
{
	/*if (!m_pSteamUserStats)
		return;
	int32 valCast = (int32) val;
	m_pSteamUserStats->SetStat(statName, valCast);*/
	
}

DllExport unsigned int SteamworksShim_GetStat(const char *statName)
{
	/*if (!m_pSteamUserStats)
		return 0;
	int32 ret;
	m_pSteamUserStats->GetStat(statName, &ret);
	return (unsigned int)ret;*/
	return 0;
}

DllExport void SteamworksShim_LoadCurrentStats()
{
	/*if (!m_pSteamUserStats)
		return;
	m_pSteamUserStats->RequestCurrentStats();*/

}

DllExport void SteamworksShim_SetAchievement( const char *achievementName )
{
	//if (!m_pSteamUserStats)
	//	return;
	//m_pSteamUserStats->SetAchievement( achievementName );
}


