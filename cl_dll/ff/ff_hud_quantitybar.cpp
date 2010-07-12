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

#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

 void CHudQuantityBar::ApplySchemeSettings( vgui::IScheme *pScheme )
 { 
	m_hfQuantityBarText = pScheme->GetFont( "QuantityBar", IsProportional() );
	m_hfQuantityBarIcon = pScheme->GetFont( "QuantityBarIcon", IsProportional() );
	m_hfQuantityBarTextShadow = pScheme->GetFont( "QuantityBarShadow", IsProportional() );
	m_hfQuantityBarIconShadow = pScheme->GetFont( "QuantityBarIconShadow", IsProportional() );

	m_hfAmount = m_hfQuantityBarText;
	m_hfIcon = m_hfQuantityBarIcon;
	m_hfLabel = m_hfQuantityBarText;

 	BaseClass::ApplySchemeSettings( pScheme );
 }


void CHudQuantityBar::SetAmountFont(vgui::HFont newAmountFont) { m_hfAmount = newAmountFont; }
void CHudQuantityBar::SetIconFont(vgui::HFont newIconFont) { m_hfIcon = newIconFont; }
void CHudQuantityBar::SetLabelFont(vgui::HFont newLabelFont) { m_hfLabel = newLabelFont; }

void CHudQuantityBar::SetAmount(int iAmount)
{ 
	m_iAmount = iAmount; 

	char szAmount[10];
	Q_snprintf( szAmount, 5, "%i%", iAmount );
	vgui::localize()->ConvertANSIToUnicode( szAmount, m_wszAmount, sizeof( m_wszAmount ) );

	m_ColorIntensityFaded = getIntensityColor(m_iAmount, m_iMaxAmount, 2, m_ColorBar.a(), m_iIntenisityRed,m_iIntenisityOrange,m_iIntenisityYellow,m_iIntenisityGreen,m_bIntenisityInvertScale);
	m_ColorIntensityStepped = getIntensityColor(m_iAmount, m_iMaxAmount, 1, m_ColorBar.a(), m_iIntenisityRed,m_iIntenisityOrange,m_iIntenisityYellow,m_iIntenisityGreen,m_bIntenisityInvertScale);

}
void CHudQuantityBar::SetAmountMax(int iAmountMax)
{
	if(iAmountMax != 0) // don't allow 0 cause that doesn't make sense - plus it'll crash with divide by 0 when calculating the colour!
	{
		m_iMaxAmount = iAmountMax; 
	
		char szAmountMax[10];
		Q_snprintf( szAmountMax, 5, "%i%", iAmountMax );
		vgui::localize()->ConvertANSIToUnicode( szAmountMax, m_wszAmountMax, sizeof( m_wszAmountMax ) );
	}
}

void CHudQuantityBar::SetIconChar(char *newIconChar)
{ 					
	char szIcon[5];
	Q_snprintf( szIcon, 2, "%s%", newIconChar );
	vgui::localize()->ConvertANSIToUnicode( szIcon, m_wszIcon, sizeof( m_wszIcon ) );
}
void CHudQuantityBar::SetLabelText(char *newLabelText)
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
void CHudQuantityBar::SetLabelText(wchar_t *newLabelText)
{
	wcscpy( m_wszLabel, newLabelText );
}

void CHudQuantityBar::SetPosition(int iPositionX, int iPositionY) { m_iLeft = iPositionX; m_iTop = iPositionY; }
void CHudQuantityBar::SetScaleX(float flScaleX){ m_flScaleX = flScaleX; }
void CHudQuantityBar::SetScaleY(float flScaleY){ m_flScaleY = flScaleY; }
//void CHudQuantityBar::SetVisible(bool isVisible) { m_bVisible = isVisible; }

void CHudQuantityBar::SetBarWidth(int iBarWidth) { m_iBarWidth = iBarWidth; }
void CHudQuantityBar::SetBarHeight(int iBarHeight) { m_iBarHeight = iBarHeight; }
void CHudQuantityBar::SetBarBorderWidth(int iBarBorderWidth) { m_iBarBorderWidth = iBarBorderWidth; };

void CHudQuantityBar::SetBarOrientation(int iOrientation) { m_iBarOrientation = iOrientation; }

void CHudQuantityBar::ShowBar(bool bShowBar) { m_bShowBar = bShowBar; }
void CHudQuantityBar::ShowBarBackground(bool bShowBarBackground) { m_bShowBarBackground = bShowBarBackground; }
void CHudQuantityBar::ShowBarBorder(bool bShowBarBorder) { m_bShowBarBorder = bShowBarBorder; }
void CHudQuantityBar::ShowIcon(bool bShowIcon) { m_bShowIcon = bShowIcon; }
void CHudQuantityBar::ShowLabel(bool bShowLabel) { m_bShowLabel = bShowLabel; }
void CHudQuantityBar::ShowAmount(bool bShowAmount) { m_bShowAmount = bShowAmount; }
void CHudQuantityBar::ShowAmountMax(bool bShowMax) { m_bShowAmountMax = bShowMax; }

void CHudQuantityBar::SetBarColor( Color newBarColor ) { m_ColorBar = newBarColor; }
void CHudQuantityBar::SetBarBorderColor( Color newBarBorderColor ) { m_ColorBarBorder = newBarBorderColor; }
void CHudQuantityBar::SetBarBackgroundColor( Color newBarBackgroundColor ) { m_ColorBarBackground = newBarBackgroundColor; }
void CHudQuantityBar::SetAmountColor( Color newAmountColor ) { m_ColorAmount = newAmountColor; }
void CHudQuantityBar::SetIconColor( Color newIconColor ) { m_ColorIcon = newIconColor; }
void CHudQuantityBar::SetLabelColor( Color newLabelColor ) { m_ColorLabel = newLabelColor; }
void CHudQuantityBar::SetTeamColor( Color newTeamColor ) { m_ColorTeam = newTeamColor; }

void CHudQuantityBar::SetBarColorMode( int iColorModeBar ) { m_ColorModeBar = iColorModeBar; }
void CHudQuantityBar::SetBarBorderColorMode( int iColorModeBarBorder ) { m_ColorModeBarBorder = iColorModeBarBorder; }
void CHudQuantityBar::SetBarBackgroundColorMode( int iColorModeBarBackround ) { m_ColorModeBarBackground = iColorModeBarBackround; }
void CHudQuantityBar::SetAmountColorMode( int iColorModeAmount ) { m_ColorModeAmount = iColorModeAmount; }
void CHudQuantityBar::SetIconColorMode( int iColorModeIcon ) { m_ColorModeIcon =  iColorModeIcon; }
void CHudQuantityBar::SetLabelColorMode( int iColorModeLabel ) { m_ColorModeLabel = iColorModeLabel; }

void CHudQuantityBar::SetBarOffsetX(int barOffsetX) { m_iOffsetXBar = barOffsetX; }
void CHudQuantityBar::SetBarOffsetY(int barOffsetY) { m_iOffsetYBar = barOffsetY; }
void CHudQuantityBar::SetIconOffsetX(int iconOffsetX) { m_iOffsetXIcon = iconOffsetX; }
void CHudQuantityBar::SetIconOffsetY(int iconOffsetY) { m_iOffsetYIcon = iconOffsetY; }
void CHudQuantityBar::SetLabelOffsetX(int labelOffsetX) { m_iOffsetXLabel = labelOffsetX; }
void CHudQuantityBar::SetLabelOffsetY(int labelOffsetY) { m_iOffsetYLabel = labelOffsetY; }
void CHudQuantityBar::SetAmountOffsetX(int amountOffsetX) { m_iOffsetXAmount = amountOffsetX; }
void CHudQuantityBar::SetAmountOffsetY(int amountOffsetY) { m_iOffsetYAmount = amountOffsetY; }

int CHudQuantityBar::GetAmount() {return m_iAmount; }

void CHudQuantityBar::SetLabelAlignment(int iLabelAlign) { m_iAlignLabel = iLabelAlign; }
void CHudQuantityBar::SetAmountAlignment(int iAmountAlign) { m_iAlignAmount = iAmountAlign; }

void CHudQuantityBar::SetIntensityControl(int iRed, int iOrange,int iYellow, int iGreen)
{
	SetIntensityControl(iRed,iOrange,iYellow,iGreen,false);
}
void CHudQuantityBar::SetIntensityControl(int iRed, int iOrange,int iYellow, int iGreen, bool bInvertScale)
{
	m_iIntenisityRed = iRed;
	m_iIntenisityOrange = iOrange;
	m_iIntenisityYellow = iYellow;
	m_iIntenisityGreen = iGreen;
	m_bIntenisityInvertScale = bInvertScale;
}

void CHudQuantityBar::PaintBackground()
{
	if((m_bShowBarBackground || m_bShowBarBorder))
	{
		int iBarHeight = m_iBarHeight * 1/m_flScaleY;
		int iBarWidth = m_iBarWidth * 1/m_flScaleY;
		
		int iOffsetXBar = m_iOffsetXBar * 1/m_flScaleX;
		int iOffsetYBar = m_iOffsetYBar * 1/m_flScaleY;

		int iTop = m_iTop * 1/m_flScaleY;
		int iLeft = m_iLeft * 1/m_flScaleY;
		int iBottom = iTop + iBarHeight;
		int iRight = iLeft + iBarWidth;

		if(m_bShowBarBorder)
		{
			Color barBorderColor;
			if(m_ColorModeBarBorder == COLOR_MODE_STEPPED )
				barBorderColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),barBorderColor.a());
			else if(m_ColorModeBarBorder == COLOR_MODE_FADED ) 
				barBorderColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorBarBorder.a());
			else if(m_ColorModeBarBorder == COLOR_MODE_TEAMCOLORED ) 
				barBorderColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorBarBorder.a());
			else 
				barBorderColor = m_ColorBarBorder;
			vgui::surface()->DrawSetColor( barBorderColor );
			for(int i = 1; m_flScaleX>m_flScaleY ? i<=m_iBarBorderWidth*1/m_flScaleX : i<=m_iBarBorderWidth*1/m_flScaleY;++i)
			{
				vgui::surface()->DrawOutlinedRect( iLeft + iOffsetXBar - i, iTop + iOffsetYBar - i, iRight + iOffsetXBar + i, iBottom + iOffsetYBar + i);
			}
		}
	}
}

void CHudQuantityBar::Paint()
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
	int iRight = iLeft + iBarWidth;

	if(m_bShowBarBackground)
	{
		Color barBackgroundColor;	
		if(m_ColorModeBarBackground == COLOR_MODE_STEPPED )
			barBackgroundColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorBarBackground.a());
		else if(m_ColorModeBarBackground == COLOR_MODE_FADED ) 
			barBackgroundColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorBarBackground.a());
		else if(m_ColorModeBarBackground == COLOR_MODE_TEAMCOLORED ) 
			barBackgroundColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorBarBackground.a());
		else
			barBackgroundColor = m_ColorBarBackground;

		vgui::surface()->DrawSetColor( barBackgroundColor );
		vgui::surface()->DrawFilledRect( iLeft + iOffsetXBar, iTop + iOffsetYBar, iLeft + iBarWidth + iOffsetXBar, iBottom + iOffsetYBar);
	}

	if(m_bShowBar)
	{
		Color barColor;	
		if(m_ColorModeBar == COLOR_MODE_STEPPED )
			barColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorBar.a());
		else if(m_ColorModeBar == 2 ) 
			barColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorBar.a());
		else if(m_ColorModeBar == 3 ) 
			barColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorBar.a());
		else
			barColor = m_ColorBar;

		vgui::surface()->DrawSetColor( barColor );
		if(m_iBarOrientation == ORIENTATION_HORIZONTAL_INVERTED)
			vgui::surface()->DrawFilledRect( iRight + iOffsetXBar - (iBarWidth * m_iAmount/m_iMaxAmount), iTop + iOffsetYBar, iRight + iOffsetXBar, iBottom + iOffsetYBar);
		else if(m_iBarOrientation == ORIENTATION_VERTICAL)
			vgui::surface()->DrawFilledRect( iLeft + iOffsetXBar, iTop + iOffsetYBar, iRight + iOffsetXBar, iTop + iOffsetYBar + (iBarHeight * m_iAmount/m_iMaxAmount));
		else if(m_iBarOrientation == ORIENTATION_VERTICAL_INVERTED)
			vgui::surface()->DrawFilledRect( iLeft + iOffsetXBar, iBottom + iOffsetYBar - (iBarHeight * m_iAmount/m_iMaxAmount), iRight + iOffsetXBar, iBottom + iOffsetYBar);
		else //m_iBarOrientation == ORIENTATION_HORIZONTAL (or anything else)
			vgui::surface()->DrawFilledRect( iLeft + iOffsetXBar, iTop + iOffsetYBar, iLeft + iOffsetXBar + (iBarWidth * m_iAmount/m_iMaxAmount), iBottom + iOffsetYBar);
	}

	if(m_bShowIcon && m_hfIcon && m_wszIcon)
	{	
		int iconWidth, iconHeight;
		Color iconColor;
		if(m_ColorModeIcon == COLOR_MODE_STEPPED )
			iconColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorIcon.a());
		else if(m_ColorModeIcon == COLOR_MODE_FADED ) 
			iconColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorIcon.a());
		else if(m_ColorModeIcon == COLOR_MODE_TEAMCOLORED ) 
			iconColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorIcon.a());
		else
			iconColor = m_ColorIcon;

		vgui::surface()->GetTextSize(m_hfIcon, m_wszIcon, iconWidth, iconHeight);
		vgui::surface()->DrawSetTextFont(m_hfIcon);
		vgui::surface()->DrawSetTextColor( iconColor );
		vgui::surface()->DrawSetTextPos(iLeft + iOffsetXIcon - iconWidth/2, iTop + iOffsetYIcon + (iBarHeight+2-iconHeight)/2 + 1);
		vgui::surface()->DrawUnicodeString( m_wszIcon );
	}
	if(m_bShowLabel && m_hfLabel && m_wszLabel)
	{
		int labelWidth, labelHeight;
		Color labelColor;		
		if(m_ColorModeLabel == COLOR_MODE_STEPPED )
			labelColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorLabel.a());
		else if(m_ColorModeLabel == COLOR_MODE_FADED ) 
			labelColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorLabel.a());
		else if(m_ColorModeLabel == COLOR_MODE_TEAMCOLORED ) 
			labelColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorLabel.a());
		else
			labelColor = m_ColorLabel;
	
		vgui::surface()->GetTextSize(m_hfLabel, m_wszLabel, labelWidth, labelHeight);
		vgui::surface()->DrawSetTextFont(m_hfLabel);
		vgui::surface()->DrawSetTextColor( labelColor );

		if(m_iAlignLabel == ALIGN_CENTER)
			vgui::surface()->DrawSetTextPos(iLeft - labelWidth/2 + iOffsetXLabel, iTop + (iBarHeight+2-labelHeight)/2 + iOffsetYLabel);
		else if(m_iAlignLabel == ALIGN_RIGHT)
			vgui::surface()->DrawSetTextPos(iLeft - labelWidth + iOffsetXLabel, iTop + (iBarHeight+2-labelHeight)/2 + iOffsetYLabel);
		else
			vgui::surface()->DrawSetTextPos(iLeft + iOffsetXLabel, iTop + (iBarHeight+2-labelHeight)/2 + iOffsetYLabel);

		vgui::surface()->DrawUnicodeString( m_wszLabel );
	}

	wchar_t wszString[ 10 ];

	if(m_bShowAmount && m_hfAmount && m_wszAmount)
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
		if(m_ColorModeAmount == COLOR_MODE_STEPPED )
			amountColor.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorAmount.a());
		else if(m_ColorModeAmount == COLOR_MODE_FADED ) 
			amountColor.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorAmount.a());
		else if(m_ColorModeAmount == COLOR_MODE_TEAMCOLORED ) 
			amountColor.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),m_ColorAmount.a());
		else
			amountColor = m_ColorAmount;

		vgui::surface()->GetTextSize(m_hfAmount, wszString, amountWidth, amountHeight);
		vgui::surface()->DrawSetTextFont(m_hfAmount);
		vgui::surface()->DrawSetTextColor( amountColor );

		if(m_iAlignAmount == ALIGN_CENTER)
			vgui::surface()->DrawSetTextPos(iLeft - amountWidth/2 + iOffsetXAmount, iTop + (iBarHeight+2-amountHeight)/2 + iOffsetYAmount);
		else if(m_iAlignAmount == ALIGN_RIGHT)
			vgui::surface()->DrawSetTextPos(iLeft - amountWidth + iOffsetXAmount, iTop + (iBarHeight+2-amountHeight)/2 + iOffsetYAmount);
		else
			vgui::surface()->DrawSetTextPos(iLeft + iOffsetXAmount, iTop + (iBarHeight+2-amountHeight)/2 + iOffsetYAmount);

		vgui::surface()->DrawUnicodeString( wszString );
	}
}