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
#include <vgui/IVgui.h>

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

using namespace vgui;

Socks g_IRCSocket;
CFFIRCPanel *g_pIRCPanel = NULL;
CFFIRCConnectPanel *g_pIRCConnectPanel = NULL;
CFFIRCHostPanel *g_pIRCHostPanel = NULL;


//-----------------------------------------------------------------------------
// CFFIRCLobbyGameList
//-----------------------------------------------------------------------------

CFFIRCLobbyGameList::CFFIRCLobbyGameList(Panel *parent, char const *panelName) : BaseClass(parent, panelName) {}

void CFFIRCLobbyGameList::OnMouseDoublePressed(MouseCode code)
{
	if( code == 0 )
	{
		CFFIRCPanel *parent = dynamic_cast <CFFIRCPanel *> (GetParent()->GetParent()->GetParent());

		if( parent && GetItem( GetSelectedItem(0) ) )
		{
			KeyValues *KV = GetItem( GetSelectedItem(0) );
			parent->AddGameTab( KV->GetString("channel"), KV->GetString("channel") );
		}
	}
}


//-----------------------------------------------------------------------------
// CFFIRCLobbyTab
//-----------------------------------------------------------------------------

CFFIRCLobbyTab::CFFIRCLobbyTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
{
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme( scheme );

	m_pGameList = new CFFIRCLobbyGameList(this, "ListPanel_GameList");

	m_pGameList->AddActionSignalTarget( this );
	
    m_pGameList->AddColumnHeader( 0, "players" , "Players" , 20, ListPanel::COLUMN_RESIZEWITHWINDOW );
    m_pGameList->AddColumnHeader( 1, "name" , "Game Name" , 200, ListPanel::COLUMN_RESIZEWITHWINDOW );
	m_pGameList->AddColumnHeader( 2, "map" , "Map Name" , 150, ListPanel::COLUMN_RESIZEWITHWINDOW  );
	m_pGameList->AddColumnHeader( 3, "ranked" , "Ranked?" , 20, ListPanel::COLUMN_RESIZEWITHWINDOW  );
	m_pGameList->AddColumnHeader( 4, "channel" , "Channel" , 0, ListPanel::COLUMN_HIDDEN  );
	m_pGameList->AddColumnHeader( 5, "serverip" , "Server IP" , 0, ListPanel::COLUMN_HIDDEN  );
	m_pGameList->AddColumnHeader( 6, "serverpassword" , "Server Password" , 0, ListPanel::COLUMN_HIDDEN  );

	KeyValues *kv = new KeyValues( "LI" );
	kv->SetString( "players", "1/8" );
	kv->SetString( "name", "Default test game - won't have bot in" );
	kv->SetString( "map", "ff_monkey" );
	kv->SetString( "ranked", "Ranked" );
	kv->SetString( "serverip", "123.23.23.23:27015" );
	kv->SetString( "serverpassword", "ffpickup" );
	kv->SetString( "channel", "ffgame-0" );
	m_pGameList->AddItem( kv, 0, false, false );
	kv->deleteThis();

	LoadControlSettings("resource/ui/FFIRCLobbyTab.res");
}

void CFFIRCLobbyTab::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );

	m_pGameList->SetFont( pScheme->GetFont( "DefaultVerySmall" ) );
}

void CFFIRCLobbyTab::OnNewLineMessage(KeyValues *data)
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

void CFFIRCLobbyTab::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	if (Q_strcmp(pszCommand, "AddGame") == 0)
	{
		CFFIRCPanel *parent = dynamic_cast <CFFIRCPanel *> (GetParent()->GetParent());

		if( parent && m_pGameList->GetItem( m_pGameList->GetSelectedItem(0) ) )
		{
			KeyValues *KV = m_pGameList->GetItem( m_pGameList->GetSelectedItem(0) );
			parent->AddGameTab( KV->GetString("channel"), KV->GetString("channel") );
		}
	}
	if (Q_strcmp(pszCommand, "HostGame") == 0)
	{
		/*
		if ( !g_IRCSocket.Send( "PRIVMSG ff-systembot :!host ff_monkey 8 1.2.1.2:27016 ffpassword Test game bla bla \r\n" ) )
			Msg("[IRC] Unable to send message: !hos\n");
		*/
		ffirchost->GetPanel()->SetVisible(true);

		// this should be commented once threading is gone, and the gametab should be created on receipt of the !join privmsg from the bot
		/*
		CFFIRCPanel *parent = dynamic_cast <CFFIRCPanel *> (GetParent()->GetParent());

		if (parent)
		{
			parent->AddGameTab( "Hosted Game", "#ffgame-313" );
		}*/
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

//-----------------------------------------------------------------------------
// CFFIRCGameTab
//-----------------------------------------------------------------------------

CFFIRCGameTab::CFFIRCGameTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
{
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme( scheme );

	m_pTeam1List = new ListPanel(this, "ListPanel_Team1List");
	m_pTeam1List->AddActionSignalTarget( this );
    m_pTeam1List->AddColumnHeader( 0, "players" , "Team 1 Players" , 200, ListPanel::COLUMN_RESIZEWITHWINDOW );
    m_pTeam1List->AddColumnHeader( 1, "ranking" , "Ranking" , 20, ListPanel::COLUMN_RESIZEWITHWINDOW );

	m_pTeam2List = new ListPanel(this, "ListPanel_Team2List");
	m_pTeam2List->AddActionSignalTarget( this );
    m_pTeam2List->AddColumnHeader( 0, "players" , "Team 2 Players" , 200, ListPanel::COLUMN_RESIZEWITHWINDOW );
    m_pTeam2List->AddColumnHeader( 1, "ranking" , "Ranking" , 20, ListPanel::COLUMN_RESIZEWITHWINDOW );

	LoadControlSettings("resource/ui/FFIRCGameTab.res");
}

void CFFIRCGameTab::OnNewLineMessage(KeyValues *data)
{
	char szCommand[256];
	m_pTextEntry_ChatEntry->GetText(szCommand, sizeof(szCommand));
	
	if ( !g_IRCSocket.Send( VarArgs("PRIVMSG #%s :%s\r\n", GetChannel(), szCommand) ) )
		Msg("[IRC] Unable to send message: %s\n", szCommand);
    
	//const char* text = data->GetString("text");
	m_pRichText_Chat->InsertString( VarArgs("%s: %s",g_pIRCPanel->irc_user.nick, szCommand) );
	m_pRichText_Chat->InsertString("\n");
	m_pTextEntry_ChatEntry->SetText("");
	m_pRichText_Chat->GotoTextEnd();

	/*
	char szCommand[256];
	m_pTextEntry_ChatEntry->GetText(szCommand, sizeof(szCommand));
    
	if ( !g_IRCSocket.Send( VarArgs("%s\r\n", szCommand) ) )
		Msg("[IRC] Unable to send message: %s\n", szCommand);

	//const char* text = data->GetString("text");
	m_pRichText_Chat->InsertString(szCommand);
	m_pRichText_Chat->InsertString("\n");
	m_pTextEntry_ChatEntry->SetText("");
	m_pRichText_Chat->GotoTextEnd();*/
}

void CFFIRCGameTab::OnButtonCommand(KeyValues *data)
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
	else if (Q_strcmp(pszCommand, "StartGame") == 0)
	{
		if ( !g_IRCSocket.Send( VarArgs("PRIVMSG #%s :%s\r\n", GetChannel(), "!start") ) )
			Msg("[IRC] Unable to send message: !start\n");
	}
	else if (Q_strcmp(pszCommand, "JoinTeam1") == 0)
	{
		if ( !g_IRCSocket.Send( VarArgs("PRIVMSG #%s :%s\r\n", GetChannel(), "!jointeam 1") ) )
			Msg("[IRC] Unable to send message: !jointeam 1\n");
	}
	else if (Q_strcmp(pszCommand, "JoinTeam2") == 0)
	{
		if ( !g_IRCSocket.Send( VarArgs("PRIVMSG #%s :%s\r\n", GetChannel(), "!jointeam 2") ) )
			Msg("[IRC] Unable to send message: !jointeam 2\n");
	}
	else if (Q_strcmp(pszCommand, "LeaveTeam") == 0)
	{
		if ( !g_IRCSocket.Send( VarArgs("PRIVMSG #%s :%s\r\n", GetChannel(), "!leaveteam") ) )
			Msg("[IRC] Unable to send message: !leaveteam\n");
	}
}

//-----------------------------------------------------------------------------
// CFFIRCPanel gameui definition
//-----------------------------------------------------------------------------

DEFINE_GAMEUI(CFFIRC, CFFIRCPanel, ffirc);
DEFINE_GAMEUI(CFFIRCConnect, CFFIRCConnectPanel, ffircconnect);
DEFINE_GAMEUI(CFFIRCHost, CFFIRCHostPanel, ffirchost);

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
	vgui::ivgui()->AddTickSignal( GetVPanel(), 100 );
	m_bDataReady = false;

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
		if (!g_IRCSocket.Send( VarArgs("JOIN %s\n\r", "#ffirc-system") ))
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

void CFFIRCPanel::AddGameTab( const char *name, const char *channel )
{
	CFFIRCGameTab *pGameTab = new CFFIRCGameTab(this, "IRCGameTab");
	pGameTab->SetChannel( channel );
	
	char joinmsg[IRC_MAX_CHANLEN+10];
	sprintf( joinmsg, "JOIN %s\n\r", channel );
	// join new channel
	if (!g_IRCSocket.Send( joinmsg ))
		Msg("[IRC] Could not send JOIN to server\n");

	m_pGameTabs.AddToTail( pGameTab );
	m_pIRCTabs->AddPage(pGameTab, name);
	m_pIRCTabs->SetActivePage(pGameTab);
}

void CFFIRCPanel::RemoveGameTab( CFFIRCGameTab *pTab )
{
	// leave the game room
	if (irc_user.status)
	{
		if (!g_IRCSocket.Send( VarArgs("PART #%s\n\r", pTab->GetChannel() ) ))
			Msg("[IRC] Could not send PART to server\n");
	}
	m_pGameTabs.FindAndRemove( pTab );
	m_pIRCTabs->DeletePage( pTab );
}

CFFIRCGameTab* CFFIRCPanel::GetGameTabByChannel( const char *channel )
{
	for(int i=0; i<m_pGameTabs.Count(); ++i)
	{
		if (m_pGameTabs.IsValidIndex(i))
		{
			CFFIRCGameTab *pGameTab = m_pGameTabs.Element( i );
			if(Q_stricmp(pGameTab->GetChannel(), channel) == 0)
			{
				return pGameTab;
			}
		}
	}
	return (CFFIRCGameTab*)NULL;
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

	// quakenet.org server info:
	//		:xs4all.nl.quakenet.org 005 squeektest 
	//			WHOX WALLCHOPS WALLVOICES USERIP CPRIVMSG CNOTICE SILENCE=15 
	//			MODES=6 MAXCHANNELS=20 MAXBANS=45 NICKLEN=15 :are supported by this server
	//		:xs4all.nl.quakenet.org 005 squeektest 
	//			MAXNICKLEN=15 TOPICLEN=250 AWAYLEN=160 KICKLEN=250 CHANNELLEN=200 
	//			MAXCHANNELLEN=200 CHANTYPES=#& PREFIX=(ov)@+ STATUSMSG=@+ 
	//			CHANMODES=b,k,l,imnpstrDducCNMT CASEMAPPING=rfc1459 
	//			NETWORK=QuakeNet :are supported by this server


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
				char from[IRC_MAX_NICKLEN+1];
				
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
						char *msg = strchr(buf,':') + 1;

						// #fortressforever
						if (Q_strnicmp((buf+1), "fortressforever", 15) == 0)
						{
							if (m_pLobbyTab)
							{
								m_pLobbyTab->UserMessage(from, msg);
								return;
							}
						}
						// #ffirc-system
						else if (Q_strnicmp((buf+1), "ffirc-system", 12) == 0)
						{
							// if PM from the system bot
							if((Q_strcmp(from, "ff-systembot") == 0) || (Q_strcmp(from, "AfterShok") == 0)) // the only 2 cool dudes on IRC
							{
								char *msg = strchr(buf,':') + 1;

								// If new game, then create a new entry in the gamelist
								// CURRENTLY NOT USED, USE !game INSTEAD
								if (Q_strnicmp(msg, "!new", 4) == 0)
								{
									//if ( !g_IRCSocket.Send( "PRIVMSG #ffirc-system :DEBUG: message is a !join message\r\n" ) )
									//	Msg("[IRC] Unable to send message\n");


									// !new #ffgame-313 ff_monkey 1/8 1500+ pros only!

									char *pToken = NULL;
									pToken = strtok(msg, " "); // !join
									pToken = strtok(NULL, " "); // #ffgame-331

									KeyValues *kv = new KeyValues( "LI" );
									kv->SetString( "channel", pToken );
									kv->SetName( pToken ); // used as a unique identifier to the game

									pToken = strtok(NULL, " "); // ff_monkey
									kv->SetString( "map", pToken );

									pToken = strtok(NULL, " "); // 1/8
									kv->SetString( "players", pToken );

									pToken = strtok(NULL, "\n\r"); // 4v4 1500+ pros
									kv->SetString( "name", pToken );

									m_pLobbyTab->m_pGameList->AddItem( kv, 0, false, false );
									kv->deleteThis();
								}
								// gamelist
								else if (Q_strnicmp(msg, "!games", 5) == 0)
								{
									//if ( !g_IRCSocket.Send( "PRIVMSG #ffirc-system :DEBUG: message is a !join message\r\n" ) )
									//	Msg("[IRC] Unable to send message\n");


									// !games #ffgame-0001 0/8 ff_monkey ranked Capture the Flag 

									char *nextmsg = new char[ strlen(msg) ];
									strcpy( nextmsg, msg ); // necessary because strtok changes the contents of msg


									// think we have to use something other than strtok to get each game string, can then use strtok to get the games details afterwards.
									//char *nextmsg = strchr(msgcopy,'#') + 1;

									char *pToken = NULL;
									pToken = strtok(msg, " "); // !games
									pToken = strtok(NULL, " "); // #ffgame-331

									while ( pToken )
									{																		
										int i = m_pLobbyTab->m_pGameList->GetItem( pToken );

										if ( i > -1 ) // game already exists, just update it
										{
											KeyValues *kv2 = m_pLobbyTab->m_pGameList->GetItem( i );

											pToken = strtok(NULL, " "); // ff_monkey
											kv2->SetString( "map", pToken );

											pToken = strtok(NULL, " "); // 1/8
											kv2->SetString( "players", pToken );

											pToken = strtok(NULL, " "); // Ranked
											kv2->SetString( "ranked", pToken );
/* no ip and password in games summary
											pToken = strtok(NULL, " "); // 123.23.23.23:27013
											kv2->SetString( "serverip", pToken );

											pToken = strtok(NULL, " "); // ffpickup
											kv2->SetString( "serverpassword", pToken );
*/
											pToken = strtok(NULL, "#\n\r"); // 4v4 1500+ pros
											kv2->SetString( "name", pToken );

											m_pLobbyTab->m_pGameList->ApplyItemChanges( i );

										}
										else // new game // !game #ffgame-313 ff_monkey 1/8 ranked 123.23.23.23:27013 ffpickup 1500+ pros only!
										{
											KeyValues *kv = new KeyValues( "LI" );
											kv->SetString( "channel", pToken );
											kv->SetName( pToken ); // used as a unique identifier to the game
										
											pToken = strtok(NULL, " "); // ff_monkey
											kv->SetString( "map", pToken );

											pToken = strtok(NULL, " "); // 1/8
											kv->SetString( "players", pToken );

											pToken = strtok(NULL, " "); // Ranked
											kv->SetString( "ranked", pToken );
/* no ip and password in games summary
											pToken = strtok(NULL, " "); // 123.23.23.23:27013
											kv->SetString( "serverip", pToken );

											pToken = strtok(NULL, " "); // ffpickup
											kv->SetString( "serverpassword", pToken );
*/
											pToken = strtok(NULL, "#\n\r"); // 4v4 1500+ pros
											kv->SetString( "name", pToken );

											m_pLobbyTab->m_pGameList->AddItem( kv, 0, false, false );
											kv->deleteThis();
										}		

										// think we have to use something other than strtok to get each game string, can then use strtok to get the games details afterwards.
										nextmsg = strchr(nextmsg,'#') + 1;
										nextmsg = strchr(nextmsg,'#');
										if ( nextmsg )
										{
											strcpy( msg, nextmsg );
											pToken = strtok(msg, " "); // #ffgame-331
										}
										else
											pToken = NULL;
									}					
								}

								// If new game, then create a new entry in the gamelist
								else if (Q_strnicmp(msg, "!game", 5) == 0)
								{
									//if ( !g_IRCSocket.Send( "PRIVMSG #ffirc-system :DEBUG: message is a !join message\r\n" ) )
									//	Msg("[IRC] Unable to send message\n");


									// !game #ffgame-313 ff_monkey 1/8 1500+ pros only!

									char *pToken = NULL;
									pToken = strtok(msg, " "); // !game
									pToken = strtok(NULL, " "); // #ffgame-331

									int i = m_pLobbyTab->m_pGameList->GetItem( pToken );

									if ( i > -1 ) // game already exists, just update it
									{
										KeyValues *kv2 = m_pLobbyTab->m_pGameList->GetItem( i );

										pToken = strtok(NULL, " "); // ff_monkey
										kv2->SetString( "map", pToken );

										pToken = strtok(NULL, " "); // 1/8
										kv2->SetString( "players", pToken );

										pToken = strtok(NULL, " "); // Ranked
										kv2->SetString( "ranked", pToken );

										pToken = strtok(NULL, " "); // 123.23.23.23:27013
										kv2->SetString( "serverip", pToken );

										pToken = strtok(NULL, " "); // ffpickup
										kv2->SetString( "serverpassword", pToken );

										pToken = strtok(NULL, "\n\r"); // 4v4 1500+ pros
										kv2->SetString( "name", pToken );

										m_pLobbyTab->m_pGameList->ApplyItemChanges( i );

									}
									else // new game // !game #ffgame-313 ff_monkey 1/8 ranked 123.23.23.23:27013 ffpickup 1500+ pros only!
									{
										KeyValues *kv = new KeyValues( "LI" );
										kv->SetString( "channel", pToken );
										kv->SetName( pToken ); // used as a unique identifier to the game
									
										pToken = strtok(NULL, " "); // ff_monkey
										kv->SetString( "map", pToken );

										pToken = strtok(NULL, " "); // 1/8
										kv->SetString( "players", pToken );

										pToken = strtok(NULL, " "); // Ranked
										kv->SetString( "ranked", pToken );

										pToken = strtok(NULL, " "); // 123.23.23.23:27013
										kv->SetString( "serverip", pToken );

										pToken = strtok(NULL, " "); // ffpickup
										kv->SetString( "serverpassword", pToken );

										pToken = strtok(NULL, "\n\r"); // 4v4 1500+ pros
										kv->SetString( "name", pToken );

										m_pLobbyTab->m_pGameList->AddItem( kv, 0, false, false );
										kv->deleteThis();
									}									
								}
								// if a game starts, remove it from the game list
								else if (Q_strnicmp(msg, "!started", 8) == 0)
								{
									//if ( !g_IRCSocket.Send( "PRIVMSG #ffirc-system :DEBUG: message is a !join message\r\n" ) )
									//	Msg("[IRC] Unable to send message\n");

									// !started #ffgame-313

									char *pToken = NULL;
									pToken = strtok(msg, " "); // !started
									pToken = strtok(NULL, " "); // #ffgame-331

									int i = m_pLobbyTab->m_pGameList->GetItem( pToken );
									m_pLobbyTab->m_pGameList->RemoveItem( i );
								}
							}
						}
						
						else
						{
							// skip the #, we know it's there
							buf+=1;
							
							char *bufcopy = new char[ strlen(buf) ];
							strcpy( bufcopy, buf ); // necessary because strtok changes the contents of buf

							char *pChannel = strtok(buf, " ");

							CFFIRCGameTab *pGameTab = GetGameTabByChannel( pChannel );
							if (pGameTab)
							{
								// if from the system bot
								if((Q_strcmp(from, "ff-systembot") == 0) || (Q_strcmp(from, "AfterShok") == 0)) // the only 2 cool dudes on IRC
								{
									char *msg = strchr(bufcopy,':') + 1;

									if (Q_strnicmp(msg, "!connect", 8) == 0)
									{
										char *msg = strchr(bufcopy,':') + 2; // skip the ! and get everything else
										engine->ClientCmd( msg ); // connect xxx.xx.xx.xx:xxx; password xxxx 
										return;								
									}

									// teams listing
									else if (Q_strnicmp(msg, "!team1", 6) == 0)
									{
										//if ( !g_IRCSocket.Send( "PRIVMSG #ffirc-system :DEBUG: message is a !join message\r\n" ) )
										//	Msg("[IRC] Unable to send message\n");

										// !team1: one two three four !team2: five six seven eight


										pGameTab->m_pTeam1List->RemoveAll();
										pGameTab->m_pTeam2List->RemoveAll();

										char *msgcopy = new char[ strlen(msg) ];
										strcpy( msgcopy, msg ); // necessary because strtok changes the contents of msg


										char *pToken = NULL;
										pToken = strtok(msg, " "); // !team1
										//pToken = strtok(NULL, " "); // #ffgame-331

										KeyValues *kv = new KeyValues( "LI" );
										int iTeam = 0;

										while ( pToken )
										{
											if (Q_strnicmp(pToken, "!", 1) == 0)
											{
												pToken = strtok(NULL, " ");
												iTeam++;
											}
											
											while (pToken && Q_strnicmp(pToken, "!", 1) != 0)
											{
												kv->SetString( "players", pToken );
												kv->SetString( "ranking", "1500" );

												if ( iTeam == 1 )
													pGameTab->m_pTeam1List->AddItem( kv, 0, false, false );
												else if ( iTeam == 2 )
													pGameTab->m_pTeam2List->AddItem( kv, 0, false, false );

												pToken = strtok(NULL, " "); // 4v4 1500+ pros											
											}
										}

										kv->deleteThis();

									}

								}
						
								pGameTab->UserMessage(from, msg);
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

						// if PM from the system bot
						if((Q_strcmp(from, "ff-systembot") == 0) || (Q_strcmp(from, "AfterShok") == 0)) // the only 2 cool dudes on IRC
						{

							// DEBUG MESSAGE TO SAY RECEIVED MESSAGE
							//if ( !g_IRCSocket.Send( "PRIVMSG #ffirc-system :DEBUG: received message from the bot\r\n" ) )
							//	Msg("[IRC] Unable to send message\n");


							char *msg = strchr(buf,':') + 1;

							// IF JOIN, THEN JOIN ROOM
							//char *pToken = NULL;
							//pToken = strtok(msg, " ");
							//if (Q_strcmp(pToken, "!join") == 0)
							if (Q_strnicmp(msg, "!join", 5) == 0)
							{
								//if ( !g_IRCSocket.Send( "PRIVMSG #ffirc-system :DEBUG: message is a !join message\r\n" ) )
								//	Msg("[IRC] Unable to send message\n");

								char *pToken = NULL;
								pToken = strtok(msg, " "); // !join
								pToken = strtok(NULL, " "); // #ffgame-331

								AddGameTab(  VarArgs( "%s",pToken ), VarArgs( "%s",pToken ) );

								// This crashes because this is currently inside a thread
								//AddGameTab( "#ffgame-313", "#ffgame-313"); 

							}
							//else if (Q_strcmp(pToken, "!connect") == 0)
							else if (Q_strnicmp(msg, "!connect", 8) == 0)
							{
								// obviously need to parse this, maybe just trim the first char?

								//char *pToken = NULL;
								//pToken = strtok(msg, " ");
								//pToken = strtok(NULL, " ");

								char *msg = strchr(buf,':') + 2; // skip the ! and get everything else
								engine->ClientCmd( msg ); // connect xxx.xx.xx.xx:xxx password xxxx 
							}
						}

						// else, normal PM:
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
							if(m_pLobbyTab->UserList_IsInList(from))
							{
								m_pLobbyTab->SystemMessage( VarArgs("%s is now known as %s", from, buf) );
								m_pLobbyTab->UserList_UpdateUserName(from, buf);
							}
						}
						// tell game tabs
						for(int i=0; i<m_pGameTabs.Count(); ++i)
						{
							if(m_pGameTabs.IsValidIndex(i))
							{
								CFFIRCGameTab *pGameTab = m_pGameTabs.Element(i);
								if(pGameTab->UserList_IsInList(from))
								{
									pGameTab->SystemMessage( VarArgs("%s is now known as %s", from, buf) );
									pGameTab->UserList_UpdateUserName(from, buf);
								}
							}
						}
						return;
					}
				}
				// QUIT
				//		:squeek!~squeek502@squeek.user.gamesurge QUIT :Reason
				else if (Q_strcmp(command,"QUIT") == 0)
				{
					char *msg = strchr(buf,':') + 1;

					// need to update all open tabs
					if (m_pLobbyTab)
					{
						if(m_pLobbyTab->UserList_IsInList(from))
						{
							m_pLobbyTab->SystemMessage( VarArgs("%s has quit (%s)", from, (msg ? msg : "Unknown reason")) );
							m_pLobbyTab->UserList_RemoveUser(from);
						}
					}
					// tell game tabs
					for(int i=0; i<m_pGameTabs.Count(); ++i)
					{
						if(m_pGameTabs.IsValidIndex(i))
						{
							CFFIRCGameTab *pGameTab = m_pGameTabs.Element(i);
							if(pGameTab->UserList_IsInList(from))
							{
								pGameTab->SystemMessage( VarArgs("%s has quit (%s)", from, (msg ? msg : "Unknown reason")) );
								pGameTab->UserList_RemoveUser(from);
							}
						}
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
						// game tab?
						else
						{
							// skip the #, we know it's there
							buf+=1;

							char *pChannel = strtok(buf, " ");

							CFFIRCGameTab *pGameTab = GetGameTabByChannel( pChannel );
							if (pGameTab)
							{
								pGameTab->SystemMessage( VarArgs("%s has joined the room", from) );
								pGameTab->UserList_AddUser(from);
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
						// game tab?
						else
						{
							// skip the #, we know it's there
							buf+=1;

							char *pChannel = strtok(buf, " ");

							CFFIRCGameTab *pGameTab = GetGameTabByChannel( pChannel );
							if (pGameTab)
							{
								pGameTab->SystemMessage( VarArgs("%s has left the room", from) );
								pGameTab->UserList_RemoveUser(from);
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
						// skip the #, we know it's there
						buf+=1;

						char *pChannel = buf;

						// skip the chan param, go to the next (modes)
						buf = strchr(buf,' ') + 1;

						pChannel = strtok(pChannel, " ");

						char *params = strchr(buf,' ');
						
						// if there are more spaces, then user modes are being set
						// we'll just ignore channel modes
						if (params)
						{
							// skip the space char
							params++;

							char modifier = 0;
							char mode;
							for (int i=0; buf[i] != ' '; ++i)
							{
								if (buf[i] == '+')
									modifier='+';
								else if (buf[i] == '-')
									modifier='-';
								else if (buf[i] == 'o' || buf[i] == 'v' || buf[i] == 'b')
								{
									// if no modifier yet, things are not going well
									if( !modifier )
										continue;

									mode = buf[i];
									
									char moded[IRC_MAX_NICKLEN+1];
									moded[IRC_MAX_NICKLEN] = '\0';
									
									for(int ti=0;ti<IRC_MAX_NICKLEN;++ti)
									{
										if (params[ti] == ' ')
										{
											moded[ti] = '\0';
											break;
										}
										moded[ti]=params[ti];
									}

									if (Q_stricmp(pChannel, "fortressforever") == 0)
									{
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
									}
									// game tab?
									else
									{
										CFFIRCGameTab *pGameTab = GetGameTabByChannel( pChannel );
										if (pGameTab)
										{
											pGameTab->SystemMessage( VarArgs("%s sets mode: %c%c %s", from, modifier, mode, moded) );
											// +
											if( modifier=='+' )
											{
												// op +o
												if (mode=='o')
													pGameTab->UserList_UpdateUserAccess( moded, 2 );
												// vip +v
												else if (mode=='v')
													pGameTab->UserList_UpdateUserAccess( moded, 1, false );
												// ban +b
												//else if (mode=='b')
											}
											// -
											else
											{
												// delop delvip -o -v
												if (mode=='v' || mode=='o')
													pGameTab->UserList_UpdateUserAccess( moded, 0 );
												// unban -b
												//if (mode=='b')
											}
										}
									}

									params = strchr(params,' ');
								}
							} // end for
						} // end if (params)
					}
				}
				// KICK
				//		:ChanServ!ChanServ@Services.GameSurge.net KICK #FortressForever squeektest :(squeek) test
				else if (Q_strcmp(command,"KICK") == 0)
				{
					// just to make sure a channel is being parted
					if (buf[0] == '#')
					{
						// skip the chan param, go to the next (kicked user)
						char *pKicked = strchr(buf,' ') + 1;

						// MAXNICKLEN is 30
						char kicked[IRC_MAX_NICKLEN+1];
						kicked[IRC_MAX_NICKLEN] = '\0';
						
						for(int ti=0;ti<30;++ti)
						{
							if (pKicked[ti] == ' ')
							{
								kicked[ti] = '\0';
								break;
							}
							kicked[ti]=pKicked[ti];
						}
						
						// skip to the reason
						char *pReason = strchr(buf,':') + 1;

						// #fortressforever
						if (Q_strnicmp((buf+1), "fortressforever", 15) == 0)
						{

							if (m_pLobbyTab)
							{
								m_pLobbyTab->SystemMessage( VarArgs("%s has been kicked by %s (Reason: %s)", kicked, from, (pReason ? pReason : "No reason given")) );
								m_pLobbyTab->UserList_RemoveUser(kicked);
								return;
							}
						}
						// game tab?
						else
						{
							// skip the #, we know it's there
							buf+=1;

							char *pChannel = strtok(buf, " ");

							CFFIRCGameTab *pGameTab = GetGameTabByChannel( pChannel );
							if (pGameTab)
							{
								pGameTab->SystemMessage( VarArgs("%s has been kicked by %s (Reason: %s)", kicked, from, (pReason ? pReason : "No reason given")) );
								pGameTab->UserList_RemoveUser(kicked);
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
					char *msg = strchr(buf,':') + 1;

					// going to a channel
					if (buf[0] == '#')
					{
						// #fortressforever
						if (Q_strnicmp((buf+1), "fortressforever", 15) == 0)
						{
							if (m_pLobbyTab)
							{
								m_pLobbyTab->UserMessage(from, msg, Color(244,244,190,255));
								return;
							}
						}
						// game tab?
						else
						{
							// skip the #, we know it's there
							buf+=1;

							char *pChannel = strtok(buf, " ");

							CFFIRCGameTab *pGameTab = GetGameTabByChannel( pChannel );
							if (pGameTab)
							{
								pGameTab->UserMessage(from, msg, Color(244,244,190,255));
								return;
							}
						}
					}
					// going to the local user
					else if (Q_strncmp(buf, irc_user.nick, Q_strlen(irc_user.nick)) == 0)
					{
						if (m_pLobbyTab)
						{
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
					// if the server changed our nickname automatically
					if (Q_strcmp(pUser, irc_user.nick) != 0)
					{
						// update ours to what the server says it is
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
						char *msg = strchr(pszChan,':') + 1;

						// #fortressforever
						if (Q_strnicmp((pszChan+1), "fortressforever", 15) == 0)
						{
							if (m_pLobbyTab)
							{
								m_pLobbyTab->UserList_Fill(msg);
								return;
							}
						}
						// game tab?
						else
						{
							// skip the #, we know it's there
							pszChan+=1;

							char *pChannel = strtok(pszChan, " ");

							CFFIRCGameTab *pGameTab = GetGameTabByChannel( pChannel );
							if (pGameTab)
							{
								pGameTab->UserList_Fill(msg);
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
						// game tab?
						else
						{
							// skip the #, we know it's there
							pszChan+=1;

							char *pChannel = strtok(pszChan, " ");

							CFFIRCGameTab *pGameTab = GetGameTabByChannel( pChannel );
							if (pGameTab)
							{
								pGameTab->UserList_SetReceiving( false );
								return;
							}
						}
					}
				}
			}
		}
	}
}

void CFFIRCPanel::OnTick()
{
	if (m_bDataReady)
	{
		m_bDataReady = false;
		RetrieveServerMessage();
	}

	BaseClass::OnTick();
}

void CFFIRCPanel::RetrieveServerMessage()
{
	static char buf[3000];
	int a = g_IRCSocket.RecvNoCheck(buf, sizeof(buf)-1);

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

	m_pTextEntry_UsernameEntry = new vgui::TextEntry(this, "UsernameEntry");
	m_pTextEntry_UsernameEntry->SetEditable( true );
	m_pTextEntry_UsernameEntry->SetText( "" );

	m_pTextEntry_PasswordEntry = new vgui::TextEntry(this, "PasswordEntry");
	m_pTextEntry_PasswordEntry->SetEditable( true );
	m_pTextEntry_PasswordEntry->SetText( "" );

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
		m_pTextEntry_UsernameEntry->GetText( curname, 30 );

		if ( cvar_name && !curname[0] )
		{
			m_pTextEntry_NickEntry->SetText(cvar_name->GetString());
			m_pTextEntry_UsernameEntry->SetText(cvar_name->GetString());
		}
		m_pTextEntry_UsernameEntry->RequestFocus();
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
	m_pTextEntry_UsernameEntry->SetEditable( true );
	m_pTextEntry_UsernameEntry->SetEnabled( true );
	m_pTextEntry_PasswordEntry->SetEditable( true );
	m_pTextEntry_PasswordEntry->SetEnabled( true );
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

	// this is producing random crashes as it's inside a thread
	/*
	if (g_pIRCPanel)
	{
		g_pIRCPanel->AddGameTab( "Debug", "#ffirc-system" );
	}*/
}

void CFFIRCConnectPanel::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	if (Q_strcmp(pszCommand, "OK") == 0)
	{
		char szNick[30];
		m_pTextEntry_NickEntry->GetText(szNick, sizeof(szNick));
		sprintf(g_pIRCPanel->irc_user.nick, szNick);

		// Need to store username and password here for later  - AfterShock

		CFFIRCThread::GetInstance().Start();

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
		if (!g_IRCSocket.Connect("irc.quakenet.org", 6667)) 
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
		m_pTextEntry_UsernameEntry->SetEditable( false );
		m_pTextEntry_UsernameEntry->SetEnabled( false );
		m_pTextEntry_PasswordEntry->SetEditable( false );
		m_pTextEntry_PasswordEntry->SetEnabled( false );
		m_pStatusLabel->SetText( "Connecting..." );
		m_pStatusLabel->SetVisible( true );
	}
	if (Q_strcmp(pszCommand, "Cancel") == 0)
		SetVisible(false);
}


//-----------------------------------------------------------------------------
// Purpose: Catch the buttons. We have OK (save + close), Cancel (close) and
//			Apply (save).
//-----------------------------------------------------------------------------
CFFIRCHostPanel::CFFIRCHostPanel( vgui::VPANEL parent ) : BaseClass( NULL, "FFIRCHostPanel" )
{
	if (g_pIRCHostPanel == NULL)
		g_pIRCHostPanel = this;

	m_pRankedTab = new CFFIRCHostRankedTab(this, "IRCHostRankedTab");
	m_pUnrankedTab = new CFFIRCHostUnrankedTab(this, "IRCHostUnrankedTab");
	m_pIRCHostTabs = new vgui::PropertySheet(this, "IRCHostTabs", true);

	m_pIRCHostTabs->AddPage(m_pRankedTab, "Ranked Game");
	m_pIRCHostTabs->AddPage(m_pUnrankedTab, "Unranked Game");
	m_pIRCHostTabs->SetActivePage(m_pRankedTab);
	m_pIRCHostTabs->SetDragEnabled(false);

	SetSizeable(false);
	LoadControlSettings("resource/ui/FFIRCHost.res");
}

void CFFIRCHostPanel::SetVisible(bool state)
{
	// set unranked game name if it's empty
	if( m_pUnrankedTab->m_pTextEntry_NameEntry->GetTextLength() == 0 )
	{
		char szGameName[32];
		sprintf( szGameName, "%s's game", g_pIRCPanel->irc_user.nick );
		m_pUnrankedTab->m_pTextEntry_NameEntry->SetText( szGameName );
	}

	if (state)
	{
		// Centre this panel on the screen for consistency.
		int nWide = GetWide();
		int nTall = GetTall();

		SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

		RequestFocus();
		MoveToFront();
	}
	BaseClass::SetVisible(state);
}

CFFIRCHostRankedTab::CFFIRCHostRankedTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
{
	new Button(this, "CreateButton", "", this, "Create");
	new Button(this, "CancelButton", "", this, "Cancel");

	m_pMapList = new ListPanel(this, "ListPanel_MapList");
	m_pMapList->AddColumnHeader( 0, "mapname" , "Map Name", 150, ListPanel::COLUMN_RESIZEWITHWINDOW );
    m_pMapList->AddColumnHeader( 1, "gametype" , "Gametype", 200, ListPanel::COLUMN_RESIZEWITHWINDOW );
    m_pMapList->AddColumnHeader( 2, "players" , "Players", 100, ListPanel::COLUMN_RESIZEWITHWINDOW );

	LoadControlSettings("resource/ui/FFIRCHostRankedTab.res");
	LoadMaps();
}

void CFFIRCHostRankedTab::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	if (Q_strcmp(pszCommand, "Create") == 0)
	{
		KeyValues *KV = m_pMapList->GetItem( m_pMapList->GetSelectedItem(0) );

		char szCommand[256];
		sprintf( szCommand, "PRIVMSG ff-systembot :!host %s %s %s %s ranked %s\r\n", KV->GetString("mapname"), KV->GetString("players"), "ip", "password", KV->GetString("gametype") );

		if ( !g_IRCSocket.Send( szCommand ) )
			Msg("[IRC] Unable to send message: !hos\n");

		g_pIRCHostPanel->SetVisible(false);
	}

	if (Q_strcmp(pszCommand, "Cancel") == 0)
		g_pIRCHostPanel->SetVisible(false);
}

void CFFIRCHostRankedTab::LoadMaps()
{
	KeyValues *kv = new KeyValues( "LI" );

	kv->SetString( "gametype", "Capture the Flag" );
	kv->SetString( "players", "8" );
	kv->SetString( "mapname", "ff_monkey" );
	m_pMapList->AddItem( kv, 0, false, false );
	kv->SetString( "mapname", "ff_destroy" );
	m_pMapList->AddItem( kv, 0, false, false );
	kv->SetString( "mapname", "ff_schtop" );
	m_pMapList->AddItem( kv, 0, false, false );
	kv->SetString( "mapname", "ff_shutdown2" );
	m_pMapList->AddItem( kv, 0, false, false );
	kv->SetString( "mapname", "ff_bases" );
	m_pMapList->AddItem( kv, 0, false, false );
	kv->SetString( "mapname", "ff_2fort" );
	m_pMapList->AddItem( kv, 0, false, false );

	kv->SetString( "gametype", "Attack and Defend" );
	kv->SetString( "players", "10" );
	kv->SetString( "mapname", "ff_ksour" );
	m_pMapList->AddItem( kv, 0, false, false );
	kv->SetString( "mapname", "ff_napoli" );
	m_pMapList->AddItem( kv, 0, false, false );
	kv->SetString( "mapname", "ff_palermo" );
	m_pMapList->AddItem( kv, 0, false, false );
	kv->SetString( "mapname", "ff_dustbowl" );
	m_pMapList->AddItem( kv, 0, false, false );

	kv->SetString( "gametype", "Command Point" );
	kv->SetString( "players", "12" );
	kv->SetString( "mapname", "ff_warpath" );
	m_pMapList->AddItem( kv, 0, false, false );

	kv->deleteThis();
	m_pMapList->SetSingleSelectedItem(0);
}

CFFIRCHostUnrankedTab::CFFIRCHostUnrankedTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
{
	new Button(this, "CreateButton", "", this, "Create");
	new Button(this, "CancelButton", "", this, "Cancel");

	m_pTextEntry_NameEntry = new vgui::TextEntry(this, "NameEntry");

	m_pMapCombo = new ComboBox(this, "Map", 0, true);
	LoadMaps();
	m_pMapCombo->ActivateItem(0);

	m_pPlayersCombo = new ComboBox(this, "Players", 0, true);
	//m_pAutoTeamsCheck = new CheckButton(this, "AutoTeams", "Auto-Assign Teams Using Player Rankings");

	KeyValues *kv = new KeyValues( "LI" );
	kv->SetString( "players", "2" );
	m_pPlayersCombo->AddItem("1v1", kv);
	kv->SetString( "players", "4" );
	m_pPlayersCombo->AddItem("2v2", kv);
	kv->SetString( "players", "6" );
	m_pPlayersCombo->AddItem("3v3", kv);
	kv->SetString( "players", "8" );
	m_pPlayersCombo->AddItem("4v4", kv);
	kv->SetString( "players", "10" );
	m_pPlayersCombo->AddItem("5v5", kv);
	kv->SetString( "players", "12" );
	m_pPlayersCombo->AddItem("6v6", kv);
	kv->SetString( "players", "14" );
	m_pPlayersCombo->AddItem("7v7", kv);
	kv->SetString( "players", "16" );
	m_pPlayersCombo->AddItem("8v8", kv);
	kv->deleteThis();

	m_pPlayersCombo->ActivateItem(3);

	LoadControlSettings("resource/ui/FFIRCHostUnrankedTab.res");
}

void CFFIRCHostUnrankedTab::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	if (Q_strcmp(pszCommand, "Create") == 0)
	{
		KeyValues *KVPlayers = m_pPlayersCombo->GetActiveItemUserData();
		KeyValues *KVMap = m_pMapCombo->GetActiveItemUserData();

		char szName[41];
		m_pTextEntry_NameEntry->GetText(szName, sizeof(szName));

		char szCommand[256];
		sprintf( szCommand, "PRIVMSG ff-systembot :!host %s %s %s %s unranked %s\r\n", KVMap->GetString("map"), KVPlayers->GetString("players"), "ip", "password", szName );
		
		if ( !g_IRCSocket.Send( szCommand ) )
			Msg("[IRC] Unable to send message: !hos\n");

		g_pIRCHostPanel->SetVisible(false);
	}

	if (Q_strcmp(pszCommand, "Cancel") == 0)
		g_pIRCHostPanel->SetVisible(false);
}

void CFFIRCHostUnrankedTab::LoadMaps()
{
	// list of all the maps on the server
	// my plan is to have it load this list from a text file hosted online so we can easily update it as maps are added to the server
	// for now, here's a hardcoded list of all the maps on my laptop... - caes

	AddMap( "ff_2fort" );
	AddMap( "ff_2morforever" );
	AddMap( "ff_aardvark" );
	AddMap( "ff_anticitizen" );
	AddMap( "ff_attrition" );
	AddMap( "ff_bases" );
	AddMap( "ff_cornfield" );
	AddMap( "ff_crossover" );
	AddMap( "ff_cz2" );
	AddMap( "ff_destroy" );
	AddMap( "ff_dm" );
	AddMap( "ff_dropdown" );
	AddMap( "ff_dustbowl" );
	AddMap( "ff_epicenter" );
	AddMap( "ff_fusion" );
	AddMap( "ff_genesis" );
	AddMap( "ff_hunted" );
	AddMap( "ff_impact" );
	AddMap( "ff_ksour" );
	AddMap( "ff_ksour_classic" );
	AddMap( "ff_monkey" );
	AddMap( "ff_napoli" );
	AddMap( "ff_openfire" );
	AddMap( "ff_palermo" );
	AddMap( "ff_pitfall" );
	AddMap( "ff_push" );
	AddMap( "ff_roasted" );
	AddMap( "ff_schtop" );
	AddMap( "ff_shutdown2" );
	AddMap( "ff_siden_b2" );
	AddMap( "ff_tiger" );
	AddMap( "ff_vertigo" );
	AddMap( "ff_waterpolo" );
	AddMap( "ff_well" );
}

void CFFIRCHostUnrankedTab::AddMap( const char *mapname )
{
	KeyValues *kv = new KeyValues( "LI" );
	kv->SetString( "map", mapname );
	m_pMapCombo->AddItem( mapname, kv );
	kv->deleteThis();
}
