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
#include "ff_utils.h" // for class_intToString

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
		gameeventmanager->AddListener( this, "player_changeclass", true );
		
		return BaseClass::Init();
	}

	bool PrintEvent( IGameEvent * event )	// override virtual function
	{
		const char *name = event->GetName();

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

		
		// BEG: Watch for players changing class
		if( !Q_strncmp( name, "player_changeclass", Q_strlen( "player_changeclass" ) ) )
		{
			const int attackerid = event->GetInt( "userid" );
			const int oldclass = event->GetInt( "oldclass" );
			const int newclass = event->GetInt( "newclass" );

			CBasePlayer *pAttacker = UTIL_PlayerByUserId( attackerid );
			if ( pAttacker )
			{
				char bracket0[50];
				char bracket1[50];

				Q_snprintf(bracket0, sizeof(bracket0)," (oldclass \"%s\")",  Class_IntToString(oldclass));

				Q_snprintf(bracket1, sizeof(bracket1), " (newclass \"%s\")", Class_IntToString(newclass));

				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"player_changeclass\"%s%s\n", 
					pAttacker->GetPlayerName(), 
					attackerid, 
					pAttacker->GetNetworkIDString(), 
					pAttacker->TeamID(),
					bracket0, 
					bracket1 );
			}
		}
		// END: Watch for players changing class

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

		// BEG: LUA events
		if( !Q_strncmp( name, "luaevent", Q_strlen( "luaevent" ) ) )
		{
			// WARNING: lua doesnt give you player IDs, it gives you player index. 
			//          This is why we use PlayerByIndex and GetPlayerUserId unlike other logging calls. - AfterShock
			const int ownerid = event->GetInt( "userid2" ); // owner is typically the victim 
			const int attackerid = event->GetInt( "userid" ); // attacker is typically the one triggering the event
			const char *eventName = event->GetString( "eventname" );

			const char *key0 = event->GetString( "key0" );
			const char *value0 = event->GetString( "value0" );
			const char *key1 = event->GetString( "key1" );
			const char *value1 = event->GetString( "value1" );
			const char *key2 = event->GetString( "key2" );
			const char *value2 = event->GetString( "value2" );
				
			char bracket0[50];
			char bracket1[50];
			char bracket2[50];

			if (strlen(key0))
			{
				Q_snprintf(bracket0, sizeof(bracket0)," (%s \"%s\")", key0, value0);
			}
			if (strlen(key1))
			{
				Q_snprintf(bracket1, sizeof(bracket1), " (%s \"%s\")", key1, value1);
			}
			if (strlen(key2))
			{
				Q_snprintf(bracket2, sizeof(bracket2), " (%s \"%s\")", key2, value2);
			}
			
			bool bNoAttacker = ( attackerid == 0 );
			bool bNoVictim = ( ownerid == 0 );

			if( bNoAttacker )
			{
				if ( bNoVictim )
				{
					
					// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
					UTIL_LogPrintf( "World triggered \"%s\"%s%s%s\n", 
						eventName, 
						strlen(key0) ? bracket0 : "", 
						strlen(key1) ? bracket1 : "", 
						strlen(key2) ? bracket2 : "" );
				}
				else
				{
					
					CBasePlayer *pOwner = UTIL_PlayerByIndex( ownerid ); // yes we used PlayerByIndex rather than PlayerByUserId
					CTeam *oteam = NULL; // owners (person doing the exposing) team
					oteam = pOwner->GetTeam();

					/*
				char bracketOriginOwner[50];

				if (strlen(key0))
				{
					Q_snprintf(bracket0, sizeof(bracket0)," (%s \"%s\")", key0, value0);
				}
				 Vector v_dist = pPlayer->pev->origin;
*/
					// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
					UTIL_LogPrintf( "World triggered \"%s\" against \"%s<%i><%s><%s>\"%s%s%s\n", 
						eventName, 
						pOwner->GetPlayerName(), 
						engine->GetPlayerUserId(pOwner->edict()), 
						pOwner->GetNetworkIDString(),	
						oteam ? oteam->GetName() : "",
						strlen(key0) ? bracket0 : "", 
						strlen(key1) ? bracket1 : "", 
						strlen(key2) ? bracket2 : "" );
				} 
			}
			else
			{
				
				CBasePlayer *pAttacker = UTIL_PlayerByIndex( attackerid );
				CTeam *ateam = NULL; // attackers (spies) team
				ateam = pAttacker->GetTeam();

				if ( bNoVictim )
				{
					// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
					UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"%s\"%s%s%s\n", 
						pAttacker->GetPlayerName(), 
						engine->GetPlayerUserId(pAttacker->edict()), 
						pAttacker->GetNetworkIDString(), 
						ateam ? ateam->GetName() : "",
						eventName, 
						strlen(key0) ? bracket0 : "", 
						strlen(key1) ? bracket1 : "", 
						strlen(key2) ? bracket2 : "" );
				}
				else
				{
					CBasePlayer *pOwner = UTIL_PlayerByIndex( ownerid );
					CTeam *oteam = NULL; // owners (person doing the exposing) team
					oteam = pOwner->GetTeam();

					// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
					UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"%s\" against \"%s<%i><%s><%s>\"%s%s%s\n", 
						pAttacker->GetPlayerName(), 
						engine->GetPlayerUserId(pAttacker->edict()), 
						pAttacker->GetNetworkIDString(), 
						ateam ? ateam->GetName() : "",
						eventName, 
						pOwner->GetPlayerName(), 
						engine->GetPlayerUserId(pOwner->edict()), 
						pOwner->GetNetworkIDString(),	
						oteam ? oteam->GetName() : "",
						strlen(key0) ? bracket0 : "", 
						strlen(key1) ? bracket1 : "", 
						strlen(key2) ? bracket2 : "" );
				}
			}
		}
		// END: lua event

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

