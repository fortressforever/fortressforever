#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"

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

#define JETPACK_MAXFUEL 100

//-----------------------------------------------------------------------------
// Purpose: Displays jetpack fuel remaining on the HUD
//-----------------------------------------------------------------------------
class CHudJetpackFuelBar : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudJetpackFuelBar, vgui::FFPanel );

	CHudJetpackFuelBar( const char *pElementName ) : vgui::FFPanel( NULL, "HudJetpackFuelBar" ), CHudElement( pElementName )
	{
		SetParent( g_pClientMode->GetViewport() );
		SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED );
	}

	virtual ~CHudJetpackFuelBar( void )
	{
	}

	virtual void Paint( void );
	virtual void VidInit( void );

protected:

private:
	CPanelAnimationVar( vgui::HFont, m_hTextFont, "TextFont", "HUD_TextSmall" );

	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "34", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "10", "proportional_float" );

	CPanelAnimationVarAliasType( float, image1_xpos, "image1_xpos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, image1_ypos, "image1_ypos", "4", "proportional_float" );

	CPanelAnimationVar( Color, m_BarColor, "HUD_Tone_Default", "HUD_Tone_Default" );
	CPanelAnimationVarAliasType( float, bar_width, "bar_width", "75", "proportional_float" );
	CPanelAnimationVarAliasType( float, bar_height, "bar_height", "24", "proportional_float" );
};

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudJetpackFuelBar::VidInit( void )
{
	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudJetpackFuelBar::Paint( void )
{
	if( !engine->IsInGame() )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !pPlayer )
		return;

	if( pPlayer->GetClassSlot() != CLASS_PYRO || FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return;

	BaseClass::PaintBackground();

	float iProgressPercent = pPlayer->m_flJetpackFuel / 100.0f;
	surface()->DrawSetColor( m_BarColor );

	surface()->DrawFilledRect( image1_xpos, image1_ypos, image1_xpos + bar_width * iProgressPercent, image1_ypos + bar_height );
}

DECLARE_HUDELEMENT(CHudJetpackFuelBar);
