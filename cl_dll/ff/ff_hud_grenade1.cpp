/********************************************************************
	created:	2006/01/04
	created:	4:1:2006   9:26
	filename: 	f:\cvs\code\cl_dll\ff\ff_hud_grenade1.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ff_hud_grenade1
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

#include "c_ff_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

// Yeah macros suck, but this is the quickest way to do it
/* Dexter - comment this out :)
#define ADD_GRENADE_ICON(id, filename) \
	m_pPrimaryGrenade[id] = new CHudTexture(); \
	m_pPrimaryGrenade[id]->textureId = surface()->CreateNewTextureID(); \
	surface()->DrawSetTextureFile(m_pPrimaryGrenade[id]->textureId, filename, true, false);
*/

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudGrenade1 : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudGrenade1, CHudNumericDisplay);

public:
	CHudGrenade1(const char *pElementName);
	void Init();
	void VidInit();
	void Reset();

	void SetGrenade(int Grenade, bool playAnimation);
	virtual void Paint();

protected:
	virtual void OnThink();

	void UpdateGrenadeDisplays();
	void UpdatePlayerGrenade(C_BasePlayer *player);

private:
	int		m_iGrenade;

	// Last recorded player class
	int		m_iClass;
	CHudTexture *icon;
	bool bIconLoaded;
	//CHudTexture	*m_pHudElementTexture;
	//CHudTexture *m_pPrimaryGrenade[2];
};

DECLARE_HUDELEMENT(CHudGrenade1);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudGrenade1::CHudGrenade1(const char *pElementName) : BaseClass(NULL, "HudGrenade1"), CHudElement(pElementName) 
{
	SetHiddenBits(/*HIDEHUD_HEALTH | */HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
	bIconLoaded = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenade1::Init() 
{
	m_iGrenade		= -1;

/*	wchar_t *tempString = vgui::localize()->Find("#FF_HUD_GRENADE");
	if (tempString) 
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"Grenade");
	}*/

	SetLabelText(L"");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenade1::VidInit() 
{
	// Precache the background texture
	//m_pHudElementTexture = new CHudTexture();
	//m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	//surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_box_ammo1", true, false);
	
	// Add the grenades icons(only these 2 are needed) 
	//ADD_GRENADE_ICON(0, "vgui/hud_grenade_frag");
	//ADD_GRENADE_ICON(1, "vgui/hud_grenade_caltop");
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudGrenade1::Reset() 
{
	BaseClass::Reset();

	m_iGrenade = 0;
	m_iClass = 0;

	UpdateGrenadeDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get Grenade info from the weapon
//-----------------------------------------------------------------------------
void CHudGrenade1::UpdatePlayerGrenade(C_BasePlayer *player) 
{
	if (!player) 
		return;

	C_FFPlayer *ffplayer = ToFFPlayer(player);

	if (!ffplayer || ffplayer->GetClassSlot() == CLASS_CIVILIAN) 
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	int Grenade1 = ffplayer->m_iPrimary;

	// Class doesn't have grenades
	if (Grenade1 == -1) 
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	if (m_iClass == ffplayer->GetClassSlot()) 
	{
		// Same class, just update counts
		SetGrenade(Grenade1, true);
	}
	else
	{
		// Different class, don't show anims
		SetGrenade(Grenade1, false);

		// Update whether to show one or two grenades(only 1 for sniper) 
		// Dexter - removed this complete because it was commented out and optimized away anyhow
		/*
			if (ffplayer->GetClassSlot() == CLASS_SNIPER) 
		{
			//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ClassHasOneGrenade");
			//SetShouldDisplaySecondaryValue(false);
		}
		else
		{
			//SetShouldDisplaySecondaryValue(true);
			//g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ClassHasTwoGrenades");
		}
		*/ 
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ClassHasGrenades");
		m_iClass = ffplayer->GetClassSlot();
	}

	if( !bIconLoaded )
	{
		icon = gHUD.GetIcon("death_grenade_normal");
		if( icon )
			bIconLoaded = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get Grenade info from the weapon
//-----------------------------------------------------------------------------
void CHudGrenade1::OnThink() 
{
	UpdateGrenadeDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: updates the Grenade display counts
//-----------------------------------------------------------------------------
void CHudGrenade1::UpdateGrenadeDisplays() 
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	
	UpdatePlayerGrenade(player);
}

//-----------------------------------------------------------------------------
// Purpose: Updates Grenade display
//-----------------------------------------------------------------------------
void CHudGrenade1::SetGrenade(int Grenade, bool playAnimation) 
{
	if (Grenade != m_iGrenade) 
	{
		if (Grenade == 0) 
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("GrenadeEmpty");
		}
		else if (Grenade < m_iGrenade) 
		{
			// Grenade has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("GrenadeDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("GrenadeIncreased");
		}

		m_iGrenade = Grenade;
	}

	SetDisplayValue(Grenade);
}

void CHudGrenade1::Paint() 
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	if (!player) 
		return;

	C_FFPlayer *ffplayer = ToFFPlayer(player);

	if (!ffplayer || ffplayer->GetClassSlot() == CLASS_CIVILIAN || !ffplayer->GetClassSlot()) 
		return;

	// Don't show while spec
	if (ffplayer->GetTeamNumber() < TEAM_BLUE || ffplayer->GetTeamNumber() > TEAM_GREEN)
		return;

	int gren_num = 0;

	if (ffplayer->GetClassSlot() == CLASS_SCOUT) 
		gren_num = 1;

	// Draw background box
	//surface()->DrawSetTexture(m_pHudElementTexture->textureId);
	//surface()->DrawSetColor(255, 255, 255, 255);
	//surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());

	// Draw grenade icon

	//CHudTexture *icon = gHUD.GetIcon("death_grenade_normal");
	if( bIconLoaded ) 
	{
		int iconWide = 0;
		int iconTall = 0;
		if( icon->bRenderUsingFont )
		{
			iconWide = surface()->GetCharacterWidth( icon->hFont, icon->cCharacterInFont );
			iconTall = surface()->GetFontTall( icon->hFont );
		}
		icon->DrawSelf( icon_xpos, icon_ypos - (iconTall/2), iconWide, iconTall, m_HudForegroundColour );
	}
	
	BaseClass::Paint();
}