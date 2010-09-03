/********************************************************************
	created:	2006/08/29
	created:	29:8:2006   18:39
	filename: 	cl_dll\ff\vgui\ff_options\ff_options.cpp
	file path:	cl_dll\ff\vgui\ff_options
	file base:	ff_options
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Main "Fortress Options" panel with tabs contining option pages

	notes: 
	
	Previously contained all option pages too. They have now 
	been moved to their own class files
*********************************************************************/

#include "cbase.h"
#include "ff_options.h"

#include "KeyValues.h"

#include "vstdlib/icommandline.h"

#include <vgui_controls/MessageBox.h>
#include <vgui_controls/ComboBox.h>

using namespace vgui;

ConVar hud_takesshots( "hud_takesshots", "0", FCVAR_ARCHIVE | FCVAR_USERINFO, "Takes a screenshot at the end of each map if set to 1 (0 to disable)" );

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

// Jiggles: End Miscellaneous Options Tab
DEFINE_GAMEUI(CFFOptions, CFFOptionsPanel, ffoptions);

//-----------------------------------------------------------------------------
// Purpose: Display the ff options
//-----------------------------------------------------------------------------
CON_COMMAND(ff_options, NULL)
{
	ffoptions->GetPanel()->SetVisible(true);
}
	
//-----------------------------------------------------------------------------
// Purpose: Set up our main options screen. This involves creating all the
//			tabbed pages too.
//-----------------------------------------------------------------------------
CFFOptionsPanel::CFFOptionsPanel(vgui::VPANEL parent) : BaseClass(NULL, "FFOptionsPanel")
{
	SetParent(parent);
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme(scheme);

	// Centre this panel on the screen for consistency.
	int nWide = GetWide();
	int nTall = GetTall();

	SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

	// This should not be visible since we're only 
	// showing it when selected in the main menu.
	SetVisible(false);

	m_pCrosshairOptions = new CFFCrosshairOptions(this, "CrosshairOptions");
	m_pTimerOptions = new CFFTimerOptions(this, "TimerOptions");
	m_pMiscOptions1 = new CFFMiscOptions(this, "MiscOptions", "resource/Options1.vdf");
	m_pMiscOptions2 = new CFFMiscOptions(this, "MiscOptions", "resource/Options2.vdf");
	m_pMiscOptions3 = new CFFMiscOptions(this, "MiscOptions", "resource/Options3.vdf");
	m_pMiscOptions4 = new CFFMiscOptions(this, "MiscOptions", "resource/Options4.vdf");
	m_pDLightOptions = new CFFDLightOptions(this, "DLightOptions");

	m_pPropertyPages = new PropertySheet(this, "OptionsPages", true);
	m_pPropertyPages->AddPage(m_pCrosshairOptions, "#GameUI_Crosshairs");
	m_pPropertyPages->AddPage(m_pTimerOptions, "#GameUI_Timers");
	m_pPropertyPages->AddPage(m_pMiscOptions1, "#GameUI_Misc1");
	m_pPropertyPages->AddPage(m_pMiscOptions2, "#GameUI_Misc2");
	m_pPropertyPages->AddPage(m_pMiscOptions3, "#GameUI_Misc3");
	m_pPropertyPages->AddPage(m_pMiscOptions4, "#GameUI_Misc4");
	m_pPropertyPages->AddPage(m_pDLightOptions, "#GameUI_DLights");
	m_pPropertyPages->SetActivePage(m_pCrosshairOptions);
	m_pPropertyPages->SetDragEnabled(false);

	m_pOKButton = new Button(this, "OKButton", "", this, "OK");
	m_pCancelButton = new Button(this, "CancelButton", "", this, "Cancel");
	m_pApplyButton = new Button(this, "ApplyButton", "", this, "Apply");

	SetSizeable(false);
	
	LoadControlSettings("resource/ui/FFOptions.res");
}

//-----------------------------------------------------------------------------
// Purpose: Catch the buttons. We have OK (save + close), Cancel (close) and
//			Apply (save).
//-----------------------------------------------------------------------------
void CFFOptionsPanel::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	// Both Apply and OK save the options window stuff
	if (Q_strcmp(pszCommand, "Apply") == 0 || Q_strcmp(pszCommand, "OK") == 0)
	{
		m_pCrosshairOptions->Apply();
		m_pTimerOptions->Apply();
		m_pMiscOptions1->Apply();
		m_pMiscOptions2->Apply();
		m_pMiscOptions3->Apply();
		m_pDLightOptions->Apply();

		// Apply doesn't quit the menu
		if (pszCommand[0] == 'A')
		{
			return;
		}
	}
	else
	{
		// Cancelled, so reset the settings
		m_pCrosshairOptions->Reset();
		m_pTimerOptions->Reset();
		m_pMiscOptions1->Reset();
		m_pMiscOptions2->Reset();
		m_pMiscOptions3->Reset();
		m_pDLightOptions->Reset();
	}

	// Now make invisible
	SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: Catch this options menu coming on screen and load up the info needed
//			for the option pages.
//-----------------------------------------------------------------------------
void CFFOptionsPanel::SetVisible(bool state)
{
	if (state)
	{
		m_pCrosshairOptions->Load();
		m_pTimerOptions->Load();
		m_pMiscOptions1->Load();
		m_pMiscOptions2->Load();
		m_pMiscOptions3->Load();
		m_pDLightOptions->Load();

		RequestFocus();
		MoveToFront();
	}

	BaseClass::SetVisible(state);
}

// This is the mod version string (defined in ff_gamerules.cpp)
// Jon: reading from a file now
//extern const char *MOD_CLIENT_VERSION;
char *GetModVersion();

// This is the URL for checking updates.
char *szSplashUrl = "http://www.fortress-forever.com/notifier/check.php?c=%s&s=%s";

// Singleton for our splash panel
static CFFSplashPanel *g_pSplashPanel = NULL;

class SplashHTML : public HTML
{
	DECLARE_CLASS_SIMPLE(SplashHTML, HTML);

public:
	SplashHTML(Panel *parent, const char *name, bool allowJavaScript = false) : HTML(parent, name, allowJavaScript) {}

	void OnFinishURL(const char *url)
	{
		BaseClass::OnFinishURL(url);

		// We've been redirected to a page that we need to display
		if (url && Q_strstr(url, "update_notice") != NULL)
		{
			Msg("Client/Server out of date\n");
			GetParent()->SetVisible(true);
			RequestFocus();
			GetParent()->MoveToFront();
		}
		else
		{
			Msg("Client/Server up to date\n");
		}
	}
};

CFFSplashPanel::CFFSplashPanel(vgui::VPANEL parent) : BaseClass(NULL, "FFSplashPanel")
{
	Assert(g_pSplashPanel == NULL);
	g_pSplashPanel = this;

	m_pSplashHTML = new SplashHTML(this, "FFSplashPanelHTML");
	m_pSplashHTML->SetVisible(true);

	SetParent(parent);
	SetSizeable(false);

	LoadControlSettings("resource/ui/FFSplash.res");

	// Do our message box in here too
	if (CommandLine()->FindParm("-showdxmsg"))
	{
		MessageBox *pMessageBox = new MessageBox("DirectX Changed", "DirectX level changed\nPlease now quit and load Fortress Forever normally for the settings to take effect.", 0);
		pMessageBox->SetVisible(true);
		pMessageBox->RequestFocus();
		pMessageBox->MoveToFront();
		pMessageBox->ShowWindow();
	}

	CheckUpdate();
};

void CFFSplashPanel::CheckUpdate(const char *pszServerVersion /*= NULL*/)
{
	m_pSplashHTML->OpenURL(VarArgs(szSplashUrl, GetModVersion(), pszServerVersion ? pszServerVersion : ""));
	m_pSplashHTML->SetVisible(true);
}

void CheckModUpdate(const char *pszServerVersion = NULL)
{
	Assert(g_pSplashPanel);
	if (g_pSplashPanel)
	{
		g_pSplashPanel->CheckUpdate(pszServerVersion);
	}
}

DEFINE_GAMEUI(CFFSplash, CFFSplashPanel, ffsplash);

float g_flLastCheck = -15.0f;

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
