//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <stdio.h>

#include <cdll_client_int.h>
#include <cdll_util.h>
#include <globalvars_base.h>
#include <igameresources.h>

#include "clientscoreboarddialog.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vstdlib/IKeyValuesSystem.h>

#include <KeyValues.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>

#include <cl_dll/iviewport.h>
#include <igameresources.h>

#include "voice_status.h"
//#include "Friends/IFriendsUser.h"

#include "in_buttons.h"

#include "IGameUIFuncs.h" // for key bindings
extern IGameUIFuncs *gameuifuncs; // for key binding details

#include "ff_gamerules.h"
#include "c_ff_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// extern vars
//extern IFriendsUser *g_pFriendsUser;

extern bool g_fBlockedStatus[256];	// |-- Mirv: The blocked status of people's text

static CClientScoreBoardDialog *g_pScoreboard = NULL;

inline char *GetFormattedMapName( void )
{	
	static char szText[ 256 ];
	Q_strcpy( szText, engine->GetLevelName() + 5 ); // Skip the "maps/" part
	szText[ ( int )strlen( szText ) - 4 ] = '\0'; // Skip the ".bsp" part

	return szText;
}

bool ActivateScoreboard()
{
	if (!g_pScoreboard || !g_pScoreboard->IsVisible())
		return false;

	// If not enabled, set mouse input as enabled and return true to swallow +attack
	if (!g_pScoreboard->IsMouseInputEnabled())
	{
		g_pScoreboard->SetMouseInputEnabled(true);
		return true;
	}

	return false;
}

using namespace vgui;

const char *szClassName[] = {	"", 
								"#FF_SCOREBOARD_SCOUT", 
								"#FF_SCOREBOARD_SNIPER",
								"#FF_SCOREBOARD_SOLDIER",
								"#FF_SCOREBOARD_DEMOMAN",
								"#FF_SCOREBOARD_MEDIC",
								"#FF_SCOREBOARD_HWGUY",
								"#FF_SCOREBOARD_PYRO",
								"#FF_SCOREBOARD_SPY",
								"#FF_SCOREBOARD_ENGINEER",
								"#FF_SCOREBOARD_CIVILIAN" };

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClientScoreBoardDialog::CClientScoreBoardDialog(IViewPort *pViewPort) : Frame( NULL, PANEL_SCOREBOARD )
{
	m_iPlayerIndexSymbol = KeyValuesSystem()->GetSymbolForString("playerIndex");

	memset( s_VoiceImage, 0x0, sizeof( s_VoiceImage ) );
	memset( s_ChannelImage, 0x0, sizeof( s_ChannelImage ) ); // |-- Mirv: Voice channels
	TrackerImage = 0;
	m_pViewPort = pViewPort;

	// initialize dialog
	SetProportional(true);
	SetKeyBoardInputEnabled(false);
	SetMouseInputEnabled(true);			// We have to enable this in the constructor
										// even if it won't always be the case
										// Otherwise panels within this one will not
										// receive mouse input (thanks valve)
	SetSizeable(false);

	// hide the system buttons
	SetTitleBarVisible( false );

	// set the scheme before any child control is created
	SetScheme("ClientScheme");

	m_iJumpKey = -1;

	m_pMapName = new Label( this, "MapName", GetFormattedMapName() );

	m_pPlayerList = new SectionedListPanel(this, "PlayerList");
	m_pPlayerList->SetVerticalScrollbar(false);

	m_pChannelButton = new Button( this, "channelbutton", "#FF_CHANNEL_GLOBAL" );
	m_pChannelButton->SetCommand( "setchannel 1" );

	LoadControlSettings("Resource/UI/ScoreBoard.res");
	m_iDesiredHeight = GetTall();
	m_pPlayerList->SetVisible( false ); // hide this until we load the images in applyschemesettings
	m_pPlayerList->AddActionSignalTarget( this );

	m_HLTVSpectators = 0;
	
	// update scoreboard instantly if on of these events occure
	gameeventmanager->AddListener(this, "hltv_status", false );
	gameeventmanager->AddListener(this, "server_spawn", false );

	for( int i = 0; i < TEAM_COUNT; i++ )
	{
		m_iTeamLatency[ i ] = 0;
		m_iNumPlayersOnTeam[ i ] = 0;
		m_iTeamSections[ i ] = 0;
	}

	g_pScoreboard = this;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CClientScoreBoardDialog::~CClientScoreBoardDialog()
{
	gameeventmanager->RemoveListener(this);
}

// --> Mirv: Catches the set channel button
//-----------------------------------------------------------------------------
// Purpose: Run the client command if needed
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::OnCommand( const char *command )
{
	DevMsg( "command: %s\n", command );

	engine->ClientCmd( command );

	// This is, on the whole, quite a silly way to do it, but I want to test out the
	// channel filtering. Will tidy this up later
	if( Q_strcmp( command, "setchannel 0" ) == 0 )
	{
		DevMsg( "setting to 1\n" );
		m_pChannelButton->SetCommand( "setchannel 1" );
		m_pChannelButton->SetText( "#FF_CHANNEL_GLOBAL" );
	}
	else if( Q_strcmp( command, "setchannel 1" ) == 0 )
	{
		DevMsg( "setting to 2\n" );
		m_pChannelButton->SetCommand( "setchannel 2" );
		m_pChannelButton->SetText( "#FF_CHANNEL_A" );
	}
	else
	{
		DevMsg( "setting to 0\n" );
		m_pChannelButton->SetCommand( "setchannel 0" );
		m_pChannelButton->SetText( "#FF_CHANNEL_B" );
	}

	// Update straight away
	Update();

	BaseClass::OnCommand(command);
}
// <-- Mirv: Catches the set channel button

//-----------------------------------------------------------------------------
// Purpose: clears everything in the scoreboard and all it's state
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::Reset()
{
	// clear
	m_pPlayerList->DeleteAllItems();
	m_pPlayerList->RemoveAllSections();

	m_iSectionId = 0;
	m_fNextUpdateTime = 0;
	// add all the sections
	InitScoreboardSections();
}

//-----------------------------------------------------------------------------
// Purpose: adds all the team sections to the scoreboard
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::InitScoreboardSections()
{
	AddHeader( );
}

//-----------------------------------------------------------------------------
// Purpose: sets up screen
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::ApplySchemeSettings( IScheme *pScheme )
{
	BaseClass::ApplySchemeSettings( pScheme );
	ImageList *imageList = new ImageList( false );

	s_VoiceImage[ 0 ] = 0;	// index 0 is always blank
	s_VoiceImage[ CVoiceStatus::VOICE_NEVERSPOKEN ] = imageList->AddImage( scheme( )->GetImage( "640_speaker1", true ) );
	s_VoiceImage[ CVoiceStatus::VOICE_NOTTALKING ] = imageList->AddImage( scheme( )->GetImage( "640_speaker2", true ) );
	s_VoiceImage[ CVoiceStatus::VOICE_TALKING ] = imageList->AddImage( scheme( )->GetImage( "640_speaker3", true ) );
	s_VoiceImage[ CVoiceStatus::VOICE_BANNED ] = imageList->AddImage( scheme( )->GetImage( "640_voiceblocked", true ) );
	s_VoiceImage[ CVoiceStatus::VOICE_BANNEDTEXT ] = imageList->AddImage( scheme( )->GetImage( "640_textblocked", true ) );		// |-- Mirv: Text ban

	// --> Mirv: Channel images
	s_ChannelImage[0] = 0;
	s_ChannelImage[ CHANNEL::NONE ] = imageList->AddImage( scheme()->GetImage( "640_channelnone", true ) );
	s_ChannelImage[ CHANNEL::CHANNELA ] = imageList->AddImage( scheme()->GetImage( "640_channela", true ) );
	s_ChannelImage[ CHANNEL::CHANNELB ] = imageList->AddImage( scheme()->GetImage( "640_channelb", true ) );
	// <-- Mirv: Channel images

	TrackerImage = imageList->AddImage( scheme( )->GetImage( "640_scoreboardtracker", true ) );

	// --> Mirv: Image resizing made things a mess
	// resize the images to our resolution
/*	for (int i = 0; i < imageList->GetImageCount(); i++ )
	{
		int wide, tall;
		imageList->GetImage(i)->GetSize(wide, tall);
		DevMsg( "Image changed from %d x %d to %d %d", wide, tall, (int)scheme()->GetProportionalScaledValue(wide), (int)scheme()->GetProportionalScaledValue(tall) );
		imageList->GetImage(i)->SetSize(scheme()->GetProportionalScaledValue(wide), scheme()->GetProportionalScaledValue(tall));
	}*/
	// <-- Mirv: Image resizing made things a mess

	m_pPlayerList->SetImageList(imageList, false);
	m_pPlayerList->SetVisible( true );

	// light up scoreboard a bit
	SetBgColor( Color( 0,0,0,0) );
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		if( m_iJumpKey == -1 ) // you need to lookup the jump key AFTER the engine has loaded
		{
			m_iJumpKey = gameuifuncs->GetEngineKeyCodeForBind( "jump" );
		}

		//SetMouseInputEnabled(true);

		Reset();
		Update();

		Activate();

		SetMouseInputEnabled(false);
	}
	else
	{
		BaseClass::SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}
}



void CClientScoreBoardDialog::FireGameEvent( IGameEvent *event )
{
	const char * type = event->GetName();

	if ( Q_strcmp(type, "hltv_status") == 0 )
	{
		// spectators = clients - proxies
		m_HLTVSpectators = event->GetInt( "clients" );
		m_HLTVSpectators -= event->GetInt( "proxies" );
	}

	else if ( Q_strcmp(type, "server_spawn") == 0 )
	{
		SetControlString("ServerName", event->GetString("hostname") );
		MoveLabelToFront("ServerName");
	}

	if( IsVisible() )
		Update();

}

bool CClientScoreBoardDialog::NeedsUpdate( void )
{
	return (m_fNextUpdateTime < gpGlobals->curtime);
}

//-----------------------------------------------------------------------------
// Purpose: Recalculate the internal scoreboard data
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::Update( void )
{
	// Set the title
	
	// Reset();
	m_pPlayerList->DeleteAllItems();
	
	FillScoreBoard();

	// grow the scoreboard to fit all the players
	int wide, tall;
	m_pPlayerList->GetContentSize(wide, tall);
	wide = GetWide();
	if (m_iDesiredHeight < tall)
	{
		SetSize(wide, tall);
		m_pPlayerList->SetSize(wide, tall);
	}
	else
	{
		SetSize(wide, m_iDesiredHeight);
		m_pPlayerList->SetSize(wide, m_iDesiredHeight);
	}

	m_pMapName->SetText( GetFormattedMapName() );

	MoveToCenterOfScreen();

	// update every second
	m_fNextUpdateTime = gpGlobals->curtime + 1.0f; 
}

//-----------------------------------------------------------------------------
// Purpose: Sort all the teams
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::UpdateTeamInfo( )
{
	IGameResources *pGR = GameResources( );
	if( !pGR )
		return;

	for( int i = TEAM_UNASSIGNED; i < TEAM_COUNT; i++ )
	{
		int iSectionId = m_iTeamSections[ i ];

		//Q_snprintf( pDest, destLen, char const *pFormat, ... );

		// --> Mirv: Fixed localisation on teams
		wchar_t *szName = localize()->Find( pGR->GetTeamName( i ) );
		wchar_t szTeamLine[ 256 ];	

		if( szName )
		{
			swprintf( szTeamLine, L"%s - (%i players)", szName, m_iNumPlayersOnTeam[i] );
		}
		else
		{
			char szString[ 256 ];
			Q_snprintf( szString, 256, "%s - (%i players)", pGR->GetTeamName( i ), m_iNumPlayersOnTeam[ i ] );

			localize( )->ConvertANSIToUnicode( szString, szTeamLine, sizeof( szTeamLine ) );
		}
		// <-- Mirv: Fixed localisation on teams

		// Display team name & # of players
		m_pPlayerList->ModifyColumn( iSectionId, "name", szTeamLine );		

		// Set up team score
		wchar_t szScore[ 6 ];
		swprintf( szScore, L"%d", pGR->GetTeamScore( i ) );
		
		// Display team score
		m_pPlayerList->ModifyColumn( iSectionId, "score", szScore );

		// Set up team deaths
		wchar_t szDeaths[ 6 ];
		swprintf( szDeaths, L"%d", pGR->GetTeamDeaths( i ) );

		// Display team deaths
		m_pPlayerList->ModifyColumn( iSectionId, "deaths", szDeaths );

		// Set up team latency
		if( m_iNumPlayersOnTeam[ i ] > 0 )
			m_iTeamLatency[ i ] /= m_iNumPlayersOnTeam[ i ];
		else
			m_iTeamLatency[ i ] = 0;

		// Display team latency
		if( m_iTeamLatency[ i ] < 1 )
			m_pPlayerList->ModifyColumn( iSectionId, "ping", L"" );
		else
		{
			wchar_t szPing[ 12 ];

			swprintf( szPing, L"%i", m_iTeamLatency[ i ] );
			m_pPlayerList->ModifyColumn( iSectionId, "ping", szPing );
		}

		// Reset team latency
		m_iTeamLatency[ i ] = 0;

		// Color it
		m_pPlayerList->SetSectionFgColor( iSectionId, pGR->GetTeamColor( i ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::UpdatePlayerInfo( )
{
	m_iSectionId = 0;
	int iSelectedRow = -1;

	// Walk all the players and make sure they're in the scoreboard
	for( int i = 1; i < gpGlobals->maxClients; i++ )
	{
		IGameResources *pGR = GameResources( );

		if( pGR && pGR->IsConnected( i ) )
		{
			// Add the player to the list
			KeyValues *pPlayerData = new KeyValues( "data" );
			GetPlayerScoreInfo( i, pPlayerData );

			const char *pszOldName = pPlayerData->GetString( "name", "" );
			int iBufSize = ( int )strlen( pszOldName ) * 2;
			char *pszNewName = ( char * )_alloca( iBufSize );

			UTIL_MakeSafeName( pszOldName, pszNewName, iBufSize );

			pPlayerData->SetString( "name", pszNewName );

			int iItemId = FindItemIDForPlayerIndex( i );
			int iPlayerTeam = pGR->GetTeam( i );
			//int iSectionId = pGR->GetTeam( i );
			int iSectionId = m_iTeamSections[ iPlayerTeam ];

			m_iNumPlayersOnTeam[ iPlayerTeam ]++;
			m_iTeamLatency[ iPlayerTeam ] += pPlayerData->GetInt( "ping" );

			if( pGR->IsLocalPlayer( i ) )
			{
				iSelectedRow = iItemId;
			}

			if( iItemId == -1 )
			{
				// Add a new row
				iItemId = m_pPlayerList->AddItem( iSectionId, pPlayerData );
			}
			else
			{
				// Modify the current row
				m_pPlayerList->ModifyItem( iItemId, iSectionId, pPlayerData );
			}

			// Set the row color based on players team
			m_pPlayerList->SetItemFgColor( iItemId, pGR->GetTeamColor( iSectionId ) );

			pPlayerData->deleteThis( );
		}
		else
		{
			// Remove the player
			int iItemId = FindItemIDForPlayerIndex( i );

			if( iItemId != -1 )
			{
				m_pPlayerList->RemoveItem( iItemId );
			}
		}
	}

	if( iSelectedRow != -1 )
	{
		m_pPlayerList->SetSelectedItem( iSelectedRow );
	}
}

//-----------------------------------------------------------------------------
// Purpose: adds the top header of the scoreboars
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::AddHeader()
{
	/*
	// add the top header
	m_pPlayerList->AddSection(m_iSectionId, "");
	m_pPlayerList->SetSectionAlwaysVisible(m_iSectionId);
	m_pPlayerList->AddColumnToSection(m_iSectionId, "name", "#PlayerName", 0, scheme()->GetProportionalScaledValue(NAME_WIDTH) );
	m_pPlayerList->AddColumnToSection(m_iSectionId, "frags", "#PlayerScore", 0, scheme()->GetProportionalScaledValue(SCORE_WIDTH) );
	m_pPlayerList->AddColumnToSection(m_iSectionId, "deaths", "#PlayerDeath", 0, scheme()->GetProportionalScaledValue(DEATH_WIDTH) );
	m_pPlayerList->AddColumnToSection(m_iSectionId, "ping", "#PlayerPing", 0, scheme()->GetProportionalScaledValue(PING_WIDTH) );
//	m_pPlayerList->AddColumnToSection(m_iSectionId, "voice", "#PlayerVoice", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, scheme()->GetProportionalScaledValue(VOICE_WIDTH) );
//	m_pPlayerList->AddColumnToSection(m_iSectionId, "tracker", "#PlayerTracker", SectionedListPanel::COLUMN_IMAGE, scheme()->GetProportionalScaledValue(FRIENDS_WIDTH) );
	*/

	// Make a blank section
	m_iSectionId = 0;
	m_pPlayerList->AddSection( m_iSectionId, "" );
	m_pPlayerList->SetSectionAlwaysVisible( m_iSectionId );
	m_pPlayerList->AddColumnToSection( m_iSectionId, "name", "", 0, scheme( )->GetProportionalScaledValue( NAME_WIDTH ) );

	++m_iSectionId;
	m_pPlayerList->AddSection( m_iSectionId, "" );
	m_pPlayerList->SetSectionAlwaysVisible( m_iSectionId );
	/*NAME_WIDTH = 160, SCORE_WIDTH = 60, DEATH_WIDTH = 60, PING_WIDTH = 80, VOICE_WIDTH = 0, FRIENDS_WIDTH = 0*/
	m_pPlayerList->AddColumnToSection( m_iSectionId, "name" , "#FF_PlayerName" , 0 , scheme( )->GetProportionalScaledValue( NAME_WIDTH ) );
	m_pPlayerList->AddColumnToSection( m_iSectionId, "class" , "#FF_PlayerClass" , 0 , scheme( )->GetProportionalScaledValue( CLASS_WIDTH ) );	// |-- Mirv: Current class
	m_pPlayerList->AddColumnToSection( m_iSectionId, "score" , "#FF_PlayerScore" , 0, scheme( )->GetProportionalScaledValue( SCORE_WIDTH ) );
	m_pPlayerList->AddColumnToSection( m_iSectionId, "deaths" , "#FF_PlayerDeath" , 0, scheme( )->GetProportionalScaledValue( DEATH_WIDTH ) );
	m_pPlayerList->AddColumnToSection( m_iSectionId, "ping" , "#FF_PlayerPing" , 0, scheme( )->GetProportionalScaledValue( PING_WIDTH ) );
	m_pPlayerList->AddColumnToSection( m_iSectionId, "voice" , "#FF_PlayerVoice" , SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, /*scheme( )->GetProportionalScaledValue(*/ VOICE_WIDTH /*)*/ );	// |-- Mirv: This should fix the messed up gfx settings
	m_pPlayerList->AddColumnToSection( m_iSectionId, "channel" , "#FF_PlayerChannel" , SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, /*scheme( )->GetProportionalScaledValue(*/ CHANNEL_WIDTH /*)*/ );	// |-- Mirv: This should fix the messed up gfx settings

	++m_iSectionId;
	m_iTeamSections[ TEAM_BLUE ]		= AddSection( TYPE_TEAM, TEAM_BLUE );

	++m_iSectionId;
	m_iTeamSections[ TEAM_RED ]			= AddSection( TYPE_TEAM, TEAM_RED );

	++m_iSectionId;
	m_iTeamSections[ TEAM_YELLOW ]		= AddSection( TYPE_TEAM, TEAM_YELLOW );

	++m_iSectionId;
	m_iTeamSections[ TEAM_GREEN ]		= AddSection( TYPE_TEAM, TEAM_GREEN );

	++m_iSectionId;
	m_iTeamSections[ TEAM_SPECTATOR ]	= AddSection( TYPE_SPECTATORS, TEAM_SPECTATOR );

	++m_iSectionId;
	m_iTeamSections[ TEAM_UNASSIGNED ]	= AddSection( TYPE_UNASSIGNED, TEAM_UNASSIGNED );
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new section to the scoreboard (i.e the team header)
//-----------------------------------------------------------------------------
int CClientScoreBoardDialog::AddSection( int teamType, int teamNumber )
{
	if( teamType == TYPE_TEAM )
	{
		IGameResources *pGR = GameResources( );
		if( !pGR )
			return -1;

		// Set up the team name
		wchar_t *szTeamName = localize( )->Find( pGR->GetTeamName( teamNumber ) );
		wchar_t szName[ 64 ];

		if( !szTeamName )
		{
			localize( )->ConvertANSIToUnicode( pGR->GetTeamName( teamNumber ), szName, sizeof( szName ) );
			szTeamName = szName;
		}

		m_pPlayerList->AddSection( m_iSectionId, "", StaticPlayerSortFunc );
		
		// --> Mirv: So we don't see teams unless there's somebody in them
		//m_pPlayerList->SetSectionAlwaysVisible( m_iSectionId );
		// <-- Mirv: So we don't see teams unless there's somebody in them

		m_pPlayerList->SetFgColor( pGR->GetTeamColor( teamNumber ) );
		
		m_pPlayerList->AddColumnToSection( m_iSectionId, "name", szTeamName, 0, scheme( )->GetProportionalScaledValue( NAME_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "class", "", 0, scheme( )->GetProportionalScaledValue( CLASS_WIDTH ) );	// |-- Mirv: Current class
		m_pPlayerList->AddColumnToSection( m_iSectionId, "score", "", 0, scheme( )->GetProportionalScaledValue( SCORE_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "deaths", "", 0, scheme( )->GetProportionalScaledValue( DEATH_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "ping", "", 0, scheme( )->GetProportionalScaledValue( PING_WIDTH ) );
		
		// --> Mirv: Voice and channel images
		m_pPlayerList->AddColumnToSection( m_iSectionId, "voice", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, VOICE_WIDTH );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "channel", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, CHANNEL_WIDTH );
		// <-- Mirv: Voice and channel images
	}
	else if( teamType == TYPE_SPECTATORS )
	{
		m_pPlayerList->AddSection( m_iSectionId, "" );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "name", "#Spectators", 0, scheme( )->GetProportionalScaledValue( NAME_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "class", "", 0, scheme( )->GetProportionalScaledValue( CLASS_WIDTH ) );	// |-- Mirv: Current class
		m_pPlayerList->AddColumnToSection( m_iSectionId, "score", "", 0, scheme( )->GetProportionalScaledValue( SCORE_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "deaths", "", 0, scheme( )->GetProportionalScaledValue( DEATH_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "ping", "", 0, scheme( )->GetProportionalScaledValue( PING_WIDTH ) );
	}
	else if( teamType == TYPE_UNASSIGNED )
	{
		m_pPlayerList->AddSection( m_iSectionId, "" );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "name", "#Unassigned", 0, scheme( )->GetProportionalScaledValue( NAME_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "class", "", 0, scheme( )->GetProportionalScaledValue( CLASS_WIDTH ) );	// |-- Mirv: Current class
		m_pPlayerList->AddColumnToSection( m_iSectionId, "score", "", 0, scheme( )->GetProportionalScaledValue( SCORE_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "deaths", "", 0, scheme( )->GetProportionalScaledValue( DEATH_WIDTH ) );
		m_pPlayerList->AddColumnToSection( m_iSectionId, "ping", "", 0, scheme( )->GetProportionalScaledValue( PING_WIDTH ) );
	}

	return m_iSectionId;
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CClientScoreBoardDialog::StaticPlayerSortFunc(vgui::SectionedListPanel *list, int itemID1, int itemID2)
{
	KeyValues *it1 = list->GetItemData(itemID1);
	KeyValues *it2 = list->GetItemData(itemID2);
	Assert(it1 && it2);

	// first compare frags
	int v1 = it1->GetInt("frags");
	int v2 = it2->GetInt("frags");
	if (v1 > v2)
		return true;
	else if (v1 < v2)
		return false;

	// next compare deaths
	v1 = it1->GetInt("deaths");
	v2 = it2->GetInt("deaths");
	if (v1 > v2)
		return false;
	else if (v1 < v2)
		return true;

	// the same, so compare itemID's (as a sentinel value to get deterministic sorts)
	return itemID1 < itemID2;
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new row to the scoreboard, from the playerinfo structure
//-----------------------------------------------------------------------------
bool CClientScoreBoardDialog::GetPlayerScoreInfo(int playerIndex, KeyValues *kv)
{
	IGameResources *gr = GameResources();

	if (!gr )
		return false;

	// BEG: Mulch
	bool bFriendly = false;

	C_FFPlayer *pLocalPlayer = ToFFPlayer( C_BasePlayer::GetLocalPlayer() );
	C_FFPlayer *pPlayer = NULL;

	// To stop one of those annoying ass c_ff_player.h asserts
	if( gr->IsConnected( playerIndex ) && gr->IsAlive( playerIndex ) )
		pPlayer = ToFFPlayer( UTIL_PlayerByIndex( playerIndex ) );

	if( pPlayer && pLocalPlayer )
	{
		//bFriendly = ( pPlayer->GetTeamNumber() == pLocalPlayer->GetTeamNumber() );
		
		// Check allies as well???
		//if( !bFriendly )
			bFriendly = ( FFGameRules()->PlayerRelationship( pLocalPlayer, pPlayer ) == GR_TEAMMATE );
	}
	// END: Mulch

	kv->SetInt( "deaths", gr->GetDeaths( playerIndex ) );
	//kv->SetInt("frags", gr->GetFrags( playerIndex ) );
	kv->SetInt( "score", gr->GetFrags( playerIndex ) );
	kv->SetInt( "ping", gr->GetPing( playerIndex ) ) ;
	kv->SetString( "name", gr->GetPlayerName( playerIndex ) );

	if( bFriendly )
		kv->SetString( "class", szClassName[gr->GetClass( playerIndex )] ); 	// |-- Mirv: Current class
	else
		kv->SetString( "class", " " );

	kv->SetInt( "playerIndex", playerIndex );

	// --> Mirv: Fixed for an extra setting
	// kv->SetInt( "voice", s_VoiceImage[ GetClientVoiceMgr( )->GetSpeakerStatus( playerIndex - 1 ) ] );	

	if( bFriendly )
	{
		kv->SetInt( "voice", s_VoiceImage[ GetClientVoiceMgr( )->GetSpeakerStatus( playerIndex ) ] + ( g_fBlockedStatus[playerIndex] ? 1 : 0 ) );	
		kv->SetInt( "channel", s_ChannelImage[ gr->GetChannel( playerIndex ) ] + 1 );
	}
	else
	{
		kv->SetInt( "voice", 0 );
		kv->SetInt( "channel", 0 );
	}

	// <-- Mirv: Fixed for an extra setting
/*	// setup the tracker column
	if (g_pFriendsUser)
	{
		unsigned int trackerID = gEngfuncs.GetTrackerIDForPlayer(row);

		if (g_pFriendsUser->IsBuddy(trackerID) && trackerID != g_pFriendsUser->GetFriendsID())
		{
			kv->SetInt("tracker",TrackerImage);
		}
	}
*/
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: reload the player list on the scoreboard
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::FillScoreBoard()
{
	/*
	// update totals information
	UpdateTeamInfo();

	// update player info
	UpdatePlayerInfo();
	//*/

	for( int i = TEAM_UNASSIGNED; i < TEAM_COUNT; i++ )
	{
		m_iNumPlayersOnTeam[ i ] = 0; //clear!
		//clear anything else for the team
	}

	UpdatePlayerInfo( );
	UpdateTeamInfo( );
} 

//-----------------------------------------------------------------------------
// Purpose: searches for the player in the scoreboard
//-----------------------------------------------------------------------------
int CClientScoreBoardDialog::FindItemIDForPlayerIndex( int playerIndex )
{
	for( int i = 0; i <= m_pPlayerList->GetHighestItemID( ); i++)
	{
		if( m_pPlayerList->IsItemIDValid( i ) )
		{
			KeyValues *kv = m_pPlayerList->GetItemData( i );
			kv = kv->FindKey( m_iPlayerIndexSymbol );
			if( kv && kv->GetInt( ) == playerIndex )
				return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::MoveLabelToFront(const char *textEntryName)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->MoveToFront();
	}
}

void CClientScoreBoardDialog::OnKeyCodePressed( KeyCode code )
{
	int lastPressedEngineKey = engine->GetLastPressedEngineKey( );

	if(( m_iJumpKey != -1 ) && ( m_iJumpKey == lastPressedEngineKey ))
	{
		SetMouseInputEnabled( true );
	}
	else
	{
		BaseClass::OnKeyCodePressed( code );
	}
}

int CClientScoreBoardDialog::FindPlayerIndexForItemID( int iItemID )
{
	if( m_pPlayerList->IsItemIDValid( iItemID ) ) 
	{
		KeyValues *kv = m_pPlayerList->GetItemData( iItemID );
		kv = kv->FindKey( m_iPlayerIndexSymbol );
		if( kv )
			return kv->GetInt( );


	}

    return -1;
}

void CClientScoreBoardDialog::OnItemSelected( KeyValues * data )
{  
	int iRowId = data->GetInt( "itemID" );
	int playerIndex = FindPlayerIndexForItemID( iRowId );

	//const char *szTemp = data->GetString( "name" );

	//DevMsg( "iRowID: %d, playerIndex: %d (%s)\n", iRowId, playerIndex, szTemp );

	if( playerIndex == -1 )
		return;

	// --> Mirv: New voice & text blocking code

	//if( GetClientVoiceMgr( )->GetSpeakerStatus( playerIndex ) != CVoiceStatus::VOICE_BANNED )

	// Interesting, doing this toggles it. so 
	//	GetClientVoiceMgr( )->SetPlayerBlockedState( playerIndex, true );
	//else 
	//	GetClientVoiceMgr( )->SetPlayerBlockedState( playerIndex, false );

	bool fVBlock = GetClientVoiceMgr( )->GetSpeakerStatus( playerIndex ) == CVoiceStatus::VOICE_BANNED;
	bool fTBlock = g_fBlockedStatus[playerIndex];

	//DevMsg( "VBlock: %s, TBlock: %s -- ", fVBlock ? "BLOCKED" : "fine", fTBlock ? "BLOCKED" : "fine" );

	// We have everything blocked, so go onto nothing blocked
	if( fVBlock && fTBlock )
	{
		//DevMsg( "Blocked: Nothing\n" );
		g_fBlockedStatus[playerIndex] = false;
		GetClientVoiceMgr()->SetPlayerBlockedState( playerIndex, false );
	}
	// We just have voice blocked, so go onto everything blocked
	else if( fVBlock )
	{
		//DevMsg( "Blocked: Everything\n" );
		g_fBlockedStatus[playerIndex] = true;
		GetClientVoiceMgr()->SetPlayerBlockedState( playerIndex, true );
	}
	// We have nothing blocked, so just block voice
	else
	{
		//DevMsg( "Blocked: Voice\n" );
		g_fBlockedStatus[playerIndex] = false;
		GetClientVoiceMgr()->SetPlayerBlockedState( playerIndex, true );
	}

	// Update right away to show voice icon change
	Update();

	// <-- Mirv: New voice & text blocking code

}