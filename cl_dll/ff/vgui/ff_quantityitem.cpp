//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
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
#include "ff_quantityitem.h"
#include "ff_quantityhelper.h"

#include "ff_utils.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"


namespace vgui
{
	FFQuantityItem::FFQuantityItem(Panel *parent, const char *pElementName) : BaseClass(parent, pElementName)
	{
		m_iWidth = 0;
		m_iHeight = 0;

		m_iScreenWide = 0;
		m_iScreenTall = 0;

		m_flScaleX = 1.0f;
		m_flScaleY = 1.0f;

		m_bDisabled = false;
		m_bHasStyleData = false;

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
		m_bIntensityAmountScaled = false;
		m_bIntensityValuesFixed = false;

		m_iIconAnchorPosition = FFQuantityHelper::ANCHORPOS_MIDDLELEFT;
		m_iLabelAnchorPosition = FFQuantityHelper::ANCHORPOS_TOPLEFT;
		m_iAmountAnchorPosition = FFQuantityHelper::ANCHORPOS_TOPRIGHT;

		m_iIconAlignHoriz = FFQuantityHelper::ALIGN_RIGHT;
		m_iLabelAlignHoriz = FFQuantityHelper::ALIGN_LEFT;
		m_iAmountAlignHoriz = FFQuantityHelper::ALIGN_RIGHT;

		m_iIconAlignVert = FFQuantityHelper::ALIGN_MIDDLE;
		m_iLabelAlignVert = FFQuantityHelper::ALIGN_BOTTOM;
		m_iAmountAlignVert = FFQuantityHelper::ALIGN_BOTTOM;

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

		m_bShowBar = true;
		m_bShowBarBorder = true;
		m_bShowBarBackground = true;
		m_bShowIcon = true;
		m_bShowLabel = true;
		m_bShowAmount = true;
		m_bShowAmountMax = true;
		m_bShowAmountSticky = true;

		memset(m_wszIcon, '\0', sizeof( m_wszAmount ) );
		memset(m_wszLabel, '\0', sizeof( m_wszAmountDisplay ) );
		memset(m_wszIcon, '\0', sizeof( m_wszAmountMax ) );
		memset(m_wszLabel, '\0', sizeof( m_wszAmountMaxDisplay ) );
		memset(m_wszIcon, '\0', sizeof( m_wszIcon ) );
		memset(m_wszLabel, '\0', sizeof( m_wszLabel ) );

		m_iMaxAmount = 100;
		m_iAmount = 100;

		swprintf(m_wszAmount, L"%i", m_iAmount);
		swprintf(m_wszAmountMax, L"%i", m_iMaxAmount);
		swprintf(m_wszAmountDisplay, L"%ls/%ls", m_wszAmount, m_wszAmountMax);
		swprintf(m_wszAmountMaxDisplay, L"%ls/%ls", m_wszAmountMax, m_wszAmountMax);
		m_bIconFontGlyph = false;

		for (int i = 0; i < QUANTITYITEMTEXTSIZES * 3; i++)
		{
			m_hfQuantityItemText[i] = NULL;
			m_hfQuantityItemTahomaText[i] = NULL;
			m_hfQuantityItemIcon[i] = NULL;
			m_hfQuantityItemGlyph[i] = NULL;
		}

		// Only update once every second
		// to catch resolution changes
		ivgui()->AddTickSignal(GetVPanel(), 1000); 
	}

	void FFQuantityItem::ApplySchemeSettings( IScheme *pScheme )
	{
		HScheme QuantityItemScheme = scheme()->LoadSchemeFromFile("resource/QuantityPanelScheme.res", "QuantityPanelScheme");
		IScheme *qbScheme = scheme()->GetIScheme(QuantityItemScheme);

		//the non scaled fonts are used for positioning calculations
		//for 'simplicity' and consistency everything is calculated as if in 640x480
		//decided I'll make the text size variable too
		//size*3 then offset for shadow and non proportional
		for(int i = 0; i < QUANTITYITEMTEXTSIZES; ++i)
		{
			m_hfQuantityItemText[i*3] = qbScheme->GetFont( VarArgs("QuantityItem%d",i), true );
			m_hfQuantityItemText[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityItemShadow%d",i), true );
			m_hfQuantityItemText[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityItem%d",i), false );
			m_hfQuantityItemTahomaText[i*3] = qbScheme->GetFont( VarArgs("QuantityItemTahoma%d",i), true );
			m_hfQuantityItemTahomaText[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityItemTahomaShadow%d",i), true );
			m_hfQuantityItemTahomaText[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityItemTahoma%d",i), false );
		}

		for(int i = 0; i < QUANTITYITEMICONSIZES; ++i)
		{
			m_hfQuantityItemIcon[i*3] = qbScheme->GetFont( VarArgs("QuantityItemIcon%d",i), true );
			m_hfQuantityItemIcon[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityItemIconShadow%d",i), true );
			m_hfQuantityItemIcon[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityItemIcon%d",i), false );
			m_hfQuantityItemGlyph[i*3] = qbScheme->GetFont( VarArgs("QuantityItemGlyph%d",i), true );
			m_hfQuantityItemGlyph[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityItemGlyphShadow%d",i), true );
			m_hfQuantityItemGlyph[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityItemGlyph%d",i), false );
		}

		m_hfAmount = m_hfQuantityItemText[0];
		m_hfLabel = m_hfQuantityItemText[0];
		m_hfIcon = m_hfQuantityItemIcon[0];
		
 		BaseClass::ApplySchemeSettings( pScheme );
	}

	void FFQuantityItem::OnTick( )
	{
		VPROF_BUDGET("FFQuantityItem::OnTick","QuantityItems");

		// Get the screen width/height
		int iScreenWide, iScreenTall;
		surface()->GetScreenSize( iScreenWide, iScreenTall );

		if (m_iScreenWide == iScreenWide 
			&& m_iScreenTall == iScreenTall)
		{
			return;
		}
	
		m_iScreenWide = iScreenWide;
		m_iScreenTall = iScreenTall;

		// "map" screen res to 640/480
		m_flScaleX 
			= 1 
				/ (640.0f / iScreenWide);
		
		m_flScaleY 
			= 1 
				/ (480.0f / iScreenTall);

		if (m_bHasStyleData) 
		{
			RecalculateIconPosition();
			RecalculateLabelPosition();
			RecalculateAmountMaxDisplay();
			RecalculateAmountMaxPosition();
			RecalculateAmountPosition();
			RecalculatePaintOffset();
		}
	}

	void FFQuantityItem::Paint( )
	{
		VPROF_BUDGET("FFQuantityItem::Paint","QuantityItems");

		BaseClass::Paint();

		int iOffsetX;
		int iOffsetY;

		if(m_iPaintOffsetOverrideX != -1)
		{
			iOffsetX = m_iPaintOffsetOverrideX;
		}
		else
		{
			iOffsetX = m_iPaintOffsetX * m_flScaleX;
		}

		if(m_iPaintOffsetOverrideY != -1)
		{
			iOffsetY = m_iPaintOffsetOverrideY;
		}
		else
		{
			iOffsetY = m_iPaintOffsetY * m_flScaleY;
		}

		// Set text position based on alignment & text
		if(m_bShowBarBorder)
		{
			surface()->DrawSetColor( m_ColorBarBorder );

			int iScaledBarBorderWidth = m_iBarBorderWidth * m_flScaleX;
			for( int i = 1; i <= iScaledBarBorderWidth; ++i )
			{
				surface()->DrawOutlinedRect(
					iOffsetX - i,
					iOffsetY - i,
					iOffsetX + m_iBarWidth * m_flScaleX + i,
					iOffsetY + m_iBarHeight * m_flScaleY + i);
			}
		}

		if(m_bShowBarBackground)
		{
			surface()->DrawSetColor( m_ColorBarBackground );
			surface()->DrawFilledRect(
				iOffsetX,
				iOffsetY,
				iOffsetX + m_iBarWidth * m_flScaleX,
				iOffsetY + m_iBarHeight * m_flScaleY);
		}

		if(m_bShowBar)
		{
			surface()->DrawSetColor( m_ColorBar );
			surface()->DrawFilledRect(
				iOffsetX + m_iBarX0QuantityOffset * m_flScaleX,
				iOffsetY + m_iBarY0QuantityOffset * m_flScaleY,
				iOffsetX + ( m_iBarWidth + m_iBarX1QuantityOffset ) * m_flScaleX,
				iOffsetY + ( m_iBarHeight + m_iBarY1QuantityOffset ) * m_flScaleY);
		}

		if(m_bShowIcon && m_wszIcon)
		{
			DrawText(
				m_wszIcon,
				m_hfIcon,
				m_ColorIcon,
				iOffsetX + m_iIconPosX,
				iOffsetY + m_iIconPosY);
		}

		if(m_bShowLabel && m_wszLabel)
		{
			DrawText(
				m_wszLabel,
				m_hfLabel,
				m_ColorLabel,
				iOffsetX + m_iLabelPosX,
				iOffsetY + m_iLabelPosY);
		}

		if(m_bShowAmount && m_wszAmountDisplay)
		{
			DrawText(
				m_wszAmountDisplay,
				m_hfAmount,
				m_ColorAmount,
				iOffsetX + m_iAmountPosX,
				iOffsetY + m_iAmountPosY);
		}
	}

	void FFQuantityItem::DrawText(
		wchar_t* wszText,
		HFont font,
		Color color,
		int iXPosition,
		int iYPosition)
	{
		surface()->DrawSetTextFont(font);
		surface()->DrawSetTextColor(color);
		surface()->DrawSetTextPos(iXPosition, iYPosition);
		surface()->DrawUnicodeString(wszText);
	}

	vgui::HFont FFQuantityItem::GetFont(
		vgui::HFont* hfFamily,
		int iSize,
		bool bUseModifier)
	{
		return FFQuantityHelper::GetFont(
			hfFamily, 
			iSize, 
			bUseModifier);
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

	bool FFQuantityItem::SetAmountFontShadow( bool bUseShadow )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bAmountFontShadow, bUseShadow);

		if(bHasChanged)
		{
			RecalculateAmountFont();
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetIconFontShadow( bool bUseShadow )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bIconFontShadow, bUseShadow);

		if(bHasChanged)
		{
			RecalculateIconFont();
			RecalculateIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetLabelFontShadow( bool bUseShadow )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bLabelFontShadow, bUseShadow);

		if(bHasChanged)
		{
			RecalculateLabelFont();
			RecalculateLabelPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetIconFontGlyph( bool bIconIsGlyph )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bIconFontGlyph, bIconIsGlyph);
		
		if(bHasChanged)
		{
			RecalculateIconFont();
			RecalculateIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetLabelFontTahoma( bool bLabelFontTahoma )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bLabelFontTahoma, bLabelFontTahoma);
		
		if(bHasChanged)
		{
			RecalculateLabelFont();
			RecalculateLabelPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetAmountFontTahoma( bool bAmountFontTahoma )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bAmountFontTahoma, bAmountFontTahoma);

		if(bHasChanged)
		{
			RecalculateAmountFont();
			RecalculateAmountMaxPosition();
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetIconSize( int newIconSize )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iSizeIcon, newIconSize);

		if(bHasChanged)
		{
			RecalculateIconFont();
			RecalculateIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetLabelSize( int newLabelSize )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iSizeLabel, newLabelSize);
		
		if(bHasChanged)
		{
			RecalculateLabelFont();
			RecalculateLabelPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetAmountSize( int newAmountSize )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iSizeAmount, newAmountSize);

		if(bHasChanged)
		{
			RecalculateAmountFont();
			RecalculateAmountMaxPosition();
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetAmount( int iAmount )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iAmount, iAmount);
		
		if(bHasChanged)
		{
			char szAmount[10];
			Q_snprintf( szAmount, 5, "%i%", iAmount );
			localize()->ConvertANSIToUnicode( szAmount, m_wszAmount, sizeof( m_wszAmount ) );

			RecalculateQuantity();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetAmountMax( int iAmountMax, bool bRecalculatePaintOffset )
	{
		// don't allow 0 cause that doesn't make sense
		// plus it'll crash with divide by 0 when calculating the colour!
		if (iAmountMax == 0) 
		{
			return false;
		}

		bool bHasChanged = FFQuantityHelper::Change(m_iMaxAmount, iAmountMax);

		if(bHasChanged)
		{
			char szAmountMax[10];
			Q_snprintf( szAmountMax, 5, "%i%", iAmountMax );
			localize()->ConvertANSIToUnicode( szAmountMax, m_wszAmountMax, sizeof( m_wszAmountMax ) );

			RecalculateAmountMaxDisplay();
			RecalculateAmountMaxPosition();
			RecalculateQuantity();

			if (bRecalculatePaintOffset)
			{
				RecalculatePaintOffset();
			}
		}

		return bHasChanged;
	}

	void FFQuantityItem::SetIconChar( char *newIconChar, bool bRecalculatePaintOffset )
	{
		char szIcon[5];
		Q_snprintf( szIcon, 2, "%s%", newIconChar );
		localize()->ConvertANSIToUnicode( szIcon, m_wszIcon, sizeof( m_wszIcon ) );

		RecalculateIconPosition();

		if (bRecalculatePaintOffset)
		{
			RecalculatePaintOffset();
		}
	}

	void FFQuantityItem::SetLabelText( char *newLabelText, bool bRecalculatePaintOffset )
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

		RecalculateLabelPosition();

		if (bRecalculatePaintOffset)
		{
			RecalculatePaintOffset();
		}
	}

	bool FFQuantityItem::SetBarWidth( int iBarWidth )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iBarWidth, iBarWidth);
		
		if (bHasChanged) 
		{
			RecalculateIconPosition();
			RecalculateLabelPosition();
			RecalculateAmountMaxPosition();
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetBarHeight( int iBarHeight )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iBarHeight, iBarHeight);

		if (bHasChanged)
		{
			RecalculateIconPosition();
			RecalculateLabelPosition();
			RecalculateAmountMaxPosition();
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetBarBorderWidth( int iBarBorderWidth )
	{
		return FFQuantityHelper::Change(m_iBarBorderWidth, iBarBorderWidth);
	}

	bool FFQuantityItem::SetBarOrientation( int iOrientation )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iBarOrientation, iOrientation);

		if(bHasChanged)
		{
			RecalculateQuantity();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::ShowBar( bool bShowBar )
	{
		return FFQuantityHelper::Change(m_bShowBar, bShowBar);
	}

	bool FFQuantityItem::ShowBarBackground( bool bShowBarBackground )
	{
		return FFQuantityHelper::Change(m_bShowBarBackground, bShowBarBackground);
	}

	bool FFQuantityItem::ShowBarBorder( bool bShowBarBorder )
	{
		return FFQuantityHelper::Change(m_bShowBarBorder, bShowBarBorder);
	}

	bool FFQuantityItem::ShowIcon( bool bShowIcon )
	{
		return FFQuantityHelper::Change(m_bShowIcon, bShowIcon);
	}

	bool FFQuantityItem::ShowLabel( bool bShowLabel )
	{
		return FFQuantityHelper::Change(m_bShowLabel, bShowLabel);
	}

	bool FFQuantityItem::ShowAmount( bool bShowAmount )
	{
		return FFQuantityHelper::Change(m_bShowAmount, bShowAmount);
	}

	bool FFQuantityItem::ShowAmountMax( bool bShowAmountMax )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bShowAmountMax, bShowAmountMax);
		
		if(bHasChanged)
		{
			RecalculateAmountMaxDisplay();
			RecalculateAmountMaxPosition();
			RecalculateQuantity();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::ShowAmountSticky( bool bShowAmountSticky )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bShowAmountSticky, bShowAmountSticky);
		
		if(bHasChanged)
		{
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}
	

	bool FFQuantityItem::SetCustomBarColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrBarCustomColor, clrCustom);

		if(bHasChanged)
		{
			RecalculateColor(m_ColorBar, m_iBarColorMode, m_clrBarCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetCustomBarBorderColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrBarBorderCustomColor, clrCustom);

		if(bHasChanged)
		{
			RecalculateColor(m_ColorBarBorder, m_iBarBorderColorMode, m_clrBarBorderCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetCustomBarBackgroundColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrBarBackgroundCustomColor, clrCustom);

		if(bHasChanged)
		{
			RecalculateColor(m_ColorBarBackground, m_iBarBackgroundColorMode, m_clrBarBackgroundCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetCustomAmountColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrAmountCustomColor, clrCustom);

		if(bHasChanged)
		{
			RecalculateColor(m_ColorAmount, m_iAmountColorMode, m_clrAmountCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetCustomIconColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrIconCustomColor, clrCustom);

		if(bHasChanged)
		{
			RecalculateColor(m_ColorIcon, m_iIconColorMode, m_clrIconCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetCustomLabelColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrLabelCustomColor, clrCustom);
		
		if(bHasChanged)
		{
			RecalculateColor(m_ColorLabel, m_iLabelColorMode, m_clrLabelCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetTeamColor( Color clrTeam )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_clrTeam, clrTeam);
		
		if (!bHasChanged)
		{
			return false;
		}

		if(m_iBarColorMode == ITEM_COLOR_MODE_TEAM)
		{
			RecalculateColor(m_ColorBar, m_iBarColorMode, m_clrBarCustomColor);
		}

		if(m_iBarBorderColorMode == ITEM_COLOR_MODE_TEAM)
		{
			RecalculateColor(m_ColorBarBorder, m_iBarBorderColorMode, m_clrBarBorderCustomColor);
		}
		
		if(m_iBarBackgroundColorMode == ITEM_COLOR_MODE_TEAM)
		{
			RecalculateColor(m_ColorBarBackground, m_iBarBackgroundColorMode, m_clrBarBackgroundCustomColor);
		}
		
		if(m_iIconColorMode == ITEM_COLOR_MODE_TEAM)
		{
			RecalculateColor(m_ColorIcon, m_iIconColorMode, m_clrIconCustomColor);
		}
		
		if(m_iLabelColorMode == ITEM_COLOR_MODE_TEAM)
		{
			RecalculateColor(m_ColorLabel, m_iLabelColorMode, m_clrLabelCustomColor);
		}
		
		if(m_iAmountColorMode == ITEM_COLOR_MODE_TEAM)
		{
			RecalculateColor(m_ColorAmount, m_iAmountColorMode, m_clrAmountCustomColor);
		}

		return true;
	}

	bool FFQuantityItem::SetBarColorMode( int iColorModeBar )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iBarColorMode, iColorModeBar);

		if(bHasChanged)
		{
			RecalculateColor(m_ColorBar, m_iBarColorMode, m_clrBarCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetBarBorderColorMode( int iColorModeBarBorder )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iBarBorderColorMode, iColorModeBarBorder);
		
		if(bHasChanged)
		{
			RecalculateColor(m_ColorBarBorder, m_iBarBorderColorMode, m_clrBarBorderCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetBarBackgroundColorMode( int iColorModeBarBackround )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iBarBackgroundColorMode, iColorModeBarBackround);
		
		if(bHasChanged)
		{
			RecalculateColor(m_ColorBarBackground, m_iBarBackgroundColorMode, m_clrBarBackgroundCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetAmountColorMode( int iColorModeAmount )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iAmountColorMode, iColorModeAmount);
		
		if(bHasChanged)
		{
			RecalculateColor(m_ColorAmount, m_iAmountColorMode, m_clrAmountCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetIconColorMode( int iColorModeIcon )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iIconColorMode, iColorModeIcon);
		
		if(bHasChanged)
		{
			RecalculateColor(m_ColorIcon, m_iIconColorMode, m_clrIconCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetLabelColorMode( int iColorModeLabel )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iLabelColorMode, iColorModeLabel);
		
		if(bHasChanged)
		{
			RecalculateColor(m_ColorLabel, m_iLabelColorMode, m_clrLabelCustomColor);
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetAmountPositionOffsetX( int iOffsetXAmount )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iAmountPositionOffsetX, iOffsetXAmount);
		
		if(bHasChanged)
		{
			RecalculateAmountMaxPosition();
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetAmountPositionOffsetY( int iOffsetYAmount )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iAmountPositionOffsetY, iOffsetYAmount);
		
		if(bHasChanged)
		{
			RecalculateAmountMaxPosition();
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetIconPositionOffsetX( int iOffsetXIcon )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iIconPositionOffsetX , iOffsetXIcon);
		
		if(bHasChanged)
		{
			RecalculateIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetIconPositionOffsetY( int iOffsetYIcon )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iIconPositionOffsetY, iOffsetYIcon);
		
		if(bHasChanged)
		{
			RecalculateIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetLabelPositionOffsetX( int iOffsetXLabel )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iLabelPositionOffsetX, iOffsetXLabel);

		if(bHasChanged)
		{
			RecalculateLabelPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetLabelPositionOffsetY( int iOffsetYLabel )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iLabelPositionOffsetY, iOffsetYLabel);
		if(bHasChanged)
		{
			RecalculateLabelPosition();
		}

		return bHasChanged;
	}

	int FFQuantityItem::GetAmount( ) { return m_iAmount; }

	int FFQuantityItem::GetMaxAmount( ) { return m_iMaxAmount; }

	bool FFQuantityItem::SetAmountAnchorPosition( int iPosAmount )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iAmountAnchorPosition, iPosAmount);

		if (bHasChanged)
		{
			RecalculateAmountMaxPosition();
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetIconAnchorPosition( int iPosIcon )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iIconAnchorPosition, iPosIcon);

		if (bHasChanged)
		{
			RecalculateIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetLabelAnchorPosition( int iPosLabel )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iLabelAnchorPosition, iPosLabel);

		if (bHasChanged)
		{
			RecalculateLabelPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetAmountAlignmentHorizontal( int iAlignHorizAmount )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iAmountAlignHoriz, iAlignHorizAmount);

		if (bHasChanged)
		{
			RecalculateAmountMaxPosition();
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetIconAlignmentHorizontal( int iIconAlignHoriz )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iIconAlignHoriz, iIconAlignHoriz);

		if (bHasChanged)
		{
			RecalculateIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetLabelAlignmentHorizontal( int iAlignHorizLabel )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iLabelAlignHoriz, iAlignHorizLabel);

		if (bHasChanged)
		{
			RecalculateLabelPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetAmountAlignmentVertical( int iAmountAlignVert )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iAmountAlignVert, iAmountAlignVert);

		if (bHasChanged)
		{
			RecalculateAmountMaxPosition();
			RecalculateAmountPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetIconAlignmentVertical( int iIconAlignVert )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iIconAlignVert, iIconAlignVert);

		if (bHasChanged)
		{
			RecalculateIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityItem::SetLabelAlignmentVertical( int iLabelAlignVert )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iLabelAlignVert, iLabelAlignVert);
		
		if (bHasChanged)
		{
			RecalculateLabelPosition();
		}

		return bHasChanged;
	}

	void FFQuantityItem::SetIntensityControl( int iRed, int iOrange,int iYellow, int iGreen )
	{
		m_iIntensityRed = iRed;
		m_iIntensityOrange = iOrange;
		m_iIntensityYellow = iYellow;
		m_iIntensityGreen = iGreen;
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
		int iBarWidth = GetInt("barWidth", kvStyleData, kvDefaultStyleData);
		int iBarHeight = GetInt("barHeight", kvStyleData, kvDefaultStyleData);
		int iBarBorderWidth = GetInt("barBorderWidth", kvStyleData, kvDefaultStyleData);
		int iBarOrientation = GetInt("barOrientation", kvStyleData, kvDefaultStyleData);

		// I feel better checking things have changed before setting them else we might call lots of unneccesary methods
		if(iBarWidth != -1 && SetBarWidth(iBarWidth))
		{
			bRecalculatePaintOffset = true;
		}

		if(iBarHeight != -1 && SetBarHeight(iBarHeight))
		{
			bRecalculatePaintOffset = true;
		}

		if(iBarBorderWidth != -1 && SetBarBorderWidth(iBarBorderWidth))
		{
			bRecalculatePaintOffset = true;
		}

		if(iBarOrientation != -1)
		{
			SetBarOrientation(iBarOrientation);
		}

		KeyValues *kvComponentStyleData = GetData("Bar", kvStyleData, kvDefaultStyleData);

		if(kvComponentStyleData)
		{
			int iShow = GetInt("show", kvComponentStyleData);

			int iRed = GetInt("red", kvComponentStyleData);
			int iGreen = GetInt("green", kvComponentStyleData);
			int iBlue = GetInt("blue", kvComponentStyleData);
			int iAlpha = GetInt("alpha", kvComponentStyleData);
			int iColorMode = GetInt("colorMode", kvComponentStyleData);

			if(iShow != -1 && ShowBar(iShow == 1))
			{
				bRecalculatePaintOffset = true;
			}

			if(iColorMode != -1)
			{
				SetBarColorMode(iColorMode);
			}

			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				SetCustomBarColor(iRed, iGreen, iBlue, iAlpha);
			}
		}

		kvComponentStyleData = GetData("BarBorder", kvStyleData, kvDefaultStyleData);

		if(kvComponentStyleData)
		{
			int iShow = GetInt("show", kvComponentStyleData);

			int iRed = GetInt("red", kvComponentStyleData);
			int iGreen = GetInt("green", kvComponentStyleData);
			int iBlue = GetInt("blue", kvComponentStyleData);
			int iAlpha = GetInt("alpha", kvComponentStyleData);
			int iColorMode = GetInt("colorMode", kvComponentStyleData);

			if(iShow != -1 && ShowBarBorder(iShow == 1))
			{
				bRecalculatePaintOffset = true;
			}

			if(iColorMode != -1)
			{
				SetBarBorderColorMode(iColorMode);
			}

			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				SetCustomBarBorderColor(iRed, iGreen, iBlue, iAlpha);
			}
		}

		kvComponentStyleData = GetData("BarBackground", kvStyleData, kvDefaultStyleData);

		if(kvComponentStyleData)
		{
			int iShow = GetInt("show", kvComponentStyleData);

			int iRed = GetInt("red", kvComponentStyleData);
			int iGreen = GetInt("green", kvComponentStyleData);
			int iBlue = GetInt("blue", kvComponentStyleData);
			int iAlpha = GetInt("alpha", kvComponentStyleData);
			int iColorMode = GetInt("colorMode", kvComponentStyleData);

			if(iShow != -1 && ShowBarBackground(iShow == 1))
			{
				bRecalculatePaintOffset = true;
			}

			if(iColorMode != -1)
			{
				SetBarBackgroundColorMode(iColorMode);
			}

			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				SetCustomBarBackgroundColor(iRed, iGreen, iBlue, iAlpha);
			}
		}

		kvComponentStyleData = GetData("Icon", kvStyleData, kvDefaultStyleData);

		if(kvComponentStyleData)
		{
			int iShow = GetInt("show", kvComponentStyleData);
			int iSize = GetInt("size", kvComponentStyleData);
			int iShadow = GetInt("shadow", kvComponentStyleData);

			int iRed = GetInt("red", kvComponentStyleData);
			int iGreen = GetInt("green", kvComponentStyleData);
			int iBlue = GetInt("blue", kvComponentStyleData);
			int iAlpha = GetInt("alpha", kvComponentStyleData);
			int iColorMode = GetInt("colorMode", kvComponentStyleData);

			int iAnchorPosition = GetInt("anchorPosition", kvComponentStyleData);
			int iAlignHoriz = GetInt("alignH", kvComponentStyleData);
			int iAlignVert = GetInt("alignV", kvComponentStyleData);

			int iOffsetX = GetInt("offsetX", kvComponentStyleData, -9999);
			int iOffsetY = GetInt("offsetY", kvComponentStyleData, -9999);

			if(iShow != -1 && ShowIcon(iShow == 1))
			{
				bRecalculatePaintOffset = true;
			}

			if(iColorMode != -1)
			{
				SetIconColorMode(iColorMode);
			}

			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				SetCustomIconColor(iRed, iGreen, iBlue, iAlpha);
			}

			if(iOffsetX != -9999 && SetIconPositionOffsetX(iOffsetX))
			{
				bRecalculatePaintOffset = true;
			}

			if(iOffsetY != -9999 && SetIconPositionOffsetY(iOffsetY))
			{
				bRecalculatePaintOffset = true;
			}

			if(iAnchorPosition != -1 && SetIconAnchorPosition(iAnchorPosition))
			{
				bRecalculatePaintOffset = true;
			}

			if(iAlignHoriz != -1 && SetIconAlignmentHorizontal(iAlignHoriz))
			{
				bRecalculatePaintOffset = true;
			}

			if(iAlignVert != -1 && SetIconAlignmentVertical(iAlignVert))
			{
				bRecalculatePaintOffset = true;
			}

			if(iSize != -1 && SetIconSize(iSize))
			{
				bRecalculatePaintOffset = true;
			}

			if(iShadow != -1 && SetIconFontShadow(iShadow == 1))
			{
				bRecalculatePaintOffset = true;
			}
		}

		kvComponentStyleData = GetData("Label", kvStyleData, kvDefaultStyleData);

		if(kvComponentStyleData)
		{
			int iShow = GetInt("show", kvComponentStyleData);
			int iSize = GetInt("size", kvComponentStyleData);
			int iShadow = GetInt("shadow", kvComponentStyleData);

			int iRed = GetInt("red", kvComponentStyleData);
			int iGreen = GetInt("green", kvComponentStyleData);
			int iBlue = GetInt("blue", kvComponentStyleData);
			int iAlpha = GetInt("alpha", kvComponentStyleData);
			int iColorMode = GetInt("colorMode", kvComponentStyleData);

			int iAnchorPosition = GetInt("anchorPosition", kvComponentStyleData);
			int iAlignHoriz = GetInt("alignH", kvComponentStyleData);
			int iAlignVert = GetInt("alignV", kvComponentStyleData);

			int iOffsetX = GetInt("offsetX", kvComponentStyleData, -9999);
			int iOffsetY = GetInt("offsetY", kvComponentStyleData, -9999);

			int iFontTahoma = GetInt("fontTahoma", kvComponentStyleData);

			if(iShow != -1 && ShowLabel(iShow == 1))
			{
				bRecalculatePaintOffset = true;
			}

			if(iColorMode != -1)
			{
				SetLabelColorMode(iColorMode);
			}

			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				SetCustomLabelColor(iRed, iGreen, iBlue, iAlpha);
			}

			if(iOffsetX != -9999 && SetLabelPositionOffsetX(iOffsetX))
			{
				bRecalculatePaintOffset = true;
			}

			if(iOffsetY != -9999 && SetLabelPositionOffsetY(iOffsetY))
			{
				bRecalculatePaintOffset = true;
			}

			if(iAnchorPosition != -1 && SetLabelAnchorPosition(iAnchorPosition))
			{
				bRecalculatePaintOffset = true;
			}

			if(iAlignHoriz != -1 && SetLabelAlignmentHorizontal(iAlignHoriz))
			{
				bRecalculatePaintOffset = true;
			}

			if(iAlignVert != -1 && SetLabelAlignmentVertical(iAlignVert))
			{
				bRecalculatePaintOffset = true;
			}

			if(iSize != -1 && SetLabelSize(iSize))
			{
				bRecalculatePaintOffset = true;
			}

			if(iShadow != -1 && SetLabelFontShadow(iShadow == 1))
			{
				bRecalculatePaintOffset = true;
			}

			if(iFontTahoma != -1 && SetLabelFontTahoma(iFontTahoma == 1))
			{
				bRecalculatePaintOffset = true;
			}
		}

		kvComponentStyleData = GetData("Amount", kvStyleData, kvDefaultStyleData);

		if(kvComponentStyleData)
		{
			int iShow = GetInt("show", kvComponentStyleData);
			int iSticky = GetInt("sticky", kvComponentStyleData);
			int iSize = GetInt("size", kvComponentStyleData);
			int iShadow = GetInt("shadow", kvComponentStyleData);

			int iRed = GetInt("red", kvComponentStyleData);
			int iGreen = GetInt("green", kvComponentStyleData);
			int iBlue = GetInt("blue", kvComponentStyleData);
			int iAlpha = GetInt("alpha", kvComponentStyleData);
			int iColorMode = GetInt("colorMode", kvComponentStyleData);

			int iAnchorPosition = GetInt("anchorPosition", kvComponentStyleData);
			int iAlignHoriz = GetInt("alignH", kvComponentStyleData);
			int iAlignVert = GetInt("alignV", kvComponentStyleData);

			int iOffsetX = GetInt("offsetX", kvComponentStyleData, -9999);
			int iOffsetY = GetInt("offsetY", kvComponentStyleData, -9999);

			int iFontTahoma = GetInt("fontTahoma", kvComponentStyleData);

			if(iShow != -1 && ShowAmount(iShow == 1))
			{
				bRecalculatePaintOffset = true;
			}

			if(iSticky != -1)
			{
				ShowAmountSticky(iSticky == 1);
			}

			if(iColorMode != -1)
			{
				SetAmountColorMode(iColorMode);
			}

			if(iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
			{
				SetCustomAmountColor(iRed, iGreen, iBlue, iAlpha);
			}

			if(iOffsetX != -9999 && SetAmountPositionOffsetX(iOffsetX))
			{
				bRecalculatePaintOffset = true;
			}

			if(iOffsetY != -9999 && SetAmountPositionOffsetY(iOffsetY))
			{
				bRecalculatePaintOffset = true;
			}

			if(iAnchorPosition != -1 && SetAmountAnchorPosition(iAnchorPosition))
			{
				bRecalculatePaintOffset = true;
			}

			if(iAlignHoriz != -1 && SetAmountAlignmentHorizontal(iAlignHoriz))
			{
				bRecalculatePaintOffset = true;
			}

			if(iAlignVert != -1 && SetAmountAlignmentVertical(iAlignVert))
			{
				bRecalculatePaintOffset = true;
			}

			if(iSize != -1 && SetAmountSize(iSize))
			{
				bRecalculatePaintOffset = true;
			}

			if(iShadow != -1 && SetAmountFontShadow(iShadow == 1))
			{
				bRecalculatePaintOffset = true;
			}

			if(iFontTahoma != -1 && SetAmountFontTahoma(iFontTahoma == 1))
			{
				bRecalculatePaintOffset = true;
			}
		}

		m_bHasStyleData = true;

		if (bRecalculatePaintOffset
			&& m_iScreenWide != 0
			&& m_iScreenTall != 0)
		{
			RecalculatePaintOffset();
		}
	}

	int FFQuantityItem::GetInt(const char *keyName, KeyValues *kvStyleData, int iDefaultValue)
	{
		return FFQuantityHelper::GetInt(keyName, kvStyleData, iDefaultValue);
	}

	int FFQuantityItem::GetInt(const char *keyName, KeyValues *kvStyleData, KeyValues *kvDefaultStyleData, int iDefaultValue)
	{
		return FFQuantityHelper::GetInt(keyName, kvStyleData, kvDefaultStyleData, iDefaultValue);
	}

	KeyValues* FFQuantityItem::GetData(const char *keyName, KeyValues *kvStyleData, KeyValues *kvDefaultStyleData)
	{
		return FFQuantityHelper::GetData(keyName, kvStyleData, kvDefaultStyleData);
	}

	void FFQuantityItem::RecalculatePaintOffset( )
	{
		float flX0 = 0.0f;
		float flY0 = 0.0f;
		float flX1 = 0.0f;
		float flY1 = 0.0f;

		if( m_bShowBar || m_bShowBarBackground || m_bShowBarBorder )
		{
			flX1 += m_iBarWidth * m_flScaleX;
			flY1 += m_iBarHeight * m_flScaleY;
		}

		if( m_bShowBarBorder )
		{
			int iScaledBarBorderWidth = m_iBarBorderWidth * m_flScaleX;
			flX0 -= iScaledBarBorderWidth;
			flY0 -= iScaledBarBorderWidth;
			flX1 += iScaledBarBorderWidth;
			flY1 += iScaledBarBorderWidth;
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

		bool bCalculatePositionalHashCode = false;

		int iPaintOffsetX = -flX0;
		int iPaintOffsetY = -flY0;
		if(m_iPaintOffsetX != iPaintOffsetX
			|| m_iPaintOffsetY != iPaintOffsetY)
		{
			m_iPaintOffsetX = iPaintOffsetX;
			m_iPaintOffsetY = iPaintOffsetY;

			bCalculatePositionalHashCode = true;
		}

		int iWidth = (int)(flX1 - flX0);
		int iHeight = (int)(flY1 - flY0);
		if(m_iWidth != iWidth 
			|| m_iHeight != iHeight)
		{
			m_iWidth = iWidth;
			m_iHeight = iHeight;

			bCalculatePositionalHashCode = true;
		}

		//just so if we ever use the quantity bar on its own and not in a panel it will set it's own size
		if(m_iPaintOffsetOverrideX == -1 || m_iPaintOffsetOverrideY == -1)
		{
			SetSize(m_iWidth, m_iHeight);
		}

		if(bCalculatePositionalHashCode)
		{
			CalculatePositionalHashCode();
		}
	}

	void FFQuantityItem::CalculatePositionalHashCode()
	{
        int iPositionalHashCode = 17;

		FFQuantityHelper::CombineHash(iPositionalHashCode, m_iPaintOffsetX);
		FFQuantityHelper::CombineHash(iPositionalHashCode, m_iPaintOffsetY);
		FFQuantityHelper::CombineHash(iPositionalHashCode, m_iWidth);
		FFQuantityHelper::CombineHash(iPositionalHashCode, m_iHeight);

		m_iPositionalHashCode = iPositionalHashCode;
	}

	int FFQuantityItem::GetPositionalHashCode()
	{
		return m_iPositionalHashCode;
	}

	void FFQuantityItem::RecalculateAmountFont()
	{
		HFont* fontFamily
			= m_bAmountFontTahoma
				? m_hfQuantityItemTahomaText
				: m_hfQuantityItemText;

		HFont font
			= GetFont(
				fontFamily,
				m_iSizeAmount,
				m_bAmountFontShadow);

		m_hfAmount = font;
	}

	void FFQuantityItem::RecalculateLabelFont()
	{
		HFont* fontFamily
			= m_bLabelFontTahoma
				? m_hfQuantityItemTahomaText
				: m_hfQuantityItemText;

		HFont font
			= GetFont(
				fontFamily,
				m_iSizeLabel,
				m_bLabelFontShadow);

		m_hfLabel = font;
	}

	void FFQuantityItem::RecalculateIconFont()
	{
		HFont* fontFamily
			= m_bIconFontGlyph
				? m_hfQuantityItemGlyph
				: m_hfQuantityItemIcon;

		HFont font
			= GetFont(
				fontFamily,
				m_iSizeIcon,
				m_bIconFontShadow);

		m_hfIcon = font;
	}

	void FFQuantityItem::RecalculateIconPosition()
	{
		CalculateTextPositionOffset(m_iIconAnchorPositionX, m_iIconAnchorPositionY, m_iIconAnchorPosition);
		CalculateTextAlignmentOffset(m_iIconAlignmentOffsetX, m_iIconAlignmentOffsetY, m_iIconWidth, m_iIconHeight, m_iIconAlignHoriz, m_iIconAlignVert, m_hfIcon, m_wszIcon);

		m_iIconPosX = m_iIconAnchorPositionX + m_iIconAlignmentOffsetX + m_iIconPositionOffsetX * m_flScaleX;
		m_iIconPosY = m_iIconAnchorPositionY + m_iIconAlignmentOffsetY + m_iIconPositionOffsetY * m_flScaleY;
	}

	void FFQuantityItem::RecalculateLabelPosition()
	{
		CalculateTextPositionOffset(m_iLabelAnchorPositionX, m_iLabelAnchorPositionY, m_iLabelAnchorPosition);
		CalculateTextAlignmentOffset(m_iLabelAlignmentOffsetX, m_iLabelAlignmentOffsetY, m_iLabelWidth, m_iLabelHeight, m_iLabelAlignHoriz, m_iLabelAlignVert, m_hfLabel, m_wszLabel);

		m_iLabelPosX = m_iLabelAnchorPositionX + m_iLabelAlignmentOffsetX + m_iLabelPositionOffsetX * m_flScaleX;
		m_iLabelPosY = m_iLabelAnchorPositionY + m_iLabelAlignmentOffsetY + m_iLabelPositionOffsetY * m_flScaleY;
	}

	void FFQuantityItem::RecalculateAmountMaxDisplay()
	{
	    if (m_bShowAmountMax)
		{
			swprintf(m_wszAmountMaxDisplay, L"%ls/%ls", m_wszAmountMax, m_wszAmountMax);
		}
		else
		{
			swprintf(m_wszAmountMaxDisplay, L"100%%");
		}
	}

	void FFQuantityItem::RecalculateAmountMaxPosition()
	{
		CalculateTextPositionOffset(m_iAmountMaxAnchorPositionX, m_iAmountMaxAnchorPositionY, m_iAmountAnchorPosition);
		CalculateTextAlignmentOffset(m_iAmountMaxAlignmentOffsetX, m_iAmountMaxAlignmentOffsetY, m_iAmountMaxWidth, m_iAmountMaxHeight, m_iAmountAlignHoriz, m_iAmountAlignVert, m_hfAmount, m_wszAmountMaxDisplay);

		m_iAmountMaxPosX = m_iAmountMaxAnchorPositionX + m_iAmountMaxAlignmentOffsetX + m_iAmountPositionOffsetX * m_flScaleX;
		m_iAmountMaxPosY = m_iAmountMaxAnchorPositionY + m_iAmountMaxAlignmentOffsetY + m_iAmountPositionOffsetY * m_flScaleY;
	}

	void FFQuantityItem::RecalculateAmountPosition()
	{
		if (m_bShowAmountSticky)
		{
			surface()->GetTextSize(m_hfAmount, m_wszAmountDisplay, m_iAmountWidth, m_iAmountHeight);

			m_iAmountPosX = m_iAmountMaxPosX + (m_iAmountMaxWidth - m_iAmountWidth);
			m_iAmountPosY = m_iAmountMaxPosY;
		}
		else 
		{	 
			CalculateTextPositionOffset(m_iAmountAnchorPositionX, m_iAmountAnchorPositionY, m_iAmountAnchorPosition);
			CalculateTextAlignmentOffset(m_iAmountAlignmentOffsetX, m_iAmountAlignmentOffsetY, m_iAmountWidth, m_iAmountHeight, m_iAmountAlignHoriz, m_iAmountAlignVert, m_hfAmount, m_wszAmountDisplay);

			m_iAmountPosX = m_iAmountAnchorPositionX + m_iAmountAlignmentOffsetX + m_iAmountPositionOffsetX * m_flScaleX;
			m_iAmountPosY = m_iAmountAnchorPositionY + m_iAmountAlignmentOffsetY + m_iAmountPositionOffsetY * m_flScaleY;
		}
	}

	void FFQuantityItem::RecalculateQuantity( )
	{
		int iAmountPercent = (float)m_iAmount/(float)m_iMaxAmount * 100;

		if(m_bIntensityAmountScaled && m_iMaxAmount != 0)
		{
			m_ColorIntensityFaded = GetIntensityColor(iAmountPercent, 100, 2, 255, m_iIntensityRed, m_iIntensityOrange, m_iIntensityYellow, m_iIntensityGreen);
			m_ColorIntensityStepped = GetIntensityColor(iAmountPercent, 100, 1, 255, m_iIntensityRed, m_iIntensityOrange ,m_iIntensityYellow, m_iIntensityGreen);
		}
		else
		{
			m_ColorIntensityFaded = GetIntensityColor(m_iAmount, m_iMaxAmount, 2, 255, m_iIntensityRed, m_iIntensityOrange, m_iIntensityYellow, m_iIntensityGreen);
			m_ColorIntensityStepped = GetIntensityColor(m_iAmount, m_iMaxAmount, 1, 255, m_iIntensityRed, m_iIntensityOrange, m_iIntensityYellow, m_iIntensityGreen);
		}

		if(m_bShowAmountMax)
		{
			swprintf( m_wszAmountDisplay, L"%ls/%ls", m_wszAmount, m_wszAmountMax );
		}
		else if(m_iMaxAmount == 100)
		{
			swprintf( m_wszAmountDisplay, L"%ls%%", m_wszAmount );
		}
		else
		{
			swprintf( m_wszAmountDisplay, L"%i%%", iAmountPercent );
		}

		RecalculateAmountPosition();

		SetIntensityColor(m_ColorBarBorder, m_iBarBorderColorMode, m_clrBarBorderCustomColor);
		SetIntensityColor(m_ColorBarBackground, m_iBarBackgroundColorMode, m_clrBarBackgroundCustomColor);
		SetIntensityColor(m_ColorBar, m_iBarColorMode, m_clrBarCustomColor);
		SetIntensityColor(m_ColorIcon, m_iIconColorMode, m_clrIconCustomColor);
		SetIntensityColor(m_ColorLabel, m_iLabelColorMode, m_clrLabelCustomColor);
		SetIntensityColor(m_ColorAmount, m_iAmountColorMode, m_clrAmountCustomColor);

		m_iBarX0QuantityOffset = 0;
		m_iBarX1QuantityOffset = 0;
		m_iBarY0QuantityOffset = 0;
		m_iBarY1QuantityOffset = 0;

		switch (m_iBarOrientation)
		{
		case ORIENTATION_HORIZONTAL_INVERTED:
			m_iBarX0QuantityOffset = m_iBarWidth - (m_iBarWidth * m_iAmount/m_iMaxAmount);
			break;

		case ORIENTATION_VERTICAL:
			m_iBarY0QuantityOffset = m_iBarHeight - (m_iBarHeight * m_iAmount/m_iMaxAmount);
			break;

		case ORIENTATION_VERTICAL_INVERTED:
			m_iBarY1QuantityOffset = (m_iBarHeight * m_iAmount/m_iMaxAmount) - m_iBarHeight;
			break;

		case ORIENTATION_HORIZONTAL:
		default:
			m_iBarX1QuantityOffset = (m_iBarWidth * m_iAmount/m_iMaxAmount) - m_iBarWidth;
			break;
		}
	}

	void FFQuantityItem::SetIntensityColor( Color &color, int iColorMode, Color &alphaColor )
	{
		if (iColorMode == ITEM_COLOR_MODE_FADED )
		{
			SetColor(color, m_ColorIntensityFaded, alphaColor);
		}
		else if(iColorMode == ITEM_COLOR_MODE_STEPPED )
		{
			SetColor(color, m_ColorIntensityStepped, alphaColor);
		}
	}

	void FFQuantityItem::RecalculateColor( Color &color, int iColorMode, Color &colorCustom )
	{
		Color rgbColor = GetRgbColor(iColorMode, colorCustom);

		SetColor(color, rgbColor, colorCustom);
	}

	Color FFQuantityItem::GetRgbColor( int iColorMode, Color &colorCustom )
	{
		switch (iColorMode)
		{
		case ITEM_COLOR_MODE_STEPPED:
			return m_ColorIntensityStepped;

		case ITEM_COLOR_MODE_FADED:
			return m_ColorIntensityFaded;

		case ITEM_COLOR_MODE_TEAM:
			return m_clrTeam;

		case ITEM_COLOR_MODE_CUSTOM:
		default:
			return colorCustom;
		}
	}

	void FFQuantityItem::SetColor( Color &color, Color &rgbColor, Color &alphaColor )
	{
		color.SetColor(
			rgbColor.r(),
			rgbColor.g(),
			rgbColor.b(),
			alphaColor.a());
	}

	void FFQuantityItem::CalculateTextPositionOffset(
		int &iX,
		int &iY,
		int iAnchorPos )
	{
		int iAlignHoriz, iAlignVert;

		FFQuantityHelper::ConvertToAlignment(
			iAnchorPos,
			iAlignHoriz,
			iAlignVert);
		
		int iWide = m_iBarWidth * m_flScaleX;
		int iTall = m_iBarHeight * m_flScaleY;

		FFQuantityHelper::CalculatePositionOffset(
			iX, 
			iY, 
			iWide, 
			iTall,
			iAlignHoriz,
			iAlignVert);
	}

	void FFQuantityItem::CalculateTextAlignmentOffset(
		int &iX,
		int &iY,
		int &iWide,
		int &iTall,
		int iAlignHoriz,
		int iAlignVert,
		HFont hfFont,
		wchar_t* wszString )
	{
		surface()->GetTextSize(hfFont, wszString, iWide, iTall);
		
		FFQuantityHelper::CalculatePositionOffset(
			iX, 
			iY, 
			iWide, 
			iTall,
			iAlignHoriz,
			iAlignVert);

		iX = iX * -1;
		iY = iY * -1;
	}
}