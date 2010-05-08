//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_irc_thread.cpp
//	@author Ryan Liptak (squeek)
//	@date 30/01/2010
//	@brief Thread for the IRC server responses
//
//	REVISIONS
//	---------
//	30/01/2010, squeek: 
//		First created

#include "cbase.h"
#include "ff_irc.h"

#include "irc/ff_socks.h"
#include "ff_irc_thread.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

extern Socks g_IRCSocket;
extern CFFIRCPanel *g_pIRCPanel;
extern CFFIRCConnectPanel *g_pIRCConnectPanel;

//-----------------------------------------------------------------------------
// CFFIRCThread
//-----------------------------------------------------------------------------

CFFIRCThread::CFFIRCThread( void )
{
	SetName("IRCThread");
	m_bIsRunning = false;
	Start();
}

CFFIRCThread::~CFFIRCThread( void )
{
}

int CFFIRCThread::Run()
{
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

	m_bIsRunning = false;
/*
	int i=0;
	while(IsAlive())
	{
		i++;
		Sleep(1000);
		Msg("[IRCThread] %d...\n", i);
	}*/

	return 1;
}

CFFIRCThread& CFFIRCThread::GetInstance()
{
	static CFFIRCThread ircthread;
	return ircthread;
}
