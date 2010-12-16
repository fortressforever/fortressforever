/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   15:58
	filename: 	F:\cvs\code\cl_dll\ff\ff_hud_quantitybar.h
	file path:	F:\cvs\code\cl_dll\ff
	file base:	ff_hud_quantitybar
	file ext:	h
	author:		Elmo
	
	purpose:	Customisable Quanitity indicator
*********************************************************************/

#ifndef FF_QUANTITYBAR_H
#define FF_QUANTITYBAR_H

#define QUANTITYBARFONTSIZES 10
#define QUANTITYBARICONSIZES 15

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
	class FFQuantityBar : public Panel
	{
	private:
		DECLARE_CLASS_SIMPLE(FFQuantityBar, Panel);
	public:
		FFQuantityBar(Panel *parent, const char *pElementName, int childId = -1);

		enum ColorMode {
			COLOR_MODE_CUSTOM=0,
			COLOR_MODE_STEPPED,
			COLOR_MODE_FADED,
			COLOR_MODE_TEAMCOLORED
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

		enum OrientationMode {
			ORIENTATION_HORIZONTAL=0,
			ORIENTATION_VERTICAL,
			ORIENTATION_HORIZONTAL_INVERTED,
			ORIENTATION_VERTICAL_INVERTED
		};

		void GetPanelPositioningData(int& iWidth, int& iHeight, int& iBarOffsetX, int&  iBarOffsetY);
		void SetPosOffset(int iLeft, int iTop);
		void SetStyle(KeyValues *kvStyleData, KeyValues *kvDefaultStyleData);

		void SetAmount(int iAmount);
		void SetAmountMax(int iAmountMax);

		void SetIconChar(char *newIconChar);
		void SetLabelText(char *newLabelText);
		void SetLabelText(wchar_t *newLabelText);

		void SetAmountSize( int newAmountSize );
		void SetIconSize( int newIconSize );
		void SetLabelSize( int newLabelSize );

		void SetAmountFontShadow(bool bHasShadow);
		void SetIconFontShadow(bool bHasShadow);
		void SetLabelFontShadow(bool bHasShadow);
		void SetIconFontGlyph(bool bIconIsGlyph);

		void SetBarWidth(int iBarWidth);
		void SetBarHeight(int iBarHeight);
		void SetBarSize(int iBarWidth, int iBarHeight);
		void SetBarBorderWidth(int iBarBorderWidth);
		void SetBarOrientation(int iOrientation);

		void SetLabelAlignmentHorizontal(int iAlignHLabel);
		void SetAmountAlignmentHorizontal(int iAlignHAmount);
		void SetIconAlignmentHorizontal(int iAlignHIcon);
		void SetLabelAlignmentVertical(int iAlignVLabel);
		void SetAmountAlignmentVertical(int iAlignVAmount);
		void SetIconAlignmentVertical(int iAlignVIcon);

		void ShowBar(bool bShowBar);
		void ShowBarBorder(bool bShowBarBorder);
		void ShowBarBackground(bool bShowBarBackground);
		void ShowAmount(bool bShowAmount);
		void ShowIcon(bool bShowIcon);
		void ShowLabel(bool bShowLabel);
		void ShowAmountMax(bool bShowAmountMax);

		void SetBarColor( Color newColorBar );
		void SetBarBorderColor( Color newBarBorderColor );
		void SetBarBackgroundColor( Color newBarBorderColor );
		void SetAmountColor( Color newAmountColor );
		void SetIconColor( Color newIconColor );
		void SetLabelColor( Color newLabelColor );
		void SetTeamColor( Color newTeamColor );

		void SetIconOffsetX(int iconOffsetX);
		void SetIconOffsetY(int iconOffsetY);
		void SetIconOffset(int iconOffsetX, int iconOffsetY);
		void SetLabelOffsetX(int labelOffsetX);
		void SetLabelOffsetY(int labelOffsetY);
		void SetLabelOffset(int labelOffsetX, int labelOffsetY);
		void SetAmountOffsetX(int amountOffsetX);
		void SetAmountOffsetY(int amountOffsetY);
		void SetAmountOffset(int amountOffsetX, int amountOffsetY);

		void SetBarColorMode( int iColorModeBar );
		void SetBarBorderColorMode( int iColorModeBarBorder );
		void SetBarBackgroundColorMode( int iColorModeBarBorder );
		void SetAmountColorMode( int iColorModeAmount );
		void SetIconColorMode( int iColorModeIcon );
		void SetLabelColorMode( int iColoModerLabel );
		
		void SetIntensityControl(int iRed, int iOrange,int iYellow, int iGreen, bool bInvertScale = false);
		void SetIntensityAmountScaled(bool bAmountScaled);
		void SetIntensityValuesFixed(bool bIntensityValuesFixed); 
		bool IsIntensityValuesFixed();

		int GetAmount();

		virtual void Paint();
		virtual void OnTick();

	protected:
		void CalculateTextAlignmentOffset(int &outX, int &outY, int &iWide, int &iTall, int iAlignH, int iAlignV, HFont hfFont, wchar_t* wszString);
		void RecalculateQuantity();
		void RecalculateIconPosition(bool bRecalculateComponentOffset = true);
		void RecalculateLabelPosition(bool bRecalculateComponentOffset = true);
		void RecalculateAmountPosition(bool bRecalculateComponentOffset = true);
		void RecalculateComponentOffset();
		void RecalculateAmountMaxStringSize();

		void RecalculateColor(int colorMode, Color &color, Color &colorCustom);

		bool m_bTriggerParentPositionUpdate;

		virtual void ApplySchemeSettings( IScheme *pScheme );
		
		HFont m_hfQuantityBarText[QUANTITYBARFONTSIZES * 3];
		HFont m_hfQuantityBarIcon[QUANTITYBARICONSIZES * 3];
		HFont m_hfQuantityBarGlyph[QUANTITYBARICONSIZES * 3];

		bool m_bAmountFontShadow;
		bool m_bLabelFontShadow;
		bool m_bIconFontShadow;
		bool m_bIconFontGlyph;

		float m_flScale;
		float m_flScaleX;
		float m_flScaleY;

		int m_iChildId;

		int m_iWidth;
		int m_iHeight;
		int m_iOffsetX;
		int m_iOffsetY;
		int m_iOffsetOverrideX;
		int m_iOffsetOverrideY;

		int m_iOffsetXIcon;
		int m_iOffsetYIcon;
		int m_iOffsetXLabel;
		int m_iOffsetYLabel;
		int m_iOffsetXAmount;
		int m_iOffsetYAmount;
		// *** These get calculated ::
		int m_iIconPosX;
		int m_iIconPosY;
		int m_iIconWidth;
		int m_iIconHeight;
		int m_iLabelPosX;
		int m_iLabelPosY;
		int m_iLabelWidth;
		int m_iLabelHeight;
		int m_iAmountPosX;
		int m_iAmountPosY;
		int m_iAmountWidth;
		int m_iAmountHeight;
		int m_iAmountMaxWidth;
		int m_iAmountMaxHeight;
		// *** These get calculated ^^

		//these are needed because the font is scaled and so changes with resolution (font, text alignment, and string content)
		int m_iIconAlignmentOffsetX;
		int m_iIconAlignmentOffsetY;
		int m_iLabelAlignmentOffsetX;
		int m_iLabelAlignmentOffsetY;
		int m_iAmountAlignmentOffsetX;
		int m_iAmountAlignmentOffsetY;

		int m_iBarX0QuantityOffset;
		int m_iBarY0QuantityOffset;
		int m_iBarX1QuantityOffset;
		int m_iBarY1QuantityOffset;

		int m_iAmount;
		int m_iMaxAmount;

		int m_iBarWidth;
		int m_iBarHeight;
		int m_iBarBorderWidth;
		int m_iBarOrientation;

		int m_iAlignHLabel;
		int m_iAlignHAmount;
		int m_iAlignHIcon;

		int m_iAlignVLabel;
		int m_iAlignVAmount;
		int m_iAlignVIcon;

		wchar_t m_wszAmount[ 5 ];
		wchar_t m_wszAmountMax[ 5 ];
		wchar_t m_wszIcon[ 2 ];
		wchar_t m_wszLabel[ 32 ];

		wchar_t m_wszAmountString[ 10 ];

		int m_iSizeAmount;
		int m_iSizeIcon;
		int m_iSizeLabel;

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
		int m_bIntensityInvertScale;

		Color m_ColorBarCustom;
		Color m_ColorBarBorderCustom;
		Color m_ColorBarBackgroundCustom;
		Color m_ColorIconCustom;
		Color m_ColorLabelCustom;
		Color m_ColorAmountCustom;
		Color m_ColorTeam;
		Color m_ColorIntensityFaded;
		Color m_ColorIntensityStepped;

		Color m_ColorBarBorder;
		Color m_ColorBarBackground;	
		Color m_ColorBar;	
		Color m_ColorIcon;
		Color m_ColorLabel;
		Color m_ColorAmount;

		int m_ColorModeBar;
		int m_ColorModeBarBorder;
		int m_ColorModeBarBackground;
		int m_ColorModeIcon;
		int m_ColorModeLabel;
		int m_ColorModeAmount;
	};
}
#endif