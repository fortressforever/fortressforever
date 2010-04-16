
// ff_lualib_globals.cpp

//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_scriptman.h"
#include "ff_entity_system.h"
#include "ff_gamerules.h"
#include "ff_goal.h"
#include "ff_grenade_base.h"
#include "ff_item_flag.h"
#include "ff_triggerclip.h"
#include "ff_player.h"
#include "ff_utils.h"

#include "beam_shared.h"
#include "buttons.h"
#include "doors.h"
#include "recipientfilter.h"
#include "triggers.h"
#include "filesystem.h"
#include "sharedInterface.h"

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

#define MAX_MENU_LEN 240

#include "luabind/luabind.hpp"
#include "luabind/iterator_policy.hpp"

#include "ff_scheduleman.h"
#include "ff_timerman.h"
#include "ff_menuman.h"

#include "omnibot_interface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern bool g_Disable_Timelimit;

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
				IsOfClass(pEntity, CLASS_GREN_CONC)
				*/
	}

	// is the entity a miniturret
	bool IsTurret( CBaseEntity *pEntity )
	{
		return IsOfClass( pEntity, CLASS_TURRET );
	}

	void ChatToAll(const char *szMessage)
	{
		bool bChat = true;
		CBroadcastRecipientFilter filter;
		UserMessageBegin( filter, "SayText" );
			WRITE_BYTE( 0 ); // world, dedicated server says
			WRITE_STRING( szMessage );
			WRITE_BYTE( bChat );
		MessageEnd();
	}

	void ChatToPlayer(CFFPlayer *pPlayer, const char *szMessage)
	{
		if (!pPlayer)
			return;

		bool bChat = true;
		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin( filter, "SayText" );
			WRITE_BYTE( 0 ); // world, dedicated server says
			WRITE_STRING( szMessage );
			WRITE_BYTE( bChat );
		MessageEnd();
	}

	void BroadcastMessage(const char* szMessage)
	{
		CBroadcastRecipientFilter filter;
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE);
			WRITE_STRING(szMessage);
		MessageEnd();

		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("broadcast_msg: %s", szMessage),"broadcast_msg");
	}

	void BroadcastMessage(const char* szMessage, float fDuration)
	{
		CBroadcastRecipientFilter filter;
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_DURATION);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
		MessageEnd();

		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("broadcast_msg: %s", szMessage),"broadcast_msg");
	}

	void BroadcastMessage(const char* szMessage, float fDuration, int iColorID)
	{
		CBroadcastRecipientFilter filter;
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_COLOR);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
			WRITE_SHORT(iColorID);
		MessageEnd();

		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("broadcast_msg: %s", szMessage),"broadcast_msg");
	}
	

	void BroadcastMessage(const char* szMessage, float fDuration, const char* szColor )
	{
		int r, g, b;
		if(sscanf( szColor, "%i %i %i", &r, &g, &b ) != 3)
			r=g=b=255;
		
		r = clamp(r,0,255);
		g = clamp(g,0,255);
		b = clamp(b,0,255);

		CBroadcastRecipientFilter filter;
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_COLOR_CUSTOM);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
			WRITE_SHORT(r);
			WRITE_SHORT(g);
			WRITE_SHORT(b);
		MessageEnd();

		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("broadcast_msg: %s", szMessage),"broadcast_msg");
	}

	void SendPlayerMessage(CFFPlayer* pPlayer, const char* szMessage)
	{
		if(NULL == pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE);
			WRITE_STRING(szMessage);
		MessageEnd();
	}

	void SendPlayerMessage(CFFPlayer* pPlayer, const char* szMessage, float fDuration)
	{
		if(NULL == pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_DURATION);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
		MessageEnd();
	}

	void SendPlayerMessage(CFFPlayer* pPlayer, const char* szMessage, float fDuration, int iColorID)
	{
		if(NULL == pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_COLOR);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
			WRITE_SHORT(iColorID);
		MessageEnd();
	}

	void SendPlayerMessage(CFFPlayer* pPlayer, const char* szMessage, float fDuration, const char* szColor)
	{
		if(NULL == pPlayer)
			return;
		
		int r, g, b;
		if(sscanf( szColor, "%i %i %i", &r, &g, &b ) != 3)
			r=g=b=255;
		
		r = clamp(r,0,255);
		g = clamp(g,0,255);
		b = clamp(b,0,255);

		CSingleUserRecipientFilter filter(pPlayer);
		UserMessageBegin(filter, "GameMessage");
			WRITE_BYTE(HUD_MESSAGE_COLOR_CUSTOM);
			WRITE_STRING(szMessage);
			WRITE_FLOAT(fDuration);
			WRITE_SHORT(r);
			WRITE_SHORT(g);
			WRITE_SHORT(b);
		MessageEnd();
	}

	void BroadcastSound(const char* szSound)
	{
		CBroadcastRecipientFilter filter;

		CFFEntitySystemHelper* pHelperInst = CFFEntitySystemHelper::GetInstance();
		if(pHelperInst)
			pHelperInst->EmitSound( filter, pHelperInst->entindex(), szSound);

		
		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("broadcast_snd: %s", szSound),"broadcast_snd");
	}

	void SendPlayerSound(CFFPlayer* pPlayer, const char* szSound)
	{
		if(NULL == pPlayer)
			return;

		CSingleUserRecipientFilter filter(pPlayer);

		CFFEntitySystemHelper* pHelperInst = CFFEntitySystemHelper::GetInstance();
		if(pHelperInst)
			pHelperInst->EmitSound( filter, pHelperInst->entindex(), szSound);
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

	void DisableTimeLimit(bool _b = true)
	{
		g_Disable_Timelimit = _b;
	}

	bool HasGameStarted()
	{
		if (FFGameRules())
			return FFGameRules()->HasGameStarted();
		else
			return true;
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

/*
	void RespawnAllPlayers( void )
	{		
		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
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
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				pPlayer->KillAndRemoveItems();
			}
		}
	}
*/

	void ConsoleToAll(const char* szMessage)
	{
		DevMsg( "Debug: " );
		DevMsg( szMessage );
		DevMsg( "\n" );

		Msg( szMessage );
		Msg( "\n" );
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

			//////////////////////////////////////////////////////////////////////////
			// Try a precache, rumor has it this will cause the engine to send the lua files to clients
			if(PRECACHE_LUA_FILES)
			{
				V_FixSlashes(realscript);
				if(filesystem->FileExists(realscript))
				{
					Util_AddDownload(realscript);

					if(!engine->IsGenericPrecached(realscript))
						engine->PrecacheGeneric(realscript, true);
				}
			}
			//////////////////////////////////////////////////////////////////////////

			if( !CFFScriptManager::LoadFile( _scriptman.GetLuaState(), realscript ) )
			{
				// Try looking in the maps directory
				Q_snprintf( realscript, sizeof( realscript ), "maps/%s.lua", script );
				CFFScriptManager::LoadFile( _scriptman.GetLuaState(), realscript );
			}
		}
		else
		{
			Msg("[SCRIPT] Warning: Invalid filename: %s\n", script);
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
	int GetTeamNumber(CFFTeam* team)
	{
		return team->m_iTeamNum;
	}
	CBaseEntity* GetEntity(int item_id)
	{
		return UTIL_EntityByIndex(item_id);
	}

	CBaseEntity* GetEntityByName(const char* szName)
	{
		return gEntList.FindEntityByName(NULL, szName, NULL);
	}

	CFFPlayer* GetPlayer(CBaseEntity *pEntity)
	{
		// ToFFPlayer checks for NULL & IsPlayer()
		// so this is safe to do.
		return ToFFPlayer( pEntity );
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
	CFuncFFScript* GetTriggerScriptById(int item_id)
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex( item_id );
		if( pEntity && ( pEntity->Classify() == CLASS_TRIGGERSCRIPT ) )
			return dynamic_cast< CFuncFFScript * >( pEntity );
		
		return NULL;
	}
	CFFPlayer* CastToPlayer( CBaseEntity* pEntity )
	{
		if( !pEntity )
			return NULL;

		if( !IsPlayer( pEntity ) )
			return NULL;

		return dynamic_cast< CFFPlayer * >( pEntity );
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

	CFFTriggerClip *CastToTriggerClip( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return NULL;

		if( pEntity->Classify() != CLASS_TRIGGER_CLIP )
			return NULL;

		return dynamic_cast< CFFTriggerClip * >( pEntity );
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

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
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
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, int iPlayerMsgColor, int iTeamMsgColor, int iOtherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, iPlayerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, iTeamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, iOtherMsgColor );
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, const char* playerMsgColor, const char* teamMsgColor, const char* otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, int playerMsgColor, const char* teamMsgColor, const char* otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, int playerMsgColor, int teamMsgColor, const char* otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, int playerMsgColor, const char* teamMsgColor, int otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, const char* playerMsgColor, int teamMsgColor, const char* otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}
	
	
	void SmartMessage(CBaseEntity *pEntity, const char* playerMsg, const char* teamMsg, const char* otherMsg, const char* playerMsgColor, int teamMsgColor, int otherMsgColor)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SendPlayerMessage(pTestPlayer, playerMsg, 0.0f, playerMsgColor );

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SendPlayerMessage(pTestPlayer, teamMsg, 0.0f, teamMsgColor );

			else
				SendPlayerMessage(pTestPlayer, otherMsg, 0.0f, otherMsgColor );
		}
	}

	void SmartSound(CBaseEntity *pEntity, const char* playerSound, const char* teamSound, const char* otherSound)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
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

	void SmartSpeak(CBaseEntity *pEntity, const char* playerSentence, const char* teamSentence, const char* otherSentence)
	{
		CFFPlayer* pPlayer = GetPlayer(pEntity);
		if(NULL == pPlayer)
			return;

		int iPlayerSentence = engine->SentenceIndexFromName(playerSentence);
		int iTeamSentence = engine->SentenceIndexFromName(teamSentence);
		int iOtherSentence = engine->SentenceIndexFromName(otherSentence);

		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pTestPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pTestPlayer )
				continue;

			if(pTestPlayer->entindex() == pPlayer->entindex())
				SENTENCEG_PlaySentenceIndex(pTestPlayer->edict(), iPlayerSentence, 1.0f, SNDLVL_TALKING, 0, 100);

			else if(pTestPlayer->GetTeamNumber() == pPlayer->GetTeamNumber())
				SENTENCEG_PlaySentenceIndex(pTestPlayer->edict(), iTeamSentence, 1.0f, SNDLVL_TALKING, 0, 100);

			else
				SENTENCEG_PlaySentenceIndex(pTestPlayer->edict(), iOtherSentence, 1.0f, SNDLVL_TALKING, 0, 100);
		}
	}

	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
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

	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg, int iTeamMsgColor, int iOtherMsgColor)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerMessage(pPlayer, teamMsg, 0.0f, iTeamMsgColor);
			else
				SendPlayerMessage(pPlayer, otherMsg, 0.0f, iOtherMsgColor);
		}
	}

	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg, const char* teamMsgColor, const char* otherMsgColor)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerMessage(pPlayer, teamMsg, 0.0f, teamMsgColor);
			else
				SendPlayerMessage(pPlayer, otherMsg, 0.0f, otherMsgColor);
		}
	}
	
	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg, int teamMsgColor, const char* otherMsgColor)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerMessage(pPlayer, teamMsg, 0.0f, teamMsgColor);
			else
				SendPlayerMessage(pPlayer, otherMsg, 0.0f, otherMsgColor);
		}
	}
	
	void SmartTeamMessage(CFFTeam *pTeam, const char* teamMsg, const char* otherMsg, const char* teamMsgColor, int otherMsgColor)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate message to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SendPlayerMessage(pPlayer, teamMsg, 0.0f, teamMsgColor);
			else
				SendPlayerMessage(pPlayer, otherMsg, 0.0f, otherMsgColor);
		}
	}

	void SmartTeamSpeak(CFFTeam *pTeam, const char* teamSentence, const char* otherSentence)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		int iTeamSentence = engine->SentenceIndexFromName(teamSentence);
		int iOtherSentence = engine->SentenceIndexFromName(otherSentence);

		// set the appropriate sound to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber())
				SENTENCEG_PlaySentenceIndex(pPlayer->edict(), iTeamSentence, 1.0f, SNDLVL_TALKING, 0, 100);
			else
				SENTENCEG_PlaySentenceIndex(pPlayer->edict(), iOtherSentence, 1.0f, SNDLVL_TALKING, 0, 100);
		}
	}

	void SmartTeamSound(CFFTeam *pTeam, const char* teamSound, const char* otherSound)
	{
		// get team
		//CFFTeam* pTeam = GetGlobalFFTeam(teamId);

		if(NULL == pTeam)
			return;

		// set the appropriate sound to each player
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
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

	// For generically handling Lua events (like flag caps, drops, or whatever)
	void LogLuaEvent( 
		int luaid,							// entity index of the Lua object (flag, ball, etc)
		int objid,							// entity index of some CBaseEntity* that is doing something to the lua object
		const char* name = "Generic_Lua_Event" )	// name for the event - like flag_cap, flag_drop, etc - something stats programs could use
	{
		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "luaevent" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", luaid );
			pEvent->SetInt( "userid2", objid );
			pEvent->SetString( "eventname", name );
			gameeventmanager->FireEvent( pEvent, true );
		}
	}

			// For generically handling Lua events (like flag caps, drops, or whatever)
	void LogLuaEvent( 
		int luaid,							// entity index of the Lua object (flag, ball, etc)
		int objid,							// entity index of some CBaseEntity* that is doing something to the lua object
		const char* name,	// name for the event - like flag_cap, flag_drop, etc - something stats programs could use
		const char* field0,				// "Extra" fields
		const char* field1)				// "Extra" fields

	{
		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "luaevent" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", luaid );
			pEvent->SetInt( "userid2", objid );
			pEvent->SetString( "eventname", name );
			pEvent->SetString( "key0", field0 );
			pEvent->SetString( "value0", field1 );

			gameeventmanager->FireEvent( pEvent, true );
		}
	}

		// For generically handling Lua events (like flag caps, drops, or whatever)
	void LogLuaEvent( 
		int luaid,							// entity index of the Lua object (flag, ball, etc)
		int objid,							// entity index of some CBaseEntity* that is doing something to the lua object
		const char* name = "Generic_Lua_Event",	// name for the event - like flag_cap, flag_drop, etc - something stats programs could use
		const char* field0 = NULL,				// "Extra" fields
		const char* field1 = NULL,				// "Extra" fields
		const char* field2 = NULL,				// "Extra" fields
		const char* field3 = NULL )				// "Extra" fields

	{
		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "luaevent" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", luaid );
			pEvent->SetInt( "userid2", objid );
			pEvent->SetString( "eventname", name );
			pEvent->SetString( "key0", field0 );
			pEvent->SetString( "value0", field1 );
			pEvent->SetString( "key1", field2 );
			pEvent->SetString( "value1", field3 );

			gameeventmanager->FireEvent( pEvent, true );
		}
	}

	// For generically handling Lua events (like flag caps, drops, or whatever)
	void LogLuaEvent( 
		int luaid,							// entity index of the Lua object (flag, ball, etc)
		int objid,							// entity index of some CBaseEntity* that is doing something to the lua object
		const char* name = "Generic_Lua_Event",	// name for the event - like flag_cap, flag_drop, etc - something stats programs could use
		const char* field0 = NULL,				// "Extra" fields
		const char* field1 = NULL,				// "Extra" fields
		const char* field2 = NULL,				// "Extra" fields
		const char* field3 = NULL,				// "Extra" fields
		const char* field4 = NULL,				// "Extra" fields
		const char* field5 = NULL )

	{
		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "luaevent" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", luaid );
			pEvent->SetInt( "userid2", objid );
			pEvent->SetString( "eventname", name );
			pEvent->SetString( "key0", field0 );
			pEvent->SetString( "value0", field1 );
			pEvent->SetString( "key1", field2 );
			pEvent->SetString( "value1", field3 );
			pEvent->SetString( "key2", field4 );
			pEvent->SetString( "value2", field5 );

			gameeventmanager->FireEvent( pEvent, true );
		}
	}
	
	// For adding an objective-related death notice
	void ObjectiveNotice( CFFPlayer *player, const char *text )
	{
		if (!player)
			return;

		// Fire an event.
		IGameEvent *pEvent = gameeventmanager->CreateEvent( "objective_event" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", player->GetUserID() );
			//pEvent->SetString( "eventname", name );
			pEvent->SetString( "eventtext", text );

			gameeventmanager->FireEvent( pEvent );
		}
	}

	// Displays a string in the hint box
	void DisplayMessage( CFFPlayer *pPlayer, const char* message )
	{
		if ( pPlayer )
			FF_SendHint( pPlayer, MAP_HINT, -1, PRIORITY_HIGH, message );
	}

	// Updates which entity the HUD objective icon is attached to
	// Unfortunately, it seems certain entities (like trigger_scripts) do not network properly (they have invalid indexes)
	// So, we also have to network the entity's position as a fallback
	void UpdateObjectiveIcon( CFFPlayer *pPlayer, CBaseEntity *pEntity )
	{
		if ( pPlayer )
		{
			pPlayer->SetObjectiveEntity( pEntity );
		}
	}

	// Updates the position of the Objective Icon (the entity it's attached to) for a whole team
	void UpdateTeamObjectiveIcon( CFFTeam *pTeam, CBaseEntity *pEntity )
	{
		if( !pTeam )
			return;

		// set the objective entity for each player on the team
		for( int i = 1 ; i <= gpGlobals->maxClients; i++ )
		{
			CFFPlayer* pPlayer = GetPlayer( UTIL_EntityByIndex(i) );

			if( !pPlayer )
				continue;

			if( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
			{
				pPlayer->SetObjectiveEntity( pEntity );
			}
		}
	}


	CFFPlayer *GetPlayerByID( int player_id )
	{
		CBaseEntity *pEntity = UTIL_EntityByIndex( player_id );
		return GetPlayer( pEntity );
	}

	CFFPlayer *GetPlayerByName( const char *_name )
	{
		CBaseEntity *pEntity = UTIL_PlayerByName( _name );
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

	float GetConvar( const char *pszConvarName )
	{
		if( !pszConvarName )
			return -1.0f;

		if( !cvar )
			return -1.0f;

		ConVar *pConvar = cvar->FindVar( pszConvarName );
		if( !pConvar )
			return -1.0f;

		if( pConvar->IsBitSet( FCVAR_CHEAT ) )
			return -1.0f;

		return pConvar->GetFloat();
	}

	void SetConvar( const char *pszConvarName, float flValue )
	{
		if( !pszConvarName )
			return;

		// Don't allow sv_cheats setting
		if( !Q_stricmp( pszConvarName, "sv_cheats" ) )
			return;

		if( !cvar )
			return;

		ConVar *pConvar = cvar->FindVar( pszConvarName );
		if( !pConvar )
			return;

		if( pConvar->IsBitSet( FCVAR_CHEAT ) )
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

		Omnibot::Notify_FireOutput(szTargetEntityName, szTargetInputName);
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

		Omnibot::Notify_FireOutput(szTargetEntityName, szTargetInputName);
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

		Omnibot::Notify_FireOutput(szTargetEntityName, szTargetInputName);
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

		Omnibot::Notify_FireOutput(szTargetEntityName, szTargetInputName);
	}

	void AddHudIcon( CFFPlayer *pPlayer, const char *pszImage, const char *pszIdentifier, int x, int y )
	{
		if( !pPlayer || !pszImage || !pszIdentifier )
			return;

		FF_LuaHudIcon( pPlayer, pszIdentifier, x, y, pszImage, 0, 0 );
	}

	// default alignment
	void AddHudIcon( CFFPlayer *pPlayer, const char *pszImage, const char *pszIdentifier, int x, int y, int iWidth, int iHeight )
	{
		if( !pPlayer || !pszImage || !pszIdentifier )
			return;

		FF_LuaHudIcon( pPlayer, pszIdentifier, x, y, pszImage, iWidth, iHeight );
	}

	void AddHudIcon( CFFPlayer *pPlayer, const char *pszImage, const char *pszIdentifier, int x, int y, int iWidth, int iHeight, int iAlign )
	{
		if( !pPlayer || !pszImage || !pszIdentifier || ( iWidth < 0 ) || ( iHeight < 0 ) )
			return;

		FF_LuaHudIcon( pPlayer, pszIdentifier, x, y, pszImage, iWidth, iHeight, iAlign );
	}

	// added y alignment
	void AddHudIcon( CFFPlayer *pPlayer, const char *pszImage, const char *pszIdentifier, int x, int y, int iWidth, int iHeight, int iAlignX, int iAlignY )
	{
		if( !pPlayer || !pszImage || !pszIdentifier || ( iWidth < 0 ) || ( iHeight < 0 ) )
			return;

		FF_LuaHudIcon( pPlayer, pszIdentifier, x, y, pszImage, iWidth, iHeight, iAlignX, iAlignY );
	}

	void AddHudIconToTeam( CFFTeam *pTeam, const char *pszImage, const char *pszIdentifier, int x, int y )
	{
		if( !pszImage || !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, 0, 0);
			}
		}
	}

	// default alignment
	void AddHudIconToTeam( CFFTeam *pTeam, const char *pszImage, const char *pszIdentifier, int x, int y, int iWidth, int iHeight )
	{
		if( !pszImage || !pszIdentifier || ( iWidth < 0 ) || ( iHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, iWidth, iHeight);
			}
		}
	}

	void AddHudIconToTeam( CFFTeam *pTeam, const char *pszImage, const char *pszIdentifier, int x, int y, int iWidth, int iHeight, int iAlign )
	{
		if( !pszImage || !pszIdentifier || ( iWidth < 0 ) || ( iHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, iWidth, iHeight, iAlign);
			}
		}
	}

	// added y alignment
	void AddHudIconToTeam( CFFTeam *pTeam, const char *pszImage, const char *pszIdentifier, int x, int y, int iWidth, int iHeight, int iAlignX, int iAlignY )
	{
		if( !pszImage || !pszIdentifier || ( iWidth < 0 ) || ( iHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, iWidth, iHeight, iAlignX, iAlignY);
			}
		}
	}

	void AddHudIconToAll( const char *pszImage, const char *pszIdentifier, int x, int y )
	{
		if( !pszImage || !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, 0, 0);
			}
		}
	}

	// default alignment
	void AddHudIconToAll( const char *pszImage, const char *pszIdentifier, int x, int y, int iWidth, int iHeight )
	{
		if( !pszImage || !pszIdentifier || ( iWidth < 0 ) || ( iHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, iWidth, iHeight);
			}
		}
	}

	void AddHudIconToAll( const char *pszImage, const char *pszIdentifier, int x, int y, int iWidth, int iHeight, int iAlign )
	{
		if( !pszImage || !pszIdentifier || ( iWidth < 0 ) || ( iHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, iWidth, iHeight, iAlign);
			}
		}
	}

	// added y alignment
	void AddHudIconToAll( const char *pszImage, const char *pszIdentifier, int x, int y, int iWidth, int iHeight, int iAlignX, int iAlignY )
	{
		if( !pszImage || !pszIdentifier || ( iWidth < 0 ) || ( iHeight < 0 ) )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudIcon(pPlayer, pszIdentifier, x, y, pszImage, iWidth, iHeight, iAlignX, iAlignY);
			}
		}
	}

	void AddHudText( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszText, int x, int y )
	{
		if( !pPlayer || !pszIdentifier || !pszText )
			return;

		FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText );
	}

	void AddHudText( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszText, int x, int y, int iAlign )
	{
		if( !pPlayer || !pszIdentifier || !pszText )
			return;

		FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, iAlign );
	}

	void AddHudText( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszText, int x, int y, int iAlignX, int iAlignY )
	{
		if( !pPlayer || !pszIdentifier || !pszText )
			return;

		FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, iAlignX, iAlignY );
	}

	void AddHudTextToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszText, int x, int y )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText );
			}
		}
	}

	void AddHudTextToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszText, int x, int y, int iAlign )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, iAlign );
			}
		}
	}

	void AddHudTextToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszText, int x, int y, int iAlignX, int iAlignY )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, iAlignX, iAlignY );
			}
		}
	}

	void AddHudTextToAll( const char *pszIdentifier, const char *pszText, int x, int y )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText );
			}
		}
	}

	void AddHudTextToAll( const char *pszIdentifier, const char *pszText, int x, int y, int iAlign )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, iAlign );
			}
		}
	}

	void AddHudTextToAll( const char *pszIdentifier, const char *pszText, int x, int y, int iAlignX, int iAlignY )
	{
		if( !pszIdentifier || !pszText )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudText( pPlayer, pszIdentifier, x, y, pszText, iAlignX, iAlignY );
			}
		}
	}
	
	void AddTimer( const char *pszIdentifier, int iStartValue, float flSpeed )
	{
		if( !pszIdentifier )
			return;

		_timerman.AddTimer( pszIdentifier, iStartValue, flSpeed );
	}
	
	void RemoveTimer( const char *pszIdentifier )
	{
		if( !pszIdentifier )
			return;

		_timerman.RemoveTimer( pszIdentifier );
	}

	float GetTimerTime( const char *pszIdentifier )
	{
		if( !pszIdentifier )
			return 0;

		return _timerman.GetTime( pszIdentifier );
	}
	
	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszTimerIdentifier, int x, int y )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ) );
	}
	
	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszTimerIdentifier, int x, int y, int iAlign )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), iAlign );
	}
	
	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, const char *pszTimerIdentifier, int x, int y, int iAlignX, int iAlignY )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), iAlignX, iAlignY );
	}

	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, int iStartValue, float flSpeed, int x, int y )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, iStartValue, flSpeed );
	}

	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, int iStartValue, float flSpeed, int x, int y, int iAlign )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, iStartValue, flSpeed, iAlign );
	}

	void AddHudTimer( CFFPlayer *pPlayer, const char *pszIdentifier, int iStartValue, float flSpeed, int x, int y, int iAlignX, int iAlignY )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, iStartValue, flSpeed, iAlignX, iAlignY );
	}
	
	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszTimerIdentifier, int x, int y )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ) );
			}
		}
	}
	
	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszTimerIdentifier, int x, int y, int iAlign )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), iAlign );
			}
		}
	}
	
	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, const char *pszTimerIdentifier, int x, int y, int iAlignX, int iAlignY )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), iAlignX, iAlignY );
			}
		}
	}

	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, int iStartValue, float flSpeed, int x, int y )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, iStartValue, flSpeed );
			}
		}
	}

	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, int iStartValue, float flSpeed, int x, int y, int iAlign )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, iStartValue, flSpeed, iAlign );
			}
		}
	}

	void AddHudTimerToTeam( CFFTeam *pTeam, const char *pszIdentifier, int iStartValue, float flSpeed, int x, int y, int iAlignX, int iAlignY )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, iStartValue, flSpeed, iAlignX, iAlignY );
			}
		}
	}
	
	void AddHudTimerToAll( const char *pszIdentifier, const char *pszTimerIdentifier, int x, int y )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ) );
			}
		}
	}
	
	void AddHudTimerToAll( const char *pszIdentifier, const char *pszTimerIdentifier, int x, int y, int iAlign )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), iAlign );
			}
		}
	}
	
	void AddHudTimerToAll( const char *pszIdentifier, const char *pszTimerIdentifier, int x, int y, int iAlignX, int iAlignY )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, _timerman.GetTime( pszTimerIdentifier ), _timerman.GetIncrement( pszTimerIdentifier ), iAlignX, iAlignY );
			}
		}
	}

	void AddHudTimerToAll( const char *pszIdentifier, int iStartValue, float flSpeed, int x, int y )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, iStartValue, flSpeed );
			}
		}
	}

	void AddHudTimerToAll( const char *pszIdentifier, int iStartValue, float flSpeed, int x, int y, int iAlign )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, iStartValue, flSpeed, iAlign );
			}
		}
	}

	void AddHudTimerToAll( const char *pszIdentifier, int iStartValue, float flSpeed, int x, int y, int iAlignX, int iAlignY )
	{
		if( !pszIdentifier )
			return;

		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudTimer( pPlayer, pszIdentifier, x, y, iStartValue, flSpeed, iAlignX, iAlignY );
			}
		}
	}

	void RemoveHudItem( CFFPlayer *pPlayer, const char *pszIdentifier )
	{
		if( !pPlayer || !pszIdentifier )
			return;

		FF_LuaHudRemove( pPlayer, pszIdentifier );
	}

	void RemoveHudItemFromTeam( CFFTeam *pTeam, const char *pszIdentifier )
	{
		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				if ( pPlayer->GetTeam()->GetTeamNumber() == pTeam->GetTeamNumber() )
					FF_LuaHudRemove( pPlayer, pszIdentifier );
			}
		}
	}

	void RemoveHudItemFromAll( const char *pszIdentifier )
	{
		// loop through each player
		for (int i=1; i<=gpGlobals->maxClients; i++)
		{
			CBasePlayer *ent = UTIL_PlayerByIndex( i );
			if (ent && ent->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer( ent );
				FF_LuaHudRemove( pPlayer, pszIdentifier );
			}
		}
	}
	
	void AddSchedule(const char* szScheduleName, float time, const luabind::adl::object& fn)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn);
	}

	void AddSchedule(const char* szScheduleName,
					 float time,
					 const luabind::adl::object& fn,
					 const luabind::adl::object& param)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, 0, param);
	}

	void AddSchedule(const char* szScheduleName,
					 float time,
					 const luabind::adl::object& fn,
					 const luabind::adl::object& param1,
					 const luabind::adl::object& param2)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, 0, param1, param2);
	}

	void AddSchedule(const char* szScheduleName,
					float time,
					const luabind::adl::object& fn,
					const luabind::adl::object& param1,
					const luabind::adl::object& param2,
					const luabind::adl::object& param3)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, 0, param1, param2, param3);
	}

	void AddSchedule(const char* szScheduleName,
					float time,
					const luabind::adl::object& fn,
					const luabind::adl::object& param1,
					const luabind::adl::object& param2,
					const luabind::adl::object& param3,
					const luabind::adl::object& param4)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, 0, param1, param2, param3, param4);
	}

	void AddScheduleRepeating(const char* szScheduleName, float time, const luabind::adl::object& fn)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, -1);
	}

	void AddScheduleRepeating(const char* szScheduleName,
							  float time,
							  const luabind::adl::object& fn,
							  const luabind::adl::object& param)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, -1, param);
	}

	void AddScheduleRepeating(const char* szScheduleName,
							  float time,
							  const luabind::adl::object& fn,
							  const luabind::adl::object& param1,
							  const luabind::adl::object& param2)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, -1, param1, param2);
	}

	void AddScheduleRepeating(const char* szScheduleName,
							float time,
							const luabind::adl::object& fn,
							const luabind::adl::object& param1,
							const luabind::adl::object& param2,
							const luabind::adl::object& param3)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, -1, param1, param2, param3);
	}

	void AddScheduleRepeating(const char* szScheduleName,
							float time,
							const luabind::adl::object& fn,
							const luabind::adl::object& param1,
							const luabind::adl::object& param2,
							const luabind::adl::object& param3,
							const luabind::adl::object& param4)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, -1, param1, param2, param3, param4);
	}

	void AddScheduleRepeatingNotInfinitely(const char* szScheduleName, float time, const luabind::adl::object& fn, int nRepeat)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, nRepeat);
	}

	void AddScheduleRepeatingNotInfinitely(const char* szScheduleName,
											float time,
											const luabind::adl::object& fn,
											int nRepeat,
											const luabind::adl::object& param)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, nRepeat, param);
	}

	void AddScheduleRepeatingNotInfinitely(const char* szScheduleName,
											float time,
											const luabind::adl::object& fn,
											int nRepeat,
											const luabind::adl::object& param1,
											const luabind::adl::object& param2)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, nRepeat, param1, param2);
	}

	void AddScheduleRepeatingNotInfinitely(const char* szScheduleName,
											float time,
											const luabind::adl::object& fn,
											int nRepeat,
											const luabind::adl::object& param1,
											const luabind::adl::object& param2,
											const luabind::adl::object& param3)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, nRepeat, param1, param2, param3);
	}

	void AddScheduleRepeatingNotInfinitely(const char* szScheduleName,
											float time,
											const luabind::adl::object& fn,
											int nRepeat,
											const luabind::adl::object& param1,
											const luabind::adl::object& param2,
											const luabind::adl::object& param3,
											const luabind::adl::object& param4)
	{
		_scheduleman.AddSchedule(szScheduleName, time, fn, nRepeat, param1, param2, param3, param4);
	}

	void DeleteSchedule(const char* szScheduleName)
	{
		_scheduleman.RemoveSchedule(szScheduleName);
	}

	void RemoveSchedule(const char* szScheduleName)
	{
		_scheduleman.RemoveSchedule(szScheduleName);
	}

	void SpeakAll(const char* szSentenceName)
	{
		// lookup the id of the sentence
		if(!szSentenceName)
			return;

		int iSentence = engine->SentenceIndexFromName(szSentenceName);

		// send the sentence on each client
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			SENTENCEG_PlaySentenceIndex(pPlayer->edict(), iSentence, 1.0f, SNDLVL_TALKING, 0, 100);
		}

		Omnibot::omnibot_interface::Trigger(NULL,NULL,UTIL_VarArgs("speak: %s", szSentenceName),"speak_all");
	}

	void SpeakPlayer(CFFPlayer *pPlayer, const char* szSentenceName)
	{
		// lookup the id of the sentence
		if(!pPlayer || !szSentenceName)
			return;

		int iSentence = engine->SentenceIndexFromName(szSentenceName);

		// send the sentence to the client
		SENTENCEG_PlaySentenceIndex(pPlayer->edict(), iSentence, 1.0f, SNDLVL_TALKING, 0, 100);
	}

	void SpeakTeam(int iTeam, const char* szSentenceName)
	{
		// lookup the id of the sentence
		if(!szSentenceName)
			return;

		int iSentence = engine->SentenceIndexFromName(szSentenceName);

		// send the sentence on each client
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			if(pPlayer->GetTeamNumber() == iTeam)
				SENTENCEG_PlaySentenceIndex(pPlayer->edict(), iSentence, 1.0f, SNDLVL_TALKING, 0, 100);
		}
	}

	void DropToFloor( CBaseEntity *pEntity )
	{
		if( !pEntity )
			return;

		UTIL_DropToFloor( pEntity, MASK_SOLID );
	}
	
	void SetGameDescription( const char *szGameDescription )
	{
		if ( g_pGameRules ) // this function may be called before the world has spawned, and the game rules initialized
			g_pGameRules->SetGameDescription( szGameDescription );
	}

	void ShowMenuToPlayer( CFFPlayer *pPlayer, const char *szMenuName )
	{
		if (!pPlayer)
			return;

		CSingleUserRecipientFilter filter( pPlayer );
		_menuman.DisplayLuaMenu( filter, szMenuName );
	}

	void ShowMenuToTeam( int iTeam, const char *szMenuName )
	{
		// send the sentence on each client
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;
			
			if(pPlayer->GetTeamNumber() == iTeam)
				ShowMenuToPlayer( pPlayer, szMenuName );
		}
	}

	void ShowMenu( const char *szMenuName )
	{
		// send the sentence on each client
		for(int i = 1 ; i <= gpGlobals->maxClients; i++)
		{
			CFFPlayer* pPlayer = GetPlayer(UTIL_EntityByIndex(i));

			if( !pPlayer )
				continue;

			ShowMenuToPlayer( pPlayer, szMenuName );
		}
	}
	
	void CreateMenu( const char *szMenuName )
	{
		if (!szMenuName[0])
			return;

		_menuman.AddLuaMenu( szMenuName );
	}
	
	void CreateMenu( const char *szMenuName, float flDisplayTime )
	{
		if (!szMenuName[0])
			return;

		_menuman.AddLuaMenu( szMenuName, flDisplayTime );
	}
	
	void CreateMenu( const char *szMenuName, const char *szMenuTitle )
	{
		if (!szMenuName[0])
			return;

		_menuman.AddLuaMenu( szMenuName, szMenuTitle );
	}
	
	void CreateMenu( const char *szMenuName, const char *szMenuTitle, float flDisplayTime )
	{
		if (!szMenuName[0])
			return;

		_menuman.AddLuaMenu( szMenuName, szMenuTitle, flDisplayTime );
	}
	
	void DestroyMenu( const char *szMenuName )
	{
		if (!szMenuName[0])
			return;

		_menuman.RemoveLuaMenu( szMenuName );
	}
	
	void SetMenuTitle( const char *szMenuName, const char *szMenuTitle )
	{
		if (!szMenuName[0])
			return;

		_menuman.SetLuaMenuTitle( szMenuName, szMenuTitle );
	}
	
	void AddMenuOption( const char *szMenuName, int iSlot, const char *szOptionText )
	{
		if (!szMenuName[0])
			return;

		_menuman.AddLuaMenuOption( szMenuName, iSlot, szOptionText );
	}

	void RemoveMenuOption( const char *szMenuName, int iSlot )
	{
		if (!szMenuName[0])
			return;

		_menuman.RemoveLuaMenuOption( szMenuName, iSlot );
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
		def("AddHudIcon",				(void(*)(CFFPlayer *, const char *, const char *, int, int))&FFLib::AddHudIcon),
		def("AddHudIcon",				(void(*)(CFFPlayer *, const char *, const char *, int, int, int, int))&FFLib::AddHudIcon),
		def("AddHudIcon",				(void(*)(CFFPlayer *, const char *, const char *, int, int, int, int, int))&FFLib::AddHudIcon),
		def("AddHudIcon",				(void(*)(CFFPlayer *, const char *, const char *, int, int, int, int, int, int))&FFLib::AddHudIcon),
		def("AddHudIconToTeam",			(void(*)(CFFTeam *, const char *, const char *, int, int))&FFLib::AddHudIconToTeam),
		def("AddHudIconToTeam",			(void(*)(CFFTeam *, const char *, const char *, int, int, int, int))&FFLib::AddHudIconToTeam),
		def("AddHudIconToTeam",			(void(*)(CFFTeam *, const char *, const char *, int, int, int, int, int))&FFLib::AddHudIconToTeam),
		def("AddHudIconToTeam",			(void(*)(CFFTeam *, const char *, const char *, int, int, int, int, int, int))&FFLib::AddHudIconToTeam),
		def("AddHudIconToAll",			(void(*)(const char *, const char *, int, int))&FFLib::AddHudIconToAll),
		def("AddHudIconToAll",			(void(*)(const char *, const char *, int, int, int, int))&FFLib::AddHudIconToAll),
		def("AddHudIconToAll",			(void(*)(const char *, const char *, int, int, int, int, int))&FFLib::AddHudIconToAll),
		def("AddHudIconToAll",			(void(*)(const char *, const char *, int, int, int, int, int, int))&FFLib::AddHudIconToAll),
		def("AddHudText",				(void(*)(CFFPlayer *, const char *, const char *, int, int))&FFLib::AddHudText),
		def("AddHudText",				(void(*)(CFFPlayer *, const char *, const char *, int, int, int))&FFLib::AddHudText),
		def("AddHudText",				(void(*)(CFFPlayer *, const char *, const char *, int, int, int, int))&FFLib::AddHudText),
		def("AddHudTextToTeam",			(void(*)(CFFTeam *, const char *, const char *, int, int))&FFLib::AddHudTextToTeam),
		def("AddHudTextToTeam",			(void(*)(CFFTeam *, const char *, const char *, int, int, int))&FFLib::AddHudTextToTeam),
		def("AddHudTextToTeam",			(void(*)(CFFTeam *, const char *, const char *, int, int, int, int))&FFLib::AddHudTextToTeam),
		def("AddHudTextToAll",			(void(*)(const char *, const char *, int, int))&FFLib::AddHudTextToAll),
		def("AddHudTextToAll",			(void(*)(const char *, const char *, int, int, int))&FFLib::AddHudTextToAll),
		def("AddHudTextToAll",			(void(*)(const char *, const char *, int, int, int, int))&FFLib::AddHudTextToAll),
		def("AddTimer",					&FFLib::AddTimer),
		def("RemoveTimer",				&FFLib::RemoveTimer),
		def("GetTimerTime",				&FFLib::GetTimerTime),
		def("AddHudTimer",				(void(*)(CFFPlayer *, const char *, const char *, int, int))&FFLib::AddHudTimer),
		def("AddHudTimer",				(void(*)(CFFPlayer *, const char *, const char *, int, int, int))&FFLib::AddHudTimer),
		def("AddHudTimer",				(void(*)(CFFPlayer *, const char *, const char *, int, int, int, int))&FFLib::AddHudTimer),
		def("AddHudTimer",				(void(*)(CFFPlayer *, const char *, int, float, int, int))&FFLib::AddHudTimer),
		def("AddHudTimer",				(void(*)(CFFPlayer *, const char *, int, float, int, int, int))&FFLib::AddHudTimer),
		def("AddHudTimer",				(void(*)(CFFPlayer *, const char *, int, float, int, int, int, int))&FFLib::AddHudTimer),
		def("AddHudTimerToTeam",		(void(*)(CFFTeam *, const char *, const char *, int, int))&FFLib::AddHudTimerToTeam),
		def("AddHudTimerToTeam",		(void(*)(CFFTeam *, const char *, const char *, int, int, int))&FFLib::AddHudTimerToTeam),
		def("AddHudTimerToTeam",		(void(*)(CFFTeam *, const char *, const char *, int, int, int, int))&FFLib::AddHudTimerToTeam),
		def("AddHudTimerToTeam",		(void(*)(CFFTeam *, const char *, int, float, int, int))&FFLib::AddHudTimerToTeam),
		def("AddHudTimerToTeam",		(void(*)(CFFTeam *, const char *, int, float, int, int, int))&FFLib::AddHudTimerToTeam),
		def("AddHudTimerToTeam",		(void(*)(CFFTeam *, const char *, int, float, int, int, int, int))&FFLib::AddHudTimerToTeam),
		def("AddHudTimerToAll",			(void(*)(const char *, const char *, int, int))&FFLib::AddHudTimerToAll),
		def("AddHudTimerToAll",			(void(*)(const char *, const char *, int, int, int))&FFLib::AddHudTimerToAll),
		def("AddHudTimerToAll",			(void(*)(const char *, const char *, int, int, int, int))&FFLib::AddHudTimerToAll),
		def("AddHudTimerToAll",			(void(*)(const char *, int, float, int, int))&FFLib::AddHudTimerToAll),
		def("AddHudTimerToAll",			(void(*)(const char *, int, float, int, int, int))&FFLib::AddHudTimerToAll),
		def("AddHudTimerToAll",			(void(*)(const char *, int, float, int, int, int, int))&FFLib::AddHudTimerToAll),
		def("AddSchedule",				(void(*)(const char*, float, const luabind::adl::object&))&FFLib::AddSchedule),
		def("AddSchedule",				(void(*)(const char*, float, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddSchedule),
		def("AddSchedule",				(void(*)(const char*, float, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddSchedule),
		def("AddSchedule",				(void(*)(const char*, float, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddSchedule),
		def("AddSchedule",				(void(*)(const char*, float, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddSchedule),
		def("AddScheduleRepeating",		(void(*)(const char*, float, const luabind::adl::object&))&FFLib::AddScheduleRepeating),
		def("AddScheduleRepeating",		(void(*)(const char*, float, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddScheduleRepeating),
		def("AddScheduleRepeating",		(void(*)(const char*, float, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddScheduleRepeating),
		def("AddScheduleRepeating",		(void(*)(const char*, float, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddScheduleRepeating),
		def("AddScheduleRepeating",		(void(*)(const char*, float, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddScheduleRepeating),
		def("AddScheduleRepeatingNotInfinitely",		(void(*)(const char*, float, const luabind::adl::object&, int))&FFLib::AddScheduleRepeatingNotInfinitely),
		def("AddScheduleRepeatingNotInfinitely",		(void(*)(const char*, float, const luabind::adl::object&, int, const luabind::adl::object&))&FFLib::AddScheduleRepeatingNotInfinitely),
		def("AddScheduleRepeatingNotInfinitely",		(void(*)(const char*, float, const luabind::adl::object&, int, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddScheduleRepeatingNotInfinitely),
		def("AddScheduleRepeatingNotInfinitely",		(void(*)(const char*, float, const luabind::adl::object&, int, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddScheduleRepeatingNotInfinitely),
		def("AddScheduleRepeatingNotInfinitely",		(void(*)(const char*, float, const luabind::adl::object&, int, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&, const luabind::adl::object&))&FFLib::AddScheduleRepeatingNotInfinitely),
		def("ApplyToAll",				&FFLib::ApplyToAll),
		def("ApplyToTeam",				&FFLib::ApplyToTeam),
		def("ApplyToPlayer",			&FFLib::ApplyToPlayer),
		def("AreTeamsAllied",			(bool(*)(CTeam*, CTeam*))&FFLib::AreTeamsAllied),
		def("AreTeamsAllied",			(bool(*)(int, int))&FFLib::AreTeamsAllied),
		def("ChatToAll",				&FFLib::ChatToAll),
		def("ChatToPlayer",				&FFLib::ChatToPlayer),
		def("BroadCastMessage",			(void(*)(const char*))&FFLib::BroadcastMessage),
		def("BroadCastMessage",			(void(*)(const char*, float))&FFLib::BroadcastMessage),
		def("BroadCastMessage",			(void(*)(const char*, float, int))&FFLib::BroadcastMessage),
		def("BroadCastMessage",			(void(*)(const char*, float, const char*))&FFLib::BroadcastMessage),
		def("BroadCastMessageToPlayer",	(void(*)(CFFPlayer*, const char*))&FFLib::SendPlayerMessage),
		def("BroadCastMessageToPlayer",	(void(*)(CFFPlayer*, const char*, float))&FFLib::SendPlayerMessage),
		def("BroadCastMessageToPlayer",	(void(*)(CFFPlayer*, const char*, float, int))&FFLib::SendPlayerMessage),
		def("BroadCastMessageToPlayer",	(void(*)(CFFPlayer*, const char*, float, const char*))&FFLib::SendPlayerMessage),
		def("BroadCastSound",			&FFLib::BroadcastSound),
		def("BroadCastSoundToPlayer",	&FFLib::SendPlayerSound),
		def("CastToBeam",				&FFLib::CastToBeam),
		def("CastToPlayer",				&FFLib::CastToPlayer),
		def("CastToInfoScript",			&FFLib::CastToItemFlag),
		def("CastToTriggerScript",		&FFLib::CastToTriggerScript),
		def("CastToTriggerClip",		&FFLib::CastToTriggerClip),
		def("CastToGrenade",			&FFLib::CastToGrenade),
		def("CastToDispenser",			&FFLib::CastToDispenser),
		def("CastToSentrygun",			&FFLib::CastToSentrygun),
		def("CastToDetpack",			&FFLib::CastToDetpack),
		def("ConsoleToAll",				&FFLib::ConsoleToAll),
		def("DeleteSchedule",			&FFLib::DeleteSchedule),
		def("DropToFloor",				&FFLib::DropToFloor),
		def("RemoveSchedule",			&FFLib::RemoveSchedule),
		def("GetConvar",				&FFLib::GetConvar),
		def("GetEntity",				&FFLib::GetEntity),
		def("GetEntityByName",			&FFLib::GetEntityByName),
		def("GetInfoScriptById",		&FFLib::GetInfoScriptById),
		def("GetInfoScriptByName",		&FFLib::GetInfoScriptByName),
		def("GetGrenade",				&FFLib::GetGrenade),
		def("GetPacketloss",			&FFLib::GetPacketloss),
		def("GetPing",					&FFLib::GetPing),
		def("GetPlayer",				&FFLib::GetPlayer),
		def("GetPlayerByID",			&FFLib::GetPlayerByID),	// TEMPORARY
		def("GetPlayerByName",			&FFLib::GetPlayerByName),		
		def("GetServerTime",			&FFLib::GetServerTime),
		def("GetSteamID",				&FFLib::GetSteamID),
		def("GetTeam",					&FFLib::GetTeam),
		def("GetTeamNumber",			&FFLib::GetTeamNumber),
		def("GetTriggerScriptById",		&FFLib::GetTriggerScriptById),
		def("GetTriggerScriptByName",	&FFLib::GetTriggerScriptByName),
		def("DisableTimeLimit",			(void(*)())&FFLib::DisableTimeLimit),
		def("DisableTimeLimit",			(void(*)(bool))&FFLib::DisableTimeLimit),
		def("HasGameStarted",			&FFLib::HasGameStarted),
		def("GoToIntermission",			&FFLib::GoToIntermission),
		def("IncludeScript",			&FFLib::IncludeScript),
		def("IsPlayer",					&FFLib::IsPlayer),
		def("IsDispenser",				&FFLib::IsDispenser),
		def("IsSentrygun",				&FFLib::IsSentrygun),
		def("IsDetpack",				&FFLib::IsDetpack),
		def("IsGrenade",				&FFLib::IsGrenade),
		def("IsTurret",					&FFLib::IsTurret),
		//def("KillAndRespawnAllPlayers",	&FFLib::KillAndRespawnAllPlayers),
		def("NumPlayers",				&FF_NumPlayers),
		def("OutputEvent",				(void(*)(const char*, const char*))&FFLib::FireOutput),
		def("OutputEvent",				(void(*)(const char*, const char*, const char*))&FFLib::FireOutput),
		def("OutputEvent",				(void(*)(const char*, const char*, const char*, float))&FFLib::FireOutput),
		def("OutputEvent",				(void(*)(const char*, const char*, const char*, float, unsigned int))&FFLib::FireOutput),
		def("PrecacheModel",			&CBaseEntity::PrecacheModel),
		def("PrecacheSound",			&CBaseEntity::PrecacheScriptSound),
		def("PrintBool",				&FFLib::PrintBool),
		def("RandomFloat",				&FFLib::RandomFloat),
		def("RandomInt",				&FFLib::RandomInt),
		def("RemoveEntity",				&FFLib::RemoveEntity),
		def("RemoveHudItem",			&FFLib::RemoveHudItem),
		def("RemoveHudItemFromTeam",	&FFLib::RemoveHudItemFromTeam),
		def("RemoveHudItemFromAll",		&FFLib::RemoveHudItemFromAll),
		//def("RespawnAllPlayers",		&FFLib::RespawnAllPlayers),
		def("ResetMap",					&FFLib::ResetMap),
		def("SetGlobalRespawnDelay",	&FFLib::SetGlobalRespawnDelay),
		def("SetPlayerLimit",			&FFLib::SetPlayerLimit),
		def("SetPlayerLimits",			&FFLib::SetPlayerLimits),
		def("SetClassLimits",			&FFLib::SmartClassLimits),
		def("SetConvar",				&FFLib::SetConvar),
		def("SetTeamClassLimit",		&FFLib::SetTeamClassLimit),
		def("SetTeamName",				&FFLib::SetTeamName),
		def("SmartMessage",				(void(*)(CBaseEntity *, const char*, const char*, const char*))&FFLib::SmartMessage),
		def("SmartMessage",				(void(*)(CBaseEntity *, const char*, const char*, const char*, int, int, int))&FFLib::SmartMessage),
		def("SmartMessage",				(void(*)(CBaseEntity *, const char*, const char*, const char*, const char*, const char*, const char*))&FFLib::SmartMessage),
		def("SmartMessage",				(void(*)(CBaseEntity *, const char*, const char*, const char*, int, const char*, const char*))&FFLib::SmartMessage),
		def("SmartMessage",				(void(*)(CBaseEntity *, const char*, const char*, const char*, int, int, const char*))&FFLib::SmartMessage),
		def("SmartMessage",				(void(*)(CBaseEntity *, const char*, const char*, const char*, int, const char*, int))&FFLib::SmartMessage),
		def("SmartMessage",				(void(*)(CBaseEntity *, const char*, const char*, const char*, const char*, int, const char*))&FFLib::SmartMessage),
		def("SmartMessage",				(void(*)(CBaseEntity *, const char*, const char*, const char*, const char*, int, int))&FFLib::SmartMessage),
		def("SmartSound",				&FFLib::SmartSound),
		def("SmartSpeak",				&FFLib::SmartSpeak),
		def("SmartTeamMessage",			(void(*)(CFFTeam *, const char*, const char*))&FFLib::SmartTeamMessage),
		def("SmartTeamMessage",			(void(*)(CFFTeam *, const char*, const char*, int, int))&FFLib::SmartTeamMessage),
		def("SmartTeamMessage",			(void(*)(CFFTeam *, const char*, const char*, const char*, const char*))&FFLib::SmartTeamMessage),
		def("SmartTeamMessage",			(void(*)(CFFTeam *, const char*, const char*, int, const char*))&FFLib::SmartTeamMessage),
		def("SmartTeamMessage",			(void(*)(CFFTeam *, const char*, const char*, const char*, int))&FFLib::SmartTeamMessage),
		def("SmartTeamSound",			&FFLib::SmartTeamSound),
		def("SmartTeamSpeak",			&FFLib::SmartTeamSpeak),
		def("SpeakAll",					&FFLib::SpeakAll),
		def("SpeakPlayer",				&FFLib::SpeakPlayer),
		def("SpeakTeam",				&FFLib::SpeakTeam),
		def("LogLuaEvent",				(void(*)(int, int, const char *))&FFLib::LogLuaEvent),
		def("LogLuaEvent",				(void(*)(int, int, const char *, const char *, const char *))&FFLib::LogLuaEvent),
		def("LogLuaEvent",				(void(*)(int, int, const char *, const char *, const char *, const char *, const char *))&FFLib::LogLuaEvent),
		def("LogLuaEvent",				(void(*)(int, int, const char *, const char *, const char *, const char *, const char *, const char *, const char *))&FFLib::LogLuaEvent),
		def("ObjectiveNotice",			&FFLib::ObjectiveNotice),
		def("UpdateObjectiveIcon",		&FFLib::UpdateObjectiveIcon),
		def("UpdateTeamObjectiveIcon",	&FFLib::UpdateTeamObjectiveIcon),
		def("DisplayMessage",			&FFLib::DisplayMessage),
		def("SetGameDescription",		&FFLib::SetGameDescription),
		def("ShowMenuToPlayer",			&FFLib::ShowMenuToPlayer),
		def("ShowMenuToTeam",			&FFLib::ShowMenuToTeam),
		def("ShowMenu",					&FFLib::ShowMenu),
		def("CreateMenu",				(void(*)(const char *))&FFLib::CreateMenu),
		def("CreateMenu",				(void(*)(const char *, float))&FFLib::CreateMenu),
		def("CreateMenu",				(void(*)(const char *, const char *))&FFLib::CreateMenu),
		def("CreateMenu",				(void(*)(const char *, const char *, float))&FFLib::CreateMenu),
		def("DestroyMenu",				&FFLib::DestroyMenu),
		def("SetMenuTitle",				&FFLib::SetMenuTitle),
		def("AddMenuOption",			&FFLib::AddMenuOption),
		def("RemoveMenuOption",			&FFLib::RemoveMenuOption)
	];
}
