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


CFFSteamworksThread::CFFSteamworksThread( void ) : m_iPollRate(500), m_hProcess(NULL), m_hThread(NULL)
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
	//	return;
}

void CFFSteamworksThread::QueueMessage( const CFFSteamworksMessage &msg )
{
	m_QueuedMessages.AddToTail( msg );
}

void CFFSteamworksThread::ShutdownServer( void ) 
{
	DevMsg( "[Steamworks thread] ShutdownServer" );
	m_Sock.Close( );
	m_bIsShutdown = true;
	KillServerProcess ( );
	// avoid race conditions
	if ( m_bIsRunning )
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

	// not sure what this is but it triggers winsocks init
	m_Sock.Open( 1, 0 );

	if ( !m_Sock.Connect("localhost", 7802) )
	{
		DevMsg( "[Steamworks thread] Run2 - failed to connect to server\n" );
		m_bIsRunning = false;
		return -2;
	}
	
	QueueMessage(CFFSteamworksMessage(SWC_HEARTBEAT, "key", "val"));
	
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

	DevMsg( "[Steamworks thread] Run3 - fallthrough\n" );
	m_Sock.Close( );
	m_bIsRunning = false;
	return 1;
}


CFFSteamworksThread& CFFSteamworksThread::GetInstance()
{
	static CFFSteamworksThread thread;
	return thread;
}
