
#include "cbase.h"
#include "ff_irc.h"

#include "irc/ff_socks.h"
#include "tier0/threadtools.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

using namespace vgui;

//threadtest.h
Socks irc_socket;

CFFIRCPanel *g_pIRCPanel = NULL;

struct cUser
{
	char nick[30]; 
	char ident[100];
	char email[100];
	char status;
};
cUser irc_user;

//-----------------------------------------------------------------------------
// CFFIRCLobbyTab
//-----------------------------------------------------------------------------

class CFFIRCLobbyTab : public CFFIRCTab
{
	DECLARE_CLASS_SIMPLE(CFFIRCLobbyTab, CFFIRCTab);

public:
	
	CFFIRCLobbyTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
	{
		m_pTextEntry_NickEntry = new vgui::TextEntry(this, "TextEntry_NickEntry");
		m_pTextEntry_NickEntry->SetText("tempnick123");

		m_pGameList = new ListPanel(this, "ListPanel_GameList");

		m_pGameList->AddActionSignalTarget( this );
		
        m_pGameList->AddColumnHeader( 0, "players" , "Players" , 20, ListPanel::COLUMN_RESIZEWITHWINDOW );
        m_pGameList->AddColumnHeader( 1, "name" , "Game Name" , 200, ListPanel::COLUMN_RESIZEWITHWINDOW );
		m_pGameList->AddColumnHeader( 2, "map" , "Map Name" , 150, ListPanel::COLUMN_RESIZEWITHWINDOW  );	// |-- Mirv: Current class

		KeyValues *kv = new KeyValues( "LI" );
		kv->SetString( "players", "1/8" );
		kv->SetString( "name", "4v4 1500+ pros" );
		kv->SetString( "map", "ff_monkey" );
		m_pGameList->AddItem( kv, 0, false, false );
		kv->deleteThis();

		LoadControlSettings("resource/ui/FFIRCLobbyTab.res");
	}
	
	virtual void ApplySchemeSettings( IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		m_pGameList->SetFont( pScheme->GetFont( "DefaultVerySmall" ) );
	}
	
	MESSAGE_FUNC_PARAMS(OnNewLineMessage, "TextNewLine", data)
	{
		char szCommand[256];
		m_pTextEntry_ChatEntry->GetText(szCommand, sizeof(szCommand));
		
		if ( !irc_socket.Send( VarArgs("PRIVMSG #fortressforever :%s\r\n", szCommand) ) )
			Msg("[IRC] Unable to send message: %s\n", szCommand);
	                 
		//const char* text = data->GetString("text");
		m_pRichText_Chat->InsertString( VarArgs("%s: %s",irc_user.nick, szCommand) );
		m_pRichText_Chat->InsertString("\n");
		m_pTextEntry_ChatEntry->SetText("");
		m_pRichText_Chat->GotoTextEnd();
	}

	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data)
	{
		const char *pszCommand = data->GetString("command");

		if (Q_strcmp(pszCommand, "AddGame") == 0)
		{
			CFFIRCPanel *parent = dynamic_cast <CFFIRCPanel *> (GetParent()->GetParent());

			if (parent)
			{
				parent->AddGameTab();
			}
		}
		if (Q_strcmp(pszCommand, "Connect") == 0)
		{
			char szNick[30];
			m_pTextEntry_NickEntry->GetText(szNick, sizeof(szNick));
			sprintf(irc_user.nick, szNick);

			// Connect to remote host
			if (!irc_socket.Connect("irc.gamesurge.net", 6667)) 
			{
				Msg("[IRC] Could not connect to server\n");
			}
			
			// Send data
			if (!irc_socket.Send( VarArgs("USER %s %s: %s %s  \n\r", irc_user.nick , irc_user.email , irc_user.ident , irc_user.ident) ))
			{
				Msg("[IRC] Could not send USER to server\n");
				irc_socket.Close();
			}

			// Send data
			if (!irc_socket.Send( VarArgs("NICK %s\n\r", irc_user.nick) )) 
			{
				Msg("[IRC] Could not send NICK to server\n");
				irc_socket.Close();
			}
		}
		if (Q_strcmp(pszCommand, "JoinChannel") == 0)
		{
			if (!irc_socket.Send( VarArgs("JOIN %s\n\r", "#fortressforever") ))
				Msg("[IRC] Could not send JOIN to server\n");
		}
	}

	vgui::ListPanel			*m_pGameList;
	vgui::TextEntry			*m_pTextEntry_NickEntry;

};

//-----------------------------------------------------------------------------
// CFFIRCGameTab
//-----------------------------------------------------------------------------

class CFFIRCGameTab : public CFFIRCTab
{
	DECLARE_CLASS_SIMPLE(CFFIRCGameTab, CFFIRCTab);

public:
	
	CFFIRCGameTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
	{

		LoadControlSettings("resource/ui/FFIRCGameTab.res");
	}

	MESSAGE_FUNC_PARAMS(OnNewLineMessage, "TextNewLine", data)
	{
		char szCommand[256];
		m_pTextEntry_ChatEntry->GetText(szCommand, sizeof(szCommand));
	    
		if ( !irc_socket.Send( VarArgs("%s\r\n", szCommand) ) )
			Msg("[IRC] Unable to send message: %s\n", szCommand);

		//const char* text = data->GetString("text");
		m_pRichText_Chat->InsertString(szCommand);
		m_pRichText_Chat->InsertString("\n");
		m_pTextEntry_ChatEntry->SetText("");
		m_pRichText_Chat->GotoTextEnd();
	}
	
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data)
	{
		const char *pszCommand = data->GetString("command");

		if (Q_strcmp(pszCommand, "RemoveGame") == 0)
		{
			CFFIRCPanel *parent = dynamic_cast <CFFIRCPanel *> (GetParent()->GetParent());

			if (parent)
			{
				parent->RemoveGameTab( this );
			}
		}
	}

};

//-----------------------------------------------------------------------------
// CFFIRCPanel gameui definition
//-----------------------------------------------------------------------------

DEFINE_GAMEUI(CFFIRC, CFFIRCPanel, ffirc);

CON_COMMAND(ToggleIRCPanel,NULL)
{
	//ToggleVisibility(ffirc->GetPanel());
	ffirc->GetPanel()->SetVisible(true);
}

//-----------------------------------------------------------------------------
// CFFIRCPanel implementation
//-----------------------------------------------------------------------------

// The main screen where everything IRC related is added to
CFFIRCPanel::CFFIRCPanel( vgui::VPANEL parent ) : BaseClass( NULL, "FFIRCPanel" )
{
	SetParent(parent);
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme( scheme );

	if (g_pIRCPanel == NULL)
		g_pIRCPanel = this;

	// Centre this panel on the screen for consistency.
	// This should be in the .res surely? - AfterShock
	//int nWide = GetWide();
	//int nTall = GetTall();

	//SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

	m_pLobbyTab = new CFFIRCLobbyTab(this, "IRCLobbyTab");

	m_pIRCTabs = new vgui::PropertySheet(this, "IRCTabs", true);
	m_pIRCTabs->AddPage(m_pLobbyTab, "Lobby");
	m_pIRCTabs->SetActivePage(m_pLobbyTab);
	m_pIRCTabs->SetDragEnabled(false);

	/*
	// need to be on a server for there to be a local player... :(
	C_BasePlayer *pPlayer =	C_BasePlayer::GetLocalPlayer();

	player_info_t sPlayerInfo;
	if ( pPlayer )
	{
		engine->GetPlayerInfo( pPlayer->entindex(), &sPlayerInfo );

		const char *pName = sPlayerInfo.name;

		sprintf(irc_user.nick, "%s", pName);
		Msg("[IRC] Nick: %s / %s\n", irc_user.nick, sPlayerInfo.name );
		Msg("[IRC] GUID: %s UID: %s friendsID: %d\n", sPlayerInfo.guid, sPlayerInfo.userID, sPlayerInfo.friendsID );
	}

	engine->GetPlayerInfo( engine->GetLocalPlayer(), &sPlayerInfo );

	Msg("ROUND 2\n", irc_user.nick, sPlayerInfo.name );
	Msg("[IRC] Nick: %s \n", sPlayerInfo.name );
	Msg("[IRC] GUID: %s UID: %s friendsID: %d\n", sPlayerInfo.guid, sPlayerInfo.userID, sPlayerInfo.friendsID );
	*/

	// IRC
	// start with some base info
	sprintf(irc_user.nick, "temptest-ff");
	sprintf(irc_user.ident,"test");
	sprintf(irc_user.email,"vertexar@yahoo.com");

	// create the response thread
	CThreadTest::GetInstance();

	// Open up a socket
	if (!irc_socket.Open(/*SOCK_STREAM */ 1, 0)) 
	{
		Msg("[IRC] Could not open socket\n");
	}

	LoadControlSettings("Resource/UI/FFIRC.res");
	//CenterThisPanelOnScreen();//keep in mind, hl2 supports widescreen 
	SetVisible(false); //made visible on command later 

	//Other useful options
	SetSizeable(false);
	//SetMoveable(false);
}

// The main screen where everything IRC related is added to
CFFIRCPanel::~CFFIRCPanel( )
{
	// horrible way to close the thread (I think), but hl2 will crash on close without it - squeek
	CThreadTest::GetInstance().Terminate();
	irc_socket.Close();
}

void CFFIRCPanel::Close()
{
	//CThreadTest::GetInstance().Terminate();
	BaseClass::Close();
	
	// leave the main lobby
	if (!irc_socket.Send( VarArgs("PART %s\n\r", "#fortressforever") ))
		Msg("[IRC] Could not send PART to server\n");
}

void CFFIRCPanel::AddGameTab()
{
	m_pGameTab = new CFFIRCGameTab(this, "IRCGameTab");
	m_pIRCTabs->AddPage(m_pGameTab, "Game Name");
}

void CFFIRCPanel::RemoveGameTab( CFFIRCTab *pTab )
{
	m_pIRCTabs->DeletePage(pTab);
}

void CFFIRCPanel::ParseServerMessage( char *buf )
{
	// print to the console for now to see whats going on
	Msg("%s\n", buf);

	// PING?
	if (!strnicmp(buf,"PING",4)) 
	{
		// PONG
		buf[1] = 'O';
		irc_socket.Send( VarArgs("%s\r\n", buf) );
		return;
	}

	// Names?
	// :NuclearFallout.WA.US.GameSurge.net 353 squeektest = #FortressForever :squeektest Paft R00Kie GenghisTron Spartacus cjeshjoir- AtLarge mib_splib j3 []Zer0 Sailor_Mercury-sama_NL_id prodigy Hellfish`away @squeek @ChanServ +padawan
	if (strstr(buf,VarArgs("353 %s", irc_user.nick)) != NULL)
	{
		char *pszChan = strchr(buf,'#');
		if (Q_strnicmp((pszChan+1), "fortressforever", 15) == 0)
		{
			// Populate lobby list
			CFFIRCLobbyTab *tab = dynamic_cast <CFFIRCLobbyTab *> (m_pLobbyTab);
			if (tab)
			{
				char *msg = strchr(pszChan,':');

				tab->UserList_Fill(msg+1);
				return;
			}
		}
	}

	// standard IRC server messages always start with a colon
	//		:nick!host@mask COMMAND parameter1 parameter2 ... [:param with spaces]
	//		:server 000 mynick parameter1 parameter2 ... [:param with spaces]
	if (buf[0] == ':')
	{
		// find position of the first space
		int firstspace_pos = strcspn(buf, " ");
		// if there is a space
		if (buf[firstspace_pos])
		{
			// commands shouldn't be bigger than 30 chars, right?
			char command[30];
			int paramstart=0;

			// command/response code can be parsed now, they both come after the first space
			int i=0;
			command[29] = '\0';
			for(int ti=firstspace_pos+1;ti<firstspace_pos+30;++ti)
			{
				if (buf[ti] == ' ')
				{
					command[i] = '\0';
					// remember this place so that we can skip to it later
					paramstart = ti+1;
					break;
				}
				command[i]=buf[ti];
				i++;
			}

			// search for an exclamation point
			int excl_pos = strcspn( buf, "!" );

			// USER MESSAGE
			// if there is a !, and the ! is before the first space, then the message is being sent by someone
			if (buf[excl_pos] && firstspace_pos > excl_pos)
			{
				// MAXNICKLEN is 30
				char from[30];
				
				for(int ti=1;ti<excl_pos;++ti)
				{
					from[ti-1]=buf[ti];
				}
				from[excl_pos-1] = '\0';

				// bypass everything that has just been parsed
				buf += paramstart;
				// buf should now only have parameters in it
				
				// PRIVMSG
				//		:squeek!~squeek502@squeek.user.gamesurge PRIVMSG #FortressForever :test
				//		:squeek!~squeek502@squeek.user.gamesurge PRIVMSG squeektest :hello
				if (Q_strcmp(command,"PRIVMSG") == 0)
				{
					// going to a channel
					if (buf[0] == '#')
					{
						// #fortressforever
						if (Q_strnicmp((buf+1), "fortressforever", 15) == 0)
						{
							if (m_pLobbyTab)
							{
								char *msg = strchr(buf,':') + 1;

								m_pLobbyTab->UserMessage(from, msg);
								return;
							}
						}
					}
					// going to the local user
					else if (Q_strncmp(buf, irc_user.nick, Q_strlen(irc_user.nick)) == 0)
					{
						if (m_pLobbyTab)
						{
							char *msg = strchr(buf,':') + 1;

							m_pLobbyTab->SystemMessage(VarArgs("Private message from %s: %s", from, msg));
							return;
						}
					}
					else 
					{
						DevMsg("[IRC] Strange PRIVMSG syntax: %s", buf);
					}
				}
				// QUIT
				//		:squeek!~squeek502@squeek.user.gamesurge QUIT :Reason
				else if (Q_strcmp(command,"QUIT") == 0)
				{
					// need to update all open tabs
					return;
				}
				// JOIN
				//		:squeek!~squeek502@squeek.user.gamesurge JOIN #FortressForever
				else if (Q_strcmp(command,"JOIN") == 0)
				{
					// just to make sure a channel is being joined
					if (buf[0] == '#')
					{
						// #fortressforever
						if (Q_strnicmp((buf+1), "fortressforever", 15) == 0)
						{
							if (m_pLobbyTab)
							{
								m_pLobbyTab->UserList_AddUser(from);
								return;
							}
						}
					}
				}
				// PART
				//		:squeek!~squeek502@squeek.user.gamesurge PART #FortressForever
				else if (Q_strcmp(command,"PART") == 0)
				{
					// just to make sure a channel is being parted
					if (buf[0] == '#')
					{
						// #fortressforever
						if (Q_strnicmp((buf+1), "fortressforever", 15) == 0)
						{
							if (m_pLobbyTab)
							{
								m_pLobbyTab->UserList_RemoveUser(from);
								return;
							}
						}
					}
				}
			}
			// SERVER MESSAGE
			// if there is no !, or the space is before the !, then its a server message
			else if( !buf[excl_pos] || (buf[excl_pos] && firstspace_pos < excl_pos) )
			{
				// don't need to parse the from, we know it's from the server
				
				// bypass everything that has just been parsed
				buf += paramstart;
				// buf should now only have parameters in it

				// 353: Names list
				//		:NuclearFallout.WA.US.GameSurge.net 353 squeektest = #FortressForever :squeektest Paft R00Kie GenghisTron Spartacus cjeshjoir- AtLarge mib_splib j3 []Zer0 Sailor_Mercury-sama_NL_id prodigy Hellfish`away @squeek @ChanServ +padawan
				if (Q_strcmp(command,"353") == 0)
				{
					char *pszChan = strchr(buf,'#');
					if (pszChan)
					{
						// #fortressforever
						if (Q_strnicmp((pszChan+1), "fortressforever", 15) == 0)
						{
							if (m_pLobbyTab)
							{
								char *msg = strchr(pszChan,':') + 1;
								
								m_pLobbyTab->UserList_Fill(msg);
								return;
							}
						}
					}
				}
				// 366: End of names list
				//		:Co-Locate.NL.EU.GameSurge.net 366 tempnick123 #FortressForever :End of /NAMES list.
				else if (Q_strcmp(command,"366") == 0)
				{
					char *pszChan = strchr(buf,'#');
					if (pszChan)
					{
						// #fortressforever
						if (Q_strnicmp((pszChan+1), "fortressforever", 15) == 0)
						{
							if (m_pLobbyTab)
							{
								m_pLobbyTab->UserList_SetReceiving( false );
								return;
							}
						}
					}
				}
			}
		}
	}


/*
	// this is all incredibly insufficient; MODE can do things like +oovo nick1 nick2 nick3 nick4

	// MODE
	// :ChanServ!ChanServ@Services.GameSurge.net MODE #FortressForever +o squeek
	if (strstr(buf,"MODE #") != NULL)
	{
		char *pszChan = strchr(buf,'#');
		if (Q_strnicmp((pszChan+1), "fortressforever", 15) == 0)
		{
			// tell lobby list what happened
			CFFIRCLobbyTab *tab = dynamic_cast <CFFIRCLobbyTab *> (m_pLobbyTab);
			if (tab)
			{
				// user doing the changing
				char user[40];
				user[39] = '\0';
				
				for(int ti=1;ti<39;++ti)
				{
					if (buf[ti]=='!')
					{
						user[ti-1] = '\0';
						break;
					}
					user[ti-1]=buf[ti];
				}
				
				char *pszAccessChange = strchr(pszChan,' ')+1;

				if (!pszAccessChange)
					return;

				char *pszUserChanged = strchr(pszAccessChange,' ')+1;

				// channel mode being changed
				if (!pszUserChanged)
				{
					tab->SystemMessage(VarArgs("%s sets channel mode: ", pszUserChanged, user));
				}
				// user mode being changed
				else
				{
					if (pszAccessChange[0] == '+')
					{
						// getting vip'd
						if (pszAccessChange[1] == 'v')
						{
							tab->UserList_UpdateUserAccess(pszUserChanged, 1);
						}
						// getting op'd
						else if (pszAccessChange[1] == 'o')
						{
							tab->UserList_UpdateUserAccess(pszUserChanged, 2);
						}
						// getting banned
						else if (pszAccessChange[1] == 'b')
						{
							tab->SystemMessage(VarArgs("%s was banned by %s", pszUserChanged, user));
						}
					}
					else if (pszAccessChange[0] == '-')
					{
						// could be moving down
						if (pszAccessChange[1] == 'v' || pszAccessChange[1] == 'o')
						{
							tab->UserList_UpdateUserAccess(pszUserChanged, 0);
						}
						else if (pszAccessChange[1] == 'b')
						{
							tab->SystemMessage(VarArgs("%s was unbanned by %s", pszUserChanged, user));
						}
					}
				}

				return;
			}
		}
	}
*/

}

//-----------------------------------------------------------------------------
// CThreadTest implementation
//-----------------------------------------------------------------------------

CThreadTest::CThreadTest( void )
{
	SetName("IRCThread");
	m_bIsRunning = false;
	Start();
}

CThreadTest::~CThreadTest( void )
{
}

int CThreadTest::Run()
{
	int a = 0;
	//int i = 0;
	char buf[3000];

	m_bIsRunning = true;

	while(IsAlive())
	{
		a = irc_socket.Recv(buf, sizeof(buf)-1);

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

			// send whatevers left
			if (g_pIRCPanel != NULL)
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

CThreadTest& CThreadTest::GetInstance()
{
	static CThreadTest ircthread;
	return ircthread;
}
