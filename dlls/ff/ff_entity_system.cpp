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
#include "omnibot_interface.h"
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
// defines
#define temp_max(a,b) (((a)>(b))?(a):(b))

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
	FFLibOpen();
	
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
// Purpose: Opens our FF functions to the vm
//----------------------------------------------------------------------------
class CClassLimits
{
public:
	CClassLimits()
	{
		int def = -1;
		scout = def;
		sniper = def;
		soldier = def;
		demoman = def;
		medic = def;
		hwguy = def;
		pyro = def;
		engineer = def;
		civilian = def;
	}

public:
	int scout;
	int sniper;
	int soldier;
	int demoman;
	int medic;
	int hwguy;
	int pyro;
	int spy;
	int engineer;
	int civilian;
};

class CPlayerLimits
{
public:
	CPlayerLimits()
	{
		blue = 0;
		red = 0;
		yellow = -1;
		green = -1;
	}

public:
	int blue;
	int red;
	int yellow;
	int green;
};

//-----------------------------------------------------------------------------
// Purpose: Parse collection filter flags
//-----------------------------------------------------------------------------
bool CollectionFilterParseFlags( const luabind::adl::object& table, bool *pbFlags )
{
	if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( table ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			if( luabind::type( val ) == LUA_TNUMBER )
			{
				// Set to out of bounds in case the cast fails
				int iIndex = -1;

				try
				{
					iIndex = luabind::object_cast< int >( val );
				}
				catch( ... )
				{
				}

				// Make sure within bounds
				if( ( iIndex >= 0 ) && ( iIndex < CF_MAX_FLAG ) )
					pbFlags[ iIndex ] = true;
			}
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PassesCollectionFilter_Players( CBaseEntity *pEntity )
{
	if( !pEntity )
		return false;

	if( !pEntity->IsPlayer() )
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PassesCollectionFilter_PlayerClass( CBaseEntity *pEntity, int iClassSlot )
{
	if( pEntity && pEntity->IsPlayer() )
	{
		CFFPlayer *pPlayer = ToFFPlayer( pEntity );
		if( pPlayer )
			return pPlayer->GetClassSlot() == iClassSlot;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PassesCollectionFilter_Team( CBaseEntity *pEntity )
{
	if( pEntity )
	{
		if( ( pEntity->GetTeamNumber() >= TEAM_SPECTATOR ) && ( pEntity->GetTeamNumber() <= TEAM_GREEN ) )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PassesCollectionFilter_TeamNum( CBaseEntity *pEntity, int iTeamNum )
{
	if( pEntity )
		return pEntity->GetTeamNumber() == iTeamNum;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Check an entity against some collection filter flags
//-----------------------------------------------------------------------------
bool PassesCollectionFilter( CBaseEntity *pEntity, bool *pbFlags, const Vector& vecTraceOrigin )
{
	if( pbFlags[ CF_PLAYERS ] )
	{
		if( !PassesCollectionFilter_Players( pEntity ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_SCOUT ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_SCOUT ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_SNIPER ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_SNIPER ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_SOLDIER ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_SOLDIER ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_DEMOMAN ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_DEMOMAN ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_MEDIC ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_MEDIC ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_HWGUY ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_HWGUY ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_PYRO ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_PYRO ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_SPY ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_SPY ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_ENGY ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_ENGINEER ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_CIVILIAN ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_CIVILIAN ) )
			return false;
	}

	if( pbFlags[ CF_TEAMS ] )
	{
		if( !PassesCollectionFilter_Team( pEntity ) )
			return false;
	}

	if( pbFlags[ CF_TEAM_SPECTATOR ] )
	{
		if( !PassesCollectionFilter_TeamNum( pEntity, TEAM_SPECTATOR ) )
			return false;
	}

	if( pbFlags[ CF_TEAM_BLUE ] )
	{
		if( !PassesCollectionFilter_TeamNum( pEntity, TEAM_BLUE ) )
			return false;
	}

	if( pbFlags[ CF_TEAM_RED ] )
	{
		if( !PassesCollectionFilter_TeamNum( pEntity, TEAM_RED ) )
			return false;
	}

	if( pbFlags[ CF_TEAM_YELLOW ] )
	{
		if( !PassesCollectionFilter_TeamNum( pEntity, TEAM_YELLOW ) )
			return false;
	}

	if( pbFlags[ CF_TEAM_GREEN ] )
	{
		if( !PassesCollectionFilter_TeamNum( pEntity, TEAM_GREEN ) )
			return false;
	}

	if( pbFlags[ CF_PROJECTILES ] )
	{
		if( dynamic_cast< CFFProjectileBase * >( pEntity ) == NULL )
			return false;
	}

	if( pbFlags[ CF_GRENADES ] )
	{
		if( !( pEntity->GetFlags() & FL_GRENADE ) )
			return false;
	}

	if( pbFlags[ CF_BUILDABLES ] )
	{
		if( ( pEntity->Classify() != CLASS_DISPENSER ) ||
			( pEntity->Classify() != CLASS_SENTRYGUN ) ||
			( pEntity->Classify() != CLASS_DETPACK ) )
			return false;
	}

	if( pbFlags[ CF_BUILDABLE_DISPENSER ] )
	{
		if( pEntity->Classify() != CLASS_DISPENSER )
			return false;
	}

	if( pbFlags[ CF_BUILDABLE_SENTRYGUN ] )
	{
		if( pEntity->Classify() != CLASS_SENTRYGUN )
			return false;
	}

	if( pbFlags[ CF_BUILDABLE_DETPACK ] )
	{
		if( pEntity->Classify() != CLASS_DETPACK )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Constructor - Create an empty collection
//-----------------------------------------------------------------------------
CFFEntity_Collection::CFFEntity_Collection( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Constructor - Create a collection from table
//-----------------------------------------------------------------------------
CFFEntity_Collection::CFFEntity_Collection( const luabind::adl::object& table )
{
	AddItem( table );
}

//-----------------------------------------------------------------------------
// Purpose: Add an item to the collection
//-----------------------------------------------------------------------------
void CFFEntity_Collection::AddItem( CBaseEntity *pItem )
{
	if( !pItem )
		return;

	if( !InternalFindPtr( pItem ) )
		m_vCollection.push_back( pItem );
}

//-----------------------------------------------------------------------------
// Purpose: Add an item to the collection
//-----------------------------------------------------------------------------
void CFFEntity_Collection::AddItem( const luabind::adl::object& table )
{
	if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( table ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			try
			{
				CBaseEntity *pEntity = luabind::object_cast< CBaseEntity * >( val );

				if( pEntity )
					AddItem( pEntity );
			}
			catch( ... )
			{
				Warning( "[Collection] Error in AddItem!\n" );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove an item from the collection
//-----------------------------------------------------------------------------
void CFFEntity_Collection::RemoveItem( CBaseEntity *pItem )
{
	if( !pItem )
		return;

	CollectionContainer::iterator vs = InternalFindItr( pItem );

	if( vs != m_vCollection.end() )
		m_vCollection.erase( vs );
}

//-----------------------------------------------------------------------------
// Purpose: Remove an item from the collection
//-----------------------------------------------------------------------------
void CFFEntity_Collection::RemoveItem( const luabind::adl::object& table )
{
	if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( table ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			try
			{
				CBaseEntity *pEntity = luabind::object_cast< CBaseEntity * >( val );

				if( pEntity )
					RemoveItem( pEntity );
			}
			catch( ... )
			{
				Warning( "[Collection] Error in RemoveItem!\n" );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove all items from the collection
//-----------------------------------------------------------------------------
void CFFEntity_Collection::RemoveAllItems( void )
{
	m_vCollection.clear();
}

//-----------------------------------------------------------------------------
// Purpose: See if an item exists
//-----------------------------------------------------------------------------
bool CFFEntity_Collection::HasItem( CBaseEntity *pItem )
{
	if( !pItem )
		return false;

	return InternalFindPtr( pItem ) ? true : false;
}

//-----------------------------------------------------------------------------
// Purpose: See if an item exists
//-----------------------------------------------------------------------------
bool CFFEntity_Collection::HasItem( const luabind::adl::object& table )
{
	if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( table ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			try
			{
				CBaseEntity *pEntity = luabind::object_cast< CBaseEntity * >( val );

				// TODO: Support seeing if all values in table were found
				if( pEntity )
					return HasItem( pEntity );
			}
			catch( ... )
			{
				Warning( "[Collection] Error in HasItem!\n" );
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Return an item from the collection
//-----------------------------------------------------------------------------
CBaseEntity *CFFEntity_Collection::GetItem( CBaseEntity *pItem )
{
	if( !pItem )
		return NULL;

	return InternalFindPtr( pItem );
}

//-----------------------------------------------------------------------------
// Purpose: Return an item from the collection.
//-----------------------------------------------------------------------------
CBaseEntity *CFFEntity_Collection::GetItem( const luabind::adl::object& table )
{
	if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( table ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			try
			{
				CBaseEntity *pEntity = luabind::object_cast< CBaseEntity * >( val );

				if( pEntity )
				{
					CBaseEntity *pFoundEntity = GetItem( pEntity );
					if( pFoundEntity )
						return pFoundEntity;
				}
			}
			catch( ... )
			{
				Warning( "[Collection] Error in GetItem!\n" );
			}
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Find an item in the collection & return a reference to it
//-----------------------------------------------------------------------------
CBaseEntity *CFFEntity_Collection::InternalFindPtr( CBaseEntity *pItem )
{
	if( !pItem )
		return NULL;

	CollectionContainer::iterator vb = m_vCollection.begin(), ve = m_vCollection.end();

	// Search for pItem
	for( ; vb != ve; vb++ )
	{
		if( *vb == pItem )
			return *vb;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Find an item in the collection & return a iterator to it
//-----------------------------------------------------------------------------
CollectionContainer::iterator CFFEntity_Collection::InternalFindItr( CBaseEntity *pItem )
{
	if( !pItem )
		return m_vCollection.end();

	CollectionContainer::iterator vb = m_vCollection.begin(), ve = m_vCollection.end();

	// Search for pItem
	for( ; vb != ve; vb++ )
	{
		if( *vb == pItem )
			return vb;
	}

	return m_vCollection.end();
}

//-----------------------------------------------------------------------------
// Purpose: Get entities inside a sphere filtered by hFilterTable
//-----------------------------------------------------------------------------
void CFFEntity_Collection::GetInSphere( const Vector& vecOrigin, float flRadius, const luabind::adl::object& hFilterTable )
{
	// Waste!
	if( flRadius < 0 )
		return;

	bool bFlags[ CF_MAX_FLAG ] = { false };
	if( CollectionFilterParseFlags( hFilterTable, bFlags ) )
	{
		CBaseEntity *pEntity = NULL;
		for( CEntitySphereQuery sphere( vecOrigin, flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( !pEntity )
				continue;

			if( PassesCollectionFilter( pEntity, bFlags, vecOrigin ) )
				m_vCollection.push_back( pEntity );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get any entities that are currently touching pTouchee filter by hFilterTable
//-----------------------------------------------------------------------------
void CFFEntity_Collection::GetTouching( CBaseEntity *pTouchee, const luabind::adl::object& hFilterTable )
{
	if( !pTouchee )
		return;

	bool bFlags[ CF_MAX_FLAG ] = { false };
	if( CollectionFilterParseFlags( hFilterTable, bFlags ) )
	{
		// TODO: Hrm, don't know how possible this is
		// Might have to be trigger_ff_script only (?)
		/*
		CBaseEntity *pEntity = gEntList.FirstEnt();
		while( pEntity )
		{
			if( pEntity->IsCurrentlyTouching() )
			{
				
			}

			pEntity = gEntList.NextEnt( pEntity );
		}
		*/
	}
}

//-----------------------------------------------------------------------------
// Purpose: Return item[ iElement ]
//-----------------------------------------------------------------------------
CBaseEntity *CFFEntity_Collection::Element( int iElement )
{
	if( iElement < 0 )
		return NULL;

	if( iElement > ( Count() - 1 ) )
		return NULL;

	return m_vCollection[ iElement ];
}

//-----------------------------------------------------------------------------
// FFLib Namespace
//-----------------------------------------------------------------------------
namespace FFLib
{
	// returns if the entity of the specified type
	// uses the Classify function for evaluation
	bool IsOfClass(CBaseEntity* pEntity, int classType)
	{
		if( !pEntity )
			return false;

		return ( pEntity->Classify() == classType );
	}

	// is entity a dispenser
	bool IsDispenser(CBaseEntity* pEntity)
	{
		return IsOfClass(pEntity, CLASS_DISPENSER);
	}

	// is entity a sentry gun
	bool IsSentrygun(CBaseEntity* pEntity)
	{
		return IsOfClass(pEntity, CLASS_SENTRYGUN);
	}

	// is entity a dispenser
	bool IsDetpack( CBaseEntity *pEntity )
	{
		return IsOfClass( pEntity, CLASS_DETPACK );
	}

	// is the entity a grenade
	bool IsGrenade(CBaseEntity* pEntity)
	{
		// Yeah, the simple life, man
		return ( pEntity->GetFlags() & FL_GRENADE ) ? true : false;
		/*
		return (IsOfClass(pEntity, CLASS_GREN) ||
				IsOfClass(pEntity, CLASS_GREN_EMP) ||
				IsOfClass(pEntity, CLASS_GREN_NAIL) ||
				IsOfClass(pEntity, CLASS_GREN_MIRV) ||
				IsOfClass(pEntity, CLASS_GREN_MIRVLET) ||
				IsOfClass(pEntity, CLASS_GREN_NAPALM) ||
				IsOfClass(pEntity, CLASS_GREN_GAS) ||
				IsOfClass(pEntity, CLASS_GREN_CONC) ||
				IsOfClass(pEntity, CLASS_GREN_CALTROP));
				*/
	}

	// is the entity a miniturret
	bool IsTurret( CBaseEntity *pEntity )
	{
		return IsOfClass( pEntity, CLASS_TURRET );
	}

	void BroadcastMessage(const char* szMessage)
	{
		CBroadcastRecipientFilter filter;
		UserMessageBegin(filter, "GameMessage");
			WRITE_STRING(szMessage);
		MessageEnd();
	}

	void SendPlayerMessage(CFFPlayer* pPlayer, const char* szMessage)
	{
		if(NULL == pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "GameMessage");
			WRITE_STRING(szMessage);
		MessageEnd();
	}

	void BroadcastSound(const char* szSound)
	{
		CBroadcastRecipientFilter filter;
		if( helper )
			helper->EmitSound( filter, helper->entindex(), szSound);
	}

	void SendPlayerSound(CFFPlayer* pPlayer, const char* szSound)
	{
		if(NULL == pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);
		if( helper )
			helper->EmitSound( filter, helper->entindex(), szSound);
	}

	void SetGlobalRespawnDelay(float delay)
	{
		mp_respawndelay.SetValue( temp_max( 0.0f, delay ) );
	}

	void GoToIntermission()
	{
		if (FFGameRules())
			FFGameRules()->GoToIntermission();
	}

	bool ApplyToParseFlags( const luabind::adl::object& table, bool *pbFlags )
	{
		if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
		{
			// Iterate through the table
			for( iterator ib( table ), ie; ib != ie; ++ib )
			{
				//std::string strKey = object_cast< std::string >( ib.key() );

				luabind::adl::object val = *ib;

				if( luabind::type( val ) == LUA_TNUMBER )
				{
					int iIndex =  - 1;
					
					try
					{
						iIndex = luabind::object_cast< int >( val );
					}
					catch( ... )
					{
					}

					// Make sure within bounds
					if( ( iIndex >= 0 ) && ( iIndex < AT_MAX_FLAG ) )
						pbFlags[ iIndex ] = true;
				}
				else
				{
					// Warning( "[ResetParseFlags] Only handles integers in the table!\n" );
				}
			}

			return true;
		}

		return false;
	}

	void ApplyToAll( const luabind::adl::object& table )
	{
		bool bFlags[ AT_MAX_FLAG ] = { false };

		if( ApplyToParseFlags( table, bFlags ) )
		{
			if( FFGameRules() )
				FFGameRules()->ResetUsingCriteria( bFlags );
		}
	}

	void ApplyToTeam( CFFTeam *pTeam, const luabind::adl::object& table )
	{
		bool bFlags[ AT_MAX_FLAG ] = { false };

		if( ApplyToParseFlags( table, bFlags ) && pTeam )
		{
			if( FFGameRules() )
				FFGameRules()->ResetUsingCriteria( bFlags, pTeam->GetTeamNumber() );
		}
	}

	void ApplyToPlayer( CFFPlayer *pPlayer, const luabind::adl::object& table )
	{
		bool bFlags[ AT_MAX_FLAG ] = { false };

		if( ApplyToParseFlags( table, bFlags ) && pPlayer )
		{
			if( FFGameRules() )
				FFGameRules()->ResetUsingCriteria( bFlags, TEAM_UNASSIGNED, pPlayer );
		}
	}

	void ResetMap( const luabind::adl::object& table )
	{
		bool bFlags[ AT_MAX_FLAG ] = { false };

		if( ApplyToParseFlags( table, bFlags ) )
		{
			if( FFGameRules() )
				FFGameRules()->ResetUsingCriteria( bFlags, TEAM_UNASSIGNED, NULL, true );
		}
	}

	void RespawnAllPlayers( void )
	{		
		// loop through each player
		for (int i=0; i<gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );

				pPlayer->RemoveItems();
				pPlayer->Spawn();
				//pPlayer->KillAndRemoveItems();
			}
		}
	}

	void KillAndRespawnAllPlayers( void )
	{
		// loop through each player
		for (int i=0; i<gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				pPlayer->KillAndRemoveItems();
			}
		}
	}

	void ConsoleToAll(const char* szMessage)
	{
		DevMsg( szMessage );
		DevMsg( "\n" );
	}

	void IncludeScript(const char* script)
	{
		char realscript[255];

		// make sure it's a valid filename (alphanum only)
		bool good = true;
		for (unsigned int i=0; i<strlen(script); i++)
		{
			if (script[i]>='a' && script[i]<='z') continue;
			if (script[i]>='A' && script[i]<='Z') continue;
			if (script[i]>='0' && script[i]<='9') continue;
			if (script[i]=='_') continue;
			
			good = false;
		}

		// if it's a good filename, then go ahead and include it
		if (good)
		{
			// Let's use a little more control
			/*
			strcpy(realscript, "maps/includes/" );
			strcat(realscript, script);
			strcat(realscript, ".lua");
			*/
			Q_snprintf( realscript, sizeof( realscript ), "maps/includes/%s.lua", script );

			if( !CFFEntitySystem::LoadLuaFile( entsys.GetLuaState(), realscript ) )
			{
				// Try looking in the maps directory
				Q_snprintf( realscript, sizeof( realscript ), "maps/%s.lua", script );
				CFFEntitySystem::LoadLuaFile( entsys.GetLuaState(), realscript );
			}
		}
		else
		{
			DevWarning("[SCRIPT] Warning: Invalid filename: %s\n", script);
		}
	}

	void RemoveEntity(CBaseEntity* pEntity)
	{
		if( !pEntity )
			return;
		
		UTIL_Remove(pEntity);
	}

	CFFTeam* GetTeam(int teamId)
	{
		if( teamId < TEAM_BLUE )
			return NULL;
		if( teamId > TEAM_GREEN )
			return NULL;

		return dynamic_cast<CFFTeam*>(g_Teams[teamId]);
	}

	CBaseEntity* GetEntity(int item_id)
	{
		return UTIL_EntityByIndex(item_id);
	}

	CBaseEntity* GetEntityByName(const char* szName)
	{
		return gEntList.FindEntityByName(NULL, szName, NULL);
	}
	
	//std::vector<CBaseEntity*> GetEntitiesByName(const char* szName)
	std::vector<int> GetEntitiesByName(const char* szName)
	{
		static std::vector<int> ret;

		CBaseEntity *ent = gEntList.FindEntityByName(NULL, szName, NULL);
		while (ent != NULL)
		{
			DevMsg("push_back(%d)\n", ENTINDEX(ent));
			ret.push_back(ENTINDEX(ent));
			ent = gEntList.FindEntityByName(ent, szName, NULL);
		}

		return ret;
	}

	std::vector<CBaseEntity*> GetEntitiesInSphere(Vector origin, float radius)
	{
		static std::vector<CBaseEntity*> ret;

		CBaseEntity *ent;
		for( CEntitySphereQuery sphere( origin, radius ); ( ent = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			ret.push_back(ent);
		}

		return ret;
	}

	CFFPlayer* GetPlayer(CBaseEntity *pEntity)
	{
		//CBaseEntity* pEnt = GetEntity(player_id);

		if(NULL == pEntity)
			return NULL;

		if(!pEntity->IsPlayer())
			return NULL;

		return dynamic_cast<CFFPlayer*>(pEntity);
	}

	CFFGrenadeBase *GetGrenade( int ent_id )
	{
		CBaseEntity *pEnt = GetEntity( ent_id );

		if( !pEnt )
			return NULL;

		if( !IsGrenade( pEnt ) )
			return NULL;

		return dynamic_cast< CFFGrenadeBase * >( pEnt );
	}

	bool IsPlayer( CBaseEntity *pEntity )
	{
		return GetPlayer( pEntity ) == NULL ? false : true;
	}

	CFFInfoScript* GetInfoScriptByName(const char* entityName)
	{
		CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( NULL, CLASS_INFOSCRIPT );

		while( pEnt != NULL )
		{
			if ( FStrEq( STRING(pEnt->GetEntityName()), entityName ) )
				return pEnt;

			// Next!
			pEnt = (CFFInfoScript*)gEntList.FindEntityByClassT( pEnt, CLASS_INFOSCRIPT );
		}

		return NULL;
	}

	CFuncFFScript *GetTriggerScriptByName( const char *pszEntityName )
	{
		CFuncFFScript *pEntity = ( CFuncFFScript * )gEntList.FindEntityByClassT( NULL, CLASS_TRIGGERSCRIPT );

		while( pEntity )
		{
			if( FStrEq( STRING( pEntity->GetEntityName() ), pszEntityName ) )
				return pEntity;

			pEntity = ( CFuncFFScript * )gEntList.FindEntityByClassT( pEntity, CLASS_TRIGGERSCRIPT );
		}

		return NULL;
	}

	CFFInfoScript* GetInfoScriptById(int item_id)
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex( item_id );
		if( pEntity && ( pEntity->Classify() == CLASS_INFOSCRIPT ) )
			return dynamic_cast< CFFInfoScript * >( pEntity );
		
		return NULL;
	}

	CFFPlayer* CastToPlayer( CBaseEntity* pEntity )
	{
		if( !pEntity )
			return NULL;

		if( pEntity->IsPlayer() )
			return dynamic_cast< CFFPlayer * >( pEntity );
		else
			return NULL;
	}

	CFFGrenadeBase *CastToGrenade( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsGrenade( pEntity ) )
			return NULL;

		return dynamic_cast< CFFGrenadeBase * >( pEntity );
	}

	CBeam* CastToBeam(CBaseEntity* pEntity)
	{
		if( !pEntity )
			return NULL;

		if( !FStrEq( pEntity->GetClassname(), "env_beam") )
			return NULL;

		return dynamic_cast< CBeam * >( pEntity );
	}

	CFFInfoScript* CastToItemFlag(CBaseEntity* pEntity)
	{
		if( !pEntity )
			return NULL;

		if( pEntity->Classify() != CLASS_INFOSCRIPT )
			return NULL;

		return dynamic_cast< CFFInfoScript * >( pEntity );
	}

	CFuncFFScript *CastToTriggerScript( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( pEntity->Classify() != CLASS_TRIGGERSCRIPT )
			return NULL;

		return dynamic_cast< CFuncFFScript * >( pEntity );
	}

	CFFDispenser *CastToDispenser( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsDispenser( pEntity ) )
			return NULL;

		return dynamic_cast< CFFDispenser * >( pEntity );
	}

	CFFSentryGun *CastToSentrygun( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsSentrygun( pEntity ) )
			return NULL;

		return dynamic_cast< CFFSentryGun * >( pEntity );
	}

	CFFDetpack *CastToDetpack( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsDetpack( pEntity ) )
			return NULL;

		return dynamic_cast< CFFDetpack * >( pEntity );
	}

	/*CFFMiniTurret *CastToTurret( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsTurret( pEntity ) )
			return NULL;
		
		return dynamic_cast< CFFMiniTurret * >( pEntity );
	}
	*/

	bool AreTeamsAllied(CTeam* pTeam1, CTeam* pTeam2)
	{
		if(NULL == pTeam1 || NULL == pTeam2)
			return false;

		int iTeam1 = pTeam1->GetTeamNumber();
		int iTeam2 = pTeam2->GetTeamNumber();

		if( ( iTeam1 >= TEAM_BLUE ) && ( iTeam1 <= TEAM_GREEN ) &&
			( iTeam2 >= TEAM_BLUE ) && ( iTeam2 <= TEAM_GREEN ) )
		{
			if( FFGameRules()->IsTeam1AlliedToTeam2( iTeam1, iTeam2 ) == GR_TEAMMATE )
				return true;
		}

		return false;
	}

	bool AreTeamsAllied(int teamA, int teamB)
	{
		CFFTeam* pTeamA = GetGlobalFFTeam(teamA);
		CFFTeam* pTeamB = GetGlobalFFTeam(teamB);

		if( !pTeamA || !pTeamB )
			return false;

		return AreTeamsAllied(pTeamA, pTeamB);
	}

	int RandomInt(int min, int max)
	{
		return random->RandomInt(min, max);
	}

	float RandomFloat(float min, float max)
	{
		return random->RandomFloat(min, max);
	}

	void SmartClassLimits(unsigned int teamId, CClassLimits& limits )
	{
		// get team
		CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set team's class limits
		pTeam->SetClassLimit(CLASS_SCOUT, limits.scout);
		pTeam->SetClassLimit(CLASS_SNIPER, limits.sniper);
		pTeam->SetClassLimit(CLASS_SOLDIER, limits.soldier);
		pTeam->SetClassLimit(CLASS_DEMOMAN, limits.demoman);
		pTeam->SetClassLimit(CLASS_MEDIC, limits.medic);
		pTeam->SetClassLimit(CLASS_HWGUY, limits.hwguy);
		pTeam->SetClassLimit(CLASS_PYRO, limits.pyro);
		pTeam->SetClassLimit(CLASS_SPY, limits.spy);
		pTeam->SetClassLimit(CLASS_ENGINEER, limits.engineer);
		pTeam->SetClassLimit(CLASS_CIVILIAN, limits.civilian);
	}

	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		int nPlayers = FF_NumPlayers();
		for(int i = 1 ; i <= nPlayers ; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg);

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg);

			else
				SendPlayerMessage(pTestPlayer, otherMsg);
		}
	}

	void SmartSound(CBaseEntity *pEntity, const char* playerSound, const char* teamSound, const char* otherSound)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		int nPlayers = FF_NumPlayers();
		for(int i = 1 ; i <= nPlayers ; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerSound(pTestPlayer, playerSound);

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerSound(pTestPlayer, teamSound);

			else
				SendPlayerSound(pTestPlayer, otherSound);
		}
	}

	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		int nPlayers = FF_NumPlayers();
		for(int i = 1 ; i <= nPlayers ; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerMessage(pPlayer, teamMsg);
			else
				SendPlayerMessage(pPlayer, otherMsg);
		}
	}

	void SmartTeamSound(CFFTeam *pTeam, const char* teamSound, const char* otherSound)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate sound to each player
		int nPlayers = FF_NumPlayers();
		for(int i = 1 ; i <= nPlayers ; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerSound(pPlayer, teamSound);
			else
				SendPlayerSound(pPlayer, otherSound);
		}
	}

	CFFPlayer *GetPlayerByID( int player_id )
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex( player_id );
		return GetPlayer( pEntity );
	}

	void SetPlayerLimits(CPlayerLimits& limits)
	{
		CFFTeam* pTeam = GetTeam(TEAM_BLUE);
		pTeam->SetTeamLimits(limits.blue);

		pTeam = GetTeam(TEAM_RED);
		pTeam->SetTeamLimits(limits.red);

		pTeam = GetTeam(TEAM_YELLOW);
		pTeam->SetTeamLimits(limits.yellow);

		pTeam = GetTeam(TEAM_GREEN);
		pTeam->SetTeamLimits(limits.green);
	}

	void SetPlayerLimit(int teamId, int limit)
	{
		CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL != pTeam)
			pTeam->SetTeamLimits(limit);
	}

	void SetTeamName(int teamId, const char* szTeamName)
	{
		CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL != pTeam)
			pTeam->SetName(szTeamName);
	}

	void SetTeamClassLimit(int teamId, int classId, int limit)
	{
		CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL != pTeam)
			pTeam->SetClassLimit(classId, limit);
	}

	float GetServerTime( void )
	{
		return gpGlobals->curtime;
	}

	void AddPlayerSpeedEffect( CFFPlayer *pPlayer, float flDuration, float flPercent )
	{
	}

	void UseEntity(const char* itemname, const char* classname, const char* action)
	{
		// get all info_ff_scripts
		CBaseEntity *pEnt = gEntList.FindEntityByClassname( NULL, classname );

		inputdata_t id;

		while( pEnt != NULL )
		{
			if ( FStrEq( STRING(pEnt->GetEntityName()), itemname ) )
			{
				// if this is the right one, then fire its output
				DevMsg("[SCRIPT] found goal %d: %s\n", ENTINDEX(pEnt), itemname);

				if (FStrEq(classname, "ff_goal"))
				{
					if (FStrEq(action, "FireOutput"))
						((CFFGoal *)pEnt)->FireOutput();
				}
				else if (FStrEq(classname, "func_door") || FStrEq(classname, "func_water"))
				{
					if (FStrEq(action, "Open"))
						((CBaseDoor *)pEnt)->InputOpen(id);
					else if (FStrEq(action, "Close"))
						((CBaseDoor *)pEnt)->InputClose(id);
					else if (FStrEq(action, "Toggle"))
						((CBaseDoor *)pEnt)->InputToggle(id);
					else if (FStrEq(action, "Lock"))
						((CBaseDoor *)pEnt)->InputLock(id);
					else if (FStrEq(action, "Unlock"))
						((CBaseDoor *)pEnt)->InputUnlock(id);
				}
				else if (FStrEq(classname, "func_button"))
				{
					if (FStrEq(action, "Lock"))
						((CBaseButton *)pEnt)->InputLock(id);
					else if (FStrEq(action, "Unlock"))
						((CBaseButton *)pEnt)->InputUnlock(id);
					else if (FStrEq(action, "Press"))
						((CBaseButton *)pEnt)->InputPress(id);
				}
			}

			// Next!
			pEnt = gEntList.FindEntityByClassname( pEnt, classname );
		}
	}

	float GetConvar( const char *pszConvarName )
	{
		if( !pszConvarName )
			return 0.0f;

		ConVar *pConvar = ( ConVar * )ConCommandBase::FindCommand( pszConvarName );
		if( !pConvar || !pConvar->IsCommand() )
			return 0.0f;

		return pConvar->GetFloat();
	}

	void SetConvar( const char *pszConvarName, float flValue )
	{
		if( !pszConvarName )
			return;

		// Don't allow sv_cheats setting
		if( !Q_stricmp( pszConvarName, "sv_cheats" ) )
			return;

		ConVar *pConvar = ( ConVar * )ConCommandBase::FindCommand( pszConvarName );
		if( !pConvar || !pConvar->IsCommand() )
			return;

		pConvar->SetValue( flValue );
	}

	const char *GetSteamID( CFFPlayer *pPlayer )
	{
		if( pPlayer )
			return pPlayer->GetSteamID();

		return "\0";
	}

	int GetPing( CFFPlayer *pPlayer )
	{
		if( pPlayer )
			return pPlayer->GetPing();

		return 0;
	}

	int GetPacketloss( CFFPlayer *pPlayer )
	{
		if( pPlayer )
			return pPlayer->GetPacketloss();

		return 0;
	}

	const char *PrintBool( bool bValue )
	{
		return bValue ? "True" : "False";
	}

	void FireOutput(const char* szTargetEntityName,
					const char* szTargetInputName)

	{
		// EventAction string is in the following format:
		//    targetname,inputname,parameter,delay,number_times_to_fire
		char szAction[2048];
		Q_snprintf(szAction,
				   sizeof(szAction),
				   "%s,%s",
				   szTargetEntityName,
				   szTargetInputName);

		COutputEvent event;
		event.ParseEventAction(szAction);
		event.FireOutput(NULL, NULL);
	}

	void FireOutput(const char* szTargetEntityName,
					const char* szTargetInputName,
					const char* szParameter)

	{
		// EventAction string is in the following format:
		//    targetname,inputname,parameter,delay,number_times_to_fire
		char szAction[2048];
		Q_snprintf(szAction,
				   sizeof(szAction),
				   "%s,%s,%s",
				   szTargetEntityName,
				   szTargetInputName,
				   szParameter);

		COutputEvent event;
		event.ParseEventAction(szAction);
		event.FireOutput(NULL, NULL);
	}

	void FireOutput(const char* szTargetEntityName,
					const char* szTargetInputName,
					const char* szParameter,
					float delay)

	{
		// EventAction string is in the following format:
		//    targetname,inputname,parameter,delay,number_times_to_fire
		char szAction[2048];
		Q_snprintf(szAction,
				   sizeof(szAction),
				   "%s,%s,%s,%f",
				   szTargetEntityName,
				   szTargetInputName,
				   szParameter,
				   delay);

		COutputEvent event;
		event.ParseEventAction(szAction);
		event.FireOutput(NULL, NULL, delay);
	}

	void FireOutput(const char* szTargetEntityName,
					const char* szTargetInputName,
					const char* szParameter,
					float delay,
					unsigned int nRepeat)

	{
		// EventAction string is in the following format:
		//    targetname,inputname,parameter,delay,number_times_to_fire
		char szAction[2048];
		Q_snprintf(szAction,
				   sizeof(szAction),
				   "%s,%s,%s,%f,%d",
				   szTargetEntityName,
				   szTargetInputName,
				   szParameter,
				   delay,
				   nRepeat);

		COutputEvent event;
		event.ParseEventAction(szAction);
		event.FireOutput(NULL, NULL, delay);
	}

} // namespace FFLib

//----------------------------------------------------------------------------
void CFFEntitySystem::FFLibOpen()
{
	module(L)
	[
		class_<Vector>("Vector")
			.def(constructor<>())
			.def(constructor<float, float, float>())
			.def_readwrite("x",			&Vector::x)
			.def_readwrite("y",			&Vector::y)
			.def_readwrite("z",			&Vector::z)
			.def("IsValid",				&Vector::IsValid)
			.def("IsZero",				&Vector::IsZero)
			.def("DistTo",				&Vector::DistTo)
			.def("DistToSq",			&Vector::DistToSqr)
			.def("Dot",					&Vector::Dot)
			.def("Length",				&Vector::Length)
			.def("LengthSqr",			&Vector::LengthSqr)
			.def("Normalize",			&Vector::NormalizeInPlace)
			.def("Negate",				&Vector::Negate),

		class_<QAngle>("QAngle")
			.def(constructor<>())
			.def(constructor<float, float, float>())
			.def_readwrite("x",			&QAngle::x)
			.def_readwrite("y",			&QAngle::y)
			.def_readwrite("z",			&QAngle::z)
			.def("IsValid",				&QAngle::IsValid)
			.def("Length",				&QAngle::Length)
			.def("LengthSqr",			&QAngle::LengthSqr),

		class_<CClassLimits>("ClassLimits")
			.def(constructor<>())
			.def_readwrite("Scout",		&CClassLimits::scout)
			.def_readwrite("Sniper",	&CClassLimits::sniper)
			.def_readwrite("Soldier",	&CClassLimits::soldier)
			.def_readwrite("Demoman",	&CClassLimits::demoman)
			.def_readwrite("Medic",		&CClassLimits::medic)
			.def_readwrite("Hwguy",		&CClassLimits::hwguy)
			.def_readwrite("Pyro",		&CClassLimits::pyro)
			.def_readwrite("Engineer",	&CClassLimits::engineer)
			.def_readwrite("Spy",		&CClassLimits::spy)
			.def_readwrite("Civilian",	&CClassLimits::civilian),

		class_<CPlayerLimits>("PlayerLimits")
			.def(constructor<>())
			.def_readwrite("Blue",		&CPlayerLimits::blue)
			.def_readwrite("Red",		&CPlayerLimits::red)
			.def_readwrite("Yellow",	&CPlayerLimits::yellow)
			.def_readwrite("Green",		&CPlayerLimits::green),

		// CFFEntity_Collection
		class_<CFFEntity_Collection>("Collection")
			.def(constructor<>())
			.def(constructor<const luabind::adl::object&>())
			.def_readwrite("items",		&CFFEntity_Collection::m_vCollection, return_stl_iterator)
			.def("AddItem",				(void(CFFEntity_Collection::*)(CBaseEntity*))&CFFEntity_Collection::AddItem)
			.def("AddItem",				(void(CFFEntity_Collection::*)(const luabind::adl::object&))&CFFEntity_Collection::AddItem)
			.def("RemoveItem",			(void(CFFEntity_Collection::*)(CBaseEntity*))&CFFEntity_Collection::RemoveItem)
			.def("RemoveItem",			(void(CFFEntity_Collection::*)(const luabind::adl::object&))&CFFEntity_Collection::RemoveItem)
			.def("RemoveAllItems",		&CFFEntity_Collection::RemoveAllItems)
			.def("IsEmpty",				&CFFEntity_Collection::IsEmpty)
			.def("Count",				&CFFEntity_Collection::Count)
			.def("NumItems",			&CFFEntity_Collection::Count)
			.def("HasItem",				(bool(CFFEntity_Collection::*)(CBaseEntity*))&CFFEntity_Collection::HasItem)
			.def("HasItem",				(bool(CFFEntity_Collection::*)(const luabind::adl::object&))&CFFEntity_Collection::HasItem)
			.def("GetItem",				(CBaseEntity*(CFFEntity_Collection::*)(CBaseEntity*))&CFFEntity_Collection::GetItem)
			.def("GetItem",				(CBaseEntity*(CFFEntity_Collection::*)(const luabind::adl::object&))&CFFEntity_Collection::GetItem)
			.def("Element",				&CFFEntity_Collection::Element),

		// CFFEntity_CollectionFilter
		class_<CFFEntity_CollectionFilter>("CF")
			.enum_("FilterId")
			[
				value("kPlayers",			CF_PLAYERS),
				value("kPlayerScout",		CF_PLAYER_SCOUT),
				value("kPlayerSniper",		CF_PLAYER_SNIPER),
				value("kPlayerSoldier",		CF_PLAYER_SOLDIER),
				value("kPlayerDemoman",		CF_PLAYER_DEMOMAN),
				value("kPlayerMedic",		CF_PLAYER_DEMOMAN),
				value("kPlayerHWGuy",		CF_PLAYER_HWGUY),
				value("kPlayerPyro",		CF_PLAYER_PYRO),
				value("kPlayerSpy",			CF_PLAYER_SPY),
				value("kPlayerEngineer",	CF_PLAYER_ENGY),
				value("kPlayerCivilian",	CF_PLAYER_CIVILIAN),

				value("kTeams",				CF_TEAMS),
				value("kTeamSpec",			CF_TEAM_SPECTATOR),
				value("kTeamBlue",			CF_TEAM_BLUE),
				value("kTeamRed",			CF_TEAM_RED),
				value("kTeamYellow",		CF_TEAM_YELLOW),
				value("kTeamGreen",			CF_TEAM_GREEN),

				value("kProjectiles",		CF_PROJECTILES),
				value("kGrenades",			CF_GRENADES),
				
				value("kBuildables",		CF_BUILDABLES),
				value("kDispenser",			CF_BUILDABLE_DISPENSER),
				value("kSentrygun",			CF_BUILDABLE_SENTRYGUN),
				value("kDetpack",			CF_BUILDABLE_DETPACK)
			],
			
		class_<CFFEntity_AmmoTypes>("Ammo")
			.enum_("AmmoType")
			[
				value("kShells",			LUA_AMMO_SHELLS),
				value("kCells",				LUA_AMMO_CELLS),
				value("kNails",				LUA_AMMO_NAILS),
				value("kRockets",			LUA_AMMO_ROCKETS),
				value("kRadiotag",			LUA_AMMO_RADIOTAG),
				value("kDetpack",			LUA_AMMO_DETPACK),
				value("kGren1",				LUA_AMMO_GREN1),
				value("kGren2",				LUA_AMMO_GREN2)
			],

		class_<CFFEntity_Effect_Flags>("EF")
			.enum_("EffectFlagId")
			[
				value("kOnfire",			LUA_EF_ONFIRE),
				value("kConc",				LUA_EF_CONC),
				value("kGas",				LUA_EF_GAS),
				value("kInfect",			LUA_EF_INFECT),
				value("kRadiotag",			LUA_EF_RADIOTAG),
				value("kHeadshot",			LUA_EF_HEADSHOT),
				value("kLegshot",			LUA_EF_LEGSHOT),
				value("kTranq",				LUA_EF_TRANQ),
				value("kCaltrop",			LUA_EF_CALTROP),
				value("kACSpinup",			LUA_EF_ACSPINUP),
				value("kSniperrifle",		LUA_EF_SNIPERRIFLE),
				value("kSpeedlua1",			LUA_EF_SPEED_LUA1),
				value("kSpeedlua2",			LUA_EF_SPEED_LUA2),
				value("kSpeedlua3",			LUA_EF_SPEED_LUA3),
				value("kSpeedlua4",			LUA_EF_SPEED_LUA4),
				value("kSpeedlua5",			LUA_EF_SPEED_LUA5),
				value("kSpeedlua6",			LUA_EF_SPEED_LUA6),
				value("kSpeedlua7",			LUA_EF_SPEED_LUA7),
				value("kSpeedlua8",			LUA_EF_SPEED_LUA8),
				value("kSpeedlua9",			LUA_EF_SPEED_LUA9),
				value("kSpeedlua10",		LUA_EF_SPEED_LUA10)
			],

		class_<CFFEntity_ApplyTo_Flags>("AT")
			.enum_("ApplyToFlagId")
			[
				value("kKillPlayers",		AT_KILL_PLAYERS),
				value("kRespawnPlayers",	AT_RESPAWN_PLAYERS),
				value("kDropItems",			AT_DROP_ITEMS),
				value("kForceDropItems",	AT_FORCE_DROP_ITEMS),
				value("kThrowItems",		AT_THROW_ITEMS),
				value("kForceThrowItems",	AT_FORCE_THROW_ITEMS),
				value("kReturnCarriedItems",	AT_RETURN_CARRIED_ITEMS),
				value("kReturnDroppedItems",	AT_RETURN_DROPPED_ITEMS),
				value("kRemoveRagdolls",	AT_REMOVE_RAGDOLLS),
				value("kRemovePacks",		AT_REMOVE_PACKS),
				value("kRemoveProjectiles",	AT_REMOVE_PROJECTILES),
				value("kRemoveBuildables",	AT_REMOVE_BUILDABLES),
				value("kRemoveDecals",		AT_REMOVE_DECALS),
				value("kEndMap",			AT_END_MAP),

				value("kChangeClassScout",	AT_CHANGECLASS_SCOUT),
				value("kChangeClassSniper",	AT_CHANGECLASS_SNIPER),
				value("kChangeClassSoldier",	AT_CHANGECLASS_SOLDIER),
				value("kChangeClassDemoman",	AT_CHANGECLASS_DEMOMAN),
				value("kChangeClassMedic",	AT_CHANGECLASS_MEDIC),
				value("kChangeClassHWGuy",	AT_CHANGECLASS_HWGUY),
				value("kChangeClassPyro",	AT_CHANGECLASS_PYRO),
				value("kChangeClassSpy",	AT_CHANGECLASS_SPY),
				value("kChangeClassEngineer",	AT_CHANGECLASS_ENGINEER),
				value("kChangeClassCivilian",	AT_CHANGECLASS_CIVILIAN),
				value("kChangeClassRandom",	AT_CHANGECLASS_RANDOM),

				value("kStopPrimedGrens",	AT_STOP_PRIMED_GRENS)
			],

		// CFFMiniTurret
		//class_<CFFMiniTurret>("Turret"),

		// CBaseEntity
		class_<CBaseEntity>("BaseEntity")
			.def("EmitSound",			&CBaseEntity::PlaySound)
			.def("GetName",				&CBaseEntity::GetName)
			.def("GetTeam",				&CBaseEntity::GetTeam)
			.def("GetTeamId",			&CBaseEntity::GetTeamNumber)
			.def("GetId",				&CBaseEntity::entindex)
			.def("GetVelocity",			&CBaseEntity::GetAbsVelocity)
			.def("IsDispenser",			&FFLib::IsDispenser)
			.def("IsGrenade",			&FFLib::IsGrenade)
			.def("IsPlayer",			&CBaseEntity::IsPlayer)
			.def("IsSentryGun",			&FFLib::IsSentrygun)
			.def("IsDetpack",			&FFLib::IsDetpack)
			.def("SetModel",			(void(CBaseEntity::*)(const char*))&CBaseEntity::SetModel)
			.def("SetModel",			(void(CBaseEntity::*)(const char*, int))&CBaseEntity::SetModel)
			.def("SetSkin",				&CBaseEntity::SetSkin)
			.def("GetOrigin",			&CBaseEntity::GetAbsOrigin)
			.def("SetOrigin",			&CBaseEntity::SetAbsOrigin)
			.def("GetAngles",			&CBaseEntity::GetAbsAngles)
			.def("SetAngles",			&CBaseEntity::SetAbsAngles)
			.def("IsOnFire",			&CBaseEntity::IsOnFire)
			.def("GetGravity",			&CBaseEntity::GetGravity)
			.def("SetGravity",			&CBaseEntity::SetGravity)
			.def("SetRenderColor",		(void(CBaseEntity::*)(byte,byte,byte))&CBaseEntity::SetRenderColor)
			.def("SetRenderMode",		&CBaseEntity::SetRenderMode)
			.def("GetFriction",			&CBaseEntity::GetFriction)
			.def("SetFriction",			&CBaseEntity::GetFriction),

		// Buildable base
		class_<CFFBuildableObject>("BaseBuildable")
			.def("GetTeamId",			&CFFBuildableObject::GetTeamNumber)
			.def("GetOwner",			&CFFBuildableObject::GetOwnerPlayer)
			.def("GetTeam",				&CFFBuildableObject::GetOwnerTeam),

		// Dispenser
		class_<CFFDispenser, CFFBuildableObject>("Dispenser"),
		
		// Sentrygun
		class_<CFFSentryGun, CFFBuildableObject>("Sentrygun"),
		
		// Detpack
		class_<CFFDetpack, CFFBuildableObject>("Detpack"),
	
		// CTeam
		class_<CTeam>("BaseTeam")
			.def("AddScore",			&CTeam::AddScore)
			.def("GetNumPlayers",		&CTeam::GetNumPlayers)
			.def("GetPlayer",			&CTeam::GetPlayer)
			.def("GetTeamId",			&CTeam::GetTeamNumber)
			.def("SetName",				&CTeam::SetName),

		// CFFTeam
		class_<CFFTeam, CTeam>("Team")
			.def("SetAllies",			&CFFTeam::SetEasyAllies)
			.def("SetClassLimit",		&CFFTeam::SetClassLimit)
			.def("SetPlayerLimit",		&CFFTeam::SetTeamLimits)
			.enum_("TeamId")
			[
				value("kUnassigned",	TEAM_UNASSIGNED),
				value("kSpectator",		TEAM_SPECTATOR),
				value("kBlue",			TEAM_BLUE),
				value("kRed",			TEAM_RED),
				value("kYellow",		TEAM_YELLOW),
				value("kGreen",			TEAM_GREEN)
			],

		// CFFGrenadeBase
		class_<CFFGrenadeBase, CBaseEntity>("Grenade")
			.def("Type",				&CFFGrenadeBase::GetGrenId)
			.enum_("GrenId")
			[
				value("kNormal",		CLASS_GREN),
				value("kCaltrop",		CLASS_GREN_CALTROP),
				value("kNail",			CLASS_GREN_NAIL),
				value("kMirv",			CLASS_GREN_MIRV),
				value("kMirvlet",		CLASS_GREN_MIRVLET),
				value("kConc",			CLASS_GREN_CONC),
				value("kNapalm",		CLASS_GREN_NAPALM),
				value("kGas",			CLASS_GREN_GAS),
				value("kEmp",			CLASS_GREN_EMP)
			],

		// CBasePlayer
		class_<CBasePlayer, CBaseEntity>("BasePlayer"),

		// CFFPlayer
		class_<CFFPlayer, CBasePlayer>("Player")
			.def("AddAmmo",				&CFFPlayer::LuaAddAmmo)
			.def("AddArmor",			&CFFPlayer::AddArmor)
			.def("AddFrags",			&CFFPlayer::IncrementFragCount)
			.def("AddHealth",			&CFFPlayer::AddHealth)
			.def("GetClass",			&CFFPlayer::GetClassSlot)
			.def("GetName",				&CFFPlayer::GetPlayerName)
			.def("HasItem",				&CFFPlayer::HasItem)
			.def("IsFeetDeepInWater",	&CFFPlayer::IsFeetDeepInWater)
			.def("IsInNoBuild",			&CFFPlayer::IsInNoBuild)
			.def("IsUnderWater",		&CFFPlayer::IsUnderWater)
			.def("IsWaistDeepInWater",	&CFFPlayer::IsWaistDeepInWater)
			.def("IsInAttack1",			&CFFPlayer::IsInAttack1)
			.def("IsInAttack2",			&CFFPlayer::IsInAttack2)
			.def("IsInUse",				&CFFPlayer::IsInUse)
			.def("IsInJump",			&CFFPlayer::IsInJump)
			.def("IsInForward",			&CFFPlayer::IsInForward)
			.def("IsInBack",			&CFFPlayer::IsInBack)
			.def("IsInMoveLeft",		&CFFPlayer::IsInMoveLeft)
			.def("IsInMoveRight",		&CFFPlayer::IsInMoveRight)
			.def("IsInLeft",			&CFFPlayer::IsInLeft)
			.def("IsInRight",			&CFFPlayer::IsInRight)
			.def("IsInRun",				&CFFPlayer::IsInRun)
			.def("IsInReload",			&CFFPlayer::IsInReload)
			.def("IsInSpeed",			&CFFPlayer::IsInSpeed)
			.def("IsInWalk",			&CFFPlayer::IsInWalk)
			.def("IsInZoom",			&CFFPlayer::IsInZoom)
			.def("IsOnGround",			&CFFPlayer::IsOnGround)
			.def("IsInAir",				&CFFPlayer::IsInAir)
			.def("IsDucking",			&CFFPlayer::IsDucking)
			.def("MarkRadioTag",		&CFFPlayer::SetRadioTagged)
			//.def("RemoveAmmo",			(void(CFFPlayer::*)(int, const char*))&CFFPlayer::RemoveAmmo)
			.def("RemoveAmmo",			&CFFPlayer::LuaRemoveAmmo)
			.def("RemoveArmor",			&CFFPlayer::RemoveArmor)
			.def("RemoveLocation",		&CFFPlayer::RemoveLocation)
			.def("Respawn",				&CFFPlayer::KillAndRemoveItems)
			.def("SetDisguisable",		&CFFPlayer::SetDisguisable)
			.def("SetLocation",			&CFFPlayer::SetLocation)
			.def("SetRespawnDelay",		&CFFPlayer::LUA_SetPlayerRespawnDelay)
			//.def("InstaSwitch",			&CFFPlayer::InstaSwitch) -- doing this as part of ApplyToPlayer()
			.def("GiveWeapon",			&CFFPlayer::GiveNamedItem)
			.def("RemoveWeapon",		&CFFPlayer::TakeNamedItem)
			.def("RemoveAllWeapons",	&CFFPlayer::RemoveAllItems)
			.def("IsFeigned",			&CFFPlayer::IsFeigned)
			.def("IsDisguised",			&CFFPlayer::IsDisguised)
			.def("GetDisguisedClass",	&CFFPlayer::GetDisguisedClass)
			.def("GetDisguisedTeam",	&CFFPlayer::GetDisguisedTeam)
			.def("AddEffect",			&CFFPlayer::LuaAddEffect)
			.def("IsEffectActive",		&CFFPlayer::LuaIsEffectActive)
			.def("RemoveEffect",		&CFFPlayer::LuaRemoveEffect)
			.def("GetSteamID",			&CFFPlayer::GetSteamID)
			.def("GetPing",				&CFFPlayer::GetPing)
			.def("GetPacketloss",		&CFFPlayer::GetPacketloss)
			.enum_("ClassId")
			[
				value("kScout",			CLASS_SCOUT),
				value("kSniper",		CLASS_SNIPER),
				value("kSoldier",		CLASS_SOLDIER),
				value("kDemoman",		CLASS_DEMOMAN),
				value("kMedic",			CLASS_MEDIC),
				value("kHwguy",			CLASS_HWGUY),
				value("kPyro",			CLASS_PYRO),
				value("kSpy",			CLASS_SPY),
				value("kEngineer",		CLASS_ENGINEER),
				value("kCivilian",		CLASS_CIVILIAN)
			],

		// CFFInfoScript
		class_<CFFInfoScript, CBaseEntity>("InfoScript")
			.def("Drop",				&CFFInfoScript::Drop)
			.def("Pickup",				&CFFInfoScript::Pickup)
			.def("Respawn",				&CFFInfoScript::Respawn)
			.def("Return",				&CFFInfoScript::Return)
			.def("IsCarried",			&CFFInfoScript::IsCarried)
			.def("IsReturned",			&CFFInfoScript::IsReturned)
			.def("IsDropped",			&CFFInfoScript::IsDropped)
			.def("IsActive",			&CFFInfoScript::IsActive)
			.def("IsInactive",			&CFFInfoScript::IsInactive)
			.def("IsRemoved",			&CFFInfoScript::IsRemoved)
			.def("Remove",				&CFFInfoScript::LUA_Remove)
			.def("EntName",				&CFFInfoScript::EntName)
			.def("Restore",				&CFFInfoScript::LUA_Restore)
			.def("SetBotGoalInfo",		&CFFInfoScript::SetBotGoalInfo),

		// CFuncFFScript - trigger_ff_script
		class_<CFuncFFScript>("TriggerScript")
			.def("IsActive",			&CFuncFFScript::IsActive)
			.def("IsInactive",			&CFuncFFScript::IsInactive)
			.def("IsRemoved",			&CFuncFFScript::IsRemoved)
			.def("Remove",				&CFuncFFScript::LuaRemove)
			.def("Restore",				&CFuncFFScript::LuaRestore)
			.def("IsTouching",			&CFuncFFScript::IsTouching),

		// CBeam
		class_<CBeam, CBaseEntity>("Beam")
			.def("SetColor",			&CBeam::SetColor),

		// global functions
		def("BroadCastMessage",			&FFLib::BroadcastMessage),
		def("BroadCastMessageToPlayer",	&FFLib::SendPlayerMessage),
		def("BroadCastSound",			&FFLib::BroadcastSound),
		def("BroadCastSoundToPlayer",	&FFLib::SendPlayerSound),
		def("CastToBeam",				&FFLib::CastToBeam),
		def("CastToPlayer",				&FFLib::CastToPlayer),
		def("CastToInfoScript",			&FFLib::CastToItemFlag),
		def("CastToTriggerScript",		&FFLib::CastToTriggerScript),
		def("CastToGrenade",			&FFLib::CastToGrenade),
		def("CastToDispenser",			&FFLib::CastToDispenser),
		def("CastToSentrygun",			&FFLib::CastToSentrygun),
		def("CastToDetpack",			&FFLib::CastToDetpack),
		//def("CastToTurret",				&FFLib::CastToTurret),
		def("GetEntity",				&FFLib::GetEntity),
		def("GetEntityByName",			&FFLib::GetEntityByName),
		def("GetEntitiesByName",		&FFLib::GetEntitiesByName,			return_stl_iterator),
		def("GetEntitiesInSphere",		&FFLib::GetEntitiesInSphere,		return_stl_iterator),
		def("GetInfoScriptById",		&FFLib::GetInfoScriptById),
		def("GetInfoScriptByName",		&FFLib::GetInfoScriptByName),
		def("GetTriggerScriptByName",	&FFLib::GetTriggerScriptByName),
		def("GetPlayer",				&FFLib::GetPlayer),
		def("GetTeam",					&FFLib::GetTeam),
		def("GetGrenade",				&FFLib::GetGrenade),
		def("IsPlayer",					&FFLib::IsPlayer),
		def("IsDispenser",				&FFLib::IsDispenser),
		def("IsSentrygun",				&FFLib::IsSentrygun),
		def("IsDetpack",				&FFLib::IsDetpack),
		def("IsGrenade",				&FFLib::IsGrenade),
		def("IsTurret",					&FFLib::IsTurret),
		def("AreTeamsAllied",			(bool(*)(CTeam*, CTeam*))&FFLib::AreTeamsAllied),
		def("AreTeamsAllied",			(bool(*)(int, int))&FFLib::AreTeamsAllied),
		def("ConsoleToAll",				&FFLib::ConsoleToAll),
		def("NumPlayers",				&FF_NumPlayers),
		def("PrecacheModel",			&CBaseEntity::PrecacheModel),
		def("PrecacheSound",			&CBaseEntity::PrecacheScriptSound),
		def("RandomFloat",				&FFLib::RandomFloat),
		def("RandomInt",				&FFLib::RandomInt),
		def("RemoveEntity",				&FFLib::RemoveEntity),
		def("RespawnAllPlayers",		&FFLib::RespawnAllPlayers),
		def("KillAndRespawnAllPlayers",	&FFLib::KillAndRespawnAllPlayers),
		def("SetGlobalRespawnDelay",	&FFLib::SetGlobalRespawnDelay),
		def("SetPlayerLimit",			&FFLib::SetPlayerLimit),
		def("SetPlayerLimits",			&FFLib::SetPlayerLimits),
		def("SetClassLimits",			&FFLib::SmartClassLimits),
		def("SetTeamClassLimit",		&FFLib::SetTeamClassLimit),
		def("SetTeamName",				&FFLib::SetTeamName),
		def("SmartMessage",				&FFLib::SmartMessage),
		def("SmartSound",				&FFLib::SmartSound),
		def("SmartTeamMessage",			&FFLib::SmartTeamMessage),
		def("SmartTeamSound",			&FFLib::SmartTeamSound),
		def("GetServerTime",			&FFLib::GetServerTime),
		def("UseEntity",				&FFLib::UseEntity),
		def("IncludeScript",			&FFLib::IncludeScript),
		def("ApplyToAll",				&FFLib::ApplyToAll),
		def("ApplyToTeam",				&FFLib::ApplyToTeam),
		def("ApplyToPlayer",			&FFLib::ApplyToPlayer),
		def("ResetMap",					&FFLib::ResetMap),
		def("GetConvar",				&FFLib::GetConvar),
		def("SetConvar",				&FFLib::SetConvar),
		def("GetSteamID",				&FFLib::GetSteamID),
		def("GetPing",					&FFLib::GetPing),
		def("GetPacketloss",			&FFLib::GetPacketloss),
		def("PrintBool",				&FFLib::PrintBool),
		def("GoToIntermission",			&FFLib::GoToIntermission),
		def("OutputEvent",				(void(*)(const char*, const char*))&FFLib::FireOutput),
		def("OutputEvent",				(void(*)(const char*, const char*, const char*))&FFLib::FireOutput),
		def("OutputEvent",				(void(*)(const char*, const char*, const char*, float))&FFLib::FireOutput),
		def("OutputEvent",				(void(*)(const char*, const char*, const char*, float, unsigned int))&FFLib::FireOutput),
		def("GetPlayerByID",			&FFLib::GetPlayerByID),	// TEMPORARY

		class_<Omnibot_GoalTypes>("Bot")
		.enum_("GoalType")
		[
			value("kNone",				Omnibot::kNone),
			value("kBackPack",			Omnibot::kBackPack),
			value("kFlag",				Omnibot::kFlag),
			value("kFlagCap",			Omnibot::kFlagCap)
		]
	];
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
