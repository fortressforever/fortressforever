
// ff_lualib_globals.cpp

//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_entity_system.h"
#include "ff_gamerules.h"
#include "ff_goal.h"
#include "ff_grenade_base.h"
#include "ff_item_flag.h"
#include "ff_player.h"
#include "ff_utils.h"

#include "beam_shared.h"
#include "buttons.h"
#include "doors.h"
#include "recipientfilter.h"
#include "triggers.h"

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
#include "luabind/iterator_policy.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//----------------------------------------------------------------------------
// defines
#define temp_max(a,b) (((a)>(b))?(a):(b))

//---------------------------------------------------------------------------
using namespace luabind;

//---------------------------------------------------------------------------
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

//---------------------------------------------------------------------------
// FFLib Namespace
//---------------------------------------------------------------------------
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
	
	// These don't work - use the "Collection" stuff instead
	/*
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
	*/

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

	void AddHudIcon( CFFPlayer *pPlayer, const char *pszImage, const char *pszIdentifier, int x, int y )
	{
		if( !pPlayer || !pszImage || !pszIdentifier )
			return;

		FF_LuaHudIcon( pPlayer, pszIdentifier, x, y, pszImage );
	}

	void AddHudText( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszText, int x, int y )
	{
		if( !pPlayer || !pszIdentifier || !pszText )
			return;

		FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText );
	}

	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, int iStartValue, float flSpeed, int x, int y )
	{
		if( !pszIdentifier || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, iStartValue, flSpeed );
	}

	void RemoveHudItem( CFFPlayer *pPlayer, const char *pszIdentifier )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudRemove( pPlayer, pszIdentifier );
	}

} // namespace FFLib

//---------------------------------------------------------------------------
// FFLib Namespace
//---------------------------------------------------------------------------
void CFFLuaLib::InitGlobals(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
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
		//def("GetEntitiesByName",		&FFLib::GetEntitiesByName,			return_stl_iterator),
		//def("GetEntitiesInSphere",		&FFLib::GetEntitiesInSphere,		return_stl_iterator),
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
		def("AddHudIcon",				&FFLib::AddHudIcon),
		def("AddHudText",				&FFLib::AddHudText),
		def("AddHudTimer",				&FFLib::AddHudTimer),
		def("RemoveHudItem",			&FFLib::RemoveHudItem)
	];
}
