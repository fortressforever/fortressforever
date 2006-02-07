//========= Copyright © 1996-2004, Valve LLC, All rights reserved. ============
//
// Purpose:
//
// $NoKeywords: $
//=============================================================================

#ifndef STEAMCLIENTPUBLIC_H
#define STEAMCLIENTPUBLIC_H
#ifdef _WIN32
#pragma once
#endif
//lint -save -e1931 -e1927 -e1924 -e613 -e726

// This header file defines the interface between the calling application and the code that
// knows how to communicate with the connection manager (CM) from the Steam service 

// This header file is intended to be portable; ideally this 1 header file plus a lib or dll
// is all you need to integrate the client library into some other tree.  So please avoid
// including or requiring other header files if possible.  This header should only describe the 
// interface layer, no need to include anything about the implementation.

#include "steamtypes.h"

class ICMCallback;		// forward declaration
class IVConnCallback;	// forward declaration

// General result codes
enum EResult
{
	k_EResultOK	= 1,							// success
	k_EResultFail = 2,							// generic failure 
	k_EResultNoConnection = 3,					// no/failed network connection
//	k_EResultNoConnectionRetry = 4,				// OBSOLETE - removed
	k_EResultInvalidPassword = 5,				// password/ticket is invalid
	k_EResultLoggedInElsewhere = 6,				// same user logged in elsewhere
	k_EResultInvalidProtocolVer = 7,			// protocol version is incorrect
	k_EResultInvalidParam = 8,					// a parameter is incorrect
	k_EResultFileNotFound = 9,					// file was not found
	k_EResultBusy = 10,							// called method busy - action not taken
	k_EResultInvalidState = 11,					// called object was in an invalid state
	k_EResultInvalidName = 12,					// name is invalid
	k_EResultInvalidEmail = 13,					// email is invalid
	k_EResultDuplicateName = 14,				// name is not unique
	k_EResultAccessDenied = 15,					// access is denied
	k_EResultTimeout = 16,						// operation timed out
	k_EResultBanned = 17,						// VAC2 banned
	k_EResultAccountNotFound = 18,				// account not found
	k_EResultInvalidSteamID = 19,				// steamID is invalid
	k_EResultServiceUnavailable = 20,			// The requested service is currently unavailable
};

// Result codes to GSHandleClientDeny/Kick
typedef enum
{
	k_EDenyInvalidVersion = 1,
	k_EDenyGeneric,
	k_EDenyNotLoggedOn,
	k_EDenyNoLicense,
	k_EDenyCheater,
} EDenyReason;

// Steam universes.  Each universe is a self-contained Steam instance.
enum EUniverse
{
	k_EUniverseInvalid = 0,
	k_EUniversePublic = 1,
	k_EUniverseBeta = 2,
	k_EUniverseInternal = 3,
	k_EUniverseDev = 4,

	k_EUniverseMax
};

EUniverse EUniverseFromName( const char * pchUniverseName );
const char * PchNameFromEUniverse( EUniverse eUniverse );

// Steam account types
enum EAccountType
{
	k_EAccountTypeInvalid = 0,			
	k_EAccountTypeIndividual = 1,		// single user account
	k_EAccountTypeMultiseat = 2,		// multiseat (e.g. cybercafe) account
	k_EAccountTypeGameServer = 3,		// game server account
	k_EAccountTypeAnonGameServer = 4,	// anonomous game server account
	k_EAccountTypePending = 5			// pending
};

// Enums for all personal questions supported by the system.
enum EPersonalQuestion
{
	// Never ever change these after initial release.
	k_EPSMsgNameOfSchool = 0,			// Question: What is the name of your school?
	k_EPSMsgFavoriteTeam = 1,			// Question: What is your favorite team?
	k_EPSMsgMothersName = 2,			// Question: What is your mother's maiden name?
	k_EPSMsgNameOfPet = 3,				// Question: What is the name of your pet?
	k_EPSMsgChildhoodHero = 4,			// Question: Who was your childhood hero?
	k_EPSMsgCityBornIn = 5,				// Question: What city were you born in?

	k_EPSMaxPersonalQuestion
};

#pragma pack( push, 1 )		

// Steam ID structure (64 bits total)
class CSteamID
{
public:

	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	//-----------------------------------------------------------------------------
	CSteamID()
	{
		m_unAccountID = 0;
		m_EAccountType = k_EAccountTypeInvalid;
		m_EUniverse = k_EUniverseInvalid;
		m_unAccountInstance = 0;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	// Input  : unAccountID -	32-bit account ID
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	CSteamID( uint32 unAccountID, EUniverse eUniverse, EAccountType eAccountType )
	{
		Set( unAccountID, eUniverse, eAccountType );
	}


	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	// Input  : unAccountID -	32-bit account ID
	//			unAccountInstance - instance 
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	CSteamID( uint32 unAccountID, unsigned int unAccountInstance, EUniverse eUniverse, EAccountType eAccountType )
	{
#ifdef _SERVER
		Assert( ! ( ( k_EAccountTypeIndividual == eAccountType ) && ( 1 != unAccountInstance ) ) );	// enforce that for individual accounts, instance is always 1
#endif // _SERVER
		InstancedSet( unAccountID, unAccountInstance, eUniverse, eAccountType );
	}


	//-----------------------------------------------------------------------------
	// Purpose: Constructor
	// Input  : ulSteamID -		64-bit representation of a Steam ID
	//-----------------------------------------------------------------------------
	CSteamID( uint64 ulSteamID )
	{
		SetFromUint64( ulSteamID );
	}


	//-----------------------------------------------------------------------------
	// Purpose: Sets parameters for steam ID
	// Input  : unAccountID -	32-bit account ID
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	void Set( uint32 unAccountID, EUniverse eUniverse, EAccountType eAccountType )
	{
		m_unAccountID = unAccountID;
		m_EUniverse = eUniverse;
		m_EAccountType = eAccountType;
		m_unAccountInstance = 1;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Sets parameters for steam ID
	// Input  : unAccountID -	32-bit account ID
	//			eUniverse -		Universe this account belongs to
	//			eAccountType -	Type of account
	//-----------------------------------------------------------------------------
	void InstancedSet( uint32 unAccountID, uint32 unInstance, EUniverse eUniverse, EAccountType eAccountType )
	{
		m_unAccountID = unAccountID;
		m_EUniverse = eUniverse;
		m_EAccountType = eAccountType;
		m_unAccountInstance = unInstance;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Initializes a steam ID from its 52 bit parts and universe/type
	// Input  : ulIdentifier - 52 bits of goodness
	//-----------------------------------------------------------------------------
	void FullSet( uint64 ulIdentifier, EUniverse eUniverse, EAccountType eAccountType )
	{
		m_unAccountID = ( ulIdentifier & 0xFFFFFFFF );						// account ID is low 32 bits
		m_unAccountInstance = ( ( ulIdentifier >> 32 ) & 0xFFFFF );			// account instance is next 20 bits
		m_EUniverse = eUniverse;
		m_EAccountType = eAccountType;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Initializes a steam ID from its 64-bit representation
	// Input  : ulSteamID -		64-bit representation of a Steam ID
	//-----------------------------------------------------------------------------
	void SetFromUint64( uint64 ulSteamID )
	{
		m_unAccountID = ( ulSteamID & 0xFFFFFFFF );							// account ID is low 32 bits
		m_unAccountInstance = ( ( ulSteamID >> 32 ) & 0xFFFFF );			// account instance is next 20 bits

		m_EAccountType = ( EAccountType ) ( ( ulSteamID >> 52 ) & 0xF );	// type is next 4 bits
		m_EUniverse = ( EUniverse ) ( ( ulSteamID >> 56 ) & 0xFF );			// universe is next 8 bits
	}

	//-----------------------------------------------------------------------------
	// Purpose: Initializes a steam ID from a Steam2 ID structure
	// Input:	pTSteamGlobalUserID -	Steam2 ID to convert
	//			eUniverse -				universe this ID belongs to
	//-----------------------------------------------------------------------------
	void SetFromSteam2( TSteamGlobalUserID *pTSteamGlobalUserID, EUniverse eUniverse )
	{
		m_unAccountID = pTSteamGlobalUserID->m_SteamLocalUserID.Split.Low32bits * 2 + 
			pTSteamGlobalUserID->m_SteamLocalUserID.Split.High32bits;
		m_EUniverse = eUniverse;		// set the universe
		m_EAccountType = k_EAccountTypeIndividual; // Steam 2 accounts always map to account type of individual
		m_unAccountInstance = 1;	// individual accounts always have an account instance ID of 1
	}

	//-----------------------------------------------------------------------------
	// Purpose: Converts steam ID to its 64-bit representation
	// Output : 64-bit representation of a Steam ID
	//-----------------------------------------------------------------------------
	uint64 ConvertToUint64() const
	{
		return (uint64) ( ( ( (uint64) m_EUniverse ) << 56 ) + ( ( (uint64) m_EAccountType ) << 52 ) + 
			( ( (uint64) m_unAccountInstance ) << 32 ) + m_unAccountID );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Fills out a Steam2 ID structure
	// Input:	pTSteamGlobalUserID -	Steam2 ID to write to
	//-----------------------------------------------------------------------------
	void ConvertToSteam2( TSteamGlobalUserID *pTSteamGlobalUserID ) const
	{
		// only individual accounts have any meaning in Steam 2, only they can be mapped
		// Assert( m_EAccountType == k_EAccountTypeIndividual );

		pTSteamGlobalUserID->m_SteamInstanceID = 0;
		pTSteamGlobalUserID->m_SteamLocalUserID.Split.High32bits = m_unAccountID % 2;
		pTSteamGlobalUserID->m_SteamLocalUserID.Split.Low32bits = m_unAccountID / 2;
	}


	//-----------------------------------------------------------------------------
	// Purpose: Converts the static parts of a steam ID to a 64-bit representation.
	//			For multiseat accounts, all instances of that account will have the
	//			same static account key, so they can be grouped together by the static
	//			account key.
	// Output : 64-bit static account key
	//-----------------------------------------------------------------------------
	uint64 GetStaticAccountKey() const
	{
		// note we do NOT include the account instance (which is a dynamic property) in the static account key
		return (uint64) ( ( ( (uint64) m_EUniverse ) << 56 ) + ((uint64) m_EAccountType << 52 ) + m_unAccountID );
	}


	//-----------------------------------------------------------------------------
	// Purpose: create an anonomous game server login to be filled in by the AM
	//-----------------------------------------------------------------------------
	void CreateBlankAnonLogon( EUniverse eUniverse )
	{
		m_unAccountID = 0;
		m_EAccountType = k_EAccountTypeAnonGameServer;
		m_EUniverse = eUniverse;
		m_unAccountInstance = 0;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this an anonomous game server login that will be filled in?
	//-----------------------------------------------------------------------------
	bool BBlankAnonAccount() const
	{
		return m_unAccountID == 0 && 
			m_EAccountType == k_EAccountTypeAnonGameServer &&
			m_unAccountInstance == 0;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Is this a game server account id?
	//-----------------------------------------------------------------------------
	bool BGameServerAccount() const
	{
		return m_EAccountType == k_EAccountTypeGameServer || m_EAccountType == k_EAccountTypeAnonGameServer;
	}

	// simple accessors
	void SetAccountID( uint32 unAccountID ) { m_unAccountID = unAccountID; }
	uint32 GetAccountID() const { return m_unAccountID; }
	uint32 GetUnAccountInstance() const { return m_unAccountInstance; }
	EAccountType GetEAccountType() const { return m_EAccountType; }
	EUniverse GetEUniverse() const { return m_EUniverse; }
	bool IsValid() const { return m_EAccountType != k_EAccountTypeInvalid; }

	// this set of functions is hidden, will be moved out of class
	CSteamID( const char *pchSteamID, EUniverse eDefaultUniverse = k_EUniverseInvalid );
	char * Render() const;				// renders this steam ID to string
	static char * Render( uint64 ulSteamID );	// static method to render a uint64 representation of a steam ID to a string

	bool SetFromSteam2String( const char *pchSteam2ID, EUniverse eUniverse );

	// DEBUG function
	bool BValidExternalSteamID() const;

private:
	void SetFromString( const char *pchSteamID, EUniverse eDefaultUniverse );

	// 64 bits total
	uint32				m_unAccountID : 32;			// unique account identifier
	unsigned int		m_unAccountInstance : 20;	// dynamic instance ID (used for multiseat type accounts only)
	EAccountType		m_EAccountType : 4;			// type of account
	EUniverse			m_EUniverse : 8;			// universe this account belongs to
};

#pragma pack( pop )


// ICMCallback
// Anyone who owns a CMInterface object should implement this to receive callbacks.
class ICMCallback
{
public:
	ICMCallback() { };
	virtual ~ICMCallback() { };

	virtual void OnLogonSuccess() = 0 ;
	virtual void OnLogonFailure( EResult eResult ) = 0;
	virtual void OnLoggedOff( EResult eResult ) = 0;
	virtual void OnBeginLogonRetry() = 0;

	// Called when we receive a VAC challenge
	virtual void HandleVACChallenge( int nClientGameID, uint8 *pubChallenge, int cubChallenge ) = 0;

	// called when a GS is getting responses to requests or user status
	virtual void GSHandleClientApprove( CSteamID & steamID ) = 0;
	virtual void GSHandleClientDeny( CSteamID & steamID, EDenyReason eDenyReason ) = 0;
	virtual void GSHandleClientKick( CSteamID & steamID, EDenyReason eDenyReason ) = 0;

#ifdef _SERVER
#ifdef TEST_CODE_ENABLED
	virtual void TEST_ReportMsgReceived( int32 m_EMsg ) = 0;
	virtual int GetTestFlag( int eTestFlag ) = 0;
	virtual void SetTestFlag( int eTestFlag, int nVal ) = 0;
#endif // TEST_CODE_ENABLED
#endif // _SERVER
};


// IVAC
// This is the wrapper class for all VAC functionaility in the client
class IVAC
{
public:

#ifdef VAC_ENABLED
	virtual bool BVACCreateProcess(  
		PVOID lpVACBlob,
		DWORD  cbBlobSize,
		LPCTSTR lpApplicationName,
		LPTSTR lpCommandLine,
		LPSECURITY_ATTRIBUTES lpProcessAttributes,
		LPSECURITY_ATTRIBUTES lpThreadAttributes,
		BOOL bInheritHandles,
		DWORD dwCreationFlags,
		LPVOID lpEnvironment,
		LPCTSTR lpCurrentDirectory,
		LPSTARTUPINFO lpStartupInfo,
		LPPROCESS_INFORMATION lpProcessInformation,
		DWORD nGameID
		) = 0;
#endif

	virtual void KillAllVAC() = 0;

	virtual uint8 *PbLoadVacBlob( int *pcbVacBlob ) = 0;
	virtual void FreeVacBlob( uint8 *pbVacBlob ) = 0;

	virtual void RealHandleVACChallenge( int nClientGameID, uint8 *pubChallenge, int cubChallenge ) = 0;
};


const int k_nGameIDUnknown = -1;
// this is a bogus number picked to be beyond any real steam2 uAppID
const int k_nGameIDNotepad = 65535;
const int k_nGameIDCSSTestApp = 65534;
// this is the real steam2 uAppID for Counter-Strike Source
const int k_nGameIDCSS = 240;
const int k_nGameIDServerCSS = 220;
// this one is half life 2 deathmatch
const int k_nGameIDHL2DM = 320;
const int k_nGameIDServerHL2DM = 320;



//lint -restore

#endif // STEAMCLIENTPUBLIC_H
