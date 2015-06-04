
// c_ff_achievementmgr.cpp

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "c_ff_achievementmgr.h"
//#include "tier0/vprof.h"
#include "c_ff_player.h"
//#include "SteamworksShim.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CFFAchievementManager _achievementMgr;

//#define DllExport __declspec ( dllexport )

extern int SteamworksShim_StartUp( void );
extern void SteamworksShim_Shutdown( void );

extern void SteamworksShim_SetStat(const char *statName, unsigned int value);
extern void SteamworksShim_SetAchievement( const char *achievementName );
//unsigned int SteamworksShim_GetStat(const char *statName);
//void SteamworksShim_LoadCurrentStats();
//void SteamworksShim_AwardAchievement(unsigned int achievementId);

extern "C" typedef void (__cdecl *pfnSteamAPIWarningMsgHandler)(int, const char *);
extern void SteamworksShim_SetSteamApiDebugMsgHandler( pfnSteamAPIWarningMsgHandler handler );
//void SteamworksShim_SetSteamApiDebugMsgHandler( pfnSteamAPIWarningMsgHandler handler );


// add -debug_steamapi to launch parameters to see steamworks msgs in our given hook
// requires C linkage
extern "C" void SteamDebugMsgHandler( int iSeverity, const char *pchDebugMsg )
{
	DevMsg( "[Achievements] Steamworks dbg message '%s'\n", pchDebugMsg );
}

CFFAchievementManager::CFFAchievementManager() : m_bIsSteamworksLoaded(false)
{
	DevMsg( "[Achievements] Ctor 0" ); 
	/*
	const char *dllName = "SteamworksShim.dll";
	CSysModule *pShimModule = filesystem->LoadModule(dllName);
	if (!pShimModule)
	{
		DevMsg("[Achievements] failed to load steamworks shim\n");
		return;
	}
	//HMODULE f;
	//Sys_GetProcAddress(
	m_pShimModule = Sys_LoadModule("SteamworksShim.dll");
	if (!m_pShimModule)
		return;
	m_hShimDll = reinterpret_cast<HMODULE>(m_pShimModule);
	if (!m_hShimDll)
		return;*/
	//CSysModule pShimModule = Sys_LoadModule(dllName);
	/*m_pfnStartUp = reinterpret_cast<pfnStartUp>(Sys_GetProcAddress(m_ShimDllName, "StartUp"));
	*/
	int iRet = SteamworksShim_StartUp();
	DevMsg( "[Achievements] StartUp = %d", iRet );
	m_bIsSteamworksLoaded = iRet >= 0;

	//SteamworksShim_SetSteamApiDebugMsgHandler(&SteamDebugMsgHandler);
	DevMsg( "[Achievements] Ctor 1" );
}

CFFAchievementManager::~CFFAchievementManager( void )
{
	// this already nullchcks
	//Sys_UnloadModule(m_pShimModule);

	if (m_bIsSteamworksLoaded)
		SteamworksShim_Shutdown();
}


void CFFAchievementManager::Think( void )
{

}
