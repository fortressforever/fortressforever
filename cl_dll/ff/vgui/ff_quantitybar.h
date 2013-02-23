/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   15:58
	filename: 	F:\cvs\code\cl_dll\ff\ff_hud_quantityitem.h
	file path:	F:\cvs\code\cl_dll\ff
	file base:	ff_hud_quantityitem
	file ext:	h
	author:		Elmo
	
	purpose:	Customisable Quanitity indicator
*********************************************************************/

#ifndef FF_QUANTITYITEM_H
#define FF_QUANTITYITEM_H

#define QUANTITYBARFONTSIZES 15
#define QUANTITYBARICONSIZES 20

#include "cbase.h"

/*
#include <KeyValues.h>
#include <vgui/ISystem.h>
*/

#include <vgui_controls/Panel.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>
namespace vgui
{
	class FFQuantityItem : public Panel
	{
	private:
		DECLARE_CLASS_SIMPLE( FFQuantityItem, Panel );
	public:
		FFQuantityItem( Panel *parent, const char *pElementName );

		enum ColorMode {
			ITEM_COLOR_MODE_CUSTOM=0,
			ITEM_COLOR_MODE_STEPPED,
			ITEM_COLOR_MODE_FADED,
			ITEM_COLOR_MODE_TEAMCOLORED
		};
		
		enum Position {
			ANCHORPOS_TOPLEFT=0,
			ANCHORPOS_TOPCENTER,
			ANCHORPOS_TOPRIGHT,
			ANCHORPOS_MIDDLELEFT,
			ANCHORPOS_MIDDLECENTER,
			ANCHORPOS_MIDDLERIGHT,
			ANCHORPOS_BOTTOMLEFT,
			ANCHORPOS_BOTTOMCENTER,
			ANCHORPOS_BOTTOMRIGHT
		};

		enum AlignmentHorizontal {
			ALIGN_LEFT=0,
			ALIGN_CENTER,
			ALIGN_RIGHT
		};

		enum AlignmentVertical {
			ALIGN_TOP=0,
			ALIGN_MIDDLE,
			ALIGN_BOTTOM
		};

		enum Orientation {
			ORIENTATION_HORIZONTAL=0,
			ORIENTATION_VERTICAL,
			ORIENTATION_HORIZONTAL_INVERTED,
			ORIENTATION_VERTICAL_INVERTED
		};

		bool GetItemDimentionsChanged();
		void SetItemDimentionsChanged( bool bItemDimentionsChanged );

		void GetPanelPositioningData( int& iWidth, int& iHeight, int& iBarOffsetX, int&  iBarOffsetY );
		virtual void SetPos( int iLeft, int iTop );
		void SetPosOffset( int iLeft, int iTop );
		void SetPaintOffset( int iLeft, int iTop );
		void SetStyle(KeyValues *kvStyleData, KeyValues *kvDefaultStyleData );

		void SetAmount( int iAmount );
		void SetAmountMax( int iAmountMax, bool bRecalculatePaintOffset = true );

		void SetIconChar(char *newIconChar, bool bRecalculatePaintOffset = true );
		void SetLabelText(char *newLabelText, bool bRecalculatePaintOffset = true );
		void SetLabelText(wchar_t *newLabelText, bool bRecalculatePaintOffset = true );

		void SetAmountSize( int newAmountSize, bool bRecalculatePaintOffset = true );
		void SetIconSize( int newIconSize, bool bRecalculatePaintOffset = true );
		void SetLabelSize( int newLabelSize, bool bRecalculatePaintOffset = true );

		void SetAmountFontShadow( bool bHasShadow );
		void SetIconFontShadow( bool bHasShadow );
		void SetLabelFontShadow( bool bHasShadow );

		void SetIconFontGlyph( bool bIconIsGlyph, bool bRecalculatePaintOffset = true );

		void SetAmountFontTahoma( bool bAmountFontTahoma, bool bRecalculatePaintOffset = true );
		void SetLabelFontTahoma( bool bLabelFontTahoma, bool bRecalculatePaintOffset = true );

		void SetBarWidth( int iBarWidth, bool bRecalculatePaintOffset = true );
		void SetBarHeight( int iBarHeight, bool bRecalculatePaintOffset = true );
		void SetBarSize( int iBarWidth, int iBarHeight, bool bRecalculatePaintOffset = true );
		void SetBarBorderWidth( int iBarBorderWidth, bool bRecalculatePaintOffset = true );
		void SetBarOrientation( int iBarOrientation );

		void SetAmountAnchorPosition( int iAmountAnchorPosition, bool bRecalculatePaintOffset = true );
		void SetIconAnchorPosition( int iIconAnchorPosition, bool bRecalculatePaintOffset = true );
		void SetLabelAnchorPosition( int iLabelAnchorPosition, bool bRecalculatePaintOffset = true );

		void SetAmountAlignmentHorizontal( int iAmountAlignHoriz, bool bRecalculatePaintOffset = true );
		void SetIconAlignmentHorizontal( int iIconAlignHoriz, bool bRecalculatePaintOffset = true );
		void SetLabelAlignmentHorizontal( int iLabelAlignHoriz, bool bRecalculatePaintOffset = true );

		void SetAmountAlignmentVertical( int iAmountAlignVert, bool bRecalculatePaintOffset = true );
		void SetIconAlignmentVertical( int iIconAlignVert, bool bRecalculatePaintOffset = true );
		void SetLabelAlignmentVertical( int iLabelAlignVert, bool bRecalculatePaintOffset = true );

		void SetAmountPositionOffsetX( int iAmountPositionOffsetX, bool bRecalculatePaintOffset = true );
		void SetAmountPositionOffsetY( int iAmountPositionOffsetY, bool bRecalculatePaintOffset = true );
		void SetAmountPositionOffset( int iAmountPositionOffsetX, int iAmountPositionOffsetY, bool bRecalculatePaintOffset = true );
		void SetIconPositionOffsetX( int iIconPositionOffsetX, bool bRecalculatePaintOffset = true );
		void SetIconPositionOffsetY( int iIconPositionOffsetY, bool bRecalculatePaintOffset = true );
		void SetIconPositionOffset( int iIconPositionOffsetX, int iIconPositionOffsetY, bool bRecalculatePaintOffset = true );
		void SetLabelPositionOffsetX( int iLabelPositionOffsetX, bool bRecalculatePaintOffset = true );
		void SetLabelPositionOffsetY( int iLabelPositionOffsetY, bool bRecalculatePaintOffset = true );
		void SetLabelPositionOffset( int iLabelPositionOffsetX, int iLabelPositionOffsetY, bool bRecalculatePaintOffset = true );

		void ShowBar( bool bShowBar, bool bRecalculatePaintOffset = true );
		void ShowBarBorder( bool bShowBarBorder, bool bRecalculatePaintOffset = true );
		void ShowBarBackground( bool bShowBarBackground, bool bRecalculatePaintOffset = true );
		void ShowAmount( bool bShowAmount, bool bRecalculatePaintOffset = true );
		void ShowAmountMax( bool bShowAmountMax, bool bRecalculatePaintOffset = true );
		void ShowIcon( bool bShowIcon, bool bRecalculatePaintOffset = true );
		void ShowLabel( bool bShowLabel, bool bRecalculatePaintOffset = true );

		void SetBarColor( Color newBarColor  );
		void SetBarBorderColor( Color newBarBorderColor  );
		void SetBarBackgroundColor( Color newBarBorderColor  );
		void SetAmountColor( Color newAmountColor  );
		void SetIconColor( Color newIconColor  );
		void SetLabelColor( Color newLabelColor  );
		void SetTeamColor( Color newTeamColor  );

		void SetBarColorMode( int iBarColorMode );
		void SetBarBorderColorMode( int iBarBorderColorMode );
		void SetBarBackgroundColorMode( int iBarBorderColorMode );
		void SetAmountColorMode( int iAmountColorMode );
		void SetIconColorMode( int iIconColorMode );
		void SetLabelColorMode( int iLabelColoModer );
		
		void SetIntensityControl( int iRed, int iOrange,int iYellow, int iGreen, bool bInvertScale = false );
		void SetIntensityAmountScaled( bool bAmountScaled );
		void SetIntensityValuesFixed( bool bIntensityValuesFixed ); 
		bool IsIntensityValuesFixed( );

		int GetAmount( );
		int GetMaxAmount( );

		virtual void Paint( );
		virtual void OnTick( );
		
		void SetDisabled(bool bState);
		bool IsDisabled( );

	protected:
		void CalculateTextPositionOffset( int &outX, int &outY, int iPos );
		void CalculateTextAlignmentOffset( int &outX, int &outY, int &iWide, int &iTall, int iAlignHoriz, int iAlignVert, HFont hfFont, wchar_t* wszString );
		void RecalculateQuantity( );
		void RecalculateIconPosition( bool bRecalculatePaintOffset = true );
		void RecalculateLabelPosition( bool bRecalculatePaintOffset = true );
		void RecalculateAmountPosition( );
		void RecalculateAmountMaxPosition( bool bRecalculatePaintOffset = true );
		void RecalculatePaintOffset( );
		void RecalculateColor( int colorMode, Color &color, Color &colorCustom );

		bool m_bItemDimentionsChanged;

		virtual void ApplySchemeSettings( IScheme *pScheme  );
		
		HFont m_hfQuantityItemText[QUANTITYBARFONTSIZES * 3];
		HFont m_hfQuantityItemTahomaText[QUANTITYBARFONTSIZES * 3];
		HFont m_hfQuantityItemIcon[QUANTITYBARICONSIZES * 3];
		HFont m_hfQuantityItemGlyph[QUANTITYBARICONSIZES * 3];

		bool m_bDisabled;

		float m_flScale;
		float m_flScaleX;
		float m_flScaleY;

	// *** QuantityItem size and paint position offsets ::
		int m_iWidth;
		int m_iHeight;
		int m_iPositionTop;
		int m_iPositionLeft;
		int m_iPositionOffsetTop;
		int m_iPositionOffsetLeft;

		//offset determined internally for positioning components around the bar
		//bar is initially considered at 0,0 icon is at -12,-12 so paint offset for all items is set to 12,12
		int m_iPaintOffsetX;
		int m_iPaintOffsetY;

		//offset override is determined by quantity panel positioning so that all bars are inline
		//IF using 1 bar per row: item 1 paint offset: 12,12 *** bar 2: 13,12 *** bar 3: 18,12
		//position override would be 13,12 for all 3 bars
		int m_iPaintOffsetOverrideX;
		int m_iPaintOffsetOverrideY;
	// *** QuantityItem size and paint position offsets ^^
		
	// *** These get calculated ::
		int m_iAmountPosX;
		int m_iAmountPosY;
		int m_iAmountWidth;
		int m_iAmountHeight;
		int m_iAmountMaxPosX;
		int m_iAmountMaxPosY;
		int m_iAmountMaxWidth;
		int m_iAmountMaxHeight;
		int m_iIconPosX;
		int m_iIconPosY;
		int m_iIconWidth;
		int m_iIconHeight;
		int m_iLabelPosX;
		int m_iLabelPosY;
		int m_iLabelWidth;
		int m_iLabelHeight;

		int m_iIconAlignmentOffsetX;
		int m_iIconAlignmentOffsetY;
		int m_iLabelAlignmentOffsetX;
		int m_iLabelAlignmentOffsetY;
		int m_iAmountAlignmentOffsetX;
		int m_iAmountAlignmentOffsetY;
		int m_iAmountMaxAlignmentOffsetX;
		int m_iAmountMaxAlignmentOffsetY;

		int m_iIconAnchorPositionX;
		int m_iIconAnchorPositionY;
		int m_iLabelAnchorPositionX;
		int m_iLabelAnchorPositionY;
		int m_iAmountAnchorPositionX;
		int m_iAmountAnchorPositionY;
		int m_iAmountMaxAnchorPositionX;
		int m_iAmountMaxAnchorPositionY;

		//Altered depending on the orienatation of the bar
		int m_iBarX0QuantityOffset;
		int m_iBarY0QuantityOffset;
		int m_iBarX1QuantityOffset;
		int m_iBarY1QuantityOffset;

		Color m_ColorBarBorder;
		Color m_ColorBarBackground;	
		Color m_ColorBar;	
		Color m_ColorIcon;
		Color m_ColorLabel;
		Color m_ColorAmount;
	// *** These get calculated ^^

		int m_iAmount;
		int m_iMaxAmount;

	// *** Style Settings ::
		int m_iBarWidth;
		int m_iBarHeight;
		int m_iBarBorderWidth;
		int m_iBarOrientation;

		int m_iAmountAnchorPosition;
		int m_iIconAnchorPosition;
		int m_iLabelAnchorPosition;

		int m_iAmountAlignHoriz;
		int m_iIconAlignHoriz;
		int m_iLabelAlignHoriz;

		int m_iAmountAlignVert;
		int m_iIconAlignVert;
		int m_iLabelAlignVert;

		int m_iAmountPositionOffsetX;
		int m_iAmountPositionOffsetY;
		int m_iIconPositionOffsetX;
		int m_iIconPositionOffsetY;
		int m_iLabelPositionOffsetX;
		int m_iLabelPositionOffsetY;

		//TEXT/FONT ::
		wchar_t m_wszAmount[ 5 ];
		wchar_t m_wszAmountMax[ 5 ];
		wchar_t m_wszIcon[ 2 ];
		wchar_t m_wszLabel[ 32 ];

		wchar_t m_wszAmountString[ 10 ];

		int m_iSizeAmount;
		int m_iSizeIcon;
		int m_iSizeLabel;
		
		bool m_bAmountFontShadow;
		bool m_bIconFontShadow;
		bool m_bLabelFontShadow;

		bool m_bIconFontGlyph;

		bool m_bAmountFontTahoma;
		bool m_bLabelFontTahoma;
		//TEXT/FONT ^^

		bool m_bShowBar;
		bool m_bShowBarBackground;
		bool m_bShowBarBorder;
		bool m_bShowIcon;
		bool m_bShowLabel;
		bool m_bShowAmount;
		bool m_bShowAmountMax;

		bool m_bIntensityAmountScaled; //make the amount a percentage and calculate against values
		bool m_bIntensityValuesFixed; //values fixed by code and can't be changed on the fly
		int m_iIntensityRed;
		int m_iIntensityOrange;
		int m_iIntensityYellow;
		int m_iIntensityGreen;
		bool m_bIntensityInvertScale;

		Color m_clrBarCustomColor;
		Color m_clrBarBorderCustomColor;
		Color m_clrBarBackgroundCustomColor;
		Color m_clrAmountCustomColor;
		Color m_clrIconCustomColor;
		Color m_clrLabelCustomColor;

		int m_iBarColorMode;
		int m_iBarBorderColorMode;
		int m_iBarBackgroundColorMode;
		int m_iAmountColorMode;
		int m_iIconColorMode;
		int m_iLabelColorMode;
	// *** Style Settings ^^

		Color m_ColorTeam;
		Color m_ColorIntensityFaded;
		Color m_ColorIntensityStepped;
	};
}
#endif