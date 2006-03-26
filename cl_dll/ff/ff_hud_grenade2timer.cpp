/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   20:02
	filename: 	f:\cvs\code\cl_dll\ff\ff_hud_grenade2timer.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ff_hud_grenade2timer
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	For now this is just a straight copy
				THIS WILL BE SORTED AT A LATER DATE OKAY!!
*********************************************************************/

#include "cbase.h"
#include "ff_hud_hint.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "ff_hud_grenade2timer.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

using namespace vgui;

CHudGrenade2Timer *g_pGrenade2Timer = NULL;

DECLARE_HUDELEMENT(CHudGrenade2Timer);

CHudGrenade2Timer::~CHudGrenade2Timer() 
{
}

void CHudGrenade2Timer::VidInit() 
{
	g_pGrenade2Timer = this;

	m_fVisible = false;
	m_flLastTime = -10.0f;
	
	m_Timers.RemoveAll();

	SetPaintBackgroundEnabled(false);

	// Precache the background texture
	m_pHudElementTexture = new CHudTexture();
	m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_box_timer2", true, false);
}

void CHudGrenade2Timer::Init() 
{
}

void CHudGrenade2Timer::SetTimer(float duration) 
{
	m_Timers.AddToTail(timer_t(gpGlobals->curtime, duration));

	// Fade it in if needed
	if (!m_fVisible) 
	{
		m_fVisible = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeInGrenade2Timer");
	}

	// We're assuming that all grens have the same timer
	m_flLastTime = gpGlobals->curtime + duration;
}

void CHudGrenade2Timer::MsgFunc_FF_Grenade1Timer(bf_read &msg) 
{
	float duration = msg.ReadFloat();

	SetTimer(duration);
}

void CHudGrenade2Timer::Paint() 
{
	if (gpGlobals->curtime > m_flLastTime + /*1.5f*/ 0.1f) 
	{
		// Begin to fade
		if (m_fVisible) 
		{
			m_fVisible = false;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutGrenade2Timer");
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