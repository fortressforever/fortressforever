/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   15:15
	filename: 	f:\cvs\code\cl_dll\ff\ff_hud_grenade1timer.cpp
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ff_hud_grenade1timer
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

#include "ff_hud_grenade1timer.h"
#include "ff_playerclass_parse.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include <vgui/ILocalize.h>

extern IFileSystem **pFilesystem;

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
	m_pIcon = new CHudTexture;

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

	if ( strcmp( pClassInfo->m_szPrimaryClassName, "None" ) != 0 )
	{
		
		const char *grenade_name = pClassInfo->m_szPrimaryClassName;

		if( Q_strnicmp( grenade_name, "ff_", 3 ) == 0 )
		{
			//UTIL_LogPrintf( "  begins with ff_, removing\n" );
			grenade_name += 3;
		}

		char grenade_icon_name[256];

		Q_snprintf( grenade_icon_name, sizeof(grenade_icon_name), "death_%s", grenade_name );

		m_pIcon = gHUD.GetIcon(grenade_icon_name);
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
	
	if (m_pIcon)
	{
		int iconWide = 0;
		int iconTall = 0;

		if( m_pIcon->bRenderUsingFont )
		{
			iconWide = surface()->GetCharacterWidth( m_pIcon->hFont, m_pIcon->cCharacterInFont );
			iconTall = surface()->GetFontTall( m_pIcon->hFont );
		}

		m_pIcon->DrawSelf( 5, iconTall - bar_height / 2, iconWide, iconTall, m_HudForegroundColour );
	}

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
