//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_scoretotal.cpp
//	@author Michael Parker (AfterShock)
//	@date 15/06/2007
//	@brief Hud Player Total Score field - with details of your total fortress points score
//
//	REVISIONS
//	---------
//	15/06/2007, AfterShock: 
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

static ConVar hud_fortpoints_total("hud_fortpoints_total", "1", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Toggle visible team scores on the HUD.");

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudPlayerTotalScore : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudPlayerTotalScore, vgui::FFPanel );

	CHudPlayerTotalScore( const char *pElementName ) : vgui::FFPanel( NULL, "HudPlayerTotalScore" ), CHudElement( pElementName )
	{
		SetParent( g_pClientMode->GetViewport() );
		SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );
	}

	virtual ~CHudPlayerTotalScore( void )
	{

	}

	virtual void Paint( void );
	virtual void Init( void );
	virtual void VidInit( void );
	void MsgFunc_SetPlayerTotalFortPoints( bf_read &msg );
	
protected:
	wchar_t		m_pTextTotalScore[ 1024 ];	// Unicode text buffer
	wchar_t		m_pTextTotalDesc[ 1024 ];	// Unicode text buffer

private:
	// Stuff we need to know	
		CPanelAnimationVar( vgui::HFont, m_hTotalScoreFont, "TotalScoreFont", "Default" );
		CPanelAnimationVar( vgui::HFont, m_hTotalDescFont, "TotalDescFont", "Default" );

		CPanelAnimationVar( vgui::HFont, m_hTotalScoreFontBG, "TotalScoreFontBG", "Default" );
		CPanelAnimationVar( vgui::HFont, m_hTotalDescFontBG, "TotalDescFontBG", "Default" );

	CPanelAnimationVarAliasType( float, TotalScoreFont_xpos, "TotalScoreFont_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, TotalScoreFont_ypos, "TotalScoreFont_ypos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, TotalDescFont_xpos, "TotalDescFont_xpos", "0", "proportional_float" );
	CPanelAnimationVarAliasType( float, TotalDescFont_ypos, "TotalDescFont_ypos", "0", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudPlayerTotalScore );
DECLARE_HUD_MESSAGE( CHudPlayerTotalScore, SetPlayerTotalFortPoints );

//-----------------------------------------------------------------------------
// Purpose: Done on loading game?
//-----------------------------------------------------------------------------
void CHudPlayerTotalScore::Init( void )
{
	HOOK_HUD_MESSAGE( CHudPlayerTotalScore, SetPlayerTotalFortPoints );

	m_pTextTotalScore[ 0 ] = '\0';
	m_pTextTotalDesc[ 0 ] = '\0';
}

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudPlayerTotalScore::VidInit( void )
{
	SetPaintBackgroundEnabled( false );
	m_pTextTotalScore[ 0 ] = '\0';
	m_pTextTotalDesc[ 0 ] = '\0';
	
}

void CHudPlayerTotalScore::MsgFunc_SetPlayerTotalFortPoints( bf_read &msg )
{
	// Read int and convert to string
	char szString3[ 1024 ];
	Q_snprintf( szString3, sizeof(szString3), "%i", msg.ReadLong() );

	char szString4[ 1024 ];
	Q_snprintf( szString4, sizeof(szString4), "Fortress Points" );

	// convert int-string to unicode
	vgui::localize()->ConvertANSIToUnicode( szString3, m_pTextTotalScore, sizeof( m_pTextTotalScore ) );
	vgui::localize()->ConvertANSIToUnicode( szString4, m_pTextTotalDesc, sizeof( m_pTextTotalDesc ) );
	
	// play animation (new points value)
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence( "NewTotalFortPoints" );
	
	//DevMsg( "[Location] Team: %i, String: %s\n", iTeam, szString );
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudPlayerTotalScore::Paint() 
{ 

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 
      if ( !pPlayer ) 
         return; 

	  if(!hud_fortpoints_total.GetBool())
		return;

   FFPanel::Paint(); // Draws the background glyphs 
      
		if( m_pTextTotalScore[ 0 ] != '\0' )
		{

			/*
			surface()->DrawSetTextFont( m_hTotalScoreFontBG );
			//surface()->DrawSetTextColor( Color(0,0,0,255) );
			surface()->DrawSetTextColor( Color(0,0,1,255) );
			surface()->DrawSetTextPos( TotalScoreFont_xpos, TotalScoreFont_ypos );

			for( wchar_t *wch = m_pTextTotalScore; *wch != 0; wch++ )
				surface()->DrawUnicodeChar( *wch );
	*/
			surface()->DrawSetTextFont( m_hTotalScoreFont );
			surface()->DrawSetTextColor( GetFgColor() );
			surface()->DrawSetTextPos( TotalScoreFont_xpos, TotalScoreFont_ypos );

			for( wchar_t *wch = m_pTextTotalScore; *wch != 0; wch++ )
				surface()->DrawUnicodeChar( *wch );

		}
		if( m_pTextTotalDesc[ 0 ] != '\0' )
		{
			/*
			surface()->DrawSetTextFont( m_hTotalDescFontBG );
			surface()->DrawSetTextColor( Color(0,0,0,255) );
			surface()->DrawSetTextPos( TotalDescFont_xpos, TotalDescFont_ypos );

			for( wchar_t *wch = m_pTextTotalDesc; *wch != 0; wch++ )
				surface()->DrawUnicodeChar( *wch );
*/
			surface()->DrawSetTextFont( m_hTotalDescFont );
			surface()->DrawSetTextColor( GetFgColor() );
			surface()->DrawSetTextPos( TotalDescFont_xpos, TotalDescFont_ypos );

			for( wchar_t *wch = m_pTextTotalDesc; *wch != 0; wch++ )
				surface()->DrawUnicodeChar( *wch );


		}
}
