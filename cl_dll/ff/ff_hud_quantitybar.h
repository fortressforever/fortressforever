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
#include <vgui/IScheme.h>

class CHudQuantityBar
{
public:
	CHudQuantityBar()
	{
		m_flScaleX = 1.0f;
		m_flScaleY = 1.0f;
		m_iTop = 0;
		m_iLeft = 0;

		setAmountMax(100); //use the function to generate the text

		m_bShowBar = true;
		m_bShowBarBackground = true;
		m_bShowBarBorder = true;
		m_bShowIcon = true;
		m_bShowLabel = true;
		m_bShowAmount = true;
		m_bShowAmountMax = false;
		m_bVisible = true;

		m_iBarWidth = 100;
		m_iBarHeight = 25;

		m_ColorBarBorder.SetColor(255,255,255,255);
		m_ColorBarBackground.SetColor(255,255,255,255);
		m_ColorBar.SetColor(255,255,255,255);
		m_ColorIcon.SetColor(255,255,255,255);
		m_ColorLabel.SetColor(255,255,255,255);
		m_ColorAmount.SetColor(255,255,255,255);
	}

	void setScaleX(float newScaleX);
	void setScaleY(float newScaleY);

	void setBarOffset(int x, int y);

	void setAmountFont(vgui::HFont newAmountFont);
	void setIconFont(vgui::HFont newIconFont);
	void setLabelFont(vgui::HFont newLabelFont);

	void setAmount(int newAmount);
	void setAmountMax(int newAmountMax);

	void setIconChar(char *newIconChar);
	void setLabelText(char *newLabelText);
	void setLabelText(wchar_t *newLabelText);

	void setBarWidth(int newBarWidth);
	void setBarHeight(int newBarHeight);
	void setPosition(int newPositionX, int newPositionY);
	void setBarBorderWidth(int newBarBorderWidth);

	void setResolutionScaleX(float newScaleX);
	void setResolutionScaleY(float newScaleY);

	void showBar(bool showBar);
	void showBarBorder(bool showBarBorder);
	void showBarBackground(bool showBarBackground);
	void showAmount(bool showAmount);
	void showIcon(bool showIcon);
	void showLabel(bool showLabel);
	void showAmountMax(bool showAmountMax);

	void setVisible(bool isVisible);

	void setBarColor( Color newColorBar );
	void setBarBorderColor( Color newBarBorderColor );
	void setBarBackgroundColor( Color newBarBorderColor );
	void setAmountColor( Color newAmountColor );
	void setIconColor( Color newIconColor );
	void setLabelColor( Color newLabelColor );
	void setTeamColor( Color newTeamColor );

	void setBarOffsetX(int barOffsetX);
	void setBarOffsetY(int barOffsetY);
	void setIconOffsetX(int iconOffsetX);
	void setIconOffsetY(int iconOffsetY);
	void setLabelOffsetX(int labelOffsetX);
	void setLabelOffsetY(int labelOffsetY);
	void setAmountOffsetX(int amountOffsetX);
	void setAmountOffsetY(int amountOffsetY);

	void setBarColorMode( int newColorModeBar );
	void setBarBorderColorMode( int newColorModeBarBorder );
	void setBarBackgroundColorMode( int newColorModeBarBorder );
	void setAmountColorMode( int newColorModeAmount );
	void setIconColorMode( int newColorModeIcon );
	void setLabelColorMode( int newColoModerLabel );
	
	void setIntensityControl(int red, int orange,int yellow, int green);
	void setIntensityControl(int red, int orange,int yellow, int green, bool invert);

	int getAmount();

	void paint();
	void paintBackground();

private:
	vgui::HFont m_hAmount;
	vgui::HFont m_hIcon;
	vgui::HFont m_hLabel;

	float m_flScaleX;
	float m_flScaleY;
	int m_iTop;
	int m_iLeft;

	int m_iAmount;
	int m_iMaxAmount;

	int m_iBarWidth;
	int m_iBarHeight;
	int m_iBarBorderWidth;

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
