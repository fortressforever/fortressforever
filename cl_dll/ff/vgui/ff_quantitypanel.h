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
			//SetHeaderTextPosition(30,15);
			//SetHeaderIconPosition(10,10);

			//these values are irrelevant and get overridden by cvars
			
			//if the child class should take precidence over general
			//buildstate_sg_x over buildstate_x
			m_bChildOverride = false;

			m_iX = 0;
			m_iY = 0;
			m_iWidth = 0;
			m_iHeight = 0;
			
			m_iHeaderTextX = 30;
			m_iHeaderTextY = 15;
			m_iHeaderIconX = 10;
			m_iHeaderIconY = 10;

			m_qb_iSpacingX = 5;
			m_qb_iSpacingY = 5;
			
			m_qb_iPositionX = 15;
			m_qb_iPositionY = 40;
			m_qb_iColumns = 1;

			/*
				rest is -1 so that it updates using cvar values
				had a weird case where quantity bar colormode default was 1
				this was 2 and cvar was 2 - It didn't detect a change and so 
				didn't update the colormode thus maintaining a colourmode of 1 

				obviously true false can't be helped... just making sure they're the same as quanitty bar defaults!
			*/

			m_qb_iIntensity_red = -1;
			m_qb_iIntensity_orange = -1;
			m_qb_iIntensity_yellow = -1;
			m_qb_iIntensity_green = -1;

			m_qb_bShowBar = true;
			m_qb_bShowBarBackground = true;
			m_qb_bShowBarBorder = true;
			m_qb_bShowIcon = true;
			m_qb_bShowLabel = true;
			m_qb_bShowAmount = true;

			m_qb_iBarWidth = -1;
			m_qb_iBarHeight = -1;
			m_qb_iBarBorderWidth = -1;

			m_qb_iColorBar_r = -1;
			m_qb_iColorBar_g = -1;
			m_qb_iColorBar_b = -1;
			m_qb_iColorBar_a = -1;
			m_qb_iColorBarBackground_r = -1;
			m_qb_iColorBarBackground_g = -1;
			m_qb_iColorBarBackground_b = -1;
			m_qb_iColorBarBackground_a = -1;
			m_qb_iColorBarBorder_r = -1;
			m_qb_iColorBarBorder_g = -1;
			m_qb_iColorBarBorder_b = -1;
			m_qb_iColorBarBorder_a = -1;
			m_qb_iColorIcon_r = -1;
			m_qb_iColorIcon_g = -1;
			m_qb_iColorIcon_b = -1;
			m_qb_iColorIcon_a = -1;
			m_qb_iColorLabel_r = -1; 
			m_qb_iColorLabel_g = -1;
			m_qb_iColorLabel_b = -1;
			m_qb_iColorLabel_a = -1;
			m_qb_iColorAmount_r = -1; 
			m_qb_iColorAmount_g = -1; 
			m_qb_iColorAmount_b = -1;
			m_qb_iColorAmount_a = -1;

			m_qb_bShadowIcon = false;
			m_qb_bShadowLabel = false;
			m_qb_bShadowAmount = false; 

			m_qb_iColorModeBar = -1;
			m_qb_iColorModeBarBackground = -1;
			m_qb_iColorModeBarBorder = -1;
			m_qb_iColorModeIcon = -1;
			m_qb_iColorModeLabel = -1;
			m_qb_iColorModeAmount = -1; 
			m_qb_iColorModeIdent = -1;

			m_qb_iOffsetBarX = -1;
			m_qb_iOffsetBarY = -1;
			m_qb_iOffsetIconX = -1;
			m_qb_iOffsetIconY = -1;
			m_qb_iOffsetLabelX = -1;
			m_qb_iOffsetLabelY = -1;
			m_qb_iOffsetAmountX = -1;
			m_qb_iOffsetAmountY = -1;
			m_qb_iOffsetIdentX = -1;
			m_qb_iOffsetIdentY = -1;
		}

		virtual void FFQuantityPanel::PaintBackground( void );
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
		
		void OnBarSizeChanged();
		void OnIntensityValuesChanged();

		void OnShowBarChanged();
		void OnShowBarBackgroundChanged();
		void OnShowBarBorderChanged();
		void OnBarBorderWidthChanged();

		void OnShowIconChanged();
		void OnShowLabelChanged();
		void OnShowAmountChanged();

		void OnSizeIconChanged();
		void OnSizeLabelChanged();
		void OnSizeAmountChanged();
		
		void OnColorBarChanged();
		void OnColorBarBackgroundChanged();
		void OnColorBarBorderChanged();
		
		void OnColorIconChanged();
		void OnColorLabelChanged();
		void OnColorAmountChanged();
		
		void UpdateQBPositions();

		void OnColorModeBarChanged();
		void OnColorModeBarBackgroundChanged();
		void OnColorModeBarBorderChanged();
		void OnColorModeIconChanged();
		void OnColorModeLabelChanged();
		void OnColorModeAmountChanged();

		void OnIconShadowChanged();
		void OnLabelShadowChanged();
		void OnAmountShadowChanged();
		
		void OnIconOffsetChanged();
		void OnLabelOffsetChanged();
		void OnAmountOffsetChanged();
	private:
		MESSAGE_FUNC_PARAMS( OnChildDimentionsChanged, "ChildDimentionsChanged", data );

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
		//if so, update fixed variables in OnPositions function too
		CHudQuantityBar* m_QBars[6];
		int m_iQBars;

		bool m_bCheckUpdates;
		bool m_bUpdateRecieved[6];

		bool m_bChildOverride;

		int m_iX;
		int m_iY;
		int m_iWidth;
		int m_iHeight;
		int m_iHorizontalAlign;
		int m_iVerticalAlign;

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

		int m_qb_iSpacingX;
		int m_qb_iSpacingY;
		int m_qb_iPositionX;
		int m_qb_iPositionY;
		int m_qb_iColumns;

		int m_qb_iIntensity_red;
		int m_qb_iIntensity_orange;
		int m_qb_iIntensity_yellow;
		int m_qb_iIntensity_green;

		bool m_qb_bShowBar;
		bool m_qb_bShowBarBackground;
		bool m_qb_bShowBarBorder;
		bool m_qb_bShowIcon;
		bool m_qb_bShowLabel;
		bool m_qb_bShowAmount;

		int m_qb_iBarWidth;
		int m_qb_iBarHeight;
		int m_qb_iBarBorderWidth;

		int m_qb_iColorBar_r;
		int m_qb_iColorBar_g;
		int m_qb_iColorBar_b;
		int m_qb_iColorBar_a;
		int m_qb_iColorBarBackground_r;
		int m_qb_iColorBarBackground_g;
		int m_qb_iColorBarBackground_b;
		int m_qb_iColorBarBackground_a;
		int m_qb_iColorBarBorder_r;
		int m_qb_iColorBarBorder_g;
		int m_qb_iColorBarBorder_b;
		int m_qb_iColorBarBorder_a;
		int m_qb_iColorIcon_r;
		int m_qb_iColorIcon_g;
		int m_qb_iColorIcon_b;
		int m_qb_iColorIcon_a;
		int m_qb_iColorLabel_r; 
		int m_qb_iColorLabel_g;
		int m_qb_iColorLabel_b;
		int m_qb_iColorLabel_a;
		int m_qb_iColorAmount_r; 
		int m_qb_iColorAmount_g; 
		int m_qb_iColorAmount_b;
		int m_qb_iColorAmount_a;

		bool m_qb_bShadowIcon;
		bool m_qb_bShadowLabel;
		bool m_qb_bShadowAmount; 

		int m_qb_iSizeIcon;
		int m_qb_iSizeLabel;
		int m_qb_iSizeAmount; 

		int m_qb_iColorModeBar;
		int m_qb_iColorModeBarBackground;
		int m_qb_iColorModeBarBorder;
		int m_qb_iColorModeIcon;
		int m_qb_iColorModeLabel;
		int m_qb_iColorModeAmount; 
		int m_qb_iColorModeIdent;

		int m_qb_iOffsetBarX;
		int m_qb_iOffsetBarY;
		int m_qb_iOffsetIconX;
		int m_qb_iOffsetIconY;
		int m_qb_iOffsetLabelX; 
		int m_qb_iOffsetLabelY;
		int m_qb_iOffsetAmountX;
		int m_qb_iOffsetAmountY;
		int m_qb_iOffsetIdentX;
		int m_qb_iOffsetIdentY;
	};
}

#endif