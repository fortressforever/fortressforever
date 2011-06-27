/********************************************************************
	created:	2006/08/12
	created:	12:8:2006   15:18
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_panel.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_panel
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Adds textures to panels
*********************************************************************/

#ifndef FF_PANEL_H
#define FF_PANEL_H

#include "cbase.h"
#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include "color.h"
#include "c_ff_player.h"

namespace vgui
{
	class FFPanel : public Panel
	{
	DECLARE_CLASS_SIMPLE( FFPanel, Panel );
	public:
		FFPanel() : BaseClass() { InitFFPanel(); }
		FFPanel(Panel *parent) : BaseClass(parent) { InitFFPanel(); }
		FFPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName) { InitFFPanel(); }
		FFPanel(Panel *parent, const char *panelName, HScheme scheme) : BaseClass(parent, panelName, scheme) { InitFFPanel(); }

	private:
		void InitFFPanel();

	protected:
		CFFPlayer		*m_pFFPlayer;
		CHudTexture		*m_pHudForeground;
		CHudTexture		*m_pHudBackground;

		int				m_iPlayerTeam;

		Color			m_HudForegroundColour;
		Color			m_HudBackgroundColour;
		Color			m_TeamColorHudBackgroundColour;
		
		void ApplySettings(KeyValues *inResourceData);
		void ApplySchemeSettings(IScheme *pScheme);
		void PaintBackground();
		void OnTick();
	};
}

#endif