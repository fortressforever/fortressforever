
/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "ff_scriptman.h"
#include "ff_entity_system.h"
#include "ff_luacontext.h"
#include "ff_lualib.h"
#include "ff_utils.h"
#include "ff_item_flag.h"
#include "triggers.h"

// engine
#include "filesystem.h"

// lua
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#undef MINMAX_H
#undef min
#undef max

// luabind
#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "luabind/iterator_policy.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// extern globals
extern bool g_Disable_Timelimit;

// custom game modes made so damn easy
ConVar sv_mapluasuffix( "sv_mapluasuffix", "", FCVAR_ARCHIVE, "Have a custom lua file (game mode) loaded when the map loads. If this suffix string is set, maps\\mapname__suffix__.lua (if it exists) is used instead of maps\\mapname.lua. To reset this cvar, make it \"\".");
ConVar sv_globalluascript( "sv_globalluascript", "", FCVAR_ARCHIVE, "Load a custom lua file globally after map scripts. Will overwrite map script. Will be loaded from maps\\globalscripts. To disable, set to \"\".");

// redirect Lua's print function to the console
static int print(lua_State *L)
{
	int n=lua_gettop(L);
	int i;
	for (i=1; i<=n; i++)
	{
	if (i>1) Msg("\t");
	if (lua_isstring(L,i))
		Msg("%s",lua_tostring(L,i));
	else if (lua_isnil(L,i))
		Msg("%s","nil");
	else if (lua_isboolean(L,i))
		Msg("%s",lua_toboolean(L,i) ? "true" : "false");
	else
		Msg("%s:%p",luaL_typename(L,i),lua_topointer(L,i));
	}
	Msg("\n");
	return 0;
}

using namespace luabind;

// global script manager instance
CFFScriptManager _scriptman;

CFFScriptManager::CFFScriptManager()
{
	L = NULL;
}

CFFScriptManager::~CFFScriptManager()
{
	Shutdown();
}

/** Close the Lua VM
*/
void CFFScriptManager::Shutdown()
{
	if(L)
	{
		lua_close(L);
		L = NULL;
	}
}

/** Open the Lua VM
	@returns True if successful, false if couldn't open Lua VM
*/
bool CFFScriptManager::Init()
{
	// shutdown VM if already running
	Shutdown();

	// initialize VM
	LuaMsg("Attempting to start the Lua VM...\n");
	L = lua_open();

	// no need to continue if VM failed to initialize
	if(!L)
	{
		LuaMsg("Unable to initialize Lua VM.\n");
		return false;
	}
	
	// initialize all the FF specific stuff
	SetupEnvironmentForFF();

	// make the standard libraries safe
	MakeEnvironmentSafe();

	LuaMsg("Lua VM initialization successful.\n");
	return true;
}

/** Loads the Lua libraries and sets all the variables needed for FF
*/
void CFFScriptManager::SetupEnvironmentForFF()
{
	Assert(L);
	if (!L) return;

	// load all libraries
	luaL_openlibs(L);

	// overwrite Lua's print with our own
	lua_register(L,"print",print);

	// set package.path to the mod search dirs, so that we can use require
	char szModSearchPaths[4096] = {0};
	// FF TODO: This is set to ignore pack files; might need to add support for them (for custom mods packaged as .vpks?)
	filesystem->GetSearchPath( "MOD", false, szModSearchPaths, sizeof(szModSearchPaths) );
	
	char szLuaSearchPaths[4096] = {0};
	for ( char *szSearchPath = strtok( szModSearchPaths, ";" ); szSearchPath; szSearchPath = strtok( NULL, ";" ) )
	{
		// check maps/includes/ first
		char fullpath[MAX_PATH];
		Q_snprintf( fullpath, sizeof( fullpath ), "%s%s%s", szSearchPath, "maps/includes/", "?.lua;" );
		Q_FixSlashes( fullpath );

		Q_strncat( szLuaSearchPaths, fullpath, sizeof(szLuaSearchPaths) );

		// check maps/ second
		Q_snprintf( fullpath, sizeof( fullpath ), "%s%s%s", szSearchPath, "maps/", "?.lua;" );
		Q_FixSlashes( fullpath );

		Q_strncat( szLuaSearchPaths, fullpath, sizeof(szLuaSearchPaths) );

		// check the mod root last
		Q_snprintf( fullpath, sizeof( fullpath ), "%s%s", szSearchPath, "?.lua;" );
		Q_FixSlashes( fullpath );

		Q_strncat( szLuaSearchPaths, fullpath, sizeof(szLuaSearchPaths) );
	}
	
	// set package.path; this is equivelent to the Lua code: _G.package["path"] = szLuaSearchPaths
	lua_pushstring(L, "package");
	lua_gettable(L, LUA_GLOBALSINDEX); // get _G.package
	lua_pushstring(L, "path");
	lua_pushstring(L, szLuaSearchPaths);
	lua_settable(L, -3); // -3 is the package table
	lua_pop(L, 1); // pop _G.package

	// initialize luabind
	luabind::open(L);

	// initialize game-specific library
	CFFLuaLib::Init(L);
}

/** Gets rid of or alter any unsafe Lua functions in the current environment
*/
void CFFScriptManager::MakeEnvironmentSafe()
{
	Assert(L);
	if (!L) return;

	// See: http://lua-users.org/wiki/SandBoxes for a general overview of the safety of Lua functions

	// dofile, load, loadfile, loadstring
	const char* ppszUnsafeGlobalFunctions[] = { "dofile", "load", "loadfile", "loadstring", NULL };
	RemoveVarsFromGlobal( L, ppszUnsafeGlobalFunctions );

	// os.*
	const char* ppszUnsafeOSFunctions[] = { "execute", "exit", "getenv", "remove", "rename", "setlocale", "tmpname", NULL };
	RemoveKeysFromGlobalTable( L, LUA_OSLIBNAME, ppszUnsafeOSFunctions );

	// package.*
	const char* ppszUnsafePackageFunctions[] = { "loadlib", NULL };
	RemoveKeysFromGlobalTable( L, LUA_LOADLIBNAME, ppszUnsafePackageFunctions );

	// io.* gives access to the filesystem, just remove it totally
	const char* ppszUnsafeLibraries[] = { "io", NULL };
	RemoveVarsFromGlobal( L, ppszUnsafeLibraries );

	// require can load .dll/.so, need to disable the loaders that search for them
	// the third index checks package.cpath and loads .so/.dll files
	// the fourth index is an all-in-one loader that can load .so/.dll files
	lua_getglobal(L, LUA_LOADLIBNAME);
	lua_pushstring(L, "loaders");
	lua_gettable(L, -2); // get _G.package.loaders
	lua_pushnil(L);
	lua_rawseti(L, -2, 4); // _G.package.loaders[4] = nil
	lua_pushnil(L);
	lua_rawseti(L, -2, 3); // _G.package.loaders[3] = nil
	lua_pop(L, 2); // pop _G.package.loaders and _G.package
}

/** Loads a Lua file into a function that is pushed on the top of the Lua stack (only when the file is succesfully loaded)
	@returns True if file is successfully loaded, false if there were any errors
*/
bool CFFScriptManager::LoadFileIntoFunction( const char *filename )
{
	VPROF_BUDGET( "CFFScriptManager::LoadFileIntoFunction", VPROF_BUDGETGROUP_FF_LUA );

	// open the file
	LuaMsg("Loading Lua File: %s\n", filename);
	FileHandle_t hFile = filesystem->Open(filename, "rb", "MOD");

	if (!hFile)
	{
		LuaWarning("%s either does not exist or could not be opened.\n", filename);
		return false;
	}

	// allocate buffer for file contents
	int fileSize = filesystem->Size(hFile);
	char *buffer = (char*)MemAllocScratch(fileSize + 1);
	Assert(buffer);
		
	// load file contents into a null-terminated buffer
	filesystem->Read(buffer, fileSize, hFile);
	buffer[fileSize] = 0;
	filesystem->Close(hFile);
	
	// load the buffer into a function that is pushed to the top of the stack
	int errorCode = luaL_loadbuffer(L, buffer, fileSize, filename);
	
	// cleanup buffer
	MemFreeScratch();

	// check if load was successful
	if (errorCode != 0)
	{
		const char *error = lua_tostring(L, -1);
		LuaWarning( "Error loading %s: %s\n", filename, error );
		lua_pop( L, 1 );
		return false;
	}
	
	return true;
}

/** Loads a Lua file into the current environment relative to a "MOD" search path
	@returns True if file successfully loaded, false if there were any errors (syntax or execution)
*/
bool CFFScriptManager::LoadFile(const char *filename)
{
	VPROF_BUDGET( "CFFScriptManager::LoadFile", VPROF_BUDGETGROUP_FF_LUA );

	if (!LoadFileIntoFunction( filename ))
		return false;

	// execute the loaded function
	int errorCode = lua_pcall(L, 0, 0, 0);
	
	// check if execution was successful
	if (errorCode != 0)
	{
		const char *error = lua_tostring(L, -1);
		LuaWarning( "Error loading %s: %s\n", filename, error );
		lua_pop( L, 1 );
		return false;
	}

	LuaMsg( "Successfully loaded %s\n", filename );
	return true;
}

void CFFScriptManager::LevelInit(const char* szMapName)
{
	const char* default_luafile = "maps/default.lua";
	VPROF_BUDGET("CFFScriptManager::LevelInit", VPROF_BUDGETGROUP_FF_LUA);

	if(!szMapName)
		return;

	g_Disable_Timelimit = false;

	// setup VM
	Init();

	// load lua files
	LoadFile("maps/includes/base.lua");

	char filename[256] = {0};
	char globalscript_filename[256] = {0};

	if ( sv_mapluasuffix.GetString()[0] != 0 )
	{
		LuaMsg( "sv_mapluasuffix set to %s | loading maps\\%s__%s__.lua\n", sv_mapluasuffix.GetString(), szMapName, sv_mapluasuffix.GetString() );

		if ( filesystem->FileExists( UTIL_VarArgs( "maps/%s__%s__.lua", szMapName, sv_mapluasuffix.GetString() ) ) )
		{
			Q_snprintf( filename, sizeof(filename), "maps/%s__%s__.lua", szMapName, sv_mapluasuffix.GetString() );
		}
		else
		{
			LuaMsg( "maps\\%s__%s__.lua not found | reverting to maps\\%s.lua\n", szMapName, sv_mapluasuffix.GetString(), szMapName);
		}
	}

	if ( !filename[0] )
		Q_snprintf( filename, sizeof(filename), "maps/%s.lua", szMapName );

	if(filesystem->FileExists(filename))
		LoadFile(filename);
	else
	{
		LuaWarning("File %s not found! Loaded fallback lua %s\n", filename, default_luafile);
		LoadFile(default_luafile);
	}

	// Load global include script, overwriting previously loaded stuff per map
	if( sv_globalluascript.GetString()[0] != 0 )
	{
		const char* szGlobalLuaScript = sv_globalluascript.GetString();
		LuaMsg("sv_globalluascript set to %s | loading maps\\globalscripts\\%s.lua\n", szGlobalLuaScript, szGlobalLuaScript );

		if( filesystem->FileExists( UTIL_VarArgs( "maps/globalscripts/%s.lua", szGlobalLuaScript ) ) )
		{
			Q_snprintf( globalscript_filename, sizeof(globalscript_filename), "maps/globalscripts/%s.lua", szGlobalLuaScript );
			LoadFile(globalscript_filename);
		}
		else
		{
			LuaMsg("global script maps\\globalscripts\\%s.lua not found - nothing loaded post map lua.\n", szGlobalLuaScript );
		}
	}

	// spawn the helper entity
	CFFEntitySystemHelper::Create();
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::LevelShutdown()
{
	Shutdown();
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::SetVar( lua_State *L, const char *name, const char *value )
{
	lua_pushstring(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::SetVar( lua_State *L, const char *name, int value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::SetVar( lua_State *L, const char *name, float value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::SetVar( const char *name, const char *value )
{
	lua_pushstring(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::SetVar( const char *name, int value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::SetVar( const char *name, float value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
const char *CFFScriptManager::GetString( const char *name )
{
	lua_getglobal(L, name);
	const char *ret = lua_tostring(L, -1);
	lua_pop(L, 1);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
int CFFScriptManager::GetInt( const char *name )
{
	lua_getglobal(L, name);
	int ret = (int)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
float CFFScriptManager::GetFloat( const char *name )
{
	lua_getglobal(L, name);
	float ret = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
bool CFFScriptManager::GetObject(CBaseEntity* pEntity, luabind::adl::object& outObject)
{
	VPROF_BUDGET( "CFFScriptManager::GetObject", VPROF_BUDGETGROUP_FF_LUA );

	if(!pEntity)
		return false;

	// lookup the object using the entity's name
	const char* szEntName = STRING(pEntity->GetEntityName());
	return GetObject(szEntName, outObject);
}

/////////////////////////////////////////////////////////////////////////////
bool CFFScriptManager::GetObject(const char* szTableName, luabind::adl::object& outObject)
{
	VPROF_BUDGET( "CFFScriptManager::GetObject", VPROF_BUDGETGROUP_FF_LUA );

	if(NULL == szTableName)
		return false;

	try
	{
		// lookup the table in the global scope
		luabind::adl::object retObject = luabind::globals(L)[szTableName];
		if(luabind::type(retObject) == LUA_TTABLE)
		{
			outObject = retObject;
			return true;
		}
	}
	catch(...)
	{
		// throw the error away, lets just keep the game running
		return false;
	}

	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool CFFScriptManager::GetFunction(CBaseEntity* pEntity,
								  const char* szFunctionName,
								  luabind::adl::object& outObject)
{
	VPROF_BUDGET( "CFFScriptManager::GetFunction", VPROF_BUDGETGROUP_FF_LUA );

	if(NULL == pEntity || NULL == szFunctionName)
		return false;

	luabind::adl::object tableObject;
	if(GetObject(pEntity, tableObject))
		return GetFunction(tableObject, szFunctionName, outObject);

	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool CFFScriptManager::GetFunction(luabind::adl::object& tableObject,
								  const char* szFunctionName,
								  luabind::adl::object& outObject)
{
	VPROF_BUDGET( "CFFScriptManager::GetFunction", VPROF_BUDGETGROUP_FF_LUA );

	if(NULL == szFunctionName)
		return false;

	if(luabind::type(tableObject) != LUA_TTABLE)
		return false;

	try
	{
		luabind::adl::object funcObject = tableObject[szFunctionName];
		if(luabind::type(funcObject) == LUA_TFUNCTION)
		{
			outObject = funcObject;
			return true;
		}
	}
	catch(...)
	{
		return false;
	}

	return false;
}

void CFFScriptManager::RemoveVarsFromGlobal( lua_State *L, const char** ppszVars )
{
	for( int i=0; ppszVars[i] != NULL; ++i )
	{
		lua_pushnil(L);
		lua_setglobal(L, ppszVars[i]);
	}
}

void CFFScriptManager::RemoveKeysFromGlobalTable( lua_State *L, const char *pszTableName, const char** ppszKeys )
{
	lua_getglobal(L, pszTableName);
	if (lua_type(L, -1) == LUA_TTABLE)
	{
		for( int i=0; ppszKeys[i] != NULL; ++i )
		{
			lua_pushstring(L, ppszKeys[i]);
			lua_pushnil(L);
			lua_settable(L, -3);
		}
	}
	lua_pop(L, 1);
}

/////////////////////////////////////////////////////////////////////////////
bool FFScriptRunPredicates( CBaseEntity *pObject, const char *pszFunction, bool bExpectedVal )
{
	VPROF_BUDGET( "FFScriptRunPredicates", VPROF_BUDGETGROUP_FF_LUA );

	if( pObject && pszFunction )
	{
		for( int i = 0; i < pObject->m_hActiveScripts.Count(); i++ )
		{
			CBaseEntity *pEntity = UTIL_EntityByIndex( pObject->m_hActiveScripts[ i ] );
			if( pEntity )
			{
				bool bEntSys = bExpectedVal;
				//CFFLuaObjectWrapper hOutput;
				CFFLuaSC hOutput( 1, pObject );
				//bool bEntSys = entsys.RunPredicates_LUA( pEntity, pObject, pszFunction ) > 0;
				if( _scriptman.RunPredicates_LUA( pEntity, &hOutput, pszFunction ) )
					bEntSys = hOutput.GetBool();

				if( bEntSys != bExpectedVal )
					return !bExpectedVal;
			}
		}
	}

	return bExpectedVal;
}

// same as above, but uses a separate box
bool FFScriptRunPredicates( CBaseEntity *pObject, const char *pszFunction, bool bExpectedVal, Vector vecOrigin, Vector vecMins, Vector vecMaxs )
{
	VPROF_BUDGET( "FFScriptRunPredicates", VPROF_BUDGETGROUP_FF_LUA );

	if( pObject && pszFunction )
	{
		CBaseEntity *pList[ 128 ];
		int count = UTIL_EntitiesInBox( pList, 128, vecOrigin + vecMins, vecOrigin + vecMaxs, 0 );

		for( int i = 0; i < count; i++ )
		{
			CBaseEntity *pEntity = pList[i];
			if( pEntity )
			{
				if ( pEntity->Classify() == CLASS_TRIGGERSCRIPT )
				{
					CFuncFFScript *pFFScript = dynamic_cast<CFuncFFScript*>(pEntity);
					if ( pFFScript && pFFScript->IsRemoved() )
						continue;
				}
				else if ( pEntity->Classify() == CLASS_INFOSCRIPT )
				{
					CFFInfoScript *pFFScript = dynamic_cast<CFFInfoScript*>(pEntity);
					if ( pFFScript && pFFScript->IsRemoved() )
						continue;
				}
				else
					continue;

				bool bEntSys = bExpectedVal;
				//CFFLuaObjectWrapper hOutput;
				CFFLuaSC hOutput( 1, pObject );
				//bool bEntSys = entsys.RunPredicates_LUA( pEntity, pObject, pszFunction ) > 0;
				if( _scriptman.RunPredicates_LUA( pEntity, &hOutput, pszFunction ) )
					bEntSys = hOutput.GetBool();

				if( bEntSys != bExpectedVal )
					return !bExpectedVal;
			}
		}
	}

	return bExpectedVal;
}

// same as above, but uses a separate sphere
bool FFScriptRunPredicates( CBaseEntity *pObject, const char *pszFunction, bool bExpectedVal, Vector vecOrigin, float flRadius )
{
	VPROF_BUDGET( "FFScriptRunPredicates", VPROF_BUDGETGROUP_FF_LUA );

	if( pObject && pszFunction )
	{
		CBaseEntity *pList[ 128 ];
		int count = UTIL_EntitiesInSphere( pList, 128, vecOrigin, flRadius, 0 );

		for( int i = 0; i < count; i++ )
		{
			CBaseEntity *pEntity = pList[i];
			if( pEntity )
			{
				if ( pEntity->Classify() == CLASS_TRIGGERSCRIPT )
				{
					CFuncFFScript *pFFScript = dynamic_cast<CFuncFFScript*>(pEntity);
					if ( pFFScript && pFFScript->IsRemoved() )
						continue;
				}
				else if ( pEntity->Classify() == CLASS_INFOSCRIPT )
				{
					CFFInfoScript *pFFScript = dynamic_cast<CFFInfoScript*>(pEntity);
					if ( pFFScript && pFFScript->IsRemoved() )
						continue;
				}
				else
					continue;

				bool bEntSys = bExpectedVal;
				//CFFLuaObjectWrapper hOutput;
				CFFLuaSC hOutput( 1, pObject );
				//bool bEntSys = entsys.RunPredicates_LUA( pEntity, pObject, pszFunction ) > 0;
				if( _scriptman.RunPredicates_LUA( pEntity, &hOutput, pszFunction ) )
					bEntSys = hOutput.GetBool();

				if( bEntSys != bExpectedVal )
					return !bExpectedVal;
			}
		}
	}

	return bExpectedVal;
}

/////////////////////////////////////////////////////////////////////////////
// Purpose: Call into lua and get a result, basically. A better runpredicates
// Output : hOutput - the value returned from the lua object/function called
//
// Function return value: function will return true if it found the lua object/function
/////////////////////////////////////////////////////////////////////////////
bool CFFScriptManager::RunPredicates_LUA( CBaseEntity *pObject, CFFLuaSC *pContext, const char *szFunctionName )
{
	VPROF_BUDGET( "CFFScriptManager::RunPredicates_LUA", VPROF_BUDGETGROUP_FF_LUA );

	if( !pContext )
		return false;

	// Not sure if this is needed but we can have a case
	// where there won't be any params so just adding
	// a NULL param for the hell of it until I find out
	// it's alright.
	if( pContext->GetNumParams() == 0 )
	{
		CBaseEntity *pEntity = NULL;
		pContext->Push( pEntity );
	}

	if( pContext->CallFunction( pObject, szFunctionName ) )
		return true;

	return false;
}

/** Wrapper for Msg that prefixes the string with info about where it's coming from in the format: [SCRIPT]
*/
void CFFScriptManager::LuaMsg( const char *pszFormat, ... )
{
	va_list		argptr;
	static char		string[4096];
	
	va_start (argptr, pszFormat);
	Q_vsnprintf(string, sizeof(string), pszFormat, argptr);
	va_end (argptr);

	Msg("[SCRIPT] %s", string );
}

/** Wrapper for Warning that prefixes the string with info about where it's coming from in the format: [SCRIPT]
*/
void CFFScriptManager::LuaWarning( const char *pszFormat, ... )
{
	va_list		argptr;
	static char		string[4096];
	
	va_start (argptr, pszFormat);
	Q_vsnprintf(string, sizeof(string), pszFormat, argptr);
	va_end (argptr);

	Warning("[SCRIPT] %s", string );
}

CON_COMMAND( lua_dostring, "Run a server-side Lua string in the global environment" )
{
	if ( engine->Cmd_Argc() == 1 )
	{
		Msg( "Usage: lua_dostring <string>\n" );
		return;
	}

	lua_State *L = _scriptman.GetLuaState();
	int status = luaL_dostring(L, engine->Cmd_Args());
	if (status != 0) {
		Warning( "%s\n", lua_tostring(L, -1) );
		lua_pop(L, 1);
		return;
	}
	if (status == 0 && lua_gettop(L) > 0) {  /* any result to print? */
		lua_getglobal(L, "print");
		lua_insert(L, 1);
		if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0)
		Warning("%s", lua_pushfstring(L,
							"error calling " LUA_QL("print") " (%s)",
							lua_tostring(L, -1)));
	}
	lua_settop(L, 0);  /* clear stack */
}
