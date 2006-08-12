/********************************************************************
	created:	2006/08/12
	created:	12:8:2006   15:39
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_panel.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_panel
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Adds textures to panels
*********************************************************************/

#include "cbase.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include "convar.h"

#include "c_ff_player.h"
#include "ff_panel.h"

ConVar cl_teamcolourhud("cl_teamcolourhud", "0");

namespace vgui
{
	void FFPanel::ApplySettings(KeyValues *inResourceData)
	{
		const char *pszFG = inResourceData->GetString("ForegroundTexture", NULL);
		const char *pszBG = inResourceData->GetString("BackgroundTexture", NULL);

		m_pHudBackground = (pszBG ? gHUD.GetIcon(pszBG) : NULL);
		m_pHudForeground = (pszFG ? gHUD.GetIcon(pszFG) : NULL);

		Panel::ApplySettings(inResourceData);
	}

	void FFPanel::ApplySchemeSettings(IScheme *pScheme)
	{
		m_HudForegroundColour = GetSchemeColor("HudItem.Foreground", pScheme);
		m_HudBackgroundColour = GetSchemeColor("HudItem.Background", pScheme);

		Panel::ApplySchemeSettings(pScheme);
	}

	void FFPanel::PaintBackground()
	{
		Color &bg = m_HudBackgroundColour;
		Color &fg = m_HudForegroundColour;

		C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

		if (cl_teamcolourhud.GetBool())
		{
			Color HudBackgroundColour = Color( pPlayer->GetTeamColor().r(), pPlayer->GetTeamColor().g(), pPlayer->GetTeamColor().b(), 175 ) ;
			Color HudForegroundColour = Color( pPlayer->GetTeamColor().r(), pPlayer->GetTeamColor().g(), pPlayer->GetTeamColor().b(), 215 ) ;

			bg = HudBackgroundColour;
			fg = HudForegroundColour;
		}

		if (m_pHudBackground)
			m_pHudBackground->DrawSelf(0, 0, bg);

		if (m_pHudForeground)
			m_pHudForeground->DrawSelf(0, 0, fg);

		Panel::PaintBackground();
	}
}
