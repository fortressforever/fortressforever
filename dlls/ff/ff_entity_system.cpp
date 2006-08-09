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

//----------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_entity_system.h"

// Filesystem stuff
#include "filesystem.h"

// Entity stuff
#include "takedamageinfo.h"
#include "ff_player.h"
#include "ff_item_flag.h"
#include "ff_goal.h"
#include "team.h"
#include "doors.h"
#include "buttons.h"
#include "triggers.h"
#include "ff_utils.h"
#include "ff_team.h"
#include "ff_gamerules.h"
#include "ff_grenade_base.h"
#include "beam_shared.h"
#include "ff_luacontext.h"
#include "ff_lualib.h"
#include "ammodef.h"
//#include "ff_miniturret.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#undef MINMAX_H
#undef min
#undef max

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "luabind/iterator_policy.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//----------------------------------------------------------------------------
// globals
CFFEntitySystem entsys;
CFFEntitySystemHelper *helper; // global variable.. OH NOES!

ConVar mp_respawndelay( "mp_respawndelay", "0", 0, "Time (in seconds) for spawn delays. Can be overridden by LUA." );

using namespace luabind;

//============================================================================
// CFFEntitySystemHelper implementation
//============================================================================
LINK_ENTITY_TO_CLASS( entity_system_helper, CFFEntitySystemHelper );

// Start of our data description for the class
BEGIN_DATADESC( CFFEntitySystemHelper )
	DEFINE_THINKFUNC( OnThink ),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: Sets up the entity's initial state
//-----------------------------------------------------------------------------
void CFFEntitySystemHelper::Spawn( void )
{
	DevMsg("[EntSys] Entity System Helper Spawned\n");

	SetThink( &CFFEntitySystemHelper::OnThink );		// |-- Mirv: Account for GCC strictness
	SetNextThink( gpGlobals->curtime + 1.0f );
}

//----------------------------------------------------------------------------
void CFFEntitySystemHelper::OnThink( void )
{
	CFFLuaSC hTick;
	entsys.RunPredicates_LUA( NULL, &hTick, "tick" );
	SetNextThink( gpGlobals->curtime + 1.0f );
}

//----------------------------------------------------------------------------
void CFFEntitySystemHelper::Precache( void )
{
	CFFLuaSC hPrecache;
	entsys.RunPredicates_LUA( NULL, &hPrecache, "precache" );
}

//============================================================================
// CFFEntitySystem implementation
//============================================================================
CFFEntitySystem::CFFEntitySystem()
{
	DevMsg( "[SCRIPT] Attempting to start up the entity system...\n" );

	// Initialise this to false
	m_ScriptExists = false;
}

//----------------------------------------------------------------------------
CFFEntitySystem::~CFFEntitySystem()
{
	m_ScriptExists = false;

	// Check it exists then close it
	if( L )
		lua_close(L);

	// Just to be safe!
	L = NULL;
}

//----------------------------------------------------------------------------
bool CFFEntitySystem::LoadLuaFile( lua_State *L, const char *filename)
{
	DevMsg("[SCRIPT] Loading Lua File: %s\n", filename);
	FileHandle_t f = filesystem->Open( filename, "rb", "MOD" );

	if ( !f )
	{
		DevWarning( "[SCRIPT] %s either does not exist or could not be opened!\n", filename );
		return false;
	}

	int fileSize = filesystem->Size(f);
	char *buffer = (char*)MemAllocScratch( fileSize + 1 );
		
	Assert( buffer );
		
	// load file into a null-terminated buffer
	filesystem->Read( buffer, fileSize, f );
	buffer[fileSize] = 0;
	filesystem->Close( f );

	// Now load this script [TODO: Some error checking here]
	//lua_dostring( L, buffer );
	int rc = luaL_loadbuffer( L, buffer, fileSize, filename );
	if ( rc )
	{
		if ( rc == LUA_ERRSYNTAX )
		{
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

	lua_pcall(L, 0, 0, 0);

	MemFreeScratch();

	DevMsg( "[SCRIPT] Successfully loaded %s\n", filename );
	return true;
}

//----------------------------------------------------------------------------
bool CFFEntitySystem::StartForMap()
{
	// [TODO]
	// Fix the fact that it keeps holding information across calls to this function
	char filename[128];


	// Clear up an existing one
	if( L )
		lua_close(L);

	// Now open Lua VM
	L = lua_open();

	// Bah!
	if( !L )
	{
		DevWarning( "[SCRIPT] Crap, couldn't get the vm started!\n" );
		return false;
	}

	// Load the base libraries [TODO] Not all of them !
	luaopen_base(L);
	luaopen_table(L);
	luaopen_string(L);
	luaopen_math(L);
	
	//lua_atpanic(L, HandleError);

	// setup luabind
	luabind::open(L);

	// And now load some of ours
	CFFLuaLib::Init(L);
	
	// Hurrah well that is set up now
	DevMsg( "[SCRIPT] Entity system all set up!\n" );

	// load the generic library
	LoadLuaFile(L, "maps/includes/base.lua");

	// Get filename from map
	strcpy( filename, "maps/" );
	strcat( filename, STRING( gpGlobals->mapname ) );
	strcat( filename, ".lua" );

	m_ScriptExists = LoadLuaFile(L, filename);

	// spawn the helper entity
	helper = (CFFEntitySystemHelper *)CreateEntityByName( "entity_system_helper" );
	helper->Spawn();

	helper->Precache();

	return true;
}

//----------------------------------------------------------------------------
int CFFEntitySystem::HandleError( lua_State *L )
{
	const char *error = lua_tostring(L, -1);
	Warning("[SCRIPT] %s", error);

	return 0;
}

//----------------------------------------------------------------------------
void CFFEntitySystem::SetVar( lua_State *L, const char *name, const char *value )
{
	lua_pushstring(L, value);
	lua_setglobal(L, name);
}

//----------------------------------------------------------------------------
void CFFEntitySystem::SetVar( lua_State *L, const char *name, int value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

//----------------------------------------------------------------------------
void CFFEntitySystem::SetVar( lua_State *L, const char *name, float value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

//----------------------------------------------------------------------------
void CFFEntitySystem::SetVar( const char *name, const char *value )
{
	lua_pushstring(L, value);
	lua_setglobal(L, name);
}

//----------------------------------------------------------------------------
void CFFEntitySystem::SetVar( const char *name, int value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

//----------------------------------------------------------------------------
void CFFEntitySystem::SetVar( const char *name, float value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

//----------------------------------------------------------------------------
const char *CFFEntitySystem::GetString( const char *name )
{
	lua_getglobal(L, name);
	const char *ret = lua_tostring(L, -1);
	lua_pop(L, 1);
	return ret;
}

//----------------------------------------------------------------------------
int CFFEntitySystem::GetInt( const char *name )
{
	lua_getglobal(L, name);
	int ret = (int)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return ret;
}

//----------------------------------------------------------------------------
float CFFEntitySystem::GetFloat( const char *name )
{
	lua_getglobal(L, name);
	float ret = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return ret;
}

//----------------------------------------------------------------------------
void CFFEntitySystem::DoString( const char *buffer )
{
	Assert( buffer );

	DevMsg("Running lua command '%s'\n", buffer);

	int rc = luaL_loadbuffer( L, buffer, strlen(buffer), "User Command" );
	if ( rc )
	{
		if ( rc == LUA_ERRSYNTAX )
		{
			const char *error = lua_tostring(L, -1);
			if (error)
			{
				Warning("Error running command. %s\n", error);
				lua_pop( L, 1 );
			}
			else
				Warning("Unknown Syntax Error loading command\n");
		}
		else
		{
			Warning("Unknown Error loading command\n");
		}
		return;
	}
	
	lua_pcall(L,0,0,0);

}


//----------------------------------------------------------------------------
bool CFFEntitySystem::GetObject(CBaseEntity* pEntity, luabind::adl::object& outObject)
{
	if(!pEntity)
		return false;

	// lookup the object using the entity's name
	const char* szEntName = STRING(pEntity->GetEntityName());
	return GetObject(szEntName, outObject);
}

//----------------------------------------------------------------------------
bool CFFEntitySystem::GetObject(const char* szTableName, luabind::adl::object& outObject)
{
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

//----------------------------------------------------------------------------
bool CFFEntitySystem::GetFunction(CBaseEntity* pEntity,
								  const char* szFunctionName,
								  luabind::adl::object& outObject)
{
	if(NULL == pEntity || NULL == szFunctionName)
		return false;

	luabind::adl::object tableObject;
	if(GetObject(pEntity, tableObject))
		return GetFunction(tableObject, szFunctionName, outObject);

	return false;
}

//----------------------------------------------------------------------------
bool CFFEntitySystem::GetFunction(luabind::adl::object& tableObject,
								  const char* szFunctionName,
								  luabind::adl::object& outObject)
{
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

//----------------------------------------------------------------------------
int CFFEntitySystem::RunPredicates( CBaseEntity *ent, CBaseEntity *player, const char *addname )
{
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
			Warning( "[entsys] ent did not have an entity name!\n" );
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

//----------------------------------------------------------------------------
bool FFScriptRunPredicates( CBaseEntity *pObject, const char *pszFunction, bool bExpectedVal )
{
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

//----------------------------------------------------------------------------
// Purpose: Call into lua and get a result, basically. A better runpredicates
// Output : hOutput - the value returned from the lua object/function called
//
// Function return value: function will return true if it found the lua object/function
//----------------------------------------------------------------------------
bool CFFEntitySystem::RunPredicates_LUA( CBaseEntity *pObject, CFFLuaSC *pContext, const char *szFunctionName )
{
	Warning( "[RunPredicates_LUA]\n" );

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

	Warning( "[RunPredicates_LUA] pContext->CallFunction\n" );
	if( pContext->CallFunction( pObject, szFunctionName ) )
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Convert lua ammo type (int) to game ammo type (string)
//-----------------------------------------------------------------------------
const char *LookupLuaAmmo( int iLuaAmmoType )
{
	switch( iLuaAmmoType )
	{
		case LUA_AMMO_SHELLS: return AMMO_SHELLS; break;
		case LUA_AMMO_CELLS: return AMMO_CELLS; break;
		case LUA_AMMO_NAILS: return AMMO_NAILS; break;
		case LUA_AMMO_ROCKETS: return AMMO_ROCKETS; break;
		case LUA_AMMO_RADIOTAG: return AMMO_RADIOTAG; break;
		case LUA_AMMO_DETPACK: return AMMO_DETPACK; break;
		case LUA_AMMO_GREN1: return AMMO_GREN1; break;
		case LUA_AMMO_GREN2: return AMMO_GREN2; break;
	}

	AssertMsg( false, "LookupLuaAmmo - invalid ammo type!" );

	return "";
}

//-----------------------------------------------------------------------------
// Purpose: Convert ammo to lua ammo
//-----------------------------------------------------------------------------
int LookupAmmoLua( int iAmmoType )
{
	// NOTE: this is kind of lame as in i don't think we even setup the ammo
	// type in our CTakeDamageInfo classes ... except for radio tag rifle.

	if( GetAmmoDef() )
	{
		char *pszName = GetAmmoDef()->GetAmmoOfIndex( iAmmoType )->pName;

		if( pszName && Q_strlen( pszName ) )
		{
			if( !Q_strcmp( pszName, AMMO_SHELLS ) )
				return LUA_AMMO_SHELLS;
			else if( !Q_strcmp( pszName, AMMO_CELLS ) )
				return LUA_AMMO_CELLS;
			else if( !Q_strcmp( pszName, AMMO_NAILS ) )
				return LUA_AMMO_NAILS;
			else if( !Q_strcmp( pszName, AMMO_ROCKETS ) )
				return LUA_AMMO_ROCKETS;
			else if( !Q_strcmp( pszName, AMMO_RADIOTAG ) )
				return LUA_AMMO_RADIOTAG;
			else if( !Q_strcmp( pszName, AMMO_DETPACK ) )
				return LUA_AMMO_DETPACK;
			// TODO: Maybe figure these in somehow?
			/*
			else if( !Q_strcmp( pszName, AMMO_GREN1 ) )
				return LUA_AMMO_GREN1;
			else if( !Q_strcmp( pszName, AMMO_GREN2 ) )
				return LUA_AMMO_GREN2;
				*/
		}
	}

	return LUA_AMMO_INVALID;
}
