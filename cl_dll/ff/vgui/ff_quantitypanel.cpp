#include "cbase.h"
#include "ff_quantitypanel.h"

#include "c_ff_player.h"
#include "c_playerresource.h"
#include "ff_utils.h"

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

extern C_PlayerResource *g_PR;

namespace vgui
{
	void FFQuantityPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		m_ColorHeaderText = pScheme->GetColor( "HUD_Tone_Bright", Color(255,0,0,255) );
		m_ColorHeaderIcon = m_ColorHeaderText;
		m_ColorText = pScheme->GetColor( "HUD_Tone_Default", Color(255,0,0,255) );
		
		SetPaintBackgroundEnabled(true);
		SetPaintBorderEnabled(true);
		SetPaintEnabled(true);

		SetBorder(pScheme->GetBorder("ScoreBoardItemBorder"));

		
		vgui::HScheme QuantityBarScheme = vgui::scheme()->LoadSchemeFromFile("resource/QuantityPanelScheme.res", "QuantityPanelScheme");
		vgui::IScheme *qbScheme = vgui::scheme()->GetIScheme(QuantityBarScheme);

		m_hfHeaderText[0] = qbScheme->GetFont( "QuantityPanelHeader", true );
		m_hfHeaderText[1] = qbScheme->GetFont( "QuantityPanelHeaderShadow", true );
		m_hfHeaderText[2] = qbScheme->GetFont( "QuantityPanelHeader", false );
		m_hfHeaderIcon[0] = qbScheme->GetFont( "QuantityPanelHeaderIcon", true );
		m_hfHeaderIcon[1] = qbScheme->GetFont( "QuantityPanelHeaderIconShadow", true );
		m_hfHeaderIcon[2] = qbScheme->GetFont( "QuantityPanelHeaderIcon", false );

		BaseClass::ApplySchemeSettings( pScheme );
	}

	void FFQuantityPanel::PaintBackground() 
	{
		C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

		SetPaintBackgroundType(2);
		SetBgColor(Color( 
			g_PR->GetTeamColor( pPlayer->GetTeamNumber() ).r(),
			g_PR->GetTeamColor( pPlayer->GetTeamNumber() ).g(),
			g_PR->GetTeamColor( pPlayer->GetTeamNumber() ).b(),
			175
			));

		BaseClass::PaintBackground(); 
	}

	void FFQuantityPanel::Paint() 
	{
		//Paint Header Text
		vgui::surface()->DrawSetTextFont( m_hfHeaderText[m_bHeaderTextShadow] );
		vgui::surface()->DrawSetTextColor( m_ColorHeaderText.r(), m_ColorHeaderText.g(), m_ColorHeaderText.b(), m_ColorHeaderText.a() );
		vgui::surface()->DrawSetTextPos( m_iHeaderTextX * m_flScale, m_iHeaderTextY * m_flScale );
		vgui::surface()->DrawUnicodeString( m_wszHeaderText );

		//Paint Header Icon
		vgui::surface()->DrawSetTextFont( m_hfHeaderIcon[m_bHeaderIconShadow] );
		vgui::surface()->DrawSetTextColor( m_ColorHeaderIcon.r(), m_ColorHeaderIcon.g(), m_ColorHeaderIcon.b(), m_ColorHeaderIcon.a() );
		vgui::surface()->DrawSetTextPos( m_iHeaderIconX * m_flScale, m_iHeaderIconY * m_flScale );
		vgui::surface()->DrawUnicodeString( m_wszHeaderIcon );

		BaseClass::Paint();
	}

	void FFQuantityPanel::OnTick()
	{
		int alignOffsetX, alignOffsetY;

		switch(m_iHorizontalAlign)
		{
		case 1:
			alignOffsetX = - m_iWidth/2;
			break;
		case 2:
			alignOffsetX = - m_iWidth;
			break;
		case 0:
		default:
			alignOffsetX = 0;
		}

		switch(m_iVerticalAlign)
		{
		case 1:
			alignOffsetY = - m_iHeight/2;
			break;
		case 2:
			alignOffsetY = - m_iHeight;
			break;
		case 0:
		default:
			alignOffsetY = 0;
		}

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
			//recalculate text sizes
			vgui::surface()->GetTextSize(m_hfHeaderText[2], m_wszHeaderText, m_iHeaderTextWidth, m_iHeaderTextHeight);
			vgui::surface()->GetTextSize(m_hfHeaderIcon[2], m_wszHeaderIcon, m_iHeaderIconWidth, m_iHeaderIconHeight);
		}

		SetPos(m_flScaleX * m_iX + alignOffsetX,  m_flScaleY * m_iY + alignOffsetY);
		SetSize( m_iWidth, m_iHeight );

		if(m_bCheckUpdates)
		{
			for(int i = 0; i < m_iQBars; ++i)
			{
				if(!m_bUpdateRecieved[i])
					break;
			}

			//if we got here then they have all been updated
			for(int i = 0; i < m_iQBars; ++i)
			//clear all updateReceived flags
				m_bUpdateRecieved[i] = false;
			//clear check update flag
			m_bCheckUpdates = false;
			//update positions
			UpdateQBPositions();
		}

	}

	void FFQuantityPanel::UpdateQBPositions() 
	{
		int iRow = 0;
		int iColumn = 0;
		
		int m_iRowHeight[] = {0,0,0,0,0,0};
		int m_iColumnWidth[] = {0,0,0,0,0,0};
		int m_iColumnOffset[] = {0,0,0,0,0,0};
		int m_iRowOffset[] = {0,0,0,0,0,0};	

		for(int i = 0; i < m_iQBars; ++i)
		{
			int iWidth, iHeight,iBarOffsetX, iBarOffsetY;
			m_QBars[i]->GetPanelPositioningData(iWidth, iHeight,iBarOffsetX, iBarOffsetY);

			if(iBarOffsetX > m_iColumnOffset[iColumn])
				m_iColumnOffset[iColumn] = iBarOffsetX;
			if(iWidth > m_iColumnWidth[iColumn])
				m_iColumnWidth[iColumn] = iWidth;

			if(iBarOffsetY > m_iRowOffset[iRow])
				m_iRowOffset[iRow] = iBarOffsetY;
			if(iHeight > m_iRowHeight[iRow])
				m_iRowHeight[iRow] = iHeight;

			++iColumn;
			if(iColumn >= m_qb_iColumns)	
			{
				iColumn = 0;
				++iRow;
			}
		}

		iRow = 0;
		iColumn = 0;
		
		int iColumnWidths = 0;
		int iRowHeights = 0;

		for(int i = 0; i < m_iQBars; ++i)
		{
			int iPosX, iPosY;
	
			iPosX = m_qb_iPositionX + iColumnWidths + m_iColumnOffset[iColumn] + (iColumn+1) * m_qb_iBarMarginHorizontal * m_flScale;
			iPosY = m_qb_iPositionY + iRowHeights + m_iRowOffset[iRow] + (iRow+1) * m_qb_iBarMarginVertical * m_flScale;

			m_QBars[i]->SetPos( iPosX, iPosY );
					
			if(i == m_iQBars -1)
			//if its the last one then we can get the position, coloumn width & row height
			//so we can calculate the overall size of the quantity panel for automatic resizing!
			{
				for(;iColumn < ( m_iQBars < m_qb_iColumns ? m_iQBars : m_qb_iColumns );)
				//make sure its calculated from the last (used) column incase we started a new row
					iColumnWidths += m_iColumnWidth[iColumn++];
				//include the height of the row
				iRowHeights += m_iRowHeight[iRow++];

				int iX1 = m_qb_iPositionX + iColumnWidths + (iColumn+1) * m_qb_iBarMarginHorizontal * m_flScale;
				int iY1 = m_qb_iPositionY + iRowHeights + (iRow+1) * m_qb_iBarMarginVertical * m_flScale;

				//these are used to get equal padding on either side
				bool HeaderLeft = false;
				bool HeaderTop = false;
				
				if( m_iHeaderTextX < m_qb_iPositionX || m_iHeaderIconX < m_qb_iPositionX )
					HeaderLeft = true;
				if( m_iHeaderTextY < m_qb_iPositionY || m_iHeaderIconY < m_qb_iPositionY )
					HeaderTop = true;

				if( (m_iHeaderTextX + m_iHeaderTextWidth) > iX1 )
					iX1 = m_iHeaderTextX + m_iHeaderTextWidth 
					+ ( m_iHeaderIconX < m_iHeaderTextX ? m_iHeaderIconX : m_iHeaderTextX )*HeaderLeft
					+ m_qb_iPositionX * !HeaderLeft;
				if( (m_iHeaderIconX + m_iHeaderIconWidth) > iX1 )
					iX1 = m_iHeaderIconX + m_iHeaderIconWidth 
					+ ( m_iHeaderIconX < m_iHeaderTextX ? m_iHeaderIconX : m_iHeaderTextX )*HeaderLeft
					+ m_qb_iPositionX * !HeaderLeft;

				if( (m_iHeaderTextY + m_iHeaderTextHeight) > iY1 )
					iY1 = m_iHeaderTextY + m_iHeaderTextHeight 
					+ ( m_iHeaderIconY < m_iHeaderTextY ? m_iHeaderIconY : m_iHeaderTextY )*HeaderTop
					+ m_qb_iPositionY * !HeaderTop;
				if( (m_iHeaderIconY + m_iHeaderIconHeight) > iY1 )
					iY1 = m_iHeaderIconY + m_iHeaderIconHeight 
					+ ( m_iHeaderIconY < m_iHeaderTextY ? m_iHeaderIconY : m_iHeaderTextY )*HeaderTop
					+ m_qb_iPositionY * !HeaderTop;

				m_iWidth = iX1 * m_flScale; 
				m_iHeight = iY1 * m_flScale;
			}
			else
			{
				iColumnWidths += m_iColumnWidth[iColumn++];
				if(iColumn >= m_qb_iColumns)	
				{
					iColumnWidths = 0;
					iColumn = 0;
					iRowHeights += m_iRowHeight[iRow++];
				}
			}
		}
	}
	
	void FFQuantityPanel::SetHeaderText(wchar_t *newHeaderText) { 
		wcscpy( m_wszHeaderText, newHeaderText ); 
		//RecalculateHeaderTextDimentions
		vgui::surface()->GetTextSize(m_hfHeaderText[2], m_wszHeaderText, m_iHeaderTextWidth, m_iHeaderTextHeight);
	}
	void FFQuantityPanel::SetHeaderIconChar(char *newHeaderIconChar)
	{
		char szIconChar[5];
		Q_snprintf( szIconChar, 2, "%s%", newHeaderIconChar );
		vgui::localize()->ConvertANSIToUnicode( szIconChar, m_wszHeaderIcon, sizeof( m_wszHeaderIcon ) );

		//RecalculateHeaderIconDimentions
		vgui::surface()->GetTextSize(m_hfHeaderIcon[2], m_wszHeaderIcon, m_iHeaderIconWidth, m_iHeaderIconHeight);
	}

	void FFQuantityPanel::SetHeaderTextColor( Color newHeaderTextColor ) { m_ColorHeaderText = newHeaderTextColor; }
	void FFQuantityPanel::SetHeaderIconColor( Color newHeaderIconColor ) { m_ColorHeaderIcon = newHeaderIconColor; }
	void FFQuantityPanel::SetHeaderTextPosition( int iPositionX, int iPositionY ) { m_iHeaderTextX = iPositionX; m_iHeaderTextY = iPositionY; }
	void FFQuantityPanel::SetHeaderIconPosition( int iPositionX, int iPositionY ) { m_iHeaderIconX = iPositionX; m_iHeaderIconY = iPositionY; }
	void FFQuantityPanel::SetHeaderTextShadow(bool bHasShadow) { m_bHeaderTextShadow = bHasShadow; }
	void FFQuantityPanel::SetHeaderIconShadow(bool bHasShadow) { m_bHeaderIconShadow = bHasShadow; }
	
	void FFQuantityPanel::OnChildDimentionsChanged (KeyValues* data)
	{
		int id = data->GetInt("id",-1);

		if(id != -1 && id < m_iQBars)
		{
			m_bUpdateRecieved[id] = true;

			//set the wait for update flag
			if(!m_bCheckUpdates)
				m_bCheckUpdates = true;
		}
	}

	FFQuantityBar* FFQuantityPanel::AddChild(const char *pElementName)
	{
		FFQuantityBar* newQBar = new FFQuantityBar(this, pElementName, m_iQBars); 
		m_QBars[m_iQBars++] = newQBar;
		return newQBar;
	}
	
	void FFQuantityPanel::OnShowBarChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->ShowBar(m_qb_bShowBar);
	}
	void FFQuantityPanel::OnShowBarBackgroundChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->ShowBarBackground(m_qb_bShowBarBackground);
	}
	void FFQuantityPanel::OnShowBarBorderChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->ShowBarBorder(m_qb_bShowBarBorder);
	}
	void FFQuantityPanel::OnShowIconChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->ShowIcon(m_qb_bShowIcon);
	}
	void FFQuantityPanel::OnShowLabelChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->ShowLabel(m_qb_bShowLabel);
	}
	void FFQuantityPanel::OnShowAmountChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->ShowAmount(m_qb_bShowAmount);
	}
	
	void FFQuantityPanel::OnBarSizeChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetBarSize(m_qb_iBarWidth,m_qb_iBarHeight);
	}

	void FFQuantityPanel::OnBarBorderWidthChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetBarBorderWidth(m_qb_iBarBorderWidth);
	}
	void FFQuantityPanel::OnIntensityValuesChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			if(!m_QBars[i]->IsIntensityValuesFixed()) //so that we dont alter ones set in code! SG levels - Pipes (whatever/etc)
				m_QBars[i]->SetIntensityControl(m_qb_iIntensity_red,m_qb_iIntensity_orange,m_qb_iIntensity_yellow,m_qb_iIntensity_green);
	}
	
	void FFQuantityPanel::OnColorBarChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetBarColor(Color(m_qb_iColorBar_r,m_qb_iColorBar_g,m_qb_iColorBar_b,m_qb_iColorBar_a));
	}
	void FFQuantityPanel::OnColorBarBackgroundChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetBarBackgroundColor(Color(m_qb_iColorBarBackground_r,m_qb_iColorBar_g,m_qb_iColorBarBackground_b,m_qb_iColorBarBackground_a));
	}
	void FFQuantityPanel::OnColorBarBorderChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetBarBorderColor(Color(m_qb_iColorBarBorder_r,m_qb_iColorBarBorder_g,m_qb_iColorBarBorder_b,m_qb_iColorBarBorder_a));
	}
	
	void FFQuantityPanel::OnColorIconChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetIconColor(Color(m_qb_iColorIcon_r,m_qb_iColorIcon_g,m_qb_iColorIcon_b,m_qb_iColorIcon_a));
	}
	void FFQuantityPanel::OnColorLabelChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetLabelColor(Color(m_qb_iColorLabel_r,m_qb_iColorLabel_g,m_qb_iColorLabel_b,m_qb_iColorLabel_a));
	}
	void FFQuantityPanel::OnColorAmountChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetAmountColor(Color(m_qb_iColorAmount_r,m_qb_iColorAmount_g,m_qb_iColorAmount_b,m_qb_iColorAmount_a));
	}
	
	void FFQuantityPanel::OnColorModeBarChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetBarColorMode(m_qb_iColorModeBar);
	}
	void FFQuantityPanel::OnColorModeBarBackgroundChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetBarBackgroundColorMode(m_qb_iColorModeBarBackground);
	}
	void FFQuantityPanel::OnColorModeBarBorderChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetBarBorderColorMode(m_qb_iColorModeBarBorder);
	}
	void FFQuantityPanel::OnColorModeIconChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetIconColorMode(m_qb_iColorModeIcon);
	}
	void FFQuantityPanel::OnColorModeLabelChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetLabelColorMode(m_qb_iColorModeLabel);
	}
	void FFQuantityPanel::OnColorModeAmountChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetAmountColorMode(m_qb_iColorModeAmount);
	}

	void FFQuantityPanel::OnSizeLabelChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetLabelSize(m_qb_iSizeLabel);
	}

	void FFQuantityPanel::OnSizeIconChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetIconSize(m_qb_iSizeIcon);
	}
	void FFQuantityPanel::OnSizeAmountChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetAmountSize(m_qb_iSizeAmount);
	}

	void FFQuantityPanel::OnIconShadowChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetIconFontShadow(m_qb_bShadowIcon);
	}

	void FFQuantityPanel::OnLabelShadowChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetLabelFontShadow(m_qb_bShadowLabel);
	}
	
	void FFQuantityPanel::OnAmountShadowChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetAmountFontShadow(m_qb_bShadowAmount);
	}

	void FFQuantityPanel::OnIconOffsetChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetIconOffset(m_qb_iOffsetIconX, m_qb_iOffsetIconY);
	}
	void FFQuantityPanel::OnLabelOffsetChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetLabelOffset(m_qb_iOffsetLabelX, m_qb_iOffsetLabelY);
	}
	void FFQuantityPanel::OnAmountOffsetChanged(  )
	{
		for(int i = 0; i < m_iQBars; i++)
			m_QBars[i]->SetAmountOffset(m_qb_iOffsetAmountX, m_qb_iOffsetAmountY);
	}
}