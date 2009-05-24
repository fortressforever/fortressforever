//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_weaponinfo.cpp
//	@author Michael Parker (AfterShock)
//	@date 27/05/2007
//	@brief Hud Weapon Indicator
//
//	REVISIONS
//	---------
//	27/05/2007, AfterShock: 
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

/*
ConVar cl_box1( 
	"cl_box1", 
	"0", 
	FCVAR_REPLICATED, 
	"sds" );
ConVar cl_box2( 
	"cl_box2", 
	"0", 
	FCVAR_REPLICATED, 
	"sds" );
*/
ConVar cl_box3( 
	"cl_box3", 
	"100", 
	FCVAR_REPLICATED, 
	"sds" );
ConVar cl_box4( 
	"cl_box4", 
	"100", 
	FCVAR_REPLICATED, 
	"sds" );


using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudWeaponInfo : public CHudElement, public vgui::FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudWeaponInfo, vgui::FFPanel );

	CHudWeaponInfo( const char *pElementName ) : vgui::FFPanel( NULL, "HudWeaponInfo" ), CHudElement( pElementName )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits( HIDEHUD_PLAYERDEAD );
	}

	virtual ~CHudWeaponInfo( void )
	{
		if( m_pWeaponIcon )
		{
			delete m_pWeaponIcon;
			m_pWeaponIcon = NULL;
		}
		if( m_pAmmoIcon )
		{
			delete m_pAmmoIcon;
			m_pAmmoIcon = NULL;
		}
	}

	virtual void Paint( void );
	virtual void VidInit( void );
	virtual bool ShouldDraw( void );

protected:
	CHudTexture		*m_pWeaponIcon;
	CHudTexture		*m_pAmmoIcon;

private:
	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hIconFont, "IconFont", "WeaponIconsHUD" );

};

DECLARE_HUDELEMENT( CHudWeaponInfo );

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudWeaponInfo::VidInit( void )
{
		
	m_pWeaponIcon = new CHudTexture;
	m_pAmmoIcon = new CHudTexture;
	SetPaintBackgroundEnabled( false );
}

//-----------------------------------------------------------------------------
// Purpose: Should we draw? (Are we ingame? have we picked a class, etc)
//-----------------------------------------------------------------------------
bool CHudWeaponInfo::ShouldDraw() 
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

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudWeaponInfo::Paint() 
{ 
   FFPanel::Paint(); // Draws the background glyphs 

   if( m_pWeaponIcon && m_pAmmoIcon) 
   { 
      C_FFPlayer *pPlayer = C_FFPlayer::GetLocalFFPlayer(); 
      if ( !pPlayer ) 
         return; 

		C_BaseCombatWeapon *pSelectedWeapon = pPlayer->GetActiveWeapon();
		if (!pSelectedWeapon)
			return;

	  if (pSelectedWeapon)
	  {
			Color col = GetFgColor();

			// Shallow copy of the weapon scrolling icon
			*m_pWeaponIcon = *pSelectedWeapon->GetSpriteInactive();
			// Change the font so it uses 28 size instead of 64
			m_pWeaponIcon->hFont = m_hIconFont;
			m_pWeaponIcon->bRenderUsingFont = true;
			SetPaintBackgroundEnabled( false );

			// Draw itself in the bottom right corner
			//m_pWeaponIcon->DrawSelf(cl_box1.GetInt(), cl_box2.GetInt(), col);
			// for widescreen stuff we take width scaled, then subtract the X not scaled (as we dont stretch the hud)
			// then we add the 44 not scaled (GetProportionalScaledValue is scaled due to height but not width)

			m_pWeaponIcon->DrawSelf( scheme()->GetProportionalScaledValue(44) , 0, col);
	//DevMsg( "wide: %i; tall: %i" , screenWide , screenTall );

			// Shallow copy of the ammo icon
			*m_pAmmoIcon = *pSelectedWeapon->GetSpriteAmmo();

			// Draw itself in the bottom right corner
			// *** commented until we find a place to fit the ammo icon! - AfterShock
			m_pAmmoIcon->DrawSelf(cl_box3.GetInt(), cl_box4.GetInt(), col);

						
					

		// Draw the icon -- yeah, it's not actually a weapon icon – it’s the hint lightbulb 
		//m_pWeaponIcon->DrawSelf( 0, 0, clr ); // Draws it in the top left corner of the panel 
	  }
	} 
}
