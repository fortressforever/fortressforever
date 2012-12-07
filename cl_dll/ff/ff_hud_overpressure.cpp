//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_spydisguise.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 09/01/2006
//	@brief Hud Disguise Indicator
//
//	REVISIONS
//	---------
//	09/01/2006, Mulchman: 
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
#include "c_playerresource.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//extern ConVar ffdev_overpressure_delay;
#define OVERPRESSURE_COOLDOWN 16	//ffdev_overpressure_delay.GetFloat()

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudOverpressure : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudOverpressure, vgui::FFPanel );

	CHudOverpressure( const char *pElementName ) : vgui::FFPanel( NULL, "HudOverpressure" ), CHudElement( pElementName )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
	}

	virtual ~CHudOverpressure( void )
	{
	}

	virtual void Paint( void );
	virtual void VidInit( void );

protected:

private:
	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HUD_TextSmall" );

	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "34", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "10", "proportional_float" );

	CPanelAnimationVarAliasType( float, image1_xpos, "image1_xpos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, image1_ypos, "image1_ypos", "4", "proportional_float" );

	// For the disguising progress bar
	CPanelAnimationVar( Color, m_BarColor, "HUD_Tone_Default", "HUD_Tone_Default" );
	CPanelAnimationVarAliasType( float, bar_width, "bar_width", "75", "proportional_float" );
	CPanelAnimationVarAliasType( float, bar_height, "bar_height", "24", "proportional_float" );
};

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudOverpressure::VidInit( void )
{
	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudOverpressure::Paint( void )
{
	if( !engine->IsInGame() )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !pPlayer )
		return;

	if( pPlayer->GetClassSlot() != CLASS_HWGUY || FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return;

	// Let's calculate and draw the disguising progress bar
	if ( pPlayer->m_flNextClassSpecificSkill > gpGlobals->curtime )
	{	
		//New cloak percent timer -GreenMushy
		float iProgressPercent = ( OVERPRESSURE_COOLDOWN - (pPlayer->m_flNextClassSpecificSkill - gpGlobals->curtime) ) / ( OVERPRESSURE_COOLDOWN );
	
		// Paint foreground/background stuff
		BaseClass::PaintBackground();

		// Draw progress bar
		surface()->DrawSetColor( m_BarColor );
		surface()->DrawFilledRect( image1_xpos, image1_ypos, image1_xpos + bar_width * iProgressPercent, image1_ypos + bar_height );
	}
}

DECLARE_HUDELEMENT(CHudOverpressure);
