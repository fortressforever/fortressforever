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
	DECLARE_CLASS_SIMPLE( FFQuantityPanel, Panel );
	public:
		FFQuantityPanel() : Panel() { FFQuantityPanelInit(); }
		FFQuantityPanel(Panel *parent) : Panel(parent) { FFQuantityPanelInit(); }
		FFQuantityPanel(Panel *parent, const char *panelName) : Panel(parent, panelName) { FFQuantityPanelInit(); }
		FFQuantityPanel(Panel *parent, const char *panelName, HScheme scheme) : Panel(parent, panelName, scheme) { FFQuantityPanelInit(); }

		void FFQuantityPanelInit() {
			m_flScale = 1.0f;
			SetHeaderTextPosition(30,15);
			SetHeaderIconPosition(10,10);
			m_iQuantityBarSpacingX = 10;
			m_iQuantityBarSpacingY = 10;
			m_iQuantityBarPositionX = 35;
			m_iQuantityBarPositionY = 35;
		}

		virtual void FFQuantityPanel::Paint( void );
		virtual void FFQuantityPanel::ApplySchemeSettings( vgui::IScheme *pScheme );
		virtual void FFQuantityPanel::OnTick( void );
		
		void SetHeaderText(wchar_t *newHeaderText);
		void SetHeaderIconChar(char *newHeaderIconChar);

		void SetHeaderTextFont(vgui::HFont newHeaderTextFont);
		void SetHeaderIconFont(vgui::HFont newHeaderIconFont);

		void SetHeaderTextPosition( int iPositionX, int iPositionY );
		void SetHeaderIconPosition( int iPositionX, int iPositionY );

		void SetHeaderTextColor( Color newHeaderTextColor );
		void SetHeaderIconColor( Color newHeaderIconColor );

	protected:
		int m_iBorderWidth;

		float m_flScale;

		vgui::HFont m_hfHeaderText;
		vgui::HFont m_hfHeaderIcon;
		vgui::HFont m_hfText;

		int m_iHeaderTextX;
		int m_iHeaderTextY;
		int m_iHeaderIconX;
		int m_iHeaderIconY;

		wchar_t m_wszHeaderText[50];
		wchar_t m_wszHeaderIcon[2];
		
		Color m_ColorHeaderText;
		Color m_ColorHeaderIcon;
		Color m_ColorText;

		int m_iQuantityBarSpacingX;
		int m_iQuantityBarSpacingY;
		int m_iQuantityBarPositionX;
		int m_iQuantityBarPositionY;
		int m_iNumQuantityBarColumns;

		int* m_iColumnOffset;
		int* m_iColumnWidth;
		int* m_iRowOffset;
		int* m_iRowWidth;
	};
}

#endif