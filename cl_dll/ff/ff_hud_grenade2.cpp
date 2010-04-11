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
#include "ff_hud_hint.h"
#include "hud.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"
#include "iclientmode.h"
#include "iclientvehicle.h"
#include "ff_playerclass_parse.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

#include "c_ff_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern IFileSystem **pFilesystem;

using namespace vgui;

// Yeah macros suck, but this is the quickest way to do it
//#define ADD_GRENADE_ICON(id, filename) \
//	m_pSecondaryGrenade[id] = new CHudTexture(); \
//	m_pSecondaryGrenade[id]->textureId = surface()->CreateNewTextureID(); \
//	surface()->DrawSetTextureFile(m_pSecondaryGrenade[id]->textureId, filename, true, false);

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudGrenade2 : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudGrenade2, CHudNumericDisplay);

public:
	CHudGrenade2(const char *pElementName);
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
	void	GetHUDIcon();
	int		m_iGrenade;
	bool	bIconLoaded;
	CHudTexture *icon;
	// Last recorded player class
	int		m_iClass;

	//CHudTexture	*m_pHudElementTexture;
	//CHudTexture *m_pSecondaryGrenade[6];
};

DECLARE_HUDELEMENT(CHudGrenade2);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudGrenade2::CHudGrenade2(const char *pElementName) : BaseClass(NULL, "HudGrenade2"), CHudElement(pElementName) 
{
	SetHiddenBits(/*HIDEHUD_HEALTH | */HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
	bIconLoaded = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenade2::Init() 
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
void CHudGrenade2::VidInit() 
{
	// Precache the background texture
	//m_pHudElementTexture = new CHudTexture();
	//m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	//surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_box_ammo2", true, false);

	// Add the grenades icons
//	ADD_GRENADE_ICON(0, "vgui/hud_grenade_conc");
//	ADD_GRENADE_ICON(1, "vgui/hud_grenade_nail");
//	ADD_GRENADE_ICON(2, "vgui/hud_grenade_mirv");
//	ADD_GRENADE_ICON(3, "vgui/hud_grenade_gas");
//	ADD_GRENADE_ICON(4, "vgui/hud_grenade_napalm");
//	ADD_GRENADE_ICON(5, "vgui/hud_grenade_emp");
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudGrenade2::Reset() 
{
	BaseClass::Reset();

	m_iGrenade = 0;
	m_iClass = 0;

	UpdateGrenadeDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get Grenade info from the weapon
//-----------------------------------------------------------------------------
void CHudGrenade2::UpdatePlayerGrenade(C_BasePlayer *player) 
{
	if (!player) 
		return;

	C_FFPlayer *ffplayer = ToFFPlayer(player);

	if (!ffplayer || ( ffplayer->GetClassSlot() == CLASS_CIVILIAN ) || ( ffplayer->GetClassSlot() == CLASS_SNIPER ) ) 
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	int Grenade2 = ffplayer->m_iSecondary;

	// Class doesn't have grenades
	if (Grenade2 == -1) 
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
		SetGrenade(Grenade2, true);
	}
	else
	{
		// Different class, don't show anims
		SetGrenade(Grenade2, false);

		// Update whether to show one or two grenades(only 1 for sniper) 
		if (ffplayer->GetClassSlot() == CLASS_SNIPER) 
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ClassHasOneGrenade");
			SetShouldDisplaySecondaryValue(false);
		}
		else
		{
			SetShouldDisplaySecondaryValue(true);
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ClassHasTwoGrenades");
		}
		// force reload of class nade icon
		bIconLoaded = false;

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ClassHasGrenades");
		m_iClass = ffplayer->GetClassSlot();
	}
	
	if( !bIconLoaded )
	{
		//icon = gHUD.GetIcon("death_grenade_normal");
		//if( icon )
		//	bIconLoaded = true;
		GetHUDIcon();
	}
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get Grenade info from the weapon
//-----------------------------------------------------------------------------
void CHudGrenade2::OnThink() 
{
	UpdateGrenadeDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: updates the Grenade display counts
//-----------------------------------------------------------------------------
void CHudGrenade2::UpdateGrenadeDisplays() 
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	UpdatePlayerGrenade(player);
}

//-----------------------------------------------------------------------------
// Purpose: Updates Grenade display
//-----------------------------------------------------------------------------
void CHudGrenade2::SetGrenade(int Grenade, bool playAnimation) 
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

void CHudGrenade2::Paint() 
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	if (!player) 
		return;

	C_FFPlayer *ffplayer = ToFFPlayer(player);

	if (!ffplayer || ffplayer->GetClassSlot() == CLASS_CIVILIAN || ffplayer->GetClassSlot() == CLASS_SNIPER || !ffplayer->GetClassSlot()) 
		return;

	// Don't show while spec
	if (ffplayer->GetTeamNumber() < TEAM_BLUE || ffplayer->GetTeamNumber() > TEAM_GREEN)
		return;

	/* dexter - this isnt used atm
	int gren_num = 0;

	// Get the correct grenade to display
	switch (ffplayer->GetClassSlot()) 
	{
		case CLASS_SCOUT:
		case CLASS_MEDIC:
			gren_num = 0;
			break;

		case CLASS_SOLDIER:
			gren_num = 1;
			break;

		case CLASS_DEMOMAN:
		case CLASS_HWGUY:
			gren_num = 2;
			break;

		case CLASS_ENGINEER:
			gren_num = 5;
			break;

		case CLASS_SPY:
			gren_num = 3;
			break;

		case CLASS_PYRO:
			gren_num = 4;
			break;
	}
	*/

	// Draw background box
	//surface()->DrawSetTexture(m_pHudElementTexture->textureId);
	//surface()->DrawSetColor(255, 255, 255, 255);
	//surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());

	// Draw grenade icon
	//surface()->DrawSetTexture(m_pSecondaryGrenade[gren_num]->textureId);
	//surface()->DrawSetColor(255, 255, 255, 255);
	//surface()->DrawTexturedRect(icon_xpos, icon_ypos, icon_xpos + icon_width, icon_ypos + icon_height);

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

void CHudGrenade2::GetHUDIcon()
{
	const char *szClassNames[] = { "scout", "sniper", "soldier", 
								 "demoman", "medic", "hwguy", 
								 "pyro", "spy", "engineer", 
								 "civilian" };
	// First get the class
	CBasePlayer *pLocalPlayer = CBasePlayer::GetLocalPlayer();

	if (pLocalPlayer == NULL)
		return;
		
	C_FFPlayer *ffplayer = ToFFPlayer(pLocalPlayer);

	PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
	bool bReadInfo = ReadPlayerClassDataFromFileForSlot(*pFilesystem, szClassNames[ffplayer->GetClassSlot() - 1], &hClassInfo, NULL);

	if (!bReadInfo)
		return;

	const CFFPlayerClassInfo *pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);

	if (!pClassInfo)
		return;

	if ( strcmp( pClassInfo->m_szPrimaryClassName, "None" ) != 0 )
	{
		
		const char *grenade_name = pClassInfo->m_szSecondaryClassName;

		if( Q_strnicmp( grenade_name, "ff_", 3 ) == 0 )
		{
			//UTIL_LogPrintf( "  begins with ff_, removing\n" );
			grenade_name += 3;
		}
		char grenade_icon_name[256];
		Q_snprintf( grenade_icon_name, sizeof(grenade_icon_name), "death_%s", grenade_name );
		icon = gHUD.GetIcon(grenade_icon_name);
		if( icon )
		{
			bIconLoaded = true;
			//return true;
		}
	}
}