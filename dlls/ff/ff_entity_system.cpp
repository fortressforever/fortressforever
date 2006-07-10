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
#include "ff_gamerules.h"
#include "ff_grenade_base.h"

#define temp_max(a,b) (((a)>(b))?(a):(b))

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/iterator_policy.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Better way of doing this maybe?
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

	void RespawnAllPlayers()
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
			strcpy(realscript, "maps/includes/" );
			strcat(realscript, script);
			strcat(realscript, ".lua");

			CFFEntitySystem::LoadLuaFile( entsys.GetLuaState(), realscript );
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
		CBaseEntity *pEntity = UTIL_EntityByIndex( item_id );
		return ( pEntity == NULL ) ? NULL : pEntity;
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

	CFFPlayer* GetPlayer(int player_id)
	{
		CBaseEntity* pEnt = GetEntity(player_id);

		if(NULL == pEnt)
			return NULL;

		if(!pEnt->IsPlayer())
			return NULL;

		return dynamic_cast<CFFPlayer*>(pEnt);
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

	bool IsPlayerFromId( int player_id )
	{
		return GetPlayer( player_id ) == NULL ? false : true;
	}

	bool IsGrenadeFromId( int ent_id )
	{
		return IsGrenade( GetEntity( ent_id ) );
	}

	bool IsDispenserFromId( int ent_id )
	{
		return IsDispenser( GetEntity( ent_id ) );
	}

	bool IsSentrygunFromId( int ent_id )
	{
		return IsSentrygun( GetEntity( ent_id ) );
	}

	bool IsDetpackFromId( int ent_id )
	{
		return IsDetpack( GetEntity( ent_id ) );
	}

	CFFInfoScript* GetInfoScriptByName(const char* entityName)
	{
		CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( NULL, "info_ff_script" );

		while( pEnt != NULL )
		{
			if ( FStrEq( STRING(pEnt->GetEntityName()), entityName ) )
				return pEnt;

			// Next!
			pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( pEnt, "info_ff_script" );
		}

		return NULL;
	}

	CFFInfoScript* GetInfoScriptById(int item_id)
	{
		CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( NULL, "info_ff_script" );

		while( pEnt != NULL )
		{
			if ( pEnt->entindex() == item_id )
				return pEnt;

			// Next!
			pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( pEnt, "info_ff_script" );
		}

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
		Warning( "[CastToGrenade]\n" );

		if( !pEntity )
			return NULL;

		return dynamic_cast< CFFGrenadeBase * >( pEntity );
	}

	CFFInfoScript* CastToItemFlag(CBaseEntity* pEntity)
	{
		if( !pEntity )
			return NULL;

		return dynamic_cast< CFFInfoScript * >( pEntity );
	}

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

	void SmartMessage(int playerId, const char* playerMsg, const char* teamMsg, const char* otherMsg)
	{
		CFFPlayer* pPlayer = GetPlayer(playerId);
		if(NULL == pPlayer)
			return;

		int nPlayers = FF_NumPlayers();
		for(int i = 1 ; i <= nPlayers ; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(i);

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

	void SmartSound(int playerId, const char* playerSound, const char* teamSound, const char* otherSound)
	{
		CFFPlayer* pPlayer = GetPlayer(playerId);
		if(NULL == pPlayer)
			return;

		int nPlayers = FF_NumPlayers();
		for(int i = 1 ; i <= nPlayers ; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(i);

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

	void SmartTeamMessage(int teamId, const char* teamMsg, const char* otherMsg)
	{
		// get team
		CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		int nPlayers = FF_NumPlayers();
		for(int i = 1 ; i <= nPlayers ; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(i);

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == teamId)
				SendPlayerMessage(pPlayer, teamMsg);
			else
				SendPlayerMessage(pPlayer, otherMsg);
		}
	}

	void SmartTeamSound(int teamId, const char* teamSound, const char* otherSound)
	{
		// get team
		CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate sound to each player
		int nPlayers = FF_NumPlayers();
		for(int i = 1 ; i <= nPlayers ; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(i);

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == teamId)
				SendPlayerSound(pPlayer, teamSound);
			else
				SendPlayerSound(pPlayer, otherSound);
		}
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

} // namespace FFLib

void CFFEntitySystem::FFLibOpen()
{
	// these functions are exposed through luabind too
	lua_register( L, "GetPlayerTeam", GetPlayerTeam );
	lua_register( L, "GetPlayerClass", GetPlayerClass );
	lua_register( L, "AddTeamScore", AddTeamScore );
	lua_register( L, "NumPlayersOnTeam", NumPlayersOnTeam );
	lua_register( L, "AddArmor", AddArmor );
	lua_register( L, "AddFrags", AddFrags );
	lua_register( L, "IsPlayer", IsPlayer );
	lua_register( L, "IsDispenser", IsDispenser );
	lua_register( L, "IsSentrygun", IsSentrygun );
	lua_register( L, "IsDetpack", IsDetpack );
	lua_register( L, "GetObjectsTeam", GetObjectsTeam );
	lua_register( L, "IsGrenade", IsGrenade );
	lua_register( L, "BroadCastMessage", BroadCastMessage );
	lua_register( L, "BroadCastMessageToPlayer", BroadCastMessageToPlayer );
	lua_register( L, "BroadCastSound", BroadCastSound );
	lua_register( L, "BroadCastSoundToPlayer", BroadCastSoundToPlayer );
	lua_register( L, "NumPlayers", NumPlayers );
	lua_register( L, "SetTeamName", SetTeamName );
	lua_register( L, "GetPlayerName", GetPlayerName );
	lua_register( L, "SetPlayerLocation", SetPlayerLocation );
	lua_register( L, "RemoveLocation", RemoveLocation );
	lua_register( L, "SetPlayerDisguisable", SetPlayerDisguisable );
	lua_register( L, "SetPlayerRespawnDelay", SetPlayerRespawnDelay );
	lua_register( L, "SetGlobalRespawnDelay", SetGlobalRespawnDelay );
	lua_register( L, "RespawnPlayer", RespawnPlayer );
	lua_register( L, "RespawnAllPlayers", RespawnAllPlayers );
	lua_register( L, "KillAndRespawnAllPlayers", KillAndRespawnAllPlayers );
	lua_register( L, "RemoveItem", RemoveItem );
	lua_register( L, "SetTeamAllies", SetTeamAllies );
	lua_register( L, "PrecacheSound", PrecacheSound );
	lua_register( L, "Respawn", Respawn );
	lua_register( L, "ReturnItem", ReturnItem );
	lua_register( L, "Pickup", Pickup );
	lua_register( L, "DropItem", DropItem );
	lua_register( L, "ConsoleToAll", ConsoleToAll );
	lua_register( L, "PlayerHasItem", PlayerHasItem );
	lua_register( L, "SetModel", SetModel);
	lua_register( L, "PrecacheModel", PrecacheModel );
	lua_register( L, "IncludeScript", IncludeScript );
	lua_register( L, "SetTeamClassLimit", SetTeamClassLimit );
	lua_register( L, "SetTeamPlayerLimit", SetTeamPlayerLimit );
	lua_register( L, "MarkRadioTag", MarkRadioTag );
	lua_register( L, "IsPlayerInNoBuild", IsPlayerInNoBuild );
	lua_register( L, "IsPlayerUnderWater", IsPlayerUnderWater );
	lua_register( L, "IsPlayerWaistDeepInWater", IsPlayerWaistDeepInWater );
	lua_register( L, "IsPlayerFeetDeepInWater", IsPlayerFeetDeepInWater );
	lua_register( L, "AddAmmo", GiveAmmo );
	lua_register( L, "AddHealth", AddHealth );
	lua_register( L, "RemoveAmmo", RemoveAmmo );
	lua_register( L, "GetPlayer", GetPlayer );
	lua_register( L, "IsTeam1AlliedToTeam2", IsTeam1AlliedToTeam2 );
	lua_register( L, "GetPlayerOnTeam", GetPlayerOnTeam );
	lua_register( L, "EmitSound", EmitSound );
	lua_register( L, "Random", Random );
	lua_register( L, "rand", Random );
	lua_register( L, "GetServerTime", GetServerTime );
	lua_register( L, "IsFeigned", IsFeigned );
	lua_register( L, "IsDisguised", IsDisguised );
	lua_register( L, "GetDisguisedClass", GetDisguisedClass );
	lua_register( L, "GetDisguisedTeam", GetDisguisedTeam );
	lua_register( L, "IsOnFire", IsOnFire );
	lua_register( L, "IsDucking", IsDucking );
	lua_register( L, "IsOnGround", IsOnGround );
	lua_register( L, "IsInAir", IsInAir );

	// these funcions are NOT exposed to luabind yet
	lua_register( L, "SpawnEntityAtPlayer", SpawnEntityAtPlayer );			// not used
	lua_register( L, "UseEntity", UseEntity );								// base.lua
	lua_register( L, "IsGrenInNoGren", IsGrenInNoGren );					// not used
	lua_register( L, "IsObjectsOriginInWater", IsObjectsOriginInWater );	// not used
	lua_register( L, "IsObjectsOriginInSlime", IsObjectsOriginInSlime );	// not used
	

	module(L)
	[
		class_<Vector>("Vector")
			.def_readwrite("x",			&Vector::x)
			.def_readwrite("y",			&Vector::y)
			.def_readwrite("z",			&Vector::z)
			.def("DistTo",				&Vector::DistTo)
			.def("DistToSq",			&Vector::DistToSqr)
			.def("Dot",					&Vector::Dot)
			.def("Length",				&Vector::Length)
			.def("Normalize",			&Vector::NormalizeInPlace),

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

		// CBaseEntity
		class_<CBaseEntity>("BaseEntity")
			.def("EmitSound",			&CBaseEntity::PlaySound)
			.def("GetName",				&CBaseEntity::GetName)
			.def("GetTeam",				&CBaseEntity::GetTeam)
			.def("GetTeamId",			&CBaseEntity::GetTeamNumber)
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
			.def("IsOnFire",			&CBaseEntity::IsOnFire),
	
		// CTeam
		class_<CTeam>("BaseTeam")
			.def("AddScore",			&CTeam::AddScore)
			.def("GetNumPlayers",		&CTeam::GetNumPlayers)
			.def("GetPlayer",			&CTeam::GetPlayer)
			.def("GetTeamId",			&CTeam::GetTeamNumber)
			.def("SetName",				&CTeam::SetName),

		// CFFTeam
		class_<CFFTeam, CTeam>("Team")
			.def("SetAllies",			&CFFTeam::SetAllies)
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
		class_<CFFGrenadeBase>("Grenade")
		.def("Type",					&CFFGrenadeBase::GetGrenId)
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

		// TODO: Should we add classes for all gren types?
		// Normal grenade,
		// Caltrop,
		// Nail grenade,
		// Mirv grenade,
		// Mirvlet,
		// Conc grenade,
		// Napalm grenade,
		// Gas grenade,
		// Emp grenade,

		// CBasePlayer
		class_<CBasePlayer, CBaseEntity>("BasePlayer"),

		// CFFPlayer
		class_<CFFPlayer, CBasePlayer>("Player")
			.def("AddAmmo",				&CFFPlayer::AddAmmo)
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
			.def("IsOnGround",			&CFFPlayer::IsOnGround)
			.def("IsInAir",				&CFFPlayer::IsInAir)
			.def("IsDucking",			&CFFPlayer::IsDucking)
			.def("MarkRadioTag",		&CFFPlayer::SetRadioTagged)
			.def("RemoveAmmo",			(void(CFFPlayer::*)(int, const char*))&CFFPlayer::RemoveAmmo)
			.def("RemoveArmor",			&CFFPlayer::RemoveArmor)
			.def("RemoveLocation",		&CFFPlayer::RemoveLocation)
			.def("Respawn",				&CFFPlayer::KillAndRemoveItems)
			.def("SetDisguisable",		&CFFPlayer::SetDisguisable)
			.def("SetLocation",			&CFFPlayer::SetLocation)
			.def("SetRespawnDelay",		&CFFPlayer::LUA_SetPlayerRespawnDelay)
			.def("InstaSwitch",			&CFFPlayer::InstaSwitch)
			.def("GiveWeapon",			&CFFPlayer::GiveNamedItem)
			.def("RemoveWeapon",		&CFFPlayer::TakeNamedItem)
			.def("RemoveAllWeapons",	&CFFPlayer::RemoveAllItems)
			.def("GetOrigin",			&CFFPlayer::GetAbsOrigin)
			.def("IsFeigned",			&CFFPlayer::IsFeigned)
			.def("IsDisguised",			&CFFPlayer::IsDisguised)
			.def("GetDisguisedClass",	&CFFPlayer::GetDisguisedClass)
			.def("GetDisguisedTeam",	&CFFPlayer::GetDisguisedTeam)
			.def("AddSpeedEffect",		&CFFPlayer::AddLuaSpeedEffect)	// currently support for one lua speed effect only
			.def("IsSpeedEffectSet",	&CFFPlayer::IsLuaSpeedEffectSet)
			.def("RemoveSpeedEffect",	&CFFPlayer::RemoveLuaSpeedEffect)
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
			.def("SetModel",			&CFFInfoScript::LUA_SetModel)	// already supported in BaseEntity
			.def("SetSkin",				&CFFInfoScript::LUA_SetSkin)	// already supported in BaseEntity
			.def("GetOrigin",			&CBaseEntity::GetAbsOrigin), // already supported in BaseEntity..
																	// do I need it here? -- Nope


		// global functions
		namespace_("ffmod")	// temp namespace so names dont collide with regular lua_register
		[
			def("BroadCastMessage",			&FFLib::BroadcastMessage),
			def("BroadCastMessageToPlayer",	&FFLib::SendPlayerMessage),
			def("BroadCastSound",			&FFLib::BroadcastSound),
			def("BroadCastSoundToPlayer",	&FFLib::SendPlayerSound),
			def("CastToPlayer",				&FFLib::CastToPlayer),
			def("CastToInfoScript",			&FFLib::CastToItemFlag),
			def("CastToGrenade",			&FFLib::CastToGrenade),
			def("GetEntity",				&FFLib::GetEntity),
			def("GetEntityByName",			&FFLib::GetEntityByName),
			def("GetEntitiesByName",		&FFLib::GetEntitiesByName,			return_stl_iterator),
			def("GetEntitiesInSphere",		&FFLib::GetEntitiesInSphere,		return_stl_iterator),
			def("GetInfoScriptById",		&FFLib::GetInfoScriptById),
			def("GetInfoScriptByName",		&FFLib::GetInfoScriptByName),
			def("GetPlayer",				&FFLib::GetPlayer),
			def("GetTeam",					&FFLib::GetTeam),
			def("GetGrenade",				&FFLib::GetGrenade),
			def("IsPlayer",					&FFLib::IsPlayerFromId),
			def("IsDispenser",				&FFLib::IsDispenserFromId),
			def("IsSentrygun",				&FFLib::IsSentrygunFromId),
			def("IsDetpack",				&FFLib::IsDetpackFromId),
			def("IsGrenade",				&FFLib::IsGrenadeFromId),
			def("AreTeamsAllied",			(bool(*)(CTeam*, CTeam*))&FFLib::AreTeamsAllied),
			def("AreTeamsAllied",			(bool(*)(int, int))&FFLib::AreTeamsAllied),
			def("IncludeScript",			&FFLib::IncludeScript),
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
			def("GetServerTime",			&FFLib::GetServerTime)
		]
	];
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
			CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( NULL, "info_ff_script" );

			while( pEnt != NULL )
			{
				// Tell the ent that it died
				if ( pEnt->GetOwnerEntity() == pPlayer && FStrEq( STRING(pEnt->GetEntityName()), itemname ) )
				{
					DevMsg("[SCRIPT] found item %d: %s\n", ENTINDEX(pEnt), itemname);
					ret = true;
				}

				// Next!
				pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( pEnt, "info_ff_script" );
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
		CFFInfoScript *pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( NULL, "info_ff_script" );

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
			pEnt = (CFFInfoScript*)gEntList.FindEntityByClassname( pEnt, "info_ff_script" );
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
			CFFInfoScript *pItem = (CFFInfoScript*)item;

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
			CFFInfoScript *pItem = (CFFInfoScript*)item;

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
			CFFInfoScript *pItem = (CFFInfoScript*)item;

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
		bool ret = false;
		const char *soundname = lua_tostring( L, 1 );

		if( helper )
		{
			DevMsg("[entsys] Precaching sound %s\n", soundname);
			helper->PrecacheScriptSound(soundname);
			ret = true;
		}
		else
			Warning( "[entsys] \"helper\" not initialized!\n" );

		// 1 result
		lua_pushboolean( L, ret );
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
				pPlayer->RemoveItems();
				pPlayer->Spawn();
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
// Purpose: KillAndRespawn all players (i.e. for round end in epicentre/hunted)
//          int RespawnAllPlayers(  );
//----------------------------------------------------------------------------
int CFFEntitySystem::KillAndRespawnAllPlayers( lua_State *L )
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
// Purpose: Sets the name of a team (ie. "attackers")
//          int SetTeamName( team, name )
//----------------------------------------------------------------------------
int CFFEntitySystem::SetTeamName( lua_State *L )
{
	int n = lua_gettop( L );

	// A 2 argument'd function
	if( n == 2 )
	{
		int iTeam = lua_tonumber( L, 1 );
		const char *pszName = lua_tostring( L, 2 );

		CFFTeam *pTeam = ( CFFTeam * )GetGlobalTeam( iTeam );

		if( pTeam )
		{
			pTeam->SetName( pszName );
			DevMsg( "Set team name for team %i to \"%s\"\n", iTeam, pszName );
		}
		else
		{
			Warning( "Unable to set the team name for team %i to \"%s\"\n", iTeam, pszName );
		}

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

			pPlayer->SetRadioTagged( NULL, gpGlobals->curtime, duration );
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
//          int SetPlayerLocation( int iPlayer, int iEntIndex, string szName, int iTeam);
//----------------------------------------------------------------------------
int CFFEntitySystem::SetPlayerLocation( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 4 )
	{
		bool bRet = false;
		int iPlayer = (int)lua_tonumber( L, 1 );
		int iEntIndex = ( int )lua_tonumber( L, 2 );
		const char *szName = lua_tostring( L, 3 );
		int iTeam = ( int )lua_tonumber( L, 4 ); // added

		CBasePlayer *pEnt = UTIL_PlayerByIndex( iPlayer );
		if (pEnt && pEnt->IsPlayer())
		{
			CFFPlayer *pPlayer = ToFFPlayer(pEnt);

			if(pPlayer)
			{
				pPlayer->SetLocation(iEntIndex, szName, iTeam);
				bRet = true;
			}
		}

		// 1 result
		lua_pushboolean( L, bRet );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Remove location ID from players locations.
//          int SetPlayerLocation( int iPlayer, int iEntIndex );
//----------------------------------------------------------------------------
int CFFEntitySystem::RemoveLocation( lua_State *L )
{
	int n = lua_gettop(L);

	if( n == 2 )
	{
		bool ret = false;
		int iPlayer = (int)lua_tonumber( L, 1 );
		int iEntIndex = ( int )lua_tonumber( L, 2 );

		CBasePlayer *pEnt = UTIL_PlayerByIndex( iPlayer );
		if (pEnt && pEnt->IsPlayer())
		{
			ToFFPlayer( pEnt )->RemoveLocation( iEntIndex );
			ret = true;
		}

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

		mp_respawndelay.SetValue( temp_max( 0.0f, delay ) );

		// 1 result
		lua_pushboolean( L, ret );
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if an entity is a player
//			int IsPlayer( ent_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsPlayer( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iEntIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iEntIndex );
		if( pEntity && pEntity->IsPlayer() )
			bRetVal = true;

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if a player is feigned
//			int IsFeigned( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsFeigned( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iIndex = lua_tonumber( L, 1 );

		CBasePlayer *pEntity = UTIL_PlayerByIndex( iIndex );
		if( pEntity && pEntity->IsPlayer() )
			bRetVal = ToFFPlayer( pEntity )->IsFeigned();

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if a player is disguised
//			int IsDisguised( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsDisguised( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iIndex = lua_tonumber( L, 1 );

		CBasePlayer *pEntity = UTIL_PlayerByIndex( iIndex );
		if( pEntity && pEntity->IsPlayer() )
			bRetVal = ToFFPlayer( pEntity )->IsDisguised();

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Get a player's disguised class
//			int GetDisguisedClass( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::GetDisguisedClass( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		int iIndex = lua_tonumber( L, 1 );
		int iClass = 0;

		CBasePlayer *pEntity = UTIL_PlayerByIndex( iIndex );
		if( pEntity && pEntity->IsPlayer() )
		{
			if( ToFFPlayer( pEntity )->IsDisguised() )
				iClass = ToFFPlayer( pEntity )->GetDisguisedClass();
		}

		lua_pushnumber( L, iClass );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Get a player's disguised team
//			int GetDisguisedTeam( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::GetDisguisedTeam( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		int iIndex = lua_tonumber( L, 1 );
		int iTeam = 0;

		CBasePlayer *pEntity = UTIL_PlayerByIndex( iIndex );
		if( pEntity && pEntity->IsPlayer() )
		{
			if( ToFFPlayer( pEntity )->IsDisguised() )
				iTeam = ToFFPlayer( pEntity )->GetDisguisedTeam();
		}

		lua_pushnumber( L, iTeam );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if an object is on fire
//			int IsOnFire( ent_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsOnFire( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iIndex );
		if( pEntity )
			bRetVal = pEntity->IsOnFire();

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if a player is ducking
//			int IsDucking( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsDucking( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iIndex = lua_tonumber( L, 1 );

		CBasePlayer *pEntity = UTIL_PlayerByIndex( iIndex );
		if( pEntity && pEntity->IsPlayer() )
			bRetVal = ToFFPlayer( pEntity )->IsDucking();

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if a player is on the ground
//			int IsOnGround( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsOnGround( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iIndex = lua_tonumber( L, 1 );

		CBasePlayer *pEntity = UTIL_PlayerByIndex( iIndex );
		if( pEntity && pEntity->IsPlayer() )
			bRetVal = ToFFPlayer( pEntity )->IsOnGround();

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if a player is in the air
//			int IsInAir( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsInAir( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iIndex = lua_tonumber( L, 1 );

		CBasePlayer *pEntity = UTIL_PlayerByIndex( iIndex );
		if( pEntity && pEntity->IsPlayer() )
			bRetVal = !ToFFPlayer( pEntity )->IsOnGround();

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if an entity is a dispenser
//			int IsDispenser( ent_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsDispenser( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iEntIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iEntIndex );
		if( pEntity && ( pEntity->Classify() == CLASS_DISPENSER ) )
			bRetVal = true;

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if an entity is a dispenser
//			int IsSentrygun( ent_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsSentrygun( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iEntIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iEntIndex );
		if( pEntity && ( pEntity->Classify() == CLASS_SENTRYGUN ) )
			bRetVal = true;

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if an entity is a detpack
//			int IsDetpack( ent_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsDetpack( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iEntIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iEntIndex );
		if( pEntity && ( pEntity->Classify() == CLASS_DETPACK ) )
			bRetVal = true;

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Get an objects team, will work for:
//				Dispenser, Sentrygun, Detpack, MiniTurret, Player, Grenades
//			int GetObjectsTeam( ent_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::GetObjectsTeam( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		int iEntIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iEntIndex );
		if( pEntity )
		{
			if( ( pEntity->IsPlayer() ) ||
				( pEntity->Classify() == CLASS_DISPENSER ) ||
				( pEntity->Classify() == CLASS_SENTRYGUN ) ||
				( pEntity->Classify() == CLASS_DETPACK ) ||
				( pEntity->Classify() == CLASS_TURRET ) ||
				( pEntity->Classify() == CLASS_GREN ) ||
				( pEntity->Classify() == CLASS_GREN_EMP ) ||
				( pEntity->Classify() == CLASS_GREN_NAIL ) ||
				( pEntity->Classify() == CLASS_GREN_MIRV ) ||
				( pEntity->Classify() == CLASS_GREN_MIRVLET ) ||
				( pEntity->Classify() == CLASS_GREN_NAPALM ) ||
				( pEntity->Classify() == CLASS_GREN_GAS ) ||
				( pEntity->Classify() == CLASS_GREN_CONC ) ||
				( pEntity->Classify() == CLASS_GREN_CALTROP ) )
			{
				lua_pushnumber( L, pEntity->GetTeamNumber() );
			}
			else
			{
				lua_pushnumber( L, 0 );
			}

			// 1 result
			return 1;
		}		
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Sees if team 1 is allied to team 2
//			int IsTeam1AlliedToTeam2( int iTeam1, int iTeam2 )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsTeam1AlliedToTeam2( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 2 )
	{		
		int iTeam1 = lua_tonumber( L, 1 );
		int iTeam2 = lua_tonumber( L, 2 );

		if( ( iTeam1 >= TEAM_BLUE ) && ( iTeam1 <= TEAM_GREEN ) &&
			( iTeam2 >= TEAM_BLUE ) && ( iTeam2 <= TEAM_GREEN ) )
		{
			bool bRetVal = false;

			if( FFGameRules()->IsTeam1AlliedToTeam2( iTeam1, iTeam2 ) == GR_TEAMMATE )
				bRetVal = true;

			lua_pushboolean( L, bRetVal );

			// 1 result
			return 1;
		}
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if a player is in a no build area
//			int IsPlayerInNoBuild( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsPlayerInNoBuild( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iPlayerIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iPlayerIndex );
		if( pEntity && pEntity->IsPlayer() )
		{
			bRetVal = !FFScriptRunPredicates( pEntity, "onbuild", true );
		}

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if a player is completely under water
//			int IsPlayerUnderWater( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsPlayerUnderWater( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iEntIndex = lua_tonumber( L, 1 );

		CBasePlayer *pPlayer = UTIL_PlayerByIndex( iEntIndex );
		if( pPlayer && pPlayer->IsPlayer() )
		{
			bRetVal = ToFFPlayer( pPlayer )->GetWaterLevel() == WL_Eyes;
		}

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if a player is waist deep in water
//			int IsPlayerWaistDeepInWater( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsPlayerWaistDeepInWater( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iEntIndex = lua_tonumber( L, 1 );

		CBasePlayer *pPlayer = UTIL_PlayerByIndex( iEntIndex );
		if( pPlayer && pPlayer->IsPlayer() )
		{
			bRetVal = ToFFPlayer( pPlayer )->GetWaterLevel() == WL_Waist;
		}

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if a player is feet deep in water
//			int IsPlayerFeetDeepInWater( player_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsPlayerFeetDeepInWater( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iEntIndex = lua_tonumber( L, 1 );

		CBasePlayer *pPlayer = UTIL_PlayerByIndex( iEntIndex );
		if( pPlayer && pPlayer->IsPlayer() )
		{
			bRetVal = ToFFPlayer( pPlayer )->GetWaterLevel() == WL_Feet;
		}

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Is a grenade in a no grenade area
//			int IsGrenInNoGren( ent_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsGrenInNoGren( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iGrenIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iGrenIndex );
		if( pEntity && 
			( ( pEntity->Classify() == CLASS_GREN ) ||
			( pEntity->Classify() == CLASS_GREN_EMP ) ||
			( pEntity->Classify() == CLASS_GREN_NAIL ) ||
			( pEntity->Classify() == CLASS_GREN_MIRV ) ||
			( pEntity->Classify() == CLASS_GREN_MIRVLET ) ||
			( pEntity->Classify() == CLASS_GREN_NAPALM ) ||
			( pEntity->Classify() == CLASS_GREN_GAS ) ||
			( pEntity->Classify() == CLASS_GREN_CONC ) ||
			( pEntity->Classify() == CLASS_GREN_CALTROP ) ) )
		{
			bRetVal = !FFScriptRunPredicates( pEntity, "onexplode", true );
		}

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if an entity is a grenade
//			int IsGrenade( ent_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsGrenade( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iGrenIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iGrenIndex );
		if( pEntity && 
			( ( pEntity->Classify() == CLASS_GREN ) ||
			( pEntity->Classify() == CLASS_GREN_EMP ) ||
			( pEntity->Classify() == CLASS_GREN_NAIL ) ||
			( pEntity->Classify() == CLASS_GREN_MIRV ) ||
			( pEntity->Classify() == CLASS_GREN_MIRVLET ) ||
			( pEntity->Classify() == CLASS_GREN_NAPALM ) ||
			( pEntity->Classify() == CLASS_GREN_GAS ) ||
			( pEntity->Classify() == CLASS_GREN_CONC ) ||
			( pEntity->Classify() == CLASS_GREN_CALTROP ) ) )
			bRetVal = true;

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if an entity's origin is in water
//			int IsObjectsOriginInWater( ent_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsObjectsOriginInWater( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iEntIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iEntIndex );
		if( pEntity && ( UTIL_PointContents( pEntity->GetAbsOrigin() ) & CONTENTS_WATER ) )
			bRetVal = true;

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: See if an entity's origin is in slime
//			int IsObjectsOriginInSlime( ent_id )
//----------------------------------------------------------------------------
int CFFEntitySystem::IsObjectsOriginInSlime( lua_State *L )
{
	int n = lua_gettop( L );

	if( n == 1 )
	{
		bool bRetVal = false;
		int iEntIndex = lua_tonumber( L, 1 );

		CBaseEntity *pEntity = UTIL_EntityByIndex( iEntIndex );
		if( pEntity && ( UTIL_PointContents( pEntity->GetAbsOrigin() ) & CONTENTS_SLIME ) )
			bRetVal = true;

		lua_pushboolean( L, bRetVal );

		// 1 result
		return 1;
	}

	// No results
	return 0;
}

//----------------------------------------------------------------------------
// Purpose: Get's the server time
//          float GetServerTime( void )
//----------------------------------------------------------------------------
int CFFEntitySystem::GetServerTime( lua_State *L )
{
	lua_pushnumber( L, gpGlobals->curtime );

	// 1 result
	return 1;
}

//----------------------------------------------------------------------------
// Purpose: Random
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

bool FFScriptRunPredicates( CBaseEntity *pObject, const char *pszFunction, bool bExpectedVal )
{
	if( pObject && pszFunction )
	{
		for( int i = 0; i < pObject->m_hActiveScripts.Count(); i++ )
		{
			CBaseEntity *pEntity = UTIL_EntityByIndex( pObject->m_hActiveScripts[ i ] );
			if( pEntity )
			{
				bool bEntSys = entsys.RunPredicates( pEntity, pObject, pszFunction ) > 0;

				if( bEntSys != bExpectedVal )
					return !bExpectedVal;
			}
		}
	}

	return bExpectedVal;
}
