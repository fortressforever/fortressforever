/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   1:40
	filename: 	f:\cvs\code\cl_dll\ff\ff_hud_grenade2.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ff_hud_grenade2
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"

#include "iclientmode.h"
#include "c_ff_player.h"

#include <KeyValues.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudGrenade2 : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudGrenade2, CHudNumericDisplay);

public:
	CHudGrenade2(const char *pElementName);
	void Init();
	void Reset();

	void SetGrenade(int Grenade, bool playAnimation);
	virtual void Paint();

protected:
	virtual void OnTick();

private:
	int		m_iGrenade;
	int		m_iClass;

	CHudTexture *iconTexture;
};

DECLARE_HUDELEMENT(CHudGrenade2);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudGrenade2::CHudGrenade2(const char *pElementName) : BaseClass(NULL, "HudGrenade2"), CHudElement(pElementName) 
{
	SetHiddenBits(/*HIDEHUD_HEALTH | */HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenade2::Init() 
{
	m_iGrenade		= -1;
	m_iClass		= 0;

	SetLabelText(L"");
	ivgui()->AddTickSignal( GetVPanel(), 100 );
}


//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudGrenade2::Reset() 
{
	BaseClass::Reset();

	m_iGrenade = 0;
	m_iClass = 0;
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get Grenade info from the weapon
//-----------------------------------------------------------------------------
void CHudGrenade2::OnTick() 
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	if (!player) 
		return;

	C_FFPlayer *ffplayer = ToFFPlayer(player);

	if (!ffplayer) 
		return;

	int iClass = ffplayer->GetClassSlot();
	int iGrenade2 = ffplayer->m_iSecondary;

	if(m_iClass != iClass)
	{
		m_iClass = iClass;
		if (!ffplayer 
			|| iClass == CLASS_CIVILIAN
			|| iClass == CLASS_SNIPER ) 
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
			return;
		}

		// Class doesn't have grenades
		if (iGrenade2 == -1) 
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
			return;
		}

		SetPaintEnabled(true);
		SetPaintBackgroundEnabled(true);

		// Different class, don't show anims
		SetGrenade(iGrenade2, false);

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ClassHasGrenades");

		// Get the correct grenade to display
		switch (iClass) 
		{
			case CLASS_SCOUT:
			case CLASS_MEDIC:
				iconTexture = gHUD.GetIcon("death_grenade_concussion");
				break;

			case CLASS_SOLDIER:
				iconTexture = gHUD.GetIcon("death_grenade_laser");
				break;

			case CLASS_DEMOMAN:
				iconTexture = gHUD.GetIcon("death_grenade_mirv");
				break;

			case CLASS_HWGUY:
				iconTexture = gHUD.GetIcon("death_grenade_slowfield");
				break;

			case CLASS_ENGINEER:
				iconTexture = gHUD.GetIcon("death_grenade_hoverturret");
				break;

			case CLASS_SPY:
				iconTexture = gHUD.GetIcon("death_grenade_cloaksmoke");
				break;

			case CLASS_PYRO:
				iconTexture = gHUD.GetIcon("death_grenade_napalm");
				break;
		}
	}
	else
	{
		// Same class, just update counts
		SetGrenade(iGrenade2, true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates Grenade display
//-----------------------------------------------------------------------------
void CHudGrenade2::SetGrenade(int iGrenade, bool playAnimation) 
{
	if (iGrenade != m_iGrenade) 
	{
		if (iGrenade == 0) 
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("GrenadeEmpty");
		}
		else if (iGrenade < m_iGrenade) 
		{
			// Grenade has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("GrenadeDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("GrenadeIncreased");
		}

		m_iGrenade = iGrenade;

		SetDisplayValue(m_iGrenade);
	}
}

void CHudGrenade2::Paint() 
{
	if(iconTexture)
	{
		Color iconColor( 255, 255, 255, 125 );
		int iconWide = iconTexture->Width();
		int iconTall = iconTexture->Height();

		//If we're using a font char, this will ignore iconTall and iconWide
		iconTexture->DrawSelf( icon_xpos, icon_ypos - (iconTall / 2), iconWide, iconTall, iconColor );
	}

	BaseClass::Paint();
}