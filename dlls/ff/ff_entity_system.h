/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_entity_system.h
/// @author Gavin Bramhill (Mirvin_Monkey)
/// @date 21 April 2005
/// @brief Handles the entity system
///
/// REVISIONS
/// ---------
/// Apr 21, 2005 Mirv: Begun


#include "cbase.h"

extern ConVar mp_respawndelay;

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}


//============================================================================
// CFFEntitySystemHelper
//============================================================================
class CFFEntitySystemHelper : public CBaseEntity
{
public:
	DECLARE_CLASS( CFFEntitySystemHelper, CBaseEntity );
	DECLARE_DATADESC();

	void Spawn( void );
	void OnThink( void );
	void Precache( void );
};


//============================================================================
// CFFEntitySystem
//============================================================================
class CFFEntitySystem
{
private:
	lua_State* L;
	bool m_ScriptExists;

public:
	CFFEntitySystem();
	~CFFEntitySystem();

	static bool LoadLuaFile( lua_State*, const char *);
	bool StartForMap();
	void FFLibOpen();

	static void SetVar( lua_State *L, const char *name, const char *value );
	static void SetVar( lua_State *L, const char *name, int value );
	static void SetVar( lua_State *L, const char *name, float value );
	void SetVar( const char *name, const char *value );
	void SetVar( const char *name, int value );
	void SetVar( const char *name, float value );
	static int HandleError( lua_State* );
	void DoString( const char *buffer );

	const char *GetString( const char *name );
	int GetInt( const char *name );
	float GetFloat( const char *name );
	int RunPredicates( CBaseEntity*, CBaseEntity*, const char * = NULL);

	static int ConsoleToAll( lua_State* );
	static int GetPlayerTeam( lua_State* );
	static int GetPlayerClass( lua_State * );
	static int GetPlayerName( lua_State * );
	static int AddTeamScore( lua_State* );
	static int SpawnEntityAtPlayer( lua_State * );
	static int PlayerHasItem( lua_State * );
	static int RemoveItem( lua_State * );
	static int ReturnItem( lua_State * );
	static int Pickup( lua_State * );
	static int Respawn( lua_State * );
	static int DropItem( lua_State * );
	static int SetModel( lua_State * );
	static int UseEntity( lua_State * );
	static int PrecacheModel( lua_State * );
	static int EmitSound( lua_State * );
	static int PrecacheSound( lua_State * );
	static int BroadCastSound( lua_State* );
	static int BroadCastSoundToPlayer( lua_State* );
	static int BroadCastMessage( lua_State* );
	static int BroadCastMessageToPlayer( lua_State* );
	static int RespawnAllPlayers( lua_State* );
	static int RespawnPlayer( lua_State* );
	static int NumPlayersOnTeam( lua_State* );
	static int GetPlayerOnTeam( lua_State* );
	static int NumPlayers( lua_State* );
	static int GetPlayer( lua_State* );
	static int IncludeScript( lua_State* );
	static int SetTeamClassLimit( lua_State* );
	static int SetTeamPlayerLimit( lua_State* );
	static int Random( lua_State* );
	static int SetTeamAllies( lua_State* );
	static int GiveAmmo( lua_State* );
	static int RemoveAmmo( lua_State* );
	static int AddArmor( lua_State* );
	static int AddHealth( lua_State* );
	static int AddFrags( lua_State* );
	static int MarkRadioTag( lua_State* );
	static int SetPlayerLocation( lua_State* );
	static int RemoveLocation( lua_State* );
	static int SetPlayerDisguisable( lua_State* );
	static int SetPlayerRespawnDelay( lua_State* );
	static int SetGlobalRespawnDelay( lua_State* );
	static int IsPlayer( lua_State* );
	static int IsDispenser( lua_State* );
	static int IsSentrygun( lua_State* );
	static int GetObjectsTeam( lua_State* );
	static int IsTeam1AlliedToTeam2( lua_State* );

};

extern CFFEntitySystem entsys;