////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: DrEvil $
// $LastChangedDate: 2006-03-28 21:32:59 -0500 (Tue, 28 Mar 2006) $
// $LastChangedRevision: 1143 $
//
// Title: BotExports
//		In order for the game to call functions from the bot, we must export
//		the functions to the game itself and allow it to call them. 
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __BOTEXPORTS_H__
#define __BOTEXPORTS_H__

#include "Functions_Bot.h"
#include "Functions_Engine.h"
#include "Omni-Bot_Types.h"
#include "Omni-Bot_Events.h"

//////////////////////////////////////////////////////////////////////////
// Export the function on platforms that require it.
#ifdef WIN32
#define OMNIBOT_API __declspec(dllexport)
#else
#define OMNIBOT_API 
#endif

// Typedef for the only exported bot function.
typedef int (*pfnGetFunctionsFromDLL)(Bot_EngineFuncs_t *_pBotFuncs, int _size);

// note: Export Functions with C Linkage
//	Export with C Linkage so the game interface can acccess it easier.
//	This gets rid of name mangling
//	Wrapped in #ifdef because the game SDK might be in pure C
#ifdef __cplusplus
extern "C" 
{
#endif
	// function: ExportBotFunctionsFromDLL
	//		Allow the bot dll to fill in a struct of bot functions the interface
	//		can then call.
	OMNIBOT_API int ExportBotFunctionsFromDLL(Bot_EngineFuncs_t *_pBotFuncs, int _size);
#ifdef __cplusplus
}
#endif

//////////////////////////////////////////////////////////////////////////
// Helpers for Interfaces

static const char *BOTERRORS[BOT_NUM_ERRORS] = 
{
	"None",
	"Bot Library not found",
	"Unable to get Bot Functions from DLL",
	"Error Initializing the Bot",
	"Invalid Interface Functions",
	"Wrong Version",
};

// Macro: BOT_ERR_MSG
//		Translates an error code into an error string.
#define BOT_ERR_MSG(iMsg) \
	(((iMsg) >= BOT_ERROR_NONE) && ((iMsg) < BOT_NUM_ERRORS)) ? BOTERRORS[(iMsg)] : ""

Bot_EngineFuncs_t		g_BotFunctions = {0};
Game_EngineFuncs_t		g_InterfaceFunctions = {0};

// Platform stuff for loading the bot dll and getting the interface set up.
#ifdef WIN32
	#define WIN32_LEAN_AND_MEAN
	#define NOWINRES
	#define NOSERVICE
	#define NOMCX
	#define NOIME
	#include <windows.h>
	#undef GetClassName // stupidass windows

	//////////////////////////////////////////////////////////////////////////	
	HINSTANCE g_BotLibrary = NULL;
	
	// Macro: INITBOTLIBRARY
	//		Initializes the bot library by attempting to load the bot from several
	//		expected bot locations. If found, it verifies a version before returning 0
	//		on success, or an error code on failure, which can be used with <BOT_ERR_MSG>
	#define INITBOTLIBRARY(version, navid, win, lin, custom, result) \
	{ \
		char szBuffer[1024] = {0}; \
		OB_snprintf(szBuffer, 1024, "%s/%s", custom, win); \
		g_BotLibrary = LoadLibrary( szBuffer ); \
		if(g_BotLibrary == 0) \
			g_BotLibrary = LoadLibrary( ".\\omni-bot\\" win ); \
		if(g_BotLibrary == 0) \
			g_BotLibrary = LoadLibrary( win ); \
		if(g_BotLibrary == 0) \
			result = BOT_ERROR_CANTLOADDLL; \
		else \
		{ \
			pfnGetFunctionsFromDLL pfnGetBotFuncs = 0; \
			memset(&g_BotFunctions, 0, sizeof(g_BotFunctions)); \
			pfnGetBotFuncs = (pfnGetFunctionsFromDLL)GetProcAddress(g_BotLibrary, "ExportBotFunctionsFromDLL"); \
			if(pfnGetBotFuncs == 0) \
			{ \
				result = BOT_ERROR_CANTGETBOTFUNCTIONS; \
			} else \
			{ \
				result = pfnGetBotFuncs(&g_BotFunctions, sizeof(g_BotFunctions)); \
				if(result == BOT_ERROR_NONE) \
				{ \
					result = g_BotFunctions.pfnBotInitialise(navid, &g_InterfaceFunctions, version); \
				} \
			} \
		} \
	} \

	// Macro: SHUTDOWNBOTLIBRARY
	//		Handles shutting down and free'ing the bot library.
	#define SHUTDOWNBOTLIBRARY \
		if(g_BotLibrary) { FreeLibrary(g_BotLibrary); g_BotLibrary = 0; memset(&g_BotFunctions, 0, sizeof(g_BotFunctions)); }
		
	//////////////////////////////////////////////////////////////////////////
#elif defined __linux__
	#include <dlfcn.h>
	#define GetProcAddress dlsym
	#define NULL 0

	//////////////////////////////////////////////////////////////////////////	
	void *g_BotLibrary = NULL;

	#define INITBOTLIBRARY(version, navid, win, lin, custom, result) \
	{ \
		const char *pError = 0; \
		char szBuffer[1024] = {0}; \
		sprintf(szBuffer, "%s/%s", custom, lin); \
		g_BotLibrary = dlopen( szBuffer, RTLD_NOW ); \
		if(pError = dlerror()) \
		{ \
			sprintf(szBuffer, "failed loading: %s", pError); \
			pfnPrintError(szBuffer); \
			g_BotLibrary = dlopen( "./omni-bot/" lin, RTLD_NOW ); \
		} \
		if(pError = dlerror()) \
		{ \
			char *homeDir = getenv( "HOME" ); \
			sprintf(szBuffer, "failed loading: %s", pError); \
			pfnPrintError(szBuffer); \
			if( homeDir && *homeDir ) \
			{ \
				char *dir = va("%s/.etwolf/omni-bot/%s", homeDir, lin ); \
				g_BotLibrary = dlopen( dir, RTLD_NOW ); \
			} \
		} \
		if(pError = dlerror()) \
		{ \
			sprintf(szBuffer, "failed loading: %s", pError); \
			pfnPrintError(szBuffer); \
			g_BotLibrary = dlopen( lin, RTLD_NOW ); \
		} \
		if(pError = dlerror()) \
		{ \
			sprintf(szBuffer, "failed loading: %s", pError); \
			pfnPrintError(szBuffer); \
			result = BOT_ERROR_CANTLOADDLL; \
		} \
		else \
		{ \
			pfnGetFunctionsFromDLL pfnGetBotFuncs = 0; \
			memset(&g_BotFunctions, 0, sizeof(g_BotFunctions)); \
			pfnGetBotFuncs = (pfnGetFunctionsFromDLL)GetProcAddress(g_BotLibrary, "ExportBotFunctionsFromDLL"); \
			if(dlerror()) \
			{ \
				result = BOT_ERROR_CANTGETBOTFUNCTIONS; \
			} \
			else \
			{ \
				result = pfnGetBotFuncs(&g_BotFunctions, sizeof(g_BotFunctions)); \
				if(result == BOT_ERROR_NONE) \
				{ \
					result = g_BotFunctions.pfnBotInitialise(navid, &g_InterfaceFunctions, version); \						
				} \
			} \
		} \
	} \

	#define SHUTDOWNBOTLIBRARY \
		if(g_BotLibrary) { dlclose(g_BotLibrary); g_BotLibrary = 0; memset(&g_BotFunctions, 0, sizeof(g_BotFunctions)); }

	//////////////////////////////////////////////////////////////////////////
#else
	// get a mac somewhere and do support...
	void *g_BotLibrary = NULL;
	#define INITBOTLIBRARY(version, navid, win, lin, custom, result) \
		result = BOT_ERROR_CANTLOADDLL;
	#define SHUTDOWNBOTLIBRARY \
		g_BotLibrary = 0;
#endif

#endif
