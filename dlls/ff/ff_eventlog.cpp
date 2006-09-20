//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
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
			CTeam *team = NULL;

			if( pPlayer )
				team = pPlayer->GetTeam();

			char szObject[ 64 ];
			if( Q_strcmp( eventName, "build_dispenser" ) == 0 )
				Q_strcpy( szObject, "dispenser" );
			else if( Q_strcmp( eventName, "build_sentrygun" ) == 0 )
				Q_strcpy( szObject, "sentrygun" );
			else if( Q_strcmp( eventName, "build_detpack" ) == 0 )
				Q_strcpy( szObject, "detpack" );

			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" built a %s\n", pPlayer->GetPlayerName(), userid, pPlayer->GetNetworkIDString(), team ? team->GetName() : "", szObject );

			DevMsg( "\"%s<%i><%s><%s>\" built a %s\n", pPlayer->GetPlayerName(), userid, pPlayer->GetNetworkIDString(), team ? team->GetName() : "", szObject );
		}
		// END: Watching when buildables get built

		// BEG: Watch for buildables getting killed
		if( !Q_strncmp( name, "dispenser_killed", Q_strlen( "dispenser_killed" ) ) )
		{
			const int ownerid = event->GetInt( "userid" );
			const int attackerid = event->GetInt( "attacker" );

			bool bWorldSpawn = ( attackerid == 0 );

			CBasePlayer *pOwner = UTIL_PlayerByUserId( ownerid );
			CBasePlayer *pAttacker = NULL;
			if( !bWorldSpawn )
				pAttacker = UTIL_PlayerByUserId( attackerid );

			CTeam *team = NULL;
			if( !bWorldSpawn )
				team = pAttacker->GetTeam();

			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" killed %s's dispenser with %s\n", bWorldSpawn ? "worldspawn" : pAttacker->GetPlayerName(), attackerid, bWorldSpawn ? "" : pAttacker->GetNetworkIDString(), team ? team->GetName() : "", pOwner->GetPlayerName(), event->GetString( "weapon" ) );
			DevMsg( "\"%s<%i><%s><%s>\" killed %s's dispenser with %s\n", bWorldSpawn ? "worldspawn" : pAttacker->GetPlayerName(), attackerid, bWorldSpawn ? "" : pAttacker->GetNetworkIDString(), team ? team->GetName() : "", pOwner->GetPlayerName(), event->GetString( "weapon" ) );
		}
		else if( !Q_strncmp( name, "sentrygun_killed", Q_strlen( "sentrygun_killed" ) ) )
		{
			const int ownerid = event->GetInt( "userid" );
			const int attackerid = event->GetInt( "attacker" );

			bool bWorldSpawn = ( attackerid == 0 );

			CBasePlayer *pOwner = UTIL_PlayerByUserId( ownerid );
			CBasePlayer *pAttacker = NULL;
			if( !bWorldSpawn )
				pAttacker = UTIL_PlayerByUserId( attackerid );

			CTeam *team = NULL;
			if( !bWorldSpawn )
				team = pAttacker->GetTeam();

			UTIL_LogPrintf( "\"%s<%i><%s><%s>\" killed %s's sentrygun with %s\n", bWorldSpawn ? "worldspawn" : pAttacker->GetPlayerName(), attackerid, bWorldSpawn ? "" : pAttacker->GetNetworkIDString(), team ? team->GetName() : "", pOwner->GetPlayerName(), event->GetString( "weapon" ) );
			DevMsg( "\"%s<%i><%s><%s>\" killed %s's sentrygun with %s\n", bWorldSpawn ? "worldspawn" : pAttacker->GetPlayerName(), attackerid, bWorldSpawn ? "" : pAttacker->GetNetworkIDString(), team ? team->GetName() : "", pOwner->GetPlayerName(), event->GetString( "weapon" ) );
		}
		// END: Watch for buildables getting killed


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

