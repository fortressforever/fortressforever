//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: The TF Game rules 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ff_gamerules.h"
#include "ammodef.h"
#include "KeyValues.h"
#include "filesystem.h"
#include "ff_weapon_base.h"
#include "ff_projectile_base.h"

#ifdef CLIENT_DLL
	#define C_FFTeam CFFTeam
	#include "c_ff_team.h"

#else
	#include "voice_gamemgr.h"
	#include "ff_team.h"
	#include "ff_player.h"
	#include "ff_playercommand.h"
	#include "ff_sentrygun.h"
	#include "ff_item_flag.h"
	#include "ff_entity_system.h"
	#include "ff_scriptman.h"
	#include "ff_luacontext.h"
	#include "ff_scheduleman.h"
	#include "ff_timerman.h"
	#include "ff_utils.h"
	#include "ff_buildableobjects_shared.h"
#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Okay the server and client are versioned separately so that we can do a 
// client- or server- only update if required.
// The client version is checked when FF is loaded
// The client + server versions are checked when a server is joined
// It is the discretion of the web-based version check script whether or not
// to tell the client to upgrade or that the server they are joining is out of date.
/*
// Jon: starting with 2.3, the versions are set through resource/version_client.txt and resource/version_server.txt
//const char *MOD_CLIENT_VERSION = "2.3";
//const char *MOD_SERVER_VERSION = "2.3";

// Ignore this for now...!
void VersionCvarChange(ConVar *var, const char *pOldString) {}
ConVar cvar_version("sync_version_cvar", MOD_SERVER_VERSION, FCVAR_REPLICATED, ""
#ifdef CLIENT_DLL
					, VersionCvarChange);
#else
					);
#endif
*/

#define MAX_MOD_VERSION_LENGTH 16

// this can be different between client and server
char MOD_VERSION[MAX_MOD_VERSION_LENGTH] = { 0 };

#ifdef CLIENT_DLL
	#define MOD_VERSION_FILENAME "resource/ff_version_client.txt"
#else
	#define MOD_VERSION_FILENAME "resource/ff_version_server.txt"
#endif

char *GetModVersion()
{
	if ( MOD_VERSION[0] )
		return MOD_VERSION;

	FileHandle_t hFile = filesystem->Open( MOD_VERSION_FILENAME, "r", "MOD" );

	if (!hFile)
	{
		Warning("[VERSION] %s either does not exist or could not be opened.\n", MOD_VERSION_FILENAME);
		return "0.0";
	}

	// allocate buffer for the file's first line
	char *buffer = (char*)MemAllocScratch(MAX_MOD_VERSION_LENGTH);
	Assert(buffer);

	// load file's first line into a null-terminated buffer
	filesystem->ReadLine(buffer, MAX_MOD_VERSION_LENGTH, hFile);
	buffer[MAX_MOD_VERSION_LENGTH] = 0;
	filesystem->Close(hFile);

	if ( buffer[0] )
	{
		Q_snprintf( MOD_VERSION, sizeof(MOD_VERSION), "%s", buffer );
		if ( MOD_VERSION[0] )
		{
			Msg(
#ifdef CLIENT_DLL
				"[VERSION] Client:  %s\n"
#else
				"[VERSION] Server:  %s\n"
#endif
				, MOD_VERSION);

			return MOD_VERSION;
		}
	}

	return "0.0";
}

#ifndef CLIENT_DLL
// Let's not use these or allow them.
//LINK_ENTITY_TO_CLASS(info_player_terrorist, CPointEntity);
//LINK_ENTITY_TO_CLASS(info_player_counterterrorist,CPointEntity);
#endif

// Believe we wanted this back in now
#define USE_HITBOX_HACK

REGISTER_GAMERULES_CLASS( CFFGameRules );


BEGIN_NETWORK_TABLE_NOBASE( CFFGameRules, DT_FFGameRules )
END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( ff_gamerules, CFFGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( FFGameRulesProxy, DT_FFGameRulesProxy )

// A bunch of bot settings. Maybe these should go in one of the omnibot files?
#ifdef GAME_DLL
	ConVar botrules_training("botrules_training", "", FCVAR_GAMEDLL);
	ConVar botrules_classlimits("botrules_classlimits", "", FCVAR_GAMEDLL);
	ConVar botrules_teamlimits("botrules_teamlimits", "", FCVAR_GAMEDLL);
	ConVar botrules_teamroles("botrules_teamroles", "", FCVAR_GAMEDLL);
	ConVar mp_respawndelay( "mp_respawndelay", "0", 0, "Time (in seconds) for spawn delays. Can be overridden by LUA." );

	bool g_Disable_Timelimit = false;
#endif

// 0000936: Horizontal push from explosions too low
#ifdef GAME_DLL
	//ConVar push_multiplier("ffdev_pushmultiplier", "8.0", FCVAR_REPLICATED | FCVAR_CHEAT);
	#define PUSH_MULTIPLIER 8.0f
	//ConVar fattypush_multiplier("ffdev_hwguypushmultiplier", ".15", FCVAR_REPLICATED | FCVAR_CHEAT);
	#define FATTYPUSH_MULTIPLIER 0.15f
	//ConVar nodamagepush_multiplier("ffdev_nodamagepushmultiplier", ".80", FCVAR_REPLICATED | FCVAR_CHEAT);
	#define NODAMAGEPUSH_MULTIPLIER 0.80f
	//ConVar push_clamp("ffdev_pushclamp", "450", FCVAR_REPLICATED | FCVAR_CHEAT);
	#define PUSH_CLAMP 450

	// AfterShock - increase IC self damage to reduce number of jumps you can do
	//ConVar ic_selfdamagemultiplier("ffdev_ic_selfdamagemultiplier", "1.8", FCVAR_REPLICATED | FCVAR_CHEAT);
	#define IC_SELFDAMAGEMULTIPLIER 1.8f
	
#endif

ConVar mp_prematch( "mp_prematch",
					"0.0",							// trepids finding it annoying so i set it to zero and not .2
					FCVAR_NOTIFY|FCVAR_REPLICATED,
					"delay before game starts" );

#ifdef CLIENT_DLL
	void RecvProxy_FFGameRules( const RecvProp *pProp, void **pOut, void *pData, int objectID )
	{
		CFFGameRules *pRules = FFGameRules();
		Assert( pRules );
		*pOut = pRules;
	}

	BEGIN_RECV_TABLE( CFFGameRulesProxy, DT_FFGameRulesProxy )
		RecvPropDataTable( "ff_gamerules_data", 0, 0, &REFERENCE_RECV_TABLE( DT_FFGameRules ), RecvProxy_FFGameRules )
	END_RECV_TABLE()
#else
	void *SendProxy_FFGameRules( const SendProp *pProp, const void *pStructBase, const void *pData, CSendProxyRecipients *pRecipients, int objectID )
	{
        CFFGameRules *pRules = FFGameRules();
		Assert( pRules );
		pRecipients->SetAllRecipients();
		return pRules;
	}

	BEGIN_SEND_TABLE( CFFGameRulesProxy, DT_FFGameRulesProxy )
		SendPropDataTable( "ff_gamerules_data", 0, &REFERENCE_SEND_TABLE( DT_FFGameRules ), SendProxy_FFGameRules )
	END_SEND_TABLE()
#endif


#ifdef CLIENT_DLL

#else

	// --> Mirv: Class limits

	// Need to update the real class limits for this map
	void ClassRestrictionChange( ConVar *var, const char *pOldString )
	{
		// Update the team limits
		for( int i = 0; i < g_Teams.Count(); i++ )
		{
            CFFTeam *pTeam = (CFFTeam *) GetGlobalTeam( i );

			pTeam->UpdateLimits();
		}
	}

	// Classes to restrict (not civilian though, would make no sense to)
	ConVar cr_scout( "cr_scout", "0", 0, "Max number of scouts", &ClassRestrictionChange );
	ConVar cr_sniper( "cr_sniper", "0", 0, "Max number of snipers", &ClassRestrictionChange );
	ConVar cr_soldier( "cr_soldier", "0", 0, "Max number of soldiers", &ClassRestrictionChange );
	ConVar cr_demoman( "cr_demoman", "0", 0, "Max number of demoman", &ClassRestrictionChange );
	ConVar cr_medic( "cr_medic", "0", 0, "Max number of medic", &ClassRestrictionChange );
	ConVar cr_hwguy( "cr_hwguy", "0", 0, "Max number of hwguy", &ClassRestrictionChange );
	ConVar cr_pyro( "cr_pyro", "0", 0, "Max number of pyro", &ClassRestrictionChange );
	ConVar cr_spy( "cr_spy", "0", 0, "Max number of spy", &ClassRestrictionChange );
	ConVar cr_engineer( "cr_engineer", "0", 0, "Max number of engineer", &ClassRestrictionChange );
	// <-- Mirv: Class limits

	// --------------------------------------------------------------------------------------------------- //
	// Voice helper
	// --------------------------------------------------------------------------------------------------- //

	class CVoiceGameMgrHelper : public IVoiceGameMgrHelper
	{
	public:
		virtual bool		CanPlayerHearPlayer( CBasePlayer *pListener, CBasePlayer *pTalker )
		{
			// Dead players can only be heard by other dead team mates
			if ( !pTalker->IsAlive() )
			{
				if ( !pListener->IsAlive())
					return g_pGameRules->PlayerRelationship(pTalker, pListener) == GR_TEAMMATE;
				return false;
			}
			return g_pGameRules->PlayerRelationship(pTalker, pListener) == GR_TEAMMATE;;
		}
	};
	CVoiceGameMgrHelper g_VoiceGameMgrHelper;
	IVoiceGameMgrHelper *g_pVoiceGameMgrHelper = &g_VoiceGameMgrHelper;



	// --------------------------------------------------------------------------------------------------- //
	// Globals.
	// --------------------------------------------------------------------------------------------------- //

	// NOTE: the indices here must match TEAM_TERRORIST, TEAM_CT, TEAM_SPECTATOR, etc.
	/*
	char *sTeamNames[] =
	{
		"Unassigned",
		"Spectator",
		"Terrorist",
		"Counter-Terrorist"
	};
	*/
	char *sTeamNames[ ] =
	{
		"#FF_TEAM_UNASSIGNED",
		"#FF_TEAM_SPECTATOR",
		"#FF_TEAM_BLUE",
		"#FF_TEAM_RED",
		"#FF_TEAM_YELLOW",
		"#FF_TEAM_GREEN"
	};


	// --------------------------------------------------------------------------------------------------- //
	// Global helper functions.
	// --------------------------------------------------------------------------------------------------- //

	// Helper function to parse arguments to player commands.
	const char* FindEngineArg( const char *pName )
	{
		int nArgs = engine->Cmd_Argc();
		for ( int i=1; i < nArgs; i++ )
		{
			if ( stricmp( engine->Cmd_Argv(i), pName ) == 0 )
				return (i+1) < nArgs ? engine->Cmd_Argv(i+1) : "";
		}
		return 0;
	}


	int FindEngineArgInt( const char *pName, int defaultVal )
	{
		const char *pVal = FindEngineArg( pName );
		if ( pVal )
			return atoi( pVal );
		else
			return defaultVal;
	}

	
	// World.cpp calls this but we don't use it in FF.
	void InitBodyQue()
	{
	}

	// --------------------------------------------------------------------------------------------------- //
	// CFFGameRules implementation.
	// --------------------------------------------------------------------------------------------------- //

	extern void ClearAllowedEffects();

	// --------------------------------------------------------------------------------
	// Purpose: Restarts the round in progress
	// --------------------------------------------------------------------------------
	void CC_FF_RestartRound( void )
	{
		if ( !UTIL_IsCommandIssuedByServerAdmin() )
		{
			Msg( "You must be a server admin to use ff_restartround\n" );
			return;
		}

		if( FFGameRules() )
		{
			if( engine->Cmd_Argc() > 1 )
			{
				float flTime = atof( engine->Cmd_Argv( 1 ) );

				// TODO: Check flTime a number?
				mp_prematch.SetValue( flTime / 60.0f );
			}
			else
			{
				// Default 15 seconds
				mp_prematch.SetValue( 15.0f / 60.0f );
			}

			FFGameRules()->RestartRound();
		}
		else
		{
			Warning( "Gamerules error!\n" );
		}
	}
	static ConCommand ff_restartround( "ff_restartround", CC_FF_RestartRound, "Restarts the round in progress." );

	// --> Mirv: Extra gamerules stuff
	CFFGameRules::CFFGameRules()
	{
		// Create the team managers
		for ( int i = 0; i < ARRAYSIZE( sTeamNames ); i++ )
		{
			// Use our team class with allies + class limits
			CFFTeam *pTeam = static_cast<CFFTeam*>(CreateEntityByName( "ff_team_manager" ));
			pTeam->Init( sTeamNames[i], i );

			g_Teams.AddToTail( pTeam );
		}

		// Init
		m_flRoundStarted = 0.0f;

		// Prematch system, game has not started
		m_flGameStarted = -1.0f;
		
		// Reset the effects timeouts
		ClearAllowedEffects();
	}

	void CFFGameRules::Precache()
	{
		m_flNextMsg = 0.0f;
		m_flGameStarted = -1.0f;
		m_flRoundStarted = gpGlobals->curtime;
		BaseClass::Precache();

#ifdef FF_BETA
		// Special stuff for beta!
		g_FFBetaList.Init();
#endif
	}
	// <-- Mirv: Extra gamerules stuff

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	CFFGameRules::~CFFGameRules()
	{
		// Note, don't delete each team since they are in the gEntList and will 
		// automatically be deleted from there, instead.
		g_Teams.Purge();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Player has just joined the game!
	// Purpose: called when a player tries to connect to the server
	// Input  : *pEdict - the new player
	//			char *pszName - the players name
	//			char *pszAddress - the IP address of the player
	//			reject - output - fill in with the reason why
	//			maxrejectlen -- sizeof output buffer
	//			the player was not allowed to connect.
	// Output : Returns TRUE if player is allowed to join, FALSE if connection is denied.
	//-----------------------------------------------------------------------------
	bool CFFGameRules::ClientConnected( edict_t *pEdict, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen )
	{
//#ifdef FF_BETA_TEST_COMPILE
//		// NO ONE CAN JOIN THE SERVER WHEN ITS A BETA TEST COMPILE!
//		// A beta test compile means this is the server.dll the 
//		// beta testers get so that the tables match up and so they
//		// can actually play with us. But, don't let them have the
//		// ability to host servers!
//		Q_snprintf( reject, maxrejectlen, "You are violating the terms\nof service for the beta" );
//		return false;
//#endif

//#ifdef FF_BETA
//		// Special stuff for beta!
//		if( !g_FFBetaList.IsValidName( pszName ) )
//		{
//			// Note: An extra period will be added onto this string so don't add
//			// one to make it proper or it will look stupid with "sentence.."!
//			Q_snprintf( reject, maxrejectlen, "You are not welcome in the\nFortress Forever beta test" );
//
//			// Sorry, buddy.
//			return false;
//		}		
//#endif

		CFFLuaSC func;
		func.Push( pszName );
		func.Push( pszAddress );
		func.Push( ENTINDEX( pEdict ) );
		if( func.CallFunction( "player_connected" ) )
		{
			if( !func.GetBool() )
			{
				// TODO: Retrieve a string from Lua with the rejection message
				Q_snprintf( reject, maxrejectlen, "Lua said you are not welcome!" );

				return false;
			}
		}

		return BaseClass::ClientConnected( pEdict, pszName, pszAddress, reject, maxrejectlen );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Player has just left the game
	//-----------------------------------------------------------------------------
	void CFFGameRules::ClientDisconnected( edict_t *pClient )
	{
		CFFPlayer *pPlayer = ToFFPlayer( CBaseEntity::Instance( pClient ) );
		if( pPlayer )
		{
			CFFLuaSC func( 1, pPlayer );
			func.CallFunction( "player_disconnected" );

			pPlayer->RemoveProjectiles();
			pPlayer->RemoveBackpacks();
			pPlayer->RemoveBuildables();

			pPlayer->SetObjectiveEntity(NULL);
			int iObjectivePlayerRefs = pPlayer->m_ObjectivePlayerRefs.Count();
			for ( int i = 0; i < iObjectivePlayerRefs; i++ )
			{
				CFFPlayer *pRefPlayer = ToFFPlayer( pPlayer->m_ObjectivePlayerRefs.Element(i) );
				pRefPlayer->SetObjectiveEntity(NULL);
			}

			// TODO: Possibly loop through and find CLASS_INFOSCRIPTS
			// and tell them OnOwnerDied()?
			CBaseEntity *pEntity = gEntList.FindEntityByOwnerAndClassT( NULL, ( CBaseEntity * )pPlayer, CLASS_INFOSCRIPT );
			while( pEntity )
			{
				CFFInfoScript *pInfoScript = static_cast< CFFInfoScript * >( pEntity );
				if( pInfoScript )
					pInfoScript->OnOwnerDied( ( CBaseEntity * )pPlayer );

				pEntity = gEntList.FindEntityByOwnerAndClassT( pEntity, ( CBaseEntity * )pPlayer, CLASS_INFOSCRIPT );
			}
		}

		// Chain on down, I'm in the chain gang, mang. What? I
		// typed way too much in this function. Just stop.
		BaseClass::ClientDisconnected( pClient );
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CFFGameRules::LevelShutdown( void )
	{
#ifdef FF_BETA
		// Special stuff for beta!
		g_FFBetaList.Shutdown();
#endif

		BaseClass::LevelShutdown();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void CFFGameRules::RestartRound( void )
	{
		// Tell gamerules the round JUST started
		Precache();

		bool bFlags[ AT_MAX_FLAG ] = { true };

		// Reset entities
		ResetUsingCriteria( bFlags, TEAM_UNASSIGNED, NULL, true );		
	}

	//-----------------------------------------------------------------------------
	// Purpose: Reset certain aspects of the game based on criteria
	//-----------------------------------------------------------------------------
	void CFFGameRules::ResetUsingCriteria( bool *pbFlags, int iTeam, CFFPlayer *pFFPlayer, bool bFullReset )
	{
		// pbFlags are the criteria used during the reset
		// bFullReset - everything reset. Just like the server was restarted.

#ifdef _DEBUG
		Assert( pbFlags );
#endif

		// Full map reset, really only used w/ ff_restartround as it restarts
		// absolutely everything - scores, players, entities, etc.		
		if( bFullReset )
		{
			// TODO: Do stuff!

			m_flIntermissionEndTime = 0.f;

			// Kill entity system helper
			UTIL_Remove( CFFEntitySystemHelper::GetInstance() );

			// Clear delete list
			gEntList.CleanupDeleteList();

			// Shutdown schedule manager
			_scheduleman.Shutdown();
			// Start schedule manager
			_scheduleman.Init();
			
			// Shutdown timer manager
			_timerman.Shutdown();
			// Start timer manager
			_timerman.Init();

			// Re-start entsys for the map
			_scriptman.LevelInit(STRING(gpGlobals->mapname));

			// Mulch: 9/6/2007: Old code
			//// Go through and delete entities
			//CBaseEntity *pEntity = gEntList.FirstEnt();
			//while( pEntity )
			//{
			//	if( m_hMapFilter.ShouldCreateEntity( pEntity->GetClassname() ) )
			//	{
			//		// Grab the next ent
			//		CBaseEntity *pTemp = gEntList.NextEnt( pEntity );

			//		// Delete current ent
			//		UTIL_Remove( pEntity );

			//		// Set up current ent again
			//		pEntity = pTemp;
			//	}
			//	else
			//	{
			//		pEntity = gEntList.NextEnt( pEntity );
			//	}
			//}

			//// Clear anything that's been deleted
			//gEntList.CleanupDeleteList();

			//// RAWR! Add all entities back
			//MapEntity_ParseAllEntities( engine->GetMapEntitiesString(), &m_hMapFilter, true );

			// Mulch: 9/6/2007: New code per: http://developer.valvesoftware.com/wiki/Resetting_Maps_and_Entities
			
			// Recreate all the map entities from the map data (preserving their indices),
			// then remove everything else except the players.

			// Get rid of all entities except players.
			CBaseEntity *pCur = gEntList.FirstEnt();
			while( pCur )
			{
				if( !FindInList( g_MapEntityFilterKeepList, pCur->GetClassname() ) )
				{
					CBaseEntity *pTemp = gEntList.NextEnt( pCur );
					UTIL_Remove( pCur );
					pCur = pTemp;
				}
				else
				{
					pCur = gEntList.NextEnt( pCur );
				}
			}

			// Really remove the entities so we can have access to their slots below.
			gEntList.CleanupDeleteList();

			CFFMapEntityFilter filter;
			filter.m_iIterator = g_MapEntityRefs.Head();

			// final task, trigger the recreation of any entities that need it.
			MapEntity_ParseAllEntities( engine->GetMapEntitiesString(), &filter, true );

			// Run startup stuff again!
			CFFLuaSC hStartup;
			_scriptman.RunPredicates_LUA(NULL, &hStartup, "startup");

			// update the list of valid spawn points
			UpdateSpawnPoints();

			// Respawn/Reset all players
			for( int i = 1; i <= gpGlobals->maxClients; i++ )
			{
				CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );
				if( pPlayer )
				{
					// Going to be deleting the last spawn entity,
					// so make sure the player knows it doesn't exist anymore!
					pPlayer->SetLastSpawn( NULL );

					if( FF_IsPlayerSpec( pPlayer ) )
						continue;

					// Do some cleanup
					pPlayer->PreForceSpawn();
					/*
					// Attempt to stop weapon sound since we're forcibly respawning
					if( pPlayer->GetActiveFFWeapon() )
						pPlayer->GetActiveFFWeapon()->WeaponSound( STOP );
						*/

					// Jiggles: ff_restartround didn't reset the m_bDisguisable flag
					//			which meant that Spies holding a flag when the restart was called
					//			weren't able to disguise until they grabbed and dropped another flag!
					pPlayer->SetDisguisable( true );
					// End fix
					pPlayer->Spawn();
					pPlayer->ResetFragCount();
					pPlayer->ResetFortPointsCount();
					pPlayer->ResetDeathCount();
				}
			}

			// Reset all team scores & deaths. Do it here
			// after we've killed/spawned players.
			for( int i = 0; i < GetNumberOfTeams(); i++ )
			{
				CTeam *pTeam = GetGlobalTeam( i );

				if( !pTeam )
					continue;

				pTeam->SetScore( 0 );
				pTeam->SetFortPoints( 0 );
				pTeam->SetDeaths( 0 );
			}

			// Send event
			IGameEvent *pEvent = gameeventmanager->CreateEvent( "ff_restartround" );
			if( pEvent )
			{
				pEvent->SetFloat( "curtime", gpGlobals->curtime );
				gameeventmanager->FireEvent( pEvent );
			}
		}
		else
		{
			// For use later
			CUtlVector< int > iChangeClassValidClasses;

			bool bUseTeam = ( ( iTeam >= TEAM_BLUE ) && ( iTeam <= TEAM_GREEN ) );
			bool bUsePlayer = pFFPlayer ? true : false;

			// Eh? Which one do we use? 
			if( bUseTeam && bUsePlayer )
			{
				// We'll go with player since it took more params to get there
				bUseTeam = false;
			}

			// Sum up the number of changeclass flags set, if any
			for( int i = AT_CHANGECLASS_SCOUT; i < AT_CHANGECLASS_RANDOM; i++ )
			{
				// TODO: No support for the random flag right now

				if( pbFlags[ i ] )
					iChangeClassValidClasses.AddToTail( i - AT_CHANGECLASS_SCOUT + 1 );
			}

			// Loop through all players
			for( int i = 1; i <= gpGlobals->maxClients; i++ )
			{	
				CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );
				if( pPlayer && pPlayer->IsPlayer() )
				{
					// Skip spectators
					if( FF_IsPlayerSpec( pPlayer ) )
						continue;

					// If bUseTeam, meaning we were sent in a valid team...
					if( bUseTeam )
					{
						// Then filter out players not on team iTeam
						if( pPlayer->GetTeamNumber() != iTeam )
							continue;
					}

					// If we're acting on one player
					if( bUsePlayer )
					{
						// TODO: Move this so we don't enter the loop
						// if bUsePlayer is true (no need to waste time
						// with UTIL_PlayerByIndex since we've already
						// got a player pointer!
						if( pPlayer != pFFPlayer )
							continue;
					}				

					// Please don't change the order. They're set up hopefully
					// to work correctly.

					// 1 or more changeclass flags was set
					if( iChangeClassValidClasses.Count() > 0 )
					{
						// Pick a class from the range available
						pPlayer->InstaSwitch( iChangeClassValidClasses[random->RandomInt( 0, iChangeClassValidClasses.Count() - 1 )] );
					}

					if( pbFlags[ AT_CHANGETEAM_BLUE ] )
					{
						pPlayer->ChangeTeam( FF_TEAM_BLUE );
					}

					if( pbFlags[ AT_CHANGETEAM_RED ] )
					{
						pPlayer->ChangeTeam( FF_TEAM_RED );
					}

					if( pbFlags[ AT_CHANGETEAM_YELLOW ] )
					{
						pPlayer->ChangeTeam( FF_TEAM_YELLOW );
					}

					if( pbFlags[ AT_CHANGETEAM_GREEN ] )
					{
						pPlayer->ChangeTeam( FF_TEAM_GREEN );
					}

					if( pbFlags[ AT_CHANGETEAM_SPEC ] )
					{
						pPlayer->ChangeTeam( FF_TEAM_SPEC );
					}

					if( pbFlags[ AT_DROP_ITEMS ] || pbFlags[ AT_THROW_ITEMS ] )
					{
						// Don't do anything...
						// ownerdie will get called if RS_KILL_PLAYERS is set and that
						// will handle whether or not the items a player is carrying
						// get thrown or dropped

						// NOTE: this is a useless flag I think...
					}

					if( pbFlags[ AT_FORCE_DROP_ITEMS ] || pbFlags[ AT_FORCE_THROW_ITEMS ] )
					{
						// TODO: iterate through getting this players' items
						// and make them be dropped before killing the player

						CBaseEntity *pEntity = gEntList.FindEntityByOwnerAndClassT( NULL, ( CBaseEntity * )pPlayer, CLASS_INFOSCRIPT );
						while( pEntity )
						{
							CFFInfoScript *pFFScript = dynamic_cast< CFFInfoScript * >( pEntity );
							if( pFFScript )
							{
								// TODO: Need to make a lua call to get the delay and throw speed
								float flDelay = 10.0f;
								float flSpeed = 0.0f;
								pFFScript->Drop( flDelay, flSpeed );
							}

							pEntity = gEntList.FindEntityByOwnerAndClassT( pEntity, ( CBaseEntity * )pPlayer, CLASS_INFOSCRIPT );
						}
					}

					// Do this before killing the player and before respawning. This is
					// for when we're doing a force respawn without killing the player.
					// In this type of situation objects are never asked if they should
					// be dropped from the player or not so we could end up in a situation
					// where a player has a flag and another team does something to trigger
					// ApplyToAll/Team/Player to get called and now we respawn with a flag
					// and we're not supposed to.
					if( pbFlags[ AT_RESPAWN_PLAYERS ] && !pbFlags[ AT_KILL_PLAYERS ] )
					{
						// Iterate through objects this player has and ask lua object what
						// it should do.
						CBaseEntity *pEntity = gEntList.FindEntityByOwnerAndClassT( NULL, ( CBaseEntity * )pPlayer, CLASS_INFOSCRIPT );

						while( pEntity )
						{
							CFFInfoScript *pFFScript = dynamic_cast< CFFInfoScript * >( pEntity );
							if( pFFScript )
							{
								// Yes, this is redundant since we're searching by owner & class_t
								// so we already know this guy is the owner of this info_ff_script.
								pFFScript->OnOwnerForceRespawn( ( CBaseEntity * )pPlayer );
							}

							pEntity = gEntList.FindEntityByOwnerAndClassT( pEntity, ( CBaseEntity * )pPlayer, CLASS_INFOSCRIPT );
						}
					}

					if( pbFlags[ AT_RETURN_CARRIED_ITEMS ] )
					{
						// Eh? This is dumb. Needs a "FORCE". We're not going to forcibly
						// return a carried item. If the player is killed, ownerdie handles
						// that. The only case left is a player being respawned and not
						// killed and for that you might want to do something
					}

					if( pbFlags[ AT_RETURN_DROPPED_ITEMS ] )
					{
						// TODO: do this globally - not on each player's iteration
					}

					if( pbFlags[ AT_STOP_PRIMED_GRENS ] )
					{
						pPlayer->RemovePrimedGrenades();
					}

					if( pbFlags[ AT_RELOAD_CLIPS ] && !pbFlags[ AT_KILL_PLAYERS ] && !pbFlags[ AT_RESPAWN_PLAYERS ] )
					{
						// No sense is doing this if we're kiling the player or forcibly spawning
						// as the with the former the guy is dying and thus respawns will full clips
						// and with the latter spawn will get it done
						pPlayer->ReloadClips();
					}

					if( pbFlags[ AT_ALLOW_RESPAWN ] )
					{
						pPlayer->SetRespawnable( true );
					}

					if( pbFlags[ AT_DISALLOW_RESPAWN ] )
					{
						pPlayer->SetRespawnable( false );
					}

					if( pbFlags[ AT_KILL_PLAYERS ] )
					{
						pPlayer->KillPlayer();
					}

					if( pbFlags[ AT_RESPAWN_PLAYERS ] )
					{
						// Do some cleanup (only if we didn't kill the guy)
						if( !pbFlags[ AT_KILL_PLAYERS ] )
							pPlayer->PreForceSpawn();

						/*
						// Attempt to stop weapon sound since we're forcibly respawning
						if( pPlayer->GetActiveFFWeapon() )
							pPlayer->GetActiveFFWeapon()->WeaponSound( STOP );
							*/

						pPlayer->Spawn();
					}

					if( pbFlags[ AT_REMOVE_RAGDOLLS ] )
					{						
					}

					if( pbFlags[ AT_REMOVE_PACKS ] )
					{
						pPlayer->RemoveBackpacks();
					}

					if( pbFlags[ AT_REMOVE_PROJECTILES ] )
					{
						pPlayer->RemoveProjectiles();
					}

					if( pbFlags[ AT_REMOVE_BUILDABLES ] )
					{
						pPlayer->RemoveBuildables();
					}

					if( pbFlags[ AT_REMOVE_DECALS ] )
					{
						engine->ClientCommand( pPlayer->edict(), "r_cleardecals" );
					}					
				}
			}

			if( pbFlags[ AT_RETURN_DROPPED_ITEMS ] )
			{
				CBaseEntity *pEntity = gEntList.FindEntityByClassT( NULL, CLASS_INFOSCRIPT );
				while( pEntity )
				{
					CFFInfoScript *pFFScript = dynamic_cast< CFFInfoScript * >( pEntity );
					if( pFFScript && pFFScript->IsDropped() )
					{
						pFFScript->ForceReturn();
					}

					pEntity = gEntList.FindEntityByClassT( pEntity, CLASS_INFOSCRIPT );
				}
			}
		}

		if( pbFlags[ AT_END_MAP ] )
		{
			GoToIntermission();
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: creates a list of valid spawn points
	//-----------------------------------------------------------------------------
	void CFFGameRules::UpdateSpawnPoints()
	{
		// start from scratch every time this function is called
		m_SpawnPoints.Purge();

		CBaseEntity	*pEntity = NULL;
		// Add all the entities with the matching class type
		while ( (pEntity = gEntList.FindEntityByClassT( pEntity, CLASS_TEAMSPAWN )) != NULL )
		{
			// See if lua says the spawn point is inactive
			CFFLuaSC hIsInactive;
			_scriptman.RunPredicates_LUA( pEntity, &hIsInactive, "isinactive" );

			// add this spawn point to the list oif valid spawns
			if (!hIsInactive.GetBool())
				m_SpawnPoints.AddToTail( pEntity );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Checks to see if a spawn point is clear
	//-----------------------------------------------------------------------------
	bool CFFGameRules::IsSpawnPointClear( CBaseEntity *pSpot, CBasePlayer *pPlayer )
	{
		if( !pSpot )
			return false;

		if( !pPlayer )
			return false;

		CFFPlayer *pFFPlayer = ToFFPlayer( pPlayer );
		if( !pFFPlayer )
			return false;

		CBaseEntity *pList[ 128 ];
		int count = UTIL_EntitiesInBox( pList, 128, pSpot->GetAbsOrigin() - Vector( 16, 16, 0 ), pSpot->GetAbsOrigin() + Vector( 16, 16, 72 ), FL_CLIENT | FL_NPC | FL_FAKECLIENT );
		if( count )
		{
			// Iterate through the list and check the results
			for( int i = 0; i < count; i++ )
			{
				CBaseEntity *ent = pList[ i ];
				if( ent )
				{
					if( ent->IsPlayer() )
					{
						if( ( ( CBasePlayer * )ent != pPlayer ) && ent->IsAlive() )
							return false;
					}
					else if( FF_IsBuildableObject( ent ) )
					{
						return false;
					}
				}
			}
		}

		return true;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Checks if the spawn point is valid
	//-----------------------------------------------------------------------------
	bool CFFGameRules::IsSpawnPointValid( CBaseEntity *pSpot, CBasePlayer *pPlayer )
	{
		if( !pSpot )
			return false;

		if( pSpot->GetLocalOrigin() == vec3_origin )
			return false;

		if( !pPlayer )
			return false;

		CFFPlayer *pFFPlayer = ToFFPlayer( pPlayer );
		if( !pFFPlayer )
			return false;		

		// Check if lua lets us spawn here			
		CFFLuaSC hAllowed;
		hAllowed.Push( pFFPlayer );
		if( _scriptman.RunPredicates_LUA( pSpot, &hAllowed, "validspawn" ) )
		{
			// Spot is a valid place for us to spawn
			if( hAllowed.GetBool() )
			{
				return true;
			}
		}

		// info_player_start's are valid spawns... for now
		if( FClassnameIs( pSpot, "info_player_start" ) )
			return true;

		return false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Checks if the player can respawn or not
	//-----------------------------------------------------------------------------
	bool CFFGameRules::FPlayerCanRespawn( CBasePlayer *pPlayer )
	{
		CFFPlayer *pFFPlayer = ToFFPlayer( pPlayer );
		if( !pFFPlayer )
			return false;

		if( !pFFPlayer->IsRespawnable() )
			return false;
		
		return BaseClass::FPlayerCanRespawn( pPlayer );
	}

	//-----------------------------------------------------------------------------
	// Purpose: TF2 Specific Client Commands
	// Input  :
	// Output :
	//-----------------------------------------------------------------------------
	bool CFFGameRules::ClientCommand( const char *pcmd, CBaseEntity *pEdict )
	{
		if(g_pPlayerCommands && g_pPlayerCommands->ProcessCommand(pcmd, (CFFPlayer*)pEdict))
			return true;

		// --> Mirv: More commands
		CFFPlayer *pPlayer = (CFFPlayer *) pEdict;

		if (pPlayer && pPlayer->ClientCommand(pcmd))
			return true;
		// <-- Mirv: More commands

		return BaseClass::ClientCommand( pcmd, pEdict );
	}

	// Uh.. we don't want that screen fade crap
	bool CFFGameRules::FlPlayerFallDeathDoesScreenFade( CBasePlayer *pPlayer )
	{
		return false;
	}

	//=========================================================
	// reduce fall damage
	//=========================================================
	float CFFGameRules::FlPlayerFallDamage(CBasePlayer *pPlayer)
	{
		CFFPlayer *pFFPlayer = ToFFPlayer(pPlayer);

		// This is bad
		if (!pFFPlayer)
		{
			DevWarning("Fall damage on non-CFFPlayer!");
			return 9999;
		}

		// Jiggles: Scouts don't take fall damage; they're just cool like that 
		// AfterShock: commenting this for a while in an attempt to stop 4x scout offense everywhere
		//if ( pFFPlayer->GetClassSlot() == CLASS_SCOUT )
		//	return 0;

		float flMaxSafe = PLAYER_MAX_SAFE_FALL_SPEED;

		bool bIsSpy = (pFFPlayer->GetClassSlot() == 8);

		// Spy can fall twice as far without hurting
		if (bIsSpy)
		{
			flMaxSafe *= 1.412;
		}

		// Escape if they shouldn't be taking damage.
		if (pPlayer->m_Local.m_flFallVelocity < flMaxSafe)
		{
			return 0;
		}

		// Speed is a good approximation for now of a class's weight
		// Therefore bigger base damage for slower classes
		float weightratio = clamp((pPlayer->MaxSpeed() - 230.0f) / 170.0f, 0, 1.0f);
		float flBaseDmg = 6.0f + (1.0f - weightratio) * 6.0f;

		// Don't worry this'll be optimised!
		float speedratio = clamp((pPlayer->m_Local.m_flFallVelocity - flMaxSafe) / (PLAYER_FATAL_FALL_SPEED - flMaxSafe), 0, 1.0f);
		float flDmg = flBaseDmg + speedratio * flBaseDmg;

		// Spies only take half damage too
		if ((bIsSpy) || (pFFPlayer->GetClassSlot() == 1))
		{
			flDmg *= 0.5f;
		}
		
		return flDmg;
	} 

	//------------------------------------------------------------------------
	// Purpose: Wow, so TFC's radius damage is not as similar to Half-Life's
	//			as we thought it was. Everything has a falloff of .5 for a start.
	//
	//			Explosions always happen at least 32units above the ground, except
	//			for concs. Rockets will also pull back 32 units from the normal
	//			of any walls.
	//
	//			You only cause 2/3 total damage on yourself.
	//
	//			The force (or change in v) is always 8x the total damage.
	//------------------------------------------------------------------------
	void CFFGameRules::RadiusDamage(const CTakeDamageInfo &info, const Vector &vecSrcIn, float flRadius, int iClassIgnore, CBaseEntity *pEntityIgnore)
	{
		CBaseEntity *pEntity = NULL;
		trace_t		tr;
		float		flAdjustedDamage, falloff;
		Vector		vecSpot;

		// Because we modify this later
		Vector		vecSrc = vecSrcIn;

#ifdef GAME_DLL
		//NDebugOverlay::Cross3D(vecSrc, 8.0f, 255, 0, 0, true, 5.0f);
#endif

		// TFC style falloff please.
		falloff = 0.5f; // AfterShock: need to change this if you want to have a radius over 2x the damage

		// Always raise by 1.0f
		// It's so that the grenade isn't in the ground
		// AfterShock: Isn't this going to raise grenades into the ceiling if they explode there? Shouldnt we be raising off the surface normal instead?
		vecSrc.z += 1.0f;

		// iterate on all entities in the vicinity.
		for (CEntitySphereQuery sphere(vecSrc, flRadius); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity()) 
		{
			if (pEntity == pEntityIgnore) 
				continue;

			if (pEntity->m_takedamage == DAMAGE_NO) 
				continue;

			// Is this a buildable of some sort
			CFFBuildableObject *pBuildable = dynamic_cast <CFFBuildableObject *> (info.GetInflictor());

			// Skip objects that are building
			if(pBuildable && !pBuildable->IsBuilt())
				continue;

#ifdef GAME_DLL
			//NDebugOverlay::EntityBounds(pEntity, 0, 0, 255, 100, 5.0f);
#endif

			// Check that the explosion can 'see' this entity.
			// TFC also uses a noisy bodytarget
			vecSpot = pEntity->BodyTarget(vecSrc, true);

			// Lets calculate some values for this grenade and player
			Vector vecDisplacement	= (vecSpot - vecSrc);
			float flDistance		= vecDisplacement.Length();
			Vector vecDirection		= vecDisplacement / flDistance;

#ifdef USE_HITBOX_HACK
			// Because our models are pretty weird, the tracelines don't work
			// as expected. So instead we use this awful little hack here. Thanks
			// modellers!
			if (pEntity->IsPlayer())
			{
				CFFPlayer *pPlayer = ToFFPlayer(pEntity);
				float flBodyTargetOffset = vecSpot.z - pPlayer->GetLegacyAbsOrigin().z;

                float dH = vecDisplacement.Length2D() - 16.0f;	// Half of model width
				float dV = fabs(vecDisplacement.z - flBodyTargetOffset) - 36.0f; // Half of model height

				// Inside our model bounds
				if (dH <= 0.0f && dV <= 0.0f)
				{
					flDistance = 0.0f;
				}
				// dH must be positive at this point (dbz safe)
				else if (dH > dV)// if target is less than 45 degrees i.e more horizontal to the grenade than vertical
				{
					flDistance *= dH / (dH + 16.0f); // this just reduces distance by an amount equivalent to 16 in horizontal (so min 16)
				}
				// dV must be positive at this point (as must vecDisplacement.z, thus (dbz safe))
				else
				{
					// 0001457: Throwing grenades vertically causes more damage
					// Dividing positive dV (yeah I know this entire thing is a AWFUL HACK) by negative displacement
					// was resulting in adding damage for falloff instead of subtracting later on.
					flDistance *= dV / fabs(vecDisplacement.z); 

					if ( dH > 0.0f) // AfterShock: make sure distance calculated is always at least distance from explosion to closest corner of bounding box 
						// (fixes bug at around 50 degrees vertically where explosions were doing too much damage)
					{
						float flDistance2 = Vector(dH, 0, dV).Length();

						if (flDistance2 > flDistance)
							flDistance = flDistance2;
					}
				}

				// Another quick fix for the movement code this time
				// This should be fixed in the movement code eventually but that
				// might be a bigger job if it breaks trimping or something.
				if (pEntity->GetGroundEntity())
				{
					Vector vecVelocity = pEntity->GetAbsVelocity();

					if (vecVelocity.z < 0.0f)
					{
						vecVelocity.z = 0;
						pEntity->SetAbsVelocity(vecVelocity);
					}
				}
			}
			else
			{
				DevMsg("NOT A PLAYER !! \n");
			}
#endif

			// Our grenades are set up so that they have the flag FL_GRENADE. So, we can do this:
			// Grenades inside each other end up not dealing out damamge cause their traces get
			// blocked! So, use a trace filter to ignore other grenades if this is a grenade that
			// is trying to deal out damage to pEntity!
			// Bug #0001003: Grenades include projectiles in LOS collision check?
			if( info.GetInflictor() && ( ( info.GetInflictor() )->GetFlags() & FL_GRENADE ) )
			{
				CTraceFilterIgnoreSingleFlag traceFilter( FL_GRENADE );
				UTIL_TraceLine( vecSrc, vecSpot, MASK_SHOT, &traceFilter, &tr );
				
				// Jiggles: This is the case where an EMP triggered a backpack to explode.
				//			We don't want the backpack to block the trace, so let's trace again ignoring it
				if ( tr.fraction == 0.0 && tr.m_pEnt && tr.m_pEnt->Classify() == CLASS_BACKPACK )
				{
					CTraceFilterSimple passBackpack( tr.m_pEnt, COLLISION_GROUP_NONE );
					UTIL_TraceLine( vecSrc, vecSpot, MASK_SHOT, &passBackpack, &tr );
				}
			}
			else
				UTIL_TraceLine(vecSrc, vecSpot, MASK_SHOT, info.GetInflictor(), COLLISION_GROUP_NONE, &tr);

#ifdef GAME_DLL
			//NDebugOverlay::Line(vecSrc, vecSpot, 0, 255, 0, true, 5.0f);

			//Vector v = vecSpot - vecSrc;
			//v *= flDistance / v.Length();
			//NDebugOverlay::Line(vecSrc, vecSrc + v, 200, 200, 0, true, 5.0f);
#endif

			// Could not see this entity, so don't hurt it
			if (tr.fraction != 1.0 && tr.m_pEnt != pEntity) 
				continue;

			float flBaseDamage = info.GetDamage();

			// Decrease damage for an ent that's farther from the explosion
			flAdjustedDamage = flBaseDamage - (flDistance * falloff); // AfterShock: this means if a player is on the radius of 2x the base damage, you'll do 0 damage
			// We're doing no damage, so don't do anything else here
			if (flAdjustedDamage <= 0) 
				continue;

			// In TFC players only do 2/3 damage to themselves
			// This also affects forces by the same amount
			// Added: Make sure the source isn't a buildable though
			// as that should do full damage!
			if (pEntity == info.GetAttacker() && !pBuildable)
				flAdjustedDamage *= 0.66666f;

			// If we're stuck inside them, fixup the position and distance
			// I'm assuming this is done in TFC too
			if (tr.startsolid) 
			{
				tr.endpos = vecSrc;
				tr.fraction = 0.0;
			}

			// Create a new TakeDamageInfo for this player
			CTakeDamageInfo adjustedInfo = info;

			// Set the new adjusted damage
			adjustedInfo.SetDamage(flAdjustedDamage);

			// Don't calculate the forces if we already have them
			if (adjustedInfo.GetDamagePosition() == vec3_origin || adjustedInfo.GetDamageForce() == vec3_origin) 
			{
				// Multiply the damage by 8.0f (ala tfc) to get the force
				// 0000936 - use convar; ensure a lower "bounds"
				float flCalculatedForce = flAdjustedDamage * PUSH_MULTIPLIER;
				
				CBaseEntity *pInflictor = info.GetInflictor();
				// If this is an IC projectile, set to a lower clamp (350)
				float flPushClamp = PUSH_CLAMP;
				
				if ( pInflictor )
				{
					switch ( pInflictor->Classify() )
					{
						case CLASS_IC_ROCKET:
							flPushClamp = 350.0f;
							// lower the push because of the increased damage needed
							flCalculatedForce /= 3;
							if (pEntity == info.GetAttacker() && !pBuildable)
							{
								flAdjustedDamage *= IC_SELFDAMAGEMULTIPLIER;
								adjustedInfo.SetDamage(flAdjustedDamage);
							}
							break;

						case CLASS_RAIL_PROJECTILE:
							flPushClamp = 300.0f;
							// Don't want people jumpin' real high with the Rail Gun :)
							flCalculatedForce /= 3;
							break;

						case CLASS_GREN_EMP:
							if (flCalculatedForce > 700.0f )
								flCalculatedForce = 700.0f;
							break;
					}
				}	

				if (flCalculatedForce < flPushClamp)
					flCalculatedForce = flPushClamp;

				CFFPlayer *pPlayer = NULL;

				if( pEntity->IsPlayer() )
					pPlayer = ToFFPlayer(pEntity);

				// We have to reduce the force further if they're fat
				// 0000936 - use convar
				if (pPlayer && pPlayer->GetClassSlot() == CLASS_HWGUY) 
					flCalculatedForce *= FATTYPUSH_MULTIPLIER;

				//CFFPlayer *pAttacker = NULL;
				CBaseEntity *pAttacker = info.GetAttacker();

				/*
				// If it's a building then take it's owner
				if( ( info.GetAttacker()->Classify() == CLASS_DISPENSER ) ||
					( info.GetAttacker()->Classify() == CLASS_SENTRYGUN ) ||
					( info.GetAttacker()->Classify() == CLASS_DISPENSER ) )
				{
					pAttacker = ToFFPlayer( ( ( CFFBuildableObject * )info.GetAttacker() )->m_hOwner.Get() );
				}
				else
                    pAttacker = ToFFPlayer( info.GetAttacker() );

				*/
                
				// And also reduce if we couldn't hurt them
				// TODO: Get exact figure for this
				// 0000936 - use convar
				if (pPlayer && pAttacker && !g_pGameRules->FCanTakeDamage(pPlayer, pAttacker))
					flCalculatedForce *= NODAMAGEPUSH_MULTIPLIER;

				// Don't use the damage source direction, use the reported position
				// if it exists
				if (adjustedInfo.GetReportedPosition() != vec3_origin)
				{
                    vecDirection = vecSpot - adjustedInfo.GetReportedPosition();
					vecDirection.NormalizeInPlace();

#ifdef GAME_DLL
					//NDebugOverlay::Line(adjustedInfo.GetReportedPosition(), vecSpot, 0, 255, 0, true, 5.0f);
					//NDebugOverlay::Cross3D(adjustedInfo.GetReportedPosition(), 5.0f, 0, 255, 0, true, 5.0f);
#endif
				}

				// Now set all our calculated values
				adjustedInfo.SetDamageForce(vecDirection * flCalculatedForce);
				adjustedInfo.SetDamagePosition(vecSrc);
			}

			// Now deal the damage
			if (pEntity->IsPlayer())
			{
				pEntity->TakeDamage(adjustedInfo);
			}
			else
			{
				adjustedInfo.ScaleDamageForce(100.0f);
				pEntity->DispatchTraceAttack(adjustedInfo, vecDirection, &tr);
				ApplyMultiDamage();
			}
			

			// For the moment we'll play blood effects if its a teammate too so its consistant with other weapons
			// NOT!  Adding no blood for teammates when FF is off. -> Defrag
			if (pEntity->IsPlayer() && g_pGameRules->FCanTakeDamage(ToFFPlayer(pEntity), info.GetAttacker())) 
			{
				// Bug #0000539: Blood decals are projected onto shit
				// (direction needed normalising)
				Vector vecTraceDir = (tr.endpos - tr.startpos);
				VectorNormalize(vecTraceDir);

				// Bug #0000168: Blood sprites for damage on players do not display
				SpawnBlood(tr.endpos, vecTraceDir, pEntity->BloodColor(), adjustedInfo.GetDamage() * 3.0f);

				pEntity->TraceBleed(adjustedInfo.GetDamage(), vecTraceDir, &tr, adjustedInfo.GetDamageType());
			}

			// Now hit all triggers along the way that respond to damage... 
			pEntity->TraceAttackToTriggers(adjustedInfo, vecSrc, tr.endpos, vecDirection);
		}
	}

	// --> Mirv: Hodgepodge of different checks (from the base functions) inc. prematch
	void CFFGameRules::Think()
	{
#ifdef FF_BETA_TEST_COMPILE
		// Crash!
		CFFPlayer *p = NULL;
		p->Spawn();
#endif

#ifndef FF_BETA_TEST_COMPILE

#ifdef FF_BETA
		// Special stuff for beta!
		g_FFBetaList.Validate();
#endif

		// Lots of these depend on the game being started
		if( !HasGameStarted() )
		{
			float flPrematch = m_flRoundStarted + mp_prematch.GetFloat() * 60;

			// We should have started now, lets go!
			if( gpGlobals->curtime > flPrematch )
			{
				StartGame();
			}
			else
			{
				// Only send message every second (not every frame)
				if( gpGlobals->curtime > m_flNextMsg )
				{
					m_flNextMsg = gpGlobals->curtime + 1.0f;

					char sztimeleft[10];

					float flTimeLeft = ( int )( flPrematch - gpGlobals->curtime + 1 );
					if( flTimeLeft > 59 )
					{
						int iMinutes = ( int )( flTimeLeft / 60.0f );
						float flSeconds = ( float )( ( ( flTimeLeft / 60.0f ) - ( float )iMinutes ) * 60.0f );

						Q_snprintf( sztimeleft, sizeof(sztimeleft), "%d:%02.0f", iMinutes, flSeconds );
					}
					else
					{
						Q_snprintf( sztimeleft, sizeof(sztimeleft), "%d", ( int )flTimeLeft );
					}

					UTIL_ClientPrintAll( HUD_PRINTCENTER, "#FF_PREMATCH", sztimeleft );
				}
			}
		}
		else
		{
			if(!g_Disable_Timelimit)
			{
				float flTimeLimit = mp_timelimit.GetFloat() * 60;

				// Changelevel after intermission
				if (g_fGameOver && gpGlobals->curtime > m_flIntermissionEndTime)
				{
					ChangeLevel();
					return;
				}

				// Catch the end of the map
				if ( flTimeLimit != 0 && gpGlobals->curtime >= flTimeLimit + m_flGameStarted )
				{
					GoToIntermission(); //ChangeLevel();
					return;
				}
			}
		}
		
		GetVoiceGameMgr()->Update( gpGlobals->frametime );
#endif // FF_BETA_TEST_COMPILE
	}
	// <-- Mirv: Hodgepodge of different checks (from the base functions) inc. prematch

	void CFFGameRules::BuildableKilled( CFFBuildableObject *pObject, const CTakeDamageInfo& info )
	{
		const char *pszWeapon = "world";
		int iKillerID = 0;
		int iKilledSGLevel = 0;
		int iKillerSGLevel = 0;

		// Find the killer & the scorer
		CBaseEntity *pInflictor = info.GetInflictor();
		CBaseEntity *pKiller = info.GetAttacker();

		// Jiggles: Maybe not the best spot to put this, but...
		// If the gun killed someone while in malicious sabotage mode
		// we want to give credit to the Spy who did it
		CFFBuildableObject *pSabotagedBuildable = CFFBuildableObject::AttackerInflictorBuildable(pKiller, pInflictor);
		if ( pSabotagedBuildable )
		{
			if ( pSabotagedBuildable->IsMaliciouslySabotaged() )
				pKiller = pSabotagedBuildable->m_hSaboteur;
		}

		// get level of SG if the killer is an SG
		CFFBuildableObject *pBuildable = CFFBuildableObject::AttackerInflictorBuildable(pKiller, pInflictor);
		if ( pBuildable )
		{
			if ( pBuildable->Classify() == CLASS_SENTRYGUN )
			{
				CFFSentryGun *pSentryGun = FF_ToSentrygun( pBuildable );
				if (pSentryGun->GetLevel() == 1)
					iKillerSGLevel = 1;
				else if (pSentryGun->GetLevel() == 2)
					iKillerSGLevel = 2;
				else if (pSentryGun->GetLevel() == 3)
					iKillerSGLevel = 3;
				else 
					DevMsg( "Unknown SG level :(" );
			}
		}

		CBasePlayer *pScorer = pScorer = GetDeathScorer( pKiller, pInflictor );

		// pVictim is the buildables owner
		CFFPlayer *pVictim = NULL, *pOwner = NULL;
		if( pObject->Classify() == CLASS_SENTRYGUN || pObject->Classify() == CLASS_DISPENSER )
		{
			pOwner = ToFFPlayer( ( pObject )->m_hOwner.Get() );
			if ( !pObject->IsMaliciouslySabotaged() )
				pVictim = pOwner;
			else
			{
				pVictim = ToFFPlayer( ( pObject )->m_hSaboteur.Get() );
				// try not to teamkill your other sabotaged buildables
				if ( g_pGameRules->PlayerRelationship( pVictim, pKiller ) == GR_TEAMMATE )
					pVictim = pOwner;
			}
		}

		// Custom kill type?
		//if( info.GetCustomKill() )
		//{
		//	pszWeapon = GetCustomKillString( info );
		//	if( pScorer )
		//	{
		//		iKillerID = pScorer->GetUserID();
		//	}
		//}
		//else
		{
			// Is the killer a client?
			if( pScorer )
			{
				iKillerID = pScorer->GetUserID();

				if( pInflictor )
				{
					if( pInflictor == pScorer )
					{
						// If the inflictor is the killer,  then it must be their current weapon doing the damage
						if( pScorer->GetActiveWeapon() )
						{
							pszWeapon = pScorer->GetActiveWeapon()->GetDeathNoticeName();
						}
					}
					else
					{
						pszWeapon = STRING( pInflictor->m_iClassname );  // it's just that easy
					}
				}
			}
			else
			{
				pszWeapon = STRING( pInflictor->m_iClassname );
			}

			// --> Mirv: Special case for projectiles
			CFFProjectileBase *pProjectile = dynamic_cast<CFFProjectileBase *> (pInflictor);

			if (pProjectile && pProjectile->m_iSourceClassname != NULL_STRING)
			{
				pszWeapon = STRING(pProjectile->m_iSourceClassname);
			}

			// Another important thing to do is to make sure that mirvlet = mirv
			// in the death messages
			if (Q_strncmp(pszWeapon, "ff_grenade_mirvlet", 18) == 0)
			{
				pszWeapon = "ff_grenade_mirv";
			}
			// <-- Mirv

			//fixes for certain time based damage.
			switch(info.GetCustomKill())
			{
			case KILLTYPE_INFECTION:
				pszWeapon = "ff_weapon_medkit";
				break;
			case KILLTYPE_BURN_LEVEL1:
				pszWeapon = "ff_burndeath_level1";
				break;
			case KILLTYPE_BURN_LEVEL2:
				pszWeapon = "ff_burndeath_level2";
				break;
			case KILLTYPE_BURN_LEVEL3:
				pszWeapon = "ff_burndeath_level3";
				break;
			case KILLTYPE_GASSED:
				pszWeapon = "ff_grenade_gas";
				break;
			case KILLTYPE_HEADSHOT:
				pszWeapon = "BOOM_HEADSHOT"; // BOOM HEADSHOT!  AAAAAAAAHHHH!
				break;
			case KILLTYPE_SENTRYGUN_DET:
				pszWeapon  = "sg_det";
				break;
			}

			//UTIL_LogPrintf( " killer_ID: %i\n", iKillerID );
			//UTIL_LogPrintf( " killer_weapon_name: %s\n", pszWeapon );

			// strip the NPC_* or weapon_* from the inflictor's classname
			if( Q_strnicmp( pszWeapon, "weapon_", 7 ) == 0 )
			{
				//UTIL_LogPrintf( "  begins with weapon_, removing\n" );
				pszWeapon += 7;
			}
			else if( Q_strnicmp( pszWeapon, "NPC_", 8 ) == 0 )
			{
				//UTIL_LogPrintf( "  begins with NPC_, removing\n" );
				pszWeapon += 8;
			}
			else if( Q_strnicmp( pszWeapon, "func_", 5 ) == 0 )
			{
				//UTIL_LogPrintf( "  begins with func_, removing\n" );
				pszWeapon += 5;
			}
			// BEG: Added by Mulchman for FF_ entities
			else if( Q_strnicmp( pszWeapon, "ff_", 3 ) == 0 )
			{
				//UTIL_LogPrintf( "  begins with ff_, removing\n" );
				pszWeapon += 3;
			}
			// END: Added by Mulchman for FF_ entities
		}

		if( pScorer )
		{
			// Award point for killing a buildable
			// Award point for killing a buildable
			if( PlayerRelationship( pScorer, pVictim ) != GR_TEAMMATE )
			{
				
				// AfterShock - Scoring System: 50 points for dispenser
				if( pObject->Classify() == CLASS_DISPENSER )
					pScorer->AddFortPoints( 50, "#FF_FORTPOINTS_KILLDISPENSER" );
				else if( pObject->Classify() == CLASS_SENTRYGUN )
				{
					pScorer->IncrementFragCount( 1 ); // 1 frag for SG kill
					CFFSentryGun *pSentryGun = FF_ToSentrygun( pObject );

					// AfterShock - Scoring System: 100 points for level 1
					if (pSentryGun->GetLevel() == 1)
						pScorer->AddFortPoints( 100, "#FF_FORTPOINTS_KILLSG1" );
					// AfterShock - Scoring System: 150 points for level 2
					else if (pSentryGun->GetLevel() == 2)
						pScorer->AddFortPoints( 150, "#FF_FORTPOINTS_KILLSG2" );
					// AfterShock - Scoring System: 200 points for level 3
					else if (pSentryGun->GetLevel() == 3)
						pScorer->AddFortPoints( 200, "#FF_FORTPOINTS_KILLSG3" );
					else 
						DevMsg( "Unknown SG level :(" );
				}
			}
		}

		// get level of killed SG
		if( pObject->Classify() == CLASS_SENTRYGUN )
		{
			CFFSentryGun *pSentryGun = FF_ToSentrygun( pObject );

			iKilledSGLevel = pSentryGun->GetLevel();
		}

		//UTIL_LogPrintf( " userid (buildable's owner): %i\n", pVictim->GetUserID() );
		//UTIL_LogPrintf( " attacker: %i\n", iKillerID );
		//UTIL_LogPrintf( " weapon: %s\n", pszWeapon );
		
		IGameEvent *pEvent = NULL;
		if( pObject->Classify() == CLASS_SENTRYGUN )
			pEvent = gameeventmanager->CreateEvent( "sentrygun_killed" );
		else if( pObject->Classify() == CLASS_DISPENSER )
			pEvent = gameeventmanager->CreateEvent( "dispenser_killed" );
		if( pEvent )
		{
			pEvent->SetInt( "userid", pVictim->GetUserID() );
			pEvent->SetInt( "attacker", iKillerID );
			pEvent->SetString( "weapon", pszWeapon );
			pEvent->SetInt( "killedsglevel", iKilledSGLevel );
			pEvent->SetInt( "killersglevel", iKillerSGLevel );
			pEvent->SetInt( "priority", 10 );

			if (pScorer)
			{
				char bracket0[29];
				Q_snprintf(bracket0, sizeof(bracket0),"%0.2f, %0.2f, %0.1f", pScorer->GetAbsOrigin().x, pScorer->GetAbsOrigin().y, pScorer->GetAbsOrigin().z);
				pEvent->SetString( "attackerpos", bracket0);
			}
			else
				pEvent->SetString( "attackerpos", "");

			gameeventmanager->FireEvent( pEvent );
		}
	}

	// --> Mirv: Prematch
	// Stuff to do when the game starts
	void CFFGameRules::StartGame()
	{
#ifndef FF_BETA_TEST_COMPILE
		m_flIntermissionEndTime = 0.f;
		m_flGameStarted = gpGlobals->curtime;

		// Don't do this upon first spawning in a map w/o any prematch
		if( gpGlobals->curtime > 2.0f )
		{
			// Stuff to do when prematch ends
			bool bFlags[ AT_MAX_FLAG ] = { true };

			// Piggy back this guy now with a FULL MAP RESET
			ResetUsingCriteria( bFlags, TEAM_UNASSIGNED, NULL, true );
		}

		IGameEvent *pEvent = gameeventmanager->CreateEvent("game_start");
		if(pEvent)
		{
			pEvent->SetInt("roundslimit", 0);
			pEvent->SetInt("timelimit", mp_timelimit.GetFloat());
			pEvent->SetInt("fraglimit", 0);
			pEvent->SetString("objective", "TF");
			gameeventmanager->FireEvent(pEvent);
		}
#endif // FF_BETA_TEST_COMPILE
	}
	// <-- Mirv: Prematch

#endif


bool CFFGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	// Do this before the groups are re-ordered. This way we can check only when
	// one entity pushing on another, and not the other way round.
	// This is checking for players moving into grenades
	if (collisionGroup0 == COLLISION_GROUP_PLAYER &&
		collisionGroup1 == COLLISION_GROUP_PROJECTILE)
	{
		return false;
	}

	// #0001026: backpacks & flags can be discarded through doors
	// Modified this as per Mirv's suggestion.  If a trigger only object flies into a regular object, they collide -> Defrag
	if( collisionGroup0 == COLLISION_GROUP_TRIGGERONLY &&
		collisionGroup1 == COLLISION_GROUP_NONE )
	{
		return true;
	}

	// We want buildables that are being built to block players trying to move into them, but pretty much nothing else
	if ( collisionGroup1 == COLLISION_GROUP_BUILDABLE_BUILDING )
	{
		if ( collisionGroup0 == COLLISION_GROUP_PLAYER || collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT )
			return true;
		// Ok this is kinda hacky but we want projectiles, shots, etc to pass through to harm the builder
		collisionGroup1 = COLLISION_GROUP_DEBRIS;
	}

	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		swap(collisionGroup0,collisionGroup1);
	}
	
	//Don't stand on COLLISION_GROUP_WEAPON
	if( collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
		return false;
	}
	
	// Don't get caught on projectiles
	if (collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT &&
		collisionGroup1 == COLLISION_GROUP_PROJECTILE)
	{
		return false;
	}

	// Nothing hits the trigger-only stuff unless its a client-side laser
	if (collisionGroup0 == COLLISION_GROUP_TRIGGERONLY ||
		collisionGroup1 == COLLISION_GROUP_TRIGGERONLY)
	{
#ifdef CLIENT_DLL
		if (collisionGroup1 == COLLISION_GROUP_LASER)
		{
			return true;
		}
#endif

		return false;
	}

	return BaseClass::ShouldCollide( collisionGroup0, collisionGroup1 ); 
}


//-----------------------------------------------------------------------------
// Purpose: Init CS ammo definitions
//-----------------------------------------------------------------------------

// shared ammo definition
// JAY: Trying to make a more physical bullet response
#define BULLET_MASS_GRAINS_TO_LB(grains)	(0.002285*(grains)/16.0f)
#define BULLET_MASS_GRAINS_TO_KG(grains)	lbs2kg(BULLET_MASS_GRAINS_TO_LB(grains))

// exaggerate all of the forces, but use real numbers to keep them consistent
#define BULLET_IMPULSE_EXAGGERATION			1	

// convert a velocity in ft/sec and a mass in grains to an impulse in kg in/s
#define BULLET_IMPULSE(grains, ftpersec)	((ftpersec)*12*BULLET_MASS_GRAINS_TO_KG(grains)*BULLET_IMPULSE_EXAGGERATION)


CAmmoDef* GetAmmoDef()
{
	static CAmmoDef def;
	static bool bInitted = false;

	if ( !bInitted )
	{
		bInitted = true;
		
		def.AddAmmoType( AMMO_NAILS,	DMG_BULLET, TRACER_LINE, 0, 0,	200/*max carry*/, 75, 0 );
		def.AddAmmoType( AMMO_SHELLS,	DMG_BULLET, TRACER_LINE, 0, 0,	200/*max carry*/, 75, 0 );
		def.AddAmmoType( AMMO_ROCKETS,	DMG_BLAST,	TRACER_LINE, 0, 0,	200/*max carry*/, 1, 0 );
		def.AddAmmoType( AMMO_CELLS,	DMG_SHOCK,	TRACER_LINE, 0, 0,	200/*max carry*/, 1, 0 );
		def.AddAmmoType( AMMO_DETPACK,	DMG_BLAST,	TRACER_LINE, 0, 0,	1/*max carry*/, 1, 0 );
		def.AddAmmoType( AMMO_MANCANNON, DMG_BLAST,	TRACER_LINE, 0, 0,	1/*max carry*/, 1, 0 );
	}

	return &def;
}


#ifndef CLIENT_DLL

const char *CFFGameRules::GetChatPrefix( bool bTeamOnly, CBasePlayer *pPlayer )
{
	// BEG: Added by Mulchman
	if( bTeamOnly )
		return "(TEAM)";
	else
		return "";
	// END: Added by Mulchman

	return "(FortressForever - chat prefix bee-hotches)";
}

const char *CFFGameRules::GetChatLocation( bool bTeamOnly, CBasePlayer *pPlayer )
{
	// TODO: abort if CVAR chooses to display location using %l (or not at all)

	CFFPlayer *pffPlayer = ToFFPlayer( pPlayer );

	if( pffPlayer )
	{
		int iTeam = pffPlayer->GetLocationTeam();
		if( iTeam < TEAM_BLUE )
			return pffPlayer->GetLocation();
		else
		{
			// Build a string to include the team
			static char szLocation[ 1024 + 32 ]; // Location size + additional size for team name
			const char *szTeam = "";

			switch( iTeam )
			{
				case TEAM_BLUE: szTeam = "#FF_TEAM_BLUE"; break;
				case TEAM_RED: szTeam = "#FF_TEAM_RED"; break;
				case TEAM_YELLOW: szTeam = "#FF_TEAM_YELLOW"; break;
				case TEAM_GREEN: szTeam = "#FF_TEAM_GREEN"; break;
			}

			// Convert to ansi on client!
			
			Q_strncpy( szLocation, szTeam, sizeof(szLocation) );
			Q_strncat( szLocation, " ", sizeof(szLocation) );
			Q_strncat( szLocation, pffPlayer->GetLocation(), sizeof(szLocation) );

			return szLocation;
		}
	}
	
	return "";
}

#endif

//-----------------------------------------------------------------------------
// Purpose: Determine if pVictim can take damage from pAttacker.
//			Each of the parameters can either be a player, sentry gun or a dispenser.
//			If the victim is a buildable, the function will work out the teammate relationships using its owner
//
//
// Parameters: pointers to victim and attacker.
//
// Returns: true = can take damage.  false = cannot take damage.
//
// Made big changes to this stuff so that it can handle buildables properly -- 2007/09/03 -> Defrag
//			Modified to:
//				1.	Solve Bug #0001705: Sentry gun takes no damage if engineer doesn't respawn.
//				2.	Moved the burden of doing all of these checks to this function.  There were lots of ugly casts
//					and checks going on that this function didn't cover, so I moved 'em here instead.
//
//-----------------------------------------------------------------------------
bool CFFGameRules::FCanTakeDamage( CBaseEntity *pVictim, CBaseEntity *pAttacker )
{
	// we need this stuff to handle the cases where the 'victim' is actually a buildable as opposed to a player.
	// if it's a buildable, then we use the buildable's owner to perform the team checks etc. -> Defrag
	CBasePlayer *pBuildableOwner = NULL;

#ifdef GAME_DLL
	// Special cases for sabotageable buildings

	// If an SG is shooting its teammates then allow it to hurt them
	if (pAttacker && pAttacker->Classify() == CLASS_SENTRYGUN)
	{
		CFFSentryGun *pSentry = dynamic_cast< CFFSentryGun* > (pAttacker);

		if (pSentry && pSentry->IsMaliciouslySabotaged())
			return true;
	}
	
	if ( pVictim && pVictim->Classify() == CLASS_SENTRYGUN )
	{
		CFFSentryGun *pSentry = dynamic_cast <CFFSentryGun *> (pVictim);

		// Allow team to kill their own SG if it is sabotaged
		if (pSentry && pSentry->IsSabotaged())
			return true;

		// if it's not sabotaged then we need to get its owner and use it later on
		pBuildableOwner = dynamic_cast< CBasePlayer* > ( pSentry->m_hOwner.Get() );
		
		if( ! pBuildableOwner )
			return false;
	}

	// Allow sabotaged dispensers to give out damage when they explode
	if ( pAttacker && pAttacker->Classify() == CLASS_DISPENSER )
	{
		CFFDispenser *pDispenser = dynamic_cast <CFFDispenser *> (pAttacker);

		if (pDispenser && pDispenser->IsSabotaged())
			return true;
	}

	// Allow sabotaged dispensers to be destroyed by shooting
	if ( pVictim && pVictim->Classify() == CLASS_DISPENSER )
	{
		CFFDispenser *pDispenser = dynamic_cast <CFFDispenser *> (pVictim);

		if (pDispenser && pDispenser->IsSabotaged())
			return true;

		// if it's not sabotaged then we need to get its owner and use it later on
		pBuildableOwner = dynamic_cast< CBasePlayer* > ( pDispenser->m_hOwner.Get() );

		if( ! pBuildableOwner )
			return false;
	}
#endif

	if ( !pVictim )
	{
		return false;
	}

	// Don't affect players who are chilling out
    if( pVictim->IsPlayer() )
	{
		CBasePlayer *pVictimPlayer = dynamic_cast< CBasePlayer* > ( pVictim );
		if( pVictimPlayer->IsObserver() || ! pVictimPlayer->IsAlive() ) 
			return false;
	}	

	// if the buildable's owner is a non-null pointer, then we must use it to determine the relationship
	// if it's null, then we're operating on a non-buildable (most likely a player), so just use the pVictim pointer instead.
	if (( pAttacker ) && ( PlayerRelationship( pBuildableOwner ? pBuildableOwner : pVictim, pAttacker ) == GR_TEAMMATE ))
	{
		// If friendly fire is off and I'm not attacking myself, then
		// someone else on my team/an ally is attacking me - don't
		// take damage
		if (( friendlyfire.GetInt() == 0 ) && ( pVictim != pAttacker ))
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Find the relationship between players (teamplay vs. deathmatch)
//-----------------------------------------------------------------------------
int CFFGameRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
	if( !pPlayer || !pTarget )
		return GR_NOTTEAMMATE;
	
	if( pPlayer->GetTeamNumber() == pTarget->GetTeamNumber() )
		return GR_TEAMMATE;

	if( pPlayer->IsPlayer() && pTarget->IsPlayer() )
	{
		// --> Mirv: Allies
		CFFTeam *pPlayerTeam = ( CFFTeam * )GetGlobalTeam( pPlayer->GetTeamNumber() );

		if( pPlayerTeam->GetAllies() & ( 1 << pTarget->GetTeamNumber() ) )
			return GR_TEAMMATE;		// Do you want them identified as an ally or a tm?
		// <-- Mirv: Allies
	}

#ifdef GAME_DLL
	CFFBuildableObject *pBuildable = FF_ToBuildableObject(pTarget);
	if( pPlayer->IsPlayer() && pBuildable )
	{
		// --> Mirv: Allies
		CFFTeam *pPlayerTeam = ( CFFTeam * )GetGlobalTeam( pPlayer->GetTeamNumber() );
		// Jiggles: I threw in some more error checking here because the server was crashing here
		//	Specifically: CBaseEntity::GetTeamNumber (this=0x0)
		CFFPlayer *pBuildableOwner = pBuildable->GetOwnerPlayer();
		if( pBuildableOwner && pPlayerTeam && ( pPlayerTeam->GetAllies() & ( 1 << pBuildableOwner->GetTeamNumber() ) ) )
		//if( pPlayerTeam->GetAllies() & ( 1 << pBuildable->GetOwnerPlayer()->GetTeamNumber() ) )
			return GR_TEAMMATE;		// Do you want them identified as an ally or a tm?
		// <-- Mirv: Allies
	}
#endif

	return GR_NOTTEAMMATE;
}

//-----------------------------------------------------------------------------
// Purpose: Find the relationship between 2 teams. GR_TEAMMATE == ALLY
//-----------------------------------------------------------------------------
int CFFGameRules::IsTeam1AlliedToTeam2( int iTeam1, int iTeam2 )
{
	// Unassigned not a teammate to anyone
	if( (iTeam1 == TEAM_UNASSIGNED) || (iTeam2 == TEAM_UNASSIGNED) )
		return GR_NOTTEAMMATE;

	// There is a spectator involved here. We let specs be allies for
	// scoreboard stuff and possibly something else I'm forgetting.
	if (iTeam1 < TEAM_BLUE || iTeam2 < TEAM_BLUE)
	{
		return GR_TEAMMATE;
	}

	// Returns GR_TEAMMATE if iTeam1 is allied to iTeam2
	Assert( ( iTeam1 >= TEAM_BLUE ) && ( iTeam1 <= TEAM_GREEN ) );
	Assert( ( iTeam2 >= TEAM_BLUE ) && ( iTeam2 <= TEAM_GREEN ) );

	// Same team, but still the result we're looking for
	if( iTeam1 == iTeam2 )
		return GR_TEAMMATE;
	else
	{
		// Use mirv's allies stuff...
		CFFTeam *pTeam1 = ( CFFTeam * )GetGlobalTeam( iTeam1 );

		// Watch out for this happening in debug builds, maybe we can catch the cause
		Assert(pTeam1);

		if( pTeam1 && (pTeam1->GetAllies() & ( 1 << iTeam2 )) )
			return GR_TEAMMATE;
	}

	return GR_NOTTEAMMATE;
}

//-----------------------------------------------------------------------------
// Purpose: Taken from Nov 06 SDK Update
//-----------------------------------------------------------------------------
void CFFGameRules::PlayerKilled(CBasePlayer *pVictim, const CTakeDamageInfo &info)
{
#ifndef CLIENT_DLL
	if (IsIntermission())
		return;

	BaseClass::PlayerKilled(pVictim, info);
#endif
}

bool CFFGameRules::IsIntermission()
{
#ifndef CLIENT_DLL
	return m_flIntermissionEndTime > gpGlobals->curtime;
#endif

	return false;
}

#ifdef GAME_DLL
bool Server_IsIntermission()
{
	CFFGameRules *g = dynamic_cast <CFFGameRules *> (g_pGameRules);
	if (!g)
		return false;
	return g->IsIntermission();
}
#endif