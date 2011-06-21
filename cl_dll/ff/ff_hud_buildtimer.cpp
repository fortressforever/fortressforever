#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"

#include "iclientmode.h"
#include "c_ff_player.h"

#include "ff_hud_buildtimer.h"
#include "ff_buildableobjects_shared.h"

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/AnimationController.h>

#include "c_playerresource.h"
extern C_PlayerResource *g_PR;

extern ConVar cl_teamcolourhud;

using namespace vgui;

DECLARE_HUDELEMENT(CHudBuildTimer);
DECLARE_HUD_MESSAGE(CHudBuildTimer, FF_BuildTimer);

CHudBuildTimer::CHudBuildTimer(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudBuildTimer") 
{
	SetParent( g_pClientMode->GetViewport() );
	SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );
}

CHudBuildTimer::~CHudBuildTimer() 
{
}

void CHudBuildTimer::Init() 
{
	HOOK_HUD_MESSAGE(CHudBuildTimer, FF_BuildTimer);
	
	ivgui()->AddTickSignal( GetVPanel(), 100 );

	m_fVisible = false;
	m_iBuildType = 0;	
	m_iTeam = -1;
	m_flStartTime = 0.0f;
	m_flDuration = 0.0f;
}

void CHudBuildTimer::VidInit()
{
	m_pDispenserIconTexture = gHUD.GetIcon("build_dispenser");
	m_pSentrygunIconTexture = gHUD.GetIcon("build_sentrygun");
	m_pDetpackIconTexture = gHUD.GetIcon("build_detpack");
	m_pMancannonIconTexture = gHUD.GetIcon("build_jumppad");
}

void CHudBuildTimer::SetBuildTimer(int iBuildType, float flDuration) 
{
	if(m_iBuildType != iBuildType)
	{
		m_iBuildType = iBuildType;
		switch(iBuildType)
		{
		case FF_BUILD_NONE: //cancel build timer
			m_pIconTexture = NULL;
			m_fVisible = false;
			m_flDuration = 0.0f;
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
			return;
		case FF_BUILD_DISPENSER:
			m_pIconTexture = m_pDispenserIconTexture;
			break;
		case FF_BUILD_SENTRYGUN:
			m_pIconTexture = m_pSentrygunIconTexture;
			break;
		case FF_BUILD_DETPACK:
			m_pIconTexture = m_pDetpackIconTexture;
			break;
		case FF_BUILD_MANCANNON:
			m_pIconTexture = m_pMancannonIconTexture;
			break;
		}
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	m_flStartTime = gpGlobals->curtime;
	m_flDuration = flDuration;

	// Fade it in if needed
	if (!m_fVisible) 
	{
		m_fVisible = true;
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeInBuildTimer");
	}
}

void CHudBuildTimer::MsgFunc_FF_BuildTimer(bf_read &msg) 
{
	int type = msg.ReadShort();
	float duration = msg.ReadFloat();

	SetBuildTimer(type, duration);
}

void CHudBuildTimer::ApplySchemeSettings(IScheme *pScheme)
{
	m_HudForegroundColour = GetSchemeColor("HudItem.Foreground", pScheme);
	m_HudBackgroundColour = GetSchemeColor("HudItem.Background", pScheme);
	m_TeamColorHudBackgroundColour = GetSchemeColor("TeamColorHud.BackgroundAlpha", pScheme);

	BaseClass::ApplySchemeSettings(pScheme);
}

void CHudBuildTimer::OnTick()
{
	CFFPlayer *ffplayer = CFFPlayer::GetLocalFFPlayer();

	if (!ffplayer) 
		return;
		
	int iTeam = ffplayer->GetTeamNumber();
	
	if(m_iTeam != iTeam)
	{
		m_iTeam = iTeam;
		Color newTeamColor = g_PR->GetTeamColor(m_iTeam);
		m_TeamColorHudBackgroundColour.SetColor(newTeamColor.r(), newTeamColor.g(), newTeamColor.b(), m_TeamColorHudBackgroundColour.a());
	}
	if ( gpGlobals->curtime > m_flStartTime + m_flDuration ) 
	{
		float iFadeLength = g_pClientMode->GetViewportAnimationController()->GetAnimationSequenceLength("FadeOutBuildTimer");
		// Begin to fade
		if (m_fVisible) 
		{
			m_fVisible = false;
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutBuildTimer");
		}
		// Fading time is over
		else if ( gpGlobals->curtime > m_flStartTime + m_flDuration + iFadeLength) 
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

void CHudBuildTimer::PaintBackground() 
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

void CHudBuildTimer::Paint() 
{
	if(m_pIconTexture)
	{
		int iconWide = 32.0f; //m_pIconTexture->Width();
		int iconTall = 32.0f; //m_pIconTexture->Height();
		
		m_pIconTexture->DrawSelf( bar_xpos - 2/*boarderwidth*/ - iconWide - icon_offset, bar_ypos + bar_height/2 - iconTall/2, iconWide, iconTall, m_HudForegroundColour );
	}
	
	float amount = clamp((gpGlobals->curtime - m_flStartTime) / m_flDuration, 0, 1.0f);
	surface()->DrawSetColor(m_HudForegroundColour);
	surface()->DrawFilledRect(bar_xpos, bar_ypos, bar_xpos + bar_width * amount, bar_ypos + bar_height);
}