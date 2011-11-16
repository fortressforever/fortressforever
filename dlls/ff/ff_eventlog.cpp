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
		gameeventmanager->AddListener( this, "build_mancannon", true );
		gameeventmanager->AddListener( this, "dispenser_killed", true );
		gameeventmanager->AddListener( this, "dispenser_dismantled", true );
		gameeventmanager->AddListener( this, "dispenser_detonated", true );
		gameeventmanager->AddListener( this, "mancannon_detonated", true );
		gameeventmanager->AddListener( this, "detpack_detonated", true );
		gameeventmanager->AddListener( this, "sentrygun_killed", true );
		gameeventmanager->AddListener( this, "sentry_dismantled", true );
		gameeventmanager->AddListener( this, "sentry_detonated", true );
		gameeventmanager->AddListener( this, "disguise_lost", true );
		gameeventmanager->AddListener( this, "cloak_lost", true );
		gameeventmanager->AddListener( this, "luaevent", true );
		gameeventmanager->AddListener( this, "player_changeclass", true );
		gameeventmanager->AddListener( this, "sentrygun_upgraded", true );
		gameeventmanager->AddListener( this, "sentry_sabotaged", true );
		gameeventmanager->AddListener( this, "dispenser_sabotaged", true );
		gameeventmanager->AddListener( this, "dispenser_sabotaged", true );
		gameeventmanager->AddListener( this, "ff_restartround", true );

		//gameeventmanager->AddListener( this, "player_team", true );

		return BaseClass::Init();
	}

	bool PrintEvent( IGameEvent * event )	// override virtual function
	{
		const char *name = event->GetName();

		// caes: some copy/paste action
		// BEG: Watch for SG sabotage
		if( !Q_strncmp( name, "sentry_sabotaged", Q_strlen( "sentry_sabotaged" ) ) )
		{
			const int ownerid = event->GetInt( "userid" );
			const int attackerid = event->GetInt( "saboteur" );

			CBasePlayer *pOwner = UTIL_PlayerByUserId( ownerid );
			CBasePlayer *pAttacker = UTIL_PlayerByUserId( attackerid );
			CTeam *oteam = pOwner->GetTeam(); // owner's (victim's) team
			CTeam *ateam = pAttacker->GetTeam(); // attacker's (saboteur's) team

			// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"sentry_sabotaged\" against \"%s<%i><%s><%s>\"\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(), oteam ? oteam->GetName() : "" );
			DevMsg( "\"%s<%i><%s><%s>\" triggered \"sentry_sabotaged\" against \"%s<%i><%s><%s>\"\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(), oteam ? oteam->GetName() : "" );
		}
		// END: Watch for SG sabotage

		// BEG: Watch for dispenser sabotage
		if( !Q_strncmp( name, "dispenser_sabotaged", Q_strlen( "dispenser_sabotaged" ) ) )
		{
			const int ownerid = event->GetInt( "userid" );
			const int attackerid = event->GetInt( "saboteur" );

			CBasePlayer *pOwner = UTIL_PlayerByUserId( ownerid );
			CBasePlayer *pAttacker = UTIL_PlayerByUserId( attackerid );
			CTeam *oteam = pOwner->GetTeam(); // owner's (victim's) team
			CTeam *ateam = pAttacker->GetTeam(); // attacker's (saboteur's) team

			// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"dispenser_sabotaged\" against \"%s<%i><%s><%s>\"\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(), oteam ? oteam->GetName() : "" );
			DevMsg( "\"%s<%i><%s><%s>\" triggered \"dispenser_sabotaged\" against \"%s<%i><%s><%s>\"\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(), oteam ? oteam->GetName() : "" );
		}
		// END: Watch for dispenser sabotage
		// caes

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
			else if( Q_strcmp( eventName, "build_mancannon" ) == 0 )
				Q_strcpy( szObject, "mancannon" );
			
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"build_%s\"\n", pPlayer->GetPlayerName(), userid, pPlayer->GetNetworkIDString(), oteam ? oteam->GetName() : "", szObject );

			DevMsg( "\"%s<%i><%s><%s>\" built a %s\n", pPlayer->GetPlayerName(), userid, pPlayer->GetNetworkIDString(), oteam ? oteam->GetName() : "", szObject );
		}
		// END: Watching when buildables get built

		// BEG: Watch for SG dismantle
		if( !Q_strncmp( name, "sentry_dismantled", Q_strlen( "sentry_dismantled" ) ) )
		{
			const int sgownerid = event->GetInt( "userid" );
			const int level = event->GetInt( "level" );

			CBasePlayer *pSGOwner = UTIL_PlayerByUserId( sgownerid );

			char bracket0[50];

			Q_snprintf(bracket0, sizeof(bracket0)," (level \"%i\")", level);
			// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"sentry_dismantled\"%s\n", 
				pSGOwner->GetPlayerName(), 
				sgownerid, 
				pSGOwner->GetNetworkIDString(), 
				pSGOwner->TeamID(),
				bracket0 );
		}
		// END: Watch for SG dismantle

		// BEG: Watch for SG detonate
		if( !Q_strncmp( name, "sentry_detonated", Q_strlen( "sentry_detonated" ) ) )
		{
			const int sgownerid = event->GetInt( "userid" );
			const int level = event->GetInt( "level" );

			CBasePlayer *pSGOwner = UTIL_PlayerByUserId( sgownerid );

			char bracket0[50];

			Q_snprintf(bracket0, sizeof(bracket0)," (level \"%i\")", level);
			// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"sentry_detonated\"%s\n", 
				pSGOwner->GetPlayerName(), 
				sgownerid, 
				pSGOwner->GetNetworkIDString(), 
				pSGOwner->TeamID(),
				bracket0 );
		}
		// END: Watch for SG detonate

		// BEG: Watch for dispenser detonate
		if( !Q_strncmp( name, "dispenser_detonated", Q_strlen( "dispenser_detonated" ) ) )
		{
			const int sgownerid = event->GetInt( "userid" );

			CBasePlayer *pSGOwner = UTIL_PlayerByUserId( sgownerid );
			// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"dispenser_detonated\"\n", 
				pSGOwner->GetPlayerName(), 
				sgownerid, 
				pSGOwner->GetNetworkIDString(), 
				pSGOwner->TeamID() );
		}
		// END: Watch for dispenser detonate

		// BEG: Watch for mancannon detonate
		if( !Q_strncmp( name, "mancannon_detonated", Q_strlen( "mancannon_detonated" ) ) )
		{
			const int sgownerid = event->GetInt( "userid" );

			CBasePlayer *pSGOwner = UTIL_PlayerByUserId( sgownerid );
			// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"mancannon_detonated\"\n", 
				pSGOwner->GetPlayerName(), 
				sgownerid, 
				pSGOwner->GetNetworkIDString(), 
				pSGOwner->TeamID() );
		}
		// END: Watch for mancannon detonate

		if( !Q_strncmp( name, "detpack_detonated", Q_strlen( "detpack_detonated" ) ) )
		{
			const int sgownerid = event->GetInt( "userid" );

			CBasePlayer *pSGOwner = UTIL_PlayerByUserId( sgownerid );
			// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"detpack_detonated\"\n", 
				pSGOwner->GetPlayerName(), 
				sgownerid, 
				pSGOwner->GetNetworkIDString(), 
				pSGOwner->TeamID() );
		}

		// BEG: Watch for dispenser dismantle
		if( !Q_strncmp( name, "dispenser_dismantled", Q_strlen( "dispenser_dismantled" ) ) )
		{
			const int sgownerid = event->GetInt( "userid" );

			CBasePlayer *pSGOwner = UTIL_PlayerByUserId( sgownerid );
			// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"dispenser_dismantled\"\n", 
				pSGOwner->GetPlayerName(), 
				sgownerid, 
				pSGOwner->GetNetworkIDString(), 
				pSGOwner->TeamID() );
		}
		// END: Watch for dispenser dismantle



		// BEG: Watch for SG upgrades
		if( !Q_strncmp( name, "sentrygun_upgraded", Q_strlen( "sentrygun_upgraded" ) ) )
		{
			const int attackerid = event->GetInt( "userid" );
			const int sgownerid = event->GetInt( "sgownerid" );
			const int level = event->GetInt( "level" );

			CBasePlayer *pAttacker = UTIL_PlayerByUserId( attackerid );
			CBasePlayer *pSGOwner = UTIL_PlayerByUserId( sgownerid );

			char bracket0[50];

			Q_snprintf(bracket0, sizeof(bracket0)," (level \"%i\")", level);
			if (attackerid == sgownerid) // upgraded your own SG
			{
				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"sentrygun_upgraded\"%s\n", 
					pAttacker->GetPlayerName(), 
					attackerid, 
					pAttacker->GetNetworkIDString(), 
					pAttacker->TeamID(),
					bracket0 );
			}
			else
			{
				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"sentrygun_upgraded\" against \"%s<%i><%s><%s>\"%s\n", 
					pAttacker->GetPlayerName(), 
					attackerid, 
					pAttacker->GetNetworkIDString(), 
					pAttacker->TeamID(),
					pSGOwner->GetPlayerName(),
					sgownerid,
					pSGOwner->GetNetworkIDString(),
					pSGOwner->TeamID(),
					bracket0 );
			}

		}
		// END: Watch for SG upgrades

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
/*
		// BEG: Watch for players changing team
		if( !Q_strncmp( name, "player_team", Q_strlen( "player_team" ) ) )
		{
			const int attackerid = event->GetInt( "userid" );
			const int oldteam = event->GetInt( "oldteam" );
			const int newteam = event->GetInt( "team" );

			CBasePlayer *pAttacker = UTIL_PlayerByUserId( attackerid );
			if ( pAttacker )
			{
				char bracket0[50];
				char bracket1[50];

				Q_snprintf(bracket0, sizeof(bracket0)," (oldteam \"%s\")", (GetGlobalTeam(oldteam))->GetName());

				Q_snprintf(bracket1, sizeof(bracket1), " (newteam \"%s\")", (GetGlobalTeam(newteam))->GetName());

				// technically we should be printing ownerid / attackerid instead of "" when teams arent set up
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"player_team\"%s%s\n", 
					pAttacker->GetPlayerName(), 
					attackerid, 
					pAttacker->GetNetworkIDString(), 
					pAttacker->TeamID(),
					bracket0, 
					bracket1 );
			}
		}
		// END: Watch for players changing team
*/

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
			const char *attackerpos = event->GetString( "attackerpos" );
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
				UTIL_LogPrintf( "\"%s<%i><%s><%s>\" triggered \"kill_sentrygun\" against \"%s<%i><%s><%s>\" (weapon \"%s\") (attackerpos \"%s\")\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "", event->GetString( "weapon" ), attackerpos );
				DevMsg( "\"%s<%i><%s><%s>\" triggered \"kill_sentrygun\" against \"%s<%i><%s><%s>\" (weapon \"%s\") (attackerpos \"%s\")\n", pAttacker->GetPlayerName(), attackerid, pAttacker->GetNetworkIDString(), ateam ? ateam->GetName() : "", pOwner->GetPlayerName(), ownerid, pOwner->GetNetworkIDString(),	oteam ? oteam->GetName() : "", event->GetString( "weapon" ), attackerpos );
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
		
		// BEG: ff_restartround
		if( !Q_strncmp( name, "ff_restartround", Q_strlen( "ff_restartround" ) ) )
		{
			UTIL_LogPrintf( "Round restarted\n");
		}
		// END: ff_restartround

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

