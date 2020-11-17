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

#define QUANTITYITEMTEXTSIZES 15
#define QUANTITYITEMICONSIZES 20

#include "cbase.h"

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
		
		HFont m_hfQuantityItemText[QUANTITYITEMTEXTSIZES * 3];
		HFont m_hfQuantityItemTahomaText[QUANTITYITEMTEXTSIZES * 3];
		HFont m_hfQuantityItemIcon[QUANTITYITEMICONSIZES * 3];
		HFont m_hfQuantityItemGlyph[QUANTITYITEMICONSIZES * 3];

		bool m_bDisabled;
		bool m_bHasStyleData;

		int m_iScreenWide;
		int m_iScreenTall;

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
		wchar_t m_wszAmount[ 4 ];
		wchar_t m_wszAmountMax[ 4 ];
		wchar_t m_wszIcon[ 2 ];
		wchar_t m_wszLabel[ 32 ];

		wchar_t m_wszAmountDisplay[ 9 ];
		wchar_t m_wszAmountMaxDisplay[ 9 ];

		int m_iSizeAmount;
		int m_iSizeIcon;
		int m_iSizeLabel;
		
		bool m_bAmountFontShadow;
		bool m_bIconFontShadow;
		bool m_bLabelFontShadow;

		bool m_bIconFontGlyph;

		bool m_bAmountFontTahoma;
		bool m_bLabelFontTahoma;

		vgui::HFont m_hfAmount;
		vgui::HFont m_hfIcon;
		vgui::HFont m_hfLabel;
		//TEXT/FONT ^^

		bool m_bShowBar;
		bool m_bShowBarBackground;
		bool m_bShowBarBorder;
		bool m_bShowIcon;
		bool m_bShowLabel;
		bool m_bShowAmount;
		bool m_bShowAmountMax;
		bool m_bShowAmountSticky;

		bool m_bIntensityAmountScaled; //make the amount a percentage and calculate against values
		bool m_bIntensityValuesFixed; //values fixed by code and can't be changed on the fly
		int m_iIntensityRed;
		int m_iIntensityOrange;
		int m_iIntensityYellow;
		int m_iIntensityGreen;
		int m_bIntensityInvertScale;

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

		Color m_clrTeam;
		Color m_ColorIntensityFaded;
		Color m_ColorIntensityStepped;

		vgui::HFont GetFont(vgui::HFont* hfFamily, int iSize, bool bUseModifier);

		bool SetIconFontGlyph( bool bIconIsGlyph );
		bool SetAmountFontTahoma( bool bAmountFontTahoma );
		bool SetLabelFontTahoma( bool bLabelFontTahoma );

		bool SetAmountFontShadow( bool bUseShadow );
		bool SetIconFontShadow( bool bUseShadow );
		bool SetLabelFontShadow( bool bUseShadow );

		bool SetAmountSize( int newAmountSize );
		bool SetIconSize( int newIconSize );
		bool SetLabelSize( int newLabelSize );

		bool SetBarWidth( int iBarWidth );
		bool SetBarHeight( int iBarHeight );
		bool SetBarBorderWidth( int iBarBorderWidth );
		bool SetBarOrientation( int iBarOrientation );

		bool SetAmountAnchorPosition( int iAmountAnchorPosition );
		bool SetIconAnchorPosition( int iIconAnchorPosition );
		bool SetLabelAnchorPosition( int iLabelAnchorPosition );

		bool SetAmountAlignmentHorizontal( int iAmountAlignHoriz );
		bool SetIconAlignmentHorizontal( int iIconAlignHoriz );
		bool SetLabelAlignmentHorizontal( int iLabelAlignHoriz );

		bool SetAmountAlignmentVertical( int iAmountAlignVert );
		bool SetIconAlignmentVertical( int iIconAlignVert );
		bool SetLabelAlignmentVertical( int iLabelAlignVert );

		bool SetAmountPositionOffsetX( int iAmountPositionOffsetX );
		bool SetAmountPositionOffsetY( int iAmountPositionOffsetY );
		bool SetIconPositionOffsetX( int iIconPositionOffsetX );
		bool SetIconPositionOffsetY( int iIconPositionOffsetY );
		bool SetLabelPositionOffsetX( int iLabelPositionOffsetX );
		bool SetLabelPositionOffsetY( int iLabelPositionOffsetY );

		bool ShowBar( bool bShowBar );
		bool ShowBarBorder( bool bShowBarBorder );
		bool ShowBarBackground( bool bShowBarBackground );
		bool ShowAmount( bool bShowAmount );
		bool ShowAmountMax( bool bShowAmountMax );
		bool ShowAmountSticky( bool bShowAmountSticky );
		bool ShowIcon( bool bShowIcon );
		bool ShowLabel( bool bShowLabel );

		bool SetCustomBarColor( int iRed, int iGreen, int iBlue, int iAlpha );
		bool SetCustomBarBorderColor( int iRed, int iGreen, int iBlue, int iAlpha );
		bool SetCustomBarBackgroundColor( int iRed, int iGreen, int iBlue, int iAlpha );
		bool SetCustomAmountColor( int iRed, int iGreen, int iBlue, int iAlpha );
		bool SetCustomIconColor( int iRed, int iGreen, int iBlue, int iAlpha );
		bool SetCustomLabelColor( int iRed, int iGreen, int iBlue, int iAlpha );
		bool SetTeamColor( Color clrTeam  );

		bool SetBarColorMode( int iBarColorMode );
		bool SetBarBorderColorMode( int iBarBorderColorMode );
		bool SetBarBackgroundColorMode( int iBarBorderColorMode );
		bool SetAmountColorMode( int iAmountColorMode );
		bool SetIconColorMode( int iIconColorMode );
		bool SetLabelColorMode( int iLabelColoModer );

		void CalculateTextPositionOffset( int &iX, int &iY, int iPos );
		void CalculatePositionOffset( int &iX, int &iY, int iWide, int iTall, int iAlignHoriz, int iAlignVert );
		void CalculateTextAlignmentOffset( int &iX, int &iY, int &iWide, int &iTall, int iAlignHoriz, int iAlignVert, HFont hfFont, wchar_t* wszString );
		void RecalculateQuantity( );

		void RecalculateIconFont( );
		void RecalculateAmountFont( );
		void RecalculateLabelFont( );

		void RecalculateAmountMaxDisplay();

		void RecalculateIconPosition( );
		void RecalculateLabelPosition( );
		void RecalculateAmountPosition( );
		void RecalculateAmountMaxPosition( );
		void RecalculatePaintOffset( );

		void RecalculateColor( Color &color, int iColorMode, Color &colorCustom );
		Color GetRgbColor( int iColorMode, Color &colorCustom );
		void SetColor( Color &color, Color &rgbColor, Color &alphaColor );
		void SetIntensityColor( Color &color, int iColorMode, Color &alphaColor );

		void DrawText(wchar_t* wszText, HFont font, Color color, int iXPosition, int iYPosition);

		int GetInt(const char *keyName, KeyValues *kvStyleData, int iDefaultValue = -1);
		int GetInt(const char *keyName, KeyValues *kvStyleData, KeyValues *kvDefaultStyleData, int iDefaultValue = -1);
		KeyValues* GetData(const char *keyName, KeyValues *kvStyleData, KeyValues *kvDefaultStyleData);

		int m_iPositionalHashCode;

	public:
		FFQuantityItem( Panel *parent, const char *pElementName );
		
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

		void CalculatePositionalHashCode();
		int GetPositionalHashCode();

		void GetPanelPositioningData( int& iWidth, int& iHeight, int& iBarOffsetX, int&  iBarOffsetY );
		virtual void SetPos( int iLeft, int iTop );
		void SetPosOffset( int iLeft, int iTop );
		void SetPaintOffset( int iLeft, int iTop );
		void SetStyle(KeyValues *kvStyleData, KeyValues *kvDefaultStyleData );

		bool SetAmount( int iAmount );
		bool SetAmountMax( int iAmountMax, bool bRecalculatePaintOffset = false );

		void SetIconChar( char *newIconChar, bool bRecalculatePaintOffset = false );
		void SetLabelText( char *newLabelText, bool bRecalculatePaintOffset = false );
		
		void SetIntensityControl( int iRed, int iOrange,int iYellow, int iGreen );
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
		virtual void ApplySchemeSettings( IScheme *pScheme  );
	};
}
#endif