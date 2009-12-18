#include "cbase.h"
#include "ff_irc.h"
// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

DEFINE_GAMEUI(CFFIRC, CFFIRCPanel, ffirc);

CON_COMMAND(ToggleIRCPanel,NULL)
{
	//ToggleVisibility(ffirc->GetPanel());
	ffirc->GetPanel()->SetVisible(true);
}


// The main screen where everything IRC related is added to
CFFIRCPanel::CFFIRCPanel( vgui::VPANEL parent ) : BaseClass( NULL, "FFIRCPanel" )
{
	SetParent(parent);
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme( scheme );

	// Centre this panel on the screen for consistency.
	// This should be in the .res surely? - AfterShock
	//int nWide = GetWide();
	//int nTall = GetTall();

	//SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

	//m_pLobbyTab = new CFFIRCLobbyTab(this, "IRCLobbyTab");
	//m_pGame1Tab = new CFFIRCGameTab(this, "IRCGameTab");
	//m_pLobbyTab = new PropertyPage(this, "IRCLobbyTab");
	//m_pGame1Tab = new PropertyPage(this, "IRCGameTab");

	//m_pIRCTabs = new PropertySheet(this, "IRCTabs", true);
	//m_pIRCTabs->AddPage(m_pLobbyTab, "#GameUI_IRCLobbyTab");
	//m_pIRCTabs->AddPage(m_pLobbyTab, "Lobby");
	//m_pIRCTabs->AddPage(m_pGame1Tab, "#GameUI_IRCGameTab");
	//m_pIRCTabs->AddPage(m_pGame1Tab, "Game1");
	//m_pIRCTabs->SetActivePage(m_pLobbyTab);
	//m_pIRCTabs->SetDragEnabled(false);

	m_pTextEntryChat1 = new vgui::TextEntry(this, "MyTextEntry");
	//m_pTextEntryChat1->SetPos(150, 310);
	//m_pTextEntryChat1->SetSize(100, 20);

	//SetPos(10,10);
	//SetSize(500, 400);
	//m_pTextEntryChat1->SetPos

	//m_pOKButton = new Button(this, "PlayButton", "", this, "Play");
	//m_pCancelButton = new Button(this, "CancelButton", "", this, "Cancel");

	SetSizeable(false);


	LoadControlSettings("Resource/UI/FFIRC.res");
	//CenterThisPanelOnScreen();//keep in mind, hl2 supports widescreen 
	SetVisible(false);//made visible on command later 

	//Other useful options
	//SetSizeable(false);
	//SetMoveable(false);
} 