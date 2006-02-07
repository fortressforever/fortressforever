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

#include <cl_dll/iviewport.h>
#include <igameresources.h>

#include "IGameUIFuncs.h"

#include "c_team.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
extern INetworkStringTable *g_pStringTableInfoPanel;
extern IGameUIFuncs *gameuifuncs;

const char *szTeamButtons[] = { "bluebutton", "redbutton", "yellowbutton", "greenbutton" };

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

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CTeamMenu::CTeamMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_TEAM) 
{
	// initialize dialog
	m_pViewPort = pViewPort;

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	// hide the system buttons
	SetTitleBarVisible(false);

	m_pMapInfo		= new RichText(this, "MapInfo");
	m_pMapName		= new Label(this, "mapname", "Unknown Map");

	m_pCancel		= new Button(this, "cancelbutton", "#FF_MENU_CANCEL");

	LoadControlSettings("Resource/UI/TeamMenu.res");
	
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CTeamMenu::~CTeamMenu() 
{
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

	// Run the command
	engine->ClientCmd(command);

	// Hide this panel
	m_pViewPort->ShowPanel(this, false);

	if (Q_strcmp(command, "team spec") == 0) 
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
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("changeteam")) 
		gViewPortInterface->ShowPanel(this, false);

	BaseClass::OnKeyCodePressed(code);
}

//-----------------------------------------------------------------------------
// Purpose: Show the panel or whatever
//-----------------------------------------------------------------------------
void CTeamMenu::ShowPanel(bool bShow) 
{
	if (BaseClass::IsVisible() == bShow) 
		return;

	m_pViewPort->ShowBackGround(bShow);

	if (bShow) 
	{
		Activate();
		SetMouseInputEnabled(true);
		SetKeyBoardInputEnabled(true);
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
	IGameResources *pGR = GameResources();

	if (!pGR) 
		return;

	vgui::Button *pTeamButton;

	// First count number of players in each team
	int iTeamNumbers[8] = {0};

	// A quick count of who is what class
	for (int i = 1; i < gpGlobals->maxClients; i++) 
	{
		if (pGR->IsConnected(i)) 
			iTeamNumbers[pGR->GetTeam(i) ]++;
	}

	// We have to do this in here because it keeps breaking otherwise
	for (int iTeam = 0; iTeam < 4; iTeam++) 
	{
		pTeamButton = (vgui::Button *) FindChildByName(szTeamButtons[iTeam]);

		// This is one of the 4 teams and isn't full
		if (GetGlobalTeam(iTeam) && (pGR->GetTeamLimits(iTeam) == 0 || iTeamNumbers[iTeam] < pGR->GetTeamLimits(iTeam))) 
		{
			pTeamButton->SetVisible(true);

			wchar_t *szName = localize()->Find(pGR->GetTeamName(iTeam + 2));

			//DevMsg("[Teammenu] Found localized text \"%s\", %s\n", pGR->GetTeamName(iTeam + 2), (szName) ? "yes" : "no");

			// Name the button(in either unicode or ansi, don't care
			if (szName) 
			{
				wchar_t szbuf[256];
				swprintf(szbuf, L"&%i. %s", iTeam + 1, szName);

				pTeamButton->SetText(szbuf);
			}
			else
			{
				char szbuf[128];
				sprintf(szbuf, "&%i. %s", iTeam + 1, pGR->GetTeamName(iTeam + 2));

				pTeamButton->SetText(szbuf);
			}				
		}
		else
			pTeamButton->SetVisible(false);
	}

	// If they are unassigned then they have to choose a team really
	if (C_BasePlayer::GetLocalPlayer()->GetTeamNumber() == 0) 
		m_pCancel->SetVisible(false);
	else
		m_pCancel->SetVisible(true);

	// Set the map name
	char mapname[MAX_MAP_NAME];
	Q_FileBase(engine->GetLevelName(), mapname, sizeof(mapname));

	m_pMapName->SetText(mapname);
	m_pMapName->SetVisible(true);


	// Set the map description
	char mapRES[ MAX_PATH ];

	Q_snprintf(mapRES, sizeof(mapRES), "maps/%s.txt", mapname);

	// if no map specific description exists, load default text
	if (vgui::filesystem()->FileExists(mapRES)) 
	{
		// read from local text from file
		FileHandle_t f = vgui::filesystem()->Open(mapRES, "rb", "GAME");

		if (!f) 
			return;

		char buffer[2048];
				
		int size = min(vgui::filesystem()->Size(f), sizeof(buffer) -1); // just allow 2KB

		vgui::filesystem()->Read(buffer, size, f);
		vgui::filesystem()->Close(f);

		buffer[size]=0; //terminate string

		//ShowText(buffer);
		m_pMapInfo->SetText(buffer);
		m_pMapInfo->SetVisible(true);
	}
}
