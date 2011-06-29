#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"
#include "hud_numericdisplay.h"

#include "iclientmode.h" //for animation stuff
#include "c_ff_player.h" //for gettuing ff player
#include "ff_playerclass_parse.h" //for parseing ff player txts
#include "ff_grenade_parse.h" //for parseing ff gren txts

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
class CHudGrenade1 : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudGrenade1, CHudNumericDisplay);

public:
	CHudGrenade1(const char *pElementName);
	void Init();
	void Reset();

	void SetGrenade(int Grenade, bool playAnimation);
	virtual void Paint();

protected:
	virtual void OnTick();

private:
	int		m_iGrenade;
	int		m_iClass;

	CHudTexture *m_pIconTexture;
};

DECLARE_HUDELEMENT(CHudGrenade1);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudGrenade1::CHudGrenade1(const char *pElementName) : BaseClass(NULL, "HudGrenade1"), CHudElement(pElementName) 
{
	SetHiddenBits(/*HIDEHUD_HEALTH | */HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudGrenade1::Init() 
{
	m_iGrenade		= -1;
	m_iClass		= 0;

	SetLabelText(L"");
	ivgui()->AddTickSignal( GetVPanel(), 100 );
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudGrenade1::Reset() 
{
	BaseClass::Reset();

	m_iGrenade = -1;
	m_iClass = -1;
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get Grenade info from the weapon
//-----------------------------------------------------------------------------
void CHudGrenade1::OnTick() 
{
	BaseClass::OnTick();

	if (!m_pFFPlayer) 
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	int iClass = m_pFFPlayer->GetClassSlot();
	int iGrenade1 = m_pFFPlayer->m_iPrimary;

	//if no class or class doesn't have grenades
	if(iClass == 0 || iGrenade1 == -1)
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		m_iClass = iClass;
		return;
	}
	else if(m_iClass != iClass)
	{
		m_iClass = iClass;

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
		SetGrenade(iGrenade1, false);
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("ClassHasGrenades");

		if ( strcmp( pClassInfo->m_szPrimaryClassName, "None" ) != 0 )
		{
			const char *grenade_name = pClassInfo->m_szPrimaryClassName;

			GRENADE_FILE_INFO_HANDLE hGrenInfo = LookupGrenadeInfoSlot(grenade_name);
			if (!hGrenInfo)
				return;

			CFFGrenadeInfo *pGrenInfo = GetFileGrenadeInfoFromHandle(hGrenInfo);
			if (!pGrenInfo)
				return;

			m_pIconTexture = pGrenInfo->iconHud;
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
		SetGrenade(iGrenade1, true);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates Grenade display
//-----------------------------------------------------------------------------
void CHudGrenade1::SetGrenade(int iGrenade, bool playAnimation) 
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
	}
	m_iGrenade = iGrenade;

	SetDisplayValue(m_iGrenade);
}

void CHudGrenade1::Paint() 
{
	if(m_pIconTexture)
	{
		Color iconColor( 255, 255, 255, 125 );
		int iconWide = m_pIconTexture->Width();
		int iconTall = m_pIconTexture->Height();

		//If we're using a font char, this will ignore iconTall and iconWide
		m_pIconTexture->DrawSelf( icon_xpos, icon_ypos - (iconTall / 2), iconWide, iconTall, iconColor );
	}

	BaseClass::Paint();
}