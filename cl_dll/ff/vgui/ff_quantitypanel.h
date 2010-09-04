/********************************************************************
	created:	2010/08
	filename: 	cl_dll\ff\ff_hud_quantitypanel.h
	file path:	cl_dll\ff
	file base:	ff_hud_quantitypanel
	file ext:	h
	author:		Elmo
	
	purpose:	Customisable Quanitity Panel for the HUD
*********************************************************************/

#ifndef FF_QUANTITYPANEL_H
#define FF_QUANTITYPANEL_H

#include "cbase.h"
#include <vgui_controls/Panel.h>
#include "keyValues.h"

#include "ff_quantitybar.h"

//this is defined in the custom hud options page too, keep it in sync
#define QUANTITYPANELFONTSIZES 10

namespace vgui
{
	class FFQuantityPanel : public Panel
	{
	DECLARE_CLASS_SIMPLE( FFQuantityPanel, Panel );
	public:
		FFQuantityPanel(Panel *parent, const char *panelName);

		virtual void Paint( void );
		virtual void OnTick( void );
		virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

		void SetHeaderText(wchar_t *newHeaderText);
		void SetHeaderIconChar(char *newHeaderIconChar);
	
		void UpdateQBPositions();
		void SetHeaderTextShadow(bool bHasShadow);
		void SetHeaderIconShadow(bool bHasShadow);

		void SetHeaderTextSize(int iSize);
		void SetHeaderIconSize(int iSize);

		void SetHeaderTextVisible(bool bIsVisible);
		void SetHeaderIconVisible(bool bIsVisible);

		void SetHeaderTextPosition( int iPositionX, int iPositionY );
		void SetHeaderIconPosition( int iPositionX, int iPositionY );

		void SetHeaderTextColor( Color newHeaderTextColor );
		void SetHeaderIconColor( Color newHeaderIconColor );
		
		void SetBarsVisible( bool bIsVisible );

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
		void AddPanelToHudOptions(const char* szSelf, const char* szParent);
		FFQuantityBar* AddChild(const char *pElementName);

		bool m_bDraw;

		char m_szSelf[128];
		char m_szParent[128];
		bool m_bAddToHud;
		bool m_bAddToHudSent;

		int m_iBorderWidth;

		float m_flScale;
		float m_flScaleX;
		float m_flScaleY;

		vgui::HFont m_hfText;
		vgui::HFont m_hfHeaderText[QUANTITYPANELFONTSIZES * 3];
		vgui::HFont m_hfHeaderIcon[QUANTITYPANELFONTSIZES * 3];
		
		bool m_bHeaderIconShadow;
		bool m_bHeaderTextShadow;

		bool m_bShowHeaderText;
		bool m_bShowHeaderIcon;

		int m_iHeaderTextSize;
		int m_iHeaderIconSize;
		
		int m_iTeamColourAlpha;
		Color m_ColorTeamBackground;
		Color m_ColorBackground;

		//maybe try to make variable rather than 6 max
		//if so, update fixed variables in OnPositions function too
		FFQuantityBar* m_QBars[6];
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

		int m_qb_iBarMarginHorizontal;
		int m_qb_iBarMarginVertical;
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