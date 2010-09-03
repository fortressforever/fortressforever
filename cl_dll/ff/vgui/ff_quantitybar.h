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
#include "cbase.h"
#include "hudelement.h"
#include "hud_macros.h"

/*
#include <KeyValues.h>
#include <vgui/ISystem.h>
*/

#include <vgui_controls/Panel.h>
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

class FFQuantityBar : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE(FFQuantityBar, vgui::Panel);
public:
	FFQuantityBar(vgui::Panel *parent, const char *pElementName, int childId = -1);

	enum ColorMode {
		COLOR_MODE_CUSTOM=0,
		COLOR_MODE_STEPPED,
		COLOR_MODE_FADED,
		COLOR_MODE_TEAMCOLORED
	};

	enum TextAlignment {
		TEXTALIGN_LEFT=0,
		TEXTALIGN_CENTER,
		TEXTALIGN_RIGHT
	};

	enum OrientationMode {
		ORIENTATION_HORIZONTAL=0,
		ORIENTATION_VERTICAL,
		ORIENTATION_HORIZONTAL_INVERTED,
		ORIENTATION_VERTICAL_INVERTED
	};
	void SetAmountFontShadow(bool bHasShadow);
	void SetIconFontShadow(bool bHasShadow);
	void SetLabelFontShadow(bool bHasShadow);
	void SetIconFontGlyph(bool bIconIsGlyph);

	void GetPanelPositioningData(int& iWidth, int& iHeight, int& iBarOffsetX, int&  iBarOffsetY);

	void SetAmount(int iAmount);
	void SetAmountMax(int iAmountMax);

	void SetIconChar(char *newIconChar);
	void SetLabelText(char *newLabelText);
	void SetLabelText(wchar_t *newLabelText);

	void SetBarWidth(int iBarWidth);
	void SetBarHeight(int iBarHeight);
	void SetBarSize(int iBarWidth, int iBarHeight);
	void SetBarBorderWidth(int iBarBorderWidth);
	void SetBarOrientation(int iOrientation);

	void SetLabelTextAlignment(int iLabelTextAlign);
	void SetAmountTextAlignment(int iAmountTextAlign);

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

	void SetAmountSize( int newAmountSize );
	void SetIconSize( int newIconSize );
	void SetLabelSize( int newLabelSize );

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
	virtual void SetPos(int iPositionX, int iPositionY);

protected:
	void CalculateTextAlignmentOffset(int &outX, int &outY, int &iWide, int &iTall, int iAlignmentMode, vgui::HFont hfFont, wchar_t* wszString);
	void RecalculateQuantity();
	void RecalculateIconPosition();
	void RecalculateLabelPosition();
	void RecalculateAmountPosition();

	void RecalculateColor(int colorMode, Color &color, Color &colorCustom);

 	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );
	
	vgui::HFont m_hfQuantityBarText[15];
	vgui::HFont m_hfQuantityBarIcon[15];
	vgui::HFont m_hfQuantityBarGlyph[15];

	bool m_bAmountFontShadow;
	bool m_bLabelFontShadow;
	bool m_bIconFontShadow;
	bool m_bIconFontGlyph;

	float m_flScale;
	float m_flScaleX;
	float m_flScaleY;

	int m_iChildId;

	int m_iTop;
	int m_iLeft;
	
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

	int m_iTextAlignLabel;
	int m_iTextAlignAmount;

	int m_iOffsetXIcon;
	int m_iOffsetYIcon;
	int m_iOffsetXLabel;
	int m_iOffsetYLabel;
	int m_iOffsetXAmount;
	int m_iOffsetYAmount;

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