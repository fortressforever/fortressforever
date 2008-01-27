/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   18:31
	filename: 	f:\cvs\code\cl_dll\ff\ff_hud_buildstate.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ff_hud_buildstate
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Show information for buildables
*********************************************************************/

#include "cbase.h"
#include "ff_hud_hint.h"
#include "hudelement.h"
#include "hud_macros.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>
#include <vgui/IVGui.h>

#include <vgui/ILocalize.h>

#include "c_ff_buildableobjects.h"
#include "ff_buildableobjects_shared.h"

using namespace vgui;

class CHudBuildState : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildState, vgui::Panel);

	// Stuff we need to know
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "ChatFont");

	CPanelAnimationVarAliasType(float, text1_xpos, "text1_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text1_ypos, "text1_ypos", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, icon1_xpos, "icon1_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon1_ypos, "icon1_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon1_width, "icon1_width", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, icon1_height, "icon1_height", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, icon2_xpos, "icon2_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon2_ypos, "icon2_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon2_width, "icon2_width", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, icon2_height, "icon2_height", "1", "proportional_float");

	CHudTexture	*m_pHudElementTexture;

	// Results of localising strings
	wchar_t m_szHealth[32];
	wchar_t m_szAmmo[32];
	wchar_t m_szNoRockets[32];

	// Icons
	CHudTexture *m_pHudSentry;
	CHudTexture *m_pHudDispenser;

	// Lines of information
	wchar_t m_szDispenser[128];
	wchar_t m_szSentry[128];

	bool m_bDrawDispenser;
	bool m_bDrawSentry;

	void MsgFunc_DispenserMsg(bf_read &msg);
	void MsgFunc_SentryMsg(bf_read &msg);

public:
	CHudBuildState(const char *pElementName) : CHudElement(pElementName), vgui::Panel(NULL, "HudBuildState") 
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits( 0 );
	}

	~CHudBuildState();

	void	Init();
	void	VidInit();
	void	Paint();

	void	OnTick();
};

CHudBuildState *g_pBuildState = NULL;

DECLARE_HUDELEMENT(CHudBuildState);
DECLARE_HUD_MESSAGE(CHudBuildState, DispenserMsg);
DECLARE_HUD_MESSAGE(CHudBuildState, SentryMsg);


CHudBuildState::~CHudBuildState() 
{
}

void CHudBuildState::VidInit() 
{
	g_pBuildState = this;

	SetPaintBackgroundEnabled(false);

	// Precache the icons
	m_pHudSentry = new CHudTexture();
	m_pHudSentry->textureId = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_pHudSentry->textureId, "vgui/hud_buildable_sentry", true, false);

	m_pHudDispenser = new CHudTexture();
	m_pHudDispenser->textureId = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_pHudDispenser->textureId, "vgui/hud_buildable_dispenser", true, false);

	// Precache the strings
	wchar_t *tempString = vgui::localize()->Find("#FF_HUD_HEALTH");

	if (!tempString) 
		tempString = L"HEALTH";

	wcsncpy(m_szHealth, tempString, sizeof(m_szHealth) / sizeof(wchar_t));
	m_szHealth[ (sizeof(m_szHealth) / sizeof(wchar_t)) - 1] = 0;

	tempString = vgui::localize()->Find("#FF_HUD_AMMO");

	if (!tempString) 
		tempString = L"AMMO";

	wcsncpy(m_szAmmo, tempString, sizeof(m_szAmmo) / sizeof(wchar_t));
	m_szAmmo[ (sizeof(m_szAmmo) / sizeof(wchar_t)) - 1] = 0;

	tempString = vgui::localize()->Find("#FF_HUD_NOROCKETS");

	if (!tempString) 
		tempString = L"No Rockets";

	wcsncpy(m_szNoRockets, tempString, sizeof(m_szNoRockets) / sizeof(wchar_t));
	m_szNoRockets[ (sizeof(m_szNoRockets) / sizeof(wchar_t)) - 1] = 0;
}

void CHudBuildState::Init() 
{
	vgui::ivgui()->AddTickSignal(GetVPanel());

	HOOK_HUD_MESSAGE(CHudBuildState, DispenserMsg);
	HOOK_HUD_MESSAGE(CHudBuildState, SentryMsg);
}

void CHudBuildState::OnTick() 
{
	m_bDrawDispenser = m_bDrawSentry = false;

	if (!engine->IsInGame()) 
		return;

	// Get the local player
	C_FFPlayer *pPlayer = ToFFPlayer(C_BasePlayer::GetLocalPlayer());

	if (!pPlayer) 
		return;

	C_FFDispenser *pDispenser = pPlayer->GetDispenser();
	C_FFSentryGun *pSentryGun = pPlayer->GetSentryGun();

	m_bDrawDispenser = pDispenser && pDispenser->IsBuilt();
	m_bDrawSentry = pSentryGun && pSentryGun->m_iLevel > 0;
}

void CHudBuildState::MsgFunc_DispenserMsg(bf_read &msg)
{
    int iHealth = (int) msg.ReadByte();
    int iAmmo = (int) msg.ReadByte();

	_snwprintf(m_szDispenser, 127, L"%s: %i%% %s: %i%%", m_szHealth, iHealth, m_szAmmo, iAmmo);
}

void CHudBuildState::MsgFunc_SentryMsg(bf_read &msg)
{
    int iHealth = (int) msg.ReadByte();
    int iAmmo = (int) msg.ReadByte();
	int iLevel = (int) msg.ReadByte();

	bool fNoRockets = false;
	
	// Last bit of ammo is the rocket warning
	if (iAmmo >= 128)
	{
		fNoRockets = true;
		iAmmo -= 128;
	}

	_snwprintf(m_szSentry, 127, L"Level %i - %s: %i%% %s: %i%% %s", iLevel , m_szHealth, iHealth, m_szAmmo, iAmmo, fNoRockets ? m_szNoRockets : L"");
}

void CHudBuildState::Paint() 
{
	if (!m_bDrawDispenser && !m_bDrawSentry) 
		return;

	// Draw icons
	surface()->DrawSetColor(255, 255, 255, 255);

	if (m_bDrawSentry) 
	{
		surface()->DrawSetTexture(m_pHudSentry->textureId);
		surface()->DrawTexturedRect(icon1_xpos, icon1_ypos, icon1_xpos + icon1_width, icon1_ypos + icon1_height);
	}

	if (m_bDrawDispenser) 
	{
		surface()->DrawSetTexture(m_pHudDispenser->textureId);
		surface()->DrawTexturedRect(icon2_xpos, icon2_ypos, icon2_xpos + icon2_width, icon2_ypos + icon2_height);
	}

	// Draw text
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(GetFgColor());

	if (m_bDrawSentry) 
	{
		surface()->DrawSetTextPos(text1_xpos, text1_ypos);

		for (wchar_t *wch = m_szSentry; *wch != 0; wch++) 
			surface()->DrawUnicodeChar(*wch);
	}

	if (m_bDrawDispenser) 
	{
		surface()->DrawSetTextPos(text2_xpos, text2_ypos);

		for (wchar_t *wch = m_szDispenser; *wch != 0; wch++) 
			surface()->DrawUnicodeChar(*wch);
	}
}
