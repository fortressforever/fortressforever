#pragma once

#define DllExport __declspec ( dllexport )

DllExport int SteamworksShim_StartUp();
DllExport void SteamworksShim_Shutdown();
DllExport void SteamworksShim_SetStat(const char *statName, unsigned int val);
DllExport unsigned int SteamworksShim_GetStat(const char *statName);
DllExport void SteamworksShim_LoadCurrentStats();

extern "C" typedef void (__cdecl *pfnSteamAPIWarningMsgHandler)(int, const char *);
DllExport void SteamworksShim_SetSteamApiDebugMsgHandler( pfnSteamAPIWarningMsgHandler handler );

DllExport void SteamworksShim_SetAchievement( const char *achievementName );
