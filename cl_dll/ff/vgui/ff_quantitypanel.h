/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   15:58
	filename: 	F:\cvs\code\cl_dll\ff\ff_hud_quantitypanel.h
	file path:	F:\cvs\code\cl_dll\ff
	file base:	ff_hud_quantitybar
	file ext:	h
	author:		Elmo
	
	purpose:	Customisable Quanitity Panel
*********************************************************************/

#ifndef FF_QUANTITYPANEL_H
#define FF_QUANTITYPANEL_H

#include "cbase.h"
#include <vgui_controls/Panel.h>

namespace vgui
{
	class FFQuantityPanel : public Panel
	{
	public:
		FFQuantityPanel() : Panel() { }
		FFQuantityPanel(Panel *parent) : Panel(parent) { }
		FFQuantityPanel(Panel *parent, const char *panelName) : Panel(parent, panelName) { }
		FFQuantityPanel(Panel *parent, const char *panelName, HScheme scheme) : Panel(parent, panelName, scheme) { }

		virtual void FFQuantityPanel::Paint( void );
		virtual void FFQuantityPanel::ApplySchemeSettings( vgui::IScheme *pScheme );
		
		void SetHeaderText(wchar_t *newHeaderText);
		void SetHeaderIconChar(char *newHeaderIconChar);

		void SetHeaderTextFont(vgui::HFont newHeaderTextFont);
		void SetHeaderIconFont(vgui::HFont newHeaderIconFont);

		void SetHeaderTextColor( Color newHeaderTextColor );
		void SetHeaderIconColor( Color newHeaderIconColor );
		void SetBackgroundColor( Color newBackgroundColor );
		void SetBorderColor( Color newBorderColor );

	protected:

		int m_iBorderWidth;

		vgui::HFont m_hfHeaderText;
		vgui::HFont m_hfHeaderIcon;
		vgui::HFont m_hfText;

		wchar_t m_wszHeaderText[50];
		wchar_t m_wszHeaderIcon[2];
		
		Color m_ColorHeaderText;
		Color m_ColorHeaderIcon;
		Color m_ColorText;
		Color m_ColorBackground;
		Color m_ColorBorder;
	};
}

#endif