//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef ICVAR_H
#define ICVAR_H
#ifdef _WIN32
#pragma once
#endif

#include "convar.h"
#include "interface.h"

//-----------------------------------------------------------------------------
// Purpose: DLL interface to ConVars/ConCommands
//-----------------------------------------------------------------------------
class ICvar
{
public:
	// Try to register cvar
	virtual void			RegisterConCommandBase ( ConCommandBase *variable ) = 0;

	// If there is a +<varname> <value> on the command line, this returns the value.
	// Otherwise, it returns NULL.
	virtual char const*		GetCommandLineValue( char const *pVariableName ) = 0;

	// Try to find the cvar pointer by name
	virtual ConVar			*FindVar ( const char *var_name ) = 0;
	virtual const ConVar	*FindVar ( const char *var_name ) const = 0;

	// Get first ConCommandBase to allow iteration
	virtual ConCommandBase	*GetCommands( void ) = 0;
};

#define VENGINE_CVAR_INTERFACE_VERSION "VEngineCvar002"

extern ICvar *cvar;

#endif // ICVAR_H
