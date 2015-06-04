#include "cbase.h"
#include "ff_steamworks_msg.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

CFFSteamworksMessage::CFFSteamworksMessage( SteamworksCommand_e eCmd, const char *key, const char *val ) : m_cmd(eCmd), 
m_strKey(key), m_strValRaw(val)
{
}

CFFSteamworksMessage::~CFFSteamworksMessage( ) 
{
}

void CFFSteamworksMessage::GetNetworkFormat( char* buff ) const 
{
	// 12am protection
	if ( !buff )
		return;

	int size = sizeof( buff );
	if (size < 512) 
		return;
	Q_memset( buff, size, 0 );
	Q_snprintf( buff, size,"%d|%s|%s!",(int)m_cmd, m_strKey, m_strValRaw );
}

const char * CFFSteamworksMessage::GetKey( ) const 
{
	return m_strKey.c_str( );
}

const char * CFFSteamworksMessage::GetVal( ) const 
{
	return m_strValRaw.c_str( );
}
