/********************************************************************
	created:	2010/08
	filename: 	cl_dll\ff\ff_hud_quantitypanel.cpp
	file path:	cl_dll\ff
	file base:	ff_hud_quantitypanel
	file ext:	cpp
	author:		Elmo
	
	purpose:	Customisable Quanitity Panel for the HUD
*********************************************************************/

#include "cbase.h"
#include "ff_quantitypanel.h"

#include "c_ff_player.h" //required to cast base player
#include "ff_utils.h"

#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "c_playerresource.h"
extern C_PlayerResource *g_PR;

//#include "ff_customhudoptions_assignpresets.h"
//extern CFFCustomHudAssignPresets *g_AP;

extern ConVar cl_teamcolourhud;

namespace vgui
{
	FFQuantityPanel::FFQuantityPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
		m_flScale = 1.0f;
		m_flScaleX = 1.0f;
		m_flScaleY = 1.0f;
		m_iQBars = 0;
		SetHeaderTextPosition(30,10);
		SetHeaderIconPosition(10,5);

		//these values are irrelevant and get overridden by cvars
		
		//if the child class should take precidence over general
		//buildstate_sg_x over buildstate_x
		m_bChildOverride = false;

		m_iX = 640;
		m_iY = 300;
		m_iWidth = 0;
		m_iHeight = 0;
		m_iHorizontalAlign = 2;
		m_iVerticalAlign = 0;

		m_iHeaderTextSize = 9;
		m_iHeaderIconSize = 9;
		m_bShowHeaderText = true;
		m_bShowHeaderIcon = true;

		m_qb_iBarMarginHorizontal = 5;
		m_qb_iBarMarginVertical = 5;
		
		m_qb_iPositionX = 10;
		m_qb_iPositionY = 20;
		m_qb_iColumns = 1;

		/*
			rest is -1 so that it updates using cvar values
			had a weird case where quantity bar colormode default was 1
			this was 2 and cvar was 2 - It didn't detect a change and so 
			didn't update the colormode thus maintaining a colourmode of 1 

			obviously true false can't be helped... just making sure they're the same as quanitty bar defaults!
		*/
		
		m_qb_iSizeIcon = -1;
		m_qb_iSizeLabel = -1;
		m_qb_iSizeAmount = -1;

		m_qb_iIntensity_red = -1;
		m_qb_iIntensity_orange = -1;
		m_qb_iIntensity_yellow = -1;
		m_qb_iIntensity_green = -1;

		m_qb_bShowBar = true;
		m_qb_bShowBarBackground = true;
		m_qb_bShowBarBorder = true;
		m_qb_bShowIcon = true;
		m_qb_bShowLabel = true;
		m_qb_bShowAmount = true;

		m_qb_iBarWidth = -1;
		m_qb_iBarHeight = -1;
		m_qb_iBarBorderWidth = -1;

		m_qb_iColorBar_r = -1;
		m_qb_iColorBar_g = -1;
		m_qb_iColorBar_b = -1;
		m_qb_iColorBar_a = -1;
		m_qb_iColorBarBackground_r = -1;
		m_qb_iColorBarBackground_g = -1;
		m_qb_iColorBarBackground_b = -1;
		m_qb_iColorBarBackground_a = -1;
		m_qb_iColorBarBorder_r = -1;
		m_qb_iColorBarBorder_g = -1;
		m_qb_iColorBarBorder_b = -1;
		m_qb_iColorBarBorder_a = -1;
		m_qb_iColorIcon_r = -1;
		m_qb_iColorIcon_g = -1;
		m_qb_iColorIcon_b = -1;
		m_qb_iColorIcon_a = -1;
		m_qb_iColorLabel_r = -1; 
		m_qb_iColorLabel_g = -1;
		m_qb_iColorLabel_b = -1;
		m_qb_iColorLabel_a = -1;
		m_qb_iColorAmount_r = -1; 
		m_qb_iColorAmount_g = -1; 
		m_qb_iColorAmount_b = -1;
		m_qb_iColorAmount_a = -1;

		m_qb_bShadowIcon = false;
		m_qb_bShadowLabel = false;
		m_qb_bShadowAmount = false; 

		m_qb_iColorModeBar = -1;
		m_qb_iColorModeBarBackground = -1;
		m_qb_iColorModeBarBorder = -1;
		m_qb_iColorModeIcon = -1;
		m_qb_iColorModeLabel = -1;
		m_qb_iColorModeAmount = -1; 
		m_qb_iColorModeIdent = -1;

		m_qb_iOffsetBarX = -1;
		m_qb_iOffsetBarY = -1;
		m_qb_iOffsetIconX = -1;
		m_qb_iOffsetIconY = -1;
		m_qb_iOffsetLabelX = -1;
		m_qb_iOffsetLabelY = -1;
		m_qb_iOffsetAmountX = -1;
		m_qb_iOffsetAmountY = -1;
		m_qb_iOffsetIdentX = -1;
		m_qb_iOffsetIdentY = -1;
	}

	void FFQuantityPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		vgui::HScheme QuantityBarScheme = vgui::scheme()->LoadSchemeFromFile("resource/QuantityPanelScheme.res", "QuantityPanelScheme");
		vgui::IScheme *qbScheme = vgui::scheme()->GetIScheme(QuantityBarScheme);

		m_ColorBackground = GetSchemeColor("Background", qbScheme);
		m_iTeamColourAlpha = GetSchemeColor("TeamBackgroundAlpha", qbScheme).a();
		
		m_ColorHeaderText = GetSchemeColor("HeaderText", qbScheme);
		m_ColorHeaderIcon = GetSchemeColor("HeaderIcon", qbScheme);
		m_ColorText = GetSchemeColor("Text", qbScheme);
		m_ColorText = GetSchemeColor("Border", qbScheme);
				
		SetBgColor(m_ColorBackground);
		SetPaintBackgroundEnabled(true);
		SetPaintBackgroundType(2);
		SetPaintBorderEnabled(false);
		SetPaintEnabled(true);


		for(int i = 0; i < QUANTITYPANELFONTSIZES; ++i)
		{
			m_hfHeaderText[i*3] = qbScheme->GetFont( VarArgs("QuantityPanelHeader%d",i), true );
			m_hfHeaderText[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderShadow%d",i), true );
			m_hfHeaderText[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityPanelHeader%d",i), false );
			m_hfHeaderIcon[i*3] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderIcon%d",i), true );
			m_hfHeaderIcon[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderIconShadow%d",i), true );
			m_hfHeaderIcon[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderIcon%d",i), false );
		}

		m_hfText = qbScheme->GetFont( "QuantityPanel", IsProportional() );
	
		UpdateQBPositions();
		BaseClass::ApplySchemeSettings( pScheme );
	}
	void FFQuantityPanel::Paint() 
	{
		if(m_bShowHeaderText)
		{
			//Paint Header Text
			vgui::surface()->DrawSetTextFont( m_hfHeaderText[m_iHeaderIconSize * 3 + (m_bHeaderTextShadow ? 1 : 0)] );
			vgui::surface()->DrawSetTextColor( m_ColorHeaderText.r(), m_ColorHeaderText.g(), m_ColorHeaderText.b(), m_ColorHeaderText.a() );
			vgui::surface()->DrawSetTextPos( m_iHeaderTextX * m_flScale, m_iHeaderTextY * m_flScale );
			vgui::surface()->DrawUnicodeString( m_wszHeaderText );
		}

		if(m_bShowHeaderIcon)
		{
			//Paint Header Icon
			vgui::surface()->DrawSetTextFont( m_hfHeaderIcon[m_iHeaderIconSize * 3 + (m_bHeaderIconShadow ? 1 : 0)] );
			vgui::surface()->DrawSetTextColor( m_ColorHeaderIcon.r(), m_ColorHeaderIcon.g(), m_ColorHeaderIcon.b(), m_ColorHeaderIcon.a() );
			vgui::surface()->DrawSetTextPos( m_iHeaderIconX * m_flScale, m_iHeaderIconY * m_flScale );
			vgui::surface()->DrawUnicodeString( m_wszHeaderIcon );
		}

		SetPaintBackgroundEnabled(true);
		SetPaintBackgroundType(2);
		SetPaintBorderEnabled(false);
		SetPaintEnabled(true);

		BaseClass::Paint();
	}
	/*
	void FFQuantityPanel::AddPanelToHudOptions(const char* szSelf, const char* szParent)
	{
		Q_strncpy(m_szSelf, szSelf, 127);
		Q_strncpy(m_szParent, szParent, 127);
		m_bAddToHud = true;
	}*/

	void FFQuantityPanel::OnTick()
	{
		/*
		if(m_bAddToHud && !m_bAddToHudSent)
		{
			//put the global check separately just incase it uses more resources
			if(g_AP != NULL)
			{
				KeyValues *kv = new KeyValues("AddQuantityPanel");
				kv->SetString("self", m_szSelf);
				kv->SetString("parent", m_szParent);
				PostMessage(g_AP, kv);
				m_bAddToHudSent = true;
			}
		}*/
		
		if(!m_bDraw)
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
		}
		else
		{
			SetPaintEnabled(true);
			SetPaintBackgroundEnabled(true);
		}

		if(cl_teamcolourhud.GetBool())
		{
			C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

			if( !(FF_IsPlayerSpec( pPlayer ) || !FF_HasPlayerPickedClass( pPlayer )) )
			{
				Color teamColor = g_PR->GetTeamColor( pPlayer->GetTeamNumber() );
				m_ColorTeamBackground = Color(teamColor.r(), teamColor.g(), teamColor.b(), m_iTeamColourAlpha);
				SetBgColor(m_ColorTeamBackground);
			}
		}
		else
			SetBgColor(m_ColorBackground);

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
			vgui::surface()->GetTextSize(m_hfHeaderText[m_iHeaderTextSize*3 + 2], m_wszHeaderText, m_iHeaderTextWidth, m_iHeaderTextHeight);
			vgui::surface()->GetTextSize(m_hfHeaderIcon[m_iHeaderIconSize*3 + 2], m_wszHeaderIcon, m_iHeaderIconWidth, m_iHeaderIconHeight);
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
					
			if(i != m_iQBars -1)
				//if not the last one
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

		//now we can calculate the overall size of the quantity panel for automatic resizing!
		for(;iColumn < ( m_iQBars < m_qb_iColumns ? m_iQBars : m_qb_iColumns );)
		//make sure its calculated from the last (used) column incase we started a new row
			iColumnWidths += m_iColumnWidth[iColumn++];
		//include the height of the row
		iRowHeights += m_iRowHeight[iRow++];

		int iX1 = m_qb_iPositionX + iColumnWidths + (iColumn+1) * m_qb_iBarMarginHorizontal * m_flScale;
		int iY1 = m_qb_iPositionY + iRowHeights + (iRow+1) * m_qb_iBarMarginVertical * m_flScale;

		//these are used to get equal padding on either side
		bool HeaderTextLeft = false;
		bool HeaderTextTop = false;
		bool HeaderIconLeft = false;
		bool HeaderIconTop = false;
		
		if( m_iHeaderTextX < m_qb_iPositionX || m_iHeaderIconX < m_qb_iPositionX )
			if(m_iHeaderTextX < m_iHeaderIconX)
				HeaderTextLeft = true;
			else
				HeaderIconLeft = true;
		if( m_iHeaderTextY < m_qb_iPositionY || m_iHeaderIconY < m_qb_iPositionY )
			if(m_iHeaderTextY < m_iHeaderIconY)
				HeaderTextTop = true;
			else
				HeaderIconTop = true;
		
		if( (m_iHeaderTextX + m_iHeaderTextWidth) > iX1 || (m_iHeaderIconX + m_iHeaderIconWidth) > iX1 )
		//if header text or header icon is furthest right
		{
			if((m_iHeaderTextX + m_iHeaderTextWidth) > (m_iHeaderIconX + m_iHeaderIconWidth))
			// if header text is furthest right
			{
				if(HeaderTextLeft)
				//if header text is furthest right and left
					iX1 = m_iHeaderTextX + m_iHeaderTextWidth + m_iHeaderTextX; //then use the xpadding of the header text on the right
				else if(HeaderIconLeft)
				//if header text is furthest right and icon furthest left
					iX1 = m_iHeaderTextX + m_iHeaderTextWidth + m_iHeaderIconX; //then use the xpadding of the header icon on the right
				else
				//if header text is furthest right and bars left
					iX1 = m_iHeaderTextX + m_iHeaderTextWidth + (m_qb_iPositionX + m_qb_iBarMarginHorizontal); //then use the xpadding of the bars
			}
			else
			//if header icon is furthest right
			{
				if(HeaderTextLeft)
				//if header icon is furthest right and text furthest left
					iX1 = m_iHeaderIconX + m_iHeaderIconWidth + m_iHeaderTextX; //then use the xpadding of the header text on the right
				else if(HeaderIconLeft)
				//if header icon is furthest right and left
					iX1 = m_iHeaderIconX + m_iHeaderIconWidth + m_iHeaderIconX; //then use the xpadding of the header icon on the right
				else
				//if header icon is furthest right and bars left
					iX1 = m_iHeaderIconX + m_iHeaderIconWidth + (m_qb_iPositionX + m_qb_iBarMarginHorizontal); //then use the xpadding of the bars on the right
			}
		}
		else
		{
			if(HeaderTextLeft)
			//if bars are furthest right and text furthest left
				iX1 = iX1 - m_qb_iBarMarginHorizontal + m_iHeaderTextX; //then use the xpadding of the header text on the right
			else if(HeaderIconLeft)
			//if bars are furthest right and icon furthest left
				iX1 = iX1 - m_qb_iBarMarginHorizontal + m_iHeaderIconX; //then use the xpadding of the header icon on the right
			else
			//if bars are furthest right and left
				iX1 = iX1 + m_qb_iPositionX; //then use the xpadding of the bars
		}

			/***************************/
			/***************************/
			/***************************/
			/***************************/


		if( (m_iHeaderTextY + m_iHeaderTextHeight) > iY1 || (m_iHeaderIconY + m_iHeaderIconHeight) > iY1 )
		//if header text or header icon is furthest bottom
		{
			if((m_iHeaderTextY + m_iHeaderTextHeight) > (m_iHeaderIconY + m_iHeaderIconHeight))
			// if header text is furthest bottom
			{
				if(HeaderTextTop)
				//if header text is furthest bottom and top
					iY1 = m_iHeaderTextY + m_iHeaderTextHeight + m_iHeaderTextY; //then use the xpadding of the header text on the right
				else if(HeaderIconTop)
				//if header text is furthest bottom and icon furthest top
					iY1 = m_iHeaderTextY + m_iHeaderTextHeight + m_iHeaderIconY; //then use the xpadding of the header icon on the right
				else
				//if header text is furthest bottom and bars top
					iY1 = m_iHeaderTextY + m_iHeaderTextHeight + (m_qb_iPositionY + m_qb_iBarMarginVertical); //then use the xpadding of the bars
			}
			else
			//if header icon is furthest bottom
			{
				if(HeaderTextTop)
				//if header icon is furthest bottom and text furthest top
					iY1 = m_iHeaderIconY + m_iHeaderIconHeight + m_iHeaderTextY; //then use the xpadding of the header text on the right
				else if(HeaderIconTop)
				//if header icon is furthest bottom and top
					iY1 = m_iHeaderIconY + m_iHeaderIconHeight + m_iHeaderIconY; //then use the xpadding of the header icon on the right
				else
				//if header icon is furthest bottom and bars top
					iY1 = m_iHeaderIconY + m_iHeaderIconHeight + (m_qb_iPositionY + m_qb_iBarMarginVertical); //then use the xpadding of the bars on the right
			}
		}
		else
		{
			if(HeaderTextTop)
			//if bars are furthest bottom and text furthest top
				iY1 = iY1 - m_qb_iBarMarginVertical + m_iHeaderTextY; //then use the xpadding of the header text on the right
			else if(HeaderIconTop)
			//if bars are furthest bottom and icon furthest top
				iY1 = iY1 - m_qb_iBarMarginVertical + m_iHeaderIconY; //then use the xpadding of the header icon on the right
			else
			//if bars are furthest bottom and top
				iY1 = iY1 + m_qb_iPositionY; //then use the xpadding of the bars
		}		

		m_iWidth = iX1 * m_flScale; 
		m_iHeight = iY1 * m_flScale;
		
	}

	void FFQuantityPanel::SetHeaderText(wchar_t *newHeaderText) { 
		wcscpy( m_wszHeaderText, newHeaderText ); 
		//RecalculateHeaderTextDimentions
		vgui::surface()->GetTextSize(m_hfHeaderText[m_iHeaderTextSize*3 + 2], m_wszHeaderText, m_iHeaderTextWidth, m_iHeaderTextHeight);
	}
	void FFQuantityPanel::SetHeaderIconChar(char *newHeaderIconChar)
	{
		char szIconChar[5];
		Q_snprintf( szIconChar, 2, "%s%", newHeaderIconChar );
		vgui::localize()->ConvertANSIToUnicode( szIconChar, m_wszHeaderIcon, sizeof( m_wszHeaderIcon ) );

		//RecalculateHeaderIconDimentions
		vgui::surface()->GetTextSize(m_hfHeaderIcon[m_iHeaderIconSize*3 + 2], m_wszHeaderIcon, m_iHeaderIconWidth, m_iHeaderIconHeight);
	}

	void FFQuantityPanel::SetHeaderTextSize(int iSize)
	{
		m_iHeaderTextSize = iSize;
	}
	void FFQuantityPanel::SetHeaderIconSize(int iSize)
	{
		m_iHeaderIconSize = iSize;
	}

	void FFQuantityPanel::SetHeaderTextVisible(bool bIsVisible) { 
		m_bShowHeaderText = bIsVisible;
	}
	void FFQuantityPanel::SetHeaderIconVisible(bool bIsVisible) { 
		m_bShowHeaderIcon = bIsVisible;
	}

	void FFQuantityPanel::SetHeaderTextColor( Color newHeaderTextColor ) { m_ColorHeaderText = newHeaderTextColor; }
	void FFQuantityPanel::SetHeaderIconColor( Color newHeaderIconColor ) { m_ColorHeaderIcon = newHeaderIconColor; }
	void FFQuantityPanel::SetHeaderTextPosition( int iPositionX, int iPositionY ) { m_iHeaderTextX = iPositionX; m_iHeaderTextY = iPositionY; }
	void FFQuantityPanel::SetHeaderIconPosition( int iPositionX, int iPositionY ) { m_iHeaderIconX = iPositionX; m_iHeaderIconY = iPositionY; }
	void FFQuantityPanel::SetHeaderTextShadow(bool bHasShadow) { m_bHeaderTextShadow = bHasShadow; }
	void FFQuantityPanel::SetHeaderIconShadow(bool bHasShadow) { m_bHeaderIconShadow = bHasShadow; }
	
	void FFQuantityPanel::SetBarsVisible(bool bIsVisible)
	{
		for(int i = 0; i < m_iQBars; ++i)
		{
			m_QBars[i]->SetVisible(bIsVisible);
		}
	}
	
	void FFQuantityPanel::OnChildDimentionsChanged (KeyValues* data)
	{
		int id = data->GetInt("id",-1);

		//loop and see if it matches the pointer
		//data->GetPtr("panel") 
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