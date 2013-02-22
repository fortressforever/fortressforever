//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
//
// ff_hud_quantityitem.cpp
//
// implementation of FFQuantityItem class
//

#include "cbase.h"
#include "ff_quantitybar.h"

#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


namespace vgui
{
	FFQuantityItem::FFQuantityItem(Panel *parent, const char *pElementName) : BaseClass(parent, pElementName)
	{
		//initialise the scale to a 640 * 480 resolution
		m_iWidth = 0;
		m_iHeight = 0;

		m_flScale = 1.0f;
		m_flScaleX = 1.0f;
		m_flScaleY = 1.0f;
		
		m_bDisabled = false;
		
		m_bItemDimentionsChanged = false;

		m_iBarWidth = 60; 
		m_iBarHeight = 10;
		m_iBarBorderWidth = 1;
		m_iBarOrientation = ORIENTATION_HORIZONTAL;
			
		m_iPositionOffsetTop = 0;
		m_iPositionOffsetLeft = 0;
		//-1 means that when painting the internal offset is used instead of these overrides
		m_iPaintOffsetOverrideX = -1;
		m_iPaintOffsetOverrideY = -1;
		m_iPaintOffsetX = 0;
		m_iPaintOffsetY = 0;

		m_iIntensityRed = 20;
		m_iIntensityOrange = 50;
		m_iIntensityYellow = 80;
		m_iIntensityGreen = 100;
		m_bIntensityInvertScale = false;
		m_bIntensityAmountScaled = false;
		m_bIntensityValuesFixed = false;

		m_iIconAnchorPosition = ANCHORPOS_MIDDLELEFT;
		m_iLabelAnchorPosition = ANCHORPOS_TOPLEFT;
		m_iAmountAnchorPosition = ANCHORPOS_TOPRIGHT;
		
		m_iIconAlignHoriz = ALIGN_RIGHT;
		m_iLabelAlignHoriz = ALIGN_LEFT;
		m_iAmountAlignHoriz = ALIGN_RIGHT;

		m_iIconAlignVert = ALIGN_MIDDLE;
		m_iLabelAlignVert = ALIGN_BOTTOM;
		m_iAmountAlignVert = ALIGN_BOTTOM;

		m_iIconPositionOffsetX = 5;
		m_iIconPositionOffsetY = 0;
		m_iLabelPositionOffsetX = -5;
		m_iLabelPositionOffsetY = 0;
		m_iAmountPositionOffsetX = 0;
		m_iAmountPositionOffsetY = 0;
		
		m_iIconPosX = 0;
		m_iIconPosY = 0;
		m_iIconWidth = 0;
		m_iIconHeight = 0;
		m_iLabelPosX = 0;
		m_iLabelPosY = 0;
		m_iLabelWidth = 0;
		m_iLabelHeight = 0;
		m_iAmountPosX = 0;
		m_iAmountPosY = 0;
		m_iAmountWidth = 0;
		m_iAmountHeight = 0;
		m_iAmountMaxPosX = 0;
		m_iAmountMaxPosY = 0;
		m_iAmountMaxWidth = 0;
		m_iAmountMaxHeight = 0;

		m_iSizeIcon = 2;
		m_iSizeLabel = 2;
		m_iSizeAmount = 2;

		m_bLabelFontTahoma = false;
		m_bAmountFontTahoma = false;

		m_bIconFontShadow = false;
		m_bLabelFontShadow = false;
		m_bAmountFontShadow = false;
		
		m_clrBarCustomColor = Color(255,255,255,255);
		m_clrBarBorderCustomColor = Color(255,255,255,255);
		m_clrBarBackgroundCustomColor = Color(192,192,192,80);
		m_clrIconCustomColor = Color(255,255,255,255);
		m_clrLabelCustomColor = Color(255,255,255,255);
		m_clrAmountCustomColor = Color(255,255,255,255);
		
		m_iBarColorMode = ITEM_COLOR_MODE_STEPPED;
		m_iBarBorderColorMode = ITEM_COLOR_MODE_CUSTOM;
		m_iBarBackgroundColorMode = ITEM_COLOR_MODE_STEPPED;
		m_iIconColorMode = ITEM_COLOR_MODE_CUSTOM;
		m_iLabelColorMode = ITEM_COLOR_MODE_CUSTOM;
		m_iAmountColorMode = ITEM_COLOR_MODE_CUSTOM;

		m_ColorIcon = m_clrIconCustomColor;
		m_ColorLabel = m_clrLabelCustomColor;
		m_ColorAmount = m_clrAmountCustomColor;
		m_ColorBar = m_clrBarCustomColor;
		m_ColorBarBorder = m_clrBarBorderCustomColor;
		m_ColorBarBackground = m_clrBarBackgroundCustomColor;
	
		m_iMaxAmount = 100; 

		char szAmountMax[10];
		Q_snprintf( szAmountMax, 5, "%i%", m_iMaxAmount );
		localize()->ConvertANSIToUnicode( szAmountMax, m_wszAmountMax, sizeof( m_wszAmountMax ) );

		m_iAmount = 0;

		char szAmount[10];
		Q_snprintf( szAmount, 5, "%i%", m_iAmount );
		localize()->ConvertANSIToUnicode( szAmount, m_wszAmount, sizeof( m_wszAmount ) );

		localize()->ConvertANSIToUnicode( "", m_wszIcon, sizeof( m_wszIcon ) );
		localize()->ConvertANSIToUnicode( "", m_wszLabel, sizeof( m_wszLabel ) );

		m_bShowBar = true;
		m_bShowBarBorder = true;
		m_bShowBarBackground = true;
		m_bShowIcon = true;
		m_bShowLabel = true;
		m_bShowAmount = true;
		m_bShowAmountMax = true;

		m_bIconFontGlyph = false;

		for (int i = 0; i < QUANTITYBARFONTSIZES * 3; i++)
		{
			m_hfQuantityItemText[i]			= NULL;
			m_hfQuantityItemTahomaText[i]	= NULL;
			m_hfQuantityItemIcon[i]			= NULL;
			m_hfQuantityItemGlyph[i]			= NULL;
		}

		//do this just in case.
		RecalculateAmountMaxPosition();

		ivgui()->AddTickSignal(GetVPanel(), 250); //only update 4 times a second
	}

	void FFQuantityItem::ApplySchemeSettings( IScheme *pScheme )
	{
		HScheme QuantityItemScheme = scheme()->LoadSchemeFromFile("resource/QuantityPanelScheme.res", "QuantityPanelScheme");
		IScheme *qbScheme = scheme()->GetIScheme(QuantityItemScheme);
				
		//the non scaled fonts are used for positioning calculations
		//for 'simplicity' and consistency everything is calculated as if in 640x480
		//decided I'll make the text size variable too
		//size*3 then offset for shadow and non proportional
		for(int i = 0; i < QUANTITYBARFONTSIZES; ++i)
		{
			m_hfQuantityItemText[i*3] = qbScheme->GetFont( VarArgs("QuantityItem%d",i), true );
			m_hfQuantityItemText[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityItemShadow%d",i), true );
			m_hfQuantityItemText[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityItem%d",i), false );
			m_hfQuantityItemTahomaText[i*3] = qbScheme->GetFont( VarArgs("QuantityItemTahoma%d",i), true );
			m_hfQuantityItemTahomaText[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityItemTahomaShadow%d",i), true );
			m_hfQuantityItemTahomaText[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityItemTahoma%d",i), false );
		}	
		for(int i = 0; i < QUANTITYBARICONSIZES; ++i)
		{
			m_hfQuantityItemIcon[i*3] = qbScheme->GetFont( VarArgs("QuantityItemIcon%d",i), true );
			m_hfQuantityItemIcon[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityItemIconShadow%d",i), true );
			m_hfQuantityItemIcon[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityItemIcon%d",i), false );
			m_hfQuantityItemGlyph[i*3] = qbScheme->GetFont( VarArgs("QuantityItemGlyph%d",i), true );
			m_hfQuantityItemGlyph[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityItemGlyphShadow%d",i), true );
			m_hfQuantityItemGlyph[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityItemGlyph%d",i), false );
		}
		SetBorder(pScheme->GetBorder("ScoreBoardItemBorder"));

		SetPaintBackgroundEnabled(true);
		SetPaintBorderEnabled(false);

		//now we've got settings lets update everything
		RecalculateIconPosition();
		RecalculateLabelPosition();
		RecalculateAmountMaxPosition();
		RecalculateQuantity();
		//RecalculateAmountPosition(); <--gets called in Recalculate Quantity

 		BaseClass::ApplySchemeSettings( pScheme );
	}

	void FFQuantityItem::OnTick( )
	{
		VPROF_BUDGET("FFQuantityItem::OnTick","QuantityItems");
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
			m_flScale = ((m_flScaleX <= m_flScaleY) ? m_flScaleX : m_flScaleY);

			RecalculateIconPosition(false);
			RecalculateLabelPosition(false);
			RecalculateAmountPosition();
			RecalculateAmountMaxPosition(false);
			RecalculatePaintOffset();

			SetItemDimentionsChanged(true);
		}
	}

	void FFQuantityItem::Paint( )
	{
		VPROF_BUDGET("FFQuantityItem::Paint","QuantityItems");

		int iOffsetX;
		int iOffsetY;

		if(m_iPaintOffsetOverrideX != -1)
		{
			iOffsetX = m_iPaintOffsetOverrideX;
		}
		else
		{
			iOffsetX = m_iPaintOffsetX * m_flScale;
		}

		if(m_iPaintOffsetOverrideX != -1)
		{
			iOffsetY = m_iPaintOffsetOverrideY;	
		}
		else
		{
			iOffsetY = m_iPaintOffsetY * m_flScale;
		}

		// Set text position based on alignment & text
		if(m_bShowBarBorder)
		{
			surface()->DrawSetColor( m_ColorBarBorder );
			for( int i = 1; i <= (m_iBarBorderWidth * m_flScale); ++i )
			{
				surface()->DrawOutlinedRect( 
					iOffsetX - i, 
					iOffsetY - i, 
					iOffsetX + m_iBarWidth * m_flScale + i, 
					iOffsetY + m_iBarHeight * m_flScale + i
					);
			}
		}

		if(m_bShowBarBackground)
		{
			surface()->DrawSetColor( m_ColorBarBackground );
			surface()->DrawFilledRect( 
				iOffsetX, 
				iOffsetY, 
				iOffsetX + m_iBarWidth * m_flScale, 
				iOffsetY + m_iBarHeight * m_flScale
				);
		}

		if(m_bShowBar)
		{
			surface()->DrawSetColor( m_ColorBar );
			surface()->DrawFilledRect( 
				iOffsetX + m_iBarX0QuantityOffset * m_flScale,
				iOffsetY + m_iBarY0QuantityOffset * m_flScale,
				iOffsetX + ( m_iBarWidth + m_iBarX1QuantityOffset ) * m_flScale,
				iOffsetY + ( m_iBarHeight + m_iBarY1QuantityOffset ) * m_flScale
				);
		}

		if(m_bShowIcon && m_wszIcon)
		{	
			if(m_bIconFontGlyph)
			{
				surface()->DrawSetTextFont(m_hfQuantityItemGlyph[m_iSizeIcon * 3 + (m_bIconFontShadow ? 1 : 0)]);
			}
			else
			{
				surface()->DrawSetTextFont(m_hfQuantityItemIcon[m_iSizeIcon * 3 + (m_bIconFontShadow ? 1 : 0)]);
			}

			surface()->DrawSetTextColor( m_ColorIcon );
			surface()->DrawSetTextPos(iOffsetX + m_iIconPosX, iOffsetY + m_iIconPosY);
			surface()->DrawUnicodeString( m_wszIcon );
		}

		if(m_bShowLabel && m_wszLabel)
		{	
			if(m_bLabelFontTahoma)
			{
				surface()->DrawSetTextFont(m_hfQuantityItemTahomaText[m_iSizeLabel * 3 + (m_bLabelFontShadow ? 1 : 0)]);
			}
			else
			{
				surface()->DrawSetTextFont(m_hfQuantityItemText[m_iSizeLabel * 3 + (m_bLabelFontShadow ? 1 : 0)]);
			}

			surface()->DrawSetTextColor( m_ColorLabel );
			surface()->DrawSetTextPos(iOffsetX + m_iLabelPosX, iOffsetY + m_iLabelPosY);
			surface()->DrawUnicodeString( m_wszLabel );
		}

		if(m_bShowAmount && m_wszAmount)
		{
			if(m_bAmountFontTahoma)
			{
				surface()->DrawSetTextFont(m_hfQuantityItemTahomaText[m_iSizeAmount * 3 + (m_bAmountFontShadow ? 1 : 0)]);
			}
			else
			{
				surface()->DrawSetTextFont(m_hfQuantityItemText[m_iSizeAmount * 3 + (m_bAmountFontShadow ? 1 : 0)]);
			}

			surface()->DrawSetTextColor( m_ColorAmount );
			surface()->DrawSetTextPos(iOffsetX + m_iAmountPosX, iOffsetY + m_iAmountPosY);
			surface()->DrawUnicodeString( m_wszAmountString );
		}

		BaseClass::Paint();
	}

	void FFQuantityItem::SetDisabled(bool bState)
	{
		m_bDisabled = bState;
		SetVisible(!bState);
	}
	
	bool FFQuantityItem::IsDisabled( )
	{
		return m_bDisabled;
	}
	
	void FFQuantityItem::SetAmountFontShadow( bool bHasShadow )
	{
		//TODO: Find out... do the font dimentions need to be altered when we add a shadow?
		m_bAmountFontShadow = bHasShadow;
	}
	void FFQuantityItem::SetIconFontShadow( bool bHasShadow )
	{
		//TODO: Find out... do the font dimentions need to be altered when we add a shadow? Is there any point?
		m_bIconFontShadow = bHasShadow;
	}
	void FFQuantityItem::SetLabelFontShadow( bool bHasShadow )
	{
		//TODO: Find out... do the font dimentions need to be altered when we add a shadow? Is there any point?
		m_bLabelFontShadow = bHasShadow;
	}
	void FFQuantityItem::SetIconFontGlyph( bool bIconIsGlyph, bool bRecalculatePaintOffset  )
	{
		m_bIconFontGlyph = bIconIsGlyph;
		RecalculateIconPosition( bRecalculatePaintOffset );
	}
	void FFQuantityItem::SetLabelFontTahoma( bool bLabelFontTahoma, bool bRecalculatePaintOffset  )
	{
		if(m_bLabelFontTahoma != bLabelFontTahoma)
		{
			m_bLabelFontTahoma = bLabelFontTahoma;
			RecalculateLabelPosition( bRecalculatePaintOffset );
		}
	}
	void FFQuantityItem::SetAmountFontTahoma( bool bAmountFontTahoma, bool bRecalculatePaintOffset  )
	{
		m_bAmountFontTahoma = bAmountFontTahoma;
		RecalculateAmountPosition();
		RecalculateAmountMaxPosition( bRecalculatePaintOffset );
	}


	void FFQuantityItem::SetIconSize( int newIconSize, bool bRecalculatePaintOffset  )
	{
		m_iSizeIcon = newIconSize;
		RecalculateIconPosition( bRecalculatePaintOffset );
	}
	void FFQuantityItem::SetLabelSize( int newLabelSize, bool bRecalculatePaintOffset  )
	{
		m_iSizeLabel = newLabelSize;
		RecalculateLabelPosition( bRecalculatePaintOffset );
	}
	void FFQuantityItem::SetAmountSize( int newAmountSize, bool bRecalculatePaintOffset  )
	{
		m_iSizeAmount = newAmountSize;
		RecalculateAmountPosition();
		RecalculateAmountMaxPosition( bRecalculatePaintOffset );
	}

	void FFQuantityItem::SetAmount( int iAmount )
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
	void FFQuantityItem::SetAmountMax( int iAmountMax, bool bRecalculatePaintOffset  )
	{
		if(m_iMaxAmount != iAmountMax && iAmountMax != 0) // don't allow 0 cause that doesn't make sense - plus it'll crash with divide by 0 when calculating the colour!
		{
			m_iMaxAmount = iAmountMax; 
		
			char szAmountMax[10];
			Q_snprintf( szAmountMax, 5, "%i%", iAmountMax );
			localize()->ConvertANSIToUnicode( szAmountMax, m_wszAmountMax, sizeof( m_wszAmountMax ) );
			RecalculateQuantity();
			RecalculateAmountMaxPosition( bRecalculatePaintOffset );
		}
	}

	void FFQuantityItem::SetIconChar( char *newIconChar, bool bRecalculatePaintOffset  )
	{ 					
		char szIcon[5];
		Q_snprintf( szIcon, 2, "%s%", newIconChar );
		localize()->ConvertANSIToUnicode( szIcon, m_wszIcon, sizeof( m_wszIcon ) );
		
		RecalculateIconPosition( bRecalculatePaintOffset );
	}
	void FFQuantityItem::SetLabelText( char *newLabelText, bool bRecalculatePaintOffset  )
	{
		wchar_t *pszTemp = localize()->Find( newLabelText );

		if( pszTemp )
		{
			wcscpy( m_wszLabel, pszTemp );
		}
		else
		{
			char szLabel[32];
			Q_snprintf( szLabel, 32, "%s%", newLabelText );
			localize()->ConvertANSIToUnicode( szLabel, m_wszLabel, sizeof( m_wszLabel ) );
		}
		RecalculateLabelPosition( bRecalculatePaintOffset );
	}
	void FFQuantityItem::SetLabelText( wchar_t *newLabelText, bool bRecalculatePaintOffset  )
	{
		wcscpy( m_wszLabel, newLabelText );
		RecalculateLabelPosition( bRecalculatePaintOffset );
	}

	void FFQuantityItem::SetBarWidth( int iBarWidth, bool bRecalculatePaintOffset  ) 
	{ 
		m_iBarWidth = iBarWidth;

		RecalculateIconPosition( bRecalculatePaintOffset );
		RecalculateLabelPosition( bRecalculatePaintOffset );
		RecalculateAmountPosition();
		RecalculateAmountMaxPosition( bRecalculatePaintOffset );
	}
	void FFQuantityItem::SetBarHeight( int iBarHeight, bool bRecalculatePaintOffset  ) 
	{ 
		m_iBarHeight = iBarHeight; 

		RecalculateIconPosition( bRecalculatePaintOffset );
		RecalculateLabelPosition( bRecalculatePaintOffset );
		RecalculateAmountPosition();
		RecalculateAmountMaxPosition( bRecalculatePaintOffset );

	}
	void FFQuantityItem::SetBarSize( int iBarWidth, int iBarHeight, bool bRecalculatePaintOffset  ) 
	{ 
		m_iBarWidth = iBarWidth; 
		m_iBarHeight = iBarHeight;

		RecalculateIconPosition( bRecalculatePaintOffset );
		RecalculateLabelPosition( bRecalculatePaintOffset );
		RecalculateAmountPosition();
		RecalculateAmountMaxPosition( bRecalculatePaintOffset );
	}

	void FFQuantityItem::SetBarBorderWidth( int iBarBorderWidth, bool bRecalculatePaintOffset  ) { 
		m_iBarBorderWidth = iBarBorderWidth; 

		if( bRecalculatePaintOffset )
		{
			RecalculatePaintOffset();
		}
	}
	
	void FFQuantityItem::SetBarOrientation( int iOrientation ) { 
		m_iBarOrientation = iOrientation; 
		RecalculateQuantity();
	}

	void FFQuantityItem::ShowBar( bool bShowBar, bool bRecalculatePaintOffset  ) 
	{ 
		if(m_bShowBar != bShowBar)
		{
			m_bShowBar = bShowBar;
			if( bRecalculatePaintOffset )
			{
				RecalculatePaintOffset();
			}
		}
	}
	void FFQuantityItem::ShowBarBackground( bool bShowBarBackground, bool bRecalculatePaintOffset  ) 
	{ 
		if(m_bShowBarBackground != bShowBarBackground)
		{
			m_bShowBarBackground = bShowBarBackground; 
			if( bRecalculatePaintOffset )
			{
				RecalculatePaintOffset();
			}
		}
	}
	void FFQuantityItem::ShowBarBorder( bool bShowBarBorder, bool bRecalculatePaintOffset  ) 
	{ 
		if(m_bShowBarBorder != bShowBarBorder)
		{
			m_bShowBarBorder = bShowBarBorder; 
			if( bRecalculatePaintOffset )
			{
				RecalculatePaintOffset();
			}
		}
	}
	void FFQuantityItem::ShowIcon( bool bShowIcon, bool bRecalculatePaintOffset  ) 
	{ 
		if(m_bShowIcon != bShowIcon)
		{
			m_bShowIcon = bShowIcon; 
			if( bRecalculatePaintOffset )
			{
				RecalculatePaintOffset();
			}
		}
	}
	void FFQuantityItem::ShowLabel( bool bShowLabel, bool bRecalculatePaintOffset  ) 
	{ 
		if(m_bShowLabel != bShowLabel)
		{
			m_bShowLabel = bShowLabel; 
			if( bRecalculatePaintOffset )
			{
				RecalculatePaintOffset();
			}
		}
	}
	void FFQuantityItem::ShowAmount( bool bShowAmount, bool bRecalculatePaintOffset  )
	{ 
		if(m_bShowAmount != bShowAmount)
		{
			m_bShowAmount = bShowAmount; 
			if( bRecalculatePaintOffset )
			{
				RecalculatePaintOffset();
			}
		}
	}
	void FFQuantityItem::ShowAmountMax( bool bShowAmountMax, bool bRecalculatePaintOffset  ) 
	{ 
		if(m_bShowAmountMax != bShowAmountMax)
		{
			m_bShowAmountMax = bShowAmountMax; 
			RecalculateAmountMaxPosition( bRecalculatePaintOffset );
		}
	}

	void FFQuantityItem::SetBarColor( Color newBarColor ) { 
		m_clrBarCustomColor = newBarColor;
		RecalculateColor(m_iBarColorMode, m_ColorBar, m_clrBarCustomColor);
	}

	void FFQuantityItem::SetBarBorderColor( Color newBarBorderColor ) 
	{ 
		m_clrBarBorderCustomColor = newBarBorderColor; 
		RecalculateColor(m_iBarBorderColorMode, m_ColorBarBorder, m_clrBarBorderCustomColor);
	}
	void FFQuantityItem::SetBarBackgroundColor( Color newBarBackgroundColor ) 
	{ 
		m_clrBarBackgroundCustomColor = newBarBackgroundColor; 
		RecalculateColor(m_iBarBackgroundColorMode, m_ColorBarBackground, m_clrBarBackgroundCustomColor);
	}
	void FFQuantityItem::SetAmountColor( Color newAmountColor ) 
	{ 
		m_clrAmountCustomColor = newAmountColor; 
		RecalculateColor(m_iAmountColorMode, m_ColorAmount, m_clrAmountCustomColor);
	}
	void FFQuantityItem::SetIconColor( Color newIconColor ) 
	{ 
		m_clrIconCustomColor = newIconColor; 
		RecalculateColor(m_iIconColorMode, m_ColorIcon, m_clrIconCustomColor);
	}
	void FFQuantityItem::SetLabelColor( Color newLabelColor ) 
	{ 
		m_clrLabelCustomColor = newLabelColor; 
		RecalculateColor(m_iLabelColorMode, m_ColorLabel, m_clrLabelCustomColor);
	}
	void FFQuantityItem::SetTeamColor( Color newTeamColor ) 
	{ 
		if(m_ColorTeam != newTeamColor)
		{
			m_ColorTeam = newTeamColor; 
			if(m_iBarColorMode == ITEM_COLOR_MODE_TEAMCOLORED)
			{
				RecalculateColor(m_iBarColorMode, m_ColorBar, m_clrBarCustomColor);
			}
			if(m_iBarBorderColorMode == ITEM_COLOR_MODE_TEAMCOLORED)
			{
				RecalculateColor(m_iBarBorderColorMode, m_ColorBarBorder, m_clrBarBorderCustomColor);
			}
			if(m_iBarBackgroundColorMode == ITEM_COLOR_MODE_TEAMCOLORED)
			{
				RecalculateColor(m_iBarBackgroundColorMode, m_ColorBarBackground, m_clrBarBackgroundCustomColor);
			}
			if(m_iIconColorMode == ITEM_COLOR_MODE_TEAMCOLORED)
			{
				RecalculateColor(m_iIconColorMode, m_ColorIcon, m_clrIconCustomColor);
			}
			if(m_iLabelColorMode == ITEM_COLOR_MODE_TEAMCOLORED)
			{
				RecalculateColor(m_iLabelColorMode, m_ColorLabel, m_clrLabelCustomColor);
			}
			if(m_iAmountColorMode == ITEM_COLOR_MODE_TEAMCOLORED)
			{
				RecalculateColor(m_iAmountColorMode, m_ColorAmount, m_clrAmountCustomColor);
			}
		}
	}

	void FFQuantityItem::SetBarColorMode( int iColorModeBar ) 
	{ 
		m_iBarColorMode = iColorModeBar; 
		RecalculateColor(m_iBarColorMode, m_ColorBar, m_clrBarCustomColor);
	}
	void FFQuantityItem::SetBarBorderColorMode( int iColorModeBarBorder ) 
	{
		m_iBarBorderColorMode = iColorModeBarBorder;
		RecalculateColor(m_iBarBorderColorMode, m_ColorBarBorder, m_clrBarBorderCustomColor);
	}
	void FFQuantityItem::SetBarBackgroundColorMode( int iColorModeBarBackround ) 
	{
		m_iBarBackgroundColorMode = iColorModeBarBackround; 
		RecalculateColor(m_iBarBackgroundColorMode, m_ColorBarBackground, m_clrBarBackgroundCustomColor);
	}
	void FFQuantityItem::SetAmountColorMode( int iColorModeAmount ) 
	{
 
		m_iAmountColorMode = iColorModeAmount; 
		RecalculateColor(m_iAmountColorMode, m_ColorAmount, m_clrAmountCustomColor);
	}
	void FFQuantityItem::SetIconColorMode( int iColorModeIcon ) 
	{ 
		m_iIconColorMode =  iColorModeIcon;
		RecalculateColor(m_iIconColorMode, m_ColorIcon, m_clrIconCustomColor);
		
	}
	void FFQuantityItem::SetLabelColorMode( int iColorModeLabel ) 
	{ 
		m_iLabelColorMode = iColorModeLabel; 
		RecalculateColor(m_iLabelColorMode, m_ColorLabel, m_clrLabelCustomColor);
	}
		
	void FFQuantityItem::SetAmountPositionOffsetX( int iOffsetXAmount, bool bRecalculatePaintOffset ) 
	{ 	
		if(m_iAmountPositionOffsetX != iOffsetXAmount)
		{
			m_iAmountPositionOffsetX = iOffsetXAmount; 
			RecalculateAmountPosition(); 
			RecalculateAmountMaxPosition( bRecalculatePaintOffset ); 
		}
	}
	void FFQuantityItem::SetAmountPositionOffsetY( int iOffsetYAmount, bool bRecalculatePaintOffset ) 
	{ 
		if(m_iAmountPositionOffsetY != iOffsetYAmount)
		{
			m_iAmountPositionOffsetY = iOffsetYAmount; 
			RecalculateAmountPosition(); 
			RecalculateAmountMaxPosition( bRecalculatePaintOffset ); 
		}
	}
	void FFQuantityItem::SetAmountPositionOffset( int iOffsetXAmount, int iOffsetYAmount, bool bRecalculatePaintOffset ) 
	{ 
		if(m_iAmountPositionOffsetY != iOffsetYAmount || m_iAmountPositionOffsetX != iOffsetXAmount)
		{
			m_iAmountPositionOffsetX = iOffsetXAmount; 
			m_iAmountPositionOffsetY = iOffsetYAmount; 
			RecalculateAmountPosition(); 
			RecalculateAmountMaxPosition( bRecalculatePaintOffset ); 
		}
	}

	void FFQuantityItem::SetIconPositionOffsetX( int iOffsetXIcon, bool bRecalculatePaintOffset ) 
	{ 
		if(m_iIconPositionOffsetX != iOffsetXIcon)
		{
			m_iIconPositionOffsetX = iOffsetXIcon; 
			RecalculateIconPosition( bRecalculatePaintOffset ); 
		}
	}
	void FFQuantityItem::SetIconPositionOffsetY( int iOffsetYIcon, bool bRecalculatePaintOffset ) 
	{ 
		if(m_iIconPositionOffsetY != iOffsetYIcon)
		{
			m_iIconPositionOffsetY = iOffsetYIcon; 
			RecalculateIconPosition( bRecalculatePaintOffset ); 
		}
	}
	void FFQuantityItem::SetIconPositionOffset( int iOffsetXIcon, int iOffsetYIcon, bool bRecalculatePaintOffset ) 
	{ 
		if(m_iIconPositionOffsetY != iOffsetYIcon || m_iIconPositionOffsetX != iOffsetXIcon)
		{
			m_iIconPositionOffsetX = iOffsetXIcon; 
			m_iIconPositionOffsetY = iOffsetYIcon;
			RecalculateIconPosition( bRecalculatePaintOffset );
		}
	}

	void FFQuantityItem::SetLabelPositionOffsetX( int iOffsetXLabel, bool bRecalculatePaintOffset ) 
	{ 
		if(m_iLabelPositionOffsetX != iOffsetXLabel)
		{
			m_iLabelPositionOffsetX = iOffsetXLabel; 
			RecalculateLabelPosition( bRecalculatePaintOffset ); 
		}
	}
	void FFQuantityItem::SetLabelPositionOffsetY( int iOffsetYLabel, bool bRecalculatePaintOffset ) 
	{
		if(m_iLabelPositionOffsetY != iOffsetYLabel)
		{
			m_iLabelPositionOffsetY = iOffsetYLabel; 
			RecalculateLabelPosition( bRecalculatePaintOffset );
		}
	}
	void FFQuantityItem::SetLabelPositionOffset( int iOffsetXLabel, int iOffsetYLabel, bool bRecalculatePaintOffset ) 
	{ 
		if(m_iLabelPositionOffsetY != iOffsetYLabel || m_iLabelPositionOffsetX != iOffsetXLabel)
		{
			m_iLabelPositionOffsetX = iOffsetXLabel; 
			m_iLabelPositionOffsetY = iOffsetYLabel; 
			RecalculateLabelPosition( bRecalculatePaintOffset ); 
		}
	}
	
	int FFQuantityItem::GetAmount( ) { return m_iAmount; }

	int FFQuantityItem::GetMaxAmount( ) { return m_iMaxAmount; }
		
	void FFQuantityItem::SetAmountAnchorPosition( int iPosAmount, bool bRecalculatePaintOffset )
	{
		m_iAmountAnchorPosition = iPosAmount; 
		RecalculateAmountPosition(); 
		RecalculateAmountMaxPosition( bRecalculatePaintOffset ); 
	}
	void FFQuantityItem::SetIconAnchorPosition( int iPosIcon, bool bRecalculatePaintOffset )
	{
		m_iIconAnchorPosition = iPosIcon; 
		RecalculateIconPosition( bRecalculatePaintOffset ); 
	}
	void FFQuantityItem::SetLabelAnchorPosition( int iPosLabel, bool bRecalculatePaintOffset )
	{
		m_iLabelAnchorPosition = iPosLabel; 
		RecalculateLabelPosition( bRecalculatePaintOffset ); 
	}
			
	void FFQuantityItem::SetAmountAlignmentHorizontal( int iAlignHorizAmount, bool bRecalculatePaintOffset )
	{
		m_iAmountAlignHoriz = iAlignHorizAmount; 
		RecalculateAmountPosition(); 
		RecalculateAmountMaxPosition( bRecalculatePaintOffset ); 
	}
	void FFQuantityItem::SetIconAlignmentHorizontal( int iIconAlignHoriz, bool bRecalculatePaintOffset ) 
	{ 
		m_iIconAlignHoriz = iIconAlignHoriz; 
		RecalculateIconPosition( bRecalculatePaintOffset ); 
	}
	void FFQuantityItem::SetLabelAlignmentHorizontal( int iAlignHorizLabel, bool bRecalculatePaintOffset ) { 
		m_iLabelAlignHoriz = iAlignHorizLabel; 
		RecalculateLabelPosition( bRecalculatePaintOffset ); 
	}
	
	void FFQuantityItem::SetAmountAlignmentVertical( int iAmountAlignVert, bool bRecalculatePaintOffset ) 
	{
		m_iAmountAlignVert = iAmountAlignVert; 
		RecalculateAmountPosition(); 
		RecalculateAmountMaxPosition( bRecalculatePaintOffset ); 
	}
	void FFQuantityItem::SetIconAlignmentVertical( int iIconAlignVert, bool bRecalculatePaintOffset ) 
	{ 
		m_iIconAlignVert = iIconAlignVert; 
		RecalculateIconPosition( bRecalculatePaintOffset ); 
	}

	void FFQuantityItem::SetLabelAlignmentVertical( int iLabelAlignVert, bool bRecalculatePaintOffset ) 
	{
		m_iLabelAlignVert = iLabelAlignVert; 
		RecalculateLabelPosition( bRecalculatePaintOffset ); 
	}
		
	void FFQuantityItem::SetIntensityControl( int iRed, int iOrange,int iYellow, int iGreen, bool bInvertScale )
	{
		m_iIntensityRed = iRed;
		m_iIntensityOrange = iOrange;
		m_iIntensityYellow = iYellow;
		m_iIntensityGreen = iGreen;
		m_bIntensityInvertScale = bInvertScale;
	}

	void FFQuantityItem::SetIntensityAmountScaled( bool bAmountScaled )
	{
		m_bIntensityAmountScaled = bAmountScaled;
	}
	void FFQuantityItem::SetIntensityValuesFixed( bool bvaluesFixed )
	{
		m_bIntensityValuesFixed = bvaluesFixed;
	}
	bool FFQuantityItem::IsIntensityValuesFixed( )
	{
		return m_bIntensityValuesFixed;
	}


	bool FFQuantityItem::GetItemDimentionsChanged()
	{
		return m_bItemDimentionsChanged;
	}
	void FFQuantityItem::SetItemDimentionsChanged( bool bItemDimentionsChanged )
	{
		//if ChildDimentionsChanged boolean is changing
		if(m_bItemDimentionsChanged != bItemDimentionsChanged) 
		{
			//send update to parent for positioning	
			if( bItemDimentionsChanged && GetVParent())
			{
				KeyValues *msg = new KeyValues("ItemDimentionsChanged");
				PostMessage(GetVParent(), msg);
			}
			m_bItemDimentionsChanged = bItemDimentionsChanged;
		}		
	}

	void FFQuantityItem::GetPanelPositioningData( int& iWidth, int& iHeight, int& iOffsetX, int&  iOffsetY )
	{
		iWidth = m_iWidth;
		iHeight = m_iHeight;
		iOffsetX = m_iPaintOffsetX;
		iOffsetY = m_iPaintOffsetY;
	}
	
	void FFQuantityItem::SetPos( int iLeft, int iTop )
	{
		m_iPositionLeft = iLeft;
		m_iPositionTop = iTop;

		return BaseClass::SetPos(m_iPositionLeft + m_iPositionOffsetLeft, m_iPositionTop + m_iPositionOffsetTop);
	}

	void FFQuantityItem::SetPosOffset( int iLeft, int iTop )
	{
		m_iPositionOffsetLeft = iLeft;
		m_iPositionOffsetTop = iTop;

		return BaseClass::SetPos(m_iPositionLeft + m_iPositionOffsetLeft, m_iPositionTop + m_iPositionOffsetTop);
	}

	void FFQuantityItem::SetPaintOffset( int iLeft, int iTop )
	{
		m_iPaintOffsetOverrideX = iLeft;
		m_iPaintOffsetOverrideY = iTop;
	}

	
	void FFQuantityItem::SetStyle(KeyValues *kvStyleData, KeyValues *kvDefaultStyleData )
	{
		bool bRecalculatePaintOffset = false;

		//-1 default so that we don't change the values if it doesn't exist for any reason
		int iBarWidth = kvStyleData->GetInt("barWidth", kvDefaultStyleData->GetInt("barWidth", -1));
		int iBarHeight = kvStyleData->GetInt("barHeight", kvDefaultStyleData->GetInt("barHeight", -1));
		int iBarBorderWidth = kvStyleData->GetInt("barBorderWidth", kvDefaultStyleData->GetInt("barBorderWidth", -1));
		int iBarOrientation = kvStyleData->GetInt("barOrientation", kvDefaultStyleData->GetInt("barOrientation", -1));

		// I feel better checking things have changed before setting them else we might call lots of unneccesary methods
		if(m_iBarWidth != iBarWidth && iBarWidth != -1)
		{
			SetBarWidth(iBarWidth, false);
			bRecalculatePaintOffset = true;
		}

		if(m_iBarHeight != iBarHeight && iBarHeight != -1)
		{
			SetBarHeight(iBarHeight, false);
			bRecalculatePaintOffset = true;
		}

		if(m_iBarBorderWidth != iBarBorderWidth && iBarBorderWidth != -1)
		{
			SetBarBorderWidth(iBarBorderWidth, false);
			bRecalculatePaintOffset = true;
		}

		if(m_iBarOrientation != iBarOrientation && iBarOrientation != -1)
		{
			SetBarOrientation(iBarOrientation);
		}

		KeyValues *kvComponentStyleData = kvStyleData->FindKey("Bar");
		if(!kvComponentStyleData)
		{
			kvComponentStyleData = kvDefaultStyleData->FindKey("Bar");
		}

		if(kvComponentStyleData)
		{
			int iShow = kvComponentStyleData->GetInt("show", -1);

			int iRed = kvComponentStyleData->GetInt("red", -1);
			int iGreen = kvComponentStyleData->GetInt("green", -1);
			int iBlue = kvComponentStyleData->GetInt("blue", -1);
			int iAlpha = kvComponentStyleData->GetInt("alpha", -1);
			int iColorMode = kvComponentStyleData->GetInt("colorMode", -1);

			if((iShow == 1 ? true : false) != m_bShowBar && iShow != -1)
			{
				ShowBar(!m_bShowBar, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iBarColorMode != iColorMode && iColorMode != -1)
			{
				SetBarColorMode(iColorMode);
			}

			bool bValidColor = false;
			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				bValidColor = true;
			}

			if(m_clrBarCustomColor != color && bValidColor)
			{
				SetBarColor(color);
			}
		}

		kvComponentStyleData = kvStyleData->FindKey("BarBorder");
		if(!kvComponentStyleData)
		{
			kvComponentStyleData = kvDefaultStyleData->FindKey("BarBorder");
		}

		if(kvComponentStyleData)
		{
			int iShow = kvComponentStyleData->GetInt("show", -1);

			int iRed = kvComponentStyleData->GetInt("red", -1);
			int iGreen = kvComponentStyleData->GetInt("green", -1);
			int iBlue = kvComponentStyleData->GetInt("blue", -1);
			int iAlpha = kvComponentStyleData->GetInt("alpha", -1);
			int iColorMode = kvComponentStyleData->GetInt("colorMode", -1);

			if((iShow == 1 ? true : false) != m_bShowBarBorder && iShow != -1)
			{
				ShowBarBorder(!m_bShowBarBorder, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iBarBorderColorMode != iColorMode && iColorMode != -1)
			{
				SetBarBorderColorMode(iColorMode);
			}

			bool bValidColor = false;
			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				bValidColor = true;
			}

			if(m_clrBarBorderCustomColor != color && bValidColor)
			{
				SetBarBorderColor(color);
			}
		}

		kvComponentStyleData = kvStyleData->FindKey("BarBackground");
		if(!kvComponentStyleData)
		{
			kvComponentStyleData = kvDefaultStyleData->FindKey("BarBackground");
		}

		if(kvComponentStyleData)
		{
			int iShow = kvComponentStyleData->GetInt("show", -1);

			int iRed = kvComponentStyleData->GetInt("red", -1);
			int iGreen = kvComponentStyleData->GetInt("green", -1);
			int iBlue = kvComponentStyleData->GetInt("blue", -1);
			int iAlpha = kvComponentStyleData->GetInt("alpha", -1);
			int iColorMode = kvComponentStyleData->GetInt("colorMode", -1);

			if((iShow == 1 ? true : false) != m_bShowBarBackground && iShow != -1)
			{
				ShowBarBackground(!m_bShowBarBackground, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iBarBackgroundColorMode != iColorMode && iColorMode != -1)
			{
				SetBarBackgroundColorMode(iColorMode);
			}

			bool bValidColor = false;
			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				bValidColor = true;
			}

			if(m_clrBarBackgroundCustomColor != color && bValidColor)
			{
				SetBarBackgroundColor(color);
			}
		}

		kvComponentStyleData = kvStyleData->FindKey("Icon");

		if(!kvComponentStyleData)
		{
			kvComponentStyleData = kvDefaultStyleData->FindKey("Icon");
		}

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

			int iAnchorPosition = kvComponentStyleData->GetInt("anchorPosition", -1);
			int iAlignHoriz = kvComponentStyleData->GetInt("alignH", -1);
			int iAlignVert = kvComponentStyleData->GetInt("alignV", -1);

			int iOffsetX = kvComponentStyleData->GetInt("offsetX", -9999);
			int iOffsetY = kvComponentStyleData->GetInt("offsetY", -9999);

			if((iShow == 1 ? true : false) != m_bShowIcon && iShow != -1)
			{
				ShowIcon(!m_bShowIcon, false);
				bRecalculatePaintOffset = true;
			}
			
			if(m_iIconColorMode != iColorMode && iColorMode != -1)
			{
				SetIconColorMode(iColorMode);
			}

			bool bValidColor = false;
			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				bValidColor = true;
			}

			if(m_clrIconCustomColor != color && bValidColor)
			{
				SetIconColor(color);
			}

			if(m_iIconPositionOffsetX != iOffsetX && iOffsetX != -9999)
			{
				SetIconPositionOffsetX(iOffsetX, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iIconPositionOffsetY != iOffsetY && iOffsetY != -9999)
			{
				SetIconPositionOffsetY(iOffsetY, false);
				bRecalculatePaintOffset = true;
			}
			
			if(m_iIconAnchorPosition != iAnchorPosition && iAnchorPosition != -1)
			{
				SetIconAnchorPosition(iAnchorPosition, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iIconAlignHoriz != iAlignHoriz && iAlignHoriz != -1)
			{
				SetIconAlignmentHorizontal(iAlignHoriz, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iIconAlignVert != iAlignVert && iAlignVert != -1)
			{
				SetIconAlignmentVertical(iAlignVert, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iSizeIcon != iSize && iSize != -1)
			{
				SetIconSize(iSize, false);
				bRecalculatePaintOffset = true;
			}

			if(m_bIconFontShadow != (iShadow == 1 ? true : false) && iShadow != -1)
			{
				SetIconFontShadow(!m_bIconFontShadow);
			}
		}

		kvComponentStyleData = kvStyleData->FindKey("Label");

		if(!kvComponentStyleData)
		{
			kvComponentStyleData = kvDefaultStyleData->FindKey("Label");
		}

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

			int iAnchorPosition = kvComponentStyleData->GetInt("anchorPosition", -1);
			int iAlignHoriz = kvComponentStyleData->GetInt("alignH", -1);
			int iAlignVert = kvComponentStyleData->GetInt("alignV", -1);

			int iOffsetX = kvComponentStyleData->GetInt("offsetX", -9999);
			int iOffsetY = kvComponentStyleData->GetInt("offsetY", -9999);

			int iFontTahoma = kvComponentStyleData->GetInt("fontTahoma", -1);

			if((iShow == 1 ? true : false) != m_bShowLabel && iShow != -1)
			{
				ShowLabel(!m_bShowLabel, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iLabelColorMode != iColorMode && iColorMode != -1)
			{
				SetLabelColorMode(iColorMode);
			}

			bool bValidColor = false;
			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				bValidColor = true;
			}

			if(m_clrLabelCustomColor != color && bValidColor)
			{
				SetLabelColor(color);
			}

			if(m_iLabelPositionOffsetX != iOffsetX && iOffsetX != -9999)
			{
				SetLabelPositionOffsetX(iOffsetX, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iLabelPositionOffsetY != iOffsetY && iOffsetY != -9999)
			{
				SetLabelPositionOffsetY(iOffsetY, false);
				bRecalculatePaintOffset = true;
			}
			
			if(m_iLabelAnchorPosition != iAnchorPosition && iAnchorPosition != -1)
			{
				SetLabelAnchorPosition(iAnchorPosition, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iLabelAlignHoriz != iAlignHoriz && iAlignHoriz != -1)
			{
				SetLabelAlignmentHorizontal(iAlignHoriz, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iLabelAlignVert != iAlignVert && iAlignVert != -1)
			{
				SetLabelAlignmentVertical(iAlignVert, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iSizeLabel != iSize && iSize != -1)
			{
				SetLabelSize(iSize, false);
				bRecalculatePaintOffset = true;
			}

			if(m_bLabelFontShadow != (iShadow == 1 ? true : false) && iShadow != -1)
			{
				SetLabelFontShadow(!m_bLabelFontShadow);
			}

			if((iFontTahoma == 1 ? true : false) != m_bLabelFontTahoma && iFontTahoma != -1)
			{
				SetLabelFontTahoma(!m_bLabelFontTahoma, false);
				bRecalculatePaintOffset = true;
			}
		}

		kvComponentStyleData = kvStyleData->FindKey("Amount");
		if(!kvComponentStyleData)
		{
			kvComponentStyleData = kvDefaultStyleData->FindKey("Amount");
		}

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

			int iAnchorPosition = kvComponentStyleData->GetInt("anchorPosition", -1);
			int iAlignHoriz = kvComponentStyleData->GetInt("alignH", -1);
			int iAlignVert = kvComponentStyleData->GetInt("alignV", -1);

			int iOffsetX = kvComponentStyleData->GetInt("offsetX", -9999);
			int iOffsetY = kvComponentStyleData->GetInt("offsetY", -9999);

			int iFontTahoma = kvComponentStyleData->GetInt("fontTahoma", -1);

			if((iShow == 1 ? true : false) != m_bShowAmount && iShow != -1)
			{
				ShowAmount(!m_bShowAmount, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iAmountColorMode != iColorMode && iColorMode != -1)
			{
				SetAmountColorMode(iColorMode);
			}

			bool bValidColor = false;
			Color color = Color(iRed, iGreen, iBlue, iAlpha);
			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				bValidColor = true;
			}

			if(m_clrAmountCustomColor != color && bValidColor)
			{
				SetAmountColor(color);
			}

			if(m_iAmountPositionOffsetX != iOffsetX && iOffsetX != -9999)
			{
				SetAmountPositionOffsetX(iOffsetX, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iAmountPositionOffsetY != iOffsetY && iOffsetY != -9999)
			{
				SetAmountPositionOffsetY(iOffsetY, false);
				bRecalculatePaintOffset = true;
			}
			
			if(m_iAmountAnchorPosition != iAnchorPosition && iAnchorPosition != -1)
			{
				SetAmountAnchorPosition(iAnchorPosition, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iAmountAlignHoriz != iAlignHoriz && iAlignHoriz != -1)
			{
				SetAmountAlignmentHorizontal(iAlignHoriz, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iAmountAlignVert != iAlignVert && iAlignVert != -1)
			{
				SetAmountAlignmentVertical(iAlignVert, false);
				bRecalculatePaintOffset = true;
			}

			if(m_iSizeAmount != iSize && iSize != -1)
			{
				SetAmountSize(iSize, false);
				bRecalculatePaintOffset = true;
			}

			if(m_bAmountFontShadow != (iShadow == 1 ? true : false) && iShadow != -1)
			{
				SetAmountFontShadow(!m_bAmountFontShadow);
			}

			if((iFontTahoma == 1 ? true : false) != m_bAmountFontTahoma && iFontTahoma != -1)
			{
				SetAmountFontTahoma(!m_bAmountFontTahoma, false);
				bRecalculatePaintOffset = true;
			}
		}

		if( bRecalculatePaintOffset )
		{
			RecalculatePaintOffset();
		}
	}
	
	void FFQuantityItem::RecalculatePaintOffset( )
	{
		float flX0 = 0.0f;
		float flY0 = 0.0f;
		float flX1 = 0.0f;
		float flY1 = 0.0f;
		
		if( m_bShowBar || m_bShowBarBackground || m_bShowBarBorder )
		{
			flX1 += m_iBarWidth * m_flScale;
			flY1 += m_iBarHeight * m_flScale;
		}

		if( m_bShowBarBorder )
		{
			float flScaledBarBorderWidth = m_iBarBorderWidth * m_flScale;
			flX1 += flScaledBarBorderWidth;
			flY1 += flScaledBarBorderWidth;
			flX0 -= flScaledBarBorderWidth;
			flY0 -= flScaledBarBorderWidth;
		}

		if( m_bShowIcon && m_iIconPosX < flX0 )
			flX0 = m_iIconPosX;
		if( m_bShowLabel && m_iLabelPosX < flX0 )
			flX0 = m_iLabelPosX;
		if( m_bShowAmount && m_iAmountMaxPosX < flX0 )
			flX0 = m_iAmountMaxPosX;

		if( m_bShowIcon && m_iIconPosY < flY0 )
			flY0 = m_iIconPosY;
		if( m_bShowLabel && m_iLabelPosY < flY0 )
			flY0 = m_iLabelPosY;
		if( m_bShowAmount && m_iAmountMaxPosY < flY0 )
			flY0 = m_iAmountMaxPosY;

		if( m_bShowIcon && (m_iIconPosX + m_iIconWidth) > flX1 )
			flX1 = m_iIconPosX + m_iIconWidth;
		if( m_bShowLabel && (m_iLabelPosX + m_iLabelWidth) > flX1 )
			flX1 = m_iLabelPosX + m_iLabelWidth;
		//we use max amount width/height instead to make sure it doesnt keep moving
		if( m_bShowAmount && (m_iAmountMaxPosX + m_iAmountMaxWidth) > flX1 )
			flX1 = m_iAmountMaxPosX + m_iAmountMaxWidth;

		if( m_bShowIcon && (m_iIconPosY + m_iIconHeight) > flY1 )
			flY1 = m_iIconPosY + m_iIconHeight;
		if( m_bShowLabel && (m_iLabelPosY + m_iLabelHeight) > flY1 )
			flY1 = m_iLabelPosY + m_iLabelHeight;
		//we use max amount width/height instead to make sure it doesnt keep moving
		if( m_bShowAmount && (m_iAmountMaxPosY + m_iAmountMaxHeight) > flY1 )
			flY1 = m_iAmountMaxPosY + m_iAmountMaxHeight;
		
		bool bSetItemDimentionsChanged = false;

		if(m_iPaintOffsetX != (int)(-flX0 + 0.6f) || m_iPaintOffsetY != (int)(-flY0 + 0.6f))
		{
			//+0.6 so it always rounds up not down.
			m_iPaintOffsetX = -flX0 + 0.6f;
			m_iPaintOffsetY = -flY0 + 0.6f;
		}

		if(m_iWidth != (int)(flX1 - flX0 + 0.6f) || m_iHeight != (int)(flY1 - flY0 + 0.6f))
		{
			//+0.6 so it always rounds up not down.
			m_iWidth = flX1 - flX0 + 0.6f;
			m_iHeight = flY1 - flY0 + 0.6f;
			bSetItemDimentionsChanged = true;
		}

		//just so if we ever use the quantity bar on its own and not in a panel it will set it's own size
		if( m_iPaintOffsetOverrideX == -1 || m_iPaintOffsetOverrideY == -1 )
		{
			//TODO: can't remember if there's a good reason SetChildDimentionsChanged is here
			bSetItemDimentionsChanged = true;
			SetSize(m_iWidth, m_iHeight);
		}

		if(bSetItemDimentionsChanged)
		{
			SetItemDimentionsChanged(true);
		}
	}
	void FFQuantityItem::RecalculateIconPosition( bool bRecalculatePaintOffset )
	{
		CalculateTextPositionOffset(m_iIconAnchorPositionX, m_iIconAnchorPositionY, m_iIconAnchorPosition);
		
		if(m_bIconFontGlyph)
		{
			CalculateTextAlignmentOffset(m_iIconAlignmentOffsetX, m_iIconAlignmentOffsetY, m_iIconWidth, m_iIconHeight, m_iIconAlignHoriz, m_iIconAlignVert, m_hfQuantityItemGlyph[m_iSizeIcon*3 + (m_bIconFontShadow ? 1 : 0)], m_wszIcon);
		}
		else
		{
			CalculateTextAlignmentOffset(m_iIconAlignmentOffsetX, m_iIconAlignmentOffsetY, m_iIconWidth, m_iIconHeight, m_iIconAlignHoriz, m_iIconAlignVert, m_hfQuantityItemIcon[m_iSizeIcon*3 + (m_bIconFontShadow ? 1 : 0)], m_wszIcon);
		}
		
		m_iIconPosX = m_iIconAnchorPositionX + m_iIconAlignmentOffsetX + m_iIconPositionOffsetX * m_flScale;
		m_iIconPosY = m_iIconAlignmentOffsetY + m_iIconAnchorPositionY + m_iIconPositionOffsetY * m_flScale;
		
		if( bRecalculatePaintOffset )
		{
			RecalculatePaintOffset();
		}
	}

	void FFQuantityItem::RecalculateLabelPosition( bool bRecalculatePaintOffset  )
	{
		CalculateTextPositionOffset(m_iLabelAnchorPositionX, m_iLabelAnchorPositionY, m_iLabelAnchorPosition);
		
		if(m_bLabelFontTahoma)
		{
			CalculateTextAlignmentOffset(m_iLabelAlignmentOffsetX, m_iLabelAlignmentOffsetY, m_iLabelWidth, m_iLabelHeight, m_iLabelAlignHoriz, m_iLabelAlignVert, m_hfQuantityItemTahomaText[m_iSizeLabel*3 + (m_bLabelFontShadow ? 1 : 0)], m_wszLabel);
		}
		else
		{
			CalculateTextAlignmentOffset(m_iLabelAlignmentOffsetX, m_iLabelAlignmentOffsetY, m_iLabelWidth, m_iLabelHeight, m_iLabelAlignHoriz, m_iLabelAlignVert, m_hfQuantityItemText[m_iSizeLabel*3 + (m_bLabelFontShadow ? 1 : 0)], m_wszLabel);
		}
		
		m_iLabelPosX = m_iLabelAnchorPositionX + m_iLabelAlignmentOffsetX + m_iLabelPositionOffsetX * m_flScale;
		m_iLabelPosY = m_iLabelAnchorPositionY + m_iLabelAlignmentOffsetY + m_iLabelPositionOffsetY * m_flScale;
		
		if( bRecalculatePaintOffset )
		{
			RecalculatePaintOffset();
		}
	}

	void FFQuantityItem::RecalculateAmountPosition( )
	{
		CalculateTextPositionOffset(m_iAmountAnchorPositionX, m_iAmountAnchorPositionY, m_iAmountAnchorPosition);
		
		if(m_bAmountFontTahoma)
		{
			CalculateTextAlignmentOffset(m_iAmountAlignmentOffsetX, m_iAmountAlignmentOffsetY, m_iAmountWidth, m_iAmountHeight, m_iAmountAlignHoriz, m_iAmountAlignVert, m_hfQuantityItemTahomaText[m_iSizeAmount*3 + (m_bAmountFontShadow ? 1 : 0)], m_wszAmountString);
		}
		else
		{
			CalculateTextAlignmentOffset(m_iAmountAlignmentOffsetX, m_iAmountAlignmentOffsetY, m_iAmountWidth, m_iAmountHeight, m_iAmountAlignHoriz, m_iAmountAlignVert, m_hfQuantityItemText[m_iSizeAmount*3 + (m_bAmountFontShadow ? 1 : 0)], m_wszAmountString);
		}

		m_iAmountPosX = m_iAmountAnchorPositionX + m_iAmountAlignmentOffsetX + m_iAmountPositionOffsetX * m_flScale;
		m_iAmountPosY = m_iAmountAnchorPositionY + m_iAmountAlignmentOffsetY + m_iAmountPositionOffsetY * m_flScale;
	}

	void FFQuantityItem::RecalculateAmountMaxPosition( bool bRecalculatePaintOffset  )
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
		
		CalculateTextPositionOffset(m_iAmountMaxAnchorPositionX, m_iAmountMaxAnchorPositionY, m_iAmountAnchorPosition);
		
		if(m_bAmountFontTahoma)
		{
			CalculateTextAlignmentOffset(m_iAmountMaxAlignmentOffsetX, m_iAmountMaxAlignmentOffsetY, m_iAmountMaxWidth, m_iAmountMaxHeight, m_iAmountAlignHoriz, m_iAmountAlignVert, m_hfQuantityItemTahomaText[m_iSizeAmount*3 + (m_bAmountFontShadow ? 1 : 0)], wszMaxAmountString);
		}
		else
		{
			CalculateTextAlignmentOffset(m_iAmountMaxAlignmentOffsetX, m_iAmountMaxAlignmentOffsetY, m_iAmountMaxWidth, m_iAmountMaxHeight, m_iAmountAlignHoriz, m_iAmountAlignVert, m_hfQuantityItemText[m_iSizeAmount*3 + (m_bAmountFontShadow ? 1 : 0)], wszMaxAmountString);
		}

		m_iAmountMaxPosX = m_iAmountMaxAnchorPositionX + m_iAmountMaxAlignmentOffsetX + m_iAmountPositionOffsetX * m_flScale;
		m_iAmountMaxPosY = m_iAmountMaxAnchorPositionY + m_iAmountMaxAlignmentOffsetY + m_iAmountPositionOffsetY * m_flScale;

		if( bRecalculatePaintOffset )
		{
			RecalculatePaintOffset();
		}
	}

	void FFQuantityItem::RecalculateQuantity( )
	//all this regardless of whether each item is actually being shown
	//(incase the user changes options during the game)
	{
		if(m_bIntensityAmountScaled && m_iMaxAmount != 0)
		{
			m_ColorIntensityFaded = GetIntensityColor((int)((float)m_iAmount/(float)m_iMaxAmount * 100), 100, 2, m_clrBarCustomColor.a(), m_iIntensityRed,m_iIntensityOrange,m_iIntensityYellow,m_iIntensityGreen,m_bIntensityInvertScale);
			m_ColorIntensityStepped = GetIntensityColor((int)((float)m_iAmount/(float)m_iMaxAmount * 100), 100, 1, m_clrBarCustomColor.a(), m_iIntensityRed,m_iIntensityOrange,m_iIntensityYellow,m_iIntensityGreen,m_bIntensityInvertScale);
		}
		else
		{
			m_ColorIntensityFaded = GetIntensityColor(m_iAmount, m_iMaxAmount, 2, m_clrBarCustomColor.a(), m_iIntensityRed,m_iIntensityOrange,m_iIntensityYellow,m_iIntensityGreen,m_bIntensityInvertScale);
			m_ColorIntensityStepped = GetIntensityColor(m_iAmount, m_iMaxAmount, 1, m_clrBarCustomColor.a(), m_iIntensityRed,m_iIntensityOrange,m_iIntensityYellow,m_iIntensityGreen,m_bIntensityInvertScale);
		}

		if(!m_bShowAmountMax)
		{
			if(m_iMaxAmount == 100)// already same as a percentage so no need to convert
			{
				_snwprintf( m_wszAmountString, 255, L"%s%%", m_wszAmount );
			}
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

		if(m_iBarBorderColorMode == ITEM_COLOR_MODE_FADED ) 
		{
			m_ColorBarBorder.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_clrBarBorderCustomColor.a());
		}
		else if(m_iBarBorderColorMode == ITEM_COLOR_MODE_STEPPED )
		{
			m_ColorBarBorder.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_clrBarBorderCustomColor.a());
		}

		if(m_iBarBackgroundColorMode == ITEM_COLOR_MODE_FADED ) 
		{
			m_ColorBarBackground.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_clrBarBackgroundCustomColor.a());
		}
		else if(m_iBarBackgroundColorMode == ITEM_COLOR_MODE_STEPPED )
		{
			m_ColorBarBackground.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_clrBarBackgroundCustomColor.a());
		}

		if(m_iBarColorMode == ITEM_COLOR_MODE_FADED ) 
		{
			m_ColorBar.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_clrBarCustomColor.a());
		}
		else if(m_iBarColorMode == ITEM_COLOR_MODE_STEPPED )
		{
			m_ColorBar.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_clrBarCustomColor.a());
		}

		if(m_iIconColorMode == ITEM_COLOR_MODE_FADED ) 
		{
			m_ColorIcon.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_clrIconCustomColor.a());
		}
		else if(m_iIconColorMode == ITEM_COLOR_MODE_STEPPED )
		{
			m_ColorIcon.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_clrIconCustomColor.a());
		}

		if(m_iLabelColorMode == ITEM_COLOR_MODE_FADED ) 
		{
			m_ColorLabel.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),m_clrLabelCustomColor.a());
		}
		else if(m_iLabelColorMode == ITEM_COLOR_MODE_STEPPED )
		{
			m_ColorLabel.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),m_clrLabelCustomColor.a());
		}

		if(m_iAmountColorMode == ITEM_COLOR_MODE_FADED ) 
		{
			m_ColorAmount.SetColor(m_ColorIntensityFaded.r(), m_ColorIntensityFaded.g(), m_ColorIntensityFaded.b(),m_clrAmountCustomColor.a());
		}
		else if(m_iAmountColorMode == ITEM_COLOR_MODE_STEPPED )
		{
			m_ColorAmount.SetColor(m_ColorIntensityStepped.r(), m_ColorIntensityStepped.g(), m_ColorIntensityStepped.b(),m_clrAmountCustomColor.a());
		}

		if(m_iBarOrientation == ORIENTATION_HORIZONTAL_INVERTED)
		{
			m_iBarX0QuantityOffset = m_iBarWidth - (m_iBarWidth * m_iAmount/m_iMaxAmount);
			if( (m_iBarX1QuantityOffset + m_iBarY0QuantityOffset + m_iBarY1QuantityOffset ) != 0 )
			{
				m_iBarX1QuantityOffset = 0;
				m_iBarY0QuantityOffset = 0;
				m_iBarY1QuantityOffset = 0;
			}
		}
		else if(m_iBarOrientation == ORIENTATION_VERTICAL)
		{
			m_iBarY0QuantityOffset = m_iBarHeight - (m_iBarHeight * m_iAmount/m_iMaxAmount);
			if( (m_iBarX0QuantityOffset + m_iBarX1QuantityOffset + m_iBarY1QuantityOffset ) != 0 )
			{
				m_iBarX0QuantityOffset = 0;
				m_iBarX1QuantityOffset = 0;
				m_iBarY1QuantityOffset = 0;
			}
		}
		else if(m_iBarOrientation == ORIENTATION_VERTICAL_INVERTED)
		{
			m_iBarY1QuantityOffset = (m_iBarHeight * m_iAmount/m_iMaxAmount) - m_iBarHeight;
			if( (m_iBarX0QuantityOffset + m_iBarX1QuantityOffset + m_iBarY0QuantityOffset ) != 0 )
			{
				m_iBarX0QuantityOffset = 0;
				m_iBarX1QuantityOffset = 0;
				m_iBarY0QuantityOffset = 0;
			}
		}
		else //m_iBarOrientation == ORIENTATION_HORIZONTAL (or anything else)
		{
			m_iBarX1QuantityOffset = (m_iBarWidth * m_iAmount/m_iMaxAmount) - m_iBarWidth;
			if( (m_iBarX0QuantityOffset + m_iBarY0QuantityOffset + m_iBarY1QuantityOffset ) != 0 )
			{
				m_iBarX0QuantityOffset = 0;
				m_iBarY0QuantityOffset = 0;
				m_iBarY1QuantityOffset = 0;
			}
		}
	}

	void FFQuantityItem::RecalculateColor( int colorMode, Color &color, Color &colorCustom )
	{
		if(colorMode == ITEM_COLOR_MODE_STEPPED )
		{
			color.SetColor(m_ColorIntensityStepped.r(),m_ColorIntensityStepped.g(),m_ColorIntensityStepped.b(),colorCustom.a());
		}
		else if(colorMode == ITEM_COLOR_MODE_FADED )
		{
			color.SetColor(m_ColorIntensityFaded.r(),m_ColorIntensityFaded.g(),m_ColorIntensityFaded.b(),colorCustom.a());
		}
		else if(colorMode >= ITEM_COLOR_MODE_TEAMCOLORED ) 
		{
			color.SetColor(m_ColorTeam.r(),m_ColorTeam.g(),m_ColorTeam.b(),colorCustom.a());
		}
		else
		{
			color = colorCustom;
		}
	}
    	 
	void FFQuantityItem::CalculateTextPositionOffset( int &outX, int &outY, int iPos )
	{	
		switch(iPos)
		{
		case ANCHORPOS_TOPLEFT:
			outX = 0;
			outY = 0;
			break;
		case ANCHORPOS_MIDDLELEFT:
			outX = 0;
			outY = (m_iBarHeight * m_flScale) / 2;
			break;
		case ANCHORPOS_BOTTOMLEFT:
			outX = 0;
			outY = (m_iBarHeight * m_flScale);
			break;
			
		case ANCHORPOS_TOPCENTER:
			outX =(m_iBarWidth * m_flScale) / 2;
			outY = 0;
			break;
		case ANCHORPOS_MIDDLECENTER:
			outX =(m_iBarWidth * m_flScale) / 2;
			outY = (m_iBarHeight * m_flScale) / 2;
			break;
		case ANCHORPOS_BOTTOMCENTER:
			outX = (m_iBarWidth * m_flScale) / 2;
			outY = (m_iBarHeight * m_flScale);
			break;
						
		case ANCHORPOS_TOPRIGHT:
			outX = (m_iBarWidth * m_flScale);
			outY = 0;
			break;
		case ANCHORPOS_MIDDLERIGHT:
			outX = (m_iBarWidth * m_flScale);
			outY = (m_iBarHeight * m_flScale) / 2;
			break;
		case ANCHORPOS_BOTTOMRIGHT:
			outX = (m_iBarWidth * m_flScale);
			outY = (m_iBarHeight * m_flScale);
			break;
		}	
	}	
	void FFQuantityItem::CalculateTextAlignmentOffset( int &iX, int &iY, int &iWide, int &iTall, int iAlignHoriz, int iAlignVert, HFont hfFont, wchar_t* wszString )
	{
		// havent loaded a font yet, probably during first tick so return
		if (hfFont == NULL)
		{
			return;
		}

		//Calculate this with the real size 
		//and then scale it down to 640*480
		//(but make sure its rouned up so we don't cut text off!)
		//int iWideTemp = 0, iTallTemp = 0;
		surface()->GetTextSize(hfFont, wszString,  iWide, iTall); //iWideTemp, iTallTemp);

		//+0.5 so it rounds up!
		//+0.1 for grace...
		//iWide = (int)((float)(iWideTemp) / m_flScale + 0.6f);
		//iTall = (int)((float)(iTallTemp) / m_flScale + 0.6f);
		
		switch(iAlignHoriz)
		{
		case ALIGN_CENTER:
			iX = -iWide/2;
			break;
		case ALIGN_RIGHT:
			iX = -iWide;
			break;
		case ALIGN_LEFT:
		default:
			iX = 0;
			break;
		}

		switch(iAlignVert)
		{
		case ALIGN_MIDDLE:
			iY = -iTall / 2;
			break;
		case ALIGN_BOTTOM:
			iY = -iTall;
			break;
		case ALIGN_TOP:
		default:
			iY = 0;
			break;
		}
	}
}