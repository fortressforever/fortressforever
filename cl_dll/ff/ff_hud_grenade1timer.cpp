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
#include "c_ff_player.h"
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

	m_Timers.RemoveAll();

	SetPaintBackgroundEnabled(false);

	// Precache the background texture
	//m_pHudElementTexture = new CHudTexture();
	//m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	//surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_box_timer", true, false);
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

//-----------------------------------------------------------------------------
// Purpose: Clear all timers
//-----------------------------------------------------------------------------
void CHudGrenade1Timer::ResetTimer( void )
{
	m_Timers.RemoveAll();

	m_flLastTime = 0.0f;

	m_fVisible = false;
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutGrenade1Timer");
}

//-----------------------------------------------------------------------------
// Purpose: See if any timers are active
//-----------------------------------------------------------------------------
bool CHudGrenade1Timer::ActiveTimer( void ) const
{
	return m_Timers.Count();
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

	// Draw progress bars for each timer
	int num_timers = m_Timers.Count();
	if( num_timers < 1 )
		return;

	// Draw fg & bg box
	BaseClass::PaintBackground();

	int colour_mod = 0, timer_to_remove = -1;

	float timer_height = bar_height / num_timers;
	float bar_newypos = bar_ypos;

	CFFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	for (int i = m_Timers.Head(); i != m_Timers.InvalidIndex(); i = m_Timers.Next(i)) 
	{
		bool bIsLastTimer = (m_Timers.Next(i) == m_Timers.InvalidIndex());
		timer_t *timer = &m_Timers.Element(i);

		// ted_maul: 0000614: Grenade timer issues
		// Mark this up for removal if needed UNDONE: (1.7s so it doesnt disappear before fadeout if last) 
		if (gpGlobals->curtime > timer->m_flStartTime + timer->m_flDuration/* + 1.7f*/) 
			timer_to_remove = i;
		else
		{
			float amount = clamp((gpGlobals->curtime - timer->m_flStartTime) / timer->m_flDuration, 0, 1.0f);

			// Draw progress bar
			if (amount < 0.15f || (bIsLastTimer && pPlayer && pPlayer->m_iGrenadeState == FF_GREN_PRIMEONE))
				surface()->DrawSetColor(bar_color.r() - colour_mod, bar_color.g() - colour_mod, bar_color.b() - colour_mod, bar_color.a());
			else
				surface()->DrawSetColor(bar_color.r() - colour_mod, bar_color.g() - colour_mod, bar_color.b() - colour_mod, bar_color.a() * 0.5f);

			surface()->DrawFilledRect(bar_xpos, bar_newypos, bar_xpos + bar_width * amount, bar_newypos + timer_height);

			bar_newypos += timer_height;
			colour_mod += 20.0f;			// TODO: Constrain this? Probably not needed.
		}
	}
	
	// Remove a timer this frame
	if (timer_to_remove > -1) 
		m_Timers.Remove(timer_to_remove);
	
	// Draw progress bar box
	// surface()->DrawSetColor(bar_color);
	// surface()->DrawOutlinedRect(bar_xpos, bar_ypos, bar_xpos + bar_width, bar_ypos + bar_height);
}
