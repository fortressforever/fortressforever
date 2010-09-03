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
// implementation of FFQuantityBar class
//

#include "cbase.h"
#include "ff_quantitybar.h"

#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


FFQuantityBar::FFQuantityBar(vgui::Panel *parent, const char *pElementName, int childId) : CHudElement(pElementName), vgui::Panel(parent, pElementName) 
{
	m_iChildId = childId;

	SetParent( parent );
	SetSize(vgui::scheme()->GetProportionalScaledValue(640),vgui::scheme()->GetProportionalScaledValue(480));

	m_flScale = 1.0f;
	m_flScaleX = 1.0f;
	m_flScaleY = 1.0f;
	SetBarSize(60, 13);
	SetBarBorderWidth(1);

	SetBarOrientation(ORIENTATION_HORIZONTAL);

	m_iTextAlignLabel = TEXTALIGN_RIGHT;
	m_iTextAlignAmount = TEXTALIGN_CENTER;

	SetIconOffset(5,0);
	SetLabelOffset(-5, 0);
	SetAmountOffset(35, 0);

	SetIntensityControl(20,50,80,100);
	SetIntensityAmountScaled(false);
	SetIntensityValuesFixed(false);

	SetBarBorderColor(Color(255,255,255,255));
	SetBarBackgroundColor(Color(192,192,192,80));
	SetBarColor(Color(255,255,255,255));
	SetIconColor(Color(255,255,255,255));
	SetLabelColor(Color(255,255,255,255));
	SetAmountColor(Color(255,255,255,255));

	SetBarColorMode(COLOR_MODE_STEPPED);
	SetBarBorderColorMode(COLOR_MODE_CUSTOM);
	SetBarBackgroundColorMode(COLOR_MODE_STEPPED);
	SetIconColorMode(COLOR_MODE_CUSTOM);
	SetLabelColorMode(COLOR_MODE_CUSTOM);
	SetAmountColorMode(COLOR_MODE_CUSTOM);

	SetAmountMax(100);

	ShowBar(true);
	ShowBarBackground(true);
	ShowBarBorder(true);
	ShowIcon(true);
	ShowLabel(true);
	ShowAmount(true);
	ShowAmountMax(true);

	SetAmountFontShadow(false);
	SetIconFontShadow(false);
	SetLabelFontShadow(false);

	vgui::ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
}

void FFQuantityBar::ApplySchemeSettings( vgui::IScheme *pScheme )
{
	//the non scaled fonts are used for positioning calculations
	//for 'simplicity' and consistency everything is calculated as if in 640x480

	//decided I'll make the text size variable too
	//size*3 then offset for shadow and non proportional

	vgui::HScheme QuantityBarScheme = vgui::scheme()->LoadSchemeFromFile("resource/QuantityPanelScheme.res", "QuantityPanelScheme");
	vgui::IScheme *qbScheme = vgui::scheme()->GetIScheme(QuantityBarScheme);
			
	m_hfQuantityBarText[0] = qbScheme->GetFont( "QuantityBar0", true );
	m_hfQuantityBarText[1] = qbScheme->GetFont( "QuantityBarShadow0", true );
	m_hfQuantityBarText[2] = qbScheme->GetFont( "QuantityBar0", false );
	m_hfQuantityBarText[3] = qbScheme->GetFont( "QuantityBar1", true );
	m_hfQuantityBarText[4] = qbScheme->GetFont( "QuantityBarShadow1", true );
	m_hfQuantityBarText[5] = qbScheme->GetFont( "QuantityBar1", false );
	m_hfQuantityBarText[6] = qbScheme->GetFont( "QuantityBar2", true );
	m_hfQuantityBarText[7] = qbScheme->GetFont( "QuantityBarShadow2", true );
	m_hfQuantityBarText[8] = qbScheme->GetFont( "QuantityBar2", false );
	m_hfQuantityBarText[9] = qbScheme->GetFont( "QuantityBar3", true );
	m_hfQuantityBarText[10] = qbScheme->GetFont( "QuantityBarShadow3", true );
	m_hfQuantityBarText[11] = qbScheme->GetFont( "QuantityBar3", false );
	m_hfQuantityBarText[12] = qbScheme->GetFont( "QuantityBar4", true );
	m_hfQuantityBarText[13] = qbScheme->GetFont( "QuantityBarShadow4", true );
	m_hfQuantityBarText[14] = qbScheme->GetFont( "QuantityBar4", false );


	m_hfQuantityBarIcon[0] = qbScheme->GetFont( "QuantityBarIcon0", true );
	m_hfQuantityBarIcon[1] = qbScheme->GetFont( "QuantityBarIconShadow0", true );
	m_hfQuantityBarIcon[2] = qbScheme->GetFont( "QuantityBarIcon0", false );
	m_hfQuantityBarIcon[3] = qbScheme->GetFont( "QuantityBarIcon1", true );
	m_hfQuantityBarIcon[4] = qbScheme->GetFont( "QuantityBarIconShadow1", true );
	m_hfQuantityBarIcon[5] = qbScheme->GetFont( "QuantityBarIcon1", false );
	m_hfQuantityBarIcon[6] = qbScheme->GetFont( "QuantityBarIcon2", true );
	m_hfQuantityBarIcon[7] = qbScheme->GetFont( "QuantityBarIconShadow2", true );
	m_hfQuantityBarIcon[8] = qbScheme->GetFont( "QuantityBarIcon2", false );
	m_hfQuantityBarIcon[9] = qbScheme->GetFont( "QuantityBarIcon3", true );
	m_hfQuantityBarIcon[10] = qbScheme->GetFont( "QuantityBarIconShadow3", true );
	m_hfQuantityBarIcon[11] = qbScheme->GetFont( "QuantityBarIcon3", false );
	m_hfQuantityBarIcon[12] = qbScheme->GetFont( "QuantityBarIcon4", true );
	m_hfQuantityBarIcon[13] = qbScheme->GetFont( "QuantityBarIconShadow4", true );
	m_hfQuantityBarIcon[14] = qbScheme->GetFont( "QuantityBarIcon4", false );

	m_hfQuantityBarGlyph[0] = qbScheme->GetFont( "QuantityBarGlyph0", true );
	m_hfQuantityBarGlyph[1] = qbScheme->GetFont( "QuantityBarGlyphShadow0", true );
	m_hfQuantityBarGlyph[2] = qbScheme->GetFont( "QuantityBarGlyph0", false );
	m_hfQuantityBarGlyph[3] = qbScheme->GetFont( "QuantityBarGlyph1", true );
	m_hfQuantityBarGlyph[4] = qbScheme->GetFont( "QuantityBarGlyphShadow1", true );
	m_hfQuantityBarGlyph[5] = qbScheme->GetFont( "QuantityBarGlyph1", false );
	m_hfQuantityBarGlyph[6] = qbScheme->GetFont( "QuantityBarGlyph2", true );
	m_hfQuantityBarGlyph[7] = qbScheme->GetFont( "QuantityBarGlyphShadow2", true );
	m_hfQuantityBarGlyph[8] = qbScheme->GetFont( "QuantityBarGlyph2", false );
	m_hfQuantityBarGlyph[9] = qbScheme->GetFont( "QuantityBarGlyph3", true );
	m_hfQuantityBarGlyph[10] = qbScheme->GetFont( "QuantityBarGlyphShadow3", true );
	m_hfQuantityBarGlyph[11] = qbScheme->GetFont( "QuantityBarGlyph3", false );
	m_hfQuantityBarGlyph[12] = qbScheme->GetFont( "QuantityBarGlyph4", true );
	m_hfQuantityBarGlyph[13] = qbScheme->GetFont( "QuantityBarGlyphShadow4", true );
	m_hfQuantityBarGlyph[14] = qbScheme->GetFont( "QuantityBarGlyph4", false );

	SetBorder(pScheme->GetBorder("ScoreBoardItemBorder"));

 	BaseClass::ApplySchemeSettings( pScheme );
}

void FFQuantityBar::SetAmountFontShadow(bool bHasShadow) 
{
	m_bAmountFontShadow = bHasShadow;
}
void FFQuantityBar::SetIconFontShadow(bool bHasShadow) 
{
	m_bIconFontShadow = bHasShadow;
}
void FFQuantityBar::SetLabelFontShadow(bool bHasShadow) 
{
	m_bLabelFontShadow = bHasShadow;
}
void FFQuantityBar::SetIconFontGlyph(bool bIconIsGlyph)
{
	m_bIconFontGlyph = bIconIsGlyph;
}


void FFQuantityBar::SetIconSize( int newIconSize )
{
	m_iSizeIcon = newIconSize;
	RecalculateIconPosition();
}
void FFQuantityBar::SetLabelSize( int newLabelSize )
{
	m_iSizeLabel = newLabelSize;
	RecalculateLabelPosition();
}
void FFQuantityBar::SetAmountSize( int newAmountSize )
{
	m_iSizeAmount = newAmountSize;
	RecalculateAmountPosition();
}

void FFQuantityBar::SetAmount(int iAmount)
{ 
	m_iAmount = iAmount; 

	char szAmount[10];
	Q_snprintf( szAmount, 5, "%i%", iAmount );
	vgui::localize()->ConvertANSIToUnicode( szAmount, m_wszAmount, sizeof( m_wszAmount ) );
	
	RecalculateQuantity();
}
void FFQuantityBar::SetAmountMax(int iAmountMax)
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

void FFQuantityBar::SetIconChar(char *newIconChar)
{ 					
	char szIcon[5];
	Q_snprintf( szIcon, 2, "%s%", newIconChar );
	vgui::localize()->ConvertANSIToUnicode( szIcon, m_wszIcon, sizeof( m_wszIcon ) );
	
	RecalculateIconPosition();
}
void FFQuantityBar::SetLabelText(char *newLabelText)
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
void FFQuantityBar::SetLabelText(wchar_t *newLabelText)
{
	wcscpy( m_wszLabel, newLabelText );
	RecalculateLabelPosition();
}

void FFQuantityBar::SetPos(int iPositionX, int iPositionY)
{
	m_iLeft = iPositionX; 
	m_iTop = iPositionY;

	RecalculateIconPosition();
	RecalculateLabelPosition();
	RecalculateAmountPosition();
}

void FFQuantityBar::SetBarWidth(int iBarWidth) { 
	m_iBarWidth = iBarWidth; 

	RecalculateIconPosition();
	RecalculateLabelPosition();
	RecalculateAmountPosition();
}
void FFQuantityBar::SetBarHeight(int iBarHeight) { 
	m_iBarHeight = iBarHeight; 

	RecalculateIconPosition();
	RecalculateLabelPosition();
	RecalculateAmountPosition();
}
void FFQuantityBar::SetBarSize(int iBarWidth, int iBarHeight) 
{ 
	m_iBarWidth = iBarWidth; 
	m_iBarHeight = iBarHeight;

	RecalculateIconPosition();
	RecalculateLabelPosition();
	RecalculateAmountPosition();
}

void FFQuantityBar::SetBarBorderWidth(int iBarBorderWidth) { m_iBarBorderWidth = iBarBorderWidth; };

void FFQuantityBar::SetBarOrientation(int iOrientation) { m_iBarOrientation = iOrientation; }

//TO-DO
//see if these change the dimentions and if an update is required - but first check if the value has really changed...
void FFQuantityBar::ShowBar(bool bShowBar) { m_bShowBar = bShowBar; }
void FFQuantityBar::ShowBarBackground(bool bShowBarBackground) { m_bShowBarBackground = bShowBarBackground; }
void FFQuantityBar::ShowBarBorder(bool bShowBarBorder) { m_bShowBarBorder = bShowBarBorder; }
void FFQuantityBar::ShowIcon(bool bShowIcon) { m_bShowIcon = bShowIcon; }
void FFQuantityBar::ShowLabel(bool bShowLabel) { m_bShowLabel = bShowLabel; }
void FFQuantityBar::ShowAmount(bool bShowAmount) { m_bShowAmount = bShowAmount; }
void FFQuantityBar::ShowAmountMax(bool bShowMax) { m_bShowAmountMax = bShowMax; }

void FFQuantityBar::SetBarColor( Color newBarColor ) { 
	m_ColorBarCustom = newBarColor;
	RecalculateColor(m_ColorModeBar, m_ColorBar, m_ColorBarCustom);
}

void FFQuantityBar::SetBarBorderColor( Color newBarBorderColor ) { 
	m_ColorBarBorderCustom = newBarBorderColor; 
	RecalculateColor(m_ColorModeBarBorder, m_ColorBarBorder, m_ColorBarBorderCustom);
}
void FFQuantityBar::SetBarBackgroundColor( Color newBarBackgroundColor ) { 
	m_ColorBarBackgroundCustom = newBarBackgroundColor; 
	RecalculateColor(m_ColorModeBarBackground, m_ColorBarBackground, m_ColorBarBackgroundCustom);
}
void FFQuantityBar::SetAmountColor( Color newAmountColor ) { 
	m_ColorAmountCustom = newAmountColor; 
	RecalculateColor(m_ColorModeAmount, m_ColorAmount, m_ColorAmountCustom);
}
void FFQuantityBar::SetIconColor( Color newIconColor ) { 
	m_ColorIconCustom = newIconColor; 
	RecalculateColor(m_ColorModeIcon, m_ColorIcon, m_ColorIconCustom);
}
void FFQuantityBar::SetLabelColor( Color newLabelColor ) { 
	m_ColorLabelCustom = newLabelColor; 
	RecalculateColor(m_ColorModeLabel, m_ColorLabel, m_ColorLabelCustom);
}
void FFQuantityBar::SetTeamColor( Color newTeamColor ) { 
	m_ColorTeam = newTeamColor; 
	//see which use team colour and update??
}

void FFQuantityBar::SetBarColorMode( int iColorModeBar ) { 
	m_ColorModeBar = iColorModeBar; 
	RecalculateColor(m_ColorModeBar, m_ColorBar, m_ColorBarCustom);
}
void FFQuantityBar::SetBarBorderColorMode( int iColorModeBarBorder ) {
	m_ColorModeBarBorder = iColorModeBarBorder;
	RecalculateColor(m_ColorModeBarBorder, m_ColorBarBorder, m_ColorBarBorderCustom);
}
void FFQuantityBar::SetBarBackgroundColorMode( int iColorModeBarBackround ) {
	m_ColorModeBarBackground = iColorModeBarBackround; 
	RecalculateColor(m_ColorModeBarBackground, m_ColorBarBackground, m_ColorBarBackgroundCustom);
}
void FFQuantityBar::SetAmountColorMode( int iColorModeAmount ) { 
	m_ColorModeAmount = iColorModeAmount; 
	RecalculateColor(m_ColorModeAmount, m_ColorAmount, m_ColorAmountCustom);
}
void FFQuantityBar::SetIconColorMode( int iColorModeIcon ) { 
	m_ColorModeIcon =  iColorModeIcon;
	RecalculateColor(m_ColorModeIcon, m_ColorIcon, m_ColorIconCustom);
	
}
void FFQuantityBar::SetLabelColorMode( int iColorModeLabel ) { 
	m_ColorModeLabel = iColorModeLabel; 
	RecalculateColor(m_ColorModeLabel, m_ColorLabel, m_ColorLabelCustom);
}

void FFQuantityBar::SetIconOffsetX(int iconOffsetX) { m_iOffsetXIcon = iconOffsetX; RecalculateIconPosition(); }
void FFQuantityBar::SetIconOffsetY(int iconOffsetY) { m_iOffsetYIcon = iconOffsetY; RecalculateIconPosition(); }
void FFQuantityBar::SetIconOffset(int iconOffsetX, int iconOffsetY) 
{ 
	m_iOffsetXIcon = iconOffsetX; 
	m_iOffsetYIcon = iconOffsetY;
	RecalculateIconPosition();
}

void FFQuantityBar::SetLabelOffsetX(int labelOffsetX) { m_iOffsetXLabel = labelOffsetX; RecalculateLabelPosition(); }
void FFQuantityBar::SetLabelOffsetY(int labelOffsetY) { m_iOffsetYLabel = labelOffsetY; RecalculateLabelPosition();}
void FFQuantityBar::SetLabelOffset(int labelOffsetX, int labelOffsetY) 
{ 
	m_iOffsetXLabel = labelOffsetX; 
	m_iOffsetYLabel = labelOffsetY; 
	RecalculateLabelPosition(); 
}

void FFQuantityBar::SetAmountOffsetX(int amountOffsetX) { m_iOffsetXAmount = amountOffsetX; RecalculateAmountPosition(); }
void FFQuantityBar::SetAmountOffsetY(int amountOffsetY) { m_iOffsetYAmount = amountOffsetY; RecalculateAmountPosition(); }
void FFQuantityBar::SetAmountOffset(int amountOffsetX, int amountOffsetY) 
{ 
	m_iOffsetXAmount = amountOffsetX; 
	m_iOffsetYAmount = amountOffsetY; 
	RecalculateAmountPosition(); 
}

int FFQuantityBar::GetAmount() {return m_iAmount; }

void FFQuantityBar::SetLabelTextAlignment(int iLabelTextAlign) { m_iTextAlignLabel = iLabelTextAlign; RecalculateLabelPosition(); }
void FFQuantityBar::SetAmountTextAlignment(int iAmountTextAlign) { m_iTextAlignAmount = iAmountTextAlign; RecalculateAmountPosition(); }

void FFQuantityBar::SetIntensityControl(int iRed, int iOrange,int iYellow, int iGreen, bool bInvertScale)
{
	m_iIntensityRed = iRed;
	m_iIntensityOrange = iOrange;
	m_iIntensityYellow = iYellow;
	m_iIntensityGreen = iGreen;
	m_bIntensityInvertScale = bInvertScale;
}

void FFQuantityBar::SetIntensityAmountScaled(bool bAmountScaled)
{
	m_bIntensityAmountScaled = bAmountScaled;
}
void FFQuantityBar::SetIntensityValuesFixed(bool bvaluesFixed)
{
	m_bIntensityValuesFixed = bvaluesFixed;
}
bool FFQuantityBar::IsIntensityValuesFixed()
{
	return m_bIntensityValuesFixed;
}

void FFQuantityBar::OnTick()
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

void FFQuantityBar::GetPanelPositioningData(int& iWidth, int& iHeight, int& iBarOffsetX, int&  iBarOffsetY)
{
	int iX0 = m_iLeft;
	int iY0 = m_iTop;
	int iX1 = m_iLeft + m_iBarWidth;
	int iY1 = m_iTop + m_iBarHeight;

	if( m_bShowIcon && m_iIconPosX < iX0 )
		iX0 = m_iIconPosX;
	if( m_bShowLabel && m_iLabelPosX < iX0 )
		iX0 = m_iLabelPosX;
	if( m_bShowAmount && m_iAmountPosX < iX0 )
		iX0 = m_iAmountPosX;

	if( m_bShowIcon && m_iIconPosY < iY0 )
		iY0 = m_iIconPosY;
	if( m_bShowLabel && m_iLabelPosY < iY0 )
		iY0 = m_iLabelPosY;
	if( m_bShowAmount && m_iAmountPosY < iY0 )
		iY0 = m_iAmountPosY;

	if( m_bShowIcon && (m_iIconPosX + m_iIconWidth) > iX1 )
		iX1 = m_iIconPosX + m_iIconWidth;
	if( m_bShowLabel && (m_iLabelPosX + m_iLabelWidth) > iX1 )
		iX1 = m_iLabelPosX + m_iLabelWidth;
	if( m_bShowAmount && (m_iAmountPosX + m_iAmountWidth) > iX1 )
		iX1 = m_iAmountPosX + m_iAmountWidth;

	if( m_bShowIcon && (m_iIconPosY + m_iIconHeight) > iY1 )
		iY1 = m_iIconPosY + m_iIconHeight;
	if( m_bShowLabel && (m_iLabelPosY + m_iLabelHeight) > iY1 )
		iY1 = m_iLabelPosY + m_iLabelHeight;
	if( m_bShowAmount && (m_iAmountPosY + m_iAmountHeight) > iY1 )
		iY1 = m_iAmountPosY + m_iAmountHeight;

	iWidth = iX1 - iX0;
	iHeight = iY1 - iY0;
	iBarOffsetX = m_iLeft - iX0;
	iBarOffsetY = m_iTop - iY0;
}


void FFQuantityBar::RecalculateIconPosition()
{
	if(m_bIconFontGlyph)
		CalculateTextAlignmentOffset(m_iIconAlignmentOffsetX, m_iIconAlignmentOffsetY, m_iIconWidth, m_iIconHeight, TEXTALIGN_CENTER, m_hfQuantityBarGlyph[m_iSizeIcon*3 + 2], m_wszIcon);
	else
		CalculateTextAlignmentOffset(m_iIconAlignmentOffsetX, m_iIconAlignmentOffsetY, m_iIconWidth, m_iIconHeight, TEXTALIGN_CENTER, m_hfQuantityBarIcon[m_iSizeIcon*3 + 2], m_wszIcon);
	m_iIconPosX = m_iLeft + m_iOffsetXIcon + m_iIconAlignmentOffsetX;
	m_iIconPosY = m_iTop + m_iOffsetYIcon + m_iIconAlignmentOffsetY;
}

void FFQuantityBar::RecalculateLabelPosition()
{
	CalculateTextAlignmentOffset(m_iLabelAlignmentOffsetX, m_iLabelAlignmentOffsetY, m_iLabelWidth, m_iLabelHeight, m_iTextAlignLabel, m_hfQuantityBarText[m_iSizeLabel*3 + 2], m_wszLabel);
	m_iLabelPosX = m_iLeft + m_iOffsetXLabel + m_iLabelAlignmentOffsetX;
	m_iLabelPosY = m_iTop + m_iOffsetYLabel + m_iLabelAlignmentOffsetY;
}

void FFQuantityBar::RecalculateAmountPosition()
{
	CalculateTextAlignmentOffset(m_iAmountAlignmentOffsetX, m_iAmountAlignmentOffsetY, m_iAmountWidth, m_iAmountHeight, m_iTextAlignAmount, m_hfQuantityBarText[m_iSizeAmount*3 + 2], m_wszAmountString);
	m_iAmountPosX = m_iLeft + m_iOffsetXAmount + m_iAmountAlignmentOffsetX;
	m_iAmountPosY = m_iTop + m_iOffsetYAmount + m_iAmountAlignmentOffsetY;
}

void FFQuantityBar::Paint()
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
			vgui::surface()->DrawSetTextFont(m_hfQuantityBarGlyph[m_iSizeIcon * 3 + (m_bIconFontShadow ? 1 : 0)]);
		else
			vgui::surface()->DrawSetTextFont(m_hfQuantityBarIcon[m_iSizeIcon * 3 + (m_bIconFontShadow ? 1 : 0)]);

		vgui::surface()->DrawSetTextColor( m_ColorIcon );
		vgui::surface()->DrawSetTextPos(m_iIconPosX * m_flScale, m_iIconPosY * m_flScale);
		vgui::surface()->DrawUnicodeString( m_wszIcon );
	}

	if(m_bShowLabel && m_wszLabel)
	{	
		vgui::surface()->DrawSetTextFont(m_hfQuantityBarText[m_iSizeLabel * 3 + (m_bLabelFontShadow ? 1 : 0)]);
		vgui::surface()->DrawSetTextColor( m_ColorLabel );
		vgui::surface()->DrawSetTextPos(m_iLabelPosX * m_flScale, m_iLabelPosY * m_flScale);
		vgui::surface()->DrawUnicodeString( m_wszLabel );
	}

	if(m_bShowAmount && m_wszAmount)
	{
		vgui::surface()->DrawSetTextFont(m_hfQuantityBarText[m_iSizeAmount * 3 + (m_bAmountFontShadow ? 1 : 0)]);
		vgui::surface()->DrawSetTextColor( m_ColorAmount );
		vgui::surface()->DrawSetTextPos(m_iAmountPosX * m_flScale, m_iAmountPosY * m_flScale);
		vgui::surface()->DrawUnicodeString( m_wszAmountString );
	}
}

void FFQuantityBar::RecalculateQuantity()
//all this regardless of whether each item is actually being shown
//(incase the user changes options during the game)
{
	if(m_bIntensityAmountScaled)
	{
		m_ColorIntensityFaded = getIntensityColor((int)((float)m_iAmount/(float)m_iMaxAmount * 100), 100, 2, m_ColorBarCustom.a(), m_iIntensityRed,m_iIntensityOrange,m_iIntensityYellow,m_iIntensityGreen,m_bIntensityInvertScale);
		m_ColorIntensityStepped = getIntensityColor((int)((float)m_iAmount/(float)m_iMaxAmount * 100), 100, 1, m_ColorBarCustom.a(), m_iIntensityRed,m_iIntensityOrange,m_iIntensityYellow,m_iIntensityGreen,m_bIntensityInvertScale);
	}
	else
	{
		m_ColorIntensityFaded = getIntensityColor(m_iAmount, m_iMaxAmount, 2, m_ColorBarCustom.a(), m_iIntensityRed,m_iIntensityOrange,m_iIntensityYellow,m_iIntensityGreen,m_bIntensityInvertScale);
		m_ColorIntensityStepped = getIntensityColor(m_iAmount, m_iMaxAmount, 1, m_ColorBarCustom.a(), m_iIntensityRed,m_iIntensityOrange,m_iIntensityYellow,m_iIntensityGreen,m_bIntensityInvertScale);
	}

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
		m_ColorBarBorder.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_ColorBarBorderCustom.a());

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

void FFQuantityBar::RecalculateColor(int colorMode, Color &color, Color &colorCustom)
{
	if(colorMode == COLOR_MODE_STEPPED )
		color.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),colorCustom.a());
	else if(colorMode == COLOR_MODE_FADED )
		color.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),colorCustom.a());
	else if(colorMode >= COLOR_MODE_TEAMCOLORED ) 
		color.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),colorCustom.a());
	else
		color = colorCustom;
}

void FFQuantityBar::CalculateTextAlignmentOffset(int &outX, int &outY, int &iWide, int &iTall, int iAlignmentMode, vgui::HFont hfFont, wchar_t* wszString)
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