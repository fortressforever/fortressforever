#ifndef FF_CDLL_CLIENT_INT_H
#define FF_CDLL_CLIENT_INT_H
#pragma once

#include "cdll_client_int.h"

class CHLClient;

//-----------------------------------------------------------------------------
// Purpose: engine to client .dll interface
//-----------------------------------------------------------------------------
class CFFClient : public CHLClient
{
public:
	// squeek: I couldn't get DECLARE_CLASS_NOBASE to compile in CHLClient, so I couldn't use DECLARE_CLASS here
	// this works but it could potentially have negative consequences
	DECLARE_CLASS_GAMEROOT( CFFClient, CHLClient );

	// <-- Extended functions (meaning the BaseClass function is always called)
	virtual int	Init( CreateInterfaceFn appSystemFactory, CreateInterfaceFn physicsFactory, CGlobalVarsBase *pGlobals );

	// dexter: added this for shutting down thread, socket, server reading from local steamworks server
	virtual void Shutdown( void );
	// --> Extended functions

	// <-- FF-specific functions
	void PopulateMissingClassConfigs();
	void PopulateMissingUserConfig();
	// -->
};

#endif