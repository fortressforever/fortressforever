/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file mapscreen.cpp
/// @author Christopher "Jiggles" Boylan
/// @date August 30, 2007
/// @brief To display the map screenshot


#include "cbase.h"
#include "mapscreen.h"
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


// Global
CMapScreen *g_pMapScreen = NULL;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CMapScreen::CMapScreen(IViewPort *pViewPort) : Frame(NULL, PANEL_MAP) 
{
	g_pMapScreen = this;

	// initialize dialog
	m_pViewPort = pViewPort;

	m_flNextUpdate = 0;
	m_bMapKeyPressed = false;

	// load the new scheme early!!
	SetScheme("ClientScheme");
	SetMoveable(false);
	SetSizeable(false);
	SetProportional(true);

	// hide the system buttons
	SetTitleBarVisible(false);

	m_pCloseButton = new FFButton(this, "CloseButton", "#FF_CLOSE");


	LoadControlSettings("Resource/UI/MapScreenshotMenu.res");

}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CMapScreen::~CMapScreen() 
{
}

//-----------------------------------------------------------------------------
// Purpose: Do whatever command is needed
//-----------------------------------------------------------------------------
void CMapScreen::OnCommand(const char *command) 
{
	if ( ( Q_strcmp(command, "close") != 0 ) && !m_bMapKeyPressed )
	{
		m_pViewPort->ShowPanel(this, false);
	}

	BaseClass::OnCommand(command);
}

//-----------------------------------------------------------------------------
// Purpose: Nothing
//-----------------------------------------------------------------------------
void CMapScreen::SetData(KeyValues *data) 
{
}

//-----------------------------------------------------------------------------
// Purpose: Show or don't show
//-----------------------------------------------------------------------------
void CMapScreen::ShowPanel(bool bShow) 
{
	if (BaseClass::IsVisible() == bShow) 
		return;

	m_pViewPort->ShowBackGround(bShow);

	if (bShow) 
	{
		Activate();
		SetMouseInputEnabled(true);
		//Update();
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
void CMapScreen::Reset() 
{
}

//-----------------------------------------------------------------------------
// Purpose: Don't need anything yet
//-----------------------------------------------------------------------------
void CMapScreen::Update() 
{
	//m_flNextUpdate = gpGlobals->curtime + 0.2f;
}


//-----------------------------------------------------------------------------
// Purpose: Let the player bring up the map at any time
//-----------------------------------------------------------------------------
void CMapScreen::KeyDown( void )
{
	SetVisible( true );
	SetMouseInputEnabled( false );
}

void CMapScreen::KeyUp( void )
{
	SetVisible( false );
}