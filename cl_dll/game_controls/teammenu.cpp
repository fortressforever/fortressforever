/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file teammenu.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date August 15, 2005
/// @brief New team selection menu
///
/// REVISIONS
/// ---------
/// Aug 15, 2005 Mirv: First creation

#include "cbase.h"
#include "teammenu.h"
#include <networkstringtabledefs.h>
#include <cdll_client_int.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <FileSystem.h>
#include <KeyValues.h>
#include <convar.h>
#include <vgui_controls/ImageList.h>

#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/ImagePanel.h>

#include "ff_menu_panel.h"

#include <cl_dll/iviewport.h>
#include <igameresources.h>

#include "IGameUIFuncs.h"
#include "ienginevgui.h"

#include "c_team.h"
#include "ff_utils.h"

#include "ff_button.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern INetworkStringTable *g_pStringTableInfoPanel;
extern IGameUIFuncs *gameuifuncs;

const char *szTeamButtons[] = { "bluebutton", "redbutton", "yellowbutton", "greenbutton" };

#define TEAM_BUTTON_GAP		20

//-----------------------------------------------------------------------------
// Purpose: Lets us make a test menu
//-----------------------------------------------------------------------------
//CON_COMMAND(teammenu, "Shows the team menu") 
//{
//	if (!gViewPortInterface) 
//		return;
//	
//	IViewPortPanel *panel = gViewPortInterface->FindPanelByName(PANEL_TEAM);
//
//	 if (panel) 
//		 gViewPortInterface->ShowPanel(panel, true);
//	 else
//		Msg("Couldn't find panel.\n");
//}

//=============================================================================
// A team button has the following components:
//		A number (this is the button text itself so that hotkeys work automatically)
//		The team insignia image
//		Score
//		Player count
//		Avg ping
//=============================================================================
class TeamButton : public FFButton
{
public:
	DECLARE_CLASS_SIMPLE(TeamButton, FFButton);

	TeamButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL) : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
	{
		m_pTeamInsignia = new ImagePanel(this, "TeamImage");
		m_pInfoDescriptions = new Label(this, "TeamDescription", (const char *) NULL);
		m_pInfoValues = new Label(this, "TeamValues", (const char *) NULL);

		m_iTeamID = -1;
	}

	void ApplySchemeSettings(IScheme *pScheme)
	{
		BaseClass::ApplySchemeSettings(pScheme);


		SetTextInset(10, 7);
		SetContentAlignment(a_northwest);

		// Line up image
		int iInsigniaSize = GetTall() * 0.4f;

		m_pTeamInsignia->SetBounds(GetWide() / 2 - iInsigniaSize / 2, GetTall() / 2.5f - iInsigniaSize / 2, iInsigniaSize, iInsigniaSize);
		m_pTeamInsignia->SetShouldScaleImage(true);
		m_pTeamInsignia->SetMouseInputEnabled(false);

		// Line up info descriptions
		m_pInfoDescriptions->SetContentAlignment(a_northwest);
		m_pInfoDescriptions->SetBounds(GetWide() * 0.1f, GetTall() * 0.6f, GetWide() * 0.4f, GetTall() * 0.4f);
		m_pInfoDescriptions->SetMouseInputEnabled(false);

		// Line up info values
		m_pInfoValues->SetContentAlignment(a_northeast);
		m_pInfoValues->SetBounds(GetWide() * 0.5f, GetTall() * 0.6f, GetWide() * 0.4f, GetTall() * 0.4f);
		m_pInfoValues->SetMouseInputEnabled(false);

		m_pInfoDescriptions->SetText("#TEAM_STATS");
		m_pInfoValues->SetText("0\n0\n0");
	}

	//-----------------------------------------------------------------------------
	// Purpose: Set the team id used by this button for updating itself
	//			Also load the team insignia image.
	//-----------------------------------------------------------------------------
	void SetTeamID(int iTeamID)
	{
		Assert(iTeamID >= TEAM_BLUE && iTeamID <= TEAM_GREEN);
		m_iTeamID = iTeamID;

		const char *pszInsignias[] = { "hud_team_blue", "hud_team_red", "hud_team_yellow", "hud_team_green" };
		m_pTeamInsignia->SetShouldScaleImage(true);
		m_pTeamInsignia->SetImage(pszInsignias[m_iTeamID - TEAM_BLUE]);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the various sections of the team button
	//-----------------------------------------------------------------------------
	void OnThink()
	{
		IGameResources *pGR = GameResources();

		if (pGR == NULL)
			return;

		int iScore = pGR->GetTeamScore(m_iTeamID);
		int nPlayers = 0;
		int iLag = 0;
		
		for (int iClient = 1; iClient <= gpGlobals->maxClients; iClient++)
		{
			if (!pGR->IsConnected(iClient) || pGR->GetTeam(iClient) != m_iTeamID)
				continue;

			nPlayers++;
			iLag += pGR->GetPing(iClient);
		}

		if (nPlayers > 0)
			iLag /= nPlayers;

		m_pInfoValues->SetText(VarArgs("%d\n%d\n%d", iScore, nPlayers, iLag));
		
		BaseClass::OnThink();
	}

private:

	ImagePanel	*m_pTeamInsignia;
	Label		*m_pInfoDescriptions;
	Label		*m_pInfoValues;

	int			m_iTeamID;
};

// Mulch: TODO: make this work for pheeeeeeeesh-y
CON_COMMAND( hud_reloadteammenu, "hud_reloadteammenu" )
{
	IViewPortPanel *pPanel = gViewPortInterface->FindPanelByName( PANEL_TEAM );

	if( !pPanel )
		return;

	CTeamMenu *pTeamMenu = dynamic_cast< CTeamMenu * >( pPanel );
	if( !pTeamMenu )
		return;

	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFileEx( enginevgui->GetPanel( PANEL_CLIENTDLL ), "resource/ClientScheme.res", "HudScheme" );

	pTeamMenu->SetScheme( scheme );
	pTeamMenu->SetProportional( true );
	pTeamMenu->LoadControlSettings( "Resource/UI/TeamMenu.res" );
}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTeamMenu::CTeamMenu(IViewPort *pViewPort) : BaseClass(NULL, PANEL_TEAM) 
{
	// initialize dialog
	m_pViewPort = pViewPort;

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetProportional(true);

	// Various Frame things
	SetTitleBarVisible(false);
	SetMoveable(false);
	SetSizeable(false);

	// ServerInfo elements
	m_pServerInfoText = new RichText(this, "ServerInfoText");

	// MapDescription elements
	m_pMapDescriptionHead = new Label(this, "MapDescriptionHead", "");
	m_pMapDescriptionText = new RichText(this, "MapDescriptionText");

	char *pszButtons[] = { "BlueTeamButton", "RedTeamButton", "YellowTeamButton", "GreenTeamButton" };
	
	for (int iTeamIndex = 0; iTeamIndex < ARRAYSIZE(pszButtons); iTeamIndex++)
	{
		m_pTeamButtons[iTeamIndex] = new TeamButton(this, pszButtons[iTeamIndex], (const char *) NULL, this, pszButtons[iTeamIndex]);
		m_pTeamButtons[iTeamIndex]->SetTeamID(iTeamIndex + TEAM_BLUE);
	}

	m_pAutoAssignButton = new FFButton(this, "AutoAssignButton", (const char *) NULL, this, "AutoAssign");
	m_pSpectateButton = new FFButton(this, "SpectateButton", (const char *) NULL, this, "Spectate");
	m_pFlythroughButton = new FFButton(this, "FlythroughButton", (const char *) NULL, this, "mapguide overview");

	m_pMapScreenshotButton = new FFButton(this, "MapScreenshotButton", (const char *) NULL, this, "MapShot");

	// Mulch: Removed this
	//new Button(this, "screenshot", "screenshot", this, "jpeg");

	LoadControlSettings("Resource/UI/TeamMenu.res");
	
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTeamMenu::~CTeamMenu() 
{
}

void CTeamMenu::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	m_pMapDescriptionText->SetBorder(NULL);
	m_pServerInfoText->SetBorder(NULL);
}

//-----------------------------------------------------------------------------
// Purpose: Run the client command if needed
//-----------------------------------------------------------------------------
void CTeamMenu::OnCommand(const char *command) 
{
	//DevMsg("[Teammenu] Command: %s\n", command);

	if (Q_strcmp(command, "cancel") == 0) 
	{
		m_pViewPort->ShowPanel(this, false);
		return;
	}

	// Create a new frame to display the Map Screenshot
	if (Q_strcmp(command, "map shot") == 0) 
	{
		gViewPortInterface->ShowPanel(PANEL_MAP, true);
		return;
	}

	// Run the command
	engine->ClientCmd(command);

	if (Q_strcmp(command, "jpeg") == 0)
	{
		return;
	}

	// Hide this panel
	m_pViewPort->ShowPanel(this, false);

	if (Q_strcmp(command, "team spec") == 0) 
		return;

	if (Q_strcmp(command, "mapguide") == 0) 
		return;

	// Display the class panel now
	gViewPortInterface->ShowPanel(PANEL_CLASS, true);

	BaseClass::OnCommand(command);
}

//-----------------------------------------------------------------------------
// Purpose: Nothings
//-----------------------------------------------------------------------------
void CTeamMenu::SetData(KeyValues *data) 
{
}

//-----------------------------------------------------------------------------
// Purpose: Give them some key control too
//-----------------------------------------------------------------------------
void CTeamMenu::OnKeyCodePressed(KeyCode code) 
{
	// Show the scoreboard over this if needed
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("showscores"))
		gViewPortInterface->ShowPanel(PANEL_SCOREBOARD, true);

	// Support hiding the team menu by hitting your changeteam button again like TFC
	// 0001232: Or if the user presses escape, kill the menu
	if ((engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("changeteam")) ||
		(engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("cancelselect"))) 
		gViewPortInterface->ShowPanel(this, false);

	// Bug #0000540: Can't changeclass while changeteam menu is up
	// Support bring the class menu back up if the team menu is showing
	if( ( engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind( "changeclass" ) ) &&
		( C_BasePlayer::GetLocalPlayer()->GetTeamNumber() >= TEAM_BLUE ) ) 
	{
		m_pViewPort->ShowPanel( this, false );
		engine->ClientCmd( "changeclass" );
	}

	BaseClass::OnKeyCodePressed(code);
}

void CTeamMenu::OnKeyCodeReleased(KeyCode code)
{
	// Bug #0000524: Scoreboard gets stuck with the class menu up when you first join
	// Hide the scoreboard now
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("showscores"))
		gViewPortInterface->ShowPanel(PANEL_SCOREBOARD, false);

	BaseClass::OnKeyCodeReleased(code);
}

//-----------------------------------------------------------------------------
// Purpose: Show the panel or whatever
//-----------------------------------------------------------------------------
void CTeamMenu::ShowPanel(bool bShow) 
{
	if (BaseClass::IsVisible() == bShow) 
		return;

	m_pViewPort->ShowBackGround(false);

	if (bShow) 
	{
		Activate();
		SetMouseInputEnabled(true);
		SetKeyBoardInputEnabled(true);
		SetEnabled(true);

		Update();

		MoveToFront();
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
		SetKeyBoardInputEnabled(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Don't need anything yet
//-----------------------------------------------------------------------------
void CTeamMenu::Reset() 
{
}

//-----------------------------------------------------------------------------
// Purpose: Update the menu with everything
//-----------------------------------------------------------------------------
void CTeamMenu::Update() 
{
	// TODO: Some of these should only happen once per map
	UpdateMapDescriptionText();
	UpdateServerInfo();
	UpdateTeamButtons();
	// When a map 1st loads, the image proxy is in front of the button (so you can't click it) -- this seems to fix that
	m_pMapScreenshotButton->SetZPos( 10 );
}

void CTeamMenu::UpdateTeamButtons()
{
	IGameResources *pGR = GameResources();

	if (pGR == NULL)
		return;

	char nTeamSpaces[4];
	UTIL_GetTeamSpaces(nTeamSpaces);

	int nActiveButtons = 0;

	for (int iTeamID = TEAM_BLUE; iTeamID <= TEAM_GREEN; iTeamID++)
	{
		int iTeamIndex = iTeamID - TEAM_BLUE;
		TeamButton *pTeamButton = m_pTeamButtons[iTeamIndex];

		// This team doesn't exist at all
		if (nTeamSpaces[iTeamIndex] == -1)
		{
			pTeamButton->SetVisible(false);
			continue;
		}

		// Make sure team button is visible
		pTeamButton->SetVisible(true);

		// Need this for later
		nActiveButtons++;

		// The team is full, so disable 
		pTeamButton->SetEnabled((nTeamSpaces[iTeamIndex] != 0));

		// set the team number
		wchar_t wchTeamNumber = iTeamIndex + '1';

		// Set the team name
		wchar_t *szTeamName = localize()->Find( pGR->GetTeamName( iTeamID ) );
		wchar_t	szName[ 256 ];

		if (szTeamName)
		{
			swprintf( szName, L"%c. %s", wchTeamNumber, szTeamName );
			szTeamName = szName;
		}
		else
		{
			// No localized text or team name not a resource string
			char szString[ 256 ];
			Q_snprintf( szString, 256, "%c. %s", wchTeamNumber, pGR->GetTeamName( iTeamID ) );
			localize()->ConvertANSIToUnicode( szString, szName, sizeof( szName ) );
			szTeamName = szName;
		}

		// one last check
		if ( szTeamName )
		{
			pTeamButton->SetText(szTeamName);
		}
		else
		{
			// no name, just use the number
			swprintf( szTeamName, L"%c.", wchTeamNumber );
			pTeamButton->SetText(szTeamName);
		}

		pTeamButton->SetHotkey(wchTeamNumber);
	}

	int iTeamButtonGap = scheme()->GetProportionalScaledValue(TEAM_BUTTON_GAP);

	// Now that we know how many active buttons there are we can
	// move them into the correct location
	int iStartX = m_pTeamButtons[0]->GetWide() * nActiveButtons * 0.5f;
	iStartX += (nActiveButtons - 1) * (iTeamButtonGap * 0.5f);
	iStartX = (GetWide() * 0.5f) - iStartX;

	// Move active buttons in correct place
	for (int iTeamIndex = 0; iTeamIndex < 4; iTeamIndex++)
	{
		TeamButton *pTeamButton = m_pTeamButtons[iTeamIndex];

		if (pTeamButton->IsVisible())
		{
			int iXPos, iYPos;
			pTeamButton->GetPos(iXPos, iYPos);

			pTeamButton->SetPos(iStartX, iYPos);
			iStartX += pTeamButton->GetWide() + iTeamButtonGap;
		}
		
	}

	// Done here for now
	int iXPos, iYPos;

	m_pSpectateButton->GetPos(iXPos, iYPos);
	m_pSpectateButton->SetPos(GetWide() / 2 - (m_pAutoAssignButton->GetWide() + iTeamButtonGap / 2), iYPos);

	m_pAutoAssignButton->GetPos(iXPos, iYPos);
	m_pAutoAssignButton->SetPos(GetWide() / 2 + iTeamButtonGap / 2, iYPos);
}

void CTeamMenu::UpdateServerInfo()
{
	//const ConVar *pHostname = cvar->FindVar("hostname");
	//const char *pszTitle = (pHostname) ? pHostname->GetString() : "#FF_DEFAULT_MOTD";

	//m_pServerInfoText->SetText(pszTitle);

	if (g_pStringTableInfoPanel == NULL)
		return;

	int iIndex = g_pStringTableInfoPanel->FindStringIndex("motd");

	if (iIndex == ::INVALID_STRING_INDEX)
	{
		m_pServerInfoText->SetText("#FF_DEFAULT_MOTD");
		return;
	}

	int nLength = 0;
	const char *pszMotd = (const char *) g_pStringTableInfoPanel->GetStringUserData(iIndex, &nLength);

	m_pServerInfoText->SetText(pszMotd);
}

void CTeamMenu::UpdateMapDescriptionText()
{
	char szMapName[MAX_MAP_NAME];
	Q_FileBase(engine->GetLevelName(), szMapName, sizeof(szMapName));

	const char *pszMapPath = VarArgs("maps/%s.txt", szMapName);

	// If no map specific description exists then escape for now
	if (!vgui::filesystem()->FileExists(pszMapPath))
	{
		m_pMapDescriptionHead->SetText("");
		m_pMapDescriptionText->SetText("");
		return;
	}

	// Read from local text from file
	FileHandle_t f = vgui::filesystem()->Open(pszMapPath, "rb", "GAME");

	if (!f) 
		return;

	char szBuffer[2048];
				
	int size = min(vgui::filesystem()->Size(f), sizeof(szBuffer) - 1);

	vgui::filesystem()->Read(szBuffer, size, f);
	vgui::filesystem()->Close(f);

	szBuffer[size] = 0;

	const char *pszEndOfHead = strstr(szBuffer, "\n");

	// Could not find a title for this, just stick everything in the normal spot
	// Or there was nothing after the title
	if (!pszEndOfHead || (pszEndOfHead + 1) == '\0')
	{
		m_pMapDescriptionHead->SetText("Unknown map style");
		m_pMapDescriptionText->SetText(szBuffer);

		return;
	}

	int iEndOfHead = pszEndOfHead - szBuffer;

	// Put the rest of the text into the description richtext first.
	// We're then just going to stick in a null terminator and then add the start
	// to the head label.
	m_pMapDescriptionText->SetText(pszEndOfHead + 1);
	
	szBuffer[iEndOfHead] = 0;
	m_pMapDescriptionHead->SetText(szBuffer);
}
