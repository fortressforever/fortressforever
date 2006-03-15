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
#include "mathlib.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/AnimationController.h>

#include "c_ff_player.h"

#include <vgui/ILocalize.h>

using namespace vgui;

CHudContextMenu *g_pHudContextMenu = NULL;

extern ConVar sensitivity;

ConVar cm_capturemouse("cl_cmcapture", "1", 0, "Context menu captures mouse");
ConVar cm_showmouse("cl_cmshowmouse", "0", 0, "Show mouse position");

#define MENU_PROGRESS_TIME	0.8f

DECLARE_HUDELEMENT(CHudContextMenu);

ADD_MENU_OPTION(builddispenser, L"Build Dispenser", "builddispensers")
{
	C_FFPlayer *ff = dynamic_cast<C_FFPlayer *>(CBasePlayer::GetLocalPlayer());

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->m_bBuilding && ( ff->m_iCurBuild == FF_BUILD_DISPENSER ) )
		return MENU_DIM;

	if (!ff || ff->m_hDispenser)
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(detdispenser, L"Detonate Dispenser", "detdispenser")
{
	C_FFPlayer *ff = dynamic_cast<C_FFPlayer *>(CBasePlayer::GetLocalPlayer());

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->m_bBuilding && ( ff->m_iCurBuild == FF_BUILD_DISPENSER ) )
		return MENU_DIM;

	if (!ff || !ff->m_hDispenser)
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(dismantledispenser, L"Dismantle Dispenser", "dismantledispenser")
{
	C_FFPlayer *ff = dynamic_cast<C_FFPlayer *>(CBasePlayer::GetLocalPlayer());

	// Bug #0000333: Buildable Behavior (non build slot) while building
	if( ff->m_bBuilding && ( ff->m_iCurBuild == FF_BUILD_DISPENSER ) )
		return MENU_DIM;

	if (!ff || !ff->m_hDispenser)
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(buildsentry, L"Build Sentry", "buildsentry")
{
	C_FFPlayer *ff = dynamic_cast<C_FFPlayer *>(CBasePlayer::GetLocalPlayer());

	if (!ff || ff->m_hDispenser)
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(detsentry, L"Detonate Sentry", "detsentry")
{
	C_FFPlayer *ff = dynamic_cast<C_FFPlayer *>(CBasePlayer::GetLocalPlayer());

	if (!ff || !ff->m_hSentryGun)
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(dismantlesentry, L"Dismantle Sentry", "dismantlesentry")
{
	C_FFPlayer *ff = dynamic_cast<C_FFPlayer *>(CBasePlayer::GetLocalPlayer());

	if (!ff || !ff->m_hSentryGun)
		return MENU_DIM;

	return MENU_SHOW;
}

ADD_MENU_OPTION(aimsentry, L"Aim Sentry", "aimsentry")
{
	C_FFPlayer *ff = dynamic_cast<C_FFPlayer *>(CBasePlayer::GetLocalPlayer());

	if (!ff || !ff->m_hSentryGun)
		return MENU_DIM;

	return MENU_SHOW;
}

// These act as intermediate menus
ADD_MENU_OPTION(disguiseteam, L"Disguise as team", "team")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguiseenemy, L"Disguise as enemy", "enemy")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguisescout, L"Disguise as scout", "scout")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguisesniper, L"Disguise as sniper", "sniper")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguisesoldier, L"Disguise as soldier", "soldier")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguisedemoman, L"Disguise as demoman", "demoman")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguisemedic, L"Disguise as medic", "medic")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguisehwguy, L"Disguise as hwguy", "hwguy")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguisespy, L"Disguise as spy", "spy")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguisepyro, L"Disguise as pyro", "pyro")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguiseengineer, L"Disguise as engineer", "engineer")
{
	return MENU_SHOW;
}

ADD_MENU_OPTION(disguisecivilian, L"Disguise as civilian", "civilian")
{
	return MENU_DIM;
}


// Actual possible menus
menuoption_t engy_menu[] = { detdispenser, dismantledispenser, detsentry, dismantlesentry, aimsentry };
menuoption_t spy_menu1[] = { disguiseteam, disguiseenemy };
menuoption_t spy_menu2[] = { disguisescout, disguisesniper, disguisesoldier, disguisedemoman, disguisemedic, disguisehwguy, disguisepyro, disguisespy, disguiseengineer, disguisecivilian };

CHudContextMenu::~CHudContextMenu() 
{
}

void CHudContextMenu::VidInit() 
{
	m_fVisible = false;

	g_pHudContextMenu = this;

	SetPaintBackgroundEnabled(false);
	m_iIcon = 0;

	// Precache the background texture
	m_pHudElementTexture = new CHudTexture();
	m_pHudElementTexture->textureId = surface()->CreateNewTextureID();
	surface()->DrawSetTextureFile(m_pHudElementTexture->textureId, "vgui/hud_button", true, false);
}

void CHudContextMenu::Init() 
{
}

void CHudContextMenu::DoCommand(const char *cmd)
{
	if (m_pMenu == &engy_menu[0])
		engine->ClientCmd(cmd);

	else if(m_pMenu == &spy_menu2[0])
	{
		char buf[128];
		sprintf(buf, "disguise %s %s", m_pszPreviousCmd, cmd);
		engine->ClientCmd(buf);
		DevMsg(buf);
	}
}

void CHudContextMenu::Display(bool state)
{
	C_FFPlayer *pPlayer = dynamic_cast<C_FFPlayer *> (CBasePlayer::GetLocalPlayer());

	if (!pPlayer)
		return;

	// There is a menu and it's cancelling
	if (m_pMenu && m_fVisible == true && state == false)
	{
		if (m_iSelected >= 0)
		{
			if (m_pMenu[m_iSelected].conditionfunc() == MENU_SHOW)
				DoCommand(m_pMenu[m_iSelected].szCommand);
		}

		m_fVisible = state;
		return;
	}

	m_fVisible = state;

	// Decide which menu is to be shown
	if (pPlayer->GetClassSlot() == CLASS_ENGINEER)
	{
		m_pMenu = &engy_menu[0];
		m_nOptions = sizeof(engy_menu) / sizeof(engy_menu[0]);
	}
	else if (pPlayer->GetClassSlot() == CLASS_SPY)
	{
		m_pMenu = &spy_menu1[0];
		m_nOptions = sizeof(spy_menu1) / sizeof(spy_menu1[0]);
	}

	SetMenu();
}

void CHudContextMenu::SetMenu()
{
	m_flSelectStart = gpGlobals->curtime;

	float midx = scheme()->GetProportionalScaledValue(320);
	float midy = scheme()->GetProportionalScaledValue(240);
	float dist = scheme()->GetProportionalScaledValue(120);

	m_flPosX = midx;
	m_flPosY = midy;

	// Get positions for buttons
	for (int i = 0; i < m_nOptions; i++)
	{
		float increments = (float) i * DEG2RAD(360) / (float) m_nOptions;

		m_flPositions[i][0] = midx + dist * sin(increments);
		m_flPositions[i][1] = midy - dist * cos(increments) * 0.6f;
	}

	// Big enough to fit whole circle
	SetWide(scheme()->GetProportionalScaledValue(640));
	SetTall(scheme()->GetProportionalScaledValue(480));

	// Position in centre
	SetPos(scheme()->GetProportionalScaledValue(320) - GetWide() * 0.5f, scheme()->GetProportionalScaledValue(240) - GetTall() * 0.5f);
}

void CHudContextMenu::Paint() 
{
	if (!m_fVisible) 
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

	float halfbuttonX = scheme()->GetProportionalScaledValue(40.0f);
	float halfbuttonY = scheme()->GetProportionalScaledValue(20.0f);

	// Button boxes
	surface()->DrawSetTexture(m_pHudElementTexture->textureId);
	surface()->DrawSetColor(255, 255, 255, 255);

	// Draw boxes
	for (int i = 0; i < m_nOptions; i++)
		surface()->DrawTexturedRect(m_flPositions[i][0] - halfbuttonX, m_flPositions[i][1] - halfbuttonY, m_flPositions[i][0] + halfbuttonX, m_flPositions[i][1] + halfbuttonY);

	// Set and get height of font
	surface()->DrawSetTextFont(m_hTextFont);
	int offsetY = 0.5f * surface()->GetFontTall(m_hTextFont);

	// Colours we need later on
	Color highlighted(255, 0, 0, 255);
	Color dimmed(100, 100, 100, 255);

	float dx = m_flPosX - scheme()->GetProportionalScaledValue(320);
	float dy = m_flPosY - scheme()->GetProportionalScaledValue(240);

	int newSelection = -1;

	// If we're not in the middle then get the right button
	if ((dx * dx) + (dy * dy) > 10000)
	{
		const float pi_2 = M_PI * 2.0f;

		float angle_per_button = pi_2 / m_nOptions;
		float angle = atan2f(dx, -dy) + angle_per_button * 0.5f;

		if (angle < 0)
			angle += pi_2;
		if (angle > pi_2)
			angle -= pi_2;

		newSelection = angle / angle_per_button;
	}

	// Draw text for each box
	for (int i = 0; i < m_nOptions; i++)
	{
		surface()->DrawSetTextColor(GetFgColor());

		// Dimmed out
		if (m_pMenu[i].conditionfunc() != MENU_SHOW)
			surface()->DrawSetTextColor(dimmed);

		// Highlighted
		else if (newSelection == i)
			surface()->DrawSetTextColor(highlighted);

		// Work out centering and position & draw text
		int offsetX = 0.5f * UTIL_ComputeStringWidth(m_hTextFont, m_pMenu[i].szName);
		surface()->DrawSetTextPos(m_flPositions[i][0] - offsetX, m_flPositions[i][1] - offsetY);
		
		for (const wchar_t *wch = m_pMenu[i].szName; *wch != 0; wch++)
			surface()->DrawUnicodeChar(*wch);
	}

	// Restart timer if a new selection
	if (newSelection != m_iSelected)
		m_flSelectStart = gpGlobals->curtime;

	m_iSelected = newSelection;

	// Progress to next menu if needed
	// TODO: A cleaner way of doing this
	if (m_iSelected > -1 && gpGlobals->curtime > m_flSelectStart + MENU_PROGRESS_TIME)
	{
		if (m_pMenu == &spy_menu1[0])
		{
			m_pszPreviousCmd = m_pMenu[m_iSelected].szCommand;
			m_pMenu = &spy_menu2[0];
			m_nOptions = sizeof(spy_menu2) / sizeof(spy_menu2[0]);
			
			m_iSelected = -1;
			m_flSelectStart = gpGlobals->curtime;

			SetMenu();
		}
	}

	// Show actual mouse location for debuggin
	if (cm_showmouse.GetBool())
	{
		surface()->DrawSetTexture(m_pHudElementTexture->textureId);
		surface()->DrawSetColor(255, 255, 255, 255);
		surface()->DrawTexturedRect(m_flPosX - 5, m_flPosY - 5, m_flPosX + 5, m_flPosY + 5);
	}
}

void CHudContextMenu::MouseMove(float *x, float *y) 
{
	float sensitivity_factor = 1.0f / (sensitivity.GetFloat() == 0 ? 0.001f : sensitivity.GetFloat());

	float midx = scheme()->GetProportionalScaledValue(320);
	float midy = scheme()->GetProportionalScaledValue(240);
	float dist = scheme()->GetProportionalScaledValue(120);

	m_flPosX = clamp(m_flPosX + (*x * sensitivity_factor), midx - dist, midx + dist);
	m_flPosY = clamp(m_flPosY + (*y * sensitivity_factor), midy - dist, midy + dist);

	if (m_fVisible && cm_capturemouse.GetBool()) 
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

void HudContextShow(bool visible) 
{
	if (g_pHudContextMenu) 
		g_pHudContextMenu->Display(visible);
}