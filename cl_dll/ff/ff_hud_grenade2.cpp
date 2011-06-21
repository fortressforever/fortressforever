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

#include "iclientmode.h" //for animation stuff
#include "c_ff_player.h" //for gettuing ff player
#include "ff_playerclass_parse.h" //for parseing ff player txts

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
	C_FFPlayer *ffplayer = C_FFPlayer::GetLocalFFPlayer();

	if (!ffplayer) 
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	int iClass = ffplayer->GetClassSlot();
	int iGrenade2 = ffplayer->m_iSecondary;

	//if no class
	if(iClass == 0)
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		m_iClass = iClass;
		return;
	}
	else if(m_iClass != iClass)
	{
		m_iClass = iClass;

		// Class doesn't have grenades (not sure this check is valid but we check again later)
		if (iGrenade2 == -1) 
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
			return;
		}
		const char *szClassNames[] = { 
			"scout", "sniper", "soldier", 
			"demoman", "medic", "hwguy", 
			"pyro", "spy", "engineer", 
			"civilian" 
		};

		PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
		bool bReadInfo = ReadPlayerClassDataFromFileForSlot( vgui::filesystem(), szClassNames[m_iClass - 1], &hClassInfo, NULL );

		if (!bReadInfo)
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
			return;
		}

		const CFFPlayerClassInfo *pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);

		if (!pClassInfo)
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
			return;
		}

		SetPaintEnabled(true);
		SetPaintBackgroundEnabled(true);
		SetGrenade(iGrenade2, false);
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ClassHasGrenades");

		if ( strcmp( pClassInfo->m_szSecondaryClassName, "None" ) != 0 )
		{
			const char *grenade_name = pClassInfo->m_szSecondaryClassName;

			//if grenade names start with ff_
			if( Q_strnicmp( grenade_name, "ff_", 3 ) == 0 )
			//remove ff_
			{
				grenade_name += 3;
			}
			
			char grenade_icon_name[MAX_PLAYERCLASS_STRING + 3];

			Q_snprintf( grenade_icon_name, sizeof(grenade_icon_name), "death_%s", grenade_name );

			iconTexture = gHUD.GetIcon(grenade_icon_name);
		}
		else
		//Player doesn't have grenades
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
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