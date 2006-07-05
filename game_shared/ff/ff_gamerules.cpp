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
#include "ff_weapon_base.h"

#ifdef CLIENT_DLL
	#define C_FFTeam CFFTeam
	#include "c_ff_team.h"

#else
	
	#include "voice_gamemgr.h"
	#include "ff_team.h"
	#include "ff_player.h"
	#include "ff_playercommand.h"
	#include "ff_sentrygun.h"
	#include "ff_statslog.h"

#endif


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


#ifndef CLIENT_DLL
LINK_ENTITY_TO_CLASS(info_player_terrorist, CPointEntity);
LINK_ENTITY_TO_CLASS(info_player_counterterrorist,CPointEntity);
#endif

REGISTER_GAMERULES_CLASS( CFFGameRules );


BEGIN_NETWORK_TABLE_NOBASE( CFFGameRules, DT_FFGameRules )
END_NETWORK_TABLE()


LINK_ENTITY_TO_CLASS( ff_gamerules, CFFGameRulesProxy );
IMPLEMENT_NETWORKCLASS_ALIASED( FFGameRulesProxy, DT_FFGameRulesProxy )

// --> Mirv: Prematch
ConVar mp_prematch( "mp_prematch",
					"0.0",							// trepids finding it annoying so i set it to zero and not .2
					FCVAR_NOTIFY|FCVAR_REPLICATED,
					"delay before game starts" );
// <-- Mirv: Prematch

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
			if ( pTalker->IsAlive() == false )
			{
				if ( pListener->IsAlive() == false )
					return ( pListener->InSameTeam( pTalker ) );

				return false;
			}

			return ( pListener->InSameTeam( pTalker ) );
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

		// Prematch system, game has not started
		m_flGameStarted = -1.0f;
		
		// Start stats engine
		SendStats();
		g_StatsLog.CleanUp();

		// Reset the effects timeouts
		ClearAllowedEffects();
	}

	void CFFGameRules::Precache()
	{
		m_flNextMsg = 0.0f;
		m_flGameStarted = -1.0f;
		BaseClass::Precache();
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

		float flMaxSafe = PLAYER_MAX_SAFE_FALL_SPEED;

		// Spy can fall twice as far without hurting
		if (pFFPlayer->GetClassSlot() == 8)
			flMaxSafe *= 1.412;

		// Should they really be losing damage
		if (pPlayer->m_Local.m_flFallVelocity < flMaxSafe)
			return 0;

		DevMsg("Falling speed: %f\n", (float) pPlayer->m_Local.m_flFallVelocity);

		// Speed is a good approximation for now of a class's weight
		// Therefore bigger base damage for slower classes
		float weightratio = clamp((pPlayer->MaxSpeed() - 230.0f) / 170.0f, 0, 1.0f);
		float flBaseDmg = 6.0f + (1.0f - weightratio) * 6.0f;

		// Don't worry this'll be optimised!
		float speedratio = clamp((pPlayer->m_Local.m_flFallVelocity - flMaxSafe) / (PLAYER_FATAL_FALL_SPEED - flMaxSafe), 0, 1.0f);
		float flDmg = flBaseDmg + speedratio * flBaseDmg;

		DevMsg("Base Damage for Class: %f, Damage for Fall: %f\n", flBaseDmg, flDmg);


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
		//NDebugOverlay::Cross3D(vecSrc, 8.0f, 255, 0, 0, false, 5.0f);
#endif

		// TFC style falloff please.
		falloff = 0.5f;

		// Always raise by 1.0f
		// It's so that the grenade isn't in the ground
		vecSrc.z += 1.0f;

		// iterate on all entities in the vicinity.
		for (CEntitySphereQuery sphere(vecSrc, flRadius); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity()) 
		{
			//if (pEntity == pEntityIgnore) 
			//	continue;

			if (pEntity->m_takedamage == DAMAGE_NO) 
				continue;

#ifdef GAME_DLL
			//NDebugOverlay::EntityBounds(pEntity, 0, 0, 255, 100, 5.0f);
#endif

			// Check that the explosion can 'see' this entity.
			// TFC also uses a noisy bodytarget
			vecSpot = pEntity->BodyTarget(vecSrc, true);

			// Lets calculate some values for this grenade and player
			Vector vecDirection		= (vecSpot - vecSrc);
			float flDistance		= vecDirection.Length();
			vecDirection /= flDistance;

			// Bugfix for #0000598: Backpacks blocking grenade damage
			UTIL_TraceLine(vecSrc, vecSpot, MASK_SHOT, info.GetInflictor(), /*COLLISION_GROUP_NONE*/ COLLISION_GROUP_PROJECTILE, &tr);

#ifdef GAME_DLL
			//NDebugOverlay::Line(vecSrc, vecSpot, 0, 255, 0, false, 5.0f);
#endif

			// Could not see this entity, so don't hurt it
			if (tr.fraction != 1.0 && tr.m_pEnt != pEntity) 
				continue;

			float flBaseDamage = info.GetDamage();

			// TFC uses 0.5 falloff for everything
			float falloff = 0.5f;

			// Decrease damage for an ent that's farther from the explosion
			flAdjustedDamage = flBaseDamage - (flDistance * falloff);

			// We're doing no damage, so don't do anything else here
			if (flAdjustedDamage <= 0) 
				continue;

			// In TFC players only do 2/3 damage to themselves
			// This also affects forces by the same amount
			if (pEntity == info.GetAttacker())
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
				float flCalculatedForce = flAdjustedDamage * 8.0f;

				CFFPlayer *pPlayer = ToFFPlayer(pEntity);

				// We have to reduce the force further if they're fat
				if (pPlayer && pPlayer->GetClassSlot() == CLASS_HWGUY) 
					flCalculatedForce *= 0.15f;

				CFFPlayer *pAttacker = ToFFPlayer(info.GetAttacker());

				// And also reduce if we couldn't hurt them
				// TODO: Get exact figure for this
				if (pAttacker && !g_pGameRules->FPlayerCanTakeDamage(pPlayer, pAttacker))
					flCalculatedForce *= 0.8f;

				// Now set all our calculated values
				adjustedInfo.SetDamageForce(vecDirection * flCalculatedForce);
				adjustedInfo.SetDamagePosition(vecSrc);
			}

			// Now deal the damage
			pEntity->TakeDamage(adjustedInfo);

			// For the moment we'll play blood effects if its a teammate too so its consistant with other weapons
			// if (pEntity->IsPlayer() && g_pGameRules->FPlayerCanTakeDamage(ToFFPlayer(pEntity), info.GetAttacker())) 
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
		// Lots of these depend on the game being started
		if( !HasGameStarted() )
		{
			float flPrematch = mp_prematch.GetFloat() * 60;

			// We should have started now, lets go!
			if( gpGlobals->curtime > flPrematch )
				StartGame();
			else
			{
				// Stuff to do during prematch (not constantly though!)
				for ( int i = 1; i <= gpGlobals->maxClients; i++ )
				{
					CBasePlayer *pPlayer = UTIL_PlayerByIndex( i );

					if ( pPlayer && pPlayer->GetTeamNumber() != TEAM_SPECTATOR )
						pPlayer->AddFlag( FL_FROZEN );
				}

				// This is a bit tempy
				//static float flNextMsg = 0;

				// Only send message every second (not every frame)
				if( gpGlobals->curtime > m_flNextMsg )
				{
					m_flNextMsg = gpGlobals->curtime + 1.0f;
				
					//CReliableBroadcastRecipientFilter user;

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
		
		GetVoiceGameMgr()->Update( gpGlobals->frametime );
	}
	// <-- Mirv: Hodgepodge of different checks (from the base functions) inc. prematch

	void CFFGameRules::BuildableKilled( CFFBuildableObject *pObject, const CTakeDamageInfo& info )
	{
		// DevMsg( "[FFGameRules] Buildable was killed!\n" );

		const char *pszWeapon = "world";
		int iKillerID = 0;

		// Find the killer & the scorer
		CBaseEntity *pInflictor = info.GetInflictor();
		CBaseEntity *pKiller = info.GetAttacker();
		CBasePlayer *pScorer = pScorer = GetDeathScorer( pKiller, pInflictor );

		// pVictim is the buildables owner
		CFFPlayer *pVictim = NULL;
		if( pObject->Classify() == CLASS_SENTRYGUN )
			pVictim = ToFFPlayer( ( ( CFFSentryGun * )pObject )->m_hOwner.Get() );
		else if( pObject->Classify() == CLASS_DISPENSER )
			pVictim = ToFFPlayer( ( ( CFFDispenser * )pObject )->m_hOwner.Get() );

		// Custom kill type?
		if( info.GetCustomKill() )
		{
			pszWeapon = GetCustomKillString( info );
			if( pScorer )
			{
				iKillerID = pScorer->GetUserID();
			}
		}
		else
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

			UTIL_LogPrintf( " killer_ID: %i\n", iKillerID );
			UTIL_LogPrintf( " killer_weapon_name: %s\n", pszWeapon );

			// strip the NPC_* or weapon_* from the inflictor's classname
			if( Q_strnicmp( pszWeapon, "weapon_", 7 ) == 0 )
			{
				UTIL_LogPrintf( "  begins with weapon_, removing\n" );
				pszWeapon += 7;
			}
			else if( Q_strnicmp( pszWeapon, "NPC_", 8 ) == 0 )
			{
				UTIL_LogPrintf( "  begins with NPC_, removing\n" );
				pszWeapon += 8;
			}
			else if( Q_strnicmp( pszWeapon, "func_", 5 ) == 0 )
			{
				UTIL_LogPrintf( "  begins with func_, removing\n" );
				pszWeapon += 5;
			}
			// BEG: Added by Mulchman for FF_ entities
			else if( Q_strnicmp( pszWeapon, "ff_", 3 ) == 0 )
			{
				UTIL_LogPrintf( "  begins with ff_, removing\n" );
				pszWeapon += 3;
			}
			// END: Added by Mulchman for FF_ entities
		}

		// Award points & modify weapon if 'team kill'

		if( pScorer )
		{
			// If the owner of the buildable is an ally or teammate
			// it's a 'team kill'
			if( PlayerRelationship( pScorer, pVictim ) == GR_TEAMMATE )
			{
				pScorer->IncrementFragCount( -1 );
				pszWeapon = "teammate";
			}
			else
				pScorer->IncrementFragCount( 1 );
		}

		UTIL_LogPrintf( " userid (buildable's owner): %i\n", pVictim->GetUserID() );
		UTIL_LogPrintf( " attacker: %i\n", iKillerID );
		UTIL_LogPrintf( " weapon: %s\n", pszWeapon );

		// For the event later
		IGameEvent *event = NULL;

		if( pObject->Classify() == CLASS_SENTRYGUN )
			event = gameeventmanager->CreateEvent( "sentrygun_killed" );
		else if( pObject->Classify() == CLASS_DISPENSER )
			event = gameeventmanager->CreateEvent( "dispenser_killed" );

		// If the event is valid, send it off
		if( event )
		{
			event->SetInt( "userid", pVictim->GetUserID() );
			event->SetInt( "attacker", iKillerID );
			event->SetString( "weapon", pszWeapon );
			gameeventmanager->FireEvent( event );
		}
	}

	// --> Mirv: Prematch
	// Stuff to do when the game starts
	void CFFGameRules::StartGame()
	{
		m_flGameStarted = gpGlobals->curtime;

		// Stuff to do during prematch
		for ( int i = 1; i <= gpGlobals->maxClients; i++ )
		{
			CFFPlayer *pPlayer = (CFFPlayer *) UTIL_PlayerByIndex( i );

			if ( pPlayer && pPlayer->GetTeamNumber() != TEAM_SPECTATOR && pPlayer->GetTeamNumber() != TEAM_UNASSIGNED )
			{
				pPlayer->RemoveFlag( FL_FROZEN );
				pPlayer->KillAndRemoveItems();
			}

			UTIL_ClientPrintAll( HUD_PRINTCENTER, "#FF_PREMATCH_END" );
		}
	}
	// <-- Mirv: Prematch

#endif


bool CFFGameRules::ShouldCollide( int collisionGroup0, int collisionGroup1 )
{
	// Bug #0000178: Grenades and player clipping
	// Bug #0000718: get stuck on mirv grens
	// HACKHACK this might break something else
	if( (collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT || collisionGroup0 == COLLISION_GROUP_PLAYER) &&
		collisionGroup1 == COLLISION_GROUP_PROJECTILE )
	{
		return false;
	}

	if ( collisionGroup0 > collisionGroup1 )
	{
		// swap so that lowest is always first
		swap(collisionGroup0,collisionGroup1);
	}

	//DevMsg("ShouldCollide: %d %d\n", collisionGroup0, collisionGroup1);

	// Mirv: Don't collide projectiles with weapon objects (eg. ammo bags)
	if (collisionGroup0 == COLLISION_GROUP_WEAPON && collisionGroup1 == COLLISION_GROUP_PROJECTILE)
		return false;

	// Mirv: Don't have ammo bags and the like colliding either
	if (collisionGroup0 == COLLISION_GROUP_WEAPON && collisionGroup1 == COLLISION_GROUP_WEAPON)
		return false;
	
	//Don't stand on COLLISION_GROUP_WEAPON
	if( (collisionGroup0 == COLLISION_GROUP_PLAYER_MOVEMENT || collisionGroup0 == COLLISION_GROUP_PLAYER) &&
		collisionGroup1 == COLLISION_GROUP_WEAPON )
	{
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
		def.AddAmmoType( AMMO_RADIOTAG,	DMG_BULLET,	TRACER_LINE, 0, 0,  4/*max carry*/, 75, 0 );
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
			
			Q_strcpy( szLocation, szTeam );
			Q_strcat( szLocation, " " );
			Q_strcat( szLocation, pffPlayer->GetLocation() );

			return szLocation;
		}
	}
	
	return "";
}

#endif

//-----------------------------------------------------------------------------
// Purpose: Determine if pPlayer can take damage from pAttacker
//-----------------------------------------------------------------------------
bool CFFGameRules::FPlayerCanTakeDamage( CBasePlayer *pPlayer, CBaseEntity *pAttacker )
{
#ifdef GAME_DLL
	// Special cases for sabotageable buildings

	// If an SG is shooting its teammates then allow it to hurt them
	if (pAttacker && pAttacker->Classify() == CLASS_SENTRYGUN)
	{
		CFFSentryGun *pSentry = dynamic_cast <CFFSentryGun *> (pAttacker);

		if (pSentry && pSentry->IsShootingTeammates())
			return true;
	}

	// If an SG is sabotaged or shooting its own team, allow them to kill it
	if (pPlayer && pPlayer->Classify() == CLASS_SENTRYGUN)
	{
		CFFSentryGun *pSentry = dynamic_cast <CFFSentryGun *> (pPlayer);

		// Allow team to kill their own SG if it is sabotaged
		if (pSentry && pSentry->IsSabotaged())
			return true;
	}

	// Allow sabotaged dispensers to give out damage when they explode
	if (pAttacker && pAttacker->Classify() == CLASS_DISPENSER)
	{
		CFFDispenser *pDispenser = dynamic_cast <CFFDispenser *> (pAttacker);

		if (pDispenser && pDispenser->IsSabotaged())
			return true;
	}

	// Allow sabotaged dispensers to be destroyed by shooting
	if (pPlayer && pPlayer->Classify() ==CLASS_DISPENSER)
	{
		CFFDispenser *pDispenser = dynamic_cast <CFFDispenser *> (pPlayer);

		if (pDispenser && pDispenser->IsSabotaged())
			return true;
	}
#endif

	if ((pAttacker) && (PlayerRelationship(pPlayer, pAttacker) == GR_TEAMMATE))
	{
		// If friendly fire is off and I'm not attacking myself, then
		// someone else on my team/an ally is attacking me - don't
		// take damage
		if ((friendlyfire.GetInt() == 0) && (pPlayer != pAttacker))
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Find the relationship between players (teamplay vs. deathmatch)
//-----------------------------------------------------------------------------
int CFFGameRules::PlayerRelationship( CBaseEntity *pPlayer, CBaseEntity *pTarget )
{
//#ifdef GAME_DLL	
	// BEG: Added by Mulchman
	// TODO: Fix!?
	if( pPlayer->GetTeamNumber() == pTarget->GetTeamNumber() )
	{
		//DevMsg( "[PlayerRelationship] [serverside] dudes are on the same team!\n" );
		return GR_TEAMMATE;
	}

	if( pPlayer->IsPlayer() && pTarget->IsPlayer() )
	{
		//DevMsg( "[PlayerRelationship] [serverside] comparing two players\n" );

		// --> Mirv: Allies
		CFFTeam *pPlayerTeam = ( CFFTeam * )GetGlobalTeam( pPlayer->GetTeamNumber() );

		if( pPlayerTeam->GetAllies() & ( 1 << pTarget->GetTeamNumber() ) )
		{
			//DevMsg( "[PlayerRelationship] [serverside] dudes are allies!\n" );
			return GR_TEAMMATE;		// Do you want them identified as an ally or a tm?
		}

		// Mulch: I think team mate is just fine
		// <-- Mirv: Allies

	}
	// END: Added by Mulchman
	
	//DevMsg("CFFGameRules::PlayerRelationship ( %d, %d )\n", ENTINDEX(pPlayer), ENTINDEX(pTarget));

//#endif

	// Commented out by Mulch 02/03/2006 
	/*
#ifndef CLIENT_DLL
	// half life multiplay has a simple concept of Player Relationships.
	// you are either on another player's team, or you are not.
	if ( !pPlayer || !pTarget || !pTarget->IsPlayer() || IsTeamplay() == false )
		return GR_NOTTEAMMATE;

	if ( (*GetTeamID(pPlayer) != '\0') && (*GetTeamID(pTarget) != '\0') && !stricmp( GetTeamID(pPlayer), GetTeamID(pTarget) ) )
		return GR_TEAMMATE;

#endif
		*/

	return GR_NOTTEAMMATE;
}

//-----------------------------------------------------------------------------
// Purpose: Find the relationship between 2 teams. GR_TEAMMATE == ALLY
//-----------------------------------------------------------------------------
int CFFGameRules::IsTeam1AlliedToTeam2( int iTeam1, int iTeam2 )
{
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
		if( pTeam1->GetAllies() & ( 1 << iTeam2 ) )
			return GR_TEAMMATE;
	}

	return GR_NOTTEAMMATE;
}
