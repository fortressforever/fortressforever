////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy: drevil $
// $LastChangedDate: 2008-01-25 08:23:37 -0800 (Fri, 25 Jan 2008) $
// $LastChangedRevision: 2370 $
//
////////////////////////////////////////////////////////////////////////////////

#include "BotExports.h"

#pragma warning(disable:4530) //C++ exception handler used, but unwind semantics are not enabled. Specify /EHsc
#pragma warning(disable:4706) //assignment within conditional expression

#include <string>

//////////////////////////////////////////////////////////////////////////

bool					g_IsOmnibotLoaded = false;
Bot_EngineFuncs_t		g_BotFunctions = {0};
IEngineInterface		*g_InterfaceFunctions = 0;
std::string				g_OmnibotLibPath;

void Omnibot_Load_PrintMsg(const char *_msg);
void Omnibot_Load_PrintErr(const char *_msg);

bool IsOmnibotLoaded()
{
	return g_IsOmnibotLoaded;
}

const char *Omnibot_GetLibraryPath()
{
	return g_OmnibotLibPath.c_str();
}

//////////////////////////////////////////////////////////////////////////

static const char *BOTERRORS[BOT_NUM_ERRORS] = 
{
	"None",
	"Bot Library not found",
	"Unable to get Bot Functions from DLL",
	"Error Initializing the Bot",
	"Invalid Interface Functions",
	"Wrong Version",
	"Error Initializing File System",
};

void Omnibot_strncpy(char *dest, const char *source, int count)
{
	// Only doing this because some engines(HL2), think it a good idea to fuck up the 
	// defines of all basic string functions throughout the entire project.
	while (count && (*dest++ = *source++)) /* copy string */
		count--;

	if (count) /* pad out with zeroes */
		while (--count)
			*dest++ = '\0';
}

const char *Omnibot_ErrorString(eomnibot_error err)
{
	return ((err >= BOT_ERROR_NONE) && (err < BOT_NUM_ERRORS)) ? BOTERRORS[err] : "";
}

const char *Omnibot_FixPath(const char *_path)
{
	const int iBufferSize = 512;
	static char pathstr[iBufferSize] = {0};
	Omnibot_strncpy(pathstr, _path, iBufferSize);

	// unixify the path slashes
	char *pC = pathstr;
	while(*pC)
	{
		if(*pC == '\\')
			*pC = '/';
		++pC;
	}

	// trim any trailing slash
	while(int iLen = strlen(pathstr))
	{
		if(pathstr[iLen-1] == '/')
			pathstr[iLen-1] = 0;
		else
			break;
	}
	return pathstr;
}

//////////////////////////////////////////////////////////////////////////

#if defined WIN32 || defined _WINDOWS || defined _WIN32

//////////////////////////////////////////////////////////////////////////
// Windows
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOWINRES
#define NOWINRES
#endif
#ifndef NOSERVICE
#define NOSERVICE
#endif
#ifndef NOMCX
#define NOMCX
#endif
#ifndef NOIME
#define NOIME
#endif
#include <stdio.h>
#include <windows.h>

//////////////////////////////////////////////////////////////////////////
// Utilities

const char *OB_VA(const char* _msg, ...)
{
	static int iCurrentBuffer = 0;
	const int iNumBuffers = 3;
	const int BUF_SIZE = 1024;
	struct BufferInstance
	{
		char buffer[BUF_SIZE];
	};
	static BufferInstance buffers[iNumBuffers];
	
	char *pNextBuffer = buffers[iCurrentBuffer].buffer;

	va_list list;
	va_start(list, _msg);
	_vsnprintf(pNextBuffer, sizeof(buffers[iCurrentBuffer].buffer), _msg, list);	
	va_end(list);

	iCurrentBuffer = (iCurrentBuffer+1)%iNumBuffers;
	return pNextBuffer;
}

//////////////////////////////////////////////////////////////////////////	
HINSTANCE g_BotLibrary = NULL;

bool OB_ShowLastError(const char *context)
{
	LPVOID lpMsgBuf;
	DWORD dw = GetLastError(); 
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER | 
		FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR) &lpMsgBuf,
		0, NULL );

	//////////////////////////////////////////////////////////////////////////
	// Strip Newlines
	char *pMessage = (char*)lpMsgBuf;
	int i = strlen(pMessage)-1;
	while(pMessage[i] == '\n' || pMessage[i] == '\r')
		pMessage[i--] = 0;
	//////////////////////////////////////////////////////////////////////////

	Omnibot_Load_PrintErr(OB_VA("%s Failed with Error: %s", context, pMessage));
	LocalFree(lpMsgBuf);
	return true;
}

HINSTANCE Omnibot_LL(const char *file)
{
	//////////////////////////////////////////////////////////////////////////
	// Parse Variables
	// $(ProgramFiles)
	// $(OMNIBOT)
	
	//////////////////////////////////////////////////////////////////////////
	g_OmnibotLibPath = file;
	HINSTANCE hndl = LoadLibrary(g_OmnibotLibPath.c_str());
	if(!hndl)
		OB_ShowLastError("LoadLibrary");
	Omnibot_Load_PrintMsg(OB_VA("Looking for %s, ", g_OmnibotLibPath.c_str(), hndl ? "found." : "not found"));
	return hndl;
}

eomnibot_error Omnibot_LoadLibrary(int version, const char *lib, const char *path)
{
	eomnibot_error r = BOT_ERROR_NONE;
	g_BotLibrary = Omnibot_LL( OB_VA("%s\\%s.dll", path ? path : ".", lib) );
	if(g_BotLibrary == 0)
		g_BotLibrary = Omnibot_LL( OB_VA(".\\omni-bot\\%s.dll", lib) );
	if(g_BotLibrary == 0)
		g_BotLibrary = Omnibot_LL( OB_VA("%s.dll", lib) );
	if(g_BotLibrary == 0)
	{
		g_OmnibotLibPath.clear();
		r = BOT_ERROR_CANTLOADDLL;
	}
	else
	{
		Omnibot_Load_PrintMsg(OB_VA("Found Omni-bot: %s, Attempting to Initialize", g_OmnibotLibPath.c_str()));
		pfnGetFunctionsFromDLL pfnGetBotFuncs = 0;
		memset(&g_BotFunctions, 0, sizeof(g_BotFunctions));
		pfnGetBotFuncs = (pfnGetFunctionsFromDLL)GetProcAddress(g_BotLibrary, "ExportBotFunctionsFromDLL");
		if(pfnGetBotFuncs == 0)
		{
			r = BOT_ERROR_CANTGETBOTFUNCTIONS;
		} 
		else
		{
			r = pfnGetBotFuncs(&g_BotFunctions, sizeof(g_BotFunctions));
			if(r == BOT_ERROR_NONE)
			{
				Omnibot_Load_PrintMsg("Omni-bot Loaded Successfully");
				r = g_BotFunctions.pfnBotInitialise(g_InterfaceFunctions, version);
				g_IsOmnibotLoaded = (r == BOT_ERROR_NONE);
			}
			else
			{
				Omnibot_Load_PrintErr(OB_VA("Omni-bot Failed with Error: %s", Omnibot_ErrorString(r)));
				Omnibot_FreeLibrary();
			}
		}
	}
	return r;
}

void Omnibot_FreeLibrary()
{
	if(g_BotLibrary)
	{
		FreeLibrary(g_BotLibrary);
		g_BotLibrary = 0;
	}
	memset(&g_BotFunctions, 0, sizeof(g_BotFunctions));
	
	delete g_InterfaceFunctions;
	g_InterfaceFunctions = 0;
	
	g_IsOmnibotLoaded = false;
}

#elif defined __linux__

#include <stdarg.h>

//////////////////////////////////////////////////////////////////////////
// Utilities

const char *OB_VA(const char* _msg, ...)
{
	static int iCurrentBuffer = 0;
	const int iNumBuffers = 3;
	const int BUF_SIZE = 1024;
	struct BufferInstance
	{
		char buffer[BUF_SIZE];
	};
	static BufferInstance buffers[iNumBuffers];

	char *pNextBuffer = buffers[iCurrentBuffer].buffer;

	va_list list;
	va_start(list, _msg);
	vsnprintf(pNextBuffer, sizeof(buffers[iCurrentBuffer].buffer), _msg, list);	
	va_end(list);

	iCurrentBuffer = (iCurrentBuffer+1)%iNumBuffers;
	return pNextBuffer;
}

#include <dlfcn.h>
#define GetProcAddress dlsym
#define NULL 0

//////////////////////////////////////////////////////////////////////////	
void *g_BotLibrary = NULL;

bool OB_ShowLastError(const char *context, const char *errormsg)
{
	Omnibot_Load_PrintErr(OB_VA("%s Failed with Error: %s", context, errormsg?errormsg:"<unknown error>"));
	return true;
}

void *Omnibot_LL(const char *file)
{
	g_OmnibotLibPath = file;
	Omnibot_Load_PrintMsg(OB_VA("Looking for %s", g_OmnibotLibPath.c_str()));
	void *pLib = dlopen(g_OmnibotLibPath.c_str(), RTLD_NOW);
	if(!pLib)
		OB_ShowLastError("LoadLibrary", dlerror());
	return pLib;
}

eomnibot_error Omnibot_LoadLibrary(int version, const char *lib, const char *path)
{
	eomnibot_error r = BOT_ERROR_NONE;

	const char *pError = 0;
	g_BotLibrary = Omnibot_LL(OB_VA("%s/%s.so", path ? path : ".", lib));
	if(!g_BotLibrary)
	{
		g_BotLibrary = Omnibot_LL(OB_VA("./%s.so", lib));
	}
	if(!g_BotLibrary)
	{
		char *homeDir = getenv("HOME");
		if(homeDir)
			g_BotLibrary = Omnibot_LL(OB_VA("%s/omni-bot/%s.so", homeDir, lib));
	}
	if(!g_BotLibrary)
	{
		char *homeDir = getenv("HOME");
		if(homeDir)
			g_BotLibrary = Omnibot_LL(OB_VA("%s.so", lib));
	}
	if(!g_BotLibrary)
	{
		g_OmnibotLibPath.clear();
		r = BOT_ERROR_CANTLOADDLL;
	}
	else
	{
		Omnibot_Load_PrintMsg(OB_VA("Found Omni-bot: %s, Attempting to Initialize", g_OmnibotLibPath.c_str()));
		pfnGetFunctionsFromDLL pfnGetBotFuncs = 0;
		memset(&g_BotFunctions, 0, sizeof(g_BotFunctions));
		pfnGetBotFuncs = (pfnGetFunctionsFromDLL)GetProcAddress(g_BotLibrary, "ExportBotFunctionsFromDLL");
		if(!pfnGetBotFuncs)
		{
			OB_ShowLastError("GetProcAddress", dlerror());
			r = BOT_ERROR_CANTGETBOTFUNCTIONS;
		}
		else
		{
			r = pfnGetBotFuncs(&g_BotFunctions, sizeof(g_BotFunctions));
			if(r == BOT_ERROR_NONE)
			{
				Omnibot_Load_PrintMsg("Omni-bot Loaded Successfully");
				r = g_BotFunctions.pfnBotInitialise(g_InterfaceFunctions, version);
				g_IsOmnibotLoaded = (r == BOT_ERROR_NONE);
			}
		}
	}
	return r;
}

void Omnibot_FreeLibrary()
{
	if(g_BotLibrary)
	{
		dlclose(g_BotLibrary);
		g_BotLibrary = 0;
	}
	memset(&g_BotFunctions, 0, sizeof(g_BotFunctions));
	memset(&g_InterfaceFunctions, 0, sizeof(g_InterfaceFunctions));
	g_IsOmnibotLoaded = false;
}

//////////////////////////////////////////////////////////////////////////

#else

#error "Unsupported Platform or Missing platform #defines";

#endif
