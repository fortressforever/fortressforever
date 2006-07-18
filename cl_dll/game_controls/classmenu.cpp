/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file classmenu.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date August 15, 2005
/// @brief New class selection menu
///
/// REVISIONS
/// ---------
/// Aug 15, 2005 Mirv: First creation

#include "cbase.h"
#include "classmenu.h"
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

#include "IGameUIFuncs.h"
#include <igameresources.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
extern INetworkStringTable *g_pStringTableInfoPanel;
extern IGameUIFuncs *gameuifuncs;

// Button names
const char *szClassButtons[] = { "scoutbutton", "sniperbutton", "soldierbutton", 
								 "demomanbutton", "medicbutton", "hwguybutton", 
								 "pyrobutton", "spybutton", "engineerbutton", 
								 "civilianbutton" };

//-----------------------------------------------------------------------------
// Purpose: Create a test menu
//-----------------------------------------------------------------------------
//CON_COMMAND(classmenu, "Shows the class menu") 
//{
//	if (!gViewPortInterface) 
//		return;
//	
//	IViewPortPanel *panel = gViewPortInterface->FindPanelByName(PANEL_CLASS);
//
//	 if (panel) 
//		 gViewPortInterface->ShowPanel(panel, true);
//	 else
//		 Msg("Couldn't find panel.\n");
//}

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CClassMenu::CClassMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_CLASS) 
{
	// initialize dialog
	m_pViewPort = pViewPort;

	m_flNextUpdate = 0;

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	// hide the system buttons
	SetTitleBarVisible(false);

	// info window about this class
	m_pPanel = new Panel(this, "ClassInfo");
	m_pPanel->SetFgColor(Color(255, 255, 255, 255));

	m_pCancel = new Button(this, "cancelbutton", "#FF_CANCEL");

	LoadControlSettings("Resource/UI/ClassMenu.res");
	
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CClassMenu::~CClassMenu() 
{
}

//-----------------------------------------------------------------------------
// Purpose: Do whatever command is needed
//-----------------------------------------------------------------------------
void CClassMenu::OnCommand(const char *command) 
{
	if (Q_strcmp(command, "cancel") != 0) 
		engine->ClientCmd(command);

	m_pViewPort->ShowPanel(this, false);

	BaseClass::OnCommand(command);
}

//-----------------------------------------------------------------------------
// Purpose: Nothing
//-----------------------------------------------------------------------------
void CClassMenu::SetData(KeyValues *data) 
{
}

//-----------------------------------------------------------------------------
// Purpose: Show or don't show
//-----------------------------------------------------------------------------
void CClassMenu::ShowPanel(bool bShow) 
{
	if (BaseClass::IsVisible() == bShow) 
		return;

	m_pViewPort->ShowBackGround(bShow);

	if (bShow) 
	{
		Activate();
		SetMouseInputEnabled(true);
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Don't need anything yet
//-----------------------------------------------------------------------------
void CClassMenu::Reset() 
{
}

//-----------------------------------------------------------------------------
// Purpose: Update the menu with everything
//-----------------------------------------------------------------------------
void CClassMenu::Update() 
{
	IGameResources *pGR = GameResources();

	if (!pGR) 
		return;

	vgui::Button *pClassButton;

	int iClassNumbers[12] = {0};

	// number of classes available
	int iVisibleButtons = 0;

	// A quick count of who is what class
	for (int i = 1; i < gpGlobals->maxClients; i++) 
	{
		if (pGR->IsConnected(i) && pGR->GetTeam(i) == C_BasePlayer::GetLocalPlayer()->GetTeamNumber()) 
			iClassNumbers[pGR->GetClass(i) ]++;
	}	

	// We have to do this in here because it keeps breaking otherwise
	for (int iClass = 0; iClass < 10; iClass++) 
	{
		// Get the number of available slots for this class
		int class_limit = pGR->GetTeamClassLimits(C_BasePlayer::GetLocalPlayer()->GetTeamNumber(), iClass + 1);
		int slots_avail = class_limit - iClassNumbers[iClass + 1];

		pClassButton = (vgui::Button *) FindChildByName(szClassButtons[iClass]);

		// Hide the button if class is marked as unavailable
		if (class_limit == -1 || (class_limit > 0 && slots_avail <= 0)) 
			pClassButton->SetVisible(false);
		else
		{
			pClassButton->SetVisible(true);
			iVisibleButtons++;
		}
	}

	// Only one available class, disable random
	if( iVisibleButtons <= 1 )
	{
		pClassButton = ( vgui::Button * )FindChildByName( "randombutton" );
		if( pClassButton )
			pClassButton->SetVisible( false );
	}

	// If they are unassigned then they have to choose a class really
	if (pGR->GetClass(C_BasePlayer::GetLocalPlayer()->entindex()) == 0) 
		m_pCancel->SetVisible(false);
	else
		m_pCancel->SetVisible(true);

	m_flNextUpdate = gpGlobals->curtime + 0.2f;
}

//-----------------------------------------------------------------------------
// Purpose: Give them some key control too
//-----------------------------------------------------------------------------
void CClassMenu::OnKeyCodePressed(KeyCode code) 
{
	// Show the scoreboard over this if needed
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("showscores")) 
		gViewPortInterface->ShowPanel(PANEL_SCOREBOARD, true);

	// Support hiding the class menu by hitting your changeteam button again like TFC
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("changeclass")) 
		gViewPortInterface->ShowPanel(this, false);

	// Support bring the team menu back up if the class menu is showing
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("changeteam")) 
	{
		m_pViewPort->ShowPanel(this, false);
		engine->ClientCmd("changeteam");
	}

	BaseClass::OnKeyCodePressed(code);
}

void CClassMenu::OnKeyCodeReleased(KeyCode code)
{
	// Bug #0000524: Scoreboard gets stuck with the class menu up when you first join
	// Hide the scoreboard now
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("showscores"))
		gViewPortInterface->ShowPanel(PANEL_SCOREBOARD, false);

	BaseClass::OnKeyCodeReleased(code);
}


//-----------------------------------------------------------------------------
// Purpose: Magic override to allow vgui to create mouse over buttons for us
//-----------------------------------------------------------------------------
Panel *CClassMenu::CreateControlByName(const char *controlName) 
{
	if (!Q_stricmp("MouseOverPanelButton", controlName)) 
	{
		MouseOverPanelButton *newButton = CreateNewMouseOverPanelButton(m_pPanel);
		
		return newButton;
	}
	else
	{
		return BaseClass::CreateControlByName(controlName);
	}
}

//-----------------------------------------------------------------------------
// Purpose: A lovely factory function
//-----------------------------------------------------------------------------
MouseOverPanelButton * CClassMenu::CreateNewMouseOverPanelButton(vgui::Panel *panel) 
{
	return new MouseOverPanelButton(this, NULL, panel);
}
