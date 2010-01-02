#include "cbase.h"
#include "ff_irc.h"
// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// CFFIRCLobbyTab
//-----------------------------------------------------------------------------

class CFFIRCLobbyTab : public CFFIRCTab
{
	DECLARE_CLASS_SIMPLE(CFFIRCLobbyTab, CFFIRCTab);

public:
	
	CFFIRCLobbyTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
	{
		m_pTextEntry_ChatEntry = new vgui::TextEntry(this, "TextEntry_ChatEntry");
		m_pRichText_LobbyChat = new vgui::RichText(this, "RichText_LobbyChat");
		m_pRichText_LobbyUserList = new vgui::RichText(this, "RichText_LobbyUserList");

		m_pTextEntry_ChatEntry->SendNewLine(true); // Pressing enter in the text box triggers a new line message
		m_pTextEntry_ChatEntry->AddActionSignalTarget( this ); // Something about sending messages to the right panel?

		LoadControlSettings("resource/ui/FFIRCLobbyTab.res");
	}
	
private:

	MESSAGE_FUNC_PARAMS(OnNewLineMessage, "TextNewLine", data)
	{
		char szCommand[256];
		m_pTextEntry_ChatEntry->GetText(szCommand, sizeof(szCommand));
	                 
		//const char* text = data->GetString("text");
		m_pRichText_LobbyChat->InsertString(szCommand);
		m_pRichText_LobbyChat->InsertString("\n");
		m_pTextEntry_ChatEntry->SetText("");
		m_pRichText_LobbyChat->GotoTextEnd();
	}
	
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data)
	{
		const char *pszCommand = data->GetString("command");

		if (Q_strcmp(pszCommand, "AddGame") == 0)
		{
			CFFIRCPanel *parent = dynamic_cast <CFFIRCPanel *> (GetParent()->GetParent());

			if (parent)
			{
				parent->AddGameTab();
			}
		}
	}

	vgui::TextEntry			*m_pTextEntry_ChatEntry;
	vgui::RichText			*m_pRichText_LobbyChat;
	vgui::RichText			*m_pRichText_LobbyUserList;

};

//-----------------------------------------------------------------------------
// CFFIRCGameTab
//-----------------------------------------------------------------------------

class CFFIRCGameTab : public CFFIRCTab
{
	DECLARE_CLASS_SIMPLE(CFFIRCGameTab, CFFIRCTab);

public:
	
	CFFIRCGameTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
	{
		m_pTextEntry_ChatEntry = new vgui::TextEntry(this, "TextEntry_ChatEntry");
		m_pRichText_GameChat = new vgui::RichText(this, "RichText_GameChat");
		m_pRichText_GameUserList = new vgui::RichText(this, "RichText_GameUserList");

		m_pTextEntry_ChatEntry->SendNewLine(true); // Pressing enter in the text box triggers a new line message
		m_pTextEntry_ChatEntry->AddActionSignalTarget( this ); // Something about sending messages to the right panel?

		LoadControlSettings("resource/ui/FFIRCGameTab.res");
	}
	
private:

	MESSAGE_FUNC_PARAMS(OnNewLineMessage, "TextNewLine", data)
	{
		char szCommand[256];
		m_pTextEntry_ChatEntry->GetText(szCommand, sizeof(szCommand));
	                 
		//const char* text = data->GetString("text");
		m_pRichText_GameChat->InsertString(szCommand);
		m_pRichText_GameChat->InsertString("\n");
		m_pTextEntry_ChatEntry->SetText("");
		m_pRichText_GameChat->GotoTextEnd();
	}
	
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data)
	{
		const char *pszCommand = data->GetString("command");

		if (Q_strcmp(pszCommand, "RemoveGame") == 0)
		{
			CFFIRCPanel *parent = dynamic_cast <CFFIRCPanel *> (GetParent()->GetParent());

			if (parent)
			{
				parent->RemoveGameTab( this );
			}
		}
	}

	vgui::TextEntry			*m_pTextEntry_ChatEntry;
	vgui::RichText			*m_pRichText_GameChat;
	vgui::RichText			*m_pRichText_GameUserList;

};

//-----------------------------------------------------------------------------
// CFFIRCPanel gameui definition
//-----------------------------------------------------------------------------

DEFINE_GAMEUI(CFFIRC, CFFIRCPanel, ffirc);

CON_COMMAND(ToggleIRCPanel,NULL)
{
	//ToggleVisibility(ffirc->GetPanel());
	ffirc->GetPanel()->SetVisible(true);
}

//-----------------------------------------------------------------------------
// CFFIRCPanel implementation
//-----------------------------------------------------------------------------

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

	m_pLobbyTab = new CFFIRCLobbyTab(this, "IRCLobbyTab");
	//m_pGameTab = new CFFIRCGameTab(this, "IRCGameTab");
	//m_pLobbyTab = new PropertyPage(this, "IRCLobbyTab");
	//m_pGame1Tab = new PropertyPage(this, "IRCGameTab");

	m_pIRCTabs = new PropertySheet(this, "IRCTabs", true);
	m_pIRCTabs->AddPage(m_pLobbyTab, "Lobby");
	//m_pIRCTabs->AddPage(m_pGame1Tab, "#GameUI_IRCGameTab");
	//m_pIRCTabs->AddPage(m_pGameTab, "Game1");
	m_pIRCTabs->SetActivePage(m_pLobbyTab);
	m_pIRCTabs->SetDragEnabled(false);

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

void CFFIRCPanel::AddGameTab()
{
	m_pGameTab = new CFFIRCGameTab(this, "IRCGameTab");
	m_pIRCTabs->AddPage(m_pGameTab, "Game Name");
}

void CFFIRCPanel::RemoveGameTab( CFFIRCTab *pTab )
{
	m_pIRCTabs->DeletePage(pTab);
}