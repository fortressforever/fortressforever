#include "cbase.h"
#include "ff_quantitypanel.h"

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>


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

		SetPaintBackgroundType(2);
		SetBorder(pScheme->GetBorder("ScoreBoardItemBorder"));
		SetBgColor(pScheme->GetColor( "HUD_BG_Default", Color(255,0,0,255)));

		m_hfHeaderText[0] = pScheme->GetFont( "QuantityPanelHeader", true );
		m_hfHeaderText[1] = pScheme->GetFont( "QuantityPanelHeaderShadow", true );
		m_hfHeaderText[2] = pScheme->GetFont( "QuantityPanelHeader", false );
		m_hfHeaderIcon[0] = pScheme->GetFont( "QuantityPanelHeaderIcon", true );
		m_hfHeaderIcon[1] = pScheme->GetFont( "QuantityPanelHeaderIconShadow", true );
		m_hfHeaderIcon[2] = pScheme->GetFont( "QuantityPanelHeaderIcon", false );

		Panel::ApplySchemeSettings( pScheme );
	}

	void FFQuantityPanel::OnChildDimentionsChanged (KeyValues* data)
	{
		int id = data->GetInt("id",-1);

		if(id != -1)
		{
			//if the id is valid (which it should be if it got here...)
			//even if panels get moved the id should be valid
			//if ids are the same then they will paint over eachother
			if(id < m_iQBars) 
			{
				m_bUpdateRecieved[id] = true;

				//set the wait for update flag
				if(!m_bCheckUpdates)
					m_bCheckUpdates = true;
			}
		}
		// else singular update (e.g not triggered from resolution change)
		// will this ever need to happen if sizes are consistent
		// when updating I think I'll let the child recalculate with each variable
		// but leave it to the parent to ensure all are repositioned after the updates are done
	}

	CHudQuantityBar* FFQuantityPanel::AddChild(const char *pElementName)
	{
		CHudQuantityBar* newQBar = new CHudQuantityBar(this, pElementName, m_iQBars); 
		m_QBars[m_iQBars++] = newQBar;
		return newQBar;

	}

	void FFQuantityPanel::OnTick()
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
			RecalculateHeaderTextDimentions();
			RecalculateHeaderIconDimentions();
		}

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
			UpdateQuantityBarPositions();
		}
	}
	
	void FFQuantityPanel::UpdateQuantityBarPositions() 
	{
		int iRow = 0;
		int iColumn = 0;
		
		int m_iRowHeight[] = {0,0,0,0,0,0};
		int m_iColumnWidth[] = {0,0,0,0,0,0};
		int m_iColumnOffset[] = {0,0,0,0,0,0};
		int m_iRowOffset[] = {0,0,0,0,0,0};	

		m_iNumQuantityBarColumns = 1;

		for(int i = 0; i < m_iQBars; ++i)
		{
			int iWidth, iHeight,iBarOffsetX, iBarOffsetY;
			m_QBars[i]->GetDimentions(iWidth, iHeight,iBarOffsetX, iBarOffsetY);

			if(iBarOffsetX > m_iColumnOffset[iColumn])
				m_iColumnOffset[iColumn] = iBarOffsetX;
			if(iWidth > m_iColumnWidth[iColumn])
				m_iColumnWidth[iColumn] = iWidth;

			if(iBarOffsetY > m_iRowOffset[iRow])
				m_iRowOffset[iRow] = iBarOffsetY;
			if(iHeight > m_iRowHeight[iRow])
				m_iRowHeight[iRow] = iHeight;

			++iColumn;
			if(iColumn >= m_iNumQuantityBarColumns)	
			{
				iColumn = 0;
				++iRow;
			}
		}

		//if(m_iColumnWidth == 0)

		iRow = 0;
		iColumn = 0;
		
		int iColumnWidths = 0;
		int iRowHeights = 0;

		for(int i = 0; i < m_iQBars; ++i)
		{
			int iPosX, iPosY;
	
			iPosX = m_iQuantityBarPositionX + iColumnWidths + m_iColumnOffset[iColumn] + (iColumn) * m_iQuantityBarSpacingX * m_flScale;
			iPosY = m_iQuantityBarPositionY + iRowHeights + m_iRowOffset[iRow] + (iRow) * m_iQuantityBarSpacingY * m_flScale;

			m_QBars[i]->SetPos( iPosX, iPosY );
					
			if(i == m_iQBars -1)
			//if its the last one then we can get the position, coloumn width & row height
			//so we can calculate the overall size of the quantity panel for automatic resizing!
			{
				for(;iColumn < m_iNumQuantityBarColumns;)
				//make sure its calculated from the last column
					iColumnWidths += m_iColumnWidth[iColumn++];
				//already on last row
				iRowHeights += m_iRowHeight[iRow++];

				int iX1 = m_iQuantityBarPositionX + iColumnWidths + iColumn * m_iQuantityBarSpacingX * m_flScale;
				int iY1 = m_iQuantityBarPositionY + iRowHeights + iRow * m_iQuantityBarSpacingY * m_flScale;

				if( (m_iHeaderTextX + m_iHeaderTextWidth) > iX1 )
					iX1 = m_iHeaderTextX + m_iHeaderTextWidth;
				if( (m_iHeaderIconX + m_iHeaderIconWidth) > iX1 )
					iX1 = m_iHeaderIconX + m_iHeaderIconWidth;

				if( (m_iHeaderTextY + m_iHeaderTextHeight) > iY1 )
					iY1 = m_iHeaderTextY + m_iHeaderTextHeight;
				if( (m_iHeaderIconY + m_iHeaderIconHeight) > iY1 )
					iY1 = m_iHeaderIconY + m_iHeaderIconHeight;

				SetSize( iX1 * m_flScale, iY1 * m_flScale );
			}
			else
			{
				iColumnWidths += m_iColumnWidth[iColumn++];
				if(iColumn >= m_iNumQuantityBarColumns)	
				{
					iColumnWidths = 0;
					iColumn = 0;
					iRowHeights += m_iRowHeight[iRow++];
				}
			}
		}
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

		Panel::Paint();
	}

	void FFQuantityPanel::SetHeaderText(wchar_t *newHeaderText) { wcscpy( m_wszHeaderText, newHeaderText ); RecalculateHeaderTextDimentions(); }
	void FFQuantityPanel::SetHeaderIconChar(char *newHeaderIconChar)
	{
		char szIconChar[5];
		Q_snprintf( szIconChar, 2, "%s%", newHeaderIconChar );
		vgui::localize()->ConvertANSIToUnicode( szIconChar, m_wszHeaderIcon, sizeof( m_wszHeaderIcon ) );

		RecalculateHeaderIconDimentions();
	}

	void FFQuantityPanel::SetHeaderTextColor( Color newHeaderTextColor ) { m_ColorHeaderText = newHeaderTextColor; }
	void FFQuantityPanel::SetHeaderIconColor( Color newHeaderIconColor ) { m_ColorHeaderIcon = newHeaderIconColor; }

	void FFQuantityPanel::SetHeaderTextPosition( int iPositionX, int iPositionY ) { m_iHeaderTextX = iPositionX; m_iHeaderTextY = iPositionY; }
	void FFQuantityPanel::SetHeaderIconPosition( int iPositionX, int iPositionY ) { m_iHeaderIconX = iPositionX; m_iHeaderIconY = iPositionY; }

	void FFQuantityPanel::SetHeaderTextShadow(bool bHasShadow) { m_bHeaderTextShadow = bHasShadow; }
	void FFQuantityPanel::SetHeaderIconShadow(bool bHasShadow) { m_bHeaderIconShadow = bHasShadow; }

	void FFQuantityPanel::RecalculateHeaderTextDimentions()
	{
		vgui::surface()->GetTextSize(m_hfHeaderText[2], m_wszHeaderText, m_iHeaderTextWidth, m_iHeaderTextHeight);
	}

	void FFQuantityPanel::RecalculateHeaderIconDimentions()
	{
		vgui::surface()->GetTextSize(m_hfHeaderIcon[2], m_wszHeaderIcon, m_iHeaderIconWidth, m_iHeaderIconHeight);

	}
}