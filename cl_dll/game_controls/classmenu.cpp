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
#include "ff_modelpanel.h"

#include <cl_dll/iviewport.h>

#include "IGameUIFuncs.h"
#include <igameresources.h>

#include "ff_utils.h"

#include "ff_button.h"

extern IFileSystem **pFilesystem;

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

using namespace vgui;

class MouseOverButton : public FFButton
{
	DECLARE_CLASS_SIMPLE(MouseOverButton, FFButton);

public:

	MouseOverButton(Panel *parent, const char *panelName, const char *text, Panel *pActionSignalTarget = NULL, const char *pCmd = NULL) : BaseClass(parent, panelName, text, pActionSignalTarget, pCmd)
	{
	}

	enum MouseEvent_t
	{
		MOUSE_ENTERED,
		MOUSE_EXITED,
	};

	virtual void OnCursorEntered() 
	{
		BaseClass::OnCursorEntered();

		KeyValues *msg = new KeyValues("MouseOverEvent");
		msg->SetInt("event", MOUSE_ENTERED);
		PostActionSignal(msg);
	}

	virtual void OnCursorExited()
	{
		BaseClass::OnCursorExited();

		KeyValues *msg = new KeyValues("MouseOverEvent");
		msg->SetInt("event", MOUSE_EXITED);
		PostActionSignal(msg);
	}
};

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

	m_pCancelButton = new FFButton(this, "CancelButton", "#FF_CANCEL");
	m_pRandomButton = new FFButton(this, "RandomButton", "#FF_RANDOM");

	char *pszButtons[] = { "ScoutButton", "SniperButton", "SoldierButton", "DemomanButton", "MedicButton", "HwguyButton", "PyroButton", "SpyButton", "EngineerButton", "CivilianButton" };

	for (int iClassIndex = 0; iClassIndex < ARRAYSIZE(pszButtons); iClassIndex++)
	{
		m_pClassButtons[iClassIndex] = new MouseOverButton(this, pszButtons[iClassIndex], (const char *) NULL, this, pszButtons[iClassIndex]);
	}

	m_pModelView = new PlayerModelPanel(this, "ClassPreview");

	// HACKHACKHACKHACK
	// The pushing and popping in m_pModelView is breaking the rendering of subsequent
	// vgui items. Therefore we are sticking this right to the front so that it is
	// rendered last.
	m_pModelView->SetZPos(50);

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
	{
		engine->ClientCmd(VarArgs("class %s", command));
	}

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

		// Update straight away
		Update();
	}
	else
	{
		SetVisible(false);
		SetMouseInputEnabled(false);
		m_pModelView->Reset();
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

	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();

	if (pLocalPlayer == NULL)
		return;

	char nSpacesRemaining[10];
	UTIL_GetClassSpaces(pLocalPlayer->GetTeamNumber(), nSpacesRemaining);

	int nOptions = 0;

	for (int iClassIndex = 0; iClassIndex < 10; iClassIndex++) 
	{
		Button *pClassButton = m_pClassButtons[iClassIndex];

		switch (nSpacesRemaining[iClassIndex])
		{
		case -1:
			pClassButton->SetVisible(false);
			break;
		case 0:
			pClassButton->SetVisible(true);
			pClassButton->SetEnabled(false);
			break;
		default:
			pClassButton->SetVisible(true);
			pClassButton->SetEnabled(true);
			
			nOptions++;
		}
	}

	// Random button only enabled when there's more than one class
	m_pRandomButton->SetVisible((nOptions > 1));

	// Cancel only visible if they already have a class
	m_pCancelButton->SetVisible((pGR->GetClass(pLocalPlayer->entindex()) != 0));

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
	{
		gViewPortInterface->ShowPanel(PANEL_SCOREBOARD, false);
	}

	BaseClass::OnKeyCodeReleased(code);
}

//-----------------------------------------------------------------------------
// Purpose: Update the main display
//-----------------------------------------------------------------------------
void CClassMenu::OnMouseOverMessage(KeyValues *data)
{
	Button *pButton = (Button *) data->GetPtr("panel", NULL);

	// Could not determine where this came from
	if (pButton == NULL)
		return;

	// Get the command from this button and parse accordingly
	if (data->GetInt("event") == MouseOverButton::MOUSE_ENTERED)
	{
		UpdateClassInfo(pButton->GetCommand()->GetString("command"));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Load the correct class into the model view
//-----------------------------------------------------------------------------
void CClassMenu::UpdateClassInfo(const char *pszClassName)
{
	m_pModelView->SetClass(pszClassName);
}