/********************************************************************
	created:	2006/08/12
	created:	12:8:2006   15:39
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_panel.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_panel
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	All our custom HUD panels should be derived from here
				Adds textures to HUD panels
*********************************************************************/

#include "cbase.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IScheme.h>
#include "convar.h"

#include "c_ff_player.h"
#include "c_playerresource.h"
#include "c_ff_team.h"
#include "ff_panel.h"
#include "ff_utils.h"

ConVar cl_teamcolourhud("cl_teamcolourhud", "1", FCVAR_ARCHIVE);

extern ConVar cl_teamcolourhud;

extern C_PlayerResource *g_PR;

namespace vgui
{
	//-----------------------------------------------------------------------------
	// Purpose: Load the textures
	//-----------------------------------------------------------------------------
	void FFPanel::ApplySettings(KeyValues *inResourceData)
	{
		const char *pszFG = inResourceData->GetString("ForegroundTexture", NULL);
		const char *pszBG = inResourceData->GetString("BackgroundTexture", NULL);

		m_pHudBackground = (pszBG ? gHUD.GetIcon(pszBG) : NULL);
		m_pHudForeground = (pszFG ? gHUD.GetIcon(pszFG) : NULL);

		Panel::ApplySettings(inResourceData);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load the colours
	//-----------------------------------------------------------------------------
	void FFPanel::ApplySchemeSettings(IScheme *pScheme)
	{
		m_HudForegroundColour = GetSchemeColor("HudItem.Foreground", pScheme);
		m_HudBackgroundColour = GetSchemeColor("HudItem.Background", pScheme);
		m_TeamColorHudBackgroundColour = GetSchemeColor("TeamColorHud.BackgroundAlpha", pScheme);

		Panel::ApplySchemeSettings(pScheme);
	}

	//-----------------------------------------------------------------------------
	// Purpose: The background consists of a separate foreground and background
	//			texture.
	//-----------------------------------------------------------------------------
	void FFPanel::PaintBackground()
	{
		Color bg = m_HudBackgroundColour;
		Color fg = m_HudForegroundColour;
		Color teamcolorbg = m_TeamColorHudBackgroundColour;

		C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

		// Don't draw if we're a spectator or we have no class
		if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
			return;

		if (cl_teamcolourhud.GetBool())
		{
			Color HudBackgroundColour = Color( g_PR->GetTeamColor( pPlayer->GetTeamNumber() ).r(), g_PR->GetTeamColor( pPlayer->GetTeamNumber() ).g(), g_PR->GetTeamColor( pPlayer->GetTeamNumber() ).b(), teamcolorbg.a() /*175*/ ) ;
			//Color HudForegroundColour = Color( pPlayer->GetTeamColor().r(), pPlayer->GetTeamColor().g(), pPlayer->GetTeamColor().b(), 215 ) ;

			bg = HudBackgroundColour;
			//fg = HudForegroundColour;
		}

		if (m_pHudBackground)
			m_pHudBackground->DrawSelf(0, 0, bg);

		if (m_pHudForeground)
			m_pHudForeground->DrawSelf(0, 0, fg);

		Panel::PaintBackground();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load the correct scheme file
	//-----------------------------------------------------------------------------
	void FFPanel::InitFFPanel()
	{
		//HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/HudScheme.res", "HudScheme");
		//SetScheme(scheme);
	}
}
