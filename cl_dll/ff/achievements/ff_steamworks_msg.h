#ifndef FF_STEAMWORKS_MSG
#define FF_STEAMWORKS_MSG

#include <string>
#include "ff_socks.h"

enum SteamworksCommand_e
{
	SWM_UNKNOWN,
	SWM_HEARTBEAT			= 1,
	SWM_SETSTAT				= 10,
	SWM_INCREMENTSTAT		= 20,
	SWM_UNLOCKACHIEVEMENT	= 30,
	SWM_RESETALL			= 99,
};

class CFFSteamworksMessage
{
public:
	const SteamworksCommand_e GetCommand( void ) const;
	const char* GetKey( void ) const;
	const char* GetVal( void ) const;
	void GetNetworkFormat ( char* buff ) const;
	//CFFSteamworksMessage(const char *rawStr);
	CFFSteamworksMessage( SteamworksCommand_e eCmd, const char *key, const char *val );
	~CFFSteamworksMessage( );
private:
	SteamworksCommand_e	m_cmd;
	std::string			m_strKey;
	std::string			m_strValRaw;
};

#endif
