#ifndef FF_IRC_H
#define FF_IRC_H

#ifdef _WIN32
	#pragma once
#endif

#include "vgui_helpers.h"
#include "KeyValues.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/RichText.h>
#include <vgui_controls/ListPanel.h>
#include "ff_gameui.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Thread Test
//-----------------------------------------------------------------------------

class CThreadTest : public CThread
{
	DECLARE_CLASS_SIMPLE( CThreadTest, CThread );
	
	public:
		int Run();
		bool IsRunning() { return m_bIsRunning; };
		static CThreadTest& GetInstance(); 
		
	protected:
		CThreadTest( void );
		~CThreadTest( void );

	private:
		bool m_bIsRunning;
};

//-----------------------------------------------------------------------------
// IRC Tab
//-----------------------------------------------------------------------------

class CFFIRCTab : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CFFIRCTab, PropertyPage);

public:
	CFFIRCTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName) 
	{
		m_pTextEntry_ChatEntry = new vgui::TextEntry(this, "TextEntry_ChatEntry");
		m_pRichText_Chat = new vgui::RichText(this, "RichText_Chat");
		m_pUserList = new ListPanel(this, "ListPanel_UserList");

		m_pTextEntry_ChatEntry->SendNewLine(true); // Pressing enter in the text box triggers a new line message
		m_pTextEntry_ChatEntry->AddActionSignalTarget( this ); // Something about sending messages to the right panel?
		
		m_pUserList->AddActionSignalTarget( this );
		
        m_pUserList->AddColumnHeader( 0, "access" , "" , 20/*, ListPanel::COLUMN_HIDDEN*/ );
        m_pUserList->AddColumnHeader( 1, "name" , "User List" , 200 );

		m_pUserList->SetSortFunc( 0, &(UserList_SortFunc) ); // access, name
		m_pUserList->SetSortColumn( 0 ); // access
		m_pUserList->SetColumnSortable( 1, false ); // turn off by-name-only sorting
		
		UserList_SetReceiving( false );
	}

	virtual void ApplySchemeSettings( IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		m_pTextEntry_ChatEntry->SetFont( pScheme->GetFont( "DefaultVerySmall" ) );
		m_pRichText_Chat->SetFont( pScheme->GetFont( "DefaultVerySmall" ) );
		m_pUserList->SetFont( pScheme->GetFont( "DefaultVerySmall" ) );
	}
	
	void UserMessage( const char *user, const char *message )
	{
		m_pRichText_Chat->InsertString( VarArgs("%s: %s\n", user, message) );
		m_pRichText_Chat->GotoTextEnd();
	}
	
	void SystemMessage( const char *message )
	{
		m_pRichText_Chat->InsertString( "< " );
		m_pRichText_Chat->InsertString( message );
		m_pRichText_Chat->InsertString( " >\n" );
		m_pRichText_Chat->GotoTextEnd();
	}

	bool UserList_IsReceiving( ) { return m_bReceivingNames; }
	void UserList_SetReceiving( bool bState ) { m_bReceivingNames = bState; }
	
	void UserList_Fill( const char *list )
	{
		// clear the list if this is the first names message
		if (!UserList_IsReceiving())
		{
			UserList_SetReceiving( true );
			m_pUserList->RemoveAll();
		}

		char *listBuffer = new char[ strlen(list) ];
		Q_strcpy(listBuffer, list);

		char *pUser = strtok(listBuffer, " \r\n");

		// populate the list
		while (pUser)
		{
			UserList_AddUser( pUser );

			pUser = strtok(NULL, " \r\n");
		}
		delete[] listBuffer;

		//m_pUserList->SortList();

	}
	
	void UserList_AddUser( char *username )
	{
		KeyValues *kv = new KeyValues( "ircuser" );
		switch(username[0])
		{
		// op
		case '@':
			kv->SetInt( "access", 2 );
			username+=1;
			break;
		// vip
		case '+':
			kv->SetInt( "access", 1 );
			username+=1;
			break;
		// regular user
		default:
			kv->SetInt( "access", 0 );
		}
		kv->SetString( "name", username );
		// add and sort
		m_pUserList->AddItem( kv, 0, false, true );
		kv->deleteThis();

		DevMsg("[IRC] Adding user to userlist: %s\n", username);
	}

	void UserList_RemoveUser( const char *username )
	{
		int index = UserList_FindUserByName( username );

		if (m_pUserList->IsValidItemID(index))
		{
			m_pUserList->RemoveItem( index );
			DevMsg("[IRC] Removing user from userlist: %s\n", username);
		}
	}
	
	void UserList_UpdateUserName( const char *username, const char *newname )
	{
		int index = UserList_FindUserByName( username );

		if (m_pUserList->IsValidItemID(index))
		{
			KeyValues *kv = m_pUserList->GetItem( index );
			DevMsg("[IRC] Updating user name from %s to: %s\n", username, newname);
			kv->SetString( "name", newname );
		}
	}
	
	void UserList_UpdateUserAccess( const char *username, int newaccess )
	{
		int index = UserList_FindUserByName( username );

		if (m_pUserList->IsValidItemID(index))
		{
			KeyValues *kv = m_pUserList->GetItem( index );
			DevMsg("[IRC] Updating user access for %s to: %d\n", username, newaccess);
			kv->SetInt( "access", newaccess );
		}
	}
	
	int UserList_FindUserByName( const char *username )
	{
		for( int i = m_pUserList->FirstItem(); m_pUserList->IsValidItemID(i); i = m_pUserList->NextItem(i) )
		{
			KeyValues *kv = m_pUserList->GetItem( i );
			kv = kv->FindKey( "name" );
			if( kv && Q_strcmp(kv->GetString(), username) == 0 )
				return i;
		}
		return -1;
	}

protected:

	static int UserList_SortFunc( ListPanel *pPanel, const ListPanelItem &item1, const ListPanelItem &item2 )
	{
		const vgui::ListPanelItem *p1 = &item1;
		const vgui::ListPanelItem *p2 = &item2;

		if ( !p1 || !p2 ) // No meaningful comparison
			return 0;

		// first compare access
		int v1 = p1->kv->GetInt( "access" );
		int v2 = p2->kv->GetInt( "access" );
		if( v1 > v2 )
			return -1;
		else if( v1 < v2 )
			return 1;

		// next compare name
		return Q_stricmp(p1->kv->GetString( "name" ), p2->kv->GetString( "name" )); 
	}

protected:

	vgui::TextEntry			*m_pTextEntry_ChatEntry;
	vgui::RichText			*m_pRichText_Chat;
	vgui::ListPanel			*m_pUserList;

private:
	
	bool m_bReceivingNames;

};

//-----------------------------------------------------------------------------
// IRC Panel
//-----------------------------------------------------------------------------

class CFFIRCPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFIRCPanel,Frame);

public:
	CFFIRCPanel( vgui::VPANEL parent );
	~CFFIRCPanel();

	void AddGameTab();
	void RemoveGameTab( CFFIRCTab *pTab );
	
	void ParseServerMessage( char *buf );

	MESSAGE_FUNC( Close, "Close" );

private:

	//MESSAGE_FUNC_PARAMS( OnNewLineMessage, "TextNewLine",data); // When TextEntry sends a TextNewLine message (when user presses enter), trigger the function OnNewLineMessage
	//MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	//MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	
	//void CFFIRCPanel::OnNewLineMessage(KeyValues *data);

	vgui::PropertySheet		*m_pIRCTabs;
	//CFFIRCLobbyTab		*m_pLobbyTab;
	//CFFIRCGameTab			*m_pGame1Tab;
	CFFIRCTab				*m_pGameTab;
	CFFIRCTab				*m_pLobbyTab;

	vgui::TextEntry*		m_pTextEntry_ChatEntry;
	vgui::RichText*			m_pRichText_LobbyChat;

	CThreadTest				*m_pThread;

	//vgui::Button			*m_pOKButton;
	//vgui::Button			*m_pCancelButton;
	//vgui::Button			*m_pApplyButton;

};

DECLARE_GAMEUI(CFFIRC, CFFIRCPanel, ffirc);

#endif // FF_IRC_H 