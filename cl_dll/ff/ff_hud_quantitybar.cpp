//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// ff_hud_quantitybar.cpp
//
// implementation of CHudQuantityBar class
//

#include "cbase.h"
#include "ff_hud_quantitybar.h"

using namespace vgui;

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define COLOR_MODE_CUSTOM 0
#define COLOR_MODE_STEPPED 1
#define COLOR_MODE_FADED 2
#define COLOR_MODE_TEAMCOLOURED 3

void CHudQuantityBar::setAmountFont(vgui::HFont newAmountFont) { m_hAmount = newAmountFont; }
void CHudQuantityBar::setIconFont(vgui::HFont newIconFont) { m_hIcon = newIconFont; }
void CHudQuantityBar::setLabelFont(vgui::HFont newLabelFont) { m_hLabel = newLabelFont; }

void CHudQuantityBar::setAmount(int newAmount)
{ 
	m_iAmount = newAmount; 

	char szAmount[10];
	Q_snprintf( szAmount, 5, "%i%", newAmount );
	vgui::localize()->ConvertANSIToUnicode( szAmount, m_wszAmount, sizeof( m_wszAmount ) );

	m_ColorIntensityFaded = getIntensityColor(m_iAmount, m_iMaxAmount, 2, m_ColorBar.a(), m_iIntenisityRed,m_iIntenisityOrange,m_iIntenisityYellow,m_iIntenisityGreen,m_bIntenisityInvertScale);
	m_ColorIntensityStepped = getIntensityColor(m_iAmount, m_iMaxAmount, 1, m_ColorBar.a(), m_iIntenisityRed,m_iIntenisityOrange,m_iIntenisityYellow,m_iIntenisityGreen,m_bIntenisityInvertScale);

}
void CHudQuantityBar::setAmountMax(int newAmountMax)
{
	m_iMaxAmount = newAmountMax; 
	
	char szAmountMax[10];
	Q_snprintf( szAmountMax, 5, "%i%", newAmountMax );
	vgui::localize()->ConvertANSIToUnicode( szAmountMax, m_wszAmountMax, sizeof( m_wszAmountMax ) );
}

void CHudQuantityBar::setIconChar(char *newIconChar)
{ 					
	char szIcon[5];
	Q_snprintf( szIcon, 2, "%s%", newIconChar );
	vgui::localize()->ConvertANSIToUnicode( szIcon, m_wszIcon, sizeof( m_wszIcon ) );
}
void CHudQuantityBar::setLabelText(char *newLabelText)
{
	wchar_t *pszTemp = vgui::localize()->Find( newLabelText );
	if( pszTemp )
		wcscpy( m_wszLabel, pszTemp );
	else
	{
		char szLabel[32];
		Q_snprintf( szLabel, 32, "%s%", newLabelText );
		vgui::localize()->ConvertANSIToUnicode( szLabel, m_wszLabel, sizeof( m_wszLabel ) );
	}
}
void CHudQuantityBar::setLabelText(wchar_t *newLabelText)
{
	wcscpy( m_wszLabel, newLabelText );
}

void CHudQuantityBar::setPosition(int newPositionX, int newPositionY) { m_iLeft = newPositionX; m_iTop = newPositionY; }
void CHudQuantityBar::setScaleX(float newScaleX){ m_flScaleX = newScaleX; }
void CHudQuantityBar::setScaleY(float newScaleY){ m_flScaleY = newScaleY; }
void CHudQuantityBar::setVisible(bool isVisible) { m_bVisible = isVisible; }

void CHudQuantityBar::setBarWidth(int newBarWidth) { m_iBarWidth = newBarWidth; }
void CHudQuantityBar::setBarHeight(int newBarHeight) { m_iBarHeight = newBarHeight; }
void CHudQuantityBar::setBarBorderWidth(int newBarBorderWidth) { m_iBarBorderWidth = newBarBorderWidth; };

void CHudQuantityBar::showBar(bool showBar) { m_bShowBar = showBar; }
void CHudQuantityBar::showBarBackground(bool showBarBackground) { m_bShowBarBackground = showBarBackground; }
void CHudQuantityBar::showBarBorder(bool showBarBorder) { m_bShowBarBorder = showBarBorder; }
void CHudQuantityBar::showIcon(bool showIcon) { m_bShowIcon = showIcon; }
void CHudQuantityBar::showLabel(bool showLabel) { m_bShowLabel = showLabel; }
void CHudQuantityBar::showAmount(bool showAmount) { m_bShowAmount = showAmount; }
void CHudQuantityBar::showAmountMax(bool showMax) { m_bShowAmountMax = showMax; }

void CHudQuantityBar::setBarColor( Color newBarColor ) { m_ColorBar = newBarColor; }
void CHudQuantityBar::setBarBorderColor( Color newBarBorderColor ) { m_ColorBarBorder = newBarBorderColor; }
void CHudQuantityBar::setBarBackgroundColor( Color newBarBackgroundColor ) { m_ColorBarBackground = newBarBackgroundColor; }
void CHudQuantityBar::setAmountColor( Color newAmountColor ) { m_ColorAmount = newAmountColor; }
void CHudQuantityBar::setIconColor( Color newIconColor ) { m_ColorIcon = newIconColor; }
void CHudQuantityBar::setLabelColor( Color newLabelColor ) { m_ColorLabel = newLabelColor; }
void CHudQuantityBar::setTeamColor( Color newTeamColor ) { m_ColorTeam = newTeamColor; }

void CHudQuantityBar::setBarColorMode( int newColorModeBar ) { m_ColorModeBar = newColorModeBar; }
void CHudQuantityBar::setBarBorderColorMode( int newColorModeBarBorder ) { m_ColorModeBarBorder = newColorModeBarBorder; }
void CHudQuantityBar::setBarBackgroundColorMode( int newColorModeBarBackround ) { m_ColorModeBarBackground = newColorModeBarBackround; }
void CHudQuantityBar::setAmountColorMode( int newColorModeAmount ) { m_ColorModeAmount = newColorModeAmount; }
void CHudQuantityBar::setIconColorMode( int newColorModeIcon ) { m_ColorModeIcon =  newColorModeIcon; }
void CHudQuantityBar::setLabelColorMode( int newColorModeLabel ) { m_ColorModeLabel = newColorModeLabel; }

void CHudQuantityBar::setBarOffsetX(int barOffsetX) { m_iOffsetXBar = barOffsetX; }
void CHudQuantityBar::setBarOffsetY(int barOffsetY) { m_iOffsetYBar = barOffsetY; }
void CHudQuantityBar::setIconOffsetX(int iconOffsetX) { m_iOffsetXIcon = iconOffsetX; }
void CHudQuantityBar::setIconOffsetY(int iconOffsetY) { m_iOffsetYIcon = iconOffsetY; }
void CHudQuantityBar::setLabelOffsetX(int labelOffsetX) { m_iOffsetXLabel = labelOffsetX; }
void CHudQuantityBar::setLabelOffsetY(int labelOffsetY) { m_iOffsetYLabel = labelOffsetY; }
void CHudQuantityBar::setAmountOffsetX(int amountOffsetX) { m_iOffsetXAmount = amountOffsetX; }
void CHudQuantityBar::setAmountOffsetY(int amountOffsetY) { m_iOffsetYAmount = amountOffsetY; }

int CHudQuantityBar::getAmount() {return m_iAmount; }

void CHudQuantityBar::setIntensityControl(int red, int orange,int yellow, int green)
{
	setIntensityControl(red,orange,yellow,green,false);
}
void CHudQuantityBar::setIntensityControl(int red, int orange,int yellow, int green, bool invertScale)
{
	m_iIntenisityRed = red;
	m_iIntenisityOrange = orange;
	m_iIntenisityYellow = yellow;
	m_iIntenisityGreen = green;
	m_bIntenisityInvertScale = invertScale;
}

void CHudQuantityBar::paintBackground()
{
	if((m_bShowBarBackground || m_bShowBarBorder) && m_bVisible)
	{
		int iBarHeight = m_iBarHeight * 1/m_flScaleY;
		int iBarWidth = m_iBarWidth * 1/m_flScaleY;
		
		int iOffsetXBar = m_iOffsetXBar * 1/m_flScaleX;
		int iOffsetYBar = m_iOffsetYBar * 1/m_flScaleY;

		int iTop = m_iTop * 1/m_flScaleY;
		int iLeft = m_iLeft * 1/m_flScaleY;
		int iBottom = iTop + iBarHeight;
		int iRight = iLeft + iBarWidth;

		if(m_bShowBarBackground)
		{
			Color barBackgroundColor;	
			if(m_ColorModeBarBackground == 1 )
				barBackgroundColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorBarBackground.a());
			else if(m_ColorModeBarBackground == 2 ) 
				barBackgroundColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorBarBackground.a());
			else if(m_ColorModeBarBackground == 3 ) 
				barBackgroundColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorBarBackground.a());
			else
				barBackgroundColor = m_ColorBarBackground;

			surface()->DrawSetColor( barBackgroundColor );
			surface()->DrawFilledRect( iLeft + iOffsetXBar, iTop + iOffsetYBar, iRight + iOffsetXBar, iBottom + iOffsetYBar);
		}

		if(m_bShowBarBorder)
		{
			Color barBorderColor;
			if(m_ColorModeBarBorder == 1 )
				barBorderColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),barBorderColor.a());
			else if(m_ColorModeBarBorder == 2 ) 
				barBorderColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorBarBorder.a());
			else if(m_ColorModeBarBorder == 3 ) 
				barBorderColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorBarBorder.a());
			else 
				barBorderColor = m_ColorBarBorder;
			surface()->DrawSetColor( barBorderColor );
			for(int i = 1; m_flScaleX>m_flScaleY ? i<=m_iBarBorderWidth*1/m_flScaleX : i<=m_iBarBorderWidth*1/m_flScaleY;++i)
			{
				surface()->DrawOutlinedRect( iLeft + iOffsetXBar - i, iTop + iOffsetYBar - i, iRight + iOffsetXBar + i, iBottom + iOffsetYBar + i);
			}
		}
	}
}

void CHudQuantityBar::paint()
{
	if(m_bVisible)
	{
		int iBarHeight = m_iBarHeight * 1/m_flScaleY;
		int iBarWidth = m_iBarWidth * 1/m_flScaleY;
		
		int iOffsetXBar = m_iOffsetXBar * 1/m_flScaleX;
		int iOffsetYBar = m_iOffsetYBar * 1/m_flScaleY;
		int iOffsetXIcon = m_iOffsetXIcon * 1/m_flScaleX;
		int iOffsetYIcon = m_iOffsetYIcon * 1/m_flScaleY;
		int iOffsetXLabel = m_iOffsetXLabel * 1/m_flScaleX;
		int iOffsetYLabel = m_iOffsetYLabel * 1/m_flScaleY;
		int iOffsetXAmount = m_iOffsetXAmount * 1/m_flScaleX;
		int iOffsetYAmount = m_iOffsetYAmount * 1/m_flScaleY;

		int iTop = m_iTop * 1/m_flScaleY;
		int iLeft = m_iLeft * 1/m_flScaleY;
		int iBottom = iTop + iBarHeight;

		/*
		if(!m_hAmount)
			m_hAmount = m_hQuantityBarFont;
		if(m_bShowIcon && !m_hIcon)
			m_hIcon = m_hQuantityBarIconFont;
		if(m_bShowLabel && !m_hLabel)
			m_hLabel = m_hQuantityBarFont;
		*/


		if(m_bShowBar)
		{
			Color barColor;	
			if(m_ColorModeBar == 1 )
				barColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorBar.a());
			else if(m_ColorModeBar == 2 ) 
				barColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorBar.a());
			else if(m_ColorModeBar == 3 ) 
				barColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorBar.a());
			else
				barColor = m_ColorBar;

			surface()->DrawSetColor( barColor );
			surface()->DrawFilledRect( iLeft + iOffsetXBar, iTop + iOffsetYBar, iLeft + iOffsetXBar + (iBarWidth * m_iAmount/m_iMaxAmount), iBottom + iOffsetYBar);
		}

		if(m_bShowIcon)
		{	
			int iconWidth, iconHeight;
			Color iconColor;
			if(m_ColorModeIcon == 1 )
				iconColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorIcon.a());
			else if(m_ColorModeIcon == 2 ) 
				iconColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorIcon.a());
			else if(m_ColorModeIcon == 3 ) 
				iconColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorIcon.a());
			else
				iconColor = m_ColorIcon;

			surface()->GetTextSize(m_hIcon, m_wszIcon, iconWidth, iconHeight);
			surface()->DrawSetTextFont(m_hIcon);
			surface()->DrawSetTextColor( iconColor );
			surface()->DrawSetTextPos(iLeft + iOffsetXIcon - iconWidth/2, iTop + iOffsetYIcon + (iBarHeight+2-iconHeight)/2 + 1);
			surface()->DrawUnicodeString( m_wszIcon );
		}
		if(m_bShowLabel)
		{
			int labelWidth, labelHeight;
			Color labelColor;		
			if(m_ColorModeLabel == 1 )
				labelColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorLabel.a());
			else if(m_ColorModeLabel == 2 ) 
				labelColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorLabel.a());
			else if(m_ColorModeLabel == 3 ) 
				labelColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorLabel.a());
			else
				labelColor = m_ColorLabel;
		
			surface()->GetTextSize(m_hLabel, m_wszLabel, labelWidth, labelHeight);
			surface()->DrawSetTextFont(m_hLabel);
			surface()->DrawSetTextColor( labelColor );
			surface()->DrawSetTextPos(iLeft + iOffsetXLabel, iTop + (iBarHeight+2-labelHeight)/2 + iOffsetYLabel);
			surface()->DrawUnicodeString( m_wszLabel );
		}

		wchar_t wszString[ 10 ];

		if(m_bShowAmount)
		{
			if(!m_bShowAmountMax)
			{
				if(m_iMaxAmount == 100)// already a percentage no need to convert
					_snwprintf( wszString, 255, L"%s%%", m_wszAmount );
				else
				{
					char szPercent[ 5 ];
					wchar_t wszPercent[ 10 ];
					float percent = ((float)m_iAmount/(float)m_iMaxAmount*100.0f);
					Q_snprintf( szPercent, 5, "%i%", (int)percent);

					vgui::localize()->ConvertANSIToUnicode( szPercent, wszPercent, sizeof( wszPercent ) );

					_snwprintf( wszString, 9, L"%s%%", wszPercent );
				}
			}
			else
			{
				_snwprintf( wszString, 9, L"%s/%s", m_wszAmount, m_wszAmountMax );
			}
			
			int amountWidth, amountHeight;
			Color amountColor;
			if(m_ColorModeAmount == 1 )
				amountColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorAmount.a());
			else if(m_ColorModeAmount == 2 ) 
				amountColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorAmount.a());
			else if(m_ColorModeAmount == 3 ) 
				amountColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorAmount.a());
			else
				amountColor = m_ColorAmount;

			surface()->GetTextSize(m_hAmount, wszString, amountWidth, amountHeight);
			surface()->DrawSetTextFont(m_hAmount);
			surface()->DrawSetTextColor( amountColor );
			surface()->DrawSetTextPos(iLeft - amountWidth/2 + iOffsetXAmount, iTop + (iBarHeight+2-amountHeight)/2 + iOffsetYAmount);
			surface()->DrawUnicodeString( wszString );
		}
	}
}