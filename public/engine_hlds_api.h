//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ENGINE_HLDS_API_H
#define ENGINE_HLDS_API_H
#ifdef _WIN32
#pragma once
#endif

#include "interface.h"

#define VENGINE_HLDS_API_VERSION "VENGINE_HLDS_API_VERSION001"
//-----------------------------------------------------------------------------
// Purpose: This is the interface exported by the engine.dll to llow a dedicated server front end
//  application to host it.
//-----------------------------------------------------------------------------
class IDedicatedServerAPI
{
// Functions
public:
	// Initialize the engine with the specified base directory and interface factories
	virtual bool		Init( const char *basedir, CreateInterfaceFn launcherFactory ) = 0;
	// Shutdown the engine
	virtual void		Shutdown( void ) = 0;
	// Run a frame
	virtual bool		RunFrame( void ) = 0;
	// Insert text into console
	virtual void		AddConsoleText( char *text ) = 0;
	// Get current status to dispaly in the hlds UI (console window title bar, e.g. )
	virtual void		UpdateStatus(float *fps, int *nActive, int *nMaxPlayers, char *pszMap, int maxlen ) = 0;
};

#endif // ENGINE_HLDS_API_H
