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

#include "ff_hud_grenade1timer.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

CHudGrenade1Timer *g_pGrenade1Timer = NULL;

DECLARE_HUDELEMENT(CHudGrenade1Timer);

CHudGrenade1Timer::~CHudGrenade1Timer() 
{
}

void CHudGrenade1Timer::VidInit() 
{
	g_pGrenade1Timer = this;

	m_fVisible = false;
	m_flLastTime = -10.0f;

	SetPaintBackgroundEnabled(false);

	// Precache the background texture
	m_pHudElementTexture = new CHudTexture();
	m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_box_progressbar", true, false);
}

void CHudGrenade1Timer::Init() 
{
}

void CHudGrenade1Timer::SetTimer(float duration) 
{
	m_Timers.AddToTail(timer_t(gpGlobals->curtime, duration));

	// Fade it in if needed
	if (!m_fVisible) 
	{
		m_fVisible = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeInGrenade1Timer");
	}

	// We're assuming that all grens have the same timer
	m_flLastTime = gpGlobals->curtime + duration;
}

void CHudGrenade1Timer::MsgFunc_FF_Grenade1Timer(bf_read &msg) 
{
	float duration = msg.ReadFloat();

	SetTimer(duration);
}

void CHudGrenade1Timer::Paint() 
{
	if (gpGlobals->curtime > m_flLastTime + /*1.5f*/ 0.1f) 
	{
		// Begin to fade
		if (m_fVisible) 
		{
			m_fVisible = false;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutGrenade1Timer");
		}
		// Fading time is over
		else if (gpGlobals->curtime > m_flLastTime + /*1.7f*/ 0.2f) 
		{
			return;
		}
	}

    // Draw background box
	surface()->DrawSetTexture(m_pHudElementTexture->textureId);
	surface()->DrawSetColor(255, 255, 255, 255);
	surface()->DrawTexturedRect(0, 0, GetWide(), GetTall());

	// Draw progres bars for each timer
	int num_timers = m_Timers.Count();
	int colour_mod = 0, timer_to_remove = -1;

	float timer_height = bar_height / num_timers;
	float bar_newypos = bar_ypos;

	for (int i = m_Timers.Head(); i != m_Timers.InvalidIndex(); i = m_Timers.Next(i)) 
	{
		timer_t *timer = &m_Timers.Element(i);

		float amount = clamp((gpGlobals->curtime - timer->m_flStartTime) / timer->m_flDuration, 0, 1.0f);

		// Draw progress bar
		surface()->DrawSetColor(131 - colour_mod, 136 - colour_mod, 129 - colour_mod, 240);
		surface()->DrawFilledRect(bar_xpos, bar_newypos, bar_xpos + bar_width * amount, bar_newypos + timer_height);

		// Mark this up for removal if needed(1.7s so it doesnt disappear before fadeout if last) 
		if (gpGlobals->curtime > timer->m_flStartTime + timer->m_flDuration + 1.7f) 
			timer_to_remove = i;

		bar_newypos += timer_height;
		colour_mod += 20.0f;			// TODO: Constrain this? Probably not needed.
	}
	
	// Remove a timer this frame
	if (timer_to_remove > -1) 
		m_Timers.Remove(timer_to_remove);
	
	// Draw progress bar box
	surface()->DrawSetColor(0, 0, 0, 255);
	surface()->DrawOutlinedRect(bar_xpos, bar_ypos, bar_xpos + bar_width, bar_ypos + bar_height);
}