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
	CPanelAnimationVar( vgui::HFont, m_hMapNameFont, "MapNameFont", "HUD_TextRoundInfo" );
	CPanelAnimationVar( Color, m_hMapNameColor, "MapNameColor", "HUD_Tone_Default" );
	CPanelAnimationVarAliasType( float, m_flMapNameX, "MapNameX", "32", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flMapNameY, "MapNameY", "3", "proportional_float" );

	CPanelAnimationVar( vgui::HFont, m_hTimerFont, "TimerFont", "HUD_TextRoundInfo" );
	CPanelAnimationVar( Color, m_hTimerColor, "TimerColor", "HUD_Tone_Default" );
	CPanelAnimationVarAliasType( float, m_flTimerX, "TimerX", "12", "proportional_float" );
	CPanelAnimationVarAliasType( float, m_flTimerY, "TimerY", "23", "proportional_float" );
	
	wchar_t		m_szMapName[ MAX_PATH ];
	wchar_t		m_szRoundTimer[ 8 ];
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
	surface()->GetTextSize( m_hMapNameFont, m_szMapName, iMapWide, iMapTall );

	m_flMapNameX = ( GetWide() / 2 ) - ( iMapWide / 2 );

	// Set up round timer
	localize()->ConvertANSIToUnicode( "00:00", m_szRoundTimer, sizeof( m_szRoundTimer ) );

	int iTimerWide, iTimerTall;
	surface()->GetTextSize( m_hTimerFont, m_szRoundTimer, iTimerWide, iTimerTall );

	m_flTimerX = ( GetWide() / 2 ) - ( iTimerWide / 2 );
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

	// Don't want to draw this over the class selection panel
	// so we'll wait until they select a class (unless guys
	// want it regardless)
	if( pPlayer->GetTeamNumber() == TEAM_UNASSIGNED || (!FF_HasPlayerPickedClass( pPlayer ) && !FF_IsPlayerSpec( pPlayer )) )
		return false;

	// Convert to minutes
	float flMinutesLeft = mp_timelimit.GetFloat() * 60;

	// Figure out new timer value
	char szTimer[ 8 ];
	int timer = flMinutesLeft - ( gpGlobals->curtime - FFGameRules()->GetRoundStart() );
	if( timer <= 0 )
		Q_snprintf( szTimer, sizeof( szTimer ), "00:00" );
	else
		Q_snprintf( szTimer, sizeof( szTimer ), "%d:%02d", ( timer / 60 ), ( timer % 60 ) );

	localize()->ConvertANSIToUnicode( szTimer, m_szRoundTimer, sizeof( m_szRoundTimer ) );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudRoundInfo::Paint( void )
{
	// Draw map text
	surface()->DrawSetTextFont( m_hMapNameFont );
	surface()->DrawSetTextColor( m_hMapNameColor );
	surface()->DrawSetTextPos( m_flMapNameX, m_flMapNameY );
	surface()->DrawUnicodeString( m_szMapName );

	// Draw round timer text
	surface()->DrawSetTextFont( m_hTimerFont );
	surface()->DrawSetTextColor( m_hTimerColor );
	surface()->DrawSetTextPos( m_flTimerX, m_flTimerY );
	surface()->DrawUnicodeString( m_szRoundTimer );

	BaseClass::Paint();
}
