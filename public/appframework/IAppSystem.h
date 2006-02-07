//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: An application framework 
//
// $Revision: $
// $NoKeywords: $
//=============================================================================//

#ifndef IAPPSYSTEM_H
#define IAPPSYSTEM_H

#ifdef _WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Client systems are singleton objects in the client codebase responsible for
// various tasks
// The order in which the client systems appear in this list are the
// order in which they are initialized and updated. They are shut down in
// reverse order from which they are initialized.
//-----------------------------------------------------------------------------

enum InitReturnVal_t
{
	INIT_FAILED = 0,
	INIT_OK,
//	INIT_TRY_LATER,
};


class IAppSystem
{
public:
	// Here's where the app systems get to learn about each other 
	virtual bool Connect( CreateInterfaceFn factory ) = 0;
	virtual void Disconnect() = 0;

	// Here's where systems can access other interfaces implemented by this object
	// Returns NULL if it doesn't implement the requested interface
	virtual void *QueryInterface( const char *pInterfaceName ) = 0;

	// Init, shutdown
	virtual InitReturnVal_t Init() = 0;
	virtual void Shutdown() = 0;
};



#endif // IAPPSYSTEM_H

