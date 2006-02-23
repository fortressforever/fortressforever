//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_location.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 02/20/2006
//	@brief client side Hud Location Info
//
//	REVISIONS
//	---------
//	02/20/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include "view.h"

using namespace vgui;

#include <vgui_controls/Panel.h>
#include <vgui_controls/Frame.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
#include <vgui/IVGui.h>

//#include "debugoverlay_shared.h"

#include "c_ff_player.h"
//#include "c_ff_buildableobjects.h"
//#include <igameresources.h>
//#include "c_ff_team.h"
//#include "ff_gamerules.h"
//#include "ff_utils.h"

//=============================================================================
//
//	class CHudLocation
//
//=============================================================================
class CHudLocation : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE( CHudLocation, vgui::Panel );

public:
	CHudLocation( const char *pElementName ) : CHudElement( pElementName ), vgui::Panel( NULL, "HudLocation" )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
	}

	~CHudLocation( void ) {}

	void Init( void );
	void VidInit( void );
	void Paint( void );

	void MsgFunc_SetPlayerLocation( bf_read &msg );

protected:
	wchar_t		m_pText[ 1024 ];	// Unicode text buffer
	int			m_iTeam;

private:

	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "Default" );

	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "8", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "20", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudLocation );
DECLARE_HUD_MESSAGE( CHudLocation, SetPlayerLocation );

void CHudLocation::Init( void )
{
	HOOK_HUD_MESSAGE( CHudLocation, SetPlayerLocation );

	m_pText[ 0 ] = '\0';
}

void CHudLocation::VidInit( void )
{	
	SetPaintBackgroundEnabled( false );
	m_pText[ 0 ] = '\0'; // Bug 0000293: clear location text buffer on map change
}

void CHudLocation::MsgFunc_SetPlayerLocation( bf_read &msg )
{
	char szString[ 1024 ];
	msg.ReadString( szString, sizeof( szString ) );

	m_iTeam = msg.ReadShort();

	wchar_t *pszTemp = vgui::localize()->Find( szString );
	if( pszTemp )
		wcscpy( m_pText, pszTemp );
	else
		vgui::localize()->ConvertANSIToUnicode( szString, m_pText, sizeof( m_pText ) );

	//DevMsg( "[Location] Team: %i, String: %s\n", iTeam, szString );
}

void CHudLocation::Paint( void )
{
	if( m_pText )
	{
		surface()->DrawSetTextFont( m_hTextFont );

		switch( m_iTeam )
		{
			case 1: surface()->DrawSetTextColor( 169, 169, 169, 255 ); break;
			case 2: surface()->DrawSetTextColor( 0, 0, 255, 255 ); break;
			case 3: surface()->DrawSetTextColor( 255, 0, 0, 255 ); break;
			case 4: surface()->DrawSetTextColor( 255, 255, 0, 255 ); break;
			case 5: surface()->DrawSetTextColor( 0, 255, 0, 255 ); break;
		}

		surface()->DrawSetTextPos( text1_xpos, text1_ypos );

		for( wchar_t *wch = m_pText; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );
	}
}
