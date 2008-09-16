//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "../EventLog.h"
#include "KeyValues.h"
#include "team.h"
#include "ff_buildableobjects_shared.h"

class CFFEventLog : public CEventLog
{
private:
	typedef CEventLog BaseClass;

public:
	virtual ~CFFEventLog() {};

public:
	bool Init( void )
	{
		gameeventmanager->AddListener( this, "build_dispenser", true );
		gameeventmanager->AddListener( this, "build_sentrygun", true );
		gameeventmanager->AddListener( this, "build_detpack", true );
		gameeventmanager->AddListener( this, "dispenser_killed", true );
		gameeventmanager->AddListener( this, "sentrygun_killed", true );
		gameeventmanager->AddListener( this, "disguise_lost", true );
		gameeventmanager->AddListener( this, "cloak_lost", true );
		gameeventmanager->AddListener( this, "luaevent", true );
		
		return BaseClass::Init();
	}

	bool PrintEvent( IGameEvent * event )	// override virtual function
	{
		const char *name = event->GetName();

		// BEG: Watching for a Lua event
		if( !Q_strncmp( name, "luaevent", Q_strlen( "luaevent" ) ) )
		{
			// entity index of the Lua object (flag, ball, etc)
			const int luaObjectEntityIndex = event->GetInt( "luaid" );
			// entity index of some CBaseEntity* that is doing something to the lua object
			const int actorEntityIndex = event->GetInt( "objid" );
			// name for the event - like flag_cap, flag_drop, etc - something stats programs could use
			const char *eventName = event->GetString( "name" );
			// "Extra" fields
			const char *field0 = event->GetString( "field0" );
			const char *field1 = event->GetString( "field1" );
			const char *field2 = event->GetString( "field2" );
			const char *field3 = event->GetString( "field3" );
			const char *field4 = event->GetString( "field4" );
			const char *field5 = event->GetString( "field5" );
			const char *field6 = event->GetString( "field6" );

			// Hot logging action!
			UTIL_LogPrintf( "%s, %i, %i, %s, %s, %s, %s, %s, %s, %s\n", eventName, luaObjectEntityIndex, actorEntityIndex, field0, field1, field2, field3, field4, field5, field6 );
			DevMsg( "%s, %i, %i, %s, %s, %s, %s, %s, %s, %s\n", eventName, luaObjectEntityIndex, actorEntityIndex, field0, field1, field2, field3, field4, field5, field6 );
		}
		// END: Watching for a Lua event

		// BEG: Watching when buildables get built
		if( Q_strncmp( name, "build_", strlen( "build_" ) ) == 0 )
		{
			const char *eventName = event->GetName();
			const int userid = event->GetInt( "userid" );

			CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByUserId( userid ) );
			CTeam *oteam = NULL; // owners (victims) team

			if( pPlayer )
				oteam = pPlayer->GetTeam();

			char szObject[ 64 ];
			if( Q_strcmp( eventName, "build_dispenser" ) == 0 )
				Q_strcpy( szObject, "dispenser" );
			else if( Q_strcmp( eventName, "build_sentrygun" ) == 0 )
				Q_strcpy( szObject, "sentrygun" );
			else if( Q_strcmp( eventName, "build_detpack" ) == 0 )
				Q_strcpy( szObject, "detpack" );

			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"build_%s\"\n", pPlayer->GetPlayerName(), userid, pPlayer->GetNetworkIDString(), oteam ? oteam->GetName() : "", szObject );

			DevMsg( "\"%s<%i><%s><%s>\" built a %s\n", pPlayer->GetPlayerName(), userid, pPlayer->GetNetworkIDString(), oteam ? oteam->GetName() : "", szObject );
		}
		// END: Watching when buildables get built

		// BEG: Watch for buildables getting killed
		if( !Q_strncmp( name, "dispenser_killed", Q_strlen( "dispenser_killed" ) ) )
		{
			const int ownerid = event->GetInt( "userid" );
			const int attackerid = event->GetInt( "attacker" );

			bool bWorldSpawn = ( attackerid == 0 );

			CBasePlayer *pOwner = UTIL_PlayerByUserId( ownerid );
			CTeam *ateam = NULL; // attackers team
			CTeam *oteam = NULL; // owners (victims) team

			if( bWorldSpawn )
			{
				// is this even possible ?
				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "World triggered \"kill_dispenser\" against \"%s<%i><%s><%s>\"\n", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "" );
			}
			else
			{
				CBasePlayer *pAttacker = UTIL_PlayerByUserId( attackerid );
				ateam = pAttacker->GetTeam();
				oteam = pOwner->GetTeam();

				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"kill_dispenser\" against \"%s<%i><%s><%s>\" (weapon \"%s\")\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "", event->GetString( "weapon" ) );
				DevMsg( "\"%s<%i><%s><%s>\" triggered \"kill_dispenser\" against \"%s<%i><%s><%s>\" (weapon \"%s\")\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "", event->GetString( "weapon" ) );
			}
		}
		else if( !Q_strncmp( name, "sentrygun_killed", Q_strlen( "sentrygun_killed" ) ) )
		{
			const int ownerid = event->GetInt( "userid" );
			const int attackerid = event->GetInt( "attacker" );

			bool bWorldSpawn = ( attackerid == 0 );

			CBasePlayer *pOwner = UTIL_PlayerByUserId( ownerid );
			CBasePlayer *pAttacker = NULL;
			CTeam *ateam = NULL; // attackers team
			CTeam *oteam = NULL; // owners (victims) team

			if( bWorldSpawn )
			{
				// is this even possible ?
				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "World triggered \"kill_sentrygun\" against \"%s<%i><%s><%s>\"\n", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "" );
			}
			else
			{
				pAttacker = UTIL_PlayerByUserId( attackerid );
				ateam = pAttacker->GetTeam();
				oteam = pOwner->GetTeam();

				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"kill_sentrygun\" against \"%s<%i><%s><%s>\" (weapon \"%s\")\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "", event->GetString( "weapon" ) );
				DevMsg( "\"%s<%i><%s><%s>\" triggered \"kill_sentrygun\" against \"%s<%i><%s><%s>\" (weapon \"%s\")\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "", event->GetString( "weapon" ) );
			}		
		}
		// END: Watch for buildables getting killed

		// BEG: Spy exposed
		if( !Q_strncmp( name, "disguise_lost", Q_strlen( "disguise_lost" ) ) )
		{
			const int ownerid = event->GetInt( "userid" ); // owner is the victim (the spy)
			const int attackerid = event->GetInt( "attackerid" ); // attacker is the scout doing the uncloaking

			bool bWorldSpawn = ( attackerid == 0 );

			CBasePlayer *pOwner = UTIL_PlayerByUserId( ownerid );
			CTeam *ateam = NULL; // attackers (spies) team
			CTeam *oteam = NULL; // owners (person doing the exposing) team

			if( bWorldSpawn )
			{
				// is this even possible ?
				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "World triggered \"disguise_lost\" against \"%s<%i><%s><%s>\"\n", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "" );
			}
			else
			{
				CBasePlayer *pAttacker = UTIL_PlayerByUserId( attackerid );
				ateam = pAttacker->GetTeam();
				oteam = pOwner->GetTeam();

				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"disguise_lost\" against \"%s<%i><%s><%s>\"\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "" );
				DevMsg( "\"%s<%i><%s><%s>\" triggered \"disguise_lost\" against \"%s<%i><%s><%s>\"\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "" );
			}
		}
		// END: spy exposed

		// BEG: Spy uncloaked
		if( !Q_strncmp( name, "cloak_lost", Q_strlen( "cloak_lost" ) ) )
		{
			
			const int ownerid = event->GetInt( "userid" ); // owner is the victim (the spy)
			const int attackerid = event->GetInt( "attackerid" ); // attacker is the scout doing the uncloaking

			bool bWorldSpawn = ( attackerid == 0 );

			CBasePlayer *pOwner = UTIL_PlayerByUserId( ownerid );
			CTeam *ateam = NULL; // attackers (spies) team
			CTeam *oteam = NULL; // owners (person doing the exposing) team

			if( bWorldSpawn )
			{
				// is this even possible ?
				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "World triggered \"cloak_lost\" against \"%s<%i><%s><%s>\"\n", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "" );
			}
			else
			{
				CBasePlayer *pAttacker = UTIL_PlayerByUserId( attackerid );
				ateam = pAttacker->GetTeam();
				oteam = pOwner->GetTeam();

				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"cloak_lost\" against \"%s<%i><%s><%s>\"\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "" );
				DevMsg( "\"%s<%i><%s><%s>\" triggered \"cloak_lost\" against \"%s<%i><%s><%s>\"\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "" );
			}
		}
		// END: spy uncloaked

		if ( BaseClass::PrintEvent( event ) )
		{
			return true;
		}
	
		if ( Q_strcmp(event->GetName(), "ff_") == 0 )
		{
			return PrintFFEvent( event );
		}

		return false;
	}

protected:

	bool PrintFFEvent( IGameEvent * event )	// print Mod specific logs
	{
		//const char * name = event->GetName() + Q_strlen("ff_"); // remove prefix
		return false;
	}

};

CFFEventLog g_FFEventLog;

//-----------------------------------------------------------------------------
// Singleton access
//-----------------------------------------------------------------------------
IGameSystem* GameLogSystem()
{
	return &g_FFEventLog;
}

