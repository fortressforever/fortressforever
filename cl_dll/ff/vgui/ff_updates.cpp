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

#include "vstdlib/icommandline.h"
#include <vgui_controls/MessageBox.h>

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

DEFINE_GAMEUI(CFFUpdatesUI, CFFUpdatesPanel, ffupdates);

char *GetModVersion();
CFFUpdatesPanel *g_pUpdatePanel = NULL;

#define UPDATESTATUS_FADETIME 1.0f
#define UPDATESTATUS_FADETIME_DELAY 5.0f

float g_flLastCheck = -15.0f;

void CheckModUpdate(const char *pszServerVersion = NULL)
{
	if (ffupdates->GetPanel())
	{
		((CFFUpdatesPanel*)ffupdates->GetPanel())->CheckUpdate(pszServerVersion);
	}
}

// We have received the server's version, so check on the website
CON_COMMAND(sync_version, "Sync version")
{
	if (engine->Cmd_Argc() > 1 && g_flLastCheck < gpGlobals->realtime)
	{
		g_flLastCheck = gpGlobals->realtime + 3.0f;
		const char *pszVersion = engine->Cmd_Argv(1);
		CheckModUpdate(pszVersion);
		Msg("Server version %s\n", pszVersion);
	}
}

CON_COMMAND(ff_updates,NULL)
{
	CheckModUpdate();
	//ffupdates->GetPanel()->SetVisible(true);
}

CFFUpdatesPanel::CFFUpdatesPanel( vgui::VPANEL parent ) : BaseClass( NULL, "FFUpdatesPanel" )
{
	if (g_pUpdatePanel == NULL)
		g_pUpdatePanel = this;

	SetParent( parent );

	// Make it screen sized
	SetBounds( 0, 0, ScreenWidth(), ScreenHeight() );

	m_pUpdateInfo = new CFFUpdateInfo( this, "ff_update_info" );
	m_pUpdateInfo->SetVisible(false);

	// use SETUP_PANEL to apply scheme settings instantly, so we can set things in the constructor instead of in ApplySchemeSettings
	// this is necessary because UpdateAvailable can finish before ApplySchemeSettings is called, and therefore overwrites settings from the update status
	m_pUpdateStatus = SETUP_PANEL( new Label( this, "ff_update_status", "" ) );
	m_pCurrentVersion = SETUP_PANEL( new Label( this, "ff_current_version", VarArgs("v%s",GetModVersion()) ) );
	
	//m_pCurrentVersion->SetFgColor(Color(255,255,255,255));
	m_pCurrentVersion->SetBgColor(Color(0,0,0,255));
	m_pCurrentVersion->SetPaintBackgroundType(0);
	m_pCurrentVersion->SetPaintBackgroundEnabled(true);
	m_pCurrentVersion->SetZPos(1);
	m_pCurrentVersion->SetSize(70,30);
	m_pCurrentVersion->SetContentAlignment(Label::a_center);

	//m_pUpdateStatus->SetFgColor(Color(255,255,255,255));
	m_pUpdateStatus->SetBgColor(Color(0,0,0,255));
	m_pUpdateStatus->SetPaintBackgroundType(0);
	m_pUpdateStatus->SetPaintBackgroundEnabled(true);
	m_pUpdateStatus->SetZPos(1);
	m_pUpdateStatus->SetSize(180,30);
	m_pUpdateStatus->SetContentAlignment(Label::a_center);
	
	m_pCurrentVersion->SetPos(ScreenWidth()-m_pCurrentVersion->GetWide()-10,ScreenHeight()-m_pCurrentVersion->GetTall()-10);
	m_pUpdateStatus->SetPos(ScreenWidth()-m_pUpdateStatus->GetWide()-m_pCurrentVersion->GetWide()-20,ScreenHeight()-m_pUpdateStatus->GetTall()-10);

	m_flStatusFadeTime = -1.0f;

	// check for updates in a separate thread
	CheckUpdate();
}

CFFUpdatesPanel::~CFFUpdatesPanel()
{
	if (CFFUpdateThread::GetInstance().IsRunning())
		CFFUpdateThread::GetInstance().Terminate();
}

void CFFUpdatesPanel::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
// Check for an update
//-----------------------------------------------------------------------------
void CFFUpdatesPanel::CheckUpdate(const char *pszServerVersion /*= NULL*/)
{
	if (!CFFUpdateThread::GetInstance().IsRunning())
	{
		m_pUpdateStatus->SetText("Checking for updates...");
		m_pUpdateStatus->SetFgColor(Color(255,255,255,255));
		m_pUpdateStatus->SetWide(180);
		m_pUpdateStatus->SetPos(ScreenWidth()-m_pUpdateStatus->GetWide()-m_pCurrentVersion->GetWide()-20,ScreenHeight()-m_pUpdateStatus->GetTall()-10);
		m_flStatusFadeTime = -1.0f;

		if (pszServerVersion)
			Q_strncpy( CFFUpdateThread::GetInstance().m_szServerVersion, pszServerVersion, sizeof(CFFUpdateThread::GetInstance().m_szServerVersion) );

		CFFUpdateThread::GetInstance().Start();
	}
	else
	{
		Msg("[update] Already checking for updates\n");
	}
}

//-----------------------------------------------------------------------------
//	Set if an update is available	
//-----------------------------------------------------------------------------
void CFFUpdatesPanel::UpdateAvailable( eUpdateResponse status )
{
	switch(status)
	{
	case UPDATE_ERROR:
		m_pUpdateStatus->SetText("Error while checking for updates");
		m_pUpdateStatus->SetFgColor(Color(255,0,0,255));
		m_pUpdateStatus->SetWide(225);
		m_flStatusFadeTime = gpGlobals->curtime + UPDATESTATUS_FADETIME + UPDATESTATUS_FADETIME_DELAY;
		break;
	case UPDATE_FOUND:
		Msg("[update] Update available\n");
		m_pUpdateInfo->SetVisible(true);
		m_pUpdateStatus->SetFgColor(Color(229,170,86,255));
		m_pUpdateStatus->SetText("Update found");
		m_pUpdateStatus->SetWide(110);
		m_flStatusFadeTime = -1.0f;
		break;
	case UPDATE_NOTFOUND:
		Msg("[update] No update available\n");
		m_pUpdateStatus->SetFgColor(Color(130,229,100,255));
		m_pUpdateStatus->SetText("Up to date");
		m_pUpdateStatus->SetWide(110);
		m_flStatusFadeTime = gpGlobals->curtime + UPDATESTATUS_FADETIME + UPDATESTATUS_FADETIME_DELAY;
		break;
	case UPDATE_SERVER_OUTOFDATE:
		m_pUpdateStatus->SetText("Server out of date");
		m_pUpdateStatus->SetFgColor(Color(255,0,0,255));
		m_pUpdateStatus->SetWide(180);
		m_flStatusFadeTime = -1.0f;
		break;
	}
	m_pUpdateStatus->SetPos(ScreenWidth()-m_pUpdateStatus->GetWide()-m_pCurrentVersion->GetWide()-20,ScreenHeight()-m_pUpdateStatus->GetTall()-10);
}

//-----------------------------------------------------------------------------
// Purpose: Catch this menu coming on screen and load up the info needed
//			
//-----------------------------------------------------------------------------
void CFFUpdatesPanel::SetVisible(bool state)
{
	//BaseClass::SetVisible(true);
	m_pUpdateInfo->SetVisible(state);
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CFFUpdatesPanel::Paint( void )
{
	if (m_flStatusFadeTime != -1.0f)
	{
		float dt = ( m_flStatusFadeTime - gpGlobals->curtime );
		if (dt > 0)
		{
			// Find our fade based on our time shown
			dt = clamp( dt, 0.0f, UPDATESTATUS_FADETIME );
			float flAlpha = SimpleSplineRemapVal( dt, 0.0f, UPDATESTATUS_FADETIME, 0, 255 );

			m_pUpdateStatus->SetAlpha( flAlpha );
		}
		else
		{
			m_pUpdateStatus->SetVisible( false );
		}
	}
	else
	{
		m_pUpdateStatus->SetVisible( true );
		m_pUpdateStatus->SetAlpha( 255 );
	}

	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// CFFUpdateInfo
//			
//-----------------------------------------------------------------------------
CFFUpdateInfo::CFFUpdateInfo(Panel *parent, const char *panelName, bool showTaskbarIcon) : BaseClass( parent, panelName, showTaskbarIcon )
{
	//Other useful options
	SetVisible(false);//made visible on command later 
	SetSizeable(true);
	SetMoveable(true);

	m_pOKButton = new Button(this, "OKButton", "", this, "OK");
	m_pCancelButton = new Button(this, "CancelButton", "", this, "Cancel");
	m_pOKButton->SetVisible(false);
	m_pCancelButton->SetVisible(false);

	LoadControlSettings("Resource/UI/FFUpdates.res");
	//CenterThisPanelOnScreen();//keep in mind, hl2 supports widescreen 
	
	int nWide = GetWide();
	int nTall = GetTall();

	SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

} 

//-----------------------------------------------------------------------------
// Purpose: Catch the buttons. We have OK (save + close), Cancel (close) and
//			Apply (save).
//-----------------------------------------------------------------------------
void CFFUpdateInfo::OnButtonCommand(KeyValues *data)
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

void CFFUpdateInfo::SetVisible(bool state)
{
	if (state)
	{
		RequestFocus();
		MoveToFront();
	}

	BaseClass::SetVisible(state);
}