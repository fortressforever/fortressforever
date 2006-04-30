//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Basic BOT handling.
//
// $Workfile:     $
// $Date: 2005/09/29 19:26:37 $
//
//-----------------------------------------------------------------------------
// $Log: ff_bot_temp.cpp,v $
// Revision 1.11  2005/09/29 19:26:37  mulchman
// no message
//
// Revision 1.10  2005/09/29 19:21:52  mulchman
// no message
//
// Revision 1.9  2005/09/29 19:19:10  mulchman
// no message
//
// Revision 1.8  2005/09/28 19:06:06  mulchman
// no message
//
// Revision 1.7  2005/09/21 14:35:37  mulchman
// no message
//
// Revision 1.6  2005/09/18 18:10:48  mulchman
// no message
//
// Revision 1.5  2005/08/26 22:00:12  fryguy
// fix bot bug added with new class menus and such
//
// Revision 1.4  2005/08/03 06:38:49  fryguy
// remove icon from hud_message (I'm not understanding why it's there)
// fix a bug with the status icons not overwriting previous icons
// buttons in maps without lua files should be usable now
// buttons can no longer be used over and over again when they are at their peak
// bots now actively change classes when instructed to with bot_changeclass <number>
// lua: BroadcastSound now works, and added BroadcastMessage and RespawnAllPlayers (still has a couple bugs)
// trigger_multiple entities now override the OnTrigger method when used in maps if instructed by lua (to create respawn doors)
// Add engineer regenning armor (TODO: fix clamping)
// Remove class for player when they change team (so they don't immediately spawn when choosing a new team)
// Ragdolls are now removed after 30 seconds of being in game
// Remove the really annoying screen fade thingy when you die because of fall damage. BEEP BEEP BEEEEEP
// Change concs again slightly
//
// Revision 1.3  2005/07/26 01:21:47  mulchman
// no message
//
// Revision 1.2  2005/06/27 16:32:38  mulchman
// no message
//
// Revision 1.1  2005/02/20 21:54:21  billdoor
// no message
//
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "ff_player.h"
#include "in_buttons.h"
#include "movehelper_server.h"
#include "gameinterface.h"
#include "ff_utils.h"

// TODO: REMOVE ME REMOVE ME
//#include "ff_detpack.h"
#include "ff_buildableobjects_shared.h"

class CFFBot;
void Bot_Think( CFFBot *pBot );


ConVar bot_forcefireweapon( "bot_forcefireweapon", "", 0, "Force bots with the specified weapon to fire." );
ConVar bot_forceattack2( "bot_forceattack2", "0", 0, "When firing, use attack2." );
ConVar bot_forceattackon( "bot_forceattackon", "0", 0, "When firing, don't tap fire, hold it down." );
ConVar bot_flipout( "bot_flipout", "0", 0, "When on, all bots fire their guns." );
ConVar bot_changeclass( "bot_changeclass", "1", 0, "Force all bots to change to the specified class." );
static ConVar bot_mimic( "bot_mimic", "0", 0, "Bot uses usercmd of player by index." );
static ConVar bot_mimic_yaw_offset( "bot_mimic_yaw_offset", "0", 0, "Offsets the bot yaw." );

ConVar bot_sendcmd( "bot_sendcmd", "", 0, "Forces bots to send the specified command." );

ConVar bot_crouch( "bot_crouch", "0", 0, "Bot crouches" );

static int g_CurBotNumber = 1;


// This is our bot class.
class CFFBot : public CFFPlayer
{
public:
	bool			m_bBackwards;

	float			m_flNextTurnTime;
	bool			m_bLastTurnToRight;

	float			m_flNextStrafeTime;
	float			m_flSideMove;

	QAngle			m_ForwardAngle;
	QAngle			m_LastAngles;
	
};

LINK_ENTITY_TO_CLASS( ff_bot, CFFBot );

class CBotManager
{
public:
	static CBasePlayer* ClientPutInServerOverride_Bot( edict_t *pEdict, const char *playername )
	{
		// This tells it which edict to use rather than creating a new one.
		CBasePlayer::s_PlayerEdict = pEdict;

		CFFBot *pPlayer = static_cast<CFFBot *>( CreateEntityByName( "ff_bot" ) );
		if ( pPlayer )
		{
			char trimmedName[MAX_PLAYER_NAME_LENGTH];
			Q_strncpy( trimmedName, playername, sizeof( trimmedName ) );
			pPlayer->PlayerData()->netname = AllocPooledString( trimmedName );
		}

		return pPlayer;
	}
};


//-----------------------------------------------------------------------------
// Purpose: Create a new Bot and put it in the game.
// Output : Pointer to the new Bot, or NULL if there's no free clients.
//-----------------------------------------------------------------------------
CBasePlayer *BotPutInServer( bool bFrozen )
{
	char botname[ 64 ];
	Q_snprintf( botname, sizeof( botname ), "Bot%02i", g_CurBotNumber );

	
	// This trick lets us create a CFFBot for this client instead of the CFFPlayer
	// that we would normally get when ClientPutInServer is called.
	ClientPutInServerOverride( &CBotManager::ClientPutInServerOverride_Bot );
	edict_t *pEdict = engine->CreateFakeClient( botname );
	ClientPutInServerOverride( NULL );

	if (!pEdict)
	{
		Msg( "Failed to create Bot.\n");
		return NULL;
	}

	// Allocate a player entity for the bot, and call spawn
	CFFBot *pPlayer = ((CFFBot*)CBaseEntity::Instance( pEdict ));

	pPlayer->ClearFlags();
	pPlayer->AddFlag( FL_CLIENT | FL_FAKECLIENT );

	if ( bFrozen )
		pPlayer->AddEFlags( EFL_BOT_FROZEN );

	//pPlayer->ChangeTeam( TEAM_UNASSIGNED );
	pPlayer->ChangeTeam( TEAM_BLUE );
	pPlayer->ChangeClass( Class_IntToString(bot_changeclass.GetInt()) );
	pPlayer->RemoveAllItems( true );
	pPlayer->Spawn();

	g_CurBotNumber++;

	return pPlayer;
}

// Handler for the "bot" command.
void BotAdd_f()
{
	extern int FindEngineArgInt( const char *pName, int defaultVal );
	extern const char* FindEngineArg( const char *pName );

	// Look at -count.
	int count = FindEngineArgInt( "-count", 1 );
	count = clamp( count, 1, 16 );

	// Look at -frozen.
	bool bFrozen = !!FindEngineArg( "-frozen" );
		
	// Ok, spawn all the bots.
	while ( --count >= 0 )
	{
		BotPutInServer( bFrozen );
	}
}
ConCommand cc_Bot( "bot_add", BotAdd_f, "Add a bot.", FCVAR_CHEAT );


//-----------------------------------------------------------------------------
// Purpose: Run through all the Bots in the game and let them think.
//-----------------------------------------------------------------------------
void Bot_RunAll( void )
{
	for ( int i = 1; i <= gpGlobals->maxClients; i++ )
	{
		CFFPlayer *pPlayer = ToFFPlayer( UTIL_PlayerByIndex( i ) );

		if ( pPlayer && (pPlayer->GetFlags() & FL_FAKECLIENT) )
		{
			CFFBot *pBot = dynamic_cast< CFFBot* >( pPlayer );
			if ( pBot )
				Bot_Think( pBot );
		}
	}
}

bool Bot_RunMimicCommand( CUserCmd& cmd )
{
	if ( bot_mimic.GetInt() <= 0 )
		return false;

	if ( bot_mimic.GetInt() > gpGlobals->maxClients )
		return false;

	
	CBasePlayer *pPlayer = UTIL_PlayerByIndex( bot_mimic.GetInt()  );
	if ( !pPlayer )
		return false;

	if ( !pPlayer->GetLastUserCommand() )
		return false;

	cmd = *pPlayer->GetLastUserCommand();
	cmd.viewangles[YAW] += bot_mimic_yaw_offset.GetFloat();

	if( bot_crouch.GetInt() )
		cmd.buttons |= IN_DUCK;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Simulates a single frame of movement for a player
// Input  : *fakeclient - 
//			*viewangles - 
//			forwardmove - 
//			m_flSideMove - 
//			upmove - 
//			buttons - 
//			impulse - 
//			msec - 
// Output : 	virtual void
//-----------------------------------------------------------------------------
static void RunPlayerMove( CFFPlayer *fakeclient, CUserCmd &cmd, float frametime )
{
	if ( !fakeclient )
		return;

	// Store off the globals.. they're gonna get whacked
	float flOldFrametime = gpGlobals->frametime;
	float flOldCurtime = gpGlobals->curtime;

	float flTimeBase = gpGlobals->curtime + gpGlobals->frametime - frametime;
	fakeclient->SetTimeBase( flTimeBase );

	MoveHelperServer()->SetHost( fakeclient );
	fakeclient->PlayerRunCommand( &cmd, MoveHelperServer() );

	// save off the last good usercmd
	fakeclient->SetLastUserCommand( cmd );

	// Clear out any fixangle that has been set
	fakeclient->pl.fixangle = FIXANGLE_NONE;

	// Restore the globals..
	gpGlobals->frametime = flOldFrametime;
	gpGlobals->curtime = flOldCurtime;
}



void Bot_UpdateStrafing( CFFBot *pBot, CUserCmd &cmd )
{
	if ( gpGlobals->curtime >= pBot->m_flNextStrafeTime )
	{
		pBot->m_flNextStrafeTime = gpGlobals->curtime + 1.0f;

		if ( random->RandomInt( 0, 5 ) == 0 )
		{
			pBot->m_flSideMove = -600.0f + 1200.0f * random->RandomFloat( 0, 2 );
		}
		else
		{
			pBot->m_flSideMove = 0;
		}
		cmd.sidemove = pBot->m_flSideMove;

		if ( random->RandomInt( 0, 20 ) == 0 )
		{
			pBot->m_bBackwards = true;
		}
		else
		{
			pBot->m_bBackwards = false;
		}
	}
}


void Bot_UpdateDirection( CFFBot *pBot )
{
	float angledelta = 15.0;
	QAngle angle;

	int maxtries = (int)360.0/angledelta;

	if ( pBot->m_bLastTurnToRight )
	{
		angledelta = -angledelta;
	}

	angle = pBot->GetLocalAngles();

	trace_t trace;
	Vector vecSrc, vecEnd, forward;
	while ( --maxtries >= 0 )
	{
		AngleVectors( angle, &forward );

		vecSrc = pBot->GetLocalOrigin() + Vector( 0, 0, 36 );

		vecEnd = vecSrc + forward * 10;

		UTIL_TraceHull( vecSrc, vecEnd, VEC_HULL_MIN, VEC_HULL_MAX, 
			MASK_PLAYERSOLID, pBot, COLLISION_GROUP_NONE, &trace );

		if ( trace.fraction == 1.0 )
		{
			if ( gpGlobals->curtime < pBot->m_flNextTurnTime )
			{
				break;
			}
		}

		angle.y += angledelta;

		if ( angle.y > 180 )
			angle.y -= 360;
		else if ( angle.y < -180 )
			angle.y += 360;

		pBot->m_flNextTurnTime = gpGlobals->curtime + 2.0;
		pBot->m_bLastTurnToRight = random->RandomInt( 0, 1 ) == 0 ? true : false;

		pBot->m_ForwardAngle = angle;
		pBot->m_LastAngles = angle;
	}
	
	pBot->SetLocalAngles( angle );
}


void Bot_FlipOut( CFFBot *pBot, CUserCmd &cmd )
{
	if ( bot_flipout.GetInt() > 0 && pBot->IsAlive() )
	{
		if ( bot_forceattackon.GetBool() || (RandomFloat(0.0,1.0) > 0.5) )
		{
			cmd.buttons |= bot_forceattack2.GetBool() ? IN_ATTACK2 : IN_ATTACK;
		}

		if ( bot_flipout.GetInt() >= 2 )
		{
			QAngle angOffset = RandomAngle( -1, 1 );

			pBot->m_LastAngles += angOffset;

			for ( int i = 0 ; i < 2; i++ )
			{
				if ( fabs( pBot->m_LastAngles[ i ] - pBot->m_ForwardAngle[ i ] ) > 15.0f )
				{
					if ( pBot->m_LastAngles[ i ] > pBot->m_ForwardAngle[ i ] )
					{
						pBot->m_LastAngles[ i ] = pBot->m_ForwardAngle[ i ] + 15;
					}
					else
					{
						pBot->m_LastAngles[ i ] = pBot->m_ForwardAngle[ i ] - 15;
					}
				}
			}

			pBot->m_LastAngles[ 2 ] = 0;

			pBot->SetLocalAngles( pBot->m_LastAngles );
		}
	}
}


void Bot_HandleSendCmd( CFFBot *pBot )
{
	if ( strlen( bot_sendcmd.GetString() ) > 0 )
	{
		DevMsg( "[Bot] Clientcmd: %s\n", bot_sendcmd.GetString() );
		//send the cmd from this bot
		pBot->ClientCommand( bot_sendcmd.GetString() );

		// BEG: Added by Mulch to get the bot to actually do stuff
		if( Q_strcmp( "dispenser", bot_sendcmd.GetString() ) == 0 )
			pBot->Command_BuildDispenser();

		if( Q_strcmp( "detpack", bot_sendcmd.GetString() ) == 0 )
			pBot->Command_BuildDetpack();

		if( Q_strcmp( "sentrygun", bot_sendcmd.GetString() ) == 0 )
			pBot->Command_BuildSentryGun();

		if( Q_strcmp( "sevtest", bot_sendcmd.GetString() ) == 0 )
			pBot->Command_SevTest();

		if( Q_strcmp( "ammo", bot_sendcmd.GetString() ) == 0 )
		{
			pBot->GiveAmmo( 200, AMMO_CELLS );
			pBot->GiveAmmo( 200, AMMO_SHELLS );
			pBot->GiveAmmo( 200, AMMO_NAILS );
			pBot->GiveAmmo( 200, AMMO_ROCKETS );
			pBot->GiveAmmo( 200, AMMO_RADIOTAG );
			pBot->GiveAmmo( 200, AMMO_DETPACK );
		}

		if( Q_strcmp( "disguise12", bot_sendcmd.GetString() ) == 0 )
			pBot->Bot_Disguise( TEAM_BLUE, CLASS_SOLDIER );		

		if( Q_strcmp( "disguise21", bot_sendcmd.GetString() ) == 0 )
			pBot->Bot_Disguise( TEAM_RED, CLASS_SCOUT );

		if( Q_strcmp( "toggleone", bot_sendcmd.GetString() ) == 0 )
			pBot->Command_ToggleOne();

		if( Q_strcmp( "toggletwo", bot_sendcmd.GetString() ) == 0 )
			pBot->Command_ToggleTwo();

		if( Q_strcmp( "primeone", bot_sendcmd.GetString() ) == 0 )
			pBot->Command_PrimeOne();

		if( Q_strcmp( "primetwo", bot_sendcmd.GetString() ) == 0 )
			pBot->Command_PrimeTwo();

		if( Q_strcmp( "throwgren", bot_sendcmd.GetString() ) == 0 )
			pBot->Command_ThrowGren();

		if( Q_strcmp( "dispensertext", bot_sendcmd.GetString() ) == 0 )
		{
			pBot->Bot_SetDispenserText( "back off my sg, fool!" );
		}
		// END: Added by Mulch to get the bot to actually do stuff

		bot_sendcmd.SetValue("");
	}
}


// If bots are being forced to fire a weapon, see if I have it
void Bot_ForceFireWeapon( CFFBot *pBot, CUserCmd &cmd )
{
	if ( bot_forcefireweapon.GetString() )
	{
		CBaseCombatWeapon *pWeapon = pBot->Weapon_OwnsThisType( bot_forcefireweapon.GetString() );
		if ( pWeapon )
		{
			// Switch to it if we don't have it out
			CBaseCombatWeapon *pActiveWeapon = pBot->GetActiveWeapon();

			// Switch?
			if ( pActiveWeapon != pWeapon )
			{
				pBot->Weapon_Switch( pWeapon );
			}
			else
			{
				// Start firing
				// Some weapons require releases, so randomise firing
				if ( bot_forceattackon.GetBool() || (RandomFloat(0.0,1.0) > 0.5) )
				{
					cmd.buttons |= bot_forceattack2.GetBool() ? IN_ATTACK2 : IN_ATTACK;
				}
			}
		}
	}
}


void Bot_SetForwardMovement( CFFBot *pBot, CUserCmd &cmd )
{
	if ( !pBot->IsEFlagSet(EFL_BOT_FROZEN) )
	{
		if ( pBot->m_iHealth == 100 )
		{
			cmd.forwardmove = 600 * ( pBot->m_bBackwards ? -1 : 1 );
			if ( pBot->m_flSideMove != 0.0f )
			{
				cmd.forwardmove *= random->RandomFloat( 0.1, 1.0f );
			}
		}
		else
		{
			// Stop when shot
			cmd.forwardmove = 0;
		}
	}
}


void Bot_HandleRespawn( CFFBot *pBot, CUserCmd &cmd )
{
	// Wait for Reinforcement wave
	if ( !pBot->IsAlive() )
	{
		// Try hitting my buttons occasionally
		if ( random->RandomInt( 0, 100 ) > 80 )
		{
			// Respawn the bot
			if ( random->RandomInt( 0, 1 ) == 0 )
			{
				cmd.buttons |= IN_JUMP;
			}
			else
			{
				cmd.buttons = 0;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Run this Bot's AI for one frame.
//-----------------------------------------------------------------------------
void Bot_Think( CFFBot *pBot )
{
	// Make sure we stay being a bot
	pBot->AddFlag( FL_FAKECLIENT );


	CUserCmd cmd;
	Q_memset( &cmd, 0, sizeof( cmd ) );
	
	// change the bot's class if need be
	if (bot_changeclass.GetInt() != 0 && bot_changeclass.GetInt() != pBot->GetClassSlot())
	{
		pBot->ChangeClass(Class_IntToString(bot_changeclass.GetInt()));
	}

	// Finally, override all this stuff if the bot is being forced to mimic a player.
	if ( !Bot_RunMimicCommand( cmd ) )
	{
		cmd.sidemove = pBot->m_flSideMove;

		if ( pBot->IsAlive() && (pBot->GetSolid() == SOLID_BBOX) )
		{
			Bot_SetForwardMovement( pBot, cmd );

			// Only turn if I haven't been hurt
			if ( !pBot->IsEFlagSet(EFL_BOT_FROZEN) && pBot->m_iHealth == 100 )
			{
				Bot_UpdateDirection( pBot );
				Bot_UpdateStrafing( pBot, cmd );
			}

			// Handle console settings.
			Bot_ForceFireWeapon( pBot, cmd );
			Bot_HandleSendCmd( pBot );
		}
		else
		{
			Bot_HandleRespawn( pBot, cmd );
		}

		Bot_FlipOut( pBot, cmd );

		// Fix up the m_fEffects flags
		pBot->PostClientMessagesSent();

		
		
		cmd.viewangles = pBot->GetLocalAngles();
		cmd.upmove = 0;
		cmd.impulse = 0;
	}


	float frametime = gpGlobals->frametime;
	RunPlayerMove( pBot, cmd, frametime );
}


