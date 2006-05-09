//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"
#include "ammodef.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include "c_ff_player.h"
#include "ff_weapon_base.h"

#include <vgui/ILocalize.h>
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudAmmo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE( CHudAmmo, CHudNumericDisplay );

public:
	CHudAmmo( const char *pElementName );
	void Init( void );
	void VidInit( void );
	void Reset();

	void SetAmmo(int ammo, bool playAnimation);
	void SetAmmo2(int ammo2, bool playAnimation);

	virtual void Paint();

protected:
	virtual void OnThink();

	void UpdateAmmoDisplays();
	void UpdatePlayerAmmo( C_BasePlayer *player );

private:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	CHandle< C_BaseEntity > m_hCurrentVehicle;
	int		m_iAmmo;
	int		m_iAmmo2;
	int		m_iAmmoType;

	CHudTexture	*m_pHudElementTexture;
	CHudTexture	*m_pHudAmmoTypes[MAX_AMMO_TYPES];
	const char *m_pszHudAmmoNames[MAX_AMMO_TYPES];
};

DECLARE_HUDELEMENT( CHudAmmo );

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAmmo::CHudAmmo( const char *pElementName ) : BaseClass(NULL, "HudAmmo"), CHudElement( pElementName )
{
	SetHiddenBits( /*HIDEHUD_HEALTH | */HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::Init( void )
{
	m_iAmmo		= -1;
	m_iAmmo2	= -1;
	m_iAmmoType = 0;

	/*
	// REMOVED TEMPORARILY - ted
	wchar_t *tempString = vgui::localize()->Find("#FF_HUD_NOAMMOTYPE");
	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"NONE");
	}
	*/
	SetLabelText(L"");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::VidInit( void )
{
	// Precache the background texture
	m_pHudElementTexture = new CHudTexture();
	m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_box_ammo", true, false);

	// Get icon for each graphic
	for (int i = 0; i < MAX_AMMO_TYPES; i++)
	{
		Ammo_t *ammo = GetAmmoDef()->GetAmmoOfIndex(i);

		if (ammo && ammo->pName)
		{
			char buf[128];
			sprintf(buf, "vgui/hud_%s", ammo->pName);

			// I'm wary of doing it this way, might cause problems in the future
			m_pszHudAmmoNames[i] = ammo->pName;

			m_pHudAmmoTypes[i] = new CHudTexture();
			m_pHudAmmoTypes[i]->textureId = surface()->CreateNewTextureID();
			surface()->DrawSetTextureFile(m_pHudAmmoTypes[i]->textureId, buf, true, false);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudAmmo::Reset()
{
	BaseClass::Reset();

	m_hCurrentActiveWeapon = NULL;
	m_hCurrentVehicle = NULL;
	m_iAmmo = 0;
	m_iAmmo2 = 0;
	m_iAmmoType = 0;

	UpdateAmmoDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get ammo info from the weapon
//-----------------------------------------------------------------------------
void CHudAmmo::UpdatePlayerAmmo( C_BasePlayer *player )
{
	// Clear out the vehicle entity
	m_hCurrentVehicle = NULL;

	C_BaseCombatWeapon *wpn = GetActiveWeapon();
	if ( !wpn || !player || !wpn->UsesPrimaryAmmo() )
	{
		//SetPaintEnabled(false);
		// REMOVED - need to keep ammo panel up for melee weapons because grenade display is now part of it - ted
		SetShouldDisplayValue(false);
		SetPaintBackgroundEnabled(true);
		return;
	}
	else
	{
		SetShouldDisplayValue(true);
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	// get the ammo in our clip
	int ammo1 = wpn->Clip1();
	int ammo2;
	if (ammo1 < 0)
	{
		// we don't use clip ammo, just use the total ammo count
		ammo1 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
		ammo2 = 0;
	}
	else
	{
		// we use clip ammo, so the second ammo is the total ammo
		ammo2 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
	}

	if (wpn == m_hCurrentActiveWeapon)
	{
		// same weapon, just update counts
		SetAmmo(ammo1, true);
		SetAmmo2(ammo2, true);
	}
	else
	{
		// diferent weapon, change without triggering
		SetAmmo(ammo1, false);
		SetAmmo2(ammo2, false);

		// update icon
		m_iAmmoType = wpn->GetPrimaryAmmoType();

		// ammo localised text is based on the weapon's localised printname
		char buf[128];
		sprintf(buf, "%s_AMMO", wpn->GetPrintName());

		// update name
		// REMOVED TEMPORARILY - ted
		/*
		wchar_t *tempString = vgui::localize()->Find(buf);
		if (tempString)
		{
			SetLabelText(tempString);
		}
		else
		{
			SetLabelText(L"AMMO");
		}
		*/

		// update whether or not we show the total ammo display
		if (wpn->UsesClipsForAmmo1())
		{
			SetShouldDisplaySecondaryValue(true);
			//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesClips");
		}
		else
		{
			//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponDoesNotUseClips");
			SetShouldDisplaySecondaryValue(false);
		}

		//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponChanged");
		m_hCurrentActiveWeapon = wpn;
	}
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get ammo info from the weapon
//-----------------------------------------------------------------------------
void CHudAmmo::OnThink()
{
	UpdateAmmoDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: updates the ammo display counts
//-----------------------------------------------------------------------------
void CHudAmmo::UpdateAmmoDisplays()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	
	UpdatePlayerAmmo( player );
}

//-----------------------------------------------------------------------------
// Purpose: Updates ammo display
//-----------------------------------------------------------------------------
void CHudAmmo::SetAmmo(int ammo, bool playAnimation)
{
	if (ammo != m_iAmmo)
	{
		if (ammo == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoEmpty");
		}
		else if (ammo < m_iAmmo)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoIncreased");
		}

		m_iAmmo = ammo;
	}

	SetDisplayValue(ammo);
}

//-----------------------------------------------------------------------------
// Purpose: Updates 2nd ammo display
//-----------------------------------------------------------------------------
void CHudAmmo::SetAmmo2(int ammo2, bool playAnimation)
{
	if (ammo2 != m_iAmmo2)
	{
		if (ammo2 == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Empty");
		}
		else if (ammo2 < m_iAmmo2)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Decreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Increased");
		}

		m_iAmmo2 = ammo2;
	}

	SetSecondaryValue(ammo2);
}

void CHudAmmo::Paint()
{
	if( C_BasePlayer::GetLocalPlayer()->GetTeamNumber() < TEAM_BLUE )
		return;

	// Draw background box
	surface()->DrawSetTexture(m_pHudElementTexture->textureId);
	surface()->DrawSetColor(255, 255, 255, 255);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());

	// Use the weapon ammo icon if possible
	CHudTexture *ammoIcon = gWR.GetAmmoIconFromWeapon(m_iAmmoType);

	// Use a generic one instead (unlikely this'll happen)
	if (!ammoIcon)
		ammoIcon = m_pHudAmmoTypes[m_iAmmoType];

	// Bug #0000391: Immediately entering specmode after map load crashes FF
	if (!ammoIcon)
		return;

	// Draw ammo icon
	surface()->DrawSetTexture(ammoIcon->textureId);
	surface()->DrawSetColor(255, 255, 255, 255);
	surface()->DrawTexturedRect(icon_xpos, icon_ypos, icon_xpos + icon_width, icon_ypos + icon_height);

	BaseClass::Paint();
}