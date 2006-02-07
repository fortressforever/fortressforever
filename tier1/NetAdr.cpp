//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
// NetAdr.cpp: implementation of the CNetAdr class.
//
//////////////////////////////////////////////////////////////////////

#include "netadr.h"
#include "vstdlib/strtools.h"

#if defined( _WIN32 )

#define WIN32_LEAN_AND_MEAN
#include <winsock.h>

typedef int socklen_t;

#else
#include <netinet/in.h> // ntohs()
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

bool netadr_t::CompareAdr (const netadr_t &a, bool onlyBase) const
{
	if ( a.type != type )
		return false;

	if ( type == NA_LOOPBACK )
		return true;

	if ( type == NA_BROADCAST )
		return true;

	if ( type == NA_IP )
	{
		if ( !onlyBase && (port != a.port) )
			return false;

		if ( a.ip[0] == ip[0] && a.ip[1] == ip[1] && a.ip[2] == ip[2] && a.ip[3] == ip[3] )
			return true;
	}

	return false;
}

bool netadr_t::CompareClassBAdr (const netadr_t &a) const
{
	if ( a.type != type )
		return false;

	if ( type == NA_LOOPBACK )
		return true;

	if ( type == NA_IP )
	{
		if (a.ip[0] == ip[0] && a.ip[1] == ip[1] )
			return true;
	}

	return false;
}

// reserved addresses are not routeable, so they can all be used in a LAN game
bool netadr_t::IsReservedAdr () const
{
	if ( type == NA_LOOPBACK )
		return true;

	if ( type == NA_IP )
	{
		if ( (ip[0] == 10) ||									// 10.x.x.x is reserved
			 (ip[0] == 127) ||									// 127.x.x.x 
			 (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) ||	// 172.16.x.x  - 172.31.x.x 
			 (ip[0] == 192 && ip[1] >= 168) ) 					// 192.168.x.x
			return true;
	}
	return false;
}

const char * netadr_t::ToString(bool baseOnly) const
{
	static	char	s[64];

	Q_strncpy (s, "unknown", sizeof( s ) );

	if (type == NA_LOOPBACK)
	{
		Q_strncpy (s, "loopback", sizeof( s ) );
	}
	else if (type == NA_BROADCAST)
	{
		Q_strncpy (s, "broadcast", sizeof( s ) );
	}
	else if (type == NA_IP)
	{
		if ( baseOnly)
		{
			Q_snprintf (s, sizeof( s ), "%i.%i.%i.%i", ip[0], ip[1], ip[2], ip[3]);
		}
		else
		{
			Q_snprintf (s, sizeof( s ), "%i.%i.%i.%i:%i", ip[0], ip[1], ip[2], ip[3], ntohs(port));
		}
	}

	return s;
}

bool netadr_t::IsLocalhost() const
{
	// are we 127.0.0.1 ?
	return (ip[0] == 127) && (ip[1] == 0) && (ip[2] == 0) && (ip[3] == 1);
}

bool netadr_t::IsLoopback() const
{
	// are we useding engine loopback buffers
	return type == NA_LOOPBACK;
}

void netadr_t::Clear()
{
	ip[0] = ip[1] = ip[2] = ip[3] = 0;
	port = 0;
	type = NA_NULL;
}

void netadr_t::SetType(netadrtype_t newtype)
{
	type = newtype;
}

netadrtype_t netadr_t::GetType() const
{
	return type;
}

unsigned short netadr_t::GetPort() const
{
	return BigShort( port );
}

void netadr_t::ToSockadr (struct sockaddr * s) const
{
	Q_memset ( s, 0, sizeof(struct sockaddr));

	if (type == NA_BROADCAST)
	{
		((struct sockaddr_in*)s)->sin_family = AF_INET;
		((struct sockaddr_in*)s)->sin_port = port;
		((struct sockaddr_in*)s)->sin_addr.s_addr = INADDR_BROADCAST;
	}
	else if (type == NA_IP)
	{
		((struct sockaddr_in*)s)->sin_family = AF_INET;
		((struct sockaddr_in*)s)->sin_addr.s_addr = *(int *)&ip;
		((struct sockaddr_in*)s)->sin_port = port;
	}
	else if (type == NA_LOOPBACK )
	{
		((struct sockaddr_in*)s)->sin_family = AF_INET;
		((struct sockaddr_in*)s)->sin_port = port;
		((struct sockaddr_in*)s)->sin_addr.s_addr = INADDR_LOOPBACK ;
	}
}

bool netadr_t::SetFromSockadr(const struct sockaddr * s)
{
	if (s->sa_family == AF_INET)
	{
		type = NA_IP;
		*(int *)&ip = ((struct sockaddr_in *)s)->sin_addr.s_addr;
		port = ((struct sockaddr_in *)s)->sin_port;
		return true;
	}
	else
	{
		Clear();
		return false;
	}
}

bool netadr_t::IsValid() const
{
	return ( (port !=0 ) && (type != NA_NULL) &&
			 ( ip[0] != 0 || ip[1] != 0 || ip[2] != 0 || ip[3] != 0 ) );
}

#ifdef _WIN32
#undef SetPort	// get around stupid WINSPOOL.H macro
#endif

void netadr_t::SetPort(unsigned short newport)
{
	port = BigShort( newport );
}


