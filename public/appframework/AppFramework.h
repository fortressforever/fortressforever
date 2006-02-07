//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: An application framework 
//
// $Revision: $
// $NoKeywords: $
//=============================================================================//

#ifndef APPFRAMEWORK_H
#define APPFRAMEWORK_H

#ifdef _WIN32
#pragma once
#endif

#include "interface.h"

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IAppSystem;


//-----------------------------------------------------------------------------
// Handle to a DLL
//-----------------------------------------------------------------------------
typedef int AppModule_t;

enum
{
	APP_MODULE_INVALID = (AppModule_t)~0
};

//-----------------------------------------------------------------------------
// This interface represents a group of app systems that all have the same
// lifetime that need to be connected/initialized, etc. in a well-defined order
//-----------------------------------------------------------------------------
class IAppSystemGroup
{
public:
	// This method will add a module (DLL) to the app system group
	// returns APP_MODULE_INVALID in case of error
	virtual AppModule_t LoadModule( const char *pDLLName ) = 0;

	// Method to add various global singleton systems
	// Passing a module == APP_MODULE_INVALID will cause this to return NULL always
	// returns NULL if it fails
	virtual IAppSystem *AddSystem( AppModule_t module, const char *pInterfaceName ) = 0;

	// Finds a system in the group..
	virtual void *FindSystem( const char *pSystemName ) = 0;

	// Gets at a factory that works just like FindSystem
	virtual CreateInterfaceFn GetFactory() = 0;
};


//-----------------------------------------------------------------------------
// Create a window (windows apps only)..
//-----------------------------------------------------------------------------
///bool CreateAppWindow( char const *pTitle, bool bWindowed, int w, int h );
//void *GetAppWindow();
void *GetAppInstance();


//-----------------------------------------------------------------------------
// NOTE: The following methods may be implemented in your application
// although you need not implement them all...
//-----------------------------------------------------------------------------
class IApplication
{
public:
	// An installed application creation function, you should tell the group
	// the DLLs and the singleton interfaces you want to instantiate.
	// Return false if there's any problems and the app will abort
	virtual bool Create( IAppSystemGroup *pAppSystemGroup ) = 0;

	// Allow the application to do some work after AppSystems are connected but 
	// they are all Initialized.
	// Return false if there's any problems and the app will abort
	virtual bool PreInit( IAppSystemGroup *pAppSystemGroup ) = 0;

	// Main loop implemented by the application
	virtual void Main() = 0;

	// Allow the application to do some work after all AppSystems are shut down
	virtual void PostShutdown() = 0;

	// Call an installed application destroy function, occurring after all modules
	// are unloaded
	virtual void Destroy() = 0;
};

#define DEFINE_APPLICATION_OBJECT( _className )	\
	static _className *s_ApplicationObject;		\
	IApplication *__g_pApplicationObject = &s_ApplicationObject

#define DEFINE_APPLICATION_OBJECT_GLOBALVAR( _globalVarName ) \
	IApplication *__g_pApplicationObject = &_globalVarName

 
#endif // APPFRAMEWORK_H
