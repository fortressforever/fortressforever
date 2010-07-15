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
#include "ff_hud_quantitybar.h"

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
			m_flScaleX = 1.0f;
			m_flScaleY = 1.0f;
			m_iQBars = 0;
			SetHeaderTextPosition(30,15);
			SetHeaderIconPosition(10,10);
			m_iQuantityBarSpacingX = 5;
			m_iQuantityBarSpacingY = 5;
			m_iNumQuantityBarColumns = 1;
			m_iQuantityBarPositionX = 15;
			m_iQuantityBarPositionY = 40;
		}

		virtual void FFQuantityPanel::Paint( void );
		virtual void FFQuantityPanel::ApplySchemeSettings( vgui::IScheme *pScheme );
		virtual void FFQuantityPanel::OnTick( void );
		
		void SetHeaderText(wchar_t *newHeaderText);
		void SetHeaderIconChar(char *newHeaderIconChar);

		void SetHeaderTextShadow(bool bHasShadow);
		void SetHeaderIconShadow(bool bHasShadow);

		void SetHeaderTextPosition( int iPositionX, int iPositionY );
		void SetHeaderIconPosition( int iPositionX, int iPositionY );

		void SetHeaderTextColor( Color newHeaderTextColor );
		void SetHeaderIconColor( Color newHeaderIconColor );

		void UpdateQuantityBarPositions();
	private:
		MESSAGE_FUNC_PARAMS( OnChildDimentionsChanged, "ChildDimentionsChanged", data );
		void RecalculateHeaderTextDimentions();
		void RecalculateHeaderIconDimentions();

	protected:
		CHudQuantityBar* AddChild(const char *pElementName);

		int m_iBorderWidth;

		float m_flScale;
		float m_flScaleX;
		float m_flScaleY;

		vgui::HFont m_hfHeaderText[3];
		vgui::HFont m_hfHeaderIcon[3];
		
		bool m_bHeaderIconShadow;
		bool m_bHeaderTextShadow;

		//maybe try to make variable rather than 6 max
		//if so, update fixed variables in UpdateQuantityBarPositions()
		CHudQuantityBar* m_QBars[6];
		int m_iQBars;

		bool m_bCheckUpdates;
		bool m_bUpdateRecieved[6];

		int m_iHeaderTextX;
		int m_iHeaderTextY;
		int m_iHeaderIconX;
		int m_iHeaderIconY;

		wchar_t m_wszHeaderText[50];
		wchar_t m_wszHeaderIcon[2];
		
		Color m_ColorHeaderText;
		Color m_ColorHeaderIcon;
		Color m_ColorText;

		int m_iHeaderTextWidth;
		int m_iHeaderTextHeight;
		int m_iHeaderIconWidth;
		int m_iHeaderIconHeight;

		int m_iQuantityBarSpacingX;
		int m_iQuantityBarSpacingY;
		int m_iQuantityBarPositionX;
		int m_iQuantityBarPositionY;
		int m_iNumQuantityBarColumns;
	};
}

#endif