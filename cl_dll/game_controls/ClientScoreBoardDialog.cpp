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

// Returns -1 if pLHS < pRHS, 1 if pLHS > pRHS, and 0 is pLHS == pRHS
inline int ScoreboardSection_Sort( const ScoreboardSection_s *pLHS, const ScoreboardSection_s* pRHS )
{
	if( pLHS->m_iScore < pRHS->m_iScore )
		return 1;
	else if( pLHS->m_iScore > pRHS->m_iScore )
		return -1;
	else
	{
		// Scores are equal, sort by score time
		if( pLHS->m_flLastScored < pRHS->m_flLastScored )
			return 1;
		else if( pLHS->m_flLastScored > pRHS->m_flLastScored )
			return -1;
		else
		{
			// Score times are equal, sort by team
			if( pLHS->m_iTeam < pRHS->m_iTeam )
				return -1;
			else if( pLHS->m_iTeam > pRHS->m_iTeam )
				return 1;
		}
	}

	return 0;
}

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
void CClientScoreBoardDialog::Reset( void )
{
	// clear
	m_pPlayerList->RemoveAll();
	m_pPlayerList->RemoveAllSections();

	//m_iSectionId = 0;
	m_fNextUpdateTime = 0;
	// add all the sections
	InitScoreboardSections();
}

//-----------------------------------------------------------------------------
// Purpose: adds all the team sections to the scoreboard
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::InitScoreboardSections( void )
{
	// Do this initially
	m_hSections[ TEAM_BLUE ].m_iTeam = TEAM_BLUE;
	m_hSections[ TEAM_RED ].m_iTeam = TEAM_RED;
	m_hSections[ TEAM_YELLOW ].m_iTeam = TEAM_YELLOW;
	m_hSections[ TEAM_GREEN ].m_iTeam = TEAM_GREEN;

	AddHeader();
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
	// Update the scoreboard
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
// Purpose: Finds what section a team is in
//-----------------------------------------------------------------------------
int CClientScoreBoardDialog::FindSectionByTeam( int iTeam ) const
{
	for( int i = 0; i < 8; i++ )
	{
		if( m_hSections[ i ].m_iTeam == iTeam )
			return i;
	}

	return TEAM_UNASSIGNED;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::UpdatePlayerInfo( void )
{
	IGameResources *pGR = GameResources();
	if( !pGR )
		return;

	int iSelectedRow = -1;

	for( int i = 1; i < gpGlobals->maxClients; i++ )
	{
		if( pGR->IsConnected( i ) )
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
			int iSectionId = FindSectionByTeam( iPlayerTeam );

			if( pGR->IsLocalPlayer( i ) )
				iSelectedRow = iItemId;

			if( iItemId == -1 )
			{
				// Add a new row
				iItemId = m_pPlayerList->AddItem( iSectionId, pPlayerData );
			}
			else
			{
				// Modify current row
				m_pPlayerList->ModifyItem( iItemId, iSectionId, pPlayerData );
			}

			// Set the row color based on players team
			m_pPlayerList->SetItemFgColor( iItemId, pGR->GetTeamColor( iPlayerTeam ) );

			pPlayerData->deleteThis();
		}
		else
		{
			// Remove the player
			int iItemId = FindItemIDForPlayerIndex( i );

			if( iItemId != -1 )
				m_pPlayerList->RemoveItem( iItemId );
		}
	}

	if( iSelectedRow != -1 )
		m_pPlayerList->SetSelectedItem( iSelectedRow );
}

//-----------------------------------------------------------------------------
// Purpose: adds the top header of the scoreboars
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::AddHeader( void )
{
	// We can get called back into here when teams
	// get sorted so that's why part is commented
	// out as the team values are already set by
	// the sort (or by the initial creation of the
	// scoreboard)

	int iSection = 0;	
	m_hSections[ iSection ].m_iTeam = AddSection( TYPE_BLANK, iSection );		// 0

	iSection++;
	m_hSections[ iSection ].m_iTeam = AddSection( TYPE_HEADER, iSection );		// 1

	iSection++;
	/*m_hSections[ iSection ].m_iTeam =*/ AddSection( TYPE_TEAM, iSection );		// 2

	iSection++;
	/*m_hSections[ iSection ].m_iTeam =*/ AddSection( TYPE_TEAM, iSection );		// 3

	iSection++;
	/*m_hSections[ iSection ].m_iTeam =*/ AddSection( TYPE_TEAM, iSection );		// 4

	iSection++;
	/*m_hSections[ iSection ].m_iTeam =*/ AddSection( TYPE_TEAM, iSection );		// 5

	iSection++;
	m_hSections[ iSection ].m_iTeam = AddSection( TYPE_SPECTATORS, iSection );	// 6

	iSection++;
	m_hSections[ iSection ].m_iTeam = AddSection( TYPE_UNASSIGNED, iSection );	// 7
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new section to the scoreboard (i.e the team header)
//-----------------------------------------------------------------------------
int CClientScoreBoardDialog::AddSection( int iType, int iSection )
{
	int iRetval = -1;

	if( iType == TYPE_BLANK )
	{
		m_pPlayerList->AddSection( iSection, "" );
		m_pPlayerList->SetSectionAlwaysVisible( iSection );
		m_pPlayerList->AddColumnToSection( iSection, "name", "", 0, scheme()->GetProportionalScaledValue( NAME_WIDTH ) );
	}
	else if( iType == TYPE_HEADER )
	{
		m_pPlayerList->AddSection( iSection, "" );
		m_pPlayerList->SetSectionAlwaysVisible( iSection );
        m_pPlayerList->AddColumnToSection( iSection, "name" , "#FF_PlayerName" , 0 , scheme()->GetProportionalScaledValue( NAME_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "class" , "#FF_PlayerClass" , 0 , scheme()->GetProportionalScaledValue( CLASS_WIDTH ) );	// |-- Mirv: Current class
		m_pPlayerList->AddColumnToSection( iSection, "score" , "#FF_PlayerScore" , 0, scheme()->GetProportionalScaledValue( SCORE_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "deaths" , "#FF_PlayerDeath" , 0, scheme()->GetProportionalScaledValue( DEATH_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "ping" , "#FF_PlayerPing" , 0, scheme()->GetProportionalScaledValue( PING_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "voice" , "#FF_PlayerVoice" , SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, /*scheme( )->GetProportionalScaledValue(*/ VOICE_WIDTH /*)*/ );	// |-- Mirv: This should fix the messed up gfx settings
		m_pPlayerList->AddColumnToSection( iSection, "channel" , "#FF_PlayerChannel" , SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, /*scheme( )->GetProportionalScaledValue(*/ CHANNEL_WIDTH /*)*/ );	// |-- Mirv: This should fix the messed up gfx settings
	}
	else if( iType == TYPE_TEAM )
	{
		m_pPlayerList->AddSection( iSection, "", StaticPlayerSortFunc_Score );
		//m_pPlayerList->SetSectionAlwaysVisible( iSection );
		m_pPlayerList->AddColumnToSection( iSection, "name", "#FF_Team", 0, scheme()->GetProportionalScaledValue( NAME_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "class", "", 0, scheme()->GetProportionalScaledValue( CLASS_WIDTH ) );	// |-- Mirv: Current class
		m_pPlayerList->AddColumnToSection( iSection, "score", "", 0, scheme()->GetProportionalScaledValue( SCORE_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "deaths", "", 0, scheme()->GetProportionalScaledValue( DEATH_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "ping", "", 0, scheme()->GetProportionalScaledValue( PING_WIDTH ) );

		// --> Mirv: Voice and channel images
		m_pPlayerList->AddColumnToSection( iSection, "voice", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, VOICE_WIDTH );
		m_pPlayerList->AddColumnToSection( iSection, "channel", "", SectionedListPanel::COLUMN_IMAGE | SectionedListPanel::COLUMN_CENTER, CHANNEL_WIDTH );
		// <-- Mirv: Voice and channel images

		iRetval = iSection;
	}
	else if( iType == TYPE_SPECTATORS )
	{
		m_pPlayerList->AddSection( iSection, "", StaticPlayerSortFunc_Name );
		//m_pPlayerList->SetSectionAlwaysVisible( iSection );
		m_pPlayerList->AddColumnToSection( iSection, "name", "#FF_Spectators", 0, scheme()->GetProportionalScaledValue( NAME_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "class", "", 0, scheme()->GetProportionalScaledValue( CLASS_WIDTH ) );	// |-- Mirv: Current class
		m_pPlayerList->AddColumnToSection( iSection, "score", "", 0, scheme()->GetProportionalScaledValue( SCORE_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "deaths", "", 0, scheme()->GetProportionalScaledValue( DEATH_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "ping", "", 0, scheme()->GetProportionalScaledValue( PING_WIDTH ) );

		iRetval = TEAM_SPECTATOR;
	}
	else if( iType == TYPE_UNASSIGNED )
	{
		m_pPlayerList->AddSection( iSection, "" );
		//m_pPlayerList->SetSectionAlwaysVisible( iSection );
		m_pPlayerList->AddColumnToSection( iSection, "name", "#FF_Unassigned", 0, scheme()->GetProportionalScaledValue( NAME_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "class", "", 0, scheme()->GetProportionalScaledValue( CLASS_WIDTH ) );	// |-- Mirv: Current class
		m_pPlayerList->AddColumnToSection( iSection, "score", "", 0, scheme()->GetProportionalScaledValue( SCORE_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "deaths", "", 0, scheme()->GetProportionalScaledValue( DEATH_WIDTH ) );
		m_pPlayerList->AddColumnToSection( iSection, "ping", "", 0, scheme()->GetProportionalScaledValue( PING_WIDTH ) );

		iRetval = TEAM_UNASSIGNED;
	}

	return iRetval;
}

//-----------------------------------------------------------------------------
// Purpose: Update team sections to correct teams & colors
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::UpdateHeaders( void )
{
	// Go from m_hSections[ 2 -> 6 ] (ie. the team's that can be sorted)
	// Assumes m_hSections has been sorted properly before doing this

	IGameResources *pGR = GameResources();
	if( !pGR )
		return;
	
	for( int i = TEAM_BLUE; i <= TEAM_GREEN; i++ )
	{
		int iTeam = m_hSections[ i ].m_iTeam;
		int iNumPlayers = m_hSections[ i ].m_iNumPlayers;
		int iLatency = m_hSections[ i ].m_iLatency;

		// This is nice and all but if teams get sorted while
		// you're viewing the scoreboard the color doesn't change :/
		m_pPlayerList->SetSectionFgColor( i, pGR->GetTeamColor( iTeam ) );

		wchar_t *szTeamName = localize()->Find( pGR->GetTeamName( iTeam ) );
		wchar_t	szName[ 256 ];

		if( !szTeamName )
		{
			// No localized text or team name not a resource string
			char szString[ 256 ];
			Q_snprintf( szString, 256, "%s - (%i players)", pGR->GetTeamName( iTeam ), iNumPlayers );
			localize()->ConvertANSIToUnicode( szString, szName, sizeof( szName ) );
			szTeamName = szName;
		}
		else
		{
			swprintf( szName, L"%s - (%i players)", szTeamName, iNumPlayers );
			szTeamName = szName;
		}

		m_pPlayerList->ModifyColumn( i, "name", szTeamName );

		// Loop up team score
		wchar_t szScore[ 6 ];
		swprintf( szScore, L"%d", pGR->GetTeamScore( iTeam ) );

		// Display team score
		m_pPlayerList->ModifyColumn( i, "score", szScore );

		// Look up team deaths
		wchar_t szDeaths[ 6 ];
		swprintf( szDeaths, L"%d", pGR->GetTeamDeaths( iTeam ) );

		// Display team deaths
		m_pPlayerList->ModifyColumn( i, "deaths", szDeaths );

		// Set up team latency
		if( iNumPlayers > 0 )
			iLatency /= iNumPlayers;
		else
			iLatency = 0;

		// Display team latency
		if( iLatency < 1 )
			m_pPlayerList->ModifyColumn( i, "ping", L"0" );
		else
		{
			wchar_t szLatency[ 12 ];
			swprintf( szLatency, L"%i", iLatency );
			m_pPlayerList->ModifyColumn( i, "ping", szLatency );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CClientScoreBoardDialog::StaticPlayerSortFunc_Score( vgui::SectionedListPanel *list, int itemID1, int itemID2 )
{
	KeyValues *it1 = list->GetItemData( itemID1 );
	KeyValues *it2 = list->GetItemData( itemID2 );
	Assert( it1 && it2 );

	// first compare frags
	int v1 = it1->GetInt( "score" );
	int v2 = it2->GetInt( "score" );
	if( v1 > v2 )
		return true;
	else if( v1 < v2 )
		return false;

	// next compare deaths
	v1 = it1->GetInt( "deaths" );
	v2 = it2->GetInt( "deaths" );
	if( v1 > v2 )
		return false;
	else if( v1 < v2 )
		return true;

	// the same, so compare itemID's (as a sentinel value to get deterministic sorts)
	return itemID1 < itemID2;
}

//-----------------------------------------------------------------------------
// Purpose: Used for sorting players
//-----------------------------------------------------------------------------
bool CClientScoreBoardDialog::StaticPlayerSortFunc_Name( vgui::SectionedListPanel *list, int itemID1, int itemID2 )
{
	KeyValues *it1 = list->GetItemData( itemID1 );
	KeyValues *it2 = list->GetItemData( itemID2 );
	Assert( it1 && it2 );

	// first compare frags
	const char *p1 = it1->GetString( "name" );
	const char *p2 = it2->GetString( "name" );
	if( Q_stricmp( p1, p2 ) < 0 )	
		return true;
	else if( Q_stricmp( p1, p2 ) > 0 )
		return false;

	// the same, so compare itemID's (as a sentinel value to get deterministic sorts)
	return itemID1 < itemID2;
}

//-----------------------------------------------------------------------------
// Purpose: Adds a new row to the scoreboard, from the playerinfo structure
//-----------------------------------------------------------------------------
bool CClientScoreBoardDialog::GetPlayerScoreInfo( int playerIndex, KeyValues *kv )
{
	IGameResources *pGR = GameResources();
	if( !pGR )
		return false;

	bool bFriendly = false;

	C_FFPlayer *pLocalPlayer = NULL;
	if( C_BasePlayer::GetLocalPlayer() )
		pLocalPlayer = ToFFPlayer( C_BasePlayer::GetLocalPlayer() );

	C_FFPlayer *pPlayer = NULL;

	// To stop one of those annoying ass c_ff_player.h asserts
	// UNDONE: This meant that we couldn't see player class details when they were
	// dead. Need to find a different solution to the asserts if they come back
	if( pGR->IsConnected( playerIndex ) /*&& pGR->IsAlive( playerIndex )*/ )
	{
		CBasePlayer *pTemp = UTIL_PlayerByIndex( playerIndex );
		if( pTemp && pTemp->IsPlayer() )
            pPlayer = ToFFPlayer( UTIL_PlayerByIndex( playerIndex ) );
	}

	if( pPlayer && pLocalPlayer )
		bFriendly = ( FFGameRules()->PlayerRelationship( pLocalPlayer, pPlayer ) == GR_TEAMMATE );

	kv->SetInt( "deaths", pGR->GetDeaths( playerIndex ) );
	kv->SetInt( "score", pGR->GetFrags( playerIndex ) );
	kv->SetInt( "ping", pGR->GetPing( playerIndex ) ) ;
	kv->SetString( "name", pGR->GetPlayerName( playerIndex ) );

	if( bFriendly )
		kv->SetString( "class", szClassName[ pGR->GetClass( playerIndex ) ] ); 	// |-- Mirv: Current class
	else
		kv->SetString( "class", "" );

	kv->SetInt( "playerIndex", playerIndex );

	// --> Mirv: Fixed for an extra setting
	// kv->SetInt( "voice", s_VoiceImage[ GetClientVoiceMgr()->GetSpeakerStatus( playerIndex - 1 ) ] );	

	if( bFriendly )
	{
		kv->SetInt( "voice", s_VoiceImage[ GetClientVoiceMgr()->GetSpeakerStatus( playerIndex ) ] + ( g_fBlockedStatus[ playerIndex ] ? 1 : 0 ) );	
		kv->SetInt( "channel", s_ChannelImage[ pGR->GetChannel( playerIndex ) ] + 1 );
	}
	else
	{
		kv->SetInt( "voice", 0 );
		kv->SetInt( "channel", 0 );
	}

	// <-- Mirv: Fixed for an extra setting

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: reload the player list on the scoreboard
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::FillScoreBoard( void )
{
	IGameResources *pGR = GameResources();
	if( !pGR )
		return;

	bool bNeedToSort = NeedToSortTeams();

	// Set up num players, latency, score, and time scored for each team
	for( int i = TEAM_BLUE; i <= TEAM_GREEN; i++ )
	{
		int iTeam = m_hSections[ i ].m_iTeam;

		// Get team score
		m_hSections[ i ].m_iScore = pGR->GetTeamScore( iTeam );
		// Get the time the team last scored
		m_hSections[ i ].m_flLastScored = pGR->GetTeamScoreTime( iTeam );

		int iPlayerCount = 0, iLatency = 0;

		for( int j = 1; j < gpGlobals->maxClients; j++ )
		{
			if( pGR->IsConnected( j ) )
			{
				// If the player is on iTeam
				if( pGR->GetTeam( j ) == iTeam )
				{
					iPlayerCount++;
					iLatency += pGR->GetPing( j );
				}
			}
		}

		// Set number of players on this team
		m_hSections[ i ].m_iNumPlayers = iPlayerCount;
		// Set the team's latency
		m_hSections[ i ].m_iLatency = iLatency;
	}

	// Do sorting
	if( bNeedToSort )
	{
		// YES, THIS IS VERY CHEESY!
		CUtlVector< ScoreboardSection_s > hTemp;
		
		// Copy to vector...
		for( int i = TEAM_BLUE; i <= TEAM_GREEN; i++ )
			hTemp.AddToTail( m_hSections[ i ] );

		// Sort!
		hTemp.Sort( ScoreboardSection_Sort );

		// Copy back over...
		for( int i = TEAM_BLUE; i <= TEAM_GREEN; i++ )
			m_hSections[ i ] = hTemp[ i - TEAM_BLUE ];

		// Have to remove all sections because if you're
		// viewing the scoreboard when teams get sorted
		// the team header doesn't change color until
		// you hide the scoreboard then bring it back up
		m_pPlayerList->RemoveAll();
		m_pPlayerList->RemoveAllSections();		

		// Re-add all headers and sections
		AddHeader();		
	}
	
	// Update team headers
	UpdateHeaders();

	// Update player info
	UpdatePlayerInfo();	
} 

//-----------------------------------------------------------------------------
// Purpose: checks to see if we need to sort the teams
//-----------------------------------------------------------------------------
bool CClientScoreBoardDialog::NeedToSortTeams( void ) const
{
	bool bSort = false;

	int iLastScore = m_hSections[ TEAM_BLUE ].m_iScore;
	float flLastUpdate = m_hSections[ TEAM_BLUE ].m_flLastScored;

	for( int i = TEAM_RED; ( i <= TEAM_GREEN ) && !bSort; i++ )
	{
		if( iLastScore < m_hSections[ i ].m_iScore )
			bSort = true;
		else if( iLastScore == m_hSections [ i ].m_iScore )
		{
			if( flLastUpdate < m_hSections[ i ].m_flLastScored )
				bSort = true;
		}

		// Update 'last'
		iLastScore = m_hSections[ i ].m_iScore;
		flLastUpdate = m_hSections[ i ].m_flLastScored;
	}

	return bSort;
}

int CClientScoreBoardDialog::FindItemIDForPlayerIndex( int playerIndex )
{
	for( int i = 0; i <= m_pPlayerList->GetHighestItemID(); i++)
	{
		if( m_pPlayerList->IsItemIDValid( i ) )
		{
			KeyValues *kv = m_pPlayerList->GetItemData( i );
			kv = kv->FindKey( m_iPlayerIndexSymbol );
			if( kv && kv->GetInt() == playerIndex )
				return i;
		}
	}

	return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::MoveLabelToFront( const char *textEntryName )
{
	Label *entry = dynamic_cast< Label * >( FindChildByName( textEntryName ) );
	if( entry )
	{
		entry->MoveToFront();
	}
}

void CClientScoreBoardDialog::OnKeyCodePressed( KeyCode code )
{
	int lastPressedEngineKey = engine->GetLastPressedEngineKey( );

	if( ( m_iJumpKey != -1 ) && ( m_iJumpKey == lastPressedEngineKey ) )
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
			return kv->GetInt();
	}

    return -1;
}

//-----------------------------------------------------------------------------
// Purpose: Scroll between different block modes
//-----------------------------------------------------------------------------
void CClientScoreBoardDialog::OnItemSelected(KeyValues *data)
{  
	int iRowId = data->GetInt("itemID");
	int playerIndex = FindPlayerIndexForItemID(iRowId);

	IGameResources *pGR = GameResources();

	// Don't change local player info
	if (pGR && pGR->IsLocalPlayer(playerIndex))
		return;

	// If player is not audible, only toggle text block
	if (!GetClientVoiceMgr()->IsPlayerAudible(playerIndex))
	{
		g_fBlockedStatus[playerIndex] = !g_fBlockedStatus[playerIndex];
		return;
	}

	// Get some current states
	bool fVBlock = GetClientVoiceMgr()->GetSpeakerStatus(playerIndex) == CVoiceStatus::VOICE_BANNED;
	bool fTBlock = g_fBlockedStatus[playerIndex];

	// We have everything blocked, so go onto nothing blocked
	if (fVBlock && fTBlock)
	{
		g_fBlockedStatus[playerIndex] = false;
		GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, false);
	}
	// We just have voice blocked, so go onto everything blocked
	else if (fVBlock)
	{
		g_fBlockedStatus[playerIndex] = true;
		GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, true);
	}
	// We have nothing blocked, so just block voice
	else
	{
		g_fBlockedStatus[playerIndex] = false;
		GetClientVoiceMgr()->SetPlayerBlockedState(playerIndex, true);
	}

	// Update right away to show voice icon change
	Update();
}
