#include "cbase.h"
#include "filesystem.h" 
#include "ff_socks.h"
#include "ff_steamworks_thread.h"
#include "vstdlib/icommandline.h"

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

#define SERVER_EXEC_NAME "bin/sws.exe"

// HEARTBEATS: send a heartbeat every 10 seconds (10 polls)
CFFSteamworksThread::CFFSteamworksThread( void ) : m_iPollRate(1000), m_hProcess(NULL), m_hThread(NULL), 
												m_iHeartbeatRate(10), m_iHeartbeatCheckCount(m_iHeartbeatRate)
{
	SetName("SteamworksThread");
	m_bIsRunning = false;
	Start();
}

CFFSteamworksThread::~CFFSteamworksThread( void )
{
	//ShutdownServer( );
	Terminate( );
}

bool CFFSteamworksThread::CreateServerProcess( void )
{
	if ( !filesystem->FileExists( SERVER_EXEC_NAME, "MOD" ) )
	{
		Msg( "[Steamworks thread] failed to find SW server '%s' in moddir\n", SERVER_EXEC_NAME );
		return false;
	}

	char szExecPath[MAX_PATH] = { 0 };
	filesystem->GetLocalPath( SERVER_EXEC_NAME, szExecPath, MAX_PATH );

	PROCESS_INFORMATION procInfo = { 0 };
	STARTUPINFO startUpInfo = { 0 };
	startUpInfo.cb = sizeof( startUpInfo );

	if ( !CreateProcess( NULL, szExecPath, NULL, NULL, false, 0, NULL, NULL, &startUpInfo, &procInfo ) )
	{
		Msg( "[Steamworks thread] failed to start SW server\n" );
		return false;
	}
	Msg( "[Steamworks thread] sw server started\n" );
	m_hProcess	= procInfo.hProcess;
	m_hThread	= procInfo.hThread;
	return true;
}

void CFFSteamworksThread::KillServerProcess( void ) 
{
	DevMsg( "[Steamworks thread] KillServerProcess" );
	// the server loop will actually end once the socket closes, however make sure and axe it here just incase dragons
	if ( !m_hProcess || !m_hThread)
		return;
	// yikes: results in race condition with os if thread falls through, so just let it..
	// CloseHandle( m_hProcess );
	// CloseHandle( m_hThread );
}

void CFFSteamworksThread::QueueMessage( const CFFSteamworksMessage &msg )
{
	m_QueuedMessages.AddToTail( msg );
}

void CFFSteamworksThread::ShutdownServer( void ) 
{
	DevMsg( "[Steamworks thread] ShutdownServer" );
	SendMsg( CFFSteamworksMessage( SWC_QUIT ) );
	m_Sock.Close( );
	m_bIsShutdown = true;
	KillServerProcess( );
	Terminate( );
}

int CFFSteamworksThread::Run()
{
	if ( !CreateServerProcess() )
	{
		DevMsg( "[Steamworks thread] Run - failed to create server process\n" );
		return -1;
	}

	m_bIsRunning = true;
	Sleep(50);

	// make sure our socket doesnt close between heartbeats
	//m_Sock.SetTimeoutSecs( m_iHeartbeatRate * 2 ); 
	m_Sock.Open( 1, 6 );//SOCK_STREAM, IPPROTO_TCP );

	if ( !m_Sock.Connect("localhost", 7802) )
	{
		DevMsg( "[Steamworks thread] Run2 - failed to connect to server\n" );
		m_bIsRunning = false;
		return -2;
	}
	
	while ( IsAlive( ) && !m_bIsShutdown )
	{
		if ( ++m_iHeartbeatCheckCount >= m_iHeartbeatRate )
		{
			m_iHeartbeatCheckCount = 0;
			SendMsg( CFFSteamworksMessage( SWC_HEARTBEAT ) );
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
			SendMsg( msg );
		}

		m_QueuedMessages.RemoveAll( );
		Sleep ( m_iPollRate );
	}

	DevMsg( "[Steamworks thread] Run3 - fallthrough\n" );
	SendMsg( CFFSteamworksMessage( SWC_QUIT ) );
	m_Sock.Close( );
	m_bIsRunning = false;
	return 1;
}

void CFFSteamworksThread::SendMsg( const CFFSteamworksMessage &msg )
{
	const int maxSize = 1024;
	char buff[maxSize] = { 0 };
	Q_snprintf( buff, maxSize,"%d|%s|%s!",(int)msg.GetCommand( ), msg.GetKey( ), msg.GetVal( ) );
	DevMsg( "[Steamworks thread] SendMsg - '%s'\n", buff );

	if ( !m_Sock.Send( buff ) )
	{
		// make sure to check this here, it will return false and we want to fall through
		m_bIsShutdown = true; 
	}
}

CFFSteamworksThread& CFFSteamworksThread::GetInstance()
{
	static CFFSteamworksThread thread;
	return thread;
}
