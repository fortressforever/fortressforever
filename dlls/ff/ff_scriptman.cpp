
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

// custom game modes made so damn easy
ConVar sv_mapluasuffix( "sv_mapluasuffix", "0", FCVAR_ARCHIVE, "Have a custom lua file (game mode) loaded when the map loads. If this suffix string is set, maps\\mapname__suffix__.lua (if it exists) is used instead of maps\\mapname.lua. To reset this cvar, make it 0.");
ConVar sv_luaglobalscript( "sv_globalluascript", "0", FCVAR_ARCHIVE, "Load a custom lua file globally after map scripts. Will overwrite map script. Will be loaded from maps\\globalscripts. To disable, set to 0.");

/////////////////////////////////////////////////////////////////////////////
using namespace luabind;

/////////////////////////////////////////////////////////////////////////////
// globals
CFFScriptManager _scriptman;

/////////////////////////////////////////////////////////////////////////////
CFFScriptManager::CFFScriptManager()
: L(NULL)
, m_isLoading(false)
, m_scriptCRC(0)
, m_ScriptExists(false)
{
}

/////////////////////////////////////////////////////////////////////////////
CFFScriptManager::~CFFScriptManager()
{
	Shutdown();
}

/////////////////////////////////////////////////////////////////////////////
bool CFFScriptManager::LoadFile( lua_State *L, const char *filename)
{
	VPROF_BUDGET( "CFFScriptManager::LoadFile", VPROF_BUDGETGROUP_FF_LUA );

	// don't allow scripters to sneak in scripts after the initial load
	if(!_scriptman.m_isLoading)
	{
		Warning("[SCRIPT] Loading of scripts after initial map load is not allowed.\n");
		return false;
	}

	// open the file
	Msg("[SCRIPT] Loading Lua File: %s\n", filename);
	FileHandle_t hFile = filesystem->Open(filename, "rb", "MOD");

	if (!hFile)
	{
		Warning("[SCRIPT] %s either does not exist or could not be opened.\n", filename);
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

	// preprocess script data
	_scriptman.OnScriptLoad(filename, buffer);

	// load script
	int errorCode = luaL_loadbuffer(L, buffer, fileSize, filename);

	// check if load was successful
	if(errorCode)
	{
		if(errorCode == LUA_ERRSYNTAX )
		{
			// syntax error, lookup description for the error
			const char *error = lua_tostring(L, -1);
			if (error)
			{
				Warning("Error loading %s: %s\n", filename, error);
				lua_pop( L, 1 );
			}
			else
				Warning("Unknown Syntax Error loading %s\n", filename);
		}
		else
		{
			Msg("Unknown Error loading %s\n", filename);
		}
		return false;
	}

	// execute script. script at top scrop gets exectued
	lua_pcall(L, 0, 0, 0);
	Msg( "[SCRIPT] Successfully loaded %s\n", filename );

	// cleanup
	MemFreeScratch();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::Init()
{
	// shutdown VM if already running
	if(L)
	{
		lua_close(L);
		L = NULL;
	}

	// initialize VM
	Msg("[SCRIPT] Attempting to start up the entity system...\n");
	L = lua_open();

	// no need to continue if VM failed to initialize
	if(!L)
	{
		Msg("[SCRIPT] Unable to initialize Lua VM.\n");
		return;
	}

	// load base libraries
	luaopen_base(L);
	luaopen_table(L);
	luaopen_string(L);
	luaopen_math(L);

	// initialize luabind
	luabind::open(L);

	// initialize game-specific library
	CFFLuaLib::Init(L);
	
	Msg("[SCRIPT] Entity system initialization successful.\n");
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::Shutdown()
{
	m_ScriptExists = false;

	// shtutdown VM
	if(L)
	{
		lua_close(L);
		L = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
extern bool g_Disable_Timelimit;

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
	BeginScriptLoad();
	LoadFile(L, "maps/includes/base.lua");

	char filename[256] = {0};
	char globalscript_filename[256] = {0};
	// Even though LoadFile already checks to see if the file exists, we'll check now so at least the default map lua file is loaded.
	// That way servers can keep their suffix set without worrying about every map having whatever game mode they always want to use.
	if ( sv_mapluasuffix.GetString()[0] != 0 )
	{
		Msg( "[SCRIPT] sv_mapluasuffix set to %s | finding maps\\%s__%s__.lua\n", sv_mapluasuffix.GetString(), szMapName, sv_mapluasuffix.GetString() );
		if ( filesystem->FileExists( UTIL_VarArgs( "maps/%s__%s__.lua", szMapName, sv_mapluasuffix.GetString() ) ) )
		{
			Q_snprintf( filename, sizeof(filename), "maps/%s__%s__.lua", szMapName, sv_mapluasuffix.GetString() );
			Msg( "[SCRIPT] maps\\%s__%s__.lua found\n", szMapName, sv_mapluasuffix.GetString() );
		}
		else
		{
			Msg( "[SCRIPT] maps\\%s__%s__.lua not found | reverting to maps\\%s.lua\n", szMapName, sv_mapluasuffix.GetString(), szMapName);
		}
	}

	// Load global include script, overwriting previously loaded stuff per map
	if( sv_luaglobalscript.GetString()[0] != 0 )
	{
		const char* scriptname = sv_luaglobalscript.GetString();
		Msg("[SCRIPT] sv_luaglobalscript set to %s | loading global script maps maps\\globalscripts\\%s.lua\n", scriptname, scriptname );
		if( filesystem->FileExists( UTIL_VarArgs( "maps/globalscripts/%s.lua", scriptname ) ) )
		{
			Q_snprintf( globalscript_filename, sizeof(globalscript_filename), "maps/globalscripts/%s.lua", scriptname );
			Msg("[SCRIPT] maps\\globalscripts\\%s.lua found\n", scriptname );\
		}
		else
		{
			Msg("[SCRIPT] global script maps\\globalscripts\\%s.lua not found - nothing loaded post map lua.\n", scriptname );
		}
	}

	if ( !filename[0] )
		Q_snprintf( filename, sizeof(filename), "maps/%s.lua", szMapName );

	//////////////////////////////////////////////////////////////////////////
	// Try a precache, rumor has it this will cause the engine to send the lua files to clients
	if(PRECACHE_LUA_FILES)
	{
		V_FixSlashes(filename);
		if(filesystem->FileExists(filename))
		{
			Util_AddDownload(filename);

			if(!engine->IsGenericPrecached(filename))
				engine->PrecacheGeneric(filename, true);
		} 
		else // - if no map lua is found, send default (for testing mainly)
		{
			// no check - this file should *always* be there
			Util_AddDownload(default_luafile);
			if(!engine->IsGenericPrecached(default_luafile))
				engine->PrecacheGeneric(default_luafile, true);
		}
		

		// if we have a globalscript, precache it as well
		if( sv_luaglobalscript.GetString()[0] != 0 && globalscript_filename[0] )
		{
			V_FixSlashes(globalscript_filename);
			if(filesystem->FileExists(globalscript_filename))
			{
				Util_AddDownload(globalscript_filename);

				if(!engine->IsGenericPrecached(globalscript_filename))
					engine->PrecacheGeneric(globalscript_filename, true);
			}
		}

		//////////////////////////////////////////////////////////////////////////
		/*char testfile[] = {"maps/ff_dm.txt"};
		V_FixSlashes(testfile);
		if(filesystem->FileExists(testfile))
		{
			Util_AddDownload(testfile);

			if(!engine->IsGenericPrecached(testfile))
				engine->PrecacheGeneric(testfile, true);
		}*/
		//////////////////////////////////////////////////////////////////////////
	}
	//////////////////////////////////////////////////////////////////////////
	if(filesystem->FileExists(filename))
		m_ScriptExists = LoadFile(L, filename);
	else
	{
		Msg("[SCRIPT] File %s not found! Loaded fallback lua %s\n", filename, default_luafile);
		m_ScriptExists = LoadFile(L, default_luafile);
	}

	EndScriptLoad();

	// force loading global script in another call :/
	if( sv_luaglobalscript.GetString()[0] != 0 && globalscript_filename[0] )
	{
		BeginScriptLoad();
		LoadFile(L, globalscript_filename);
		EndScriptLoad();
	}

	// spawn the helper entity
	CFFEntitySystemHelper::Create();
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::LevelShutdown()
{
	// shutdown the VM
	if(L)
	{
		lua_close(L);
		L = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::OnScriptLoad(const char* szFileName,
								   const char* szFileContents)
{
	// ignore the message if we are not still in the "loading" phase
	if(!m_isLoading)
		return;

	// compute checksums of file contents
	CRC32_ProcessBuffer(&m_scriptCRC,
						szFileContents,
						strlen(szFileContents));
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::BeginScriptLoad()
{
	CRC32_Init(&m_scriptCRC);
	m_isLoading = true;
}

/////////////////////////////////////////////////////////////////////////////
void CFFScriptManager::EndScriptLoad()
{
	CRC32_Final(&m_scriptCRC);
	m_isLoading = false;
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

/////////////////////////////////////////////////////////////////////////////
int CFFScriptManager::RunPredicates( CBaseEntity *ent, CBaseEntity *player, const char *addname )
{
	VPROF_BUDGET( "CFFScriptManager::RunPredicates", VPROF_BUDGETGROUP_FF_LUA );

	Msg( "[RunPredicates] Shit is deprecated!\n" );
	return true;

	// If there is no active script then allow the ents to always go
	if( !m_ScriptExists || !L )
		return true;

	int player_id = player ? ENTINDEX( player ) : -1;
	int ent_id = ent ? ENTINDEX( ent ) : -1;

	SetVar("entid", ent_id);

	// set lua's reference to the calling entity
	luabind::object globals = luabind::globals(L);
	globals["entity"] = luabind::object(L, ent);

	// FryGuy: removed the classname from the function -- mappers shouldn't use conflicting names
	// because it makes things confusing, and also typing out the class name for each func is bad.
	//strcpy( func, ent->GetClassname() );
	//strcat( func, "_" );
	if (ent)
	{
		if (!addname || !strlen(addname))
		{
			Msg("Can't call entsys.runpredicates with an entity and no addname\n");
			return true /* mirv: let it cont. regardless */;
		}

		if (!strlen(STRING(ent->GetEntityName())))
		{
			// REMOVED: Really annoying to see this
			//Warning( "[entsys] ent did not have an entity name!\n" );
			return true /* mirv: let it cont. regardless */;
		}

		SetVar("entname", STRING(ent->GetEntityName()));

		// push the function onto stack ( entname:addname )
		lua_getglobal( L, STRING(ent->GetEntityName()) );
		if (lua_isnil(L, -1))
		{
			//Warning("Table '%s' doesn't exist!\n", STRING(ent->GetEntityName()) );
			lua_pop(L, 1);
			return true /* mirv: let it cont. regardless */;
		}
		lua_pushstring( L, addname );
		lua_gettable( L, -2 );
		lua_insert( L, -2 );

	}
	else
	{
		// Get the function
		lua_getglobal( L, addname );
	}

	// The first argument is the player #
	lua_pushnumber( L, player_id );

	// 1 argument, 1 result, escape cleanly if it breaks
	if( lua_pcall( L, (ent)?2:1, 1, 0 ) != 0 )
	{
		// TODO: Definately don't want this message showing up permantly as it's kind of deceptive
		// AND: should this return value be a parameter of run predicates? (so it'll work right w/ FFScriptRunPredicates?
		Msg( "[SCRIPT] Error calling %s (%s) ent: %s\n", addname, lua_tostring(L, -1), ent ? STRING( ent->GetEntityName() ) : "NULL" );
		return true;
	}

	// Get the result
	int retVal;
	if (lua_isboolean( L, -1 ))
		retVal = (int)lua_toboolean( L, -1 );
	else
		retVal = (int)lua_tonumber( L, -1 );
	
	lua_pop( L, 1 );

	//DevMsg( "[SCRIPT] %s returns: %d\n", func, retVal );

	// Life carries on for normal if return's 0
	return retVal;
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
