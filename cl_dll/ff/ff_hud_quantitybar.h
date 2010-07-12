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

class CHudQuantityBar : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE(CHudQuantityBar, vgui::Panel);
public:
	CHudQuantityBar(vgui::Panel *parent, const char *pElementName) : CHudElement(pElementName), vgui::Panel(parent, pElementName) 
	{
		SetParent( parent );
		SetSize(vgui::scheme()->GetProportionalScaledValue(640), vgui::scheme()->GetProportionalScaledValue(480));

		m_flScaleX = 1.0f;
		m_flScaleY = 1.0f;
		m_iTop = 50;
		m_iLeft = 50;

		m_iBarWidth = 60;
		m_iBarHeight = 13;
		m_iBarBorderWidth = 1;
		
		m_iBarOrientation = ORIENTATION_HORIZONTAL;

		m_iAlignLabel = ALIGN_RIGHT;
		m_iAlignAmount = ALIGN_CENTER;

		m_iOffsetXBar = 0;
		m_iOffsetYBar = 0;
		m_iOffsetXIcon = 5;
		m_iOffsetYIcon = 0;
		m_iOffsetXLabel = -5;
		m_iOffsetYLabel = 0;
		m_iOffsetXAmount = 35;
		m_iOffsetYAmount = 0;

		m_iIntenisityRed = 20;
		m_iIntenisityOrange = 50;
		m_iIntenisityYellow = 80;
		m_iIntenisityGreen = 100;
		m_bIntenisityInvertScale = false;

		m_ColorModeBar = COLOR_MODE_STEPPED;
		m_ColorModeBarBorder = COLOR_MODE_CUSTOM;
		m_ColorModeBarBackground = COLOR_MODE_STEPPED;
		m_ColorModeIcon = COLOR_MODE_CUSTOM;
		m_ColorModeLabel = COLOR_MODE_CUSTOM;
		m_ColorModeAmount = COLOR_MODE_CUSTOM;

		SetAmountMax(100); //use the function to generate the text

		m_bShowBar = true;
		m_bShowBarBackground = true;
		m_bShowBarBorder = true;
		m_bShowIcon = true;
		m_bShowLabel = true;
		m_bShowAmount = true;
		m_bShowAmountMax = true;
		m_bVisible = true;

		m_ColorBarBorder.SetColor(192,192,192,255);
		m_ColorBarBackground.SetColor(255,255,255,80);
		m_ColorBar.SetColor(255,255,255,255);
		m_ColorIcon.SetColor(255,255,255,255);
		m_ColorLabel.SetColor(255,255,255,255);
		m_ColorAmount.SetColor(255,255,255,255);
	}

	enum {
		COLOR_MODE_CUSTOM=0,
		COLOR_MODE_STEPPED,
		COLOR_MODE_FADED,
		COLOR_MODE_TEAMCOLORED
	};

	enum {
		ALIGN_LEFT=0,
		ALIGN_CENTER,
		ALIGN_RIGHT
	};

	enum {
		ORIENTATION_HORIZONTAL=0,
		ORIENTATION_VERTICAL,
		ORIENTATION_HORIZONTAL_INVERTED,
		ORIENTATION_VERTICAL_INVERTED
	};

	void SetScaleX(float flScaleX);
	void SetScaleY(float flScaleY);

	//void SetBarOffset(int x, int y);

	void SetAmountFont(vgui::HFont newAmountFont);
	void SetIconFont(vgui::HFont newIconFont);
	void SetLabelFont(vgui::HFont newLabelFont);

	void SetAmount(int iAmount);
	void SetAmountMax(int iAmountMax);

	void SetIconChar(char *newIconChar);
	void SetLabelText(char *newLabelText);
	void SetLabelText(wchar_t *newLabelText);

	void SetPosition(int iPositionX, int iPositionY);
	void SetBarWidth(int iBarWidth);
	void SetBarHeight(int iBarHeight);
	void SetBarBorderWidth(int iBarBorderWidth);
	void SetBarOrientation(int iOrientation);

	void SetLabelAlignment(int iLabelAlign);
	void SetAmountAlignment(int iAmountAlign);

	void ShowBar(bool bShowBar);
	void ShowBarBorder(bool bShowBarBorder);
	void ShowBarBackground(bool bShowBarBackground);
	void ShowAmount(bool bShowAmount);
	void ShowIcon(bool bShowIcon);
	void ShowLabel(bool bShowLabel);
	void ShowAmountMax(bool bShowAmountMax);

	//void SetVisible(bool bIsVisible);

	void SetBarColor( Color newColorBar );
	void SetBarBorderColor( Color newBarBorderColor );
	void SetBarBackgroundColor( Color newBarBorderColor );
	void SetAmountColor( Color newAmountColor );
	void SetIconColor( Color newIconColor );
	void SetLabelColor( Color newLabelColor );
	void SetTeamColor( Color newTeamColor );

	void SetBarOffsetX(int barOffsetX);
	void SetBarOffsetY(int barOffsetY);
	void SetIconOffsetX(int iconOffsetX);
	void SetIconOffsetY(int iconOffsetY);
	void SetLabelOffsetX(int labelOffsetX);
	void SetLabelOffsetY(int labelOffsetY);
	void SetAmountOffsetX(int amountOffsetX);
	void SetAmountOffsetY(int amountOffsetY);

	void SetBarColorMode( int iColorModeBar );
	void SetBarBorderColorMode( int iColorModeBarBorder );
	void SetBarBackgroundColorMode( int iColorModeBarBorder );
	void SetAmountColorMode( int iColorModeAmount );
	void SetIconColorMode( int iColorModeIcon );
	void SetLabelColorMode( int iColoModerLabel );
	
	void SetIntensityControl(int iRed, int iOrange,int iYellow, int iGreen);
	void SetIntensityControl(int iRed, int iOrange,int iYellow, int iGreen, bool bInvertScale);

	int GetAmount();

	virtual void Paint();
	virtual void PaintBackground();
protected:
 	virtual void ApplySchemeSettings( vgui::IScheme *pScheme );

	vgui::HFont m_hfAmount;
	vgui::HFont m_hfIcon;
	vgui::HFont m_hfLabel;
	
	vgui::HFont m_hfQuantityBarText;
	vgui::HFont m_hfQuantityBarIcon;
	vgui::HFont m_hfQuantityBarTextShadow;
	vgui::HFont m_hfQuantityBarIconShadow;

	float m_flScaleX;
	float m_flScaleY;
	int m_iTop;
	int m_iLeft;

	int m_iAmount;
	int m_iMaxAmount;

	int m_iBarWidth;
	int m_iBarHeight;
	int m_iBarBorderWidth;
	int m_iBarOrientation;

	int m_iAlignLabel;
	int m_iAlignAmount;

	int m_iOffsetXBar;
	int m_iOffsetYBar;
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

	bool m_bShowBar;
	bool m_bShowBarBackground;
	bool m_bShowBarBorder;
	bool m_bShowIcon;
	bool m_bShowLabel;
	bool m_bShowAmount;
	bool m_bShowAmountMax;

	bool m_bVisible;

	int m_iIntenisityRed;
	int m_iIntenisityOrange;
	int m_iIntenisityYellow;
	int m_iIntenisityGreen;
	int m_bIntenisityInvertScale;

	Color m_ColorBar;
	Color m_ColorBarBorder;
	Color m_ColorBarBackground;
	Color m_ColorIcon;
	Color m_ColorLabel;
	Color m_ColorAmount;
	Color m_ColorTeam;
	Color m_ColorIntensityFaded;
	Color m_ColorIntensityStepped;

	int m_ColorModeBar;
	int m_ColorModeBarBorder;
	int m_ColorModeBarBackground;
	int m_ColorModeIcon;
	int m_ColorModeLabel;
	int m_ColorModeAmount;
};