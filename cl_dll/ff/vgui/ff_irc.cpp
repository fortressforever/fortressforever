//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_irc.cpp
//	@author Ryan Liptak (squeek)
//	@date 30/01/2010
//	@brief IRC interface
//
//	REVISIONS
//	---------

#include "cbase.h"
#include "ff_irc.h"

#include "irc/ff_socks.h"
#include "irc/ff_irc_thread.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

using namespace vgui;

Socks g_IRCSocket;
CFFIRCPanel *g_pIRCPanel = NULL;
CFFIRCConnectPanel *g_pIRCConnectPanel = NULL;

//-----------------------------------------------------------------------------
// CFFIRCLobbyTab
//-----------------------------------------------------------------------------

class CFFIRCLobbyTab : public CFFIRCTab
{
	DECLARE_CLASS_SIMPLE(CFFIRCLobbyTab, CFFIRCTab);

public:
	
	CFFIRCLobbyTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
	{
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
		
		if ( !g_IRCSocket.Send( VarArgs("PRIVMSG #fortressforever :%s\r\n", szCommand) ) )
			Msg("[IRC] Unable to send message: %s\n", szCommand);
	                 
		//const char* text = data->GetString("text");
		m_pRichText_Chat->InsertString( VarArgs("%s: %s",g_pIRCPanel->irc_user.nick, szCommand) );
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
		if (Q_strcmp(pszCommand, "Disconnect") == 0)
		{
			CFFIRCPanel *parent = dynamic_cast <CFFIRCPanel *> (GetParent()->GetParent());

			if (parent)
			{
				parent->irc_user.status = 0;

				if ( !g_IRCSocket.Send( "QUIT\r\n" ) )
					Msg("[IRC] Unable to send message: QUIT\n");

				parent->SetVisible( false );
				g_pIRCConnectPanel->Reset();
				g_pIRCConnectPanel->SetVisible( true );
			}
		}
	}

	vgui::ListPanel			*m_pGameList;

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
	    
		if ( !g_IRCSocket.Send( VarArgs("%s\r\n", szCommand) ) )
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
DEFINE_GAMEUI(CFFIRCConnect, CFFIRCConnectPanel, ffircconnect);

CON_COMMAND(ToggleIRCPanel,NULL)
{
	//ToggleVisibility(ffirc->GetPanel());
	//ffirc->GetPanel()->SetVisible(true);
	if( g_pIRCPanel->irc_user.status )
		ffirc->GetPanel()->SetVisible(true);
	else
		ffircconnect->GetPanel()->SetVisible(true);
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
	irc_user.status = 0;

	// Open up a socket
	if (!g_IRCSocket.Open(/*SOCK_STREAM */ 1, 0)) 
	{
		Msg("[IRC] Could not open socket\n");
	}

	// create the response thread
	CFFIRCThread::GetInstance();

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
	CFFIRCThread::GetInstance().Terminate();
	g_IRCSocket.Close();
}

void CFFIRCPanel::SetVisible(bool state)
{
	if (state)
	{		
		// Centre this panel on the screen for consistency.
		int nWide = GetWide();
		int nTall = GetTall();

		SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

		RequestFocus();
		MoveToFront();

		// join main lobby
		if (!g_IRCSocket.Send( VarArgs("JOIN %s\n\r", "#fortressforever") ))
			Msg("[IRC] Could not send JOIN to server\n");
	}

	BaseClass::SetVisible(state);
}

void CFFIRCPanel::Close()
{
	//CFFIRCThread::GetInstance().Terminate();
	BaseClass::Close();
	
	// leave the main lobby
	if (irc_user.status)
	{
		if (!g_IRCSocket.Send( VarArgs("PART %s\n\r", "#fortressforever") ))
			Msg("[IRC] Could not send PART to server\n");
	}
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
		g_IRCSocket.Send( VarArgs("%s\r\n", buf) );
		return;
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
			char command[31];
			int paramstart=0;

			// command/response code can be parsed now, they both come after the first space
			int i=0;
			command[30] = '\0';
			for(int ti=firstspace_pos+1;ti<firstspace_pos+31;++ti)
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
				char from[31];
				
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
						// ignore CTCP
						if(Q_strcmp(from, "CTCP") == 0)
							return;

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
				// NICK
				//		:squeektest!~squeektes@76-201-84-238.lightspeed.frokca.sbcglobal.net NICK :squeektest2
				else if (Q_strcmp(command,"NICK") == 0)
				{
					// new nickname is preceded by a colon
					if (buf[0] == ':')
					{
						// skip to the nick (past the colon)
						buf += 1;

						// need to tell all open tabs
						if (m_pLobbyTab)
						{
							m_pLobbyTab->SystemMessage( VarArgs("%s is now known as %s", from, buf) );
							m_pLobbyTab->UserList_UpdateUserName(from, buf);
							return;
						}
					}
				}
				// QUIT
				//		:squeek!~squeek502@squeek.user.gamesurge QUIT :Reason
				else if (Q_strcmp(command,"QUIT") == 0)
				{
					// need to update all open tabs
					if (m_pLobbyTab)
					{
						char *msg = strchr(buf,':') + 1;

						m_pLobbyTab->SystemMessage( VarArgs("%s has quit (%s)", from, (msg ? msg : "Unknown reason")) );
						m_pLobbyTab->UserList_RemoveUser(from);
					}
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
								m_pLobbyTab->SystemMessage( VarArgs("%s has joined the room", from) );
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
								m_pLobbyTab->SystemMessage( VarArgs("%s has left the room", from) );
								m_pLobbyTab->UserList_RemoveUser(from);
								return;
							}
						}
					}
				}
				// MODE
				//		:ChanServ!ChanServ@Services.GameSurge.net MODE #FortressForever +oo-v nick1 nick2 nick3 ...
				else if (Q_strcmp(command,"MODE") == 0)
				{
					// just to make sure a channel is being moded
					if (buf[0] == '#')
					{
						// #fortressforever
						if (Q_strnicmp((buf+1), "fortressforever", 15) == 0)
						{
							// skip the chan param, go to the next (modes)
							buf = strchr(buf,' ') + 1;

							char *users = strchr(buf,' ');
							
							// if there are more spaces, then user modes are being set
							if (users)
							{
								// skip the space char
								users++;

								char modifier = 0;
								char mode;
								for (int i=0; buf[i] != ' '; ++i)
								{
									if (buf[i] == '+')
										modifier='+';
									else if (buf[i] == '-')
										modifier='-';
									else
									{
										// if no modifier yet, things are not going well
										if( !modifier )
											continue;

										mode = buf[i];
										
										// MAXNICKLEN is 30
										char moded[31];
										moded[30] = '\0';
										
										for(int ti=0;ti<30;++ti)
										{
											if (users[ti] == ' ')
											{
												moded[ti] = '\0';
												break;
											}
											moded[ti]=users[ti];
										}

										if (m_pLobbyTab)
										{
											m_pLobbyTab->SystemMessage( VarArgs("%s sets mode: %c%c %s", from, modifier, mode, moded) );
											// +
											if( modifier=='+' )
											{
												// op +o
												if (mode=='o')
													m_pLobbyTab->UserList_UpdateUserAccess( moded, 2 );
												// vip +v
												else if (mode=='v')
													m_pLobbyTab->UserList_UpdateUserAccess( moded, 1, false );
												// ban +b
												//else if (mode=='b')
											}
											// -
											else
											{
												// delop delvip -o -v
												if (mode=='v' || mode=='o')
													m_pLobbyTab->UserList_UpdateUserAccess( moded, 0 );
												// unban -b
												//if (mode=='b')
											}
										}

										users = strchr(users,' ');
									}
								}
							}
							// else channel modes are being set
							else
							{
								char modifier = 0;
								char mode;
								for (int i=0; buf[i] != ' '; ++i)
								{
									if (buf[i] == '+')
										modifier='+';
									else if (buf[i] == '-')
										modifier='-';
									else
									{
										// if no modifier yet, things are not going well
										if( !modifier )
											continue;

										mode = buf[i];
									}
								}
							}
						}
					}
				}
				// KICK
				//		:ChanServ!ChanServ@Services.GameSurge.net KICK #FortressForever squeektest :(squeek) test
				else if (Q_strcmp(command,"KICK") == 0)
				{
					// just to make sure a channel is being parted
					if (buf[0] == '#')
					{
						// #fortressforever
						if (Q_strnicmp((buf+1), "fortressforever", 15) == 0)
						{
							// skip the chan param, go to the next (kicked user)
							buf = strchr(buf,' ') + 1;

							// MAXNICKLEN is 30
							char kicked[31];
							kicked[30] = '\0';
							
							for(int ti=0;ti<30;++ti)
							{
								if (buf[ti] == ' ')
								{
									kicked[ti] = '\0';
									break;
								}
								kicked[ti]=buf[ti];
							}
							
							// skip to the reason
							buf = strchr(buf,':') + 1;

							if (m_pLobbyTab)
							{
								m_pLobbyTab->SystemMessage( VarArgs("%s has been kicked by %s (Reason: %s)", kicked, from, (buf ? buf : "No reason given")) );
								m_pLobbyTab->UserList_RemoveUser(kicked);
								return;
							}
						}
					}
				}
				// NOTICE
				//		:squeek!~squeek502@squeek.user.gamesurge NOTICE #fortressforever :test
				//		:ChanServ!ChanServ@Services.GameSurge.net NOTICE squeektest :(#FortressForever) www.fortress-forever.com ...
				else if (Q_strcmp(command,"NOTICE") == 0)
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
								
								m_pLobbyTab->UserMessage(from, msg, Color(244,244,190,255));
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

							// if it's from global, print to the console and return
							if (Q_strcmp(from, "Global") == 0)
							{
								//Msg(VarArgs("%s", msg));
								return;
							}

							m_pLobbyTab->SystemMessage(VarArgs("Notice from %s: %s", from, msg), Color(244,244,190,255));
							return;
						}
					}
					else 
					{
						DevMsg("[IRC] Strange NOTICE syntax: %s", buf);
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

				// 001: Welcome message
				//		:Snoke.NL.EU.GameSurge.net 001 squeektest :Welcome to the GameSurge IRC Network via Snoke.nl, squeektest
				if (Q_strcmp(command,"001") == 0)
				{
					char *pUser = strtok(buf, " ");
					if (Q_strcmp(pUser, irc_user.nick) != 0)
					{
						sprintf(irc_user.nick, pUser);
						Msg("[IRC] Nickname auto-changed to %s", irc_user.nick);
					}
					// connected!
					irc_user.status=1;
					g_pIRCConnectPanel->UpdateStatus("Connected!");
					g_pIRCConnectPanel->Connected();
					return;
				}
				// 433: Nickname in use
				//		:Burstfire.UK.EU.GameSurge.net 433 * squeek :Nickname is already in use.
				else if (Q_strcmp(command,"433") == 0)
				{
					// if not connected, bad news
					if(!irc_user.status)
					{
						g_pIRCConnectPanel->UpdateStatus("ERROR: Nickname already in use");
						g_pIRCConnectPanel->UpdateStatus("Nickname already in use");
						g_pIRCConnectPanel->ConnectFailed();
					}
				}
				// 251: Users
				//		:Burstfire.UK.EU.GameSurge.net 251 squeektest :There are 34 users and 14805 invisible on 21 servers
				else if (Q_strcmp(command,"251") == 0)
				{
					// fully connected at this point
					//g_pIRCConnectPanel->Connected();
					return;
				}
				// 353: Names list
				//		:NuclearFallout.WA.US.GameSurge.net 353 squeektest = #FortressForever :squeektest Paft R00Kie GenghisTron Spartacus cjeshjoir- AtLarge mib_splib j3 []Zer0 Sailor_Mercury-sama_NL_id prodigy Hellfish`away @squeek @ChanServ +padawan
				else if (Q_strcmp(command,"353") == 0)
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
}


//-----------------------------------------------------------------------------
// Purpose: Catch the buttons. We have OK (save + close), Cancel (close) and
//			Apply (save).
//-----------------------------------------------------------------------------
CFFIRCConnectPanel::CFFIRCConnectPanel( vgui::VPANEL parent ) : BaseClass( NULL, "FFIRCConnectPanel" )
{
	if (g_pIRCConnectPanel == NULL)
		g_pIRCConnectPanel = this;

	m_pTextEntry_NickEntry = new vgui::TextEntry(this, "NickEntry");
	m_pTextEntry_NickEntry->SetEditable( true );
	m_pTextEntry_NickEntry->SetText( "" );

	m_pStatusLabel = new vgui::Label(this, "StatusLabel", "...");

	new Button(this, "OKButton", "", this, "OK");
	new Button(this, "CancelButton", "", this, "Cancel");

	SetSizeable(false);
	LoadControlSettings("resource/ui/FFIRCConnect.res");

	Reset();
}
	
void CFFIRCConnectPanel::SetVisible(bool state)
{
	if (state)
	{		
		// Centre this panel on the screen for consistency.
		int nWide = GetWide();
		int nTall = GetTall();

		SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

		RequestFocus();
		MoveToFront();

		ConVar *cvar_name;
		cvar_name = cvar->FindVar( "name" );

		char curname[31];
		m_pTextEntry_NickEntry->GetText( curname, 30 );

		if ( cvar_name && !curname[0] )
			m_pTextEntry_NickEntry->SetText(cvar_name->GetString());

		m_pTextEntry_NickEntry->RequestFocus();
		//m_pTextEntry_NickEntry->SelectAllText( true );
	}

	BaseClass::SetVisible(state);
}

void CFFIRCConnectPanel::Reset()
{
	m_pStatusLabel->SetVisible( false );
	m_pStatusLabel->SetText( "..." );
	m_pTextEntry_NickEntry->SetEditable( true );
	m_pTextEntry_NickEntry->SetEnabled( true );
}

void CFFIRCConnectPanel::UpdateStatus( const char *status )
{
	m_pStatusLabel->SetText( status );
}

void CFFIRCConnectPanel::ConnectFailed()
{
	m_pTextEntry_NickEntry->SetEditable( true );
	m_pTextEntry_NickEntry->SetEnabled( true );
	m_pTextEntry_NickEntry->RequestFocus();
}

void CFFIRCConnectPanel::Connected()
{
	g_pIRCPanel->SetVisible( true );
	SetVisible( false );
}

void CFFIRCConnectPanel::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	if (Q_strcmp(pszCommand, "OK") == 0)
	{
		char szNick[30];
		m_pTextEntry_NickEntry->GetText(szNick, sizeof(szNick));
		sprintf(g_pIRCPanel->irc_user.nick, szNick);

		if (g_pIRCPanel->irc_user.status)
		{
			if (!g_IRCSocket.Send( "QUIT\n\r" ))
			{
				Msg("[IRC] Could not send QUIT to server\n");
			}
		}

		// Open up a socket
		if (!g_IRCSocket.Open(/*SOCK_STREAM */ 1, 0)) 
		{
			Msg("[IRC] Could not open socket\n");
		}

		// Connect to remote host
		if (!g_IRCSocket.Connect("irc.gamesurge.net", 6667)) 
		{
			Msg("[IRC] Could not connect to server\n");
		}
		
		// Send data
		if (!g_IRCSocket.Send( VarArgs("USER %s %s: %s %s  \n\r", g_pIRCPanel->irc_user.nick , g_pIRCPanel->irc_user.email , g_pIRCPanel->irc_user.ident , g_pIRCPanel->irc_user.ident) ))
		{
			Msg("[IRC] Could not send USER to server\n");
			g_IRCSocket.Close();
		}

		// Send data
		if (!g_IRCSocket.Send( VarArgs("NICK %s\n\r", g_pIRCPanel->irc_user.nick) )) 
		{
			Msg("[IRC] Could not send NICK to server\n");
			g_IRCSocket.Close();
		}

		m_pTextEntry_NickEntry->SetEditable( false );
		m_pTextEntry_NickEntry->SetEnabled( false );
		m_pStatusLabel->SetText( "Connecting..." );
		m_pStatusLabel->SetVisible( true );
	}
	if (Q_strcmp(pszCommand, "Cancel") == 0)
		SetVisible(false);
}