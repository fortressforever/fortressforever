/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   15:15
	filename: 	f:\cvs\code\cl_dll\ff\ff_hud_buildable.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ff_hud_buildable
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	The building progress bar
*********************************************************************/

#include "cbase.h"
#include "ff_hud_hint.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "ff_hud_buildtimer.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

CHudBuildTimer *g_pBuildTimer = NULL;

DECLARE_HUDELEMENT(CHudBuildTimer);
DECLARE_HUD_MESSAGE(CHudBuildTimer, FF_BuildTimer);

// Ye ol' macro to save some typing
#define ADD_BUILD_ICON(id, filename, displaytext) \
	m_pszBuildLabels[id] = displaytext; \
	m_pHudBuildIcons[id] = new CHudTexture(); \
	m_pHudBuildIcons[id]->textureId = surface()->CreateNewTextureID(); \
	surface()->DrawSetTextureFile(m_pHudBuildIcons[id]->textureId, filename, true, false);


CHudBuildTimer::~CHudBuildTimer() 
{
}

void CHudBuildTimer::VidInit() 
{
	m_fVisible = false;

	g_pBuildTimer = this;

	SetPaintBackgroundEnabled(false);
	SetLabelText(L"");
	m_iIcon = 0;

	// Precache the background texture
	m_pHudElementTexture = new CHudTexture();
	m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_box_buildable", true, false);

	ADD_BUILD_ICON(0, "vgui/hud_buildable_dispenser", "#FF_BUILDING_DISPENSER");
	ADD_BUILD_ICON(1, "vgui/hud_buildable_dispenser", "#FF_BUILDING_DISPENSER");
	ADD_BUILD_ICON(2, "vgui/hud_buildable_sentry", "#FF_BUILDING_SENTRY");
	ADD_BUILD_ICON(3, "vgui/hud_buildable_detpack", "#FF_BUILDING_DETPACK");
	ADD_BUILD_ICON(4, "vgui/hud_buildable_jumppad", "#FF_BUILDING_MANCANNON");
}

void CHudBuildTimer::Init() 
{
	HOOK_HUD_MESSAGE(CHudBuildTimer, FF_BuildTimer);
}

void CHudBuildTimer::SetBuildTimer(int type, float duration) 
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

void CHudBuildTimer::MsgFunc_FF_BuildTimer(bf_read &msg) 
{
	int type = msg.ReadShort();
	float duration = msg.ReadFloat();

	SetBuildTimer(type, duration);
}

void CHudBuildTimer::SetLabelText(const wchar_t *text) 
{
	wcsncpy(m_LabelText, text, sizeof(m_LabelText) / sizeof(wchar_t));
	m_LabelText[ (sizeof(m_LabelText) / sizeof(wchar_t)) - 1] = 0;
}

void CHudBuildTimer::Paint() 
{
	if (m_flDuration == 0) 
		m_flDuration = 0.001f;

	if (gpGlobals->curtime > m_flStartTime + m_flDuration + 1.5f) 
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
	}

	// Draw background box
//	surface()->DrawSetTexture(m_pHudElementTexture->textureId);
//	surface()->DrawSetColor(255, 255, 255, 255);
//	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());

	float amount = clamp((gpGlobals->curtime - m_flStartTime) / m_flDuration, 0, 1.0f);

	// Draw progress bar
	surface()->DrawSetColor(bar_color);
	surface()->DrawFilledRect(bar_xpos, bar_ypos, bar_xpos + bar_width * amount, bar_ypos + bar_height);

	// Draw progress bar box
	surface()->DrawSetColor(bar_color);
	surface()->DrawOutlinedRect(bar_xpos, bar_ypos, bar_xpos + bar_width, bar_ypos + bar_height);

	// Draw label
	surface()->DrawSetTextFont(m_hTextFont);
	surface()->DrawSetTextColor(GetFgColor());
	surface()->DrawSetTextPos(text_xpos, text_ypos);

	for (wchar_t *wch = m_LabelText; *wch != 0; wch++) 
	{
		surface()->DrawUnicodeChar(*wch);
	}

	// Draw icon
	surface()->DrawSetTexture(m_pHudBuildIcons[m_iIcon]->textureId);
	surface()->DrawSetColor(255, 255, 255, 255);
	surface()->DrawTexturedRect(icon_xpos, icon_ypos, icon_xpos + icon_width, icon_ypos + icon_height);
}