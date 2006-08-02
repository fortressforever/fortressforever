//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include <cdll_client_int.h>
#include <globalvars_base.h>
#include <cdll_util.h>
#include <KeyValues.h>

#include "spectatorgui.h"

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <vgui/IPanel.h>
#include <vgui_controls/ImageList.h>
#include <vgui_controls/MenuItem.h>

#include <stdio.h> // _snprintf define

#include <cl_dll/iviewport.h>
#include "commandmenu.h"
#include "hltvcamera.h"

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Menu.h>
#include "IGameUIFuncs.h" // for key bindings
#include <imapoverview.h>
#include <shareddefs.h>
#include <igameresources.h>
#include "c_ff_team.h"
#include "ff_gamerules.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IGameUIFuncs *gameuifuncs; // for key binding details

// void DuckMessage(const char *str); // from vgui_teamfortressviewport.cpp

ConVar spec_scoreboard( "spec_scoreboard", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );

CSpectatorGUI *g_pSpectatorGUI = NULL;

static char *s_SpectatorModes[] = { "#Spec_Mode0", "#Spec_Mode1", "#Spec_Mode2", "#Spec_Mode3", "#Spec_Mode4", "#Spec_Mode5", "" };

using namespace vgui;

ConVar cl_spec_mode(
	"cl_spec_mode",
	"1",
	FCVAR_ARCHIVE | FCVAR_USERINFO,
	"spectator mode" );

//-----------------------------------------------------------------------------
// Purpose: left and right buttons pointing buttons
//-----------------------------------------------------------------------------
class CSpecButton : public Button
{
public:
	CSpecButton(Panel *parent, const char *panelName): Button(parent, panelName, "") {}

private:
	void ApplySchemeSettings(vgui::IScheme *pScheme)
	{
		Button::ApplySchemeSettings(pScheme);
		SetFont(pScheme->GetFont("Marlett", IsProportional()) );
	}
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSpectatorMenu::CSpectatorMenu( IViewPort *pViewPort ) : Frame( NULL, PANEL_SPECMENU )
{
	m_iDuckKey = -1;
		
	m_pViewPort = pViewPort;

	SetMouseInputEnabled( true );
	SetKeyBoardInputEnabled( true );
	SetTitleBarVisible( false ); // don't draw a title bar
	SetMoveable( false );
	SetSizeable( false );
	SetProportional(true);

	SetScheme("ClientScheme");

	m_pPlayerList = new ComboBox(this, "playercombo", 10 , false);
	m_pViewOptions = new ComboBox(this, "viewcombo", 10 , false );
	m_pConfigSettings = new ComboBox(this, "settingscombo", 10 , false );	

	m_pLeftButton = new CSpecButton( this, "specprev");
	m_pLeftButton->SetText("3");
	m_pRightButton = new CSpecButton( this, "specnext");
	m_pRightButton->SetText("4");

	m_pPlayerList->SetText("");
	m_pViewOptions->SetText("#Spec_Modes");
	m_pConfigSettings->SetText("#Spec_Options");

	m_pPlayerList->SetOpenDirection( ComboBox::UP );
	m_pViewOptions->SetOpenDirection( ComboBox::UP );
	m_pConfigSettings->SetOpenDirection( ComboBox::UP );

	// create view config menu
	CommandMenu * menu = new CommandMenu(m_pViewOptions, "spectatormenu", gViewPortInterface);
	menu->LoadFromFile( "Resource/spectatormenu.res" );
	m_pConfigSettings->SetMenu( menu );	// attach menu to combo box

	// create view mode menu
	menu = new CommandMenu(m_pViewOptions, "spectatormodes", gViewPortInterface);
	menu->LoadFromFile("Resource/spectatormodes.res");
	m_pViewOptions->SetMenu( menu );	// attach menu to combo box

	LoadControlSettings("Resource/UI/BottomSpectator.res");
}

void CSpectatorMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	// need to MakeReadyForUse() on the menus so we can set their bg color before they are displayed
	m_pConfigSettings->GetMenu()->MakeReadyForUse();
	m_pViewOptions->GetMenu()->MakeReadyForUse();
	m_pPlayerList->GetMenu()->MakeReadyForUse();

	m_pConfigSettings->GetMenu()->SetBgColor( BLACK_BAR_COLOR );
	m_pViewOptions->GetMenu()->SetBgColor( BLACK_BAR_COLOR );
	m_pPlayerList->GetMenu()->SetBgColor( BLACK_BAR_COLOR );
}

//-----------------------------------------------------------------------------
// Purpose: makes the GUI fill the screen
//-----------------------------------------------------------------------------
void CSpectatorMenu::PerformLayout()
{
	int w,h;
	surface()->GetScreenSize(w, h);

	// fill the screen
	SetSize(w,GetTall());
}


//-----------------------------------------------------------------------------
// Purpose: Handles changes to combo boxes
//-----------------------------------------------------------------------------
void CSpectatorMenu::OnTextChanged(KeyValues *data)
{
	Panel *panel = reinterpret_cast<vgui::Panel *>( data->GetPtr("panel") );

	vgui::ComboBox *box = dynamic_cast<vgui::ComboBox *>( panel );

	if( box == m_pConfigSettings) // don't change the text in the config setting combo
	{
		m_pConfigSettings->SetText("#Spec_Options");
	}
	else if ( box == m_pPlayerList )
	{
		KeyValues *kv = box->GetActiveItemUserData();
		if ( kv && GameResources() )
		{
			const char *player = kv->GetString("player");

			int currentPlayerNum = GetSpectatorTarget();
			const char *currentPlayerName = GameResources()->GetPlayerName( currentPlayerNum );

			if ( !FStrEq( currentPlayerName, player ) )
			{
				char command[128];
				Q_snprintf( command, sizeof(command), "spec_player \"%s\"", player );
				engine->ClientCmd( command );
			}
		}
	}
}

void CSpectatorMenu::OnCommand( const char *command )
{
	if (!stricmp(command, "specnext") )
	{
		engine->ClientCmd("spec_next");
	}
	else if (!stricmp(command, "specprev") )
	{
		engine->ClientCmd("spec_prev");
	}
}


//-----------------------------------------------------------------------------
// Purpose: when duck is pressed it hides the active part of the GUI
//-----------------------------------------------------------------------------
void CSpectatorMenu::OnKeyCodePressed(KeyCode code)
{
	// we can't compare the keycode to a known code, because translation from bound keys
	// to vgui key codes is not 1:1. Get the engine version of the key for the binding
	// and the actual pressed key, and compare those..
	int iLastTrappedKey = engine->GetLastPressedEngineKey();	// the enginekey version of the code param

	if( iLastTrappedKey == m_iDuckKey )
	{
		// hide if DUCK is pressed again
		m_pViewPort->ShowPanel( this, false );
	}
}

void CSpectatorMenu::ShowPanel(bool bShow)
{
	if ( BaseClass::IsVisible() == bShow )
		return;

	if ( bShow )
	{
		Activate();
		SetMouseInputEnabled( true );
		SetKeyBoardInputEnabled( true );
	}
	else
	{
		SetVisible( false );
		SetMouseInputEnabled( false );
		SetKeyBoardInputEnabled( false );
	}

	// during HLTV live broadcast, some interactive elements are disabled
	bool bIsEnabled = !engine->IsHLTV() || engine->IsPlayingDemo();
	
	m_pLeftButton->SetVisible( bIsEnabled );
	m_pRightButton->SetVisible( bIsEnabled );
	m_pPlayerList->SetVisible( bIsEnabled );
	m_pViewOptions->SetVisible( bIsEnabled );
}


int CSpectatorMenu::PlayerAddItem( int itemID, wchar_t *name, KeyValues *data ) 
{
	if ( m_pPlayerList->IsItemIDValid( itemID ) )
	{	
		m_pPlayerList->UpdateItem( itemID, name, data );
		return itemID + 1;
	}
	else
	{
		return m_pPlayerList->AddItem( name, data ) + 1; 
	}
}

void CSpectatorMenu::SetPlayerNameText(const wchar_t *text )
{
	char *ansiText = (char *) _alloca( (wcslen( text ) + 1) * sizeof( char ) );
	if ( ansiText )
	{
		localize()->ConvertUnicodeToANSI( text, ansiText, (wcslen( text ) + 1) * sizeof( char ) );
		m_pPlayerList->SetText( ansiText );
	}
}

//-----------------------------------------------------------------------------
// main spectator panel



//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CSpectatorGUI::CSpectatorGUI(IViewPort *pViewPort) : Frame( NULL, PANEL_SPECGUI )
{
// 	m_bHelpShown = false;
//	m_bInsetVisible = false;
//	m_iDuckKey = KEY_NONE;
	m_bSpecScoreboard = false;

	m_pViewPort = pViewPort;
	g_pSpectatorGUI = this;

	// initialize dialog
	SetVisible(false);
	SetTitle("SpectatorGUI", true);
	SetProportional(true);

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetMouseInputEnabled( false );
	SetKeyBoardInputEnabled( false );

	// hide the system buttons
	SetTitleBarVisible( false );

	m_pTopBar = new Panel( this, "topbar" );
 	m_pBottomBarBlank = new Panel( this, "bottombarblank" );

	// m_pBannerImage = new ImagePanel( m_pTopBar, NULL );
	m_pPlayerLabel = new Label( this, "playerlabel", "" );
	m_pPlayerLabel->SetVisible( false );

	LoadControlSettings("Resource/UI/Spectator.res");
	
	SetPaintBorderEnabled(false);
	SetPaintBackgroundEnabled(false);

	m_pBottomBarBlank->SetVisible( true );
	m_pTopBar->SetVisible( true );

	// m_pBannerImage->SetVisible(false);
	InvalidateLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CSpectatorGUI::~CSpectatorGUI()
{
	g_pSpectatorGUI = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Sets the colour of the top and bottom bars
//-----------------------------------------------------------------------------
void CSpectatorGUI::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings( pScheme );
	SetBgColor(Color( 0,0,0,0 ) ); // make the background transparent
	m_pTopBar->SetBgColor(BLACK_BAR_COLOR);
	m_pBottomBarBlank->SetBgColor(BLACK_BAR_COLOR);
	// m_pBottomBar->SetBgColor(Color( 0,0,0,0 ));
	SetPaintBorderEnabled(false);

	SetBorder( NULL );

}

//-----------------------------------------------------------------------------
// Purpose: makes the GUI fill the screen
//-----------------------------------------------------------------------------
void CSpectatorGUI::PerformLayout()
{
	int w,h;
	surface()->GetScreenSize(w, h);

	// fill the screen
	SetBounds(0,0,w,h);

	// stretch the bottom bar across the screen
	m_pBottomBarBlank->SetWide( w );
}

//-----------------------------------------------------------------------------
// Purpose: checks spec_scoreboard cvar to see if the scoreboard should be displayed
//-----------------------------------------------------------------------------
void CSpectatorGUI::OnThink()
{
	BaseClass::OnThink();

	if ( IsVisible() )
	{
		if ( m_bSpecScoreboard != spec_scoreboard.GetBool() )
		{
			if ( !spec_scoreboard.GetBool() || !gViewPortInterface->GetActivePanel() )
			{
				m_bSpecScoreboard = spec_scoreboard.GetBool();
				gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, m_bSpecScoreboard );
			}
		}

		// Update the timer
		UpdateTimer();

		// Update scores
		UpdateScores();
	}
}

//-----------------------------------------------------------------------------
// Purpose: sets the image to display for the banner in the top right corner
//-----------------------------------------------------------------------------
void CSpectatorGUI::SetLogoImage(const char *image)
{
	if ( m_pBannerImage )
	{
		m_pBannerImage->SetImage( scheme()->GetImage(image, false) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CSpectatorGUI::SetLabelText(const char *textEntryName, const char *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetText(text);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CSpectatorGUI::SetLabelText(const char *textEntryName, wchar_t *text)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->SetText(text);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets the text of a control by name
//-----------------------------------------------------------------------------
void CSpectatorGUI::MoveLabelToFront(const char *textEntryName)
{
	Label *entry = dynamic_cast<Label *>(FindChildByName(textEntryName));
	if (entry)
	{
		entry->MoveToFront();
	}
}

//-----------------------------------------------------------------------------
// Purpose: shows/hides the buy menu
//-----------------------------------------------------------------------------
void CSpectatorGUI::ShowPanel(bool bShow)
{
	if ( bShow && !IsVisible() )
	{
		m_bSpecScoreboard = false;
	}
	SetVisible( bShow );
	if ( !bShow && m_bSpecScoreboard )
	{
		gViewPortInterface->ShowPanel( PANEL_SCOREBOARD, false );
	}
}

bool CSpectatorGUI::ShouldShowPlayerLabel( int specmode )
{
	return ( (specmode == OBS_MODE_IN_EYE) ||	(specmode == OBS_MODE_CHASE) );
}
//-----------------------------------------------------------------------------
// Purpose: Updates the gui, rearranges elements
//-----------------------------------------------------------------------------
void CSpectatorGUI::Update()
{
	int wide, tall;
	int bx, by, bwide, btall;

	surface()->GetScreenSize(wide, tall);
	m_pTopBar->GetBounds( bx, by, bwide, btall );

	IGameResources *gr = GameResources();
	int specmode = GetSpectatorMode();
	int playernum = GetSpectatorTarget();

	IViewPortPanel *overview = gViewPortInterface->FindPanelByName( PANEL_OVERVIEW );

	if ( overview && overview->IsVisible() )
	{
		int mx, my, mwide, mtall;

		VPANEL p = overview->GetVPanel();
		vgui::ipanel()->GetPos( p, mx, my );
		vgui::ipanel()->GetSize( p, mwide, mtall );
				
		if ( my < btall )
		{
			// reduce to bar 
			m_pTopBar->SetSize( wide - (mx + mwide), btall );
			m_pTopBar->SetPos( (mx + mwide), 0 );
		}
		else
		{
			// full top bar
			m_pTopBar->SetSize( wide , btall );
			m_pTopBar->SetPos( 0, 0 );
		}
	}
	else
	{
		// full top bar
		m_pTopBar->SetSize( wide , btall ); // change width, keep height
		m_pTopBar->SetPos( 0, 0 );
	}

	m_pPlayerLabel->SetVisible( ShouldShowPlayerLabel(specmode) );

	// update player name filed, text & color

	if ( playernum > 0 && gr)
	{
		Color c = gr->GetTeamColor( gr->GetTeam(playernum) ); // Player's team color

		m_pPlayerLabel->SetFgColor( c );
		
		wchar_t playerText[ 80 ], playerName[ 64 ], health[ 10 ];
		wcscpy( playerText, L"Unable to find #Spec_PlayerItem*" );
		memset( playerName, 0x0, sizeof( playerName ) * sizeof( wchar_t ) );

		localize()->ConvertANSIToUnicode( gr->GetPlayerName( playernum ), playerName, sizeof( playerName ) );
		int iHealth = gr->GetHealth( playernum );
		if ( iHealth > 0  && gr->IsAlive(playernum) )
		{
			_snwprintf( health, sizeof( health ), L"%i", iHealth );
			localize()->ConstructString( playerText, sizeof( playerText ), localize()->Find( "#Spec_PlayerItem_Team" ), 2, playerName,  health );
		}
		else
		{
			localize()->ConstructString( playerText, sizeof( playerText ), localize()->Find( "#Spec_PlayerItem" ), 1, playerName );
		}

		m_pPlayerLabel->SetText( playerText );
	}

	// update extra info field
	wchar_t string1[1024];
/*	if ( gViewPortInterface->GetClientDllInterface()->IsHLTVMode() ) TODO
	{
		char numplayers[6];
		wchar_t wNumPlayers[6];

		Q_snprintf(numplayers,6,"%i", 666); // TODO show HLTV spectator number
		localize()->ConvertANSIToUnicode(numplayers,wNumPlayers,sizeof(wNumPlayers));
		localize()->ConstructString( string1,sizeof( string1 ), localize()->Find("#Spectators" ),1, wNumPlayers );
	}
	else */
	{
		// otherwise show map name
		char mapname[255];
		Q_FileBase( engine->GetLevelName(), mapname, sizeof(mapname) );

		wchar_t wMapName[64];
		localize()->ConvertANSIToUnicode(mapname,wMapName,sizeof(wMapName));
		localize()->ConstructString( string1,sizeof( string1 ), localize()->Find("#Spec_Map" ),1, wMapName );
	}

	SetLabelText("extrainfo", string1 );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSpectatorGUI::UpdateScores( void )
{
	IGameResources *pGR = GameResources();

	if( !pGR )
		return;

	// Draw team scores up top-ish

	for( int i = TEAM_BLUE; i <= TEAM_GREEN; i++ )
	{
		int iLabelNum = i - TEAM_BLUE + 1;

		char szLabel[ 32 ];
		Q_snprintf( szLabel, sizeof( szLabel ), "team%ilabel", iLabelNum );

		char szLabelScore[ 32 ];
		Q_snprintf( szLabelScore, sizeof( szLabel ), "team%iscorelabel", iLabelNum );

		wchar_t szTeam[ MAX_TEAM_NAME_LENGTH + 1 ];
		wchar_t szTeamScore[ MAX_TEAM_NAME_LENGTH + 1 ];

		// Grab valid teams where team limit isn't -1
		if( GetGlobalTeam( i ) && ( pGR->GetTeamLimits( i ) != -1 ) )
		{
			wchar_t *szName = localize()->Find( pGR->GetTeamName( i ) );
			if( szName )
			{
				_snwprintf( szTeam, sizeof( szTeam ), L"%s", szName );
			}
			else
			{
				wchar_t szTemp[ MAX_TEAM_NAME_LENGTH + 1 ];
				
				localize()->ConvertANSIToUnicode( pGR->GetTeamName( i ), szTemp, sizeof( szTemp ) );
				_snwprintf( szTeam, sizeof( szTeam ), L"%s", szTemp );				
			}

			_snwprintf( szTeamScore, sizeof( szTeam ), L"%i", pGR->GetTeamScore( i ) );
		}
		else
		{
			// Team limit -1 or getglobalteam failed
			_snwprintf( szTeam, sizeof( szTeam ), L"" );
			_snwprintf( szTeamScore, sizeof( szTeamScore ), L"" );
		}

		// Team
		SetLabelText( szLabel, szTeam );
		// Team score
		SetLabelText( szLabelScore, szTeamScore );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Resets the list of players
//-----------------------------------------------------------------------------
void CSpectatorMenu::Update( void )
{
	int itemID = 0;

	IGameResources *gr = GameResources();

	Reset();

	if( m_iDuckKey < 0 )
	{
		m_iDuckKey = gameuifuncs->GetEngineKeyCodeForBind( "duck" );
	}
	
	if ( !gr )
		return;

	int iPlayerIndex;
	for ( iPlayerIndex = 1 ; iPlayerIndex <= gpGlobals->maxClients; iPlayerIndex++ )
	{

		// does this slot in the array have a name?
		if ( !gr->IsConnected( iPlayerIndex ) )
			continue;
			
		if ( gr->IsLocalPlayer( iPlayerIndex ) )
			continue;

		if ( !gr->IsAlive( iPlayerIndex ) )
			continue;

		wchar_t playerText[ 80 ], playerName[ 64 ], *team, teamText[ 64 ];
		char localizeTeamName[64];
		localize()->ConvertANSIToUnicode( gr->GetPlayerName(iPlayerIndex), playerName, sizeof( playerName ) );
		const char * teamname = gr->GetTeamName( gr->GetTeam(iPlayerIndex) );
		if ( teamname )
		{	
			Q_snprintf( localizeTeamName, sizeof( localizeTeamName ), "%s", teamname );
			team=localize()->Find( localizeTeamName );

			if ( !team ) 
			{
				localize()->ConvertANSIToUnicode( teamname , teamText, sizeof( teamText ) );
				team = teamText;
			}

			localize()->ConstructString( playerText, sizeof( playerText ), localize()->Find( "#Spec_PlayerItem_Team" ), 2, playerName, team );
		}
		else
		{
			localize()->ConstructString( playerText, sizeof( playerText ), localize()->Find( "#Spec_PlayerItem" ), 1, playerName );
		}

		KeyValues *kv = new KeyValues("UserData", "player", gr->GetPlayerName( iPlayerIndex ) );
		itemID = PlayerAddItem( itemID, playerText, kv ); // -1 means a new slot
		kv->deleteThis();
	}

	// make sure the player combo box is up to date
	int playernum = GetSpectatorTarget();
	const char *selectedPlayerName = gr->GetPlayerName( playernum );
	for ( iPlayerIndex=0; iPlayerIndex<m_pPlayerList->GetItemCount(); ++iPlayerIndex )
	{
		KeyValues *kv = m_pPlayerList->GetItemUserData( iPlayerIndex );
		if ( kv && FStrEq( kv->GetString( "player" ), selectedPlayerName ) )
		{
			m_pPlayerList->ActivateItemByRow( iPlayerIndex );
			m_pPlayerList->SetText( selectedPlayerName );
			break;
		}
	}
}

void CSpectatorMenu::OnThink()
{
	BaseClass::OnThink();

	IGameResources *gr = GameResources();
	if ( !gr )
		return;

	// make sure the player combo box is up to date
	int playernum = GetSpectatorTarget();
	const char *selectedPlayerName = gr->GetPlayerName( playernum );
	const char *currentPlayerName = "";
	KeyValues *kv = m_pPlayerList->GetActiveItemUserData();
	if ( kv )
	{
		currentPlayerName = kv->GetString("player");
	}
	if ( !FStrEq( currentPlayerName, selectedPlayerName ) )
	{
		for ( int i=0; i<m_pPlayerList->GetItemCount(); ++i )
		{
			KeyValues *kv = m_pPlayerList->GetItemUserData( i );
			if ( kv && FStrEq( kv->GetString( "player" ), selectedPlayerName ) )
			{
				m_pPlayerList->ActivateItemByRow( i );
				m_pPlayerList->SetText( selectedPlayerName );
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates the timer label if one exists
//-----------------------------------------------------------------------------
void CSpectatorGUI::UpdateTimer()
{
	wchar_t szText[ 63 ];

	// Yeah, doesn't work just yet, need to modify gamerules
	int timer = gpGlobals->curtime - FFGameRules()->GetRoundStart();
	//Warning( "[hi] getroundstart %f\n", FFGameRules()->GetRoundStart() );

	_snwprintf ( szText, sizeof( szText ), L"%d:%02d", (timer / 60), (timer % 60) );

	szText[63] = 0;


	SetLabelText("timerlabel", szText );
}

static void ForwardSpecCmdToServer()
{
	if ( engine->IsPlayingDemo() )
		return;

	if ( engine->Cmd_Argc() == 1 )
	{
		// just forward the command without parameters
		engine->ServerCmd( engine->Cmd_Argv(0) );
	}
	else if ( engine->Cmd_Argc() == 2 )
	{
		// forward the command with parameter
		char command[128];
		Q_snprintf( command, sizeof(command), "%s \"%s\"", engine->Cmd_Argv(0), engine->Cmd_Argv(1) );
		engine->ServerCmd( command );
	}
}

CON_COMMAND( spec_next, "Spectate next player" )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->IsHLTV() && engine->IsPlayingDemo() )
	{
		// handle the command clientside
		HLTVCamera()->SpecNextPlayer( false );
	}
	else
	{
		ForwardSpecCmdToServer();
	}
}

CON_COMMAND( spec_prev, "Spectate previous player" )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->IsHLTV() && engine->IsPlayingDemo() )
	{
		// handle the command clientside
		HLTVCamera()->SpecNextPlayer( true );
	}
	else
	{
		ForwardSpecCmdToServer();
	}
}

CON_COMMAND( spec_mode, "Set spectator mode" )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->IsHLTV() && engine->IsPlayingDemo() )
	{
		int mode;

		if ( engine->Cmd_Argc() == 2 )
		{
			mode = Q_atoi( engine->Cmd_Argv(1) );
		}
		else
		{
			mode = HLTVCamera()->GetObserverMode()+1;

			if ( mode > OBS_MODE_ROAMING )
				mode = OBS_MODE_IN_EYE;
		}

		// handle the command clientside
		HLTVCamera()->SetMode( mode );
	}
	else
	{
		ForwardSpecCmdToServer();
	}
}

CON_COMMAND( spec_player, "Spectate player by name" )
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if ( !pPlayer || !pPlayer->IsObserver() )
		return;

	if ( engine->Cmd_Argc() != 2 )
		return;

	if ( engine->IsHLTV() && engine->IsPlayingDemo() )
	{
		// handle the command clientside
		HLTVCamera()->SpecNamedPlayer( engine->Cmd_Argv(1) );
	}
	else
	{
		ForwardSpecCmdToServer();
	}
}



