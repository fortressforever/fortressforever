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
#include "c_ff_player.h"
#include "ff_hud_hint.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "ff_hud_grenade2timer.h"
#include "ff_playerclass_parse.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

extern IFileSystem **pFilesystem;

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
	//m_pHudElementTexture = new CHudTexture();
	//m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	//surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_box_timer2", true, false);
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

//-----------------------------------------------------------------------------
// Purpose: Clear all timers
//-----------------------------------------------------------------------------
void CHudGrenade2Timer::ResetTimer( void )
{
	m_Timers.RemoveAll();

	m_flLastTime = 0.0f;

	m_fVisible = false;
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutGrenade2Timer");
}

//-----------------------------------------------------------------------------
// Purpose: See if any timers are active
//-----------------------------------------------------------------------------
bool CHudGrenade2Timer::ActiveTimer( void ) const
{
	return m_Timers.Count();
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

	// Draw progress bars for each timer
	int num_timers = m_Timers.Count();
	if( num_timers < 1 )
		return;

	// Draw fg & bg box
	BaseClass::PaintBackground();
	
	// First get the class
	CBasePlayer *pLocalPlayer = CBasePlayer::GetLocalPlayer();

	if (pLocalPlayer == NULL)
		return;
		
	C_FFPlayer *ffplayer = ToFFPlayer(pLocalPlayer);

	if (!ffplayer || ffplayer->GetClassSlot() == CLASS_CIVILIAN || !ffplayer->GetClassSlot()) 
		return;

	
	const char *szClassNames[] = { "scout", "sniper", "soldier", 
								 "demoman", "medic", "hwguy", 
								 "pyro", "spy", "engineer", 
								 "civilian" };

	PLAYERCLASS_FILE_INFO_HANDLE hClassInfo;
	bool bReadInfo = ReadPlayerClassDataFromFileForSlot(*pFilesystem, szClassNames[ffplayer->GetClassSlot() - 1], &hClassInfo, NULL);

	if (!bReadInfo)
		return;

	const CFFPlayerClassInfo *pClassInfo = GetFilePlayerClassInfoFromHandle(hClassInfo);

	if (!pClassInfo)
		return;

	if ( strcmp( pClassInfo->m_szSecondaryClassName, "None" ) != 0 )
	{
		
		const char *grenade_name = pClassInfo->m_szSecondaryClassName;

		if( Q_strnicmp( grenade_name, "ff_", 3 ) == 0 )
		{
			//UTIL_LogPrintf( "  begins with ff_, removing\n" );
			grenade_name += 3;
		}

		char grenade_icon_name[256];

		Q_snprintf( grenade_icon_name, sizeof(grenade_icon_name), "death_%s", grenade_name );

		CHudTexture *icon = gHUD.GetIcon(grenade_icon_name);

		int iconWide = 0;
		int iconTall = 0;

		if( icon->bRenderUsingFont )
		{
			iconWide = surface()->GetCharacterWidth( icon->hFont, icon->cCharacterInFont );
			iconTall = surface()->GetFontTall( icon->hFont );
		}

		icon->DrawSelf( 5, iconTall - bar_height / 2, iconWide, iconTall, m_HudForegroundColour );
	}
	
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
			if (amount < 0.15f || (bIsLastTimer && pPlayer && pPlayer->m_iGrenadeState == FF_GREN_PRIMETWO))
				surface()->DrawSetColor(bar_color.r() - colour_mod, bar_color.g() - colour_mod, bar_color.b() - colour_mod, bar_color.a());
			else
				surface()->DrawSetColor(bar_color.r() - colour_mod, bar_color.g() - colour_mod, bar_color.b() - colour_mod, bar_color.a() * 0.3f);

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
