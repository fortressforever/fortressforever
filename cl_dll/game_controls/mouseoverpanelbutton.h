//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef MOUSEOVERPANELBUTTON_H
#define MOUSEOVERPANELBUTTON_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/IScheme.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>
#include <filesystem.h>

extern vgui::Panel *g_lastPanel;

//-----------------------------------------------------------------------------
// Purpose: Triggers a new panel when the mouse goes over the button
//-----------------------------------------------------------------------------
class MouseOverPanelButton : public vgui::Button
{
private:
	DECLARE_CLASS_SIMPLE( MouseOverPanelButton, vgui::Button );
	
public:
	MouseOverPanelButton( vgui::Panel *parent, const char *panelName, vgui::Panel *templatePanel ) :
	  vgui::Button( parent, panelName, L"MouseOverPanelButton" )
	{
		m_pPanel = new vgui::HTML( parent, NULL );
		m_pPanel->SetVisible( false );

		// copy size&pos from template panel
		int x,y, wide, tall;
		templatePanel->GetBounds( x, y, wide, tall );
		m_pPanel->SetBounds( x, y, wide, tall );
	}

	void ShowPage( )
	{
		if( m_pPanel )
		{
			m_pPanel->SetVisible( true );
			m_pPanel->MoveToFront( );
			g_lastPanel = m_pPanel;
		}
	}
	
	void HidePage( )
	{
		if( m_pPanel )
		{
			m_pPanel->SetVisible( false );
		}
	}

	const char *GetClassPage( const char *className )
	{
		static char classPanel[ _MAX_PATH ];

		// --> Mirv: [HACK] Quick way to get round renaming files for now (V SILLY)
		char name[128];
		sprintf( name, GetName() );
		
		if( strlen(name) > 6 )
			name[strlen(name) - 6] = 0;

		Q_snprintf( classPanel, sizeof( classPanel ), "resource/classes/%s.html", /*className*/ name );
		// <-- Mirv: [HACK] Quick way to get round renaming files for now (V SILLY)

		if( vgui::filesystem( )->FileExists( classPanel ) )
		{
		}
		else if( vgui::filesystem( )->FileExists( "resource/classes/default.html" ) )
		{
			Q_snprintf ( classPanel, sizeof( classPanel ), "resource/classes/default.html" );
		}
		else
		{
			return NULL;
		}

		
		return classPanel;
	}

	virtual void ApplySettings( KeyValues *resourceData ) 
	{
		BaseClass::ApplySettings( resourceData );

		char szLocalFile[ _MAX_PATH ];

		vgui::filesystem( )->GetLocalPath( GetClassPage( GetName( ) ), szLocalFile, sizeof( szLocalFile ) );
		m_pPanel->OpenURL( szLocalFile );
	}

private:

	virtual void OnCursorEntered( ) 
	{
		BaseClass::OnCursorEntered( );

		if( m_pPanel && IsEnabled( ) )
		{
			if( g_lastPanel )
			{
				g_lastPanel->SetVisible( false );
			}

			ShowPage( );
		}
	}

	vgui::HTML *m_pPanel;
};


#endif // MOUSEOVERPANELBUTTON_H
