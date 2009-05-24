//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

// This header defines the interface convention used in the valve engine.
// To make an interface and expose it:
//    1. The interface must be ALL pure virtuals, and have no data members.
//    2. Define a name for it.
//    3. In its implementation file, use EXPOSE_INTERFACE or EXPOSE_SINGLE_INTERFACE.

// Versioning
// There are two versioning cases that are handled by this:
// 1. You add functions to the end of an interface, so it is binary compatible with the previous interface. In this case, 
//    you need two EXPOSE_INTERFACEs: one to expose your class as the old interface and one to expose it as the new interface.
// 2. You update an interface so it's not compatible anymore (but you still want to be able to expose the old interface 
//    for legacy code). In this case, you need to make a new version name for your new interface, and make a wrapper interface and 
//    expose it for the old interface.

// Static Linking:
// Must mimic unique seperate class 'InterfaceReg' constructors per subsystem.
// Each subsystem can then import and export interfaces as expected.
// This is achieved through unique namespacing 'InterfaceReg' via symbol _SUBSYSTEM.
// Static Linking also needs to generate unique symbols per interface so as to
// provide a 'stitching' method whereby these interface symbols can be referenced
// via the lib's primary module (usually the lib's interface exposure)
// therby stitching all of that lib's code/data together for eventual final exe link inclusion.

#ifndef INTERFACE_H
#define INTERFACE_H

#ifdef _WIN32
#pragma once
#endif

#ifdef _LINUX
#include <dlfcn.h> // dlopen,dlclose, et al
#include <unistd.h>

#define HMODULE void *
#define GetProcAddress dlsym

#define _snprintf snprintf
#endif

// TODO: move interface.cpp into tier0 library.
#include "tier0/platform.h"

// All interfaces derive from this.
class IBaseInterface
{
public:
	virtual	~IBaseInterface() {}
};


#define CREATEINTERFACE_PROCNAME	"CreateInterface"
typedef void* (*CreateInterfaceFn)(const char *pName, int *pReturnCode);
typedef void* (*InstantiateInterfaceFn)();



// Used internally to register classes.
class InterfaceReg
{
public:
	InterfaceReg(InstantiateInterfaceFn fn, const char *pName);

public:
	InstantiateInterfaceFn	m_CreateFn;
	const char				*m_pName;

	InterfaceReg			*m_pNext; // For the global list.
	static InterfaceReg		*s_pInterfaceRegs;
};

#if defined(_STATIC_LINKED) && defined(_SUBSYSTEM)

#endif

// Use this to expose an interface that can have multiple instances.
// e.g.:
// EXPOSE_INTERFACE( CInterfaceImp, IInterface, "MyInterface001" )
// This will expose a class called CInterfaceImp that implements IInterface (a pure class)
// clients can receive a pointer to this class by calling CreateInterface( "MyInterface001" )
//
// In practice, the shared header file defines the interface (IInterface) and version name ("MyInterface001")
// so that each component can use these names/vtables to communicate
//
// A single class can support multiple interfaces through multiple inheritance
//
// Use this if you want to write the factory function.
#if !defined(_STATIC_LINKED) || !defined(_SUBSYSTEM)
#define EXPOSE_INTERFACE_FN(functionName, interfaceName, versionName) \
	static InterfaceReg __g_Create##interfaceName##_reg(functionName, versionName);
#else
#define EXPOSE_INTERFACE_FN(functionName, interfaceName, versionName) \
	namespace _SUBSYSTEM \
	{	\
		static InterfaceReg __g_Create##interfaceName##_reg(functionName, versionName); \
	}
#endif

#if !defined(_STATIC_LINKED) || !defined(_SUBSYSTEM)
#define EXPOSE_INTERFACE(className, interfaceName, versionName) \
	static void* __Create##className##_interface() {return (interfaceName *)new className;} \
	static InterfaceReg __g_Create##className##_reg(__Create##className##_interface, versionName );
#else
#define EXPOSE_INTERFACE(className, interfaceName, versionName) \
	namespace _SUBSYSTEM \
	{	\
		static void* __Create##className##_interface() {return (interfaceName *)new className;} \
		static InterfaceReg __g_Create##className##_reg(__Create##className##_interface, versionName ); \
	}
#endif

// Use this to expose a singleton interface with a global variable you've created.
#if !defined(_STATIC_LINKED) || !defined(_SUBSYSTEM)
#define EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, globalVarName) \
	static void* __Create##className##interfaceName##_interface() {return (interfaceName *)&globalVarName;} \
	static InterfaceReg __g_Create##className##interfaceName##_reg(__Create##className##interfaceName##_interface, versionName);
#else
#define EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, globalVarName) \
	namespace _SUBSYSTEM \
	{ \
		static void* __Create##className##interfaceName##_interface() {return (interfaceName *)&globalVarName;} \
		static InterfaceReg __g_Create##className##interfaceName##_reg(__Create##className##interfaceName##_interface, versionName); \
	}
#endif

// Use this to expose a singleton interface. This creates the global variable for you automatically.
#if !defined(_STATIC_LINKED) || !defined(_SUBSYSTEM)
#define EXPOSE_SINGLE_INTERFACE(className, interfaceName, versionName) \
	static className __g_##className##_singleton; \
	EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, __g_##className##_singleton)
#else
#define EXPOSE_SINGLE_INTERFACE(className, interfaceName, versionName) \
	namespace _SUBSYSTEM \
	{	\
		static className __g_##className##_singleton; \
	}	\
	EXPOSE_SINGLE_INTERFACE_GLOBALVAR(className, interfaceName, versionName, __g_##className##_singleton)
#endif

// This function is automatically exported and allows you to access any interfaces exposed with the above macros.
// if pReturnCode is set, it will return one of the following values
// extend this for other error conditions/code

// load/unload components
class CSysModule;

// interface return status
enum 
{
	IFACE_OK = 0,
	IFACE_FAILED
};

#if defined(_STATIC_LINKED) && defined(_SUBSYSTEM)

#endif

//-----------------------------------------------------------------------------
// This function is automatically exported and allows you to access any interfaces exposed with the above macros.
// if pReturnCode is set, it will return one of the following values (IFACE_OK, IFACE_FAILED)
// extend this for other error conditions/code
//-----------------------------------------------------------------------------
DLL_EXPORT void* CreateInterface(const char *pName, int *pReturnCode);

extern CreateInterfaceFn	Sys_GetFactoryThis( void );


//-----------------------------------------------------------------------------
// UNDONE: This is obsolete, use the module load/unload/get instead!!!
//-----------------------------------------------------------------------------
extern CreateInterfaceFn	Sys_GetFactory( const char *pModuleName );


//-----------------------------------------------------------------------------
// Load & Unload should be called in exactly one place for each module
// The factory for that module should be passed on to dependent components for
// proper versioning.
//-----------------------------------------------------------------------------
extern CSysModule			*Sys_LoadModule( const char *pModuleName );
extern void					Sys_UnloadModule( CSysModule *pModule );

extern CreateInterfaceFn	Sys_GetFactory( CSysModule *pModule );

// This is a helper function to load a module, get its factory, and get a specific interface.
// You are expected to free all of these things.
// Returns false and cleans up if any of the steps fail.
bool Sys_LoadInterface(
	const char *pModuleName,
	const char *pInterfaceVersionName,
	CSysModule **pOutModule,
	void **pOutInterface );


#endif



