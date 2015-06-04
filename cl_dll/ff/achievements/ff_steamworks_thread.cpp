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

CFFSteamworksThread::CFFSteamworksThread( void )
{
	SetName("SteamworksThread");
	m_bIsRunning = false;
	Start();
}

CFFSteamworksThread::~CFFSteamworksThread( void )
{
}

bool CFFSteamworksThread::CreateServerProcess( void )
{
	return true;
}

void CFFSteamworksThread::QueueMessage( const CFFSteamworksMessage &msg )
{
	m_QueuedMessages.AddToTail( msg );
}

int CFFSteamworksThread::Run()
{
	if ( !CreateServerProcess() )
		return 0;

	m_bIsRunning = true;
	Sleep(100);

	// not sure what this is but it triggers winsocks init
	m_Sock.Open( 1, 0 );

	if ( !m_Sock.Connect("localhost", 7802) )
	{
		m_bIsRunning = false;
		return 0;
	}
	
	//bool anyDataAvailable = false;
	while ( IsAlive( ) )
	{
		int queueCount = m_QueuedMessages.Count( );
		if (queueCount < 1)
			continue;

		for (int i = 0; i < queueCount; ++i)
		{
			CFFSteamworksMessage &msg = m_QueuedMessages.Element ( i );
			char buff[1024];
			msg.GetNetworkFormat( buff );
			m_Sock.Send( buff );
		}

		m_QueuedMessages.RemoveAll( );
		//CUtlString
		Sleep( 50 );
	}
	//CreateProcess();
	/*
	bool a = false;
	//int i = 0;
	//char buf[3000];

	m_bIsRunning = true;

	while(IsAlive())
	{
		a = g_IRCSocket.CheckBuffer();

		// if length of message is bigger than 1
		if (a)
		{
			if (g_pIRCPanel != NULL)
				g_pIRCPanel->m_bDataReady = true;
		}

		a=false;
	}

	
/*
	int i=0;
	while(IsAlive())
	{
		i++;
		Sleep(1000);
		Msg("[IRCThread] %d...\n", i);
	}*/

	Sleep(100);
	m_bIsRunning = false;
	return 1;
}

CFFSteamworksThread& CFFSteamworksThread::GetInstance()
{
	static CFFSteamworksThread thread;
	return thread;
}
