#include "cbase.h"
#include "ff_quantitypanel.h"
#include "ff_hud_quantitybar.h"
#include <vgui_controls/Panel.h>
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

		m_hfHeaderText = pScheme->GetFont( "QuantityPanelHeader", IsProportional() );
		m_hfHeaderIcon = pScheme->GetFont( "QuantityPanelHeaderIcon", IsProportional() );
		m_hfText = pScheme->GetFont( "QuantityPanel", IsProportional() );

		Panel::ApplySchemeSettings( pScheme );
	}

	void FFQuantityPanel::OnTick()
	{
		// Get the screen width/height
		int iScreenWide, iScreenTall;
		vgui::surface()->GetScreenSize( iScreenWide, iScreenTall );

		// "map" screen res to 640/480
		float flScaleX = 1 / (640.0f / iScreenWide);
		float flScaleY = 1 / (480.0f / iScreenTall);

		if( m_flScale != (flScaleX<flScaleY ? flScaleX : flScaleY))
		// if scale has changed (if user changes resolution)
		{
			m_flScale = (flScaleX<flScaleY ? flScaleX : flScaleY);

			//update child quantity panel positions
			//depends if the children have already updated - need to sort that.
			//maybe use messaging to say "I have updated!" wait for all to have reported, set a flag for update
			/*
			int iRow = 0;
			int iColumn = 0;

			for(int i = 0; i < GetChildCount(); ++i)
			{
				CHudQuantityBar* qbPtr = dynamic_cast<CHudQuantityBar*>(GetChild(i));
				if (qbPtr)
				// If child is a quantity bar
				{
					if(iRow == 0)
						if(iColumn == 0)
						//very first item
							qbPtr->SetPos(
							m_iQuantityBarPositionX + m_iColumnOffset[iColumn],
							m_iQuantityBarPositionY + m_iRowOffset[iRow]
						);
						else
							qbPtr->SetPos(
							m_iQuantityBarPositionX + m_iColumnOffset[iColumn] + m_iColumnWidth[iColumn - 1] + iColumn * m_iQuantityBarSpacingX,
							m_iQuantityBarPositionY + m_iRowOffset[iRow]
						);
					else
						if(iColumn == 0)
							qbPtr->SetPos(
							m_iQuantityBarPositionX + m_iColumnOffset[iColumn],
							m_iQuantityBarPositionY + m_iRowOffset[iRow] + m_iRowWidth[iColumn - 1] + iRow * m_iQuantityBarSpacingY
						);
						else
							qbPtr->SetPos(
							m_iQuantityBarPositionX + m_iColumnOffset[iColumn] + m_iColumnWidth[iColumn - 1] + iColumn * m_iQuantityBarSpacingX,
							m_iQuantityBarPositionY + m_iRowOffset[iRow] + m_iRowWidth[iColumn - 1] + iRow * m_iQuantityBarSpacingY
						);

					//m_qbShells->SetPosition(iLeft + iOffsetX, iTop + iOffsetY);
					++iColumn;
					if(iColumn >= m_iNumQuantityBarColumns)	
					{
						iColumn = 0;
						++iRow;
					}
				}
			}
			*/
		}
	}
	
	void FFQuantityPanel::Paint() 
	{
		//Paint Header Text
		vgui::surface()->DrawSetTextFont( m_hfHeaderText );
		vgui::surface()->DrawSetTextColor( m_ColorHeaderText.r(), m_ColorHeaderText.g(), m_ColorHeaderText.b(), m_ColorHeaderText.a() );
		vgui::surface()->DrawSetTextPos( m_iHeaderTextX * m_flScale, m_iHeaderTextY * m_flScale );
		vgui::surface()->DrawUnicodeString( m_wszHeaderText );

		//Paint Header Icon
		vgui::surface()->DrawSetTextFont( m_hfHeaderIcon );
		vgui::surface()->DrawSetTextColor( m_ColorHeaderIcon.r(), m_ColorHeaderIcon.g(), m_ColorHeaderIcon.b(), m_ColorHeaderIcon.a() );
		vgui::surface()->DrawSetTextPos( m_iHeaderIconX * m_flScale, m_iHeaderIconY * m_flScale );
		vgui::surface()->DrawUnicodeString( m_wszHeaderIcon );

		Panel::Paint();
	}

	void FFQuantityPanel::SetHeaderText(wchar_t *newHeaderText) { wcscpy( m_wszHeaderText, newHeaderText ); }
	void FFQuantityPanel::SetHeaderIconChar(char *newHeaderIconChar)
	{
		char szIconChar[5];
		Q_snprintf( szIconChar, 2, "%s%", newHeaderIconChar );
		vgui::localize()->ConvertANSIToUnicode( szIconChar, m_wszHeaderIcon, sizeof( m_wszHeaderIcon ) );
	}

	void FFQuantityPanel::SetHeaderTextColor( Color newHeaderTextColor ) { m_ColorHeaderText = newHeaderTextColor; }
	void FFQuantityPanel::SetHeaderIconColor( Color newHeaderIconColor ) { m_ColorHeaderIcon = newHeaderIconColor; }

	void FFQuantityPanel::SetHeaderTextPosition( int iPositionX, int iPositionY ) { m_iHeaderTextX = iPositionX; m_iHeaderTextY = iPositionY; }
	void FFQuantityPanel::SetHeaderIconPosition( int iPositionX, int iPositionY ) { m_iHeaderIconX = iPositionX; m_iHeaderIconY = iPositionY; }

	void FFQuantityPanel::SetHeaderTextFont(vgui::HFont newHeaderTextFont) { m_hfHeaderText = newHeaderTextFont; }
	void FFQuantityPanel::SetHeaderIconFont(vgui::HFont newHeaderIconFont) { m_hfHeaderIcon = newHeaderIconFont; }
}