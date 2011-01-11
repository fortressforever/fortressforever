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


namespace vgui
{
	FFQuantityBar::FFQuantityBar(Panel *parent, const char *pElementName) : BaseClass(parent, pElementName) 
	{
		m_flScale = 1.0f;
		m_flScaleX = 1.0f;
		m_flScaleY = 1.0f;
		
		m_bChildDimentionsChanged = false;

		m_iBarWidth = 60; 
		m_iBarHeight = 12;
		m_iBarBorderWidth = 1;
		m_iBarOrientation = ORIENTATION_HORIZONTAL;
	
		//-1 means that when painting the internal offset is used instead of these overrides
		m_iOffsetOverrideX = -1;
		m_iOffsetOverrideY = -1;
		m_iOffsetX = 0;
		m_iOffsetY = 0;

		m_iIntensityRed = 20;
		m_iIntensityOrange = 50;
		m_iIntensityYellow = 80;
		m_iIntensityGreen = 100;
		m_bIntensityInvertScale = false;
		m_bIntensityAmountScaled = false;
		m_bIntensityValuesFixed = false;

		m_iAlignHIcon = ALIGN_RIGHT;
		m_iAlignHLabel = ALIGN_LEFT;
		m_iAlignHAmount = ALIGN_CENTER;

		m_iAlignVIcon = ALIGN_MIDDLE;
		m_iAlignVLabel = ALIGN_MIDDLE;
		m_iAlignVAmount = ALIGN_MIDDLE;

		m_iOffsetXIcon = 5;
		m_iOffsetYIcon = 0;
		m_iOffsetXLabel = -5;
		m_iOffsetYLabel = 0;
		m_iOffsetXAmount = 0;
		m_iOffsetYAmount = 0;
		
		m_iSizeIcon = 2;
		m_iSizeLabel = 2;
		m_iSizeAmount = 2;

		m_bLabelFontTahoma = false;
		m_bAmountFontTahoma = false;

		m_bIconFontShadow = false;
		m_bLabelFontShadow = false;
		m_bAmountFontShadow = false;
		
		m_ColorBarCustom = Color(255,255,255,255);
		m_ColorBarBorderCustom = Color(255,255,255,255);
		m_ColorBarBackgroundCustom = Color(192,192,192,80);
		m_ColorIconCustom = Color(255,255,255,255);
		m_ColorLabelCustom = Color(255,255,255,255);
		m_ColorAmountCustom = Color(255,255,255,255);
		
		m_ColorModeBar = COLOR_MODE_STEPPED;
		m_ColorModeBarBorder = COLOR_MODE_CUSTOM;
		m_ColorModeBarBackground = COLOR_MODE_STEPPED;
		m_ColorModeIcon = COLOR_MODE_CUSTOM;
		m_ColorModeLabel = COLOR_MODE_CUSTOM;
		m_ColorModeAmount = COLOR_MODE_CUSTOM;

		m_ColorIcon = m_ColorIconCustom;
		m_ColorLabel = m_ColorLabelCustom;
		m_ColorAmount = m_ColorAmountCustom;
		m_ColorBar = m_ColorBarCustom;
		m_ColorBarBorder = m_ColorBarBorderCustom;
		m_ColorBarBackground = m_ColorBarBackgroundCustom;

		m_iMaxAmount = 100;

		m_bShowBar = true;
		m_bShowBarBorder = true;
		m_bShowBarBackground = true;
		m_bShowIcon = true;
		m_bShowLabel = true;
		m_bShowAmount = true;
		m_bShowAmountMax = true;

		//do this just in case.
		RecalculateAmountMaxPosition();

		ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	}

	void FFQuantityBar::ApplySchemeSettings( IScheme *pScheme )
	{
		HScheme QuantityBarScheme = scheme()->LoadSchemeFromFile("resource/QuantityPanelScheme.res", "QuantityPanelScheme");
		IScheme *qbScheme = scheme()->GetIScheme(QuantityBarScheme);
				
		//the non scaled fonts are used for positioning calculations
		//for 'simplicity' and consistency everything is calculated as if in 640x480
		//decided I'll make the text size variable too
		//size*3 then offset for shadow and non proportional
		for(int i = 0; i < QUANTITYBARFONTSIZES; ++i)
		{
			m_hfQuantityBarText[i*3] = qbScheme->GetFont( VarArgs("QuantityBar%d",i), true );
			m_hfQuantityBarText[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityBarShadow%d",i), true );
			m_hfQuantityBarText[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityBar%d",i), false );
			m_hfQuantityBarTahomaText[i*3] = qbScheme->GetFont( VarArgs("QuantityBarTahoma%d",i), true );
			m_hfQuantityBarTahomaText[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityBarTahomaShadow%d",i), true );
			m_hfQuantityBarTahomaText[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityBarTahoma%d",i), false );
		}	
		for(int i = 0; i < QUANTITYBARICONSIZES; ++i)
		{
			m_hfQuantityBarIcon[i*3] = qbScheme->GetFont( VarArgs("QuantityBarIcon%d",i), true );
			m_hfQuantityBarIcon[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityBarIconShadow%d",i), true );
			m_hfQuantityBarIcon[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityBarIcon%d",i), false );
			m_hfQuantityBarGlyph[i*3] = qbScheme->GetFont( VarArgs("QuantityBarGlyph%d",i), true );
			m_hfQuantityBarGlyph[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityBarGlyphShadow%d",i), true );
			m_hfQuantityBarGlyph[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityBarGlyph%d",i), false );
		}
		SetBorder(pScheme->GetBorder("ScoreBoardItemBorder"));

		SetPaintBackgroundEnabled(true);
		//SetPaintBackgroundType(2);
		SetPaintBorderEnabled(false);
		//SetPaintEnabled(true);

		//now we've got settings lets update everything
		RecalculateIconPosition();
		RecalculateLabelPosition();
		RecalculateAmountMaxPosition();
		RecalculateQuantity();
		//RecalculateAmountPosition(); <--gets called in Recalculate Quantity

 		BaseClass::ApplySchemeSettings( pScheme );
	}

	void FFQuantityBar::SetAmountFontShadow( bool bHasShadow ) 
	{
		m_bAmountFontShadow = bHasShadow;
	}
	void FFQuantityBar::SetIconFontShadow( bool bHasShadow ) 
	{
		m_bIconFontShadow = bHasShadow;
	}
	void FFQuantityBar::SetLabelFontShadow( bool bHasShadow ) 
	{
		m_bLabelFontShadow = bHasShadow;
	}
	void FFQuantityBar::SetIconFontGlyph( bool bIconIsGlyph, bool bRecalculateBarPositioningOffset  )
	{
		m_bIconFontGlyph = bIconIsGlyph;
		RecalculateIconPosition(bRecalculateBarPositioningOffset );
	}	
	void FFQuantityBar::SetLabelFontTahoma( bool bLabelFontTahoma, bool bRecalculateBarPositioningOffset  )
	{
		if(m_bLabelFontTahoma != bLabelFontTahoma)
		{
			m_bLabelFontTahoma = bLabelFontTahoma;
			RecalculateLabelPosition(bRecalculateBarPositioningOffset );
		}
	}
	void FFQuantityBar::SetAmountFontTahoma( bool bAmountFontTahoma, bool bRecalculateBarPositioningOffset  )
	{
		m_bAmountFontTahoma = bAmountFontTahoma;
		RecalculateAmountPosition();
		RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset);
	}

	
	void FFQuantityBar::SetIconSize( int newIconSize, bool bRecalculateBarPositioningOffset  )
	{
		m_iSizeIcon = newIconSize;
		RecalculateIconPosition(bRecalculateBarPositioningOffset );
	}
	void FFQuantityBar::SetLabelSize( int newLabelSize, bool bRecalculateBarPositioningOffset  )
	{
		m_iSizeLabel = newLabelSize;
		RecalculateLabelPosition(bRecalculateBarPositioningOffset );
	}
	void FFQuantityBar::SetAmountSize( int newAmountSize, bool bRecalculateBarPositioningOffset  )
	{
		m_iSizeAmount = newAmountSize;
		RecalculateAmountPosition();
		RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset);
	}

	void FFQuantityBar::SetAmount( int iAmount )
	{ 
		if(m_iAmount != iAmount)
		{
			m_iAmount = iAmount; 

			char szAmount[10];
			Q_snprintf( szAmount, 5, "%i%", iAmount );
			localize()->ConvertANSIToUnicode( szAmount, m_wszAmount, sizeof( m_wszAmount ) );
			
			RecalculateQuantity();
		}
	}
	void FFQuantityBar::SetAmountMax( int iAmountMax, bool bRecalculateBarPositioningOffset  )
	{
		if(m_iMaxAmount != iAmountMax && iAmountMax != 0) // don't allow 0 cause that doesn't make sense - plus it'll crash with divide by 0 when calculating the colour!
		{
			m_iMaxAmount = iAmountMax; 
		
			char szAmountMax[10];
			Q_snprintf( szAmountMax, 5, "%i%", iAmountMax );
			localize()->ConvertANSIToUnicode( szAmountMax, m_wszAmountMax, sizeof( m_wszAmountMax ) );
			RecalculateQuantity();
			RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset);
		}
	}

	void FFQuantityBar::SetIconChar( char *newIconChar, bool bRecalculateBarPositioningOffset  )
	{ 					
		char szIcon[5];
		Q_snprintf( szIcon, 2, "%s%", newIconChar );
		localize()->ConvertANSIToUnicode( szIcon, m_wszIcon, sizeof( m_wszIcon ) );
		
		RecalculateIconPosition(bRecalculateBarPositioningOffset );
	}
	void FFQuantityBar::SetLabelText( char *newLabelText, bool bRecalculateBarPositioningOffset  )
	{
		wchar_t *pszTemp = localize()->Find( newLabelText );

		if( pszTemp )
			wcscpy( m_wszLabel, pszTemp );
		else
		{
			char szLabel[32];
			Q_snprintf( szLabel, 32, "%s%", newLabelText );
			localize()->ConvertANSIToUnicode( szLabel, m_wszLabel, sizeof( m_wszLabel ) );
		}
		RecalculateLabelPosition(bRecalculateBarPositioningOffset );
	}
	void FFQuantityBar::SetLabelText( wchar_t *newLabelText, bool bRecalculateBarPositioningOffset  )
	{
		wcscpy( m_wszLabel, newLabelText );
		RecalculateLabelPosition(bRecalculateBarPositioningOffset );
	}

	void FFQuantityBar::SetBarWidth( int iBarWidth, bool bRecalculateBarPositioningOffset  ) 
	{ 
		m_iBarWidth = iBarWidth; 

		RecalculateIconPosition(bRecalculateBarPositioningOffset );
		RecalculateLabelPosition(bRecalculateBarPositioningOffset );
		RecalculateAmountPosition();
		RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset);
	}
	void FFQuantityBar::SetBarHeight( int iBarHeight, bool bRecalculateBarPositioningOffset  ) 
	{ 
		m_iBarHeight = iBarHeight; 

		RecalculateIconPosition(bRecalculateBarPositioningOffset );
		RecalculateLabelPosition(bRecalculateBarPositioningOffset );
		RecalculateAmountPosition();
		RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset);

	}
	void FFQuantityBar::SetBarSize( int iBarWidth, int iBarHeight, bool bRecalculateBarPositioningOffset  ) 
	{ 
		m_iBarWidth = iBarWidth; 
		m_iBarHeight = iBarHeight;

		RecalculateIconPosition(bRecalculateBarPositioningOffset );
		RecalculateLabelPosition(bRecalculateBarPositioningOffset );
		RecalculateAmountPosition();
		RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset);
	}

	void FFQuantityBar::SetBarBorderWidth( int iBarBorderWidth, bool bRecalculateBarPositioningOffset  ) { 
		m_iBarBorderWidth = iBarBorderWidth; 
		if(bRecalculateBarPositioningOffset )
			RecalculateBarPositioningOffset();
	}
	
	void FFQuantityBar::SetBarOrientation( int iOrientation ) { 
		m_iBarOrientation = iOrientation; 
		RecalculateQuantity(); 
		//TODO: recalculate others?
	}

	//TO-DO
	//see if these change the dimentions and if an update is required - but first check if the value has really changed...
	void FFQuantityBar::ShowBar( bool bShowBar, bool bRecalculateBarPositioningOffset  ) 
	{ 
		m_bShowBar = bShowBar;
		if(bRecalculateBarPositioningOffset )
			RecalculateBarPositioningOffset();
	}
	void FFQuantityBar::ShowBarBackground( bool bShowBarBackground, bool bRecalculateBarPositioningOffset  ) 
	{ 
		m_bShowBarBackground = bShowBarBackground; 
		if(bRecalculateBarPositioningOffset )
			RecalculateBarPositioningOffset();
	}
	void FFQuantityBar::ShowBarBorder( bool bShowBarBorder, bool bRecalculateBarPositioningOffset  ) 
	{ 
		m_bShowBarBorder = bShowBarBorder; 
		if(bRecalculateBarPositioningOffset )
			RecalculateBarPositioningOffset();
	}
	void FFQuantityBar::ShowIcon( bool bShowIcon, bool bRecalculateBarPositioningOffset  ) 
	{ 
		m_bShowIcon = bShowIcon; 
		if(bRecalculateBarPositioningOffset )
			RecalculateBarPositioningOffset();
	}
	void FFQuantityBar::ShowLabel( bool bShowLabel, bool bRecalculateBarPositioningOffset  ) 
	{ 
		m_bShowLabel = bShowLabel; 
		if(bRecalculateBarPositioningOffset )
			RecalculateBarPositioningOffset();
	}
	void FFQuantityBar::ShowAmount( bool bShowAmount, bool bRecalculateBarPositioningOffset  ) { 
		m_bShowAmount = bShowAmount; 
		if(bRecalculateBarPositioningOffset )
			RecalculateBarPositioningOffset();
	}
	void FFQuantityBar::ShowAmountMax( bool bShowMax, bool bRecalculateBarPositioningOffset  ) { 
		m_bShowAmountMax = bShowMax; 
		RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset);
	}

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
		if(m_ColorTeam != newTeamColor)
		{
			m_ColorTeam = newTeamColor; 
			if(m_ColorModeBar == COLOR_MODE_TEAMCOLORED)
				RecalculateColor(m_ColorModeBar, m_ColorBar, m_ColorBarCustom);
			if(m_ColorModeBarBorder == COLOR_MODE_TEAMCOLORED)
				RecalculateColor(m_ColorModeBarBorder, m_ColorBarBorder, m_ColorBarBorderCustom);
			if(m_ColorModeBarBackground == COLOR_MODE_TEAMCOLORED)
				RecalculateColor(m_ColorModeBarBackground, m_ColorBarBackground, m_ColorBarBackgroundCustom);
			if(m_ColorModeIcon == COLOR_MODE_TEAMCOLORED)
				RecalculateColor(m_ColorModeIcon, m_ColorIcon, m_ColorIconCustom);
			if(m_ColorModeLabel == COLOR_MODE_TEAMCOLORED)
				RecalculateColor(m_ColorModeLabel, m_ColorLabel, m_ColorLabelCustom);
			if(m_ColorModeAmount == COLOR_MODE_TEAMCOLORED)
				RecalculateColor(m_ColorModeAmount, m_ColorAmount, m_ColorAmountCustom);
		}
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

	void FFQuantityBar::SetIconOffsetX( int iconOffsetX, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iOffsetXIcon = iconOffsetX; 
		RecalculateIconPosition(bRecalculateBarPositioningOffset ); 
	}
	void FFQuantityBar::SetIconOffsetY( int iconOffsetY, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iOffsetYIcon = iconOffsetY; 
		RecalculateIconPosition(bRecalculateBarPositioningOffset ); 
	}
	void FFQuantityBar::SetIconOffset( int iconOffsetX, int iconOffsetY, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iOffsetXIcon = iconOffsetX; 
		m_iOffsetYIcon = iconOffsetY;
		RecalculateIconPosition(bRecalculateBarPositioningOffset );
	}

	void FFQuantityBar::SetLabelOffsetX( int labelOffsetX, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iOffsetXLabel = labelOffsetX; 
		RecalculateLabelPosition(bRecalculateBarPositioningOffset ); 
	}
	void FFQuantityBar::SetLabelOffsetY( int labelOffsetY, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iOffsetYLabel = labelOffsetY; 
		RecalculateLabelPosition(bRecalculateBarPositioningOffset );
	}
	void FFQuantityBar::SetLabelOffset( int labelOffsetX, int labelOffsetY, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iOffsetXLabel = labelOffsetX; 
		m_iOffsetYLabel = labelOffsetY; 
		RecalculateLabelPosition(bRecalculateBarPositioningOffset ); 
	}

	void FFQuantityBar::SetAmountOffsetX( int amountOffsetX, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iOffsetXAmount = amountOffsetX; 
		RecalculateAmountPosition(); 
		RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset); 
	}
	void FFQuantityBar::SetAmountOffsetY( int amountOffsetY, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iOffsetYAmount = amountOffsetY; 
		RecalculateAmountPosition(); 
		RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset); 
	}
	void FFQuantityBar::SetAmountOffset( int amountOffsetX, int amountOffsetY, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iOffsetXAmount = amountOffsetX; 
		m_iOffsetYAmount = amountOffsetY; 
		RecalculateAmountPosition(); 
		RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset); 
	}

	int FFQuantityBar::GetAmount( ) {return m_iAmount; }
		
	void FFQuantityBar::SetLabelAlignmentHorizontal( int iAlignHLabel, bool bRecalculateBarPositioningOffset ) { 
		m_iAlignHLabel = iAlignHLabel; 
		RecalculateLabelPosition(bRecalculateBarPositioningOffset ); 
	}
	void FFQuantityBar::SetAmountAlignmentHorizontal( int iAlignHAmount, bool bRecalculateBarPositioningOffset )
	{
		m_iAlignHAmount = iAlignHAmount; 
		RecalculateAmountPosition(); RecalculateAmountMaxPosition(); 
	}
	void FFQuantityBar::SetIconAlignmentHorizontal( int iAlignHIcon, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iAlignHIcon = iAlignHIcon; 
		RecalculateIconPosition(bRecalculateBarPositioningOffset ); 
	}
		
	void FFQuantityBar::SetLabelAlignmentVertical( int iAlignVLabel, bool bRecalculateBarPositioningOffset ) 
	{
		m_iAlignVLabel = iAlignVLabel; 
		RecalculateLabelPosition(bRecalculateBarPositioningOffset ); 
	}
	void FFQuantityBar::SetAmountAlignmentVertical( int iAlignVAmount, bool bRecalculateBarPositioningOffset ) 
	{
		m_iAlignVAmount = iAlignVAmount; 
		RecalculateAmountPosition(); 
		RecalculateAmountMaxPosition(bRecalculateBarPositioningOffset); 
	}
	void FFQuantityBar::SetIconAlignmentVertical( int iAlignVIcon, bool bRecalculateBarPositioningOffset ) 
	{ 
		m_iAlignVIcon = iAlignVIcon; 
		RecalculateIconPosition(bRecalculateBarPositioningOffset ); 
	}
		
	void FFQuantityBar::SetIntensityControl( int iRed, int iOrange,int iYellow, int iGreen, bool bInvertScale )
	{
		m_iIntensityRed = iRed;
		m_iIntensityOrange = iOrange;
		m_iIntensityYellow = iYellow;
		m_iIntensityGreen = iGreen;
		m_bIntensityInvertScale = bInvertScale;
	}

	void FFQuantityBar::SetIntensityAmountScaled( bool bAmountScaled )
	{
		m_bIntensityAmountScaled = bAmountScaled;
	}
	void FFQuantityBar::SetIntensityValuesFixed( bool bvaluesFixed )
	{
		m_bIntensityValuesFixed = bvaluesFixed;
	}
	bool FFQuantityBar::IsIntensityValuesFixed( )
	{
		return m_bIntensityValuesFixed;
	}

	void FFQuantityBar::OnTick( )
	{
		if (!engine->IsInGame()) 
			return;

		// Get the screen width/height
		int iScreenWide, iScreenTall;
		surface()->GetScreenSize( iScreenWide, iScreenTall );

		// "map" screen res to 640/480
		float flScaleX = 1 / (640.0f / iScreenWide);
		float flScaleY = 1 / (480.0f / iScreenTall);

		if( m_flScaleX != flScaleX || m_flScaleY != flScaleY)
		// if user changes resolution
		{
			m_flScaleX = flScaleX;
			m_flScaleY = flScaleY;
			m_flScale = (m_flScaleX<=m_flScaleY ? m_flScaleX : m_flScaleY);

			RecalculateIconPosition(false);
			RecalculateLabelPosition(false);
			RecalculateAmountPosition();
			RecalculateAmountMaxPosition(false);
			RecalculateBarPositioningOffset();

			SetChildDimentionsChanged(true);

		}
		else if(m_bTriggerParentPositionUpdate)
		{	
			m_bTriggerParentPositionUpdate = false;
			SetChildDimentionsChanged(true);
		}
	}

	bool FFQuantityBar::GetChildDimentionsChanged()
	{
		return m_bChildDimentionsChanged;
	}
	void FFQuantityBar::SetChildDimentionsChanged( bool bChildDimentionsChanged )
	{
		//if ChildDimentionsChanged boolean is changing
		if(m_bChildDimentionsChanged != bChildDimentionsChanged) 
		{
			//send update to parent for positioning	
			if(bChildDimentionsChanged && GetVParent())
			{
				KeyValues *msg = new KeyValues("ChildDimentionsChanged");
				PostMessage(GetVParent(), msg);
			}
			m_bChildDimentionsChanged = bChildDimentionsChanged;
		}		
	}

	void FFQuantityBar::GetPanelPositioningData( int& iWidth, int& iHeight, int& iOffsetX, int&  iOffsetY )
	{
		iWidth = m_iWidth;
		iHeight = m_iHeight;
		iOffsetX = m_iOffsetX;
		iOffsetY = m_iOffsetY;
	}
	
	void FFQuantityBar::SetPosOffset( int iLeft, int iTop )
	{
		m_iOffsetOverrideX = iLeft;
		m_iOffsetOverrideY = iTop;
	}

	void FFQuantityBar::RecalculateBarPositioningOffset( )
	{
		int iX0 = 0;
		int iY0 = 0;
		int iX1 = 0;
		int iY1 = 0;
		
		if( m_bShowBar || m_bShowBarBackground || m_bShowBarBorder )
		{
			iX1 += m_iBarWidth;
			iY1 += m_iBarHeight;
		}

		if( m_bShowBarBorder )
		{
			iX1 += m_iBarBorderWidth;
			iY1 += m_iBarBorderWidth;
			iX0 -= m_iBarBorderWidth;
			iY0 -= m_iBarBorderWidth;
		}

		if( m_bShowIcon && m_iIconPosX < iX0 )
			iX0 = m_iIconPosX;
		if( m_bShowLabel && m_iLabelPosX < iX0 )
			iX0 = m_iLabelPosX;
		if( m_bShowAmount && m_iAmountMaxPosX < iX0 )
			iX0 = m_iAmountMaxPosX;

		if( m_bShowIcon && m_iIconPosY < iY0 )
			iY0 = m_iIconPosY;
		if( m_bShowLabel && m_iLabelPosY < iY0 )
			iY0 = m_iLabelPosY;
		if( m_bShowAmount && m_iAmountMaxPosY < iY0 )
			iY0 = m_iAmountMaxPosY;

		if( m_bShowIcon && (m_iIconPosX + m_iIconWidth) > iX1 )
			iX1 = m_iIconPosX + m_iIconWidth;
		if( m_bShowLabel && (m_iLabelPosX + m_iLabelWidth) > iX1 )
			iX1 = m_iLabelPosX + m_iLabelWidth;
		//we use max amount instea of amount width/height to make sure it doesnt keep moving
		if( m_bShowAmount && (m_iAmountMaxPosX + m_iAmountMaxWidth) > iX1 )
			iX1 = m_iAmountMaxPosX + m_iAmountMaxWidth;

		if( m_bShowIcon && (m_iIconPosY + m_iIconHeight) > iY1 )
			iY1 = m_iIconPosY + m_iIconHeight;
		if( m_bShowLabel && (m_iLabelPosY + m_iLabelHeight) > iY1 )
			iY1 = m_iLabelPosY + m_iLabelHeight;
		//we use max amount instea of amount width/height to make sure it doesnt keep moving
		if( m_bShowAmount && (m_iAmountMaxPosY + m_iAmountMaxHeight) > iY1 )
			iY1 = m_iAmountMaxPosY + m_iAmountMaxHeight;
		
		if(m_iWidth != (iX1 - iX0) || m_iHeight != (iY1 - iY0))
			m_bTriggerParentPositionUpdate = true;

		m_iWidth = iX1 - iX0;
		m_iHeight = iY1 - iY0;
		m_iOffsetX = -iX0;
		m_iOffsetY = -iY0;

		//just so if we ever use the quantity bar on its own and not in a panel it will set it's own size
		if( m_iOffsetOverrideX == -1 || m_iOffsetOverrideY == -1 )
			SetSize(m_iWidth * m_flScale, m_iHeight * m_flScale);
	}
	
	void FFQuantityBar::SetStyle(KeyValues *kvStyleData, KeyValues *kvDefaultStyleData )
	{
		bool bRecalculateBarPositioningOffset = false;

		//-1 default so that we don't change the values if it doesn't exist for any reason
		int iBarWidth = kvStyleData->GetInt("barWidth", kvDefaultStyleData->GetInt("barWidth", -1));
		int iBarHeight = kvStyleData->GetInt("barHeight", kvDefaultStyleData->GetInt("barHeight", -1));
		int iBarBorderWidth = kvStyleData->GetInt("barBorderWidth", kvDefaultStyleData->GetInt("barBorderWidth", -1));
		int iBarOrientation = kvStyleData->GetInt("barOrientation", kvDefaultStyleData->GetInt("barOrientation", -1));

		// I feel better checkeing things have changed before setting them else we might call lots of unneccesary methods
		if(iBarWidth != -1)
		{
			SetBarWidth(iBarWidth, false);
			bRecalculateBarPositioningOffset = true;
		}

		if(iBarHeight != -1)
		{
			SetBarHeight(iBarHeight, false);
			bRecalculateBarPositioningOffset = true;
		}

		if(iBarBorderWidth != -1)
		{
			SetBarBorderWidth(iBarBorderWidth, false);
			bRecalculateBarPositioningOffset = true;
		}

		if(iBarOrientation != -1)
			SetBarOrientation(iBarOrientation);

		KeyValues *kvComponentStyleData = kvStyleData->FindKey("Bar");
		if(!kvComponentStyleData)
			kvComponentStyleData = kvDefaultStyleData->FindKey("Bar");
		if(kvComponentStyleData)
		{
			int iShow = kvComponentStyleData->GetInt("show", -1);

			int iRed = kvComponentStyleData->GetInt("red", -1);
			int iGreen = kvComponentStyleData->GetInt("green", -1);
			int iBlue = kvComponentStyleData->GetInt("blue", -1);
			int iAlpha = kvComponentStyleData->GetInt("alpha", -1);
			int iColorMode = kvComponentStyleData->GetInt("colorMode", -1);

			bool bValidColor = false;

			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
				bValidColor = true;

			if((iShow == 1 ? true : false) != m_bShowBar && iShow != -1)
			{
				ShowBar(!m_bShowBar, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_ColorModeBar != iColorMode && iColorMode != -1)
				SetBarColorMode(iColorMode);
			if(m_ColorBarCustom != color && bValidColor)
				SetBarColor(color);
		}

		kvComponentStyleData = kvStyleData->FindKey("BarBorder");
		if(!kvComponentStyleData)
			kvComponentStyleData = kvDefaultStyleData->FindKey("BarBorder");
		if(kvComponentStyleData)
		{
			int iShow = kvComponentStyleData->GetInt("show", -1);

			int iRed = kvComponentStyleData->GetInt("red", -1);
			int iGreen = kvComponentStyleData->GetInt("green", -1);
			int iBlue = kvComponentStyleData->GetInt("blue", -1);
			int iAlpha = kvComponentStyleData->GetInt("alpha", -1);
			int iColorMode = kvComponentStyleData->GetInt("colorMode", -1);

			bool bValidColor = false;

			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
				bValidColor = true;

			if((iShow == 1 ? true : false) != m_bShowBarBorder && iShow != -1)
			{
				ShowBarBorder(!m_bShowBarBorder, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_ColorModeBarBorder != iColorMode && iColorMode != -1)
				SetBarBorderColorMode(iColorMode);
			if(m_ColorBarBorderCustom != color && bValidColor)
				SetBarBorderColor(color);
		}

		kvComponentStyleData = kvStyleData->FindKey("BarBackground");
		if(!kvComponentStyleData)
			kvComponentStyleData = kvDefaultStyleData->FindKey("BarBackground");
		if(kvComponentStyleData)
		{
			int iShow = kvComponentStyleData->GetInt("show", -1);

			int iRed = kvComponentStyleData->GetInt("red", -1);
			int iGreen = kvComponentStyleData->GetInt("green", -1);
			int iBlue = kvComponentStyleData->GetInt("blue", -1);
			int iAlpha = kvComponentStyleData->GetInt("alpha", -1);
			int iColorMode = kvComponentStyleData->GetInt("colorMode", -1);

			bool bValidColor = false;

			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
				bValidColor = true;

			if((iShow == 1 ? true : false) != m_bShowBarBackground && iShow != -1)
			{
				ShowBarBackground(!m_bShowBarBackground, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_ColorModeBarBackground != iColorMode && iColorMode != -1)
				SetBarBackgroundColorMode(iColorMode);
			if(m_ColorBarBackgroundCustom != color && bValidColor)
				SetBarBackgroundColor(color);
		}

		kvComponentStyleData = kvStyleData->FindKey("Icon");
		if(!kvComponentStyleData)
			kvComponentStyleData = kvDefaultStyleData->FindKey("Icon");
		if(kvComponentStyleData)
		{
			int iShow = kvComponentStyleData->GetInt("show", -1);
			int iSize = kvComponentStyleData->GetInt("size", -1);
			int iShadow = kvComponentStyleData->GetInt("shadow", -1);

			int iRed = kvComponentStyleData->GetInt("red", -1);
			int iGreen = kvComponentStyleData->GetInt("green", -1);
			int iBlue = kvComponentStyleData->GetInt("blue", -1);
			int iAlpha = kvComponentStyleData->GetInt("alpha", -1);
			int iColorMode = kvComponentStyleData->GetInt("colorMode", -1);

			int iAlignH = kvComponentStyleData->GetInt("alignH", -1);
			int iAlignV = kvComponentStyleData->GetInt("alignV", -1);

			int iOffsetX = kvComponentStyleData->GetInt("offsetX", -1);
			int iOffsetY = kvComponentStyleData->GetInt("offsetY", -1);

			bool bValidColor = false;

			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
				bValidColor = true;

			if((iShow == 1 ? true : false) != m_bShowIcon && iShow != -1)
			{
				ShowIcon(!m_bShowIcon, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_ColorModeIcon != iColorMode && iColorMode != -1)
				SetIconColorMode(iColorMode);
			if(m_ColorIconCustom != color && bValidColor)
				SetIconColor(color);
			if(m_iOffsetXIcon != iOffsetX && iOffsetX != -1)
			{
				SetIconOffsetX(iOffsetX, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iOffsetYIcon != iOffsetY && iOffsetY != -1)
			{
				SetIconOffsetY(iOffsetY, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iAlignHIcon != iAlignH && iAlignH != -1)
			{
				SetIconAlignmentHorizontal(iAlignH, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iAlignVIcon != iAlignV && iAlignV != -1)
			{
				SetIconAlignmentVertical(iAlignV, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iSizeIcon != iSize && iSize != -1)
			{
				SetIconSize(iSize, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_bIconFontShadow != (iShadow == 1 ? true : false) && iShadow != -1)
				SetIconFontShadow(!m_bIconFontShadow);
		}

		kvComponentStyleData = kvStyleData->FindKey("Label");
		if(!kvComponentStyleData)
			kvComponentStyleData = kvDefaultStyleData->FindKey("Label");
		if(kvComponentStyleData)
		{
			int iShow = kvComponentStyleData->GetInt("show", -1);
			int iSize = kvComponentStyleData->GetInt("size", -1);
			int iShadow = kvComponentStyleData->GetInt("shadow", -1);

			int iRed = kvComponentStyleData->GetInt("red", -1);
			int iGreen = kvComponentStyleData->GetInt("green", -1);
			int iBlue = kvComponentStyleData->GetInt("blue", -1);
			int iAlpha = kvComponentStyleData->GetInt("alpha", -1);
			int iColorMode = kvComponentStyleData->GetInt("colorMode", -1);

			int iAlignH = kvComponentStyleData->GetInt("alignH", -1);
			int iAlignV = kvComponentStyleData->GetInt("alignV", -1);

			int iOffsetX = kvComponentStyleData->GetInt("offsetX", -1);
			int iOffsetY = kvComponentStyleData->GetInt("offsetY", -1);

			int iFontTahoma = kvComponentStyleData->GetInt("fontTahoma", -1);

			bool bValidColor = false;

			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
				bValidColor = true;

			if((iShow == 1 ? true : false) != m_bShowLabel && iShow != -1)
			{
				ShowLabel(!m_bShowLabel, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_ColorModeLabel != iColorMode && iColorMode != -1)
				SetLabelColorMode(iColorMode);
			if(m_ColorLabelCustom != color && bValidColor)
				SetLabelColor(color);
			if(m_iOffsetXLabel != iOffsetX && iOffsetX != -1)
			{
				SetLabelOffsetX(iOffsetX, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iOffsetYLabel != iOffsetY && iOffsetY != -1)
			{
				SetLabelOffsetY(iOffsetY, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iAlignHLabel != iAlignH && iAlignH != -1)
			{
				SetLabelAlignmentHorizontal(iAlignH, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iAlignVLabel != iAlignV && iAlignV != -1)
			{
				SetLabelAlignmentVertical(iAlignV, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iSizeLabel != iSize && iSize != -1)
				SetLabelSize(iSize, false);
			if(m_bLabelFontShadow != (iShadow == 1 ? true : false) && iShadow != -1)
				SetLabelFontShadow(!m_bLabelFontShadow);
			if((iFontTahoma == 1 ? true : false) != m_bLabelFontTahoma && iFontTahoma != -1)
			{
				SetLabelFontTahoma(!m_bLabelFontTahoma, false);
				bRecalculateBarPositioningOffset = true;
			}
		}

		kvComponentStyleData = kvStyleData->FindKey("Amount");
		if(!kvComponentStyleData)
			kvComponentStyleData = kvDefaultStyleData->FindKey("Amount");
		if(kvComponentStyleData)
		{
			int iShow = kvComponentStyleData->GetInt("show", -1);
			int iSize = kvComponentStyleData->GetInt("size", -1);
			int iShadow = kvComponentStyleData->GetInt("shadow", -1);

			int iRed = kvComponentStyleData->GetInt("red", -1);
			int iGreen = kvComponentStyleData->GetInt("green", -1);
			int iBlue = kvComponentStyleData->GetInt("blue", -1);
			int iAlpha = kvComponentStyleData->GetInt("alpha", -1);
			int iColorMode = kvComponentStyleData->GetInt("colorMode", -1);

			int iAlignH = kvComponentStyleData->GetInt("alignH", -1);
			int iAlignV = kvComponentStyleData->GetInt("alignV", -1);

			int iOffsetX = kvComponentStyleData->GetInt("offsetX", -1);
			int iOffsetY = kvComponentStyleData->GetInt("offsetY", -1);

			int iFontTahoma = kvComponentStyleData->GetInt("fontTahoma", -1);

			bool bValidColor = false;

			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
				bValidColor = true;

			if((iShow == 1 ? true : false) != m_bShowAmount && iShow != -1)
			{
				ShowAmount(!m_bShowAmount, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_ColorModeAmount != iColorMode && iColorMode != -1)
				SetAmountColorMode(iColorMode);
			if(m_ColorAmountCustom != color && bValidColor)
				SetAmountColor(color);
			if(m_iOffsetXAmount != iOffsetX && iOffsetX != -1)
			{
				SetAmountOffsetX(iOffsetX, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iOffsetYAmount != iOffsetY && iOffsetY != -1)
			{
				SetAmountOffsetY(iOffsetY, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iAlignHAmount != iAlignH && iAlignH != -1)
			{
				SetAmountAlignmentHorizontal(iAlignH, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iAlignVAmount != iAlignV && iAlignV != -1)
			{
				SetAmountAlignmentVertical(iAlignV, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_iSizeAmount != iSize && iSize != -1)
			{
				SetAmountSize(iSize, false);
				bRecalculateBarPositioningOffset = true;
			}
			if(m_bAmountFontShadow != (iShadow == 1 ? true : false) && iShadow != -1)
				SetAmountFontShadow(!m_bAmountFontShadow);
			if((iFontTahoma == 1 ? true : false) != m_bAmountFontTahoma && iFontTahoma != -1)
			{
				SetAmountFontTahoma(!m_bAmountFontTahoma, false);
				bRecalculateBarPositioningOffset = true;
			}
		}

		if(bRecalculateBarPositioningOffset)
			RecalculateBarPositioningOffset();
	}
	
	void FFQuantityBar::RecalculateIconPosition( bool bRecalculateBarPositioningOffset  )
	{
		int iIconAlignmentOffsetX, iIconAlignmentOffsetY;

		if(m_bIconFontGlyph)
			CalculateTextAlignmentOffset(iIconAlignmentOffsetX, iIconAlignmentOffsetY, m_iIconWidth, m_iIconHeight, m_iAlignHIcon, m_iAlignVIcon, m_hfQuantityBarGlyph[m_iSizeIcon*3 + 2], m_wszIcon);
		else
			CalculateTextAlignmentOffset(iIconAlignmentOffsetX, iIconAlignmentOffsetY, m_iIconWidth, m_iIconHeight, m_iAlignHIcon, m_iAlignVIcon, m_hfQuantityBarIcon[m_iSizeIcon*3 + 2], m_wszIcon);
		
		m_iIconPosX = m_iOffsetXIcon + iIconAlignmentOffsetX;
		m_iIconPosY = m_iOffsetYIcon + iIconAlignmentOffsetY;
		
		if(bRecalculateBarPositioningOffset )
			RecalculateBarPositioningOffset();
	}

	void FFQuantityBar::RecalculateLabelPosition( bool bRecalculateBarPositioningOffset  )
	{
		int iLabelAlignmentOffsetX, iLabelAlignmentOffsetY;

		if(m_bLabelFontTahoma)
			CalculateTextAlignmentOffset(iLabelAlignmentOffsetX, iLabelAlignmentOffsetY, m_iLabelWidth, m_iLabelHeight, m_iAlignHLabel, m_iAlignVLabel, m_hfQuantityBarTahomaText[m_iSizeLabel*3 + 2], m_wszLabel);
		else
			CalculateTextAlignmentOffset(iLabelAlignmentOffsetX, iLabelAlignmentOffsetY, m_iLabelWidth, m_iLabelHeight, m_iAlignHLabel, m_iAlignVLabel, m_hfQuantityBarText[m_iSizeLabel*3 + 2], m_wszLabel);
		
		m_iLabelPosX = m_iOffsetXLabel + iLabelAlignmentOffsetX;
		m_iLabelPosY = m_iOffsetYLabel + iLabelAlignmentOffsetY;
		
		if(bRecalculateBarPositioningOffset )
			RecalculateBarPositioningOffset();
	}

	void FFQuantityBar::RecalculateAmountPosition( )
	{
		int iAmountAlignmentOffsetX, iAmountAlignmentOffsetY;

		if(m_bAmountFontTahoma)
			CalculateTextAlignmentOffset(iAmountAlignmentOffsetX, iAmountAlignmentOffsetY, m_iAmountWidth, m_iAmountHeight, m_iAlignHAmount, m_iAlignVAmount, m_hfQuantityBarTahomaText[m_iSizeAmount*3 + 2], m_wszAmountString);
		else
			CalculateTextAlignmentOffset(iAmountAlignmentOffsetX, iAmountAlignmentOffsetY, m_iAmountWidth, m_iAmountHeight, m_iAlignHAmount, m_iAlignVAmount, m_hfQuantityBarText[m_iSizeAmount*3 + 2], m_wszAmountString);
		m_iAmountPosX = m_iOffsetXAmount + iAmountAlignmentOffsetX;
		m_iAmountPosY = m_iOffsetYAmount + iAmountAlignmentOffsetY;
	}

	void FFQuantityBar::RecalculateAmountMaxPosition( bool bRecalculateBarPositioningOffset  )
	{
		wchar_t wszMaxAmountString[ 10 ];

		if(!m_bShowAmountMax)
		{
			_snwprintf( wszMaxAmountString, 9, L"100%%");			
		}
		else
		{
			_snwprintf( wszMaxAmountString, 9, L"%s/%s", m_wszAmountMax, m_wszAmountMax );
		}
		
		int iAmountMaxAlignmentOffsetX;
		int iAmountMaxAlignmentOffsetY;
		
		if(m_bAmountFontTahoma)
			CalculateTextAlignmentOffset(iAmountMaxAlignmentOffsetX, iAmountMaxAlignmentOffsetY, m_iAmountMaxWidth, m_iAmountMaxHeight, m_iAlignHAmount, m_iAlignVAmount, m_hfQuantityBarTahomaText[m_iSizeAmount*3 + 2], wszMaxAmountString);
		else
			CalculateTextAlignmentOffset(iAmountMaxAlignmentOffsetX, iAmountMaxAlignmentOffsetY, m_iAmountMaxWidth, m_iAmountMaxHeight, m_iAlignHAmount, m_iAlignVAmount, m_hfQuantityBarText[m_iSizeAmount*3 + 2], wszMaxAmountString);
		m_iAmountMaxPosX = m_iOffsetXAmount + iAmountMaxAlignmentOffsetX;
		m_iAmountMaxPosY = m_iOffsetYAmount + iAmountMaxAlignmentOffsetY;

		if(bRecalculateBarPositioningOffset)
			RecalculateBarPositioningOffset();
	}

	void FFQuantityBar::Paint( )
	{
		int iOffsetX;
		int iOffsetY;

		if(m_iOffsetOverrideX != -1)
			iOffsetX = m_iOffsetOverrideX;
		else
			iOffsetX = m_iOffsetX;

		if(m_iOffsetOverrideX != -1)
			iOffsetY = m_iOffsetOverrideY;
		else
			iOffsetY = m_iOffsetY;

		// Set text position based on alignment & text
		if(m_bShowBarBorder)
		{
			surface()->DrawSetColor( m_ColorBarBorder );
			for( int i = 1; i <= (m_iBarBorderWidth * m_flScale); ++i )
			{
				surface()->DrawOutlinedRect( 
					iOffsetX * m_flScale - i, 
					iOffsetY * m_flScale - i, 
					(iOffsetX + m_iBarWidth) * m_flScale + i, 
					(iOffsetY + m_iBarHeight) * m_flScale + i
					);
			}
		}

		if(m_bShowBarBackground)
		{
			surface()->DrawSetColor( m_ColorBarBackground );
			surface()->DrawFilledRect( 
				iOffsetX * m_flScale, 
				iOffsetY * m_flScale, 
				(iOffsetX + m_iBarWidth) * m_flScale, 
				(iOffsetY + m_iBarHeight) * m_flScale
				);
		}

		if(m_bShowBar)
		{
			surface()->DrawSetColor( m_ColorBar );
			surface()->DrawFilledRect( 
				(iOffsetX + m_iBarX0QuantityOffset) * m_flScale,
				(iOffsetY + m_iBarY0QuantityOffset) * m_flScale,
				(iOffsetX + m_iBarWidth + m_iBarX1QuantityOffset) * m_flScale,
				(iOffsetY + m_iBarHeight + m_iBarY1QuantityOffset) * m_flScale
				);
		}

		if(m_bShowIcon && m_wszIcon)
		{	
			if(m_bIconFontGlyph)
				surface()->DrawSetTextFont(m_hfQuantityBarGlyph[m_iSizeIcon * 3 + (m_bIconFontShadow ? 1 : 0)]);
			else
				surface()->DrawSetTextFont(m_hfQuantityBarIcon[m_iSizeIcon * 3 + (m_bIconFontShadow ? 1 : 0)]);

			surface()->DrawSetTextColor( m_ColorIcon );
			surface()->DrawSetTextPos((iOffsetX + m_iIconPosX) * m_flScale, (iOffsetY + m_iIconPosY) * m_flScale);
			surface()->DrawUnicodeString( m_wszIcon );
		}

		if(m_bShowLabel && m_wszLabel)
		{	
			if(m_bLabelFontTahoma)
				surface()->DrawSetTextFont(m_hfQuantityBarTahomaText[m_iSizeLabel * 3 + (m_bLabelFontShadow ? 1 : 0)]);
			else
				surface()->DrawSetTextFont(m_hfQuantityBarText[m_iSizeLabel * 3 + (m_bLabelFontShadow ? 1 : 0)]);

			
			surface()->DrawSetTextColor( m_ColorLabel );
			surface()->DrawSetTextPos((iOffsetX + m_iLabelPosX) * m_flScale, (iOffsetY + m_iLabelPosY) * m_flScale);
			surface()->DrawUnicodeString( m_wszLabel );
		}

		if(m_bShowAmount && m_wszAmount)
		{
			if(m_bAmountFontTahoma)
				surface()->DrawSetTextFont(m_hfQuantityBarTahomaText[m_iSizeAmount * 3 + (m_bAmountFontShadow ? 1 : 0)]);
			else
				surface()->DrawSetTextFont(m_hfQuantityBarText[m_iSizeAmount * 3 + (m_bAmountFontShadow ? 1 : 0)]);
			surface()->DrawSetTextColor( m_ColorAmount );
			surface()->DrawSetTextPos((iOffsetX + m_iAmountPosX) * m_flScale, (iOffsetY + m_iAmountPosY) * m_flScale);
			surface()->DrawUnicodeString( m_wszAmountString );
		}

		BaseClass::Paint();
	}

	void FFQuantityBar::RecalculateQuantity( )
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

				localize()->ConvertANSIToUnicode( szPercent, wszPercent, sizeof( wszPercent ) );

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
			m_ColorAmount.SetColor(m_ColorIntensityFaded.r(), m_ColorIntensityFaded.g(), m_ColorIntensityFaded.b(),m_ColorAmountCustom.a());
		else if(m_ColorModeAmount == COLOR_MODE_STEPPED )
			m_ColorAmount.SetColor(m_ColorIntensityStepped.r(), m_ColorIntensityStepped.g(), m_ColorIntensityStepped.b(),m_ColorAmountCustom.a());

		if(m_iBarOrientation == ORIENTATION_HORIZONTAL_INVERTED)
		{
			m_iBarX0QuantityOffset = m_iBarWidth -(m_iBarHeight * m_iAmount/m_iMaxAmount);
			if( (m_iBarX1QuantityOffset + m_iBarY0QuantityOffset + m_iBarY1QuantityOffset) != 0 )
			{
				m_iBarX1QuantityOffset = 0;
				m_iBarY0QuantityOffset = 0;
				m_iBarY1QuantityOffset = 0;
			}
		}
		else if(m_iBarOrientation == ORIENTATION_VERTICAL)
		{
			m_iBarY0QuantityOffset = m_iBarHeight - (m_iBarHeight * m_iAmount/m_iMaxAmount);
			if( (m_iBarX0QuantityOffset + m_iBarX1QuantityOffset + m_iBarY1QuantityOffset) != 0 )
			{
				m_iBarX0QuantityOffset = 0;
				m_iBarX1QuantityOffset = 0;
				m_iBarY1QuantityOffset = 0;
			}
		}
		else if(m_iBarOrientation == ORIENTATION_VERTICAL_INVERTED)
		{
			m_iBarY1QuantityOffset = (m_iBarHeight * m_iAmount/m_iMaxAmount) - m_iBarHeight;
			if( (m_iBarX0QuantityOffset + m_iBarX1QuantityOffset + m_iBarY0QuantityOffset) != 0 )
			{
				m_iBarX0QuantityOffset = 0;
				m_iBarX1QuantityOffset = 0;
				m_iBarY0QuantityOffset = 0;
			}
		}
		else //m_iBarOrientation == ORIENTATION_HORIZONTAL (or anything else)
		{
			m_iBarX1QuantityOffset = (m_iBarWidth * m_iAmount/m_iMaxAmount) - m_iBarWidth;
			if( (m_iBarX0QuantityOffset + m_iBarY0QuantityOffset + m_iBarY1QuantityOffset) != 0 )
			{
				m_iBarX0QuantityOffset = 0;
				m_iBarY0QuantityOffset = 0;
				m_iBarY1QuantityOffset = 0;
			}
		}
	}

	void FFQuantityBar::RecalculateColor( int colorMode, Color &color, Color &colorCustom )
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

	void FFQuantityBar::CalculateTextAlignmentOffset( int &outX, int &outY, int &iWide, int &iTall, int iAlignH, int iAlignV, HFont hfFont, wchar_t* wszString )
	{
		surface()->GetTextSize(hfFont, wszString, iWide, iTall);

		switch(iAlignH)
		{
		case ALIGN_CENTER:
			outX = (m_iBarWidth - iWide)/2;
			break;
		case ALIGN_RIGHT:
			outX = m_iBarWidth;
			break;
		case ALIGN_LEFT:
		default:
			outX = - iWide;
		}
		switch(iAlignV)
		{
		case ALIGN_MIDDLE:
			outY = (m_iBarHeight - iTall) / 2;
			break;
		case ALIGN_BOTTOM:
			outY = m_iBarHeight;
			break;
		case ALIGN_TOP:
		default:
			outY = - iTall;
		}
	}
}