//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_hint.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 05/13/2005
//	@brief Hud Hint class - container for all active
//			hud hints - manages them all
//
//	REVISIONS
//	---------
//	05/13/2005, Mulchman: 
//		First created
//
//	07/11/2005, Mulchman:
//		Added client side ability to add hints

#include "cbase.h"
#include "ff_hud_hint.h"
#include "hudelement.h"
#include "hud_macros.h"
//#include "iclientmode.h"
//#include "engine/IVDebugOverlay.h"

using namespace vgui;

//#include <vgui_controls/Panel.h>
//#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

// [integer] Duration [in seconds] that each hut hint is
// drawn on the screen
static ConVar hint_duration( "ffdev_hint_duration", "5" );

// Helper var
CHudHint *pHudHintHelper = NULL;

DECLARE_HUDELEMENT( CHudHint );
DECLARE_HUD_MESSAGE( CHudHint, FF_HudHint );

CHudHint::~CHudHint( void )
{
	pHudHintHelper = NULL;
}

void CHudHint::VidInit( void )
{
	// Point our helper to us
	pHudHintHelper = this;

	SetPaintBackgroundEnabled( false );
}

void CHudHint::Init( void )
{
	HOOK_HUD_MESSAGE( CHudHint, FF_HudHint );
}

void CHudHint::AddHudHint( const char *pszMessage )
{
	DevMsg( "[Hud Hint] AddHudHint: %s\n", pszMessage );	

	// TODO: TODO: TODO:
	// Do this here for now in case the dev team
	// is testing different durations and/or trying
	// to find the duration they want. Hard code it
	// later in VidInit or Init. This just lets it
	// get updated everytime we get a new hud hint.
	m_flDuration = hint_duration.GetInt( );

	// Do something w/ the string!
	CHint	hHint( pszMessage, gpGlobals->curtime );
}

void CHudHint::MsgFunc_FF_HudHint( bf_read &msg )
{
	// Buffer
	char szString[ 4096 ];

	// Grab the string up to newline
	if( !msg.ReadString( szString, sizeof( szString ), true ) )
	{
		Warning( "[Hud Hint] String larger than buffer - not parsing!\n" );

		return;
	}

	// Pass the string along
	AddHudHint( szString );	
}

void CHudHint::Paint( void )
{
	/*
	if( m_hHints.Count( ) >= 1 )
	{
		if( !IsVisible( ) )
			SetVisible( true );

		// Get our scheme and font information
		vgui::HScheme scheme = vgui::scheme( )->GetScheme( "ClientScheme" );
		vgui::HFont hFont = vgui::scheme( )->GetIScheme( scheme )->GetFont( "Default" );

		char szText[ 256 ];
		Q_strcpy( szText, "Hud Hints - WIP" );
		localize( )->ConvertANSIToUnicode( szText, m_pText, sizeof( m_pText ) );

		// Draw our text	
		surface( )->DrawSetTextFont( hFont ); // set the font	
		surface( )->DrawSetTextColor( GetFgColor( ) ); // white
		surface( )->DrawSetTextPos( 5, 2 ); // x,y position
		surface( )->DrawPrintText( m_pText, wcslen( m_pText ) ); // print text
	}
	else
	{
		if( IsVisible( ) )
			SetVisible( false );
	}
	*/
}
