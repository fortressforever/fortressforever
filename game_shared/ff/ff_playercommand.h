/********************************************************************
	created:	2004/12/26
	created:	26:12:2004   19:24
	filename: 	c:\Cvs\FortressForever\FortressForever\src\dlls\ff\ff_playercommand.h
	file path:	c:\Cvs\FortressForever\FortressForever\src\dlls\ff
	file base:	ff_playercommand
	file ext:	h
	author:		Niall FitzGibbon
	
	purpose:	server-side player command handler
				links server commands to player class functions
*********************************************************************/

#ifndef ff_playercommand_H
#define ff_playercommand_H

#include <map>
#include <string>

// use this macro do define commands that are trivial on the client (exist only to enable tab completion)
// the implementation of the command should reside on the server dll as a member of CFFPlayer
#ifndef CLIENT_DLL
#define FF_AUTO_COMMAND(cmd, ServerFunc, Description, Flags) \
	static CPlayerCommand SrvCmd_##cmd(#cmd, ServerFunc, Flags);
#else
#define FF_AUTO_COMMAND(cmd, ServerFunc, Description, Flags) \
	void CliCmdFunc_##cmd(void) \
	{ \
		if(engine->IsInGame()) \
		{ \
			std::string fullcmd; \
			for(int i = 0; i < engine->Cmd_Argc(); i++) \
			{ \
				if(i > 0) \
				fullcmd += ' '; \
				fullcmd += engine->Cmd_Argv(i); \
			} \
			engine->ServerCmd(fullcmd.c_str()); \
		} \
	} \
	static ConCommand CliCmd_##cmd(#cmd, CliCmdFunc_##cmd, Description);
#endif

// use this macro to declare commands that have a client function as well as a server function
// it has pretty much the same functionality as FF_AUTO_COMMAND, but it also calls ClientFunc on the client
// the client func will be called regardless of whether the player is in game or not, and regardless of the command flags
#ifndef CLIENT_DLL
#define FF_SHARED_COMMAND(cmd, ServerFunc, ClientFunc, Description, Flags) \
	static CPlayerCommand SrvCmd_##cmd(#cmd, ServerFunc, Flags);
#else
#define FF_SHARED_COMMAND(cmd, ServerFunc, ClientFunc, Description, Flags) \
	void CliCmdFunc_##cmd(void) \
	{ \
		bool shouldSend = ClientFunc(); \
		if(shouldSend && engine->IsInGame()) \
		{ \
			std::string fullcmd; \
			for(int i = 0; i < engine->Cmd_Argc(); i++) \
			{ \
				if(i > 0) \
				fullcmd += ' '; \
				fullcmd += engine->Cmd_Argv(i); \
			} \
			engine->ServerCmd(fullcmd.c_str()); \
		} \
	} \
	static ConCommand CliCmd_##cmd(#cmd, CliCmdFunc_##cmd, Description);
#endif


// map insertion is a costly process, especially when the valuetype for the map is a class
// all commands should be registered on game load (actually, they are done from the constructor of this class)
// most if not all these commands should also be registered on the client side so that help is available
// we should probably develop a combined system for this synchronisation of commands between server and client

// forward declaration
class CFFPlayer;

typedef void (CFFPlayer::*PLAYERCMD_FUNC_TYPE)(void);

#define FF_CMD_NOFLAGS			0
#define FF_CMD_SKILL_COMMAND	(1<<0)			// command only runs if player has that skill
#define FF_CMD_ALIVE			(1<<1)			// command runs on live in-game players
#define FF_CMD_DEAD				(1<<2)			// command runs on dead in-game players
#define FF_CMD_SPEC				(1<<3)			// command runs on spectators
#define FF_CMD_PREMATCH			(1<<4)			// command runs while in prematch
//#define FF_CMD_FEIGNED			(1<<5)			// command runs while player feigned
#define FF_CMD_CLOAKED			(1<<5)			// commadn runs while player is cloaked

// data class to contain information about a specific console command
class CPlayerCommand
{
public:
	CPlayerCommand(std::string strCommand, PLAYERCMD_FUNC_TYPE pfn, unsigned int uiFlags);
	std::string m_strCommand;		// the actual console command that the player has to bind or type (case sensitive)
    PLAYERCMD_FUNC_TYPE m_pfn;		// pointer to the command function inside the CFFPlayer class
	unsigned int m_uiFlags;			// bitfield of FF_CMD defines
private:
	CPlayerCommand(void);
};

class CPlayerCommands
{
private:
	typedef std::map<std::string, CPlayerCommand*> playercmdmap_t;
public:
	CPlayerCommands(void);
	~CPlayerCommands(void);

	void RegisterCommand(CPlayerCommand *pNewCmd);
    bool ProcessCommand(std::string strCommand, CBaseEntity *pEntity);

private:
	// map of commands
	playercmdmap_t m_mapPlayerCommands;
};

extern CPlayerCommands *g_pPlayerCommands;

#endif

