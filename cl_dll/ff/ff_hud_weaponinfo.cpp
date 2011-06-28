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

#include <vgui/ISurface.h>
#include <vgui/IVGUI.h>

#include "ff_panel.h"
#include "c_ff_player.h"
#include "iclientmode.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current disguised class
//-----------------------------------------------------------------------------
class CHudWeaponInfo : public CHudElement, public FFPanel
{
public:
	DECLARE_CLASS_SIMPLE( CHudWeaponInfo, FFPanel );

	CHudWeaponInfo( const char *pElementName ) : BaseClass( NULL, "HudWeaponInfo" ), CHudElement( pElementName )
	{
		// Set our parent window
		SetParent( g_pClientMode->GetViewport() );

		// Hide when player is dead
		SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
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
	virtual void Init( void );
	virtual void VidInit( void );
	virtual void OnTick( void );

protected:
	CHudTexture		*m_pWeaponIcon;
	CHudTexture		*m_pAmmoIcon;

private:
	// Stuff we need to know
	CPanelAnimationVar( vgui::HFont, m_hIconFont, "IconFont", "WeaponIconsHUD" );
	CPanelAnimationVar( vgui::HFont, m_hAmmoIconFont, "AmmoFont", "WeaponIconsHUD" );

	CPanelAnimationVarAliasType( float, ammo_xpos, "ammo_xpos", "10", "proportional_float" );
	CPanelAnimationVarAliasType( float, ammo_ypos, "ammo_ypos", "32", "proportional_float" );
};

DECLARE_HUDELEMENT( CHudWeaponInfo );

//-----------------------------------------------------------------------------
// Purpose: Done each map load
//-----------------------------------------------------------------------------
void CHudWeaponInfo::Init( void )
{
	ivgui()->AddTickSignal( GetVPanel(), 100 );
}


void CHudWeaponInfo::VidInit( void )
{
	m_pWeaponIcon = new CHudTexture();
	m_pAmmoIcon = new CHudTexture();
}

//-----------------------------------------------------------------------------
// Purpose: Get stuff!
//-----------------------------------------------------------------------------
void CHudWeaponInfo::OnTick() 
{
	BaseClass::OnTick();

	if(!m_pFFPlayer)
		return;
	
	C_BaseCombatWeapon *pSelectedWeapon = m_pFFPlayer->GetActiveWeapon();

	if (!pSelectedWeapon)
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
	}
	else
	{
		*m_pWeaponIcon = *pSelectedWeapon->GetSpriteInactive();
		*m_pAmmoIcon = *pSelectedWeapon->GetSpriteAmmo();
		SetPaintEnabled(true);
		SetPaintBackgroundEnabled(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Draw stuff!
//-----------------------------------------------------------------------------
void CHudWeaponInfo::Paint() 
{ 
	if(m_pWeaponIcon)
	{
		// Shallow copy of the weapon scrolling icon
		// Change the font so it uses 28 size instead of 64
		m_pWeaponIcon->hFont = m_hIconFont;
		m_pWeaponIcon->bRenderUsingFont = true;

		// Draw itself in the bottom right corner
		//m_pWeaponIcon->DrawSelf(cl_box1.GetInt(), cl_box2.GetInt(), col);
		// for widescreen stuff we take width scaled, then subtract the X not scaled (as we dont stretch the hud)
		// then we add the 44 not scaled (GetProportionalScaledValue is scaled due to height but not width)

		m_pWeaponIcon->DrawSelf( scheme()->GetProportionalScaledValue(44) , 0, GetFgColor());
	}

	if(m_pAmmoIcon)
	{
		// Shallow copy of the ammo icon
		m_pAmmoIcon->hFont = m_hAmmoIconFont;

		// Draw itself in the bottom right corner
		m_pAmmoIcon->DrawSelf(ammo_xpos, ammo_ypos, GetFgColor());
	}
}
