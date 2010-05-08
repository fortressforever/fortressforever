//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_irc.h
//	@author Ryan Liptak (squeek)
//	@date 30/01/2010
//	@brief IRC interface
//
//	REVISIONS
//	---------

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
#include <vgui_controls/Button.h>
#include "ff_gameui.h"

using namespace vgui;

// quakenet.org server info:
//		:xs4all.nl.quakenet.org 005 squeektest 
//			WHOX WALLCHOPS WALLVOICES USERIP CPRIVMSG CNOTICE SILENCE=15 
//			MODES=6 MAXCHANNELS=20 MAXBANS=45 NICKLEN=15 :are supported by this server
//		:xs4all.nl.quakenet.org 005 squeektest 
//			MAXNICKLEN=15 TOPICLEN=250 AWAYLEN=160 KICKLEN=250 CHANNELLEN=200 
//			MAXCHANNELLEN=200 CHANTYPES=#& PREFIX=(ov)@+ STATUSMSG=@+ 
//			CHANMODES=b,k,l,imnpstrDducCNMT CASEMAPPING=rfc1459 
//			NETWORK=QuakeNet :are supported by this server

#define IRC_MAX_CHANLEN 200
#define IRC_MAX_NICKLEN 15
#define IRC_MAX_TOPICLEN 250
#define IRC_USERMODES "ovb"

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

		m_szChannel[0] = 0;
		
		UserList_SetReceiving( false );
	}

	virtual void ApplySchemeSettings( IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		m_pTextEntry_ChatEntry->SetFont( pScheme->GetFont( "DefaultVerySmall" ) );
		m_pRichText_Chat->SetFont( pScheme->GetFont( "DefaultVerySmall" ) );
		m_pUserList->SetFont( pScheme->GetFont( "DefaultVerySmall" ) );
	}
	
	void SetChannel( const char *channel, bool joinchannel = true )
	{
		if(channel[0] == '#')
			Q_strncpy( m_szChannel, channel+1, IRC_MAX_CHANLEN );
		else
			Q_strncpy( m_szChannel, channel, IRC_MAX_CHANLEN );
	}
	
	const char* GetChannel()
	{
		return m_szChannel;
	}

	void UserMessage( const char *user, const char *message )
	{
		m_pRichText_Chat->InsertString( VarArgs("%s: %s\n", user, message) );
		m_pRichText_Chat->GotoTextEnd();
	}
	
	void UserMessage( const char *user, const char *message, Color clr )
	{
		m_pRichText_Chat->InsertColorChange( clr );
		m_pRichText_Chat->InsertString( VarArgs("%s: %s\n", user, message) );
		m_pRichText_Chat->InsertColorChange( m_pRichText_Chat->GetFgColor() );
		m_pRichText_Chat->GotoTextEnd();
	}
	
	void SystemMessage( const char *message )
	{
		m_pRichText_Chat->InsertColorChange( Color(255,255,255,255) );
		m_pRichText_Chat->InsertString( "< " );
		m_pRichText_Chat->InsertString( message );
		m_pRichText_Chat->InsertString( " >\n" );
		m_pRichText_Chat->InsertColorChange( m_pRichText_Chat->GetFgColor() );
		m_pRichText_Chat->GotoTextEnd();
	}

	void SystemMessage( const char *message, Color clr )
	{
		m_pRichText_Chat->InsertColorChange( clr );
		m_pRichText_Chat->InsertString( "< " );
		m_pRichText_Chat->InsertString( message );
		m_pRichText_Chat->InsertString( " >\n" );
		m_pRichText_Chat->InsertColorChange( m_pRichText_Chat->GetFgColor() );
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
	
	bool UserList_IsInList( const char *username )
	{
		int index = UserList_FindUserByName( username );

		if (m_pUserList->IsValidItemID(index))
			return true;
		else
			return false;
	}
	
	void UserList_UpdateUserName( const char *username, const char *newname )
	{
		int index = UserList_FindUserByName( username );

		if (m_pUserList->IsValidItemID(index))
		{
			KeyValues *kv = m_pUserList->GetItem( index );
			DevMsg("[IRC] Updating user name from %s to: %s\n", username, newname);
			kv->SetString( "name", newname );
			m_pUserList->ApplyItemChanges( index );
			m_pUserList->SortList();
		}
	}
	
	void UserList_UpdateUserAccess( const char *username, int newaccess, bool bCanSetLower=true )
	{
		int index = UserList_FindUserByName( username );

		if (m_pUserList->IsValidItemID(index))
		{
			KeyValues *kv = m_pUserList->GetItem( index );
			DevMsg("[IRC] Updating user access for %s to: %d\n", username, newaccess);
			// only go through with the update if its necessary to
			if(kv->GetInt("access") != newaccess && bCanSetLower || (!bCanSetLower && newaccess > kv->GetInt("access")))
			{
				kv->SetInt( "access", newaccess );
				m_pUserList->ApplyItemChanges( index );
				m_pUserList->SortList();
			}
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
	vgui::TextEntry		*m_pTextEntry_ChatEntry;
	vgui::RichText		*m_pRichText_Chat;
	vgui::ListPanel		*m_pUserList;
	char				m_szChannel[IRC_MAX_CHANLEN+1];

private:
	bool m_bReceivingNames;

};


//-----------------------------------------------------------------------------
// CFFIRCLobbyTab
//-----------------------------------------------------------------------------

class CFFIRCLobbyTab : public CFFIRCTab
{
	DECLARE_CLASS_SIMPLE(CFFIRCLobbyTab, CFFIRCTab);

public:
	CFFIRCLobbyTab(Panel *parent, char const *panelName);
	virtual void ApplySchemeSettings( IScheme *pScheme );

	vgui::ListPanel			*m_pGameList;

private:
	MESSAGE_FUNC_PARAMS(OnNewLineMessage, "TextNewLine", data);
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);



};


//-----------------------------------------------------------------------------
// CFFIRCGameTab
//-----------------------------------------------------------------------------

class CFFIRCGameTab : public CFFIRCTab
{
	DECLARE_CLASS_SIMPLE(CFFIRCGameTab, CFFIRCTab);

public:
	CFFIRCGameTab(Panel *parent, char const *panelName);

private:
	MESSAGE_FUNC_PARAMS(OnNewLineMessage, "TextNewLine", data);
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);

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

	void AddGameTab( const char *name, const char *channel );
	void RemoveGameTab( CFFIRCGameTab *pTab );
	CFFIRCGameTab* GetGameTabByChannel( const char * );

	void SetVisible(bool state);
	void RetrieveServerMessage();
	
	void ParseServerMessage( char *buf );
	virtual void OnTick();

	MESSAGE_FUNC( Close, "Close" );
	
	struct cUser
	{
		char nick[IRC_MAX_NICKLEN+1]; 
		char ident[100];
		char email[100];
		char status;
	};
	cUser irc_user;
	bool m_bDataReady;

private:
	//MESSAGE_FUNC_PARAMS( OnNewLineMessage, "TextNewLine",data); // When TextEntry sends a TextNewLine message (when user presses enter), trigger the function OnNewLineMessage
	//MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	//MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	
	//void CFFIRCPanel::OnNewLineMessage(KeyValues *data);

	vgui::PropertySheet			*m_pIRCTabs;
	CUtlVector<CFFIRCGameTab*>	m_pGameTabs;
	CFFIRCLobbyTab				*m_pLobbyTab;

	vgui::TextEntry*		m_pTextEntry_ChatEntry;
	vgui::RichText*			m_pRichText_LobbyChat;

};

DECLARE_GAMEUI(CFFIRC, CFFIRCPanel, ffirc);

//=============================================================================
// Popup window that displays status, allows login, etc
//=============================================================================
class CFFIRCConnectPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFIRCConnectPanel, Frame);

public:
	CFFIRCConnectPanel( vgui::VPANEL parent );

	void Reset();
	
	void SetVisible(bool state);
	void UpdateStatus( const char *status );
	void ConnectFailed();
	void Connected();

private:
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);

private:
	vgui::TextEntry* m_pTextEntry_NickEntry;
	vgui::Label* m_pStatusLabel;

};

DECLARE_GAMEUI(CFFIRCConnect, CFFIRCConnectPanel, ffircconnect);

#endif // FF_IRC_H 