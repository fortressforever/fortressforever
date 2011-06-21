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
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Panel.h>
#include "color.h"

namespace vgui
{
	class FFPanel : public Panel
	{
		DECLARE_CLASS_SIMPLE( FFPanel, Panel ); 

	public:
		FFPanel() : Panel() { InitFFPanel(); }
		FFPanel(Panel *parent) : Panel(parent) { InitFFPanel(); }
		FFPanel(Panel *parent, const char *panelName) : Panel(parent, panelName) { InitFFPanel(); }
		FFPanel(Panel *parent, const char *panelName, HScheme scheme) : Panel(parent, panelName, scheme) { InitFFPanel(); }

		virtual void FFPanel::ApplySettings(KeyValues *inResourceData);
		virtual void FFPanel::ApplySchemeSettings(IScheme *pScheme);
		virtual void FFPanel::PaintBackground();

	private:

		void InitFFPanel();

	protected:

		CHudTexture		*m_pHudForeground;
		CHudTexture		*m_pHudBackground;

		Color			m_HudForegroundColour;
		Color			m_HudBackgroundColour;
		Color			m_TeamColorHudBackgroundColour;
	};
}

#endif