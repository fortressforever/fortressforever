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
	int a = 0;
	//int i = 0;
	char buf[3000];

	m_bIsRunning = true;

	while(IsAlive())
	{
		a = g_IRCSocket.Recv(buf, sizeof(buf)-1);

		// if length of message is bigger than 1
		if (a > 1)
		{
			buf[a] = '\0';

			//Msg("%s\n", buf);

			int ichar =0;
			char *p = buf;

			// loop through each char
			for (int t=0;t<(int)strlen(buf);t++)
			{
				// break at '\r' and send it off for parsing
				if(p[ichar]=='\r')
				{
					p[ichar] = '\0';

					if (g_pIRCPanel != NULL)
						g_pIRCPanel->ParseServerMessage(p);
					
					p[ichar] = '\r';
					// '\r' char is always followed by '\n'
					p = (p+ichar+2);
					ichar=0;
					continue;
				}
				ichar++;
			}

			// send whatevers left, if it has substantial content
			if (g_pIRCPanel != NULL && strlen(p) > 1)
				g_pIRCPanel->ParseServerMessage(p);
		}

		a=0;
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
