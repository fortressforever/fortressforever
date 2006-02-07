//====== Copyright © 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef ISTEAMCLIENT_H
#define ISTEAMCLIENT_H
#ifdef _WIN32
#pragma once
#endif

// handle to single instance of a steam client
// bugbug johnc: rename all this to HUserSession / IUserSession so that it makes sense
typedef int32 HSteamClientUser;

// interface predec
class ISteamUser;
class IVAC;
struct StatsVConn_t;

//-----------------------------------------------------------------------------
// Purpose: Interface to creating a new steam instance, or to
//			connect to an existing steam instance, whether it's in a
//			different process or is local
//-----------------------------------------------------------------------------
class ISteamClient
{
public:
	// creates a global instance of a steam client, so that other processes can share it
	// used by the steam UI, to share it's account info/connection with any games it launches
	// fails (returns NULL) if an existing instance already exists
	virtual HSteamClientUser CreateGlobalInstance() = 0;

	// connects to an existing global instance, failing if none exists
	// used by the game to coordinate with the steamUI
    virtual HSteamClientUser ConnectToGlobalInstance() = 0;

	// used by game servers, create a steam client that won't be shared with anyone else
	virtual HSteamClientUser CreateLocalInstance() = 0;

	// removes an allocated instance
	virtual void ReleaseInstance( HSteamClientUser hUser ) = 0;

	// retrieves the ISteamUser interface associated with the handle
	virtual ISteamUser *GetISteamUser( HSteamClientUser hSteamClientUser, const char *pchVersion ) = 0;

	// retrieves the IVac interface associated with the handle
	// there is normally only one instance of VAC running, but using this connects it to the right user/account
	virtual IVAC *GetIVAC( HSteamClientUser hSteamClientUser ) = 0;

	// runs a single frame, letting the clients update and maintain themselves
	// bugbug johnc this will be removed when the client has it's own thread
	virtual bool BMainLoop( uint64 sTimeCurrentTime, bool bStressMode ) = 0;

	// test interfaces
	virtual void Test_SetSpew( const char *pchGroup, int nSpewLevel ) = 0;
	virtual void Test_SetSpewFunc( void * /*SpewOutputFunc_t*/ func ) = 0;
	virtual void Test_OverrideIPs( uint unPublicIP, uint unPrivateIP ) = 0;
	virtual void Test_SetServerLoadState( bool bBusy, bool bCritical ) = 0;
	virtual void Test_SetStressMode( bool bClientStressMode ) = 0;
	virtual StatsVConn_t &Test_GetStatsVConn() = 0;
	virtual void Test_RemoveAllClients() = 0; 
};


#define STEAMCLIENT_INTERFACE_VERSION		"SteamClient002"


#endif // ISTEAMCLIENT_H
