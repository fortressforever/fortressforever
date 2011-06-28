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
#include <vgui/IScheme.h>
#include <vgui/IVGui.h>
#include "convar.h"

#include "c_playerresource.h"
#include "c_ff_team.h"
#include "ff_panel.h"
#include "ff_utils.h"

ConVar cl_teamcolourhud( "cl_teamcolourhud", "1", FCVAR_ARCHIVE );

extern ConVar cl_teamcolourhud;

extern C_PlayerResource *g_PR;

namespace vgui
{
	//-----------------------------------------------------------------------------
	// Purpose: Load the textures
	//-----------------------------------------------------------------------------
	void FFPanel::ApplySettings( KeyValues *inResourceData )
	{
		const char *pszFG = inResourceData->GetString( "ForegroundTexture", NULL );
		const char *pszBG = inResourceData->GetString( "BackgroundTexture", NULL );

		m_pHudBackground = ( pszBG ? gHUD.GetIcon(pszBG) : NULL );
		m_pHudForeground = ( pszFG ? gHUD.GetIcon(pszFG) : NULL );

		BaseClass::ApplySettings(inResourceData);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load the colours
	//-----------------------------------------------------------------------------
	void FFPanel::ApplySchemeSettings( IScheme *pScheme )
	{
		//reinit the players team so that the team colour gets recalculated
		//this is needed when changing resolutions!
		m_iPlayerTeam = -1;

		m_HudForegroundColour = GetSchemeColor( "HudItem.Foreground", pScheme );
		m_HudBackgroundColour = GetSchemeColor( "HudItem.Background", pScheme );
		m_TeamColorHudBackgroundColour = GetSchemeColor( "TeamColorHud.BackgroundAlpha", pScheme );

		BaseClass::ApplySchemeSettings( pScheme );
	}

	//-----------------------------------------------------------------------------
	// Purpose: The background consists of a separate foreground and background
	//			texture.
	//-----------------------------------------------------------------------------
	void FFPanel::PaintBackground()
	{
		// Don't draw if we're a spectator or we have no class	
		//if( FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer ) )
		//	return;

		if ( m_pHudBackground )
			if ( cl_teamcolourhud.GetBool() )
				m_pHudBackground->DrawSelf( 0, 0, m_TeamColorHudBackgroundColour );
			else
				m_pHudBackground->DrawSelf( 0, 0, m_HudBackgroundColour );
		if ( m_pHudForeground )
			m_pHudForeground->DrawSelf( 0, 0, m_HudForegroundColour );

		BaseClass::PaintBackground();
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: The background consists of a separate foreground and background
	//			texture.
	//-----------------------------------------------------------------------------
	void FFPanel::OnTick()
	{
		if( !engine->IsInGame() )
		{
			m_pFFPlayer = NULL; // not in game so make m_pFFPlayer NULL!
			return;
		}

		m_pFFPlayer = CFFPlayer::GetLocalFFPlayer();

		if (!m_pFFPlayer) 
			return;

		if( cl_teamcolourhud.GetBool() )
		{
			int iPlayerTeam = m_pFFPlayer->GetTeamNumber();
			
			if( m_iPlayerTeam != iPlayerTeam )
			{
				m_iPlayerTeam = iPlayerTeam;
				Color newTeamColor = g_PR->GetTeamColor( m_iPlayerTeam );
				m_TeamColorHudBackgroundColour.SetColor( newTeamColor.r(), newTeamColor.g(), newTeamColor.b(), m_TeamColorHudBackgroundColour.a() );
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load the correct scheme file
	//-----------------------------------------------------------------------------
	void FFPanel::InitFFPanel()
	{
		m_iPlayerTeam = -1;
		
		ivgui()->AddTickSignal( GetVPanel(), 500 );
		//HScheme scheme = vgui::scheme()->LoadSchemeFromFile( "resource/HudScheme.res", "HudScheme");
		//SetScheme(scheme);
	}
}
