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
#include "ff_utils.h"
#include "ff_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Better way of doing this maybe?
CFFEntitySystem entsys;
CFFEntitySystemHelper *helper; // global variable.. OH NOES!

ConVar mp_respawndelay( "mp_respawndelay", "0", 0, "Time (in seconds) for spawn delays. Can be overridden by LUA." );

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

void CFFEntitySystemHelper::OnThink( void )
{
	entsys.RunPredicates( NULL, NULL, "tick" );
	SetNextThink( gpGlobals->curtime + 1.0f );
}

void CFFEntitySystemHelper::Precache( void )
{
	entsys.RunPredicates( NULL, NULL, "precache" );
}

//============================================================================
// CFFEntitySystem implementation
//============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor, sets up the vm and that's all!
//----------------------------------------------------------------------------
CFFEntitySystem::CFFEntitySystem()
{
	DevMsg( "[SCRIPT] Attempting to start up the entity system...\n" );

	// Initialise this to false
	m_ScriptExists = false;

}

//----------------------------------------------------------------------------
// Purpose: Destructor!
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
// Purpose: This loads the correct script for our map
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
	lua_baselibopen(L);
	
	//lua_atpanic(L, HandleError);

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
void CFFEntitySystem::FFLibOpen()
{
	lua_register( L, "ConsoleToAll", ConsoleToAll );
	lua_register( L, "GetPlayerTeam", GetPlayerTeam );
	lua_register( L, "GetPlayerClass", GetPlayerClass );
	lua_register( L, "GetPlayerName", GetPlayerName );
	lua_register( L, "AddTeamScore", AddTeamScore );
	lua_register( L, "SpawnEntityAtPlayer", SpawnEntityAtPlayer );
	lua_register( L, "PlayerHasItem", PlayerHasItem );
	lua_register( L, "RemoveItem", RemoveItem );
	lua_register( L, "ReturnItem", ReturnItem );
	lua_register( L, "Pickup", Pickup );
	lua_register( L, "Respawn", Respawn );
	lua_register( L, "DropItem", DropItem );
	lua_register( L, "SetModel", SetModel);
	lua_register( L, "PrecacheModel", PrecacheModel );
	lua_register( L, "EmitSound", EmitSound );
	lua_register( L, "PrecacheSound", PrecacheSound );
	lua_register( L, "BroadCastSound", BroadCastSound );
	lua_register( L, "BroadCastSoundToPlayer", BroadCastSoundToPlayer );
	lua_register( L, "BroadCastMessage", BroadCastMessage );
	lua_register( L, "BroadCastMessageToPlayer", BroadCastMessageToPlayer );
	lua_register( L, "RespawnAllPlayers", RespawnAllPlayers );
	lua_register( L, "RespawnPlayer", RespawnPlayer );
	lua_register( L, "UseEntity", UseEntity );
	lua_register( L, "NumPlayersOnTeam", NumPlayersOnTeam );
	lua_register( L, "GetPlayerOnTeam", GetPlayerOnTeam );
	lua_register( L, "NumPlayers", NumPlayers );
	lua_register( L, "GetPlayer", GetPlayer );
	lua_register( L, "IncludeScript", IncludeScript );
	lua_register( L, "SetTeamClassLimit", SetTeamClassLimit );
	lua_register( L, "SetTeamPlayerLimit", SetTeamPlayerLimit );
	lua_register( L, "Random", Random );
	lua_register( L, "rand", Random );
	lua_register( L, "SetTeamAllies", SetTeamAllies );
	lua_register( L, "AddAmmo", GiveAmmo );
	lua_register( L, "RemoveAmmo", RemoveAmmo );
	lua_register( L, "AddArmor", AddArmor );
	lua_register( L, "AddHealth", AddHealth );
	lua_register( L, "AddFrags", AddFrags );
	lua_register( L, "MarkRadioTag", MarkRadioTag );
	lua_register( L, "SetPlayerLocation", SetPlayerLocation );
	lua_register( L, "SetPlayerDisguisable", SetPlayerDisguisable );
	lua_register( L, "SetPlayerRespawnDelay", SetPlayerRespawnDelay );
	lua_register( L, "SetGlobalRespawnDelay", SetGlobalRespawnDelay );
}

//----------------------------------------------------------------------------
// Purpose: Print something into every players' console
//          void ConsoleMessage( const char* message );
//----------------------------------------------------------------------------
int CFFEntitySystem::ConsoleToAll( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 1 )
	{
		const char *msg = lua_tostring(L, 1);

		DevMsg( msg );
		DevMsg( "\n" );
	}

	// return the number of results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Include a script from the /ff/maps/includes directory
//          void IncludeScript( const char* script );
//----------------------------------------------------------------------------
int CFFEntitySystem::IncludeScript( lua_State *L )
{
	if( lua_gettop(L) == 1 )
	{
		const char *script = (const char *)lua_tostring( L, 1 );
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
			strcpy(realscript, "maps/includes/" );
			strcat(realscript, script);
			strcat(realscript, ".lua");

			LoadLuaFile( L, realscript );
		}
		else
		{
			DevWarning("[SCRIPT] Warning: Invalid filename: %s\n", script);
		}
	}

	// No results
	return 0;
}

// ---- Accessors for finding out the players in the game ---
int CFFEntitySystem::NumPlayersOnTeam( lua_State* L )
{
	if( lua_gettop(L) == 1 )
	{
		lua_pushnumber( L, FF_NumPlayersOnTeam( (int)lua_tonumber(L, 1) ) );
		return 1;
	}

	return 0;
}
int CFFEntitySystem::GetPlayerOnTeam( lua_State* L )
{
	if( lua_gettop(L) == 2 )
	{
		lua_pushnumber( L, FF_GetPlayerOnTeam( (int)lua_tonumber(L, 1), (int)lua_tonumber(L, 2) ) );
		return 1;
	}

	return 0;
}
int CFFEntitySystem::NumPlayers( lua_State* L )
{
	lua_pushnumber( L, FF_NumPlayers() );
	return 1;
}
int CFFEntitySystem::GetPlayer( lua_State* L )
{
	if( lua_gettop(L) == 1 )
	{
		lua_pushnumber( L, FF_GetPlayer( (int)lua_tonumber(L, 1) ) );
		return 1;
	}

	return 0;
}


//----------------------------------------------------------------------------
// Purpose: Returns the team number of a player
//          int PlayerTeam( int player_index );
//----------------------------------------------------------------------------
int CFFEntitySystem::GetPlayerTeam( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 1 )
	{
		int player_id = (int)lua_tonumber( L, 1 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player_id );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *Player = ToFFPlayer( ent );

			lua_pushnumber( L, Player->GetTeamNumber() );
		}
		else
		{
			lua_pushnumber( L, 0 );
		}

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Returns the class of a player
//          int GetPlayerClass( int player_index );
//----------------------------------------------------------------------------
int CFFEntitySystem::GetPlayerClass( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 1 )
	{
		int player_id = (int)lua_tonumber( L, 1 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player_id );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *Player = ToFFPlayer( ent );

			const CFFPlayerClassInfo &pPlayerClassInfo = Player->GetFFClassData( );

			lua_pushnumber( L, Class_StringToInt( pPlayerClassInfo.m_szClassName ) );
		}
		else
		{
			lua_pushnumber( L, -1 );
		}

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Returns the name of a player
//          int GetPlayerName( int player_index );
//----------------------------------------------------------------------------
int CFFEntitySystem::GetPlayerName( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 1 )
	{
		int player_id = (int)lua_tonumber( L, 1 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player_id );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *Player = ToFFPlayer( ent );

			lua_pushstring( L, Player->GetPlayerName() );
		}
		else
		{
			lua_pushstring( L, "unknown" );
		}

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Adds points to the given team
//          int AddTeamScore( int team_id, int num_points );
//----------------------------------------------------------------------------
int CFFEntitySystem::AddTeamScore( lua_State *L )
{
	int n = lua_gettop(L);

	// A two argument'd function
	if( n == 2 )
	{
		int team_id = (int)lua_tonumber( L, 1 );
		int num_points = (int)lua_tonumber( L, 2 );

		CTeam *Team = GetGlobalTeam( team_id );

		if ( Team != NULL )
		{
			Team->AddScore( num_points );
			DevMsg( "[SCRIPT] Successfully adding %d points to team %d; New score: %d\n", num_points, team_id, Team->GetScore());
			lua_pushnumber( L, 1 );
		} 
		else 
		{
			DevMsg( "[SCRIPT] Failed adding %d points to team %d\n", num_points, team_id );
			lua_pushnumber( L, 0 );
		}

		// 1 results
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Determine if a given player has an item or not
//          int PlayerHasItem( int player_id, string item_name );
//----------------------------------------------------------------------------
int CFFEntitySystem::PlayerHasItem( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 2 )
	{
		int player_id = (int)lua_tonumber( L, 1 );
		const char *itemname = lua_tostring( L, 2 );
		bool ret = false;

		CBasePlayer *ent = UTIL_PlayerByIndex( player_id );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer( ent );

			// get all info_ff_scripts
			CFFItemFlag *pEnt = (CFFItemFlag*)gEntList.FindEntityByClassname( NULL, "info_ff_script" );

			while( pEnt != NULL )
			{
				// Tell the ent that it died
				if ( pEnt->GetOwnerEntity() == pPlayer && FStrEq( STRING(pEnt->GetEntityName()), itemname ) )
				{
					DevMsg("[SCRIPT] found item %d: %s\n", ENTINDEX(pEnt), itemname);
					ret = true;
				}

				// Next!
				pEnt = (CFFItemFlag*)gEntList.FindEntityByClassname( pEnt, "info_ff_script" );
			}
		}

		lua_pushboolean( L, ret );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}


//----------------------------------------------------------------------------
// Purpose: Spawns a given entity at the player's location.
//					Useful right now for testing stuff out.. probably won't be in
//					final version.
//          int AddTeamScore( string entityclass, int player_id, string entityname );
//----------------------------------------------------------------------------
int CFFEntitySystem::SpawnEntityAtPlayer( lua_State *L )
{
	int n = lua_gettop(L);

	// A two argument'd function
	if( n == 3 )
	{
		const char *entclass = lua_tostring(L, 1);
		int player_id = (int)lua_tonumber( L, 2 );
		const char *entname = lua_tostring(L, 3);

		CBasePlayer *ent = UTIL_PlayerByIndex( player_id );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *p = ToFFPlayer( ent );

			Vector vOrigin = p->GetAbsOrigin();
			Vector vVelocity = p->GetAbsVelocity();

			CBaseEntity *ent = CreateEntityByName( entclass );

			UTIL_SetOrigin( ent, vOrigin );
			ent->SetAbsAngles(QAngle(0,0,0)); //make the model stand on end
			ent->SetAbsVelocity(vVelocity);
			ent->SetLocalAngularVelocity(QAngle(0,0,0));
			ent->SetName( MAKE_STRING(entname) );
			ent->Spawn();
			ent->SetOwnerEntity( p );

			// Set the speed and the initial transmitted velocity
			//pCaltrop->SetAbsVelocity( vecVelocity );
			//pCaltrop->SetElasticity( p->GetGrenadeElasticity() );
			//pCaltrop->ChangeTeam( p->GetOwnerEntity()->GetTeamNumber() );
			//pCaltrop->SetGravity( GetGrenadeGravity() + 0.2f );
			//pCaltrop->SetFriction( GetGrenadeFriction() );

			// TODO: Return the actual entity here
			lua_pushnumber( L, ENTINDEX( ent ) );
		}
		else
		{
			lua_pushnumber( L, -1 );
		}

		// 1 results
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Removes an item from the game.
//          int RemoveItem( int player_index );
//----------------------------------------------------------------------------
int CFFEntitySystem::RemoveItem( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 1 )
	{
		int ent_id = (int)lua_tonumber( L, 1 );

		CBaseEntity *ent = UTIL_EntityByIndex( ent_id );
		if (ent)
		{
			UTIL_Remove( ent );

			lua_pushnumber( L, 1 );
		}
		else
		{
			lua_pushnumber( L, 0 );
		}

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Returns an item to its starting position.
//          int ReturnItem( string item_name );
//----------------------------------------------------------------------------
int CFFEntitySystem::ReturnItem( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 1 )
	{
		const char *itemname = lua_tostring( L, 1 );

		bool ret = false;

		// get all info_ff_scripts
		CFFItemFlag *pEnt = (CFFItemFlag*)gEntList.FindEntityByClassname( NULL, "info_ff_script" );

		while( pEnt != NULL )
		{
			if ( FStrEq( STRING(pEnt->GetEntityName()), itemname ) )
			{
				// if this is the right one, then respawn it.
				DevMsg("[SCRIPT] found item %d: %s\n", ENTINDEX(pEnt), itemname);
				pEnt->Return();
				ret = true;
			}

			// Next!
			pEnt = (CFFItemFlag*)gEntList.FindEntityByClassname( pEnt, "info_ff_script" );
		}

		lua_pushnumber( L, ret );

		// 1 result
		return 0;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Attaches an item to a player, causing an event to be fired when
//					the player carrying player dies.
//          int PickupItem( int item_id, int player_index );
//----------------------------------------------------------------------------
int CFFEntitySystem::Pickup( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		int item_id = (int)lua_tonumber( L, 1 );
		int player_id = (int)lua_tonumber( L, 2 );

		bool ret = false;

		CBaseEntity *item = UTIL_EntityByIndex( item_id );
		CBasePlayer *player = UTIL_PlayerByIndex( player_id );
		if (item && player && player->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer( player );
			CFFItemFlag *pItem = (CFFItemFlag*)item;

			pItem->Pickup(pPlayer);

			ret = true;
		}

		lua_pushnumber( L, ret );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Activates this item/goal, causing it to return in x seconds
//          int PickupItem( int item_id, int delay );
//----------------------------------------------------------------------------
int CFFEntitySystem::Respawn( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		int item_id = (int)lua_tonumber( L, 1 );
		float delay = (float)lua_tonumber( L, 2 );

		bool ret = false;

		CBaseEntity *item = UTIL_EntityByIndex( item_id );
		if (item)
		{
			CFFItemFlag *pItem = (CFFItemFlag*)item;

			pItem->Respawn(delay);

			ret = true;
		}

		lua_pushnumber( L, ret );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Causes the item to be dropped from the player to the ground.
//          int DropItem( int item_id, int delay );
//----------------------------------------------------------------------------
int CFFEntitySystem::DropItem( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		int item_id = (int)lua_tonumber( L, 1 );
		float delay = (float)lua_tonumber( L, 2 );

		bool ret = false;

		CBaseEntity *item = UTIL_EntityByIndex( item_id );
		if (item)
		{
			CFFItemFlag *pItem = (CFFItemFlag*)item;

			pItem->Drop(delay);

			ret = true;
		}

		lua_pushnumber( L, ret );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Changes the model of the given entity, to the model passed in.
//          int DropItem( int item_id, int delay );
//----------------------------------------------------------------------------
int CFFEntitySystem::SetModel( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 || n == 3)
	{
		int item_id = (int)lua_tonumber( L, 1 );
		const char *model = lua_tostring( L, 2 );
		int skin = (int)lua_tonumber( L, 3 );

		bool ret = false;

		CBaseEntity *item = UTIL_EntityByIndex( item_id );
		if (item)
		{
			UTIL_SetModel(item, model);

			if (n >= 3)
			{
				((CBaseAnimating *)item)->m_nSkin = skin;
			}

			ret = true;
		}

		lua_pushnumber( L, ret );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Returns an item to its starting position.
//          int UseEntity( string entity_name );
//----------------------------------------------------------------------------
int CFFEntitySystem::UseEntity( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 3 )
	{
		const char *itemname = lua_tostring( L, 1 );
		const char *classname = lua_tostring( L, 2 );
		const char *action = lua_tostring( L, 3 );

		bool ret = false;

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
				ret = true;
			}

			// Next!
			pEnt = gEntList.FindEntityByClassname( pEnt, classname );
		}

		lua_pushnumber( L, ret );

		// 1 result
		return 0;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Precaches a model so HL won't crash when you set the model
//          int PrecacheModel( string modelname );
//----------------------------------------------------------------------------
int CFFEntitySystem::PrecacheModel( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 2 )
	{
		int item_id = (int)lua_tonumber( L, 1 );
		const char *model = lua_tostring( L, 2 );

		bool ret = false;

		CBaseEntity *item = UTIL_EntityByIndex( item_id );
		if (item)
		{
			item->PrecacheModel(model);

			ret = true;
		}

		lua_pushnumber( L, ret );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Emits a sound at an entity
//          int EmitSound( int item_id, string sound );
//----------------------------------------------------------------------------
int CFFEntitySystem::EmitSound( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 2 )
	{
		int item_id = (int)lua_tonumber( L, 1 );
		const char *sound = lua_tostring( L, 2 );

		bool ret = false;

		CBaseEntity *item = UTIL_EntityByIndex( item_id );
		if (item)
		{
			CPASAttenuationFilter sndFilter( item );
			item->EmitSound( sndFilter, item_id, sound );	

			ret = true;
		}

		lua_pushnumber( L, ret );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Precaches a sound so HL will play it
//          int PrecacheSound( string modelname );
//----------------------------------------------------------------------------
int CFFEntitySystem::PrecacheSound( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 1 )
	{
		const char *soundname = lua_tostring( L, 1 );

		DevMsg("[entsys] Precaching sound %s\n", soundname);
		helper->PrecacheScriptSound(soundname);

		// 1 result
		lua_pushboolean( L, true );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Broadcasts a sound to all players on the server
//          int BroadcastSound( string name );
//----------------------------------------------------------------------------
int CFFEntitySystem::BroadCastSound( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 1 )
	{
		const char *soundname = lua_tostring( L, 1 );

		DevMsg("[entsys] Broadcasting sound %s\n", soundname);
		CBroadcastRecipientFilter filter;
		helper->EmitSound( filter, helper->entindex(), soundname );

		// 1 result
		lua_pushboolean( L, true );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Broadcasts a sound to a single player
//          int BroadcastSoundToPlayer( string name, number player );
//----------------------------------------------------------------------------
int CFFEntitySystem::BroadCastSoundToPlayer( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		const char *soundname = lua_tostring( L, 1 );
		int player = (int)lua_tonumber( L, 2 );

		DevMsg("[entsys] Broadcasting sound %s\n", soundname);
		CSingleUserRecipientFilter filter(UTIL_PlayerByIndex(player));
		helper->EmitSound( filter, helper->entindex(), soundname );

		// 1 result
		lua_pushboolean( L, true );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Broadcasts a message to all players on the server
//          int BroadcastMessage( string name );
//----------------------------------------------------------------------------
int CFFEntitySystem::BroadCastMessage( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 1 )
	{
		const char *message = lua_tostring( L, 1 );

		DevMsg("[entsys] Broadcasting message: \"%s\"\n", message);

		CBroadcastRecipientFilter filter;
		UserMessageBegin(filter, "GameMessage");
			WRITE_STRING(message);
		MessageEnd();

		// 1 result
		lua_pushboolean( L, true );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Broadcasts a message to all players on the server
//          int BroadcastMessageToPlayer( string name, number player);
//----------------------------------------------------------------------------
int CFFEntitySystem::BroadCastMessageToPlayer( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		const char *message = lua_tostring( L, 1 );
		int player = (int)lua_tonumber( L, 2 );

		CSingleUserRecipientFilter filter(UTIL_PlayerByIndex(player));
		UserMessageBegin(filter, "GameMessage");
			WRITE_STRING(message);
		MessageEnd();

		// 1 result
		lua_pushboolean( L, true );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Respawn all players (i.e. for round end in epicentre/hunted)
//          int RespawnAllPlayers(  );
//----------------------------------------------------------------------------
int CFFEntitySystem::RespawnAllPlayers( lua_State *L )
{
	int n = lua_gettop(L);

	// A zero argument'd function
	if( n == 0 )
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

		// 1 result
		lua_pushboolean( L, true );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Respawn player
//          int RespawnPlayer( player );
//----------------------------------------------------------------------------
int CFFEntitySystem::RespawnPlayer( lua_State *L )
{
	int n = lua_gettop(L);

	// A one argument'd function
	if( n == 1 )
	{
		int player = (int)lua_tonumber( L, 1 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer( ent );
			pPlayer->KillAndRemoveItems();
		}

		// 1 result
		lua_pushboolean( L, true );
		return 1;
	}

	// No results
	return 0;
}


//----------------------------------------------------------------------------
// Purpose: Sets the limit for a particular class on a team
//          int SetTeamClassLimit( team, class, limit );
//----------------------------------------------------------------------------
int CFFEntitySystem::SetTeamClassLimit( lua_State *L )
{
	int n = lua_gettop(L);

	// A 3 argument'd function
	if( n == 3 )
	{
		int team = (int)lua_tonumber( L, 1 );
		int playerclass = (int)lua_tonumber( L, 2 );
		int limit = (int)lua_tonumber( L, 3 );

		CFFTeam *pTeam = (CFFTeam *) GetGlobalTeam( team );

		if (pTeam)
		{
			pTeam->SetClassLimit( playerclass, limit );
			pTeam->UpdateLimits();
		}

		DevMsg("Set class limit for team %d, class %d to %d\n", team, playerclass, limit);

		// 1 result
		lua_pushboolean( L, true );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Sets the total number of players that can be on a team
//          int SetTeamPlayerLimit( team, class, limit );
//----------------------------------------------------------------------------
int CFFEntitySystem::SetTeamPlayerLimit( lua_State *L )
{
	int n = lua_gettop(L);

	// A 2 argument'd function
	if( n == 2 )
	{
		int team = (int)lua_tonumber( L, 1 );
		int limit = (int)lua_tonumber( L, 2 );

		CFFTeam *pTeam = (CFFTeam *) GetGlobalTeam( team );

		if (pTeam)
		{
			pTeam->SetTeamLimits( limit );
			pTeam->UpdateLimits();
		}

		DevMsg("Set player limit for team %d to %d\n", team, limit);

		// 1 result
		lua_pushboolean( L, true );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Sets the allies of a particular team
//          int SetTeamAllies( team, allies );
//----------------------------------------------------------------------------
int CFFEntitySystem::SetTeamAllies( lua_State *L )
{
	int n = lua_gettop(L);

	// A 3 argument'd function
	if( n == 2 )
	{
		int team = (int)lua_tonumber( L, 1 );
		int allies = (int)lua_tonumber( L, 2 );

		CFFTeam *pTeam = (CFFTeam *) GetGlobalTeam( team );

		if (pTeam)
		{
			pTeam->SetAllies(allies);
			DevMsg("Set allies for team %d to: %d\n", team, allies);
		}
		else
		{
			Warning("Unable to set allies for team %d to %d\n");
		}

		// 1 result
		lua_pushboolean( L, true );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Gives ammo to a player
//          int GiveAmmo( int player, string ammotype, int amount );
//----------------------------------------------------------------------------
int CFFEntitySystem::GiveAmmo( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 3 )
	{
		bool ret = false;
		int player = (int)lua_tonumber( L, 1 );
		const char *ammo = lua_tostring( L, 2 );
		int amount = (int)lua_tonumber( L, 3 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer( ent );

			int dispensed = 0;

			if (FStrEq(ammo, "AMMO_GREN1"))
				dispensed = pPlayer->AddPrimaryGrenades( amount );
			else if (FStrEq(ammo, "AMMO_GREN2"))
				dispensed = pPlayer->AddSecondaryGrenades( amount );
			else
				dispensed = pPlayer->GiveAmmo( amount, ammo, true );

			if (dispensed > 0)
				SetVar(L, "dispensedammo", true);

			ret = true;
		}

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Removes ammo from a player
//          int RemoveAmmo( int player, string ammotype );
//----------------------------------------------------------------------------
int CFFEntitySystem::RemoveAmmo( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 3 )
	{
		bool ret = false;
		int player = (int)lua_tonumber( L, 1 );
		const char *ammo = lua_tostring( L, 2 );
		int amount = (int)lua_tonumber( L, 3 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer( ent );

			pPlayer->RemoveAmmo(amount, ammo);
			ret = true;
		}

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Adds armor to player
//          int AddArmor( int player, int amount );
//----------------------------------------------------------------------------
int CFFEntitySystem::AddArmor( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		bool ret = false;
		int player = (int)lua_tonumber( L, 1 );
		int amount = (int)lua_tonumber( L, 2 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer( ent );

			if (pPlayer->AddArmor( amount ) > 0)
				SetVar(L, "dispensedammo", true);
			ret = true;
		}

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Adds Health to player
//          int AddHealth( int player, int amount );
//----------------------------------------------------------------------------
int CFFEntitySystem::AddHealth( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		bool ret = false;
		int player = (int)lua_tonumber( L, 1 );
		int amount = (int)lua_tonumber( L, 2 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer( ent );

			if (pPlayer->TakeHealth( amount, DMG_GENERIC ) > 0)
				SetVar(L, "dispensedammo", true);
			ret = true;
		}

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Adds Frags to a player
//          int AddFrags( int player, int amount );
//----------------------------------------------------------------------------
int CFFEntitySystem::AddFrags( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		bool ret = false;
		int player = (int)lua_tonumber( L, 1 );
		int amount = (int)lua_tonumber( L, 2 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer( ent );

			pPlayer->IncrementFragCount( amount );
			ret = true;
		}

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}


//----------------------------------------------------------------------------
// Purpose: Marks a player as having been hit with the radio tag.
//          int MarkRadioTag( int player, int duration );
//----------------------------------------------------------------------------
int CFFEntitySystem::MarkRadioTag( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		bool ret = false;
		int player = (int)lua_tonumber( L, 1 );
		float duration = (float)lua_tonumber( L, 2 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player );
		if (ent && ent->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer( ent );

			pPlayer->m_bRadioTagged = true;
			pPlayer->m_flRadioTaggedStartTime = gpGlobals->curtime;
			pPlayer->m_flRadioTaggedDuration = duration; // time in seconds for the radio tag duration... change to whatever you want
			pPlayer->m_pWhoTaggedMe = NULL; 
			ret = true;
		}

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Sets the player's location text
//          int SetPlayerLocation( int player, string name, int team );
//----------------------------------------------------------------------------
int CFFEntitySystem::SetPlayerLocation( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 3 )
	{
		bool ret = false;
		int player = (int)lua_tonumber( L, 1 );
		const char *name = lua_tostring( L, 2 );
		//int r = (int)lua_tonumber( L, 3 );
		//int g = (int)lua_tonumber( L, 4 );
		//int b = (int)lua_tonumber( L, 5 );
		int iTeam = ( int )lua_tonumber( L, 3 ); // added

		CBasePlayer *ent = UTIL_PlayerByIndex( player );
		if (ent && ent->IsPlayer())
		{
			if( ToFFPlayer( ent )->CanUpdateLocation( name ) )
			{
				// do the stuff!
				CSingleUserRecipientFilter filter( ent );
				filter.MakeReliable();	// added
				UserMessageBegin( filter, "SetPlayerLocation" );
					WRITE_STRING( name );
					//WRITE_CHAR(r);
					//WRITE_CHAR(g);
					//WRITE_CHAR(b);
					WRITE_SHORT( iTeam - 1 ); // changed
				MessageEnd();

				ret = true;
			}
		}

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}


//----------------------------------------------------------------------------
// Purpose: Sets the player's ability to disguise
//          int SetPlayerDisguisable( int player, boolean disguisable );
//----------------------------------------------------------------------------
int CFFEntitySystem::SetPlayerDisguisable( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		bool ret = false;
		int player = (int)lua_tonumber( L, 1 );
		bool disguisable = lua_toboolean( L, 2 )!=0;

		CBasePlayer *ent = UTIL_PlayerByIndex( player );
		if (ent && ent->IsPlayer())
		{
			ToFFPlayer(ent)->SetDisguisable( disguisable );
		}

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Sets the player's LUA controlled individual spawn delay
//          int SetPlayerRespawnDelay( int player, float delay );
//----------------------------------------------------------------------------
int CFFEntitySystem::SetPlayerRespawnDelay( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 2 )
	{
		bool ret = false;
		int player = (int)lua_tonumber( L, 1 );
		float delay = (float)lua_tonumber( L, 2 );

		CBasePlayer *ent = UTIL_PlayerByIndex( player );
		if( ent && ent->IsPlayer() )
		{
			ToFFPlayer( ent )->LUA_SetPlayerRespawnDelay( delay );
		}

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Sets the global spawn delay (mp_spawndelay)
//          int SetGlobalRespawnDelay( float delay );
//----------------------------------------------------------------------------
int CFFEntitySystem::SetGlobalRespawnDelay( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool ret = false;
		float delay = (float)lua_tonumber( L, 1 );

		mp_respawndelay.SetValue( max( 0.0f, delay ) );

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Sets the limit for a particular class on a team
//          int SetTeamClassLimit( team, class, limit );
//----------------------------------------------------------------------------
int CFFEntitySystem::Random( lua_State *L )
{
	int n = lua_gettop(L);

	int ret = 0;

	if( n == 2 )
	{
		int min = (int)lua_tonumber( L, 1 );
		int max = (int)lua_tonumber( L, 2 );
		ret = rand()%(max-min) + min;
	}
	else if ( n == 1 )
	{
		int max = (int)lua_tonumber( L, 1 );
		ret = rand()%max;
	}
	else
	{
		ret = rand();
	}

	// 1 result
	lua_pushnumber( L, ret );
	return 1;
}

//----------------------------------------------------------------------------
// Purpose: Handles Error
//----------------------------------------------------------------------------
int CFFEntitySystem::HandleError( lua_State *L )
{
	const char *error = lua_tostring(L, -1);
	Warning("[SCRIPT] %s", error);

	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Sets a variable
//----------------------------------------------------------------------------
void CFFEntitySystem::SetVar( lua_State *L, const char *name, const char *value )
{
	lua_pushstring(L, value);
	lua_setglobal(L, name);
}
void CFFEntitySystem::SetVar( lua_State *L, const char *name, int value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}
void CFFEntitySystem::SetVar( lua_State *L, const char *name, float value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}


void CFFEntitySystem::SetVar( const char *name, const char *value )
{
	lua_pushstring(L, value);
	lua_setglobal(L, name);
}
void CFFEntitySystem::SetVar( const char *name, int value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}
void CFFEntitySystem::SetVar( const char *name, float value )
{
	lua_pushnumber(L, value);
	lua_setglobal(L, name);
}

const char *CFFEntitySystem::GetString( const char *name )
{
	lua_getglobal(L, name);
	const char *ret = lua_tostring(L, -1);
	lua_pop(L, 1);
	return ret;
}
int CFFEntitySystem::GetInt( const char *name )
{
	lua_getglobal(L, name);
	int ret = (int)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return ret;
}
float CFFEntitySystem::GetFloat( const char *name )
{
	lua_getglobal(L, name);
	float ret = (float)lua_tonumber(L, -1);
	lua_pop(L, 1);
	return ret;
}

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
// Purpose: Runs the appropriate script function
//----------------------------------------------------------------------------
int CFFEntitySystem::RunPredicates( CBaseEntity *ent, CBaseEntity *player, const char *addname )
{
	// If there is no active script then allow the ents to always go
	if( !m_ScriptExists || !L )
		return true;

	int player_id = player ? ENTINDEX( player ) : -1;
	int ent_id = ent ? ENTINDEX( ent ) : -1;

	SetVar("entid", ent_id);
	SetVar("dispensedammo", false);

	// FryGuy: removed the classname from the function -- mappers shouldn't use conflicting names
	// because it makes things confusing, and also typing out the class name for each func is bad.
	//strcpy( func, ent->GetClassname() );
	//strcat( func, "_" );
	if (ent)
	{
		if (!addname || !strlen(addname))
		{
			Warning("Can't call entsys.runpredicates with an entity and no addname\n");
			return false;
		}

		if (!strlen(STRING(ent->GetEntityName())))
			return false;

		SetVar("entname", STRING(ent->GetEntityName()));

		// push the function onto stack ( entname:addname )
		lua_getglobal( L, STRING(ent->GetEntityName()) );
		if (lua_isnil(L, -1))
		{
			Warning("Table '%s' doesn't exist!\n", STRING(ent->GetEntityName()) );
			lua_pop(L, 1);
			return false;
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
		DevWarning( "[SCRIPT] Error calling %s\n", addname );
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
