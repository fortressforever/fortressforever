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
#include <igameresources.h>

#include "c_ff_player.h"
#include "ff_weapon_base.h"
#include "ff_hud_boxes.h"
#include "ff_utils.h"

#include <vgui/ILocalize.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudAmmo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudAmmo, CHudNumericDisplay);

public:
	
	CHudAmmo(const char *pElementName);
	
	virtual void Init();
	virtual void Reset();
	virtual void OnThink();

protected:

	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;

	virtual void SetAmmo(int ammo, bool playAnimation);
	virtual int GetPlayerAmmo(C_FFPlayer *pPlayer, C_BaseCombatWeapon *pWeapon);

	int		m_iAmmo;
};

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level in clip
//-----------------------------------------------------------------------------
class CHudAmmoClip : public CHudAmmo
{
	DECLARE_CLASS_SIMPLE(CHudAmmoClip, CHudAmmo);

public:

	CHudAmmoClip(const char *pElementName) : BaseClass(pElementName)
	{
		// Hopefully not too late to do this
		SetName("HudAmmoClip");
	}

	virtual int GetPlayerAmmo(C_FFPlayer *pPlayer, C_BaseCombatWeapon *pWeapon)
	{
		if (!pWeapon)
			return -1;

		return pWeapon->Clip1();
	}
};

//-----------------------------------------------------------------------------
// Purpose: Displays current weapon & ammo
//-----------------------------------------------------------------------------
class CHudAmmoInfo : public CHudElement, public vgui::FFPanel
{
public:
	CHudAmmoInfo(const char *pElementName) : CHudElement(pElementName), vgui::FFPanel(NULL, "HudAmmoInfo")
	{
		// Set our parent window
		SetParent(g_pClientMode->GetViewport());

		// Hide when player is dead
		SetHiddenBits(HIDEHUD_PLAYERDEAD);
	}

	virtual void Paint()
	{
		// Various things here
		// pWeapon->GetAmmoIcon
		// pWeapon->GetWeaponIcon

		FFPanel::Paint();
	}
};

//-----------------------------------------------------------------------------
// Purpose: A copy/paste so that glyphs draw correctly
//-----------------------------------------------------------------------------
class CHudAmmoInfo2 : public CHudElement, public vgui::FFPanel
{
public:
	CHudAmmoInfo2(const char *pElementName) : CHudElement(pElementName), vgui::FFPanel(NULL, "HudAmmoInfo2")
	{
		// Set our parent window
		SetParent(g_pClientMode->GetViewport());

		// Hide when player is dead
		SetHiddenBits(HIDEHUD_PLAYERDEAD);
	}

	virtual void Paint()
	{
		// Various things here
		// pWeapon->GetAmmoIcon
		// pWeapon->GetWeaponIcon

		FFPanel::Paint();
	}
};

DECLARE_HUDELEMENT(CHudAmmo);
DECLARE_HUDELEMENT(CHudAmmoClip);
DECLARE_HUDELEMENT(CHudAmmoInfo);
DECLARE_HUDELEMENT(CHudAmmoInfo2);

CHudAmmo::CHudAmmo(const char *pElementName) : BaseClass(NULL, "HudAmmo"), CHudElement(pElementName)
{
	SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
}

void CHudAmmo::Init()
{
	m_iAmmo = -1;
	SetLabelText(L"");
}

void CHudAmmo::Reset()
{
	m_iAmmo = 0;
}

int CHudAmmo::GetPlayerAmmo(C_FFPlayer *pPlayer, C_BaseCombatWeapon *pWeapon)
{
	if (!pPlayer || !pWeapon)
		return -1;

	return pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType());
}

void CHudAmmo::OnThink()
{
	// Fix for ToFFPlayer( NULL ) being called.
	if( !engine->IsInGame() )
		return;

	// A ha! One of the ToFFPlayer( NULL ) calls!
	C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	if (!pPlayer)
		return;

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();

	int iAmmo = -1;

	if (pWeapon && pWeapon->UsesPrimaryAmmo())
	{
		iAmmo = GetPlayerAmmo(pPlayer, pWeapon);
	}

	if (iAmmo < 0 || FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
	{
		SetPaintEnabled(false);
		SetShouldDisplayValue(false);
		SetPaintBackgroundEnabled(false);

		return;
	}
	else
	{
		SetPaintEnabled(true);
		SetShouldDisplayValue(true);
		SetPaintBackgroundEnabled(true);
	}

	if (pWeapon == m_hCurrentActiveWeapon)
	{
		// Same weapon, update w/ animations
		SetAmmo(iAmmo, true);
	}
	else
	{
		// Different weapon, update w/o animations
		SetAmmo(iAmmo, false);
	}

	m_hCurrentActiveWeapon = pWeapon;
}

void CHudAmmo::SetAmmo(int iAmmo, bool bPlayAnimation)
{
	if (bPlayAnimation && m_iAmmo != iAmmo)
	{
		//const char *pszAnimation = (iAmmo == 0 ? "AmmoEmpty" : (iAmmo < 0 ? "AmmoDecreased" : "AmmoIncreased"));
		
		// Mulch: Disabling all hud animations except for health/armor
		//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence(pszAnimation);
	}

	m_iAmmo = iAmmo;
	SetDisplayValue(iAmmo);
}