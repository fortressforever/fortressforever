#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "iclientmode.h"

#include "ff_hud_grenade1timer.h"
#include "ff_playerclass_parse.h"
#include "ff_grenade_parse.h"

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>

#include "c_playerresource.h"
extern C_PlayerResource *g_PR;

extern ConVar cl_teamcolourhud;

extern ConVar hud_grenadetimers;

using namespace vgui;

CHudGrenade1Timer *g_pGrenade1Timer = NULL;

DECLARE_HUDELEMENT(CHudGrenade1Timer);

CHudGrenade1Timer::CHudGrenade1Timer(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudGrenade1Timer") 
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );
}

CHudGrenade1Timer::~CHudGrenade1Timer() 
{
}

void CHudGrenade1Timer::Init() 
{
	g_pGrenade1Timer = this;
	ivgui()->AddTickSignal( GetVPanel(), 100 );

	ResetTimer();
}

void CHudGrenade1Timer::SetTimer(float duration) 
{
	// Fade it in if needed
	if (!m_fVisible) 
	{
		m_fVisible = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeInGrenade1Timer");
	}
	m_Timers.AddToTail(timer_t(gpGlobals->curtime, duration));
	m_flLastTime = gpGlobals->curtime + duration;
}

//-----------------------------------------------------------------------------
// Purpose: Clear all timers
//-----------------------------------------------------------------------------
void CHudGrenade1Timer::ResetTimer( void )
{
	SetAlpha(0);
	m_Timers.RemoveAll();
	m_fVisible = false;
	m_flLastTime = -10.0f;
	m_iClass = 0;
	m_iPlayerTeam = -1;
}

//-----------------------------------------------------------------------------
// Purpose: See if any timers are active
//-----------------------------------------------------------------------------
bool CHudGrenade1Timer::ActiveTimer( void ) const
{
	return m_Timers.Count() > 0;
}

// dexter: we wanna know how many are going. we should have about that many sounds playing too
int CHudGrenade1Timer::ActiveTimerCount( void ) const
{
	return m_Timers.Count();
}

void CHudGrenade1Timer::MsgFunc_FF_Grenade1Timer(bf_read &msg) 
{
	float duration = msg.ReadFloat();

	SetTimer(duration);
}

void CHudGrenade1Timer::OnTick()
{
	BaseClass::OnTick();

	if (!m_pFFPlayer) 
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}
	
	if (!hud_grenadetimers.GetBool()) 
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	int iClass = m_pFFPlayer->GetClassSlot();

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
		bool bReadInfo = ReadPlayerClassDataFromFileForSlot( vgui::filesystem(), szClassNames[m_iClass - 1], &hClassInfo, g_pGameRules->GetEncryptionKey());

		if (!bReadInfo)
			return;

		const CFFPlayerClassInfo *pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);

		if (!pClassInfo)
			return;

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
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
		}
	}

	if ( gpGlobals->curtime > m_flLastTime ) 
	{
		float iFadeLength = g_pClientMode->GetViewportAnimationController()->GetAnimationSequenceLength("FadeOutGrenade1Timer");
		// Begin to fade
		if (m_fVisible) 
		{
			m_fVisible = false;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutGrenade1Timer");
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

void CHudGrenade1Timer::PaintBackground() 
{
	// Draw progress bar background
	if(cl_teamcolourhud.GetBool())
		surface()->DrawSetColor(m_TeamColorHudBackgroundColour);
	else
		surface()->DrawSetColor(m_HudBackgroundColour);
	surface()->DrawFilledRect(bar_xpos, bar_ypos, bar_xpos + bar_width, bar_ypos + bar_height);

	// Draw progress bar border
	surface()->DrawSetColor(m_HudForegroundColour);
	surface()->DrawOutlinedRect(bar_xpos-1, bar_ypos-1, bar_xpos + bar_width+1, bar_ypos + bar_height+1);
	surface()->DrawOutlinedRect(bar_xpos-2, bar_ypos-2, bar_xpos + bar_width+2, bar_ypos + bar_height+2);
}

void CHudGrenade1Timer::Paint() 
{
	if(m_pIconTexture)
	{
		int iconWide = m_pIconTexture->Width();
		int iconTall = m_pIconTexture->Height();

		m_pIconTexture->DrawSelf( bar_xpos - 2/*boarderwidth*/ - iconWide - icon_offset, bar_ypos + bar_height/2 - iconTall/2, iconWide, iconTall, icon_color );
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
			if (amount < 0.15f || ( bIsLastTimer && pPlayer && pPlayer->m_iGrenadeState == FF_GREN_PRIMEONE ))
				surface()->DrawSetColor(m_HudForegroundColour.r(), m_HudForegroundColour.g(), m_HudForegroundColour.b(), m_HudForegroundColour.a());
			else
				surface()->DrawSetColor(m_HudForegroundColour.r(), m_HudForegroundColour.g(), m_HudForegroundColour.b(), m_HudForegroundColour.a() * 0.3f);

			surface()->DrawFilledRect(bar_xpos, bar_newypos, bar_xpos + bar_width * amount, bar_newypos + timer_height);

			bar_newypos += timer_height;
		}
	}

	// Remove a timer this frame
	if (timer_to_remove > -1) 
		m_Timers.Remove(timer_to_remove);
}
