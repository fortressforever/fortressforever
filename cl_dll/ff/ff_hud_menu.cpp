/********************************************************************
	created:	2006/02/12
	created:	12:2:2006   1:03
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\ff_hud_menu.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_hud_menu
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"
#include "ff_hud_hint.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "ff_hud_menu.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

CHudContextMenu *g_pHudContextMenu = NULL;

ConVar cm_capturemouse("cl_cmcapture", "0", 0, "Context menu captures mouse");

DECLARE_HUDELEMENT(CHudContextMenu);
DECLARE_HUD_MESSAGE(CHudContextMenu, FF_BuildTimer);

CHudContextMenu::~CHudContextMenu() 
{
}

void CHudContextMenu::VidInit() 
{
	m_fVisible = false;

	g_pHudContextMenu = this;

	SetPaintBackgroundEnabled(false);
	SetLabelText(L"");
	m_iIcon = 0;

	// Precache the background texture
	m_pHudElementTexture = new CHudTexture();
	m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_box_buildable", true, false);

	// Cover whole screen for now
	SetWide(scheme()->GetProportionalScaledValue(640));
	SetTall(scheme()->GetProportionalScaledValue(480));
	SetPos(0, 0);
}

void CHudContextMenu::Init() 
{
	HOOK_HUD_MESSAGE(CHudContextMenu, FF_BuildTimer);
}

void CHudContextMenu::SetBuildTimer(int type, float duration) 
{
	m_iIcon = type;

	wchar_t *tempString = vgui::localize()->Find(m_pszBuildLabels[type]);
	if (tempString) 
		SetLabelText(tempString);
	else
		SetLabelText(L"NONE");

	// Fade it in if needed
	if (!m_fVisible && duration > 0) 
	{
		m_fVisible = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeInBuildTimer");
	}

	m_flDuration = duration;
	m_flStartTime = gpGlobals->curtime;

	// Special case for no duration, set so we fade out instantly (so we can cancel a timer)
	if (duration <= 0)
		m_flStartTime = gpGlobals->curtime - 1.5f;
}

void CHudContextMenu::MsgFunc_FF_BuildTimer(bf_read &msg) 
{
	int type = msg.ReadShort();
	float duration = msg.ReadFloat();

	SetBuildTimer(type, duration);
}

void CHudContextMenu::SetLabelText(const wchar_t *text) 
{
	wcsncpy(m_LabelText, text, sizeof(m_LabelText) / sizeof(wchar_t));
	m_LabelText[ (sizeof(m_LabelText) / sizeof(wchar_t)) - 1] = 0;
}

void CHudContextMenu::Paint() 
{
	return;

	if (m_flDuration == 0) 
		m_flDuration = 0.001f;

	/*if (gpGlobals->curtime > m_flStartTime + m_flDuration + 1.5f) 
	{
		// Begin to fade
		if (m_fVisible) 
		{
			m_fVisible = false;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutBuildTimer");
		}
		// Fading time is over
		else if (gpGlobals->curtime > m_flStartTime + m_flDuration + 1.7f) 
		{
			return;
		}
	}*/

	// Draw background box
	surface()->DrawSetTexture(m_pHudElementTexture->textureId);
	surface()->DrawSetColor(255, 255, 255, 255);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());
}

void CHudContextMenu::MouseMove(float *x, float *y)
{
	int posx, posy;

	GetPos(posx, posy);

	SetPos(clamp(posx + *x, -100, 900), clamp(posy + *y, -100, 700));

	if (cm_capturemouse.GetBool())
	{
		*x = 0;
		*y = 0;
	}
}

void HudContextMenuInput(float *x, float *y)
{
	if (g_pHudContextMenu)
		g_pHudContextMenu->MouseMove(x, y);
}
