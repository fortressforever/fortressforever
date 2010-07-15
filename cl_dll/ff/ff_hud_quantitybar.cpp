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

#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void CHudQuantityBar::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	m_hfQuantityBarText[0] = pScheme->GetFont( "QuantityBar", true );
	m_hfQuantityBarText[1] = pScheme->GetFont( "QuantityBarShadow", true );
	m_hfQuantityBarText[2] = pScheme->GetFont( "QuantityBar", false );
	m_hfQuantityBarIcon[0] = pScheme->GetFont( "QuantityBarIcon", true );
	m_hfQuantityBarIcon[1] = pScheme->GetFont( "QuantityBarIconShadow", true );
	m_hfQuantityBarIcon[2] = pScheme->GetFont( "QuantityBarIcon", false );
	m_hfQuantityBarGlyph[0] = pScheme->GetFont( "QuantityBarIcon", true );
	m_hfQuantityBarGlyph[1] = pScheme->GetFont( "QuantityBarIconShadow", true );
	m_hfQuantityBarGlyph[2] = pScheme->GetFont( "QuantityBarIcon", false );

	SetBorder(pScheme->GetBorder("ScoreBoardItemBorder"));

 	BaseClass::ApplySchemeSettings( pScheme );
}

void CHudQuantityBar::SetAmountFontShadow(bool bHasShadow) 
{
	m_bAmountFontShadow = bHasShadow;
}
void CHudQuantityBar::SetIconFontShadow(bool bHasShadow) 
{
	m_bIconFontShadow = bHasShadow;
}
void CHudQuantityBar::SetLabelFontShadow(bool bHasShadow) 
{
	m_bLabelFontShadow = bHasShadow;
}
void CHudQuantityBar::SetIconFontGlyph(bool bIconIsGlyph)
{
	m_bIconFontGlyph = bIconIsGlyph;
}

void CHudQuantityBar::SetAmount(int iAmount)
{ 
	m_iAmount = iAmount; 

	char szAmount[10];
	Q_snprintf( szAmount, 5, "%i%", iAmount );
	vgui::localize()->ConvertANSIToUnicode( szAmount, m_wszAmount, sizeof( m_wszAmount ) );
	
	RecalculateQuantity();
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

	RecalculateQuantity();
}

void CHudQuantityBar::SetIconChar(char *newIconChar)
{ 					
	char szIcon[5];
	Q_snprintf( szIcon, 2, "%s%", newIconChar );
	vgui::localize()->ConvertANSIToUnicode( szIcon, m_wszIcon, sizeof( m_wszIcon ) );
	
	RecalculateIconPosition();
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
	RecalculateLabelPosition();
}
void CHudQuantityBar::SetLabelText(wchar_t *newLabelText)
{
	wcscpy( m_wszLabel, newLabelText );
	RecalculateLabelPosition();
}

void CHudQuantityBar::SetPos(int iPositionX, int iPositionY)
{
	m_iLeft = iPositionX; 
	m_iTop = iPositionY;

	RecalculateIconPosition();
	RecalculateLabelPosition();
	RecalculateAmountPosition();
}

void CHudQuantityBar::SetBarWidth(int iBarWidth) { 
	m_iBarWidth = iBarWidth; 

	RecalculateIconPosition();
	RecalculateLabelPosition();
	RecalculateAmountPosition();
}
void CHudQuantityBar::SetBarHeight(int iBarHeight) { 
	m_iBarHeight = iBarHeight; 

	RecalculateIconPosition();
	RecalculateLabelPosition();
	RecalculateAmountPosition();
}
void CHudQuantityBar::SetBarSize(int iBarWidth, int iBarHeight) 
{ 
	m_iBarWidth = iBarWidth; 
	m_iBarHeight = iBarHeight;

	RecalculateIconPosition();
	RecalculateLabelPosition();
	RecalculateAmountPosition();
}

void CHudQuantityBar::SetBarBorderWidth(int iBarBorderWidth) { m_iBarBorderWidth = iBarBorderWidth; };

void CHudQuantityBar::SetBarOrientation(int iOrientation) { m_iBarOrientation = iOrientation; }

//TO-DO
//see if these change the dimentions and if an update is required - but first check if the value has really changed...
void CHudQuantityBar::ShowBar(bool bShowBar) { m_bShowBar = bShowBar; }
void CHudQuantityBar::ShowBarBackground(bool bShowBarBackground) { m_bShowBarBackground = bShowBarBackground; }
void CHudQuantityBar::ShowBarBorder(bool bShowBarBorder) { m_bShowBarBorder = bShowBarBorder; }
void CHudQuantityBar::ShowIcon(bool bShowIcon) { m_bShowIcon = bShowIcon; }
void CHudQuantityBar::ShowLabel(bool bShowLabel) { m_bShowLabel = bShowLabel; }
void CHudQuantityBar::ShowAmount(bool bShowAmount) { m_bShowAmount = bShowAmount; }
void CHudQuantityBar::ShowAmountMax(bool bShowMax) { m_bShowAmountMax = bShowMax; }

void CHudQuantityBar::SetBarColor( Color newBarColor ) { 
	m_ColorBarCustom = newBarColor;
	RecalculateColor(m_ColorModeBar, m_ColorBar, m_ColorBarCustom);
}

void CHudQuantityBar::SetBarBorderColor( Color newBarBorderColor ) { 
	m_ColorBarBorderCustom = newBarBorderColor; 
	RecalculateColor(m_ColorModeBarBorder, m_ColorBarBorder, m_ColorBarBorderCustom);
}
void CHudQuantityBar::SetBarBackgroundColor( Color newBarBackgroundColor ) { 
	m_ColorBarBackgroundCustom = newBarBackgroundColor; 
	RecalculateColor(m_ColorModeBarBackground, m_ColorBarBackground, m_ColorBarBackgroundCustom);
}
void CHudQuantityBar::SetAmountColor( Color newAmountColor ) { 
	m_ColorAmountCustom = newAmountColor; 
	RecalculateColor(m_ColorModeAmount, m_ColorAmount, m_ColorAmountCustom);
}
void CHudQuantityBar::SetIconColor( Color newIconColor ) { 
	m_ColorIconCustom = newIconColor; 
	RecalculateColor(m_ColorModeIcon, m_ColorIcon, m_ColorIconCustom);
}
void CHudQuantityBar::SetLabelColor( Color newLabelColor ) { 
	m_ColorLabelCustom = newLabelColor; 
	RecalculateColor(m_ColorModeLabel, m_ColorLabel, m_ColorLabelCustom);
}
void CHudQuantityBar::SetTeamColor( Color newTeamColor ) { 
	m_ColorTeam = newTeamColor; 
	//see which use team colour and update??
}

void CHudQuantityBar::SetBarColorMode( int iColorModeBar ) { 
	m_ColorModeBar = iColorModeBar; 
	RecalculateColor(m_ColorModeBar, m_ColorBarBorder, m_ColorBarCustom);
}
void CHudQuantityBar::SetBarBorderColorMode( int iColorModeBarBorder ) {
	m_ColorModeBarBorder = iColorModeBarBorder;
	RecalculateColor(m_ColorModeBarBorder, m_ColorBarBorder, m_ColorBarBorderCustom);
}
void CHudQuantityBar::SetBarBackgroundColorMode( int iColorModeBarBackround ) {
	m_ColorModeBarBackground = iColorModeBarBackround; 
	RecalculateColor(m_ColorModeBarBackground, m_ColorBarBackground, m_ColorBarBackgroundCustom);
}
void CHudQuantityBar::SetAmountColorMode( int iColorModeAmount ) { 
	m_ColorModeAmount = iColorModeAmount; 
	RecalculateColor(m_ColorModeAmount, m_ColorAmount, m_ColorAmountCustom);
}
void CHudQuantityBar::SetIconColorMode( int iColorModeIcon ) { 
	m_ColorModeIcon =  iColorModeIcon;
	RecalculateColor(m_ColorModeIcon, m_ColorIcon, m_ColorIconCustom);
	
}
void CHudQuantityBar::SetLabelColorMode( int iColorModeLabel ) { 
	m_ColorModeLabel = iColorModeLabel; 
	RecalculateColor(m_ColorModeLabel, m_ColorLabel, m_ColorLabelCustom);
}

void CHudQuantityBar::SetIconOffsetX(int iconOffsetX) { m_iOffsetXIcon = iconOffsetX; RecalculateIconPosition(); }
void CHudQuantityBar::SetIconOffsetY(int iconOffsetY) { m_iOffsetYIcon = iconOffsetY; RecalculateIconPosition(); }
void CHudQuantityBar::SetIconOffset(int iconOffsetX, int iconOffsetY) 
{ 
	m_iOffsetXIcon = iconOffsetX; 
	m_iOffsetYIcon = iconOffsetY;
	RecalculateIconPosition();
}

void CHudQuantityBar::SetLabelOffsetX(int labelOffsetX) { m_iOffsetXLabel = labelOffsetX; RecalculateLabelPosition(); }
void CHudQuantityBar::SetLabelOffsetY(int labelOffsetY) { m_iOffsetYLabel = labelOffsetY; RecalculateLabelPosition();}
void CHudQuantityBar::SetLabelOffset(int labelOffsetX, int labelOffsetY) 
{ 
	m_iOffsetXLabel = labelOffsetX; 
	m_iOffsetYLabel = labelOffsetY; 
	RecalculateLabelPosition(); 
}

void CHudQuantityBar::SetAmountOffsetX(int amountOffsetX) { m_iOffsetXAmount = amountOffsetX; RecalculateAmountPosition(); }
void CHudQuantityBar::SetAmountOffsetY(int amountOffsetY) { m_iOffsetYAmount = amountOffsetY; RecalculateAmountPosition(); }
void CHudQuantityBar::SetAmountOffset(int amountOffsetX, int amountOffsetY) 
{ 
	m_iOffsetXAmount = amountOffsetX; 
	m_iOffsetYAmount = amountOffsetY; 
	RecalculateAmountPosition(); 
}

int CHudQuantityBar::GetAmount() {return m_iAmount; }

void CHudQuantityBar::SetLabelTextAlignment(int iLabelTextAlign) { m_iTextAlignLabel = iLabelTextAlign; RecalculateLabelPosition(); }
void CHudQuantityBar::SetAmountTextAlignment(int iAmountTextAlign) { m_iTextAlignAmount = iAmountTextAlign; RecalculateAmountPosition(); }

void CHudQuantityBar::SetIntensityControl(int iRed, int iOrange,int iYellow, int iGreen, bool bInvertScale)
{
	m_iIntenisityRed = iRed;
	m_iIntenisityOrange = iOrange;
	m_iIntenisityYellow = iYellow;
	m_iIntenisityGreen = iGreen;
	m_bIntenisityInvertScale = bInvertScale;
}

void CHudQuantityBar::OnTick()
{
	if (!engine->IsInGame()) 
		return;

	// Get the screen width/height
	int iScreenWide, iScreenTall;
	vgui::surface()->GetScreenSize( iScreenWide, iScreenTall );

	// "map" screen res to 640/480
	float flScaleX = 1 / (640.0f / iScreenWide);
	float flScaleY = 1 / (480.0f / iScreenTall);

	if( m_flScaleX != flScaleX || m_flScaleY != flScaleY)
	// if user changes resolution
	{
		m_flScaleX = flScaleX;
		m_flScaleY = flScaleY;
		m_flScale = (m_flScaleX<=m_flScaleY ? m_flScaleX : m_flScaleY);

		RecalculateIconPosition();
		RecalculateLabelPosition();
		RecalculateAmountPosition();
		
		//send update to parent for positioning	
		if ( GetVParent() )
		{
			KeyValues* msg = new KeyValues("ChildDimentionsChanged");
			msg->SetInt("id",m_iChildId);
			PostMessage(GetVParent(), msg);
		}
	}
}

void CHudQuantityBar::GetDimentions(int& iWidth, int& iHeight, int& iBarOffsetX, int&  iBarOffsetY)
{
	int iX0 = m_iLeft;
	int iY0 = m_iTop;
	int iX1 = m_iLeft + m_iBarWidth;
	int iY1 = m_iTop + m_iBarHeight;

	if( m_iIconPosX < iX0 )
		iX0 = m_iIconPosX;
	if( m_iLabelPosX < iX0 )
		iX0 = m_iLabelPosX;
	if( m_iAmountPosX < iX0 )
		iX0 = m_iAmountPosX;

	if( m_iIconPosY < iY0 )
		iY0 = m_iIconPosY;
	if( m_iLabelPosY < iY0 )
		iY0 = m_iLabelPosY;
	if( m_iAmountPosY < iY0 )
		iY0 = m_iAmountPosY;

	if( (m_iIconPosX + m_iIconWidth) > iX1 )
		iX1 = m_iIconPosX + m_iIconWidth;
	if( (m_iLabelPosX + m_iLabelWidth) > iX1 )
		iX1 = m_iLabelPosX + m_iLabelWidth;
	if( (m_iAmountPosX + m_iAmountWidth) > iX1 )
		iX1 = m_iAmountPosX + m_iAmountWidth;

	if( (m_iIconPosY + m_iIconHeight) > iY1 )
		iY1 = m_iIconPosY + m_iIconHeight;
	if( (m_iLabelPosY + m_iLabelHeight) > iY1 )
		iY1 = m_iLabelPosY + m_iLabelHeight;
	if( (m_iAmountPosY + m_iAmountHeight) > iY1 )
		iY1 = m_iAmountPosY + m_iAmountHeight;

	iWidth = iX1 - iX0;
	iHeight = iY1 - iY0;
	iBarOffsetX = m_iLeft - iX0;
	iBarOffsetY = m_iTop - iY0;
}


void CHudQuantityBar::RecalculateIconPosition()
{
	if(m_bIconFontGlyph)
		CalculateTextAlignmentOffset(m_iIconAlignmentOffsetX, m_iIconAlignmentOffsetY, m_iIconWidth, m_iIconHeight, TEXTALIGN_CENTER, m_hfQuantityBarGlyph[2], m_wszIcon);
	else
		CalculateTextAlignmentOffset(m_iIconAlignmentOffsetX, m_iIconAlignmentOffsetY, m_iIconWidth, m_iIconHeight, TEXTALIGN_CENTER, m_hfQuantityBarIcon[2], m_wszIcon);
	m_iIconPosX = m_iLeft + m_iOffsetXIcon + m_iIconAlignmentOffsetX;
	m_iIconPosY = m_iTop + m_iOffsetYIcon + m_iIconAlignmentOffsetY;
}

void CHudQuantityBar::RecalculateLabelPosition()
{
	CalculateTextAlignmentOffset(m_iLabelAlignmentOffsetX, m_iLabelAlignmentOffsetY, m_iLabelWidth, m_iLabelHeight, m_iTextAlignLabel, m_hfQuantityBarText[2], m_wszLabel);
	m_iLabelPosX = m_iLeft + m_iOffsetXLabel + m_iLabelAlignmentOffsetX;
	m_iLabelPosY = m_iTop + m_iOffsetYLabel + m_iLabelAlignmentOffsetY;
}

void CHudQuantityBar::RecalculateAmountPosition()
{
	CalculateTextAlignmentOffset(m_iAmountAlignmentOffsetX, m_iAmountAlignmentOffsetY, m_iAmountWidth, m_iAmountHeight, m_iTextAlignAmount, m_hfQuantityBarText[2], m_wszAmountString);
	m_iAmountPosX = m_iLeft + m_iOffsetXAmount + m_iAmountAlignmentOffsetX;
	m_iAmountPosY = m_iTop + m_iOffsetYAmount + m_iAmountAlignmentOffsetY;
}

void CHudQuantityBar::Paint()
{
	// Set text position based on alignment & text

	if(m_bShowBarBorder)
	{
		vgui::surface()->DrawSetColor( m_ColorBarBorder );
		for( int i = 1; i <= (m_iBarBorderWidth * m_flScale); ++i )
		{
			vgui::surface()->DrawOutlinedRect( 
				m_iLeft * m_flScale - i, 
				m_iTop * m_flScale - i, 
				(m_iLeft + m_iBarWidth) * m_flScale + i, 
				(m_iTop + m_iBarHeight) * m_flScale + i
				);
		}
	}

	if(m_bShowBarBackground)
	{
		vgui::surface()->DrawSetColor( m_ColorBarBackground );
		vgui::surface()->DrawFilledRect( 
			m_iLeft * m_flScale, 
			m_iTop * m_flScale, 
			(m_iLeft + m_iBarWidth) * m_flScale, 
			(m_iTop + m_iBarHeight) * m_flScale
			);
	}

	if(m_bShowBar)
	{
		vgui::surface()->DrawSetColor( m_ColorBar );
		vgui::surface()->DrawFilledRect( 
			(m_iLeft + m_iBarX0QuantityOffset) * m_flScale,
			(m_iTop + m_iBarY0QuantityOffset) * m_flScale,
			(m_iLeft + m_iBarWidth + m_iBarX1QuantityOffset) * m_flScale,
			(m_iTop + m_iBarHeight + m_iBarY1QuantityOffset) * m_flScale
			);
	}

	if(m_bShowIcon && m_wszIcon)
	{	
		if(m_bIconFontGlyph)
			vgui::surface()->DrawSetTextFont(m_hfQuantityBarGlyph[m_bIconFontShadow]);
		else
			vgui::surface()->DrawSetTextFont(m_hfQuantityBarIcon[m_bIconFontShadow]);

		vgui::surface()->DrawSetTextColor( m_ColorIcon );
		vgui::surface()->DrawSetTextPos(m_iIconPosX * m_flScale, m_iIconPosY * m_flScale);
		vgui::surface()->DrawUnicodeString( m_wszIcon );
	}

	if(m_bShowLabel && m_wszLabel)
	{	
		vgui::surface()->DrawSetTextFont(m_hfQuantityBarText[m_bLabelFontShadow]);
		vgui::surface()->DrawSetTextColor( m_ColorLabel );
		vgui::surface()->DrawSetTextPos(m_iLabelPosX * m_flScale, m_iLabelPosY * m_flScale);
		vgui::surface()->DrawUnicodeString( m_wszLabel );
	}

	if(m_bShowAmount && m_wszAmount)
	{
		vgui::surface()->DrawSetTextFont(m_hfQuantityBarText[m_bAmountFontShadow]);
		vgui::surface()->DrawSetTextColor( m_ColorAmount );
		vgui::surface()->DrawSetTextPos(m_iAmountPosX * m_flScale, m_iAmountPosY * m_flScale);
		vgui::surface()->DrawUnicodeString( m_wszAmountString );
	}
}

void CHudQuantityBar::RecalculateQuantity()
//all this regardless of whether each item is actually being shown
//(incase the user changes options during the game)
{
	m_ColorIntensityFaded = getIntensityColor(m_iAmount, m_iMaxAmount, 2, m_ColorBarCustom.a(), m_iIntenisityRed,m_iIntenisityOrange,m_iIntenisityYellow,m_iIntenisityGreen,m_bIntenisityInvertScale);
	m_ColorIntensityStepped = getIntensityColor(m_iAmount, m_iMaxAmount, 1, m_ColorBarCustom.a(), m_iIntenisityRed,m_iIntenisityOrange,m_iIntenisityYellow,m_iIntenisityGreen,m_bIntenisityInvertScale);

	if(!m_bShowAmountMax)
	{
		if(m_iMaxAmount == 100)// already a percentage no need to convert
			_snwprintf( m_wszAmountString, 255, L"%s%%", m_wszAmount );
		else
		{
			char szPercent[ 5 ];
			wchar_t wszPercent[ 10 ];
			float percent = ((float)m_iAmount/(float)m_iMaxAmount*100.0f);
			Q_snprintf( szPercent, 5, "%i%", (int)percent);

			vgui::localize()->ConvertANSIToUnicode( szPercent, wszPercent, sizeof( wszPercent ) );

			_snwprintf( m_wszAmountString, 9, L"%s%%", wszPercent );
		}
	}
	else
	{
		_snwprintf( m_wszAmountString, 9, L"%s/%s", m_wszAmount, m_wszAmountMax );
	}

	RecalculateAmountPosition();

	if(m_ColorModeBarBorder == COLOR_MODE_FADED ) 
		m_ColorBarBorder.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorBarBorderCustom.a());
	else if(m_ColorModeBarBorder == COLOR_MODE_STEPPED )
		m_ColorBarBorder.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorBarBorder.a());

	if(m_ColorModeBarBackground == COLOR_MODE_FADED ) 
		m_ColorBarBackground.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorBarBackgroundCustom.a());
	else if(m_ColorModeBarBackground == COLOR_MODE_STEPPED )
		m_ColorBarBackground.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorBarBackgroundCustom.a());

	if(m_ColorModeBar == COLOR_MODE_FADED ) 
		m_ColorBar.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorBarCustom.a());
	else if(m_ColorModeBar == COLOR_MODE_STEPPED )
		m_ColorBar.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorBarCustom.a());

	if(m_ColorModeIcon == COLOR_MODE_FADED ) 
		m_ColorIcon.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorIconCustom.a());
	else if(m_ColorModeIcon == COLOR_MODE_STEPPED )
		m_ColorIcon.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorIconCustom.a());

	if(m_ColorModeLabel == COLOR_MODE_FADED ) 
		m_ColorLabel.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorLabelCustom.a());
	else if(m_ColorModeLabel == COLOR_MODE_STEPPED )
		m_ColorLabel.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorLabelCustom.a());

	if(m_ColorModeAmount == COLOR_MODE_FADED ) 
		m_ColorAmount.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_ColorAmountCustom.a());
	else if(m_ColorModeAmount == COLOR_MODE_STEPPED )
		m_ColorAmount.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorAmountCustom.a());

	if(m_iBarOrientation == ORIENTATION_HORIZONTAL_INVERTED)
	{
		m_iBarX0QuantityOffset = m_iBarWidth-(m_iBarHeight * m_iAmount/m_iMaxAmount);
		if( (m_iBarX1QuantityOffset + m_iBarY0QuantityOffset + m_iBarY1QuantityOffset) > 0 )
		{
			m_iBarX1QuantityOffset = 0;
			m_iBarY0QuantityOffset = 0;
			m_iBarY1QuantityOffset = 0;
		}
	}
	else if(m_iBarOrientation == ORIENTATION_VERTICAL)
	{
		m_iBarY0QuantityOffset = m_iBarHeight - (m_iBarHeight * m_iAmount/m_iMaxAmount);
		if( (m_iBarX0QuantityOffset + m_iBarX1QuantityOffset + m_iBarY1QuantityOffset) > 0 )
		{
			m_iBarX0QuantityOffset = 0;
			m_iBarX1QuantityOffset = 0;
			m_iBarY1QuantityOffset = 0;
		}
	}
	else if(m_iBarOrientation == ORIENTATION_VERTICAL_INVERTED)
	{
		m_iBarY1QuantityOffset = (m_iBarHeight * m_iAmount/m_iMaxAmount) - m_iBarHeight;
		if( (m_iBarX0QuantityOffset + m_iBarX1QuantityOffset + m_iBarY0QuantityOffset) > 0 )
		{
			m_iBarX0QuantityOffset = 0;
			m_iBarX1QuantityOffset = 0;
			m_iBarY0QuantityOffset = 0;
		}
	}
	else //m_iBarOrientation == ORIENTATION_HORIZONTAL (or anything else)
	{
		m_iBarX1QuantityOffset = (m_iBarWidth * m_iAmount/m_iMaxAmount) - m_iBarWidth;
		if( (m_iBarX0QuantityOffset + m_iBarY0QuantityOffset + m_iBarY1QuantityOffset) > 0 )
		{
			m_iBarX0QuantityOffset = 0;
			m_iBarY0QuantityOffset = 0;
			m_iBarY1QuantityOffset = 0;
		}
	}
}

void CHudQuantityBar::RecalculateColor(int &colorMode, Color &color, Color &colorCustom)
{
	if(colorMode == COLOR_MODE_STEPPED )
		color.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),colorCustom.a());
	else if(m_ColorModeBarBackground == COLOR_MODE_TEAMCOLORED ) 
		color.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),colorCustom.a());
	else if( m_ColorModeBarBorder == COLOR_MODE_FADED )
		color.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),colorCustom.a());
	else
		color = colorCustom;
}

void CHudQuantityBar::CalculateTextAlignmentOffset(int &outX, int &outY, int &iWide, int &iTall, int iAlignmentMode, vgui::HFont hfFont, wchar_t* wszString)
{
	vgui::surface()->GetTextSize(hfFont, wszString, iWide, iTall);

	outY = (m_iBarHeight - iTall) / 2;
	switch(iAlignmentMode)
	{
	case TEXTALIGN_CENTER:
		outX = - iWide/2;
		break;
	case TEXTALIGN_RIGHT:
		outX = - iWide;
		break;
	case TEXTALIGN_LEFT:
	default:
		outX = 0;
	}
}