//====== Copyright © 1996-2004, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef ISTEAMUSER_H
#define ISTEAMUSER_H
#ifdef _WIN32
#pragma once
#endif

// handle to an asyncronous call
typedef int32 HAsyncCall;

enum EAsyncResult
{
        k_EAsyncResultStillWorking = 0,
        k_EAsyncResultNoSuchAsyncCall,
        k_EAsyncResultFailed,
        k_EAsyncResultSucceeded,
};

class IVConn;

//-----------------------------------------------------------------------------
// Purpose: types of VAC bans
//-----------------------------------------------------------------------------
enum EVACBan
{
	k_EVACBanGoldsrc,
	k_EVACBanSource,
	k_EVACBanDayOfDefeatSource,
};

// game server flags
const uint k_unServerFlagNone		= 0x00;
const uint k_unServerFlagActive		= 0x01;
const uint k_unServerFlagSecure		= 0x02;
const uint k_unServerFlagDedicated	= 0x04;
const uint k_unServerFlagLinux		= 0x08;
const uint k_unServerFlagPassworded	= 0x10;

enum ELogonState
{
	k_ELogonStateNotLoggedOn = 0,
	k_ELogonStateLoggingOn = 1,
	k_ELogonStateLoggingOff = 2,
	k_ELogonStateLoggedOn = 3
};


//-----------------------------------------------------------------------------
// Purpose: Functions for accessing and manipulating a steam account
//			associated with one client instance
//-----------------------------------------------------------------------------
class ISteamUser
{
public:
	// initializes use of this interface
	// pSteam2Auth is currently required
	virtual void Init( ICMCallback *pCMCallback, class ISteam2Auth *pSteam2Auth ) = 0;

	// async call processing
	virtual EAsyncResult ProcessCall( HAsyncCall hAsyncCall ) = 0;

	// steam account management functions
	virtual void LogOn( CSteamID & steamID ) = 0;
	virtual void LogOff() = 0;
	virtual bool BLoggedOn() = 0;
	virtual ELogonState GetLogonState() = 0;
	virtual bool BConnected() = 0;

	virtual HAsyncCall CreateAccount( const char *pchAccountName, SHADigest_t PasswordHash, Salt_t PasswordSalt,
						const char *pchEmailAddress, EPersonalQuestion EPersonalQuestion, SHADigest_t PersonalAnswerHash ) = 0;

	// account state

	// returns true if this account is VAC banned from the specified ban set
	virtual bool IsVACBanned( EVACBan eVACBan ) = 0;

	// returns true if the user needs to see the newly-banned message from the specified ban set
	virtual bool RequireShowVACBannedMessage( EVACBan eVACBan ) = 0;

	// tells the server that the user has seen the 'you have been banned' dialog
	virtual void AcknowledgeVACBanning( EVACBan eVACBan ) = 0;

	// Game Server functions
	// bugbug johnc these should be moved into a different interface
	virtual bool GSSendLogonRequest( CSteamID & steamID ) = 0;
	virtual bool GSSendDisconnect( CSteamID & steamID ) = 0;
	virtual bool GSSendStatusResponse( CSteamID & steamID, int nSecondsConnected, int nSecondsSinceLast ) = 0;
	virtual bool GSSetStatus( int32 unAppIdServed, uint unServerFlags, int cPlayers, int cPlayersMax, int cFakePlayers, int unGamePort, const char *pchServerName, const char *pchGameDir, const char *pchMapName, const char *pchVersion ) = 0;

	// registering/unregistration game launches functions
	// unclear as to where these should live
	virtual int NClientGameIDAdd( int nGameID ) = 0;
	virtual void RemoveClientGame( int nClientGameID )  = 0;
	virtual void SetClientGameServer( int nClientGameID, uint unIPServer, uint16 usPortServer ) = 0;

	// test functions
	// bugbug johnc these should be moved into a different test-specific interface
	virtual void Test_SuspendActivity() = 0;
	virtual void Test_ResumeActivity() = 0;

	virtual bool Test_SendVACResponse( int nClientGameID, uint8 *pubResponse, int cubResponse ) = 0;
	virtual void Test_SetFakePrivateIP( uint unIPPrivate ) = 0;
	virtual void Test_SendBigMessage() = 0;							// send a big (multi-packet) message to the server to Test packetization code
	virtual bool Test_BBigMessageResponseReceived() = 0;			// returns true if a big Test message response has been sent back from the server
	virtual void Test_SetPktLossPct( int nPct ) = 0;				// sets a simulated packet loss percentage for Testing
	virtual void Test_SetForceTCP( bool bForceTCP ) = 0;			// forces client to use TCP connection
	virtual void Test_SetMaxUDPConnectionAttempts( int nMaxUDPConnectionAttempts ) = 0;	// sets max connection attempts over UDP
	virtual void Test_Heartbeat() = 0;								// forces the client to send a heartbeat
	virtual void Test_FakeDisconnect() = 0;							// fakes a disconnect
	virtual EUniverse Test_GetEUniverse() = 0;
};

//-----------------------------------------------------------------------------
// Purpose: functions for compatibility with steam2 steam.dll
//			use as a grab-bag of functions needed to extract info from steam2 client
//			passed into steamclient from steam.dll
//-----------------------------------------------------------------------------
class ISteam2Auth
{
public:
	// gets a configuration value
	virtual bool GetValue( const char * pchName, char *pchValue, const int cchValue ) = 0;

	// called during logon to get encrypted user ticket
	virtual bool GetServerReadableTicket( unsigned int unIPPublic, unsigned int unIPPrivate, 
		void * pubTicket, unsigned int cubTicketBuf, unsigned int * pcubTicketSize ) = 0;
};


#define STEAMUSER_INTERFACE_VERSION "SteamUser002"

#endif // ISTEAMUSER_H
