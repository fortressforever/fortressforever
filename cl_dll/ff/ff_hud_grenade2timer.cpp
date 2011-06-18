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
#include "hudelement.h"
#include "hud_macros.h"

#include "iclientmode.h"
#include "c_ff_player.h"

#include "ff_hud_grenade2timer.h"
#include "ff_playerclass_parse.h"

#include <KeyValues.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>

using namespace vgui;

CHudGrenade2Timer *g_pGrenade2Timer = NULL;

DECLARE_HUDELEMENT(CHudGrenade2Timer);

CHudGrenade2Timer::~CHudGrenade2Timer() 
{
}

void CHudGrenade2Timer::Init() 
{
	g_pGrenade2Timer = this;
	ivgui()->AddTickSignal( GetVPanel(), 100 );

	m_Timers.RemoveAll();
	m_fVisible = false;
	m_flLastTime = -10.0f;
	m_iClass = 0;
}

void CHudGrenade2Timer::SetTimer(float duration) 
{
	// Fade it in if needed
	if (!m_fVisible) 
	{
		SetAlpha(0);
		m_fVisible = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeInGrenade2Timer");
	}
	m_Timers.AddToTail(timer_t(gpGlobals->curtime, duration));
	m_flLastTime = gpGlobals->curtime + duration;
}

//-----------------------------------------------------------------------------
// Purpose: Clear all timers
//-----------------------------------------------------------------------------
void CHudGrenade2Timer::ResetTimer( void )
{
	SetAlpha(0);
	m_Timers.RemoveAll();
	m_fVisible = false;
	m_flLastTime = -10.0f;
	m_iClass = 0;
}

//-----------------------------------------------------------------------------
// Purpose: See if any timers are active
//-----------------------------------------------------------------------------
bool CHudGrenade2Timer::ActiveTimer( void ) const
{
	return m_Timers.Count() > 0;
}

void CHudGrenade2Timer::MsgFunc_FF_Grenade1Timer(bf_read &msg) 
{
	float duration = msg.ReadFloat();

	SetTimer(duration);
}

void CHudGrenade2Timer::OnTick()
{
	CBasePlayer *player = CBasePlayer::GetLocalPlayer();

	if (!player) 
		return;

	C_FFPlayer *ffplayer = ToFFPlayer(player);

	if (!ffplayer) 
		return;

	int iClass = ffplayer->GetClassSlot();

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

		const char *szClassNames[] = { "scout", "sniper", "soldier", 
									 "demoman", "medic", "hwguy", 
									 "pyro", "spy", "engineer", 
									 "civilian" };

		PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
		bool bReadInfo = ReadPlayerClassDataFromFileForSlot( vgui::filesystem(), szClassNames[m_iClass - 1], &hClassInfo, NULL );

		if (!bReadInfo)
			return;

		const CFFPlayerClassInfo *pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);

		if (!pClassInfo)
			return;

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
	}

	if ( gpGlobals->curtime > m_flLastTime ) 
	{
		float iFadeLength = g_pClientMode->GetViewportAnimationController()->GetAnimationSequenceLength("FadeOutGrenade2Timer");
		// Begin to fade
		if (m_fVisible) 
		{
			m_fVisible = false;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutGrenade2Timer");
		}
		// Fading time is over
		else if ( gpGlobals->curtime > m_flLastTime + iFadeLength) 
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
		}
	}
	else
	{
		SetPaintEnabled(true);
		SetPaintBackgroundEnabled(true);
	}
}

void CHudGrenade2Timer::Paint() 
{
	if(iconTexture)
	{
		int iconWide = iconTexture->Width();
		int iconTall = iconTexture->Height();

		iconTexture->DrawSelf( 5, iconTall - bar_height / 2, iconWide, iconTall, m_HudForegroundColour );
	}

	int num_timers = m_Timers.Count();
	int timer_to_remove = -1;

	float timer_height = bar_height / num_timers;
	float bar_newypos = bar_ypos;

	CFFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

	for (int i = m_Timers.Head(); i != m_Timers.InvalidIndex(); i = m_Timers.Next(i)) 
	{
		bool bIsLastTimer = (m_Timers.Next(i) == m_Timers.InvalidIndex());
		timer_t *timer = &m_Timers.Element(i);

		if (gpGlobals->curtime > timer->m_flStartTime + timer->m_flDuration) 
		{
			timer_to_remove = i;
		}
		else
		{
			float amount = clamp((gpGlobals->curtime - timer->m_flStartTime) / timer->m_flDuration, 0, 1.0f);

			// Draw progress bar
			if (amount < 0.15f || ( bIsLastTimer && pPlayer && pPlayer->m_iGrenadeState == FF_GREN_PRIMETWO ))
				surface()->DrawSetColor(bar_color.r(), bar_color.g(), bar_color.b(), bar_color.a());
			else
				surface()->DrawSetColor(bar_color.r(), bar_color.g(), bar_color.b(), bar_color.a() * 0.3f);

			surface()->DrawFilledRect(bar_xpos, bar_newypos, bar_xpos + bar_width * amount, bar_newypos + timer_height);

			bar_newypos += timer_height;
		}
	}

	// Remove a timer this frame
	if (timer_to_remove > -1) 
		m_Timers.Remove(timer_to_remove);
}
