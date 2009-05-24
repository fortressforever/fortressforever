//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VGUITEXTWINDOW_H
#define VGUITEXTWINDOW_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include <cl_dll/iviewport.h>

namespace vgui
{
	class TextEntry;
}

//-----------------------------------------------------------------------------
// Purpose: displays the MOTD
//-----------------------------------------------------------------------------

enum
{
	TYPE_TEXT = 0,	// just display this plain text
	TYPE_INDEX,		// lookup text & title in stringtable
	TYPE_URL,		// show this URL
	TYPE_FILE,		// show this local file
} ;

class CTextWindow : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE( CTextWindow, vgui::Frame );

public:
	CTextWindow(IViewPort *pViewPort);
	virtual ~CTextWindow();

	virtual const char *GetName( void ) { return PANEL_INFO; }
	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual bool NeedsUpdate( void ) { return false; }
	virtual bool HasInputElements( void ) { return true; }
	virtual void ShowPanel( bool bShow );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel( void ) { return BaseClass::GetVPanel(); }
  	virtual bool IsVisible() { return BaseClass::IsVisible(); }
  	virtual void SetParent( vgui::VPANEL parent ) { BaseClass::SetParent( parent ); }

public:

	virtual void SetData( int type, const char *title, const char *message, const char *command );
	virtual void ShowFile( const char *filename);
	virtual void ShowText( const char *text);
	virtual void ShowURL( const char *URL);
	virtual void ShowIndex( const char *entry);

protected:	
	// vgui overrides
	virtual void OnCommand( const char *command);

	IViewPort	*m_pViewPort;
	char		m_szTitle[255];
	char		m_szMessage[2048];
	char		m_szExitCommand[255];
	int			m_nContentType;

	vgui::TextEntry	*m_pTextMessage;
	vgui::HTML		*m_pHTMLMessage;
	vgui::Button	*m_pOK;
	vgui::Label		*m_pTitleLable;
};


#endif // VGUITEXTWINDOW_H
