/********************************************************************
	created:	2005/12/27
	created:	27:12:2005   20:41
	filename: 	f:\cvs\code\stats\ff_socks.cpp
	file path:	f:\cvs\code\stats
	file base:	ff_socks
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "ff_socks.h"

#pragma comment(lib, "wsock32.lib") 
#pragma warning(disable : 4005)		// Macro redefinition

#ifdef _WIN32
	#include <winsock.h>		// For socket(), connect(), send(), and recv() 
	typedef int socklen_t;

	typedef char raw_type;		// Type used for raw data on this platform
#else
	#include <sys/types.h>		// For data types
	#include <sys/socket.h>		// For socket(), connect(), send(), and recv() 
	#include <netdb.h>			// For gethostbyname() 
	#include <arpa/inet.h>		// For inet_addr() 
	#include <unistd.h>			// For close() 
	#include <fcntl.h>			// For fcntl() 
	#include <netinet/in.h>		// For sockaddr_in

	typedef void raw_type;		// Type used for raw data on this platform
#endif

#include <errno.h>
#include <string.h>

#include "tier0/memdbgon.h"

#ifdef _WIN32
	static bool wsinit = false;
	static unsigned long nonblocking = 1;
#endif

#define TIMEOUT		3

/**
* Constructor
*/
Socks::Socks() 
{
	m_iSocket = -1;
}

/**
* Destructor
*/
Socks::~Socks() 
{
	Close();
}

/**
* Creates a socket, initialises winsock(_WIN32) 
*
* @param type Type of connection
* @param protocol Protocol to be used
*/
bool Socks::Open(int type, int protocol) 
{
#ifdef _WIN32
	if (!wsinit) 
	{
		WORD wVersionRequested;
		WSADATA wsaData;

		// Request WinSock v2.0
		wVersionRequested = MAKEWORD(2, 0);

		// Load winsock
		if (WSAStartup(wVersionRequested, &wsaData) != 0) 
			return false;

		wsinit = true;
	}
#endif

	// Make a new socket
	if ((m_iSocket = socket(PF_INET, type, protocol)) < 0) 
		return false;

#ifdef _WIN32
	// Try to make the socket non-blocking(wsock version) 
	ioctlsocket(m_iSocket, FIONBIO, &nonblocking);
#else
	// Try to make the socket non blocking(linux socket version) 
	fcntl(m_iSocket, F_SETFL, O_NONBLOCK);
#endif

	return true;
}

/**
* Connect to remote host
*
* @param hostname Remote hostname
* @param port Remote port
*/
bool Socks::Connect(const char *hostname, unsigned short port) 
{
	// Get the address of the requested host
	sockaddr_in destAddr;

	// Internet address
	destAddr.sin_family = AF_INET;
	
	// Assign port in network byte order
	destAddr.sin_port = htons(port);

	// Resolve name
	hostent *host;

	// Abandon if we can't
	if ((host = gethostbyname(hostname)) == NULL) 
		return false;

	// Get the address
	destAddr.sin_addr.s_addr = * ((unsigned long *) host->h_addr_list[0]);

	// This is our timeout thing
	fd_set socks;
	FD_ZERO(&socks);
	FD_SET(m_iSocket, &socks);

	// A 2 second timeout
	struct timeval timeout;
	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;

	// Now attempt a connect
	connect(m_iSocket, (sockaddr *) &destAddr, sizeof(destAddr));

	int readsocks = select(m_iSocket + 1, (fd_set *) 0, &socks, (fd_set *) 0, &timeout);

	// We connected
	if (readsocks > 0) 
		return true;

	return false;
}

/**
* Send data
*
* @param buffer Data to send
*/
bool Socks::Send(const char *buffer) 
{
	fd_set socks;

	FD_ZERO(&socks);
	FD_SET(m_iSocket, &socks);

	struct timeval timeout;
	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;

	// We're waiting here for the socket to be writeable
	int writesocks = select(m_iSocket + 1, (fd_set *) 0, &socks, (fd_set *) 0, &timeout);

	// No sockets ready with data, so abandon plan
	if (writesocks < 1) 
		return false;

	// Send data then
	if ((send(m_iSocket, (raw_type *) buffer, strlen(buffer), 0)) < 0) 
		return false;

	return true;
}

/**
* Check if there is data to recieve
*
* @param buffer Buffer to store received data
* @param bufferlen Length of buffer
*/
bool Socks::CheckBuffer() 
{
	fd_set socks;

	FD_ZERO(&socks);
	FD_SET(m_iSocket, &socks);

	struct timeval timeout;
	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;

	// We're waiting here for data to be available
	int readsocks = select(m_iSocket + 1, &socks, (fd_set *) 0, (fd_set *) 0, &timeout);

	// No sockets ready with data, so abandon plan
	if (readsocks < 1)
		return false;

	return true;
}

/**
* Receive data without checking if there is data to recieve
*
* @param buffer Buffer to store received data
* @param bufferlen Length of buffer
*/
int Socks::RecvNoCheck(void *buffer, int bufferlen) 
{
	// Get whatever data there was
	return recv(m_iSocket, (raw_type *) buffer, bufferlen, 0);
}

/**
* Receive data
*
* @param buffer Buffer to store received data
* @param bufferlen Length of buffer
*/
int Socks::Recv(void *buffer, int bufferlen) 
{
	fd_set socks;

	FD_ZERO(&socks);
	FD_SET(m_iSocket, &socks);

	struct timeval timeout;
	timeout.tv_sec = TIMEOUT;
	timeout.tv_usec = 0;

	// We're waiting here for data to be available
	int readsocks = select(m_iSocket + 1, &socks, (fd_set *) 0, (fd_set *) 0, &timeout);

	// No sockets ready with data, so abandon plan
	if (readsocks < 1)
		return 0;

	// Get whatever data there was
	return recv(m_iSocket, (raw_type *) buffer, bufferlen, 0);
}

/**
* Close connection
*/
bool Socks::Close() 
{
	if (m_iSocket == -1) 
		return true;

#ifdef _WIN32
	if (closesocket(m_iSocket)) 
#else
	if (close(m_iSocket)) 
#endif
		return false;

	m_iSocket = -1;
	return true;
}
