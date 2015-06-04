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

const SteamworksCommand_e CFFSteamworksMessage::GetCommand() const
{
	return m_cmd;
}
const char * CFFSteamworksMessage::GetKey( ) const 
{
	return m_strKey.c_str( );
}

const char * CFFSteamworksMessage::GetVal( ) const 
{
	return m_strValRaw.c_str( );
}
