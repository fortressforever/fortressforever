//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======

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
#include "ff_weapon_electricknife.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudElectricKnifeChargeBar : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudElectricKnifeChargeBar, vgui::FFPanel );

	// Use the HudSpyDisguise setup from the hud scripts in /FortressForever/
	CHudElectricKnifeChargeBar( const char *pElementName ) : vgui::FFPanel( NULL, "HudSpyDisguise" ), CHudElement( pElementName )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
	}

	virtual ~CHudElectricKnifeChargeBar( void )
	{
		if( m_pHudChargeBarTexture )
		{
			delete m_pHudChargeBarTexture;
			m_pHudChargeBarTexture = NULL;
		}
	}

	virtual void Paint( void );
	virtual void VidInit( void );

protected:
	CHudTexture		*m_pHudChargeBarTexture;

private:
	// Stuff we need to know
	CPanelAnimationVarAliasType( float, text1_xpos, "text1_xpos", "34", "proportional_float" );
	CPanelAnimationVarAliasType( float, text1_ypos, "text1_ypos", "10", "proportional_float" );

	CPanelAnimationVarAliasType( float, image1_xpos, "image1_xpos", "2", "proportional_float" );
	CPanelAnimationVarAliasType( float, image1_ypos, "image1_ypos", "4", "proportional_float" );

	// For the disguising progress bar
	CPanelAnimationVar( Color, m_BarColor, "HUD_Tone_Default", "HUD_Tone_Default" );
	CPanelAnimationVar( Color, m_FadedBarColor, "196 196 196 196", "196 196 196 196" );
	CPanelAnimationVarAliasType( float, bar_width, "bar_width", "70", "proportional_float" );
	CPanelAnimationVarAliasType( float, bar_height, "bar_height", "24", "proportional_float" );
};

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudElectricKnifeChargeBar::VidInit( void )
{
	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudElectricKnifeChargeBar::Paint( void )
{
	if( !engine->IsInGame() )
		return;

	C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer();

	if( !pPlayer )
		return;

	if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		return;

	// Let's calculate and draw the disguising progress bar
	//if ( pPlayer->IsCloaked() )
	CFFWeaponElectricKnife *pKnife = (CFFWeaponElectricKnife *) pPlayer->GetFFWeapon(FF_WEAPON_ELECTRICKNIFE);
	if (!pKnife)
		return;

	float flProgressPercent = 0.0f;
	if (pKnife->IsElectrified())
	{	
		flProgressPercent = 1 - pKnife->GetElectrifyPercent();
	}
	else
	{
		flProgressPercent = pKnife->GetCooldownPercent();
	}
	
	// Paint foreground/background stuff
	BaseClass::PaintBackground();

	// Draw progress bar
	surface()->DrawSetColor( m_FadedBarColor );
	surface()->DrawFilledRect( image1_xpos, image1_ypos, image1_xpos + bar_width * flProgressPercent, image1_ypos + bar_height );
}

DECLARE_HUDELEMENT(CHudElectricKnifeChargeBar);
