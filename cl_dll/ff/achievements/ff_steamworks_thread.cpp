#include "cbase.h"
#include "ff_socks.h"
#include "ff_steamworks_thread.h"

// supressing macro redefinition warnings
#undef ARRAYSIZE
#undef GetCommandLine
#undef ReadConsoleInput
#undef RegCreateKey
#undef RegCreateKeyEx
#undef RegOpenKey
#undef RegOpenKeyEx
#undef RegQueryValue
#undef RegQueryValueEx
#undef RegSetValue
#undef RegSetValueEx
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

CFFSteamworksThread::CFFSteamworksThread( void ) : m_iPollRate(500)
{
	SetName("SteamworksThread");
	m_bIsRunning = false;
	Start();
}

CFFSteamworksThread::~CFFSteamworksThread( void )
{
	ShutdownServer( );
}

bool CFFSteamworksThread::CreateServerProcess( void )
{
	return true;
}

void CFFSteamworksThread::QueueMessage( const CFFSteamworksMessage &msg )
{
	m_QueuedMessages.AddToTail( msg );
}

void CFFSteamworksThread::ShutdownServer( void ) 
{
	m_Sock.Close( );
	m_bIsShutdown = true;
	// TODO: KillServerProcess( );
	// avoid race conditions
	Terminate( );
	
}

int CFFSteamworksThread::Run()
{
	if ( !CreateServerProcess() )
		return -1;

	m_bIsRunning = true;
	Sleep(50);

	// not sure what this is but it triggers winsocks init
	m_Sock.Open( 1, 0 );

	if ( !m_Sock.Connect("localhost", 7802) )
	{
		m_bIsRunning = false;
		return -2;
	}
	
	QueueMessage(CFFSteamworksMessage(SWC_HEARTBEAT, "key", "val"));
	//bool anyDataAvailable = false;
	while ( IsAlive( ) )
	{
		if (m_bIsShutdown)
		{
			m_bIsRunning = false;
			return 1;
		}

		int queueCount = m_QueuedMessages.Count( );
		if (queueCount < 1)
		{
			Sleep ( m_iPollRate );
			continue;
		}

		for (int i = 0; i < queueCount; ++i)
		{
			CFFSteamworksMessage &msg = m_QueuedMessages.Element ( i );
			const int maxSize = 1024;
			char buff[maxSize];
			Q_memset( buff, maxSize, 0 );
			Q_snprintf( buff, maxSize,"%d|%s|%s!",(int)msg.GetCommand( ), msg.GetKey( ), msg.GetVal( ) );
			m_Sock.Send( buff );
		}

		m_QueuedMessages.RemoveAll( );
	}

	m_Sock.Close( );
	m_bIsRunning = false;
	return 1;
}

CFFSteamworksThread& CFFSteamworksThread::GetInstance()
{
	static CFFSteamworksThread thread;
	return thread;
}
