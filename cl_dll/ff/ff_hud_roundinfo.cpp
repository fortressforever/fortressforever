//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_roundinfo.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 11/04/2006
//	@brief Hud Round Info
//
//	REVISIONS
//	---------
//	11/04/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

//#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>

#include "ff_panel.h"
#include "c_ff_player.h"
#include "ff_utils.h"
#include "ff_gamerules.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

extern ConVar mp_timelimit;

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudRoundInfo : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudRoundInfo, vgui::FFPanel );

	CHudRoundInfo( const char *pElementName ) : vgui::FFPanel( NULL, "HudRoundInfo" ), CHudElement( pElementName )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		// SetHiddenBits( HIDEHUD_PLAYERDEAD );
	}

	virtual ~CHudRoundInfo( void )
	{
	}

	virtual bool ShouldDraw( void );
	virtual void Paint( void );
	virtual void VidInit( void );

private:
	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HUD_TextRoundInfo" );
	CPanelAnimationVar( vgui::HFont, m_hNumFont, "NumberFont", "HUD_TextRoundInfo" );

	CPanelAnimationVarAliasType( Color, m_hTextColor, "TextColor", "255 255 255", "HUD_Tone_Default" );
	CPanelAnimationVarAliasType( Color, m_hNumColor, "NumberColor", "255 255 255", "HUD_Tone_Default" );

	/*
	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "64", "proportional_float" );

	CPanelAnimationVarAliasType( float, image1_xpos, "image1_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, image1_ypos, "image1_ypos", "0", "proportional_float" );
	*/

	wchar_t		m_szMapName[ MAX_PATH ];
	int			m_iMapNameX;
	int			m_iMapNameY;

	wchar_t		m_szRoundTimer[ 8 ];
	int			m_iRoundTimerX;
	int			m_iRoundTimerY;
};

DECLARE_HUDELEMENT( CHudRoundInfo );

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudRoundInfo::VidInit( void )
{
	// Set up map name
	localize()->ConvertANSIToUnicode( UTIL_GetFormattedMapName(), m_szMapName, sizeof( m_szMapName ) );

	int iMapWide, iMapTall;
	surface()->GetTextSize( m_hTextFont, m_szMapName, iMapWide, iMapTall );

	m_iMapNameX = ( GetWide() / 2 ) - ( iMapWide / 2 );
	m_iMapNameY = 5;

	// Set up round timer
	localize()->ConvertANSIToUnicode( "00:00", m_szRoundTimer, sizeof( m_szRoundTimer ) );
	
	int iRoundWide, iRoundTall;
	surface()->GetTextSize( m_hNumFont, m_szRoundTimer, iRoundWide, iRoundTall );

	m_iRoundTimerX = ( GetWide() / 2 ) - ( iRoundWide / 2 );
	m_iRoundTimerY = m_iMapNameY + iMapTall + 5;
}

//-----------------------------------------------------------------------------
// Purpose: To draw or not to draw
//-----------------------------------------------------------------------------
bool CHudRoundInfo::ShouldDraw( void )
{
	if( !engine->IsInGame() )
		return false;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();
	if( !pPlayer )
		return false;

	// Specs have their own timer and stuff
	if( FF_IsPlayerSpec( pPlayer ) )
		return false;

	// Don't want to draw this over the class selection panel
	// so we'll wait until they select a class (unless guys
	// want it regardless)
	if( !FF_HasPlayerPickedClass( pPlayer ) )
		return false;

	// Convert to minutes
	float flMinutesLeft = mp_timelimit.GetFloat() * 60;

	// Figure out new timer value
	int timer = flMinutesLeft - ( gpGlobals->curtime - FFGameRules()->GetRoundStart() );
	if( timer <= 0 )
		_snwprintf( m_szRoundTimer, sizeof( m_szRoundTimer ), L"00:00" );
	else
		_snwprintf( m_szRoundTimer, sizeof( m_szRoundTimer ), L"%d:%02d", ( timer / 60 ), ( timer % 60 ) );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudRoundInfo::Paint( void )
{
	// Draw map text
	surface()->DrawSetTextFont( m_hTextFont );
	surface()->DrawSetTextColor( GetFgColor() );
	surface()->DrawSetTextPos( m_iMapNameX, m_iMapNameY );
	surface()->DrawUnicodeString( m_szMapName );

	// Draw round timer text
	surface()->DrawSetTextFont( m_hNumFont );
	surface()->DrawSetTextColor( GetFgColor() );
	surface()->DrawSetTextPos( m_iRoundTimerX, m_iRoundTimerY );
	surface()->DrawUnicodeString( m_szRoundTimer );

	BaseClass::Paint();
}
