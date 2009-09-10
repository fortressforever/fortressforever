/********************************************************************
	created:	2004/12/26
	created:	26:12:2004   19:25
	filename: 	c:\Cvs\FortressForever\FortressForever\src\dlls\ff\ff_playercommand.cpp
	file path:	c:\Cvs\FortressForever\FortressForever\src\dlls\ff
	file base:	ff_playercommand
	file ext:	cpp
	author:		Niall FitzGibbon
	
	purpose:	server-side player command handler
				links server commands to player class functions
*********************************************************************/

#include "cbase.h"

#ifndef CLIENT_DLL
#include "ff_player.h"
#else
#include "c_ff_player.h"
#endif

#include "ff_playercommand.h"	// |-- Mirv: Moved down here because of GCC
#include "ff_gamerules.h"

// instance is actually created inside the first FF_AUTOLINK_COMMAND macro on the server
CPlayerCommands *g_pPlayerCommands = NULL;

CPlayerCommands::CPlayerCommands(void)
{
}

CPlayerCommands::~CPlayerCommands(void)
{
}

void CPlayerCommands::RegisterCommand(CPlayerCommand *pNewCmd)
{
	if(m_mapPlayerCommands.find(pNewCmd->m_strCommand) != m_mapPlayerCommands.end())
	{
		// command already registered
		return;
	}

	m_mapPlayerCommands.insert(std::make_pair(pNewCmd->m_strCommand, pNewCmd));
}

// returns true if command was sent to run on the player's object
// returns true if command exists but player didn't meet the intial condition check
// returns false if the command wasn't registered at all
bool CPlayerCommands::ProcessCommand(std::string strCommand, CBaseEntity *pEntity)
{
#ifndef CLIENT_DLL
	// trivial rejection if triggering entity isn't a player
	CFFPlayer *pPlayer = ToFFPlayer(pEntity);
	if(pPlayer == NULL)
		return false;

	playercmdmap_t::iterator i = m_mapPlayerCommands.find(strCommand);

	if(i == m_mapPlayerCommands.end())
	{
		// command not registered
		return false;
	}

	// check to see if this is a skill command, and if it is we need to check if the player has that ability

	//DevMsg("Player %s triggered server player command %s.\n", pPlayer->GetPlayerName(), strCommand.c_str());

	if(!(i->second->m_uiFlags & FF_CMD_ALIVE) && pPlayer->IsAlive())
	{
		//Warning("Player %s tried to use command %s, but failed because that command requires player not to be alive.\n", pPlayer->GetPlayerName(), strCommand.c_str());
		return true;
	}

	if(!(i->second->m_uiFlags & FF_CMD_DEAD) && !pPlayer->IsAlive())
	{
		//DevMsg("Player %s tried to use command %s, but failed because that command requires player not to be dead.\n", pPlayer->GetPlayerName(), strCommand.c_str());
		return true;
	}

	// --> Mirv: IsObserver includes being dead, so using TEAM_SPECTATOR now (add TEAM_UNASSIGNED too?)
	//if(!(i->second->m_uiFlags & FF_CMD_SPEC) && pPlayer->IsObserver())
	if(!(i->second->m_uiFlags & FF_CMD_SPEC) && pPlayer->GetTeamNumber() == TEAM_SPECTATOR)
	// <-- Mirv: IsObserver includes being dead, so using TEAM_SPECTATOR now (add TEAM_UNASSIGNED too?)
	{
		//DevMsg("Player %s tried to use command %s, but failed because that command requires player not to be spec.\n", pPlayer->GetPlayerName(), strCommand.c_str());
		return true;
	}

	// --> Mirv: Some extra checks
	if( !( i->second->m_uiFlags & FF_CMD_PREMATCH ) && !FFGameRules()->HasGameStarted() )
	{
		//DevMsg( "Player %s tried to use command %s, but failed because that command cannot be done in prematch.\n", pPlayer->GetPlayerName(), strCommand.c_str() );
		return true;
	}

	//if( !( i->second->m_uiFlags & FF_CMD_FEIGNED ) && pPlayer->IsFeigned() )
	if( !( i->second->m_uiFlags & FF_CMD_CLOAKED ) && pPlayer->IsCloaked() )
	{
		//DevMsg( "Player %s tried to use command %s, but failed because that command cannot be done while cloaked.\n", pPlayer->GetPlayerName(), strCommand.c_str() );
		return true;
	}
	// <-- Mirv: Some extra checks

	if(i->second->m_uiFlags & FF_CMD_SKILL_COMMAND)
	{
		if(!pPlayer->PlayerHasSkillCommand(strCommand.c_str()))
		{
			//DevMsg("Player %s tried to use skill command %s, but he does not have that skill.\n", pPlayer->GetPlayerName(), strCommand.c_str());
			return true;
		}
	}

	// call the member function that deals with this command
	(pPlayer->*i->second->m_pfn)();

	return true;
#else
	return false;
#endif
}

CPlayerCommand::CPlayerCommand(std::string strCommand, PLAYERCMD_FUNC_TYPE pfn, unsigned int uiFlags)
{
	m_strCommand = strCommand;
	m_pfn = pfn;
	m_uiFlags = uiFlags;

	if(g_pPlayerCommands == NULL)
		g_pPlayerCommands = new CPlayerCommands;

	g_pPlayerCommands->RegisterCommand(this);
}

FF_AUTO_COMMAND( testcmd, &CFFPlayer::Command_TestCommand, "test command description", FF_CMD_ALIVE );

FF_AUTO_COMMAND( mapguide, &CFFPlayer::Command_MapGuide, "Start/stop a map guide.", FF_CMD_ALIVE | FF_CMD_DEAD | FF_CMD_SPEC | FF_CMD_PREMATCH ); // |-- Mirv: Channel setting command
FF_AUTO_COMMAND( setchannel, &CFFPlayer::Command_SetChannel, "Choose your voice comm channel.", FF_CMD_ALIVE | FF_CMD_DEAD | FF_CMD_SPEC | FF_CMD_PREMATCH ); // |-- Mirv: Channel setting command

FF_AUTO_COMMAND( class, &CFFPlayer::Command_Class, "Choose your player class.", FF_CMD_ALIVE | FF_CMD_DEAD | FF_CMD_PREMATCH | FF_CMD_CLOAKED );
FF_AUTO_COMMAND( team, &CFFPlayer::Command_Team, "Choose your team.", FF_CMD_ALIVE | FF_CMD_DEAD | FF_CMD_SPEC | FF_CMD_PREMATCH | FF_CMD_CLOAKED );
FF_AUTO_COMMAND( whatteam, &CFFPlayer::Command_WhatTeam, "Tells what team you're on for debugging.", FF_CMD_ALIVE | FF_CMD_DEAD | FF_CMD_SPEC | FF_CMD_PREMATCH | FF_CMD_CLOAKED );
FF_AUTO_COMMAND( dispenser, &CFFPlayer::Command_BuildDispenser, "Build or detonate a dispenser.", FF_CMD_SKILL_COMMAND | FF_CMD_ALIVE | FF_CMD_PREMATCH );
FF_AUTO_COMMAND( sentrygun, &CFFPlayer::Command_BuildSentryGun, "Build or detonate a sentry gun.", FF_CMD_SKILL_COMMAND | FF_CMD_ALIVE | FF_CMD_PREMATCH );
FF_AUTO_COMMAND( dispensertext, &CFFPlayer::Command_DispenserText, "Set custom text for your dispenser.", FF_CMD_ALIVE | FF_CMD_DEAD | FF_CMD_SPEC | FF_CMD_PREMATCH );
FF_AUTO_COMMAND( detpack, &CFFPlayer::Command_BuildDetpack, "Drop a detpack.", FF_CMD_SKILL_COMMAND | FF_CMD_ALIVE | FF_CMD_PREMATCH );
FF_AUTO_COMMAND( mancannon, &CFFPlayer::Command_BuildManCannon, "Build a man cannon.", FF_CMD_SKILL_COMMAND | FF_CMD_ALIVE | FF_CMD_PREMATCH );
FF_AUTO_COMMAND( discard, &CFFPlayer::Command_Discard, "Discards unneeded ammo", FF_CMD_ALIVE | FF_CMD_PREMATCH | FF_CMD_CLOAKED );
FF_AUTO_COMMAND( saveme, &CFFPlayer::Command_SaveMe, "Call for medical attention", FF_CMD_ALIVE | FF_CMD_PREMATCH );
FF_AUTO_COMMAND( engyme, &CFFPlayer::Command_EngyMe, "Call for engineer attention", FF_CMD_ALIVE | FF_CMD_PREMATCH );
FF_AUTO_COMMAND( ammome, &CFFPlayer::Command_AmmoMe, "Call for ammo", FF_CMD_ALIVE | FF_CMD_PREMATCH );
FF_AUTO_COMMAND( disguise, &CFFPlayer::Command_Disguise, "Disguise <team> <class>", FF_CMD_SKILL_COMMAND | FF_CMD_ALIVE | FF_CMD_PREMATCH | FF_CMD_CLOAKED )

// entity system interfacing stuffs
FF_AUTO_COMMAND( flaginfo, &CFFPlayer::Command_FlagInfo, "Displays information about the flag", FF_CMD_ALIVE | FF_CMD_DEAD | FF_CMD_PREMATCH | FF_CMD_CLOAKED );
FF_AUTO_COMMAND( dropitems, &CFFPlayer::Command_DropItems, "Drops items (flags)", FF_CMD_ALIVE | FF_CMD_PREMATCH );
FF_AUTO_COMMAND( detpipes, &CFFPlayer::Command_DetPipes, "Detontates a demoman's pipes", FF_CMD_ALIVE | FF_CMD_PREMATCH | FF_CMD_SKILL_COMMAND );

// spy sabotage stuff
FF_AUTO_COMMAND( dispensersabotage, &CFFPlayer::Command_SabotageDispenser, "Detonate enemy dispenser that you have sabtoaged", FF_CMD_ALIVE | FF_CMD_DEAD | FF_CMD_PREMATCH | FF_CMD_CLOAKED );
FF_AUTO_COMMAND( sentrysabotage, &CFFPlayer::Command_SabotageSentry, "Turn enemy SG on own team", FF_CMD_ALIVE | FF_CMD_DEAD | FF_CMD_PREMATCH | FF_CMD_CLOAKED );

// Mulch: Making spy commands shared
FF_SHARED_COMMAND( cloak, &CFFPlayer::Command_SpyCloak, CC_SpyCloak, "Cloak", FF_CMD_ALIVE | FF_CMD_CLOAKED | FF_CMD_PREMATCH | FF_CMD_SKILL_COMMAND );
FF_SHARED_COMMAND( scloak, &CFFPlayer::Command_SpySilentCloak, CC_SpySilentCloak, "Silent Cloak", FF_CMD_ALIVE | FF_CMD_CLOAKED | FF_CMD_PREMATCH | FF_CMD_SKILL_COMMAND );
FF_SHARED_COMMAND( smartcloak, &CFFPlayer::Command_SpySmartCloak, CC_SpySmartCloak, "Smart Cloak", FF_CMD_ALIVE | FF_CMD_CLOAKED | FF_CMD_PREMATCH | FF_CMD_SKILL_COMMAND );

// --> Mirv: Toggle grenades (requested by defrag)
FF_SHARED_COMMAND( toggleone, &CFFPlayer::Command_ToggleOne, CC_ToggleOne, "Primes and throws primary grenade on subsequent touches", FF_CMD_ALIVE | FF_CMD_CLOAKED | FF_CMD_PREMATCH );
FF_SHARED_COMMAND( toggletwo, &CFFPlayer::Command_ToggleTwo, CC_ToggleTwo, "Primes and throws secondary grendae on subsequent touches", FF_CMD_ALIVE | FF_CMD_CLOAKED | FF_CMD_PREMATCH );
// <-- Mirv: Toggle grenades (requested by defrag)

//-- Added by L0ki -------------------------------------------------------
//
// Grenade related
FF_SHARED_COMMAND(primeone, &CFFPlayer::Command_PrimeOne, CC_PrimeOne, "Primes one of your primary grenades.", FF_CMD_ALIVE | FF_CMD_PREMATCH );
FF_SHARED_COMMAND(primetwo, &CFFPlayer::Command_PrimeTwo, CC_PrimeTwo, "Prime one of your secondary grenades.", FF_CMD_ALIVE | FF_CMD_PREMATCH );
FF_SHARED_COMMAND(throwgren, &CFFPlayer::Command_ThrowGren, CC_ThrowGren, "Throw a primed grenade.", FF_CMD_ALIVE | FF_CMD_CLOAKED | FF_CMD_PREMATCH );
//		had to do it this way because you cant use + and - with the macros
#ifndef CLIENT_DLL
	static CPlayerCommand SrvCmd_PlusGrenOne("+gren1", &CFFPlayer::Command_PrimeOne, FF_CMD_ALIVE | FF_CMD_PREMATCH );
	static CPlayerCommand SrvCmd_MinusGrenOne("-gren1", &CFFPlayer::Command_ThrowGren, FF_CMD_ALIVE | FF_CMD_CLOAKED | FF_CMD_PREMATCH );
	static CPlayerCommand SrvCmd_PlusGrenTwo("+gren2", &CFFPlayer::Command_PrimeTwo, FF_CMD_ALIVE | FF_CMD_PREMATCH );
	static CPlayerCommand SrvCmd_MinusGrenTwo("-gren2", &CFFPlayer::Command_ThrowGren, FF_CMD_ALIVE | FF_CMD_CLOAKED | FF_CMD_PREMATCH );
#else
	void CliCmdFunc_PlusGrenOne(void)
	{
		CC_PrimeOne();
		if(engine->IsInGame())
		{
			std::string fullcmd;
			for(int i = 0; i < engine->Cmd_Argc(); i++)
			{
				if(i > 0)
					fullcmd += ' ';
				fullcmd += engine->Cmd_Argv(i);
			}
			engine->ServerCmd(fullcmd.c_str());
		}
	}
	static ConCommand CliCmd_PlusGrenOne("+gren1", CliCmdFunc_PlusGrenOne, "Primes a primary grenade while the key is held.");
	void CliCmdFunc_MinusGrenOne(void)
	{
		CC_ThrowGren();
		if(engine->IsInGame())
		{
			std::string fullcmd;
			for(int i = 0; i < engine->Cmd_Argc(); i++)
			{
				if(i > 0)
					fullcmd += ' ';
				fullcmd += engine->Cmd_Argv(i);
			}
			engine->ServerCmd(fullcmd.c_str());
		}
	}
	static ConCommand CliCmd_MinusGrenOne("-gren1", CliCmdFunc_MinusGrenOne, "Releases a primary grenade that was previously primed with +gren1.");
	void CliCmdFunc_PlusGrenTwo(void)
	{
		CC_PrimeTwo();
		if(engine->IsInGame())
		{
			std::string fullcmd;
			for(int i = 0; i < engine->Cmd_Argc(); i++)
			{
				if(i > 0)
					fullcmd += ' ';
				fullcmd += engine->Cmd_Argv(i);
			}
			engine->ServerCmd(fullcmd.c_str());
		}
	}
	static ConCommand CliCmd_PlusGrenTwo("+gren2", CliCmdFunc_PlusGrenTwo, "Primes a secondary grenade while the key is held.");
	void CliCmdFunc_MinusGrenTwo(void)
	{
		CC_ThrowGren();
		if(engine->IsInGame())
		{
			std::string fullcmd;
			for(int i = 0; i < engine->Cmd_Argc(); i++)
			{
				if(i > 0)
					fullcmd += ' ';
				fullcmd += engine->Cmd_Argv(i);
			}
			engine->ServerCmd(fullcmd.c_str());
		}
	}
	static ConCommand CliCmd_MinusGrenTwo("-gren2", CliCmdFunc_MinusGrenTwo, "Releases a secondary grenade that was previously primed with +gren2.");
#endif
//------------------------------------------------------------------------
