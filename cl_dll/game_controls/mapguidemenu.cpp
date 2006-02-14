/********************************************************************
	created:	2006/02/13
	created:	13:2:2006   18:45
	filename: 	f:\ff-svn\code\trunk\cl_dll\game_controls\mapguidemenu.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\game_controls
	file base:	mapguidemenu
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "mapguidemenu.h"
#include <networkstringtabledefs.h>
#include <cdll_client_int.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include <FileSystem.h>
#include <KeyValues.h>
#include <convar.h>
#include <vgui_controls/ImageList.h>

#include <vgui_controls/BitmapImagePanel.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/ImagePanel.h>

#include <cl_dll/iviewport.h>

#include "IGameUIFuncs.h"
#include <igameresources.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
extern INetworkStringTable *g_pStringTableInfoPanel;
extern IGameUIFuncs *gameuifuncs;

//-----------------------------------------------------------------------------
// Purpose: Create a test menu
//-----------------------------------------------------------------------------
CON_COMMAND(mapguidemenu, "Shows the mapguide menu") 
{
	if (!gViewPortInterface) 
		return;
	
	IViewPortPanel *panel = gViewPortInterface->FindPanelByName(PANEL_MAPGUIDE);

	 if (panel)
	 {
		 gViewPortInterface->ShowPanel(panel, true);
		 gViewPortInterface->ShowPanel(PANEL_TEAM, false);
		 gViewPortInterface->ShowPanel(PANEL_CLASS, false);
	 }
	 else
		 Msg("Couldn't find panel.\n");
}

//-----------------------------------------------------------------------------
// Purpose: This handles our mouseover buttons
//-----------------------------------------------------------------------------
class GuideMouseOverPanelButton : public vgui::Button
{
private:
	DECLARE_CLASS_SIMPLE( GuideMouseOverPanelButton, vgui::Button );

	vgui::ImagePanel *m_pImagePanel;

public:
	GuideMouseOverPanelButton( vgui::Panel *parent, const char *panelName, vgui::Panel *templatePanel ) :
	  vgui::Button( parent, panelName, L"GuideMouseOverPanelButton" )
	  {
		  m_pImagePanel = (vgui::ImagePanel *) templatePanel;
	  }

private:

	virtual void OnCursorEntered( ) 
	{
		BaseClass::OnCursorEntered();

		char mapName[MAX_MAP_NAME + 4];
		char imagePath[MAX_PATH];

		Q_strncpy(mapName, engine->GetLevelName(), strlen(engine->GetLevelName()) - 3);
		sprintf(imagePath, "%s/%s", mapName, GetName());

		if (m_pImagePanel)
			m_pImagePanel->SetImage(imagePath);
	}
};

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapGuideMenu::CMapGuideMenu(IViewPort *pViewPort) : Frame(NULL, PANEL_MAPGUIDE) 
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

	m_pImagePanel = new ImagePanel(this, "MapGuideImage");

	m_pCancel = new Button(this, "cancelbutton", "#FF_CANCEL");

	LoadControlSettings("Resource/UI/MapGuideMenu.res");

	((vgui::Button *) FindChildByName("overview"))->SetVisible(true);
	((vgui::Button *) FindChildByName("interest"))->SetVisible(true);
	((vgui::Button *) FindChildByName("extraguide1"))->SetVisible(false);
	((vgui::Button *) FindChildByName("extraguide2"))->SetVisible(false);
	((vgui::Button *) FindChildByName("extraguide3"))->SetVisible(false);
	((vgui::Button *) FindChildByName("extraguide4"))->SetVisible(false);
	
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapGuideMenu::~CMapGuideMenu() 
{
}

//-----------------------------------------------------------------------------
// Purpose: Do whatever command is needed
//-----------------------------------------------------------------------------
void CMapGuideMenu::OnCommand(const char *command) 
{
	if (Q_strcmp(command, "cancel") != 0) 
		engine->ClientCmd(command);
	else
		// Back to the team menu
		m_pViewPort->ShowPanel(PANEL_TEAM, true);

	m_pViewPort->ShowPanel(this, false);

	BaseClass::OnCommand(command);
}

//-----------------------------------------------------------------------------
// Purpose: Nothing
//-----------------------------------------------------------------------------
void CMapGuideMenu::SetData(KeyValues *data) 
{
}

//-----------------------------------------------------------------------------
// Purpose: Show or don't show
//-----------------------------------------------------------------------------
void CMapGuideMenu::ShowPanel(bool bShow) 
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
void CMapGuideMenu::Reset() 
{
}

//-----------------------------------------------------------------------------
// Purpose: Update the menu with everything
//-----------------------------------------------------------------------------
void CMapGuideMenu::Update() 
{
}

//-----------------------------------------------------------------------------
// Purpose: Give them some key control too
//-----------------------------------------------------------------------------
void CMapGuideMenu::OnKeyCodePressed(KeyCode code) 
{
	// Show the scoreboard over this if needed
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("showscores")) 
		gViewPortInterface->ShowPanel(PANEL_SCOREBOARD, true);

	// Support hiding the menu by hitting your changeteam button again like TFC
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("changeteam")) 
		gViewPortInterface->ShowPanel(this, false);

	// Support bring the team menu back up if the class menu is showing
	if (engine->GetLastPressedEngineKey() == gameuifuncs->GetEngineKeyCodeForBind("changeteam")) 
	{
		m_pViewPort->ShowPanel(this, false);
		engine->ClientCmd("changeteam");
	}

	BaseClass::OnKeyCodePressed(code);
}

//-----------------------------------------------------------------------------
// Purpose: Magic override to allow vgui to create mouse over buttons for us
//-----------------------------------------------------------------------------
Panel *CMapGuideMenu::CreateControlByName(const char *controlName) 
{
	if (!Q_stricmp("MouseOverPanelButton", controlName)) 
	{
		GuideMouseOverPanelButton *newButton = CreateNewMouseOverPanelButton(m_pImagePanel);
		
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
GuideMouseOverPanelButton *CMapGuideMenu::CreateNewMouseOverPanelButton(vgui::Panel *panel) 
{
	return new GuideMouseOverPanelButton(this, NULL, panel);
}
