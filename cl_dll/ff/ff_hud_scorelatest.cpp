

//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_scorelatest.cpp
//	@author Michael Parker (AfterShock)
//	@date 30/06/2007
//	@brief Hud Player Score field - with details of your latest score
//
//	REVISIONS
//	---------
//	30/06/2007, AfterShock: 
//		First created (from ff_hud_spydisguise.cpp)

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
#include "c_playerresource.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;
static ConVar hud_fortpoints_latest("hud_fortpoints_latest", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Toggle visible team scores on the HUD.");

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudPlayerLatestScore : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudPlayerLatestScore, vgui::FFPanel );

	CHudPlayerLatestScore( const char *pElementName ) : vgui::FFPanel( NULL, "HudPlayerLatestScore" ), CHudElement( pElementName )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
	}

	virtual ~CHudPlayerLatestScore( void )
	{

	}

	virtual void Paint( void );
	virtual void Init( void );
	virtual void VidInit( void );
	virtual bool ShouldDraw( void );
	void MsgFunc_SetPlayerLatestFortPoints( bf_read &msg );

protected:
	wchar_t		m_pTextScore[ 1024 ];	// Unicode text buffer
	wchar_t		m_pTextDesc[ 1024 ];	// Unicode text buffer
	int			m_iLatestFortPoints;

private:
	// Stuff we need to know	
	CPanelAnimationVar( vgui::HFont, m_hScoreFont, "ScoreFont", "Default" );
	CPanelAnimationVar( vgui::HFont, m_hDescFont, "DescFont", "Default" );

	CPanelAnimationVar( vgui::HFont, m_hScoreFontBG, "ScoreFontBG", "Default" );
	CPanelAnimationVar( vgui::HFont, m_hDescFontBG, "DescFontBG", "Default" );

	CPanelAnimationVarAliasType( float, ScoreFont_xpos, "ScoreFont_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, ScoreFont_ypos, "ScoreFont_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, DescFont_xpos, "DescFont_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, DescFont_ypos, "DescFont_ypos", "0", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudPlayerLatestScore );
DECLARE_HUD_MESSAGE( CHudPlayerLatestScore, SetPlayerLatestFortPoints );

//-----------------------------------------------------------------------------
// Purpose: Done on loading game?
//-----------------------------------------------------------------------------
void CHudPlayerLatestScore::Init( void )
{
	HOOK_HUD_MESSAGE( CHudPlayerLatestScore, SetPlayerLatestFortPoints );

	m_pTextScore[ 0 ] = '\0';
	m_pTextDesc[ 0 ] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudPlayerLatestScore::VidInit( void )
{
	SetPaintBackgroundEnabled( false );
	m_pTextScore[ 0 ] = '\0'; 
	m_pTextDesc[ 0 ] = '\0';

}

//-----------------------------------------------------------------------------
// Purpose: Should we draw? (Are we ingame? have we picked a class, etc)
//-----------------------------------------------------------------------------
bool CHudPlayerLatestScore::ShouldDraw() 
{ 
	if( !engine->IsInGame() ) 
		return false; 

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 

	if( !pPlayer ) 
		return false; 

	if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) ) 
		return false; 

	return true; 
} 

void CHudPlayerLatestScore::MsgFunc_SetPlayerLatestFortPoints( bf_read &msg )
{
	// Read description string
	char szString[ 1024 ];
	msg.ReadString( szString, sizeof( szString ) );

	// Read int and convert to string
	const int ptVal = msg.ReadShort();
	if(ptVal==0)
		return;

	char szString2[ 1024 ];
	Q_snprintf( szString2, sizeof(szString2), "%s%i", ptVal>0?"+":"",ptVal );

	// Convert string to unicode
	wchar_t *pszTemp = vgui::localize()->Find( szString );
	if( pszTemp )
		wcscpy( m_pTextDesc, pszTemp );
	else
		vgui::localize()->ConvertANSIToUnicode( szString, m_pTextDesc, sizeof( m_pTextDesc ) );

	// convert int-string to unicode
	vgui::localize()->ConvertANSIToUnicode( szString2, m_pTextScore, sizeof( m_pTextScore ) );

	// play animation (new points value)
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewLatestFortPoints" );
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudPlayerLatestScore::Paint() 
{
	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 
	if ( !pPlayer ) 
		return; 

	if(!hud_fortpoints_latest.GetBool())
		return;

	FFPanel::Paint(); // Draws the background glyphs 

	if( m_pTextDesc[ 0 ] != '\0' )
	{
		/*
		surface()->DrawSetTextFont( m_hDescFontBG );
		surface()->DrawSetTextColor( Color(0,0,0,255) );
		surface()->DrawSetTextPos( DescFont_xpos, DescFont_ypos );

		for( wchar_t *wch = m_pTextDesc; *wch != 0; wch++ )
		surface()->DrawUnicodeChar( *wch );
		*/

		surface()->DrawSetTextFont( m_hDescFont );
		surface()->DrawSetTextColor( GetFgColor() );
		surface()->DrawSetTextPos( DescFont_xpos, DescFont_ypos );

		for( wchar_t *wch = m_pTextDesc; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );


	}

	if( m_pTextScore[ 0 ] != '\0' )
	{
		/*
		surface()->DrawSetTextFont( m_hScoreFontBG );
		surface()->DrawSetTextColor( Color(0,0,0,255) );
		surface()->DrawSetTextPos( ScoreFont_xpos, ScoreFont_ypos );

		for( wchar_t *wch = m_pTextScore; *wch != 0; wch++ )
		surface()->DrawUnicodeChar( *wch );
		*/
		surface()->DrawSetTextFont( m_hScoreFont );
		surface()->DrawSetTextColor( GetFgColor() );
		surface()->DrawSetTextPos( ScoreFont_xpos, ScoreFont_ypos );

		for( wchar_t *wch = m_pTextScore; *wch != 0; wch++ )
			surface()->DrawUnicodeChar( *wch );

	}
}
