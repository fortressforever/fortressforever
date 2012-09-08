/********************************************************************
	created:	2006/09/28
	created:	28:9:2006   13:03
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_gamemodes.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_gamemodes
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "KeyValues.h"
#include "ff_updates.h"
#include <vgui_controls/Button.h>
#include <vgui_controls/Label.h>

#include "update/autoupdate.h"

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

DEFINE_GAMEUI(CFFUpdatesUI, CFFUpdatesPanel, ffupdates);

CFFUpdatesPanel *g_pUpdatePanel = NULL;

CON_COMMAND(ff_updates,NULL)
{
	//ToggleVisibility(ffirc->GetPanel());
	ffupdates->GetPanel()->SetVisible(true);
}


// The main screen where everything IRC related is added to
CFFUpdatesPanel::CFFUpdatesPanel( vgui::VPANEL parent ) : BaseClass( NULL, "FFUpdatesPanel" )
{
	SetParent(parent);
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme( scheme );

	if (g_pUpdatePanel == NULL)
		g_pUpdatePanel = this;

	// Centre this panel on the screen for consistency.
	// This should be in the .res surely? - AfterShock

	m_pOKButton = new Button(this, "OKButton", "", this, "OK");
	m_pCancelButton = new Button(this, "CancelButton", "", this, "Cancel");
	m_pOKButton->SetVisible(false);
	m_pCancelButton->SetVisible(false);

	LoadControlSettings("Resource/UI/FFUpdates.res");
	//CenterThisPanelOnScreen();//keep in mind, hl2 supports widescreen 
	
	int nWide = GetWide();
	int nTall = GetTall();

	SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

	//Other useful options
	SetVisible(false);//made visible on command later 
	SetSizeable(false);
	SetMoveable(true);

	// check for updates in a separate thread
	CFFUpdateThread::GetInstance().Start();
} 

//-----------------------------------------------------------------------------
// Purpose: Catch the buttons. We have OK (save + close), Cancel (close) and
//			Apply (save).
//-----------------------------------------------------------------------------
void CFFUpdatesPanel::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	// Play starts the mode
	if (Q_strcmp(pszCommand, "OK") == 0)
	{
		wyInstallUpdate();
		// Just make it invisible now...
		SetVisible(false);
	}
	else if (Q_strcmp(pszCommand, "Cancel") == 0)
	{
		// Now make invisible
		SetVisible(false);
	}

}

//-----------------------------------------------------------------------------
// Purpose: Catch this menu coming on screen and load up the info needed
//			
//-----------------------------------------------------------------------------
void CFFUpdatesPanel::UpdateAvailable(bool state)
{
	if (state)
	{
		Msg("[update] Update available\n");
		SetVisible(true);
	}
	else
	{
		Msg("[update] No update available\n");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Catch this menu coming on screen and load up the info needed
//			
//-----------------------------------------------------------------------------
void CFFUpdatesPanel::SetVisible(bool state)
{
	if (state)
	{
		RequestFocus();
		MoveToFront();
	}
	else
	{

	}

	BaseClass::SetVisible(state);
}