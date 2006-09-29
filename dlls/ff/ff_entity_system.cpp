/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_entity_system.cpp
/// @author Gavin Bramhill (Mirvin_Monkey)
/// @date 21 April 2005
/// @brief Handles the entity system
///
/// REVISIONS
/// ---------
/// Apr 21, 2005 Mirv: Begun
/// Jun 13, 2005 FryGuy: Added AddTeamScore
/// Jun 25, 2005 FryGuy: Added SpawnEntityAtPlayer, GetPlayerClass, GetPlayerName
/// Jun 29, 2005 FryGuy: Added PlayerHasItem, RemoveItem
/// Jul 10, 2005 FryGuy: Added ReturnItem
/// Jul 15, 2005 FryGuy: Changed ReturnItem to use a string instead of entity ID
/// Jul 31, 2005 FryGuy: Added the entity helper, along with the sound stuffs
/// Aug 01, 2005 FryGuy: Added BroadcastMessage and RespawnAllPlayers

/////////////////////////////////////////////////////////////////////////////
// includes
#include "cbase.h"
#include "ff_entity_system.h"
#include "ff_luacontext.h"
#include "ff_lualib.h"
#include "ff_scheduleman.h"
#include "ff_utils.h"

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

/////////////////////////////////////////////////////////////////////////////
// globals
CFFEntitySystem entsys;

using namespace luabind;

/////////////////////////////////////////////////////////////////////////////
// CFFEntitySystemHelper implementation
/////////////////////////////////////////////////////////////////////////////
LINK_ENTITY_TO_CLASS( ff_entity_system_helper, CFFEntitySystemHelper );

// Start of our data description for the class
BEGIN_DATADESC( CFFEntitySystemHelper )
	DEFINE_THINKFUNC( OnThink ),
END_DATADESC()

CFFEntitySystemHelper* CFFEntitySystemHelper::s_pInstance = NULL;

/////////////////////////////////////////////////////////////////////////////
// Purpose: Sets up the entity's initial state
/////////////////////////////////////////////////////////////////////////////
CFFEntitySystemHelper::CFFEntitySystemHelper()
{
	ASSERT(!s_pInstance);
	s_pInstance = this;
}

/////////////////////////////////////////////////////////////////////////////
CFFEntitySystemHelper::~CFFEntitySystemHelper()
{
	s_pInstance = NULL;
}

/////////////////////////////////////////////////////////////////////////////
CFFEntitySystemHelper* CFFEntitySystemHelper::GetInstance()
{
	ASSERT(s_pInstance);
	return s_pInstance;
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystemHelper::Spawn()
{
	DevMsg("[EntSys] Entity System Helper Spawned\n");

	SetThink( &CFFEntitySystemHelper::OnThink );
	SetNextThink( gpGlobals->curtime + 1.0f );
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystemHelper::OnThink()
{
	VPROF_BUDGET( "CFFEntitySystemHelper::OnThink", VPROF_BUDGETGROUP_FF_LUA );

	_scheduleman.Update();
	SetNextThink(gpGlobals->curtime + TICK_INTERVAL);
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystemHelper::Precache()
{
	VPROF_BUDGET( "CFFEntitySystemHelper::Precache", VPROF_BUDGETGROUP_FF_LUA );

	CFFLuaSC hPrecache;
	entsys.RunPredicates_LUA( NULL, &hPrecache, "precache" );
}

/////////////////////////////////////////////////////////////////////////////
CFFEntitySystemHelper* CFFEntitySystemHelper::Create()
{
	CFFEntitySystemHelper* pHelper = (CFFEntitySystemHelper*)CreateEntityByName("ff_entity_system_helper");
	pHelper->Spawn();
	pHelper->Precache();
	return pHelper;
}

/////////////////////////////////////////////////////////////////////////////
// CFFEntitySystem implementation
/////////////////////////////////////////////////////////////////////////////
CFFEntitySystem::CFFEntitySystem()
: m_isLoading(false)
, m_scriptCRC(0)
, m_ScriptExists(false)
{
}

/////////////////////////////////////////////////////////////////////////////
CFFEntitySystem::~CFFEntitySystem()
{
	Shutdown();
	_scheduleman.Shutdown();
}

/////////////////////////////////////////////////////////////////////////////
bool CFFEntitySystem::LoadFile( lua_State *L, const char *filename)
{
	VPROF_BUDGET( "CFFEntitySystem::LoadFile", VPROF_BUDGETGROUP_FF_LUA );

	// don't allow scripters to sneak in scripts after the initial load
	if(!entsys.m_isLoading)
	{
		DevMsg("[SCRIPT] Loading of scripts after inital map load is not allowed.\n");
		return false;
	}

	// open the file
	DevMsg("[SCRIPT] Loading Lua File: %s\n", filename);
	FileHandle_t hFile = filesystem->Open(filename, "rb", "MOD");

	if (!hFile)
	{
		DevWarning("[SCRIPT] %s either does not exist or could not be opened.\n", filename);
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
	entsys.OnScriptLoad(filename, buffer);

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
			Warning("Unknown Error loading %s\n", filename);
		}
		return false;
	}

	// execute script. script at top scrop gets exectued
	lua_pcall(L, 0, 0, 0);
	DevMsg( "[SCRIPT] Successfully loaded %s\n", filename );

	// cleanup
	MemFreeScratch();
	return true;
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::Init()
{
	// shutdown VM if already running
	if(L)
	{
		lua_close(L);
		L = NULL;
	}

	// initialize VM
	DevMsg("[SCRIPT] Attempting to start up the entity system...\n");
	L = lua_open();

	// no need to continue if VM failed to initialize
	if(!L)
	{
		DevWarning("[SCRIPT] Unable to initialize Lua VM.\n");
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
	
	DevMsg("[SCRIPT] Entity system initialization successful.\n");
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::Shutdown()
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
void CFFEntitySystem::LevelInit(const char* szMapName)
{
	VPROF_BUDGET("CFFEntitySystem::LevelInit", VPROF_BUDGETGROUP_FF_LUA);

	if(!szMapName)
		return;

	// setup VM
	Init();
	
	// load lua files
	BeginScriptLoad();
	LoadFile(L, "maps/includes/base.lua");

	char filename[128];
	Q_snprintf(filename, sizeof(filename), "maps/%s.lua", szMapName);
	m_ScriptExists = LoadFile(L, filename);
	EndScriptLoad();

	// spawn the helper entity
	CFFEntitySystemHelper::Create();
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::LevelShutdown()
{
	// shutdown the VM
	if(L)
	{
		lua_close(L);
		L = NULL;
	}
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::OnScriptLoad(const char* szFileName,
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
void CFFEntitySystem::BeginScriptLoad()
{
	CRC32_Init(&m_scriptCRC);
	m_isLoading = true;
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::EndScriptLoad()
{
	CRC32_Final(&m_scriptCRC);
	m_isLoading = false;
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::SetVar( lua_State *L, const char *name, const char *value )
{
	lua_pushstring(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::SetVar( lua_State *L, const char *name, int value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::SetVar( lua_State *L, const char *name, float value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::SetVar( const char *name, const char *value )
{
	lua_pushstring(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::SetVar( const char *name, int value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
void CFFEntitySystem::SetVar( const char *name, float value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

/////////////////////////////////////////////////////////////////////////////
const char *CFFEntitySystem::GetString( const char *name )
{
	lua_getglobal(L, name);
	const char *ret = lua_tostring(L, -1);
	lua_pop(L, 1);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
int CFFEntitySystem::GetInt( const char *name )
{
	lua_getglobal(L, name);
	int ret = (int)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
float CFFEntitySystem::GetFloat( const char *name )
{
	lua_getglobal(L, name);
	float ret = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return ret;
}

/////////////////////////////////////////////////////////////////////////////
bool CFFEntitySystem::GetObject(CBaseEntity* pEntity, luabind::adl::object& outObject)
{
	VPROF_BUDGET( "CFFEntitySystem::GetObject", VPROF_BUDGETGROUP_FF_LUA );

	if(!pEntity)
		return false;

	// lookup the object using the entity's name
	const char* szEntName = STRING(pEntity->GetEntityName());
	return GetObject(szEntName, outObject);
}

/////////////////////////////////////////////////////////////////////////////
bool CFFEntitySystem::GetObject(const char* szTableName, luabind::adl::object& outObject)
{
	VPROF_BUDGET( "CFFEntitySystem::GetObject", VPROF_BUDGETGROUP_FF_LUA );

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
bool CFFEntitySystem::GetFunction(CBaseEntity* pEntity,
								  const char* szFunctionName,
								  luabind::adl::object& outObject)
{
	VPROF_BUDGET( "CFFEntitySystem::GetFunction", VPROF_BUDGETGROUP_FF_LUA );

	if(NULL == pEntity || NULL == szFunctionName)
		return false;

	luabind::adl::object tableObject;
	if(GetObject(pEntity, tableObject))
		return GetFunction(tableObject, szFunctionName, outObject);

	return false;
}

/////////////////////////////////////////////////////////////////////////////
bool CFFEntitySystem::GetFunction(luabind::adl::object& tableObject,
								  const char* szFunctionName,
								  luabind::adl::object& outObject)
{
	VPROF_BUDGET( "CFFEntitySystem::GetFunction", VPROF_BUDGETGROUP_FF_LUA );

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
int CFFEntitySystem::RunPredicates( CBaseEntity *ent, CBaseEntity *player, const char *addname )
{
	VPROF_BUDGET( "CFFEntitySystem::RunPredicates", VPROF_BUDGETGROUP_FF_LUA );

	Warning( "[RunPredicates] Shit is deprecated!\n" );
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
			Warning("Can't call entsys.runpredicates with an entity and no addname\n");
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
		DevWarning( "[SCRIPT] Error calling %s (%s) ent: %s\n", addname, lua_tostring(L, -1), ent ? STRING( ent->GetEntityName() ) : "NULL" );
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
				if( entsys.RunPredicates_LUA( pEntity, &hOutput, pszFunction ) )
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
bool CFFEntitySystem::RunPredicates_LUA( CBaseEntity *pObject, CFFLuaSC *pContext, const char *szFunctionName )
{
	VPROF_BUDGET( "CFFEntitySystem::RunPredicates_LUA", VPROF_BUDGETGROUP_FF_LUA );

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
