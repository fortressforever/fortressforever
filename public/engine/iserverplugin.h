//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef ISERVERPLUGIN_H
#define ISERVERPLUGIN_H

#ifdef _WIN32
#pragma once
#endif

#include "edict.h"
#include "interface.h"
#include "KeyValues.h"

//
// you will also want to listen for game events via IGameEventManager::AddListener()
//

typedef enum 
{
	PLUGIN_CONTINUE = 0, // keep going
	PLUGIN_OVERRIDE, // run the game dll function but use our return value instead
	PLUGIN_STOP, // don't run the game dll function at all
} PLUGIN_RESULT;


#define INTERFACEVERSION_ISERVERPLUGINCALLBACKS	"ISERVERPLUGINCALLBACKS001"
//-----------------------------------------------------------------------------
// Purpose: callbacks the engine exposes to the 3rd party plugins (ala MetaMod)
//-----------------------------------------------------------------------------
abstract_class IServerPluginCallbacks
{
public:
	// Initialize the plugin to run
	// Return false if there is an error during startup.
	virtual bool			Load(	CreateInterfaceFn interfaceFactory, CreateInterfaceFn gameServerFactory  ) = 0;

	// Called when the plugin should be shutdown
	virtual void			Unload( void ) = 0;

	// called when a plugins execution is stopped but the plugin is not unloaded
	virtual void			Pause( void ) = 0;

	// called when a plugin should start executing again (sometime after a Pause() call)
	virtual void			UnPause( void ) = 0;

	// Returns string describing current plugin.  e.g., Admin-Mod.  
	virtual const char     *GetPluginDescription( void ) = 0;      

	// Called any time a new level is started (after GameInit() also on level transitions within a game)
	virtual void			LevelInit( char const *pMapName ) = 0;

	// The server is about to activate
	virtual void			ServerActivate( edict_t *pEdictList, int edictCount, int clientMax ) = 0;

	// The server should run physics/think on all edicts
	virtual void			GameFrame( bool simulating ) = 0;

	// Called when a level is shutdown (including changing levels)
	virtual void			LevelShutdown( void ) = 0;

	// Client is going active
	virtual void			ClientActive( edict_t *pEntity ) = 0;
	
	// Client is disconnecting from server
	virtual void			ClientDisconnect( edict_t *pEntity ) = 0;
	
	// Client is connected and should be put in the game
	virtual void			ClientPutInServer( edict_t *pEntity, char const *playername ) = 0;

	// Sets the client index for the client who typed the command into their console
	virtual void			SetCommandClient( int index ) = 0;

	// A player changed one/several replicated cvars (name etc)
	virtual void			ClientSettingsChanged( edict_t *pEdict ) = 0;

	// Client is connecting to server ( set retVal to false to reject the connection )
	//	You can specify a rejection message by writing it into reject
	virtual PLUGIN_RESULT	ClientConnect( bool *bAllowConnect, edict_t *pEntity, const char *pszName, const char *pszAddress, char *reject, int maxrejectlen ) = 0;

	// The client has typed a command at the console
	virtual PLUGIN_RESULT	ClientCommand( edict_t *pEntity ) = 0;

	// A user has had their network id setup and validated 
	virtual PLUGIN_RESULT	NetworkIDValidated( const char *pszUserName, const char *pszNetworkID ) = 0;
};

#define INTERFACEVERSION_ISERVERPLUGINHELPERS	"ISERVERPLUGINHELPERS001"


typedef enum
{
	DIALOG_MSG = 0, // just an on screen message
	DIALOG_MENU, // an options menu
	DIALOG_TEXT, // a richtext dialog
	DIALOG_ENTRY // an entry box
} DIALOG_TYPE;

//-----------------------------------------------------------------------------
// Purpose: functions that only 3rd party plugins need
//-----------------------------------------------------------------------------
abstract_class IServerPluginHelpers
{
public:
	// creates an onscreen menu with various option buttons
	//	The keyvalues param can contain these fields:
	//	"title" - (string) the title to show in the hud and in the title bar
	//	"msg" - (string) a longer message shown in the GameUI
	//  "color" - (color) the color to display the message in the hud (white by default)
	//	"level" - (int) the priority of this message (closer to 0 is higher), only 1 message can be outstanding at a time
	//	"time" - (int) the time in seconds this message should stay active in the GameUI (min 10 sec, max 200 sec)
	//
	// For DIALOG_MENU add sub keys for each option with these fields:
	//  "command" - (string) client command to run if selected
	//  "msg" - (string) button text for this option
	//
	virtual void CreateMessage( edict_t *pEntity, DIALOG_TYPE type, KeyValues *data, IServerPluginCallbacks *plugin ) = 0;
	virtual void ClientCommand( edict_t *pEntity, const char *cmd ) = 0;
};

#endif //ISERVERPLUGIN_H
