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
#include "ff_utils.h" //GetIntensityColor

#include "ienginevgui.h"
#include <vgui/IVGui.h>
#include <vgui/ISurface.h>
#include <vgui/ILocalize.h>

#include "c_playerresource.h"
extern C_PlayerResource *g_PR;

#include "ff_customhudoptions_assignpresets.h"
extern CFFCustomHudAssignPresets *g_AP;

extern ConVar cl_teamcolourhud;

#define BARUPDATE_WAITTIME 1 //1 second

namespace vgui
{
	FFQuantityPanel::FFQuantityPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
		m_flScale = 1.0f;
		m_flScaleX = 1.0f;
		m_flScaleY = 1.0f;
	
		m_iHeaderTextPositionOffsetX = 0; 
		m_iHeaderTextPositionOffsetY = -2;		
		m_iHeaderIconPositionOffsetX = 0; 
		m_iHeaderIconPositionOffsetY = -2;
		m_iTextPositionOffsetX = 0; 
		m_iTextPositionOffsetY = 0;
		
		m_iHeaderTextAnchorPosition = ANCHORPOS_TOPRIGHT;
		m_iHeaderIconAnchorPosition = ANCHORPOS_TOPLEFT;
		m_iTextAnchorPosition = ANCHORPOS_TOPLEFT;
		
		m_iHeaderTextAlignHoriz = ALIGN_RIGHT;
		m_iHeaderIconAlignHoriz = ALIGN_LEFT;
		m_iTextAlignHoriz = ALIGN_LEFT;

		m_iHeaderTextAlignVert = ALIGN_BOTTOM;
		m_iHeaderIconAlignVert = ALIGN_BOTTOM;
		m_iTextAlignVert = ALIGN_TOP;

		m_iColorModeHeaderText = COLOR_MODE_CUSTOM;
		m_iColorModeHeaderIcon = COLOR_MODE_CUSTOM;
		m_iColorModeText = COLOR_MODE_CUSTOM;
		m_bCustomBackroundColor = false;

		m_clrHeaderTextColor = Color(255, 255, 255, 255);
		m_clrHeaderIconColor = Color(255, 255, 255, 255);
		m_clrTextColor = Color(255, 255, 255, 255);

		m_iX = 640;
		m_iY = 300;
		m_iWidth = 0;
		m_iHeight = 0;
		m_iHorizontalAlign = ALIGN_RIGHT;
		m_iVerticalAlign = ALIGN_TOP;
		m_iHeaderTextSize = 4;
		m_iHeaderIconSize = 4;
		m_bShowHeaderText = true;
		m_bShowHeaderIcon = true;
		m_bHeaderIconFontShadow = false;
		m_bHeaderTextFontShadow = false;

		m_bShowPanel = true;

		m_iPanelMargin = 5;

		m_iItemMarginHorizontal = 5;
		m_iItemMarginVertical = 5;
		m_iItemColumns = 1;

		m_flCheckUpdateFlagTime = -1;
	}
	
	void FFQuantityPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		vgui::HScheme QuantityItemScheme = vgui::scheme()->LoadSchemeFromFile("resource/QuantityPanelScheme.res", "QuantityPanelScheme");
		vgui::IScheme *qbScheme = vgui::scheme()->GetIScheme(QuantityItemScheme);

		m_ColorPanelBackground = GetSchemeColor("Background", qbScheme);
		m_iTeamColourAlpha = GetSchemeColor("TeamBackgroundAlpha", qbScheme).a();
		
		m_clrHeaderTextColor = GetSchemeColor("HeaderText", qbScheme);
		m_clrHeaderIconColor = GetSchemeColor("HeaderIcon", qbScheme);
		m_clrTextColor = GetSchemeColor("Text", qbScheme);
		m_clrTextColor = GetSchemeColor("Border", qbScheme);
		
		SetBgColor(m_ColorPanelBackground);
		SetPaintBackgroundEnabled(true);
		SetPaintBackgroundType(2);
		SetPaintBorderEnabled(false);
		SetPaintEnabled(true);

		for(int i = 0; i < QUANTITYPANELFONTSIZES; ++i)
		{
			m_hfHeaderText[i*3] = qbScheme->GetFont( VarArgs("QuantityPanelHeader%d",i), true );
			m_hfHeaderText[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderShadow%d",i), true );
			m_hfHeaderText[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityPanelHeader%d",i), false );
		}
		for(int i = 0; i < QUANTITYBARICONSIZES; ++i)
		{
			m_hfHeaderIcon[i*3] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderIcon%d",i), true );
			m_hfHeaderIcon[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderIconShadow%d",i), true );
			m_hfHeaderIcon[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderIcon%d",i), false );
		}
		for(int i = 0; i < QUANTITYPANELFONTSIZES; ++i)
		{
			m_hfText[i*3] = qbScheme->GetFont( VarArgs("QuantityPanel%d",i), true );
			m_hfText[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityPanelShadow%d",i), true );
			m_hfText[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityPanel%d",i), false );
		}		
		
		BaseClass::ApplySchemeSettings( pScheme );
		//this is needed to reverse the HudLayout.res setting default size...
		SetSize(m_iWidth, m_iHeight);
	}

	KeyValues* FFQuantityPanel::GetDefaultStyleData()
	{
		return new KeyValues("styleData");
	}

	void FFQuantityPanel::Paint() 
	{
		VPROF_BUDGET("FFQuantityPanel::Paint","QuantityItems");

		if(m_bShowHeaderText)
		{
			//Paint Header Text
			vgui::surface()->DrawSetTextFont( m_hfHeaderText[m_iHeaderTextSize * 3 + (m_bHeaderTextFontShadow ? 1 : 0)] );
			if(m_iColorModeHeaderText == COLOR_MODE_TEAMCOLORED)
				vgui::surface()->DrawSetTextColor( m_ColorTeam.r(), m_ColorTeam.g(), m_ColorTeam.b(), m_clrHeaderTextColor.a() );
			else if(m_iColorModeHeaderText == COLOR_MODE_CUSTOM)
				vgui::surface()->DrawSetTextColor( m_clrHeaderTextCustomColor.r(), m_clrHeaderTextCustomColor.g(), m_clrHeaderTextCustomColor.b(), m_clrHeaderTextCustomColor.a() );
			else
				vgui::surface()->DrawSetTextColor( m_clrHeaderTextColor.r(), m_clrHeaderTextColor.g(), m_clrHeaderTextColor.b(), m_clrHeaderTextColor.a() );
			vgui::surface()->DrawSetTextPos( m_iOffsetX + m_iHeaderTextPositionX, m_iOffsetY + m_iHeaderTextPositionY );
			vgui::surface()->DrawUnicodeString( m_wszHeaderText );
		}

		if(m_bShowHeaderIcon)
		{
			//Paint Header Icon
			vgui::surface()->DrawSetTextFont( m_hfHeaderIcon[m_iHeaderIconSize * 3 + (m_bHeaderIconFontShadow ? 1 : 0)] );
			if(m_iColorModeHeaderIcon == COLOR_MODE_TEAMCOLORED)
				vgui::surface()->DrawSetTextColor( m_ColorTeam.r(), m_ColorTeam.g(), m_ColorTeam.b(), m_clrHeaderTextColor.a() );
			else if(m_iColorModeHeaderIcon == COLOR_MODE_CUSTOM)
				vgui::surface()->DrawSetTextColor( m_clrHeaderIconCustomColor.r(), m_clrHeaderIconCustomColor.g(), m_clrHeaderIconCustomColor.b(), m_clrHeaderIconCustomColor.a() );
			else
				vgui::surface()->DrawSetTextColor( m_clrHeaderIconColor.r(), m_clrHeaderIconColor.g(), m_clrHeaderIconColor.b(), m_clrHeaderIconColor.a() );
			vgui::surface()->DrawSetTextPos( m_iOffsetX + m_iHeaderIconPositionX, m_iOffsetY + m_iHeaderIconPositionY );
			vgui::surface()->DrawUnicodeString( m_wszHeaderIcon );
		}

		if(m_bShowText && (m_bUseToggleText && m_bToggleTextVisible || !m_bUseToggleText))
		{
			//Paint Text
			vgui::surface()->DrawSetTextFont( m_hfText[m_iTextSize * 3 + (m_bTextFontShadow ? 1 : 0)] );
			if(m_iColorModeText == COLOR_MODE_TEAMCOLORED)
				vgui::surface()->DrawSetTextColor( m_ColorTeam.r(), m_ColorTeam.g(), m_ColorTeam.b(), m_clrHeaderTextColor.a() );
			
			else if(m_iColorModeText == COLOR_MODE_CUSTOM)
				vgui::surface()->DrawSetTextColor( m_clrTextCustomColor.r(), m_clrTextCustomColor.g(), m_clrTextCustomColor.b(), m_clrTextCustomColor.a() );
			else
				vgui::surface()->DrawSetTextColor( m_clrTextColor.r(), m_clrTextColor.g(), m_clrTextColor.b(), m_clrTextColor.a() );
			vgui::surface()->DrawSetTextPos( m_iOffsetX + m_iTextPositionX, m_iOffsetY + m_iTextPositionY );
			vgui::surface()->DrawUnicodeString( m_wszText );
		}

		BaseClass::Paint();
	}

	void FFQuantityPanel::AddPanelToHudOptions(const char* szSelfName, const char* szSelfText, const char* szParentName, const char* szParentText)
	{
		Q_strncpy(m_szSelfName, szSelfName, 127);
		Q_strncpy(m_szParentName, szParentName, 127);
		Q_strncpy(m_szSelfText, szSelfText, 127);
		Q_strncpy(m_szParentText, szParentText, 127);
		m_bAddToHud = true;
	}

	void FFQuantityPanel::AddBooleanOption(KeyValues* kvMessage, const char *pszName, const char *pszText, const bool defaultValue)
	{
		KeyValues *kv = new KeyValues("Boolean");
		kv->SetString("name", pszName);
		//TODO: localise "text" first
		kv->SetString("text", pszText);
		if(defaultValue)
			kv->SetInt("defaultValue", 1);
		kvMessage->AddSubKey(kv);
	}


	void FFQuantityPanel::AddComboOption(KeyValues* kvMessage, const char *pszName, const char *pszText, KeyValues* kvOptions, const int defaultValue)
	{
		KeyValues *kv = new KeyValues("ComboBox");
		kv->SetString("name", pszName);
		//TODO: localise "text" first
		kv->SetString("text", pszText);
		if(defaultValue != -1)
		{
			//fix default value one way or another!
			kv->SetInt("defaultValue", defaultValue);
		}
		kv->AddSubKey(kvOptions);
		kvMessage->AddSubKey(kv);
	}

	KeyValues* FFQuantityPanel::AddPanelSpecificOptions(KeyValues *kvPanelSpecificOptions)
	{
		return NULL;
	}

	KeyValues* FFQuantityPanel::AddItemStyleList(KeyValues *kvItemStyleList)
	{
		for(int i = 0; i < m_DarQuantityItems.Count(); ++i)
		{
			kvItemStyleList->SetString(m_DarQuantityItems[i]->GetName(), "Default");
		}
		
		return kvItemStyleList;
	}

	Color FFQuantityPanel::GetTeamColor()
	{
		C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

		return g_PR->GetTeamColor( pPlayer->GetTeamNumber() );

	}
			
	void FFQuantityPanel::CalculateTextAnchorPosition( int &outX, int &outY, int iAnchorPosition )
	{	
		switch(iAnchorPosition)
		{
		case ANCHORPOS_TOPLEFT:
			outX = 0;
			outY = 0;
			break;
		case ANCHORPOS_MIDDLELEFT:
			outX = 0;
			outY = m_iItemsHeight / 2;
			break;
		case ANCHORPOS_BOTTOMLEFT:
			outX = 0;
			outY = m_iItemsHeight;
			break;
			
		case ANCHORPOS_TOPCENTER:
			outX =m_iItemsWidth / 2;
			outY = 0;
			break;
		case ANCHORPOS_MIDDLECENTER:
			outX = m_iItemsWidth / 2;
			outY = m_iItemsHeight / 2;
			break;
		case ANCHORPOS_BOTTOMCENTER:
			outX = m_iItemsWidth / 2;
			outY = m_iItemsHeight;
			break;
						
		case ANCHORPOS_TOPRIGHT:
			outX = m_iItemsWidth;
			outY = 0;
			break;
		case ANCHORPOS_MIDDLERIGHT:
			outX = m_iItemsWidth;
			outY = m_iItemsHeight / 2;
			break;
		case ANCHORPOS_BOTTOMRIGHT:
			outX = m_iItemsWidth;
			outY = m_iItemsHeight;
			break;
		}	
	}	
	void FFQuantityPanel::CalculateTextAlignmentOffset( int &iX, int &iY, int &iWide, int &iTall, int iAlignHoriz, int iAlignVert, HFont hfFont, wchar_t* wszString )
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

	void FFQuantityPanel::SetTeamColor(Color teamColor)
	{
		if(m_ColorTeam != teamColor)
		{
			m_ColorTeam = teamColor;
			for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
			{		
				m_DarQuantityItems[i]->SetTeamColor(teamColor);
			}	
		}
	}

	void FFQuantityPanel::OnTick()
	{
		VPROF_BUDGET("FFQuantityPanel::OnTick","QuantityItems");

		if(m_bAddToHud && !m_bAddToHudSent)
		{
			//put the global check separately from the above case 
			//just in case it uses more resources
			if(g_AP != NULL)
			{
				if(g_AP->IsReady())
				{
					KeyValues *kv = new KeyValues("AddQuantityPanel");
					kv->SetString("selfName", m_szSelfName);
					kv->SetString("selfText", m_szSelfText);
					kv->SetString("parentName", m_szParentName);
					kv->SetString("parentText", m_szParentText);
					kv->AddSubKey(AddPanelSpecificOptions(new KeyValues("PanelSpecificOptions")));
					kv->AddSubKey(AddItemStyleList(new KeyValues("Items")));
					kv->SetPtr("panel", this);
					PostMessage(g_AP, kv);
					m_bAddToHudSent = true;
				}
			}
		}

		if (!engine->IsInGame()) 
			return;

		SetPaintBackgroundEnabled(m_bShowPanel);
		SetTeamColor(GetTeamColor());

		if(m_bShowPanel)
		{
			if(m_bCustomBackroundColor)
				SetBgColor(m_ColorPanelBackgroundCustom);
			else if(cl_teamcolourhud.GetBool())
				SetBgColor(Color(m_ColorTeam.r(), m_ColorTeam.g(), m_ColorTeam.b(), m_iTeamColourAlpha));
			else
				SetBgColor(m_ColorPanelBackground);
		}
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
			RecalculateItemPositions();
		}

		int panelAlignmentOffsetX, panelAlignmentOffsetY;

		switch(m_iHorizontalAlign)
		{
		case ALIGN_CENTER:
			panelAlignmentOffsetX = -m_iWidth/2;
			break;
		case ALIGN_RIGHT:
			panelAlignmentOffsetX = -m_iWidth;
			break;
		case ALIGN_LEFT:
		default:
			panelAlignmentOffsetX = 0;
		}

		switch(m_iVerticalAlign)
		{
		case ALIGN_MIDDLE:
			panelAlignmentOffsetY = -m_iHeight/2;
			break;
		case ALIGN_BOTTOM:
			panelAlignmentOffsetY = -m_iHeight;
			break;
		case ALIGN_TOP:
		default:
			panelAlignmentOffsetY = 0;
		}

		SetPos(m_flScaleX * m_iX + panelAlignmentOffsetX,  m_flScaleY * m_iY + panelAlignmentOffsetY);

		if(m_bCheckUpdates)
		{
			if(m_flCheckUpdateFlagTime == -1) 
				m_flCheckUpdateFlagTime = gpGlobals->curtime;
			
			bool bAllItemDimentionsChanged = true;

			//if the wait time hasn't been exceeded
			if( gpGlobals->curtime < ( m_flCheckUpdateFlagTime + BARUPDATE_WAITTIME ) )
			{
				for( int i = 0; i < m_DarQuantityItems.GetCount(); ++i )
				{
					if(!m_DarQuantityItems[i]->GetItemDimentionsChanged())
					{
						bAllItemDimentionsChanged = false;
						break;
					}
				}
			}
			
			if(bAllItemDimentionsChanged)
			{
				//if we got here then they have all been updated or the wait time was exceeded
				for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
				{
					//clear all updateReceived flags
					m_DarQuantityItems[i]->SetItemDimentionsChanged(false);
				}

				//clear check update flag
				m_bCheckUpdates = false;

				//update item positions
				RecalculateItemPositions();
				
				m_flCheckUpdateFlagTime = -1;		
			}
		}
	}
		
	void FFQuantityPanel::RecalculateItemPositions() 
	{
		//these are used in a few ways just to keep count in loops
		int iRow = 0;
		int iColumn = 0;
		
		int iSkipFromIndex = m_DarQuantityItems.GetCount();
		int* m_iRowHeight = new int[iSkipFromIndex];// = {0,0,0,0,0,0};
		int* m_iColumnWidth = new int[iSkipFromIndex];// = {0,0,0,0,0,0};
		int* m_iColumnOffset = new int[iSkipFromIndex];// = {0,0,0,0,0,0};
		int* m_iRowOffset = new int[iSkipFromIndex];// = {0,0,0,0,0,0};	
		int iCountVisible = 0;

		//go through items - check their enabled flag and initialise sizing data to zero
		for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
		{
			m_iColumnOffset[i] = 0;
			m_iColumnWidth[i] = 0;
			m_iRowOffset[i] = 0;
			m_iRowHeight[i] = 0;

			if(m_DarQuantityItems[i]->IsDisabled())
			{
				if(i > 0 && iSkipFromIndex == m_DarQuantityItems.GetCount())
					iSkipFromIndex = i;
			}
			else
			{
				iCountVisible++;
				iSkipFromIndex = m_DarQuantityItems.GetCount();
			}
		}

		if(iCountVisible == 0)
		//if they're all invisible
		{
			//don't skip! draw the container for all invis items
			iSkipFromIndex = m_DarQuantityItems.GetCount();
		}

		//go through items and get their individual heights and widths and drawing position offsets 
		for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
		{
			if(i >= iSkipFromIndex)
				break;

			int iWidth, iHeight, iOffsetX, iOffsetY;
			m_DarQuantityItems[i]->GetPanelPositioningData(iWidth, iHeight, iOffsetX, iOffsetY);

			if(iOffsetX > m_iColumnOffset[iColumn])
				m_iColumnOffset[iColumn] = iOffsetX;
			if(iWidth > m_iColumnWidth[iColumn])
				m_iColumnWidth[iColumn] = iWidth;

			if(iOffsetY > m_iRowOffset[iRow])
				m_iRowOffset[iRow] = iOffsetY;
			if(iHeight > m_iRowHeight[iRow])
				m_iRowHeight[iRow] = iHeight;

			++iColumn;
			if(iColumn >= m_iItemColumns)	
			{
				iColumn = 0;
				++iRow;
			}
		}

		iRow = 0;
		iColumn = 0;
		
		int iColumnWidths = 0;
		int iRowHeights = 0;

		for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
		{
			if(i >= iSkipFromIndex)
				break;

			int iPosX, iPosY;
	
			iPosX = (/*m_iItemPositionX + */iColumnWidths + iColumn * m_iItemMarginHorizontal);
			iPosY = (/*m_iItemPositionY + */iRowHeights + iRow * m_iItemMarginVertical);

			m_DarQuantityItems[i]->SetPos( iPosX, iPosY );
			m_DarQuantityItems[i]->SetPaintOffset( m_iColumnOffset[iColumn], m_iRowOffset[iRow] );
			m_DarQuantityItems[i]->SetSize( m_iColumnWidth[iColumn], m_iRowHeight[iRow]);
				
			//if not the last bar	
			if(i < (iSkipFromIndex - 1))
			{
				iColumnWidths += m_iColumnWidth[iColumn++];

				//if the next column is greater than the number of columns allowed (0 index!)
				if(iColumn >= m_iItemColumns)	
				{
					iColumnWidths = 0;
					iColumn = 0;
					iRowHeights += m_iRowHeight[iRow++];
				}
			}
		}
		
		/*
		* now we can calculate the overall size of the quantity panel for automatic resizing!
		*/

		//include the height of the row in the rowHeights
		iRowHeights += m_iRowHeight[iRow++];

		//if there are less items than there are columns then the column count is the number of items
		int iColumnCount = (m_DarQuantityItems.GetCount() < m_iItemColumns ? m_DarQuantityItems.GetCount() : m_iItemColumns);
		
		//lets add up the remaining empty columns on this row (in case we started a new row in the last for loop)
		while(iColumn < iColumnCount)
		{	
			if((iColumn + 1 * iRow) <= iSkipFromIndex)
				iColumnWidths += m_iColumnWidth[iColumn++];
			else
				break;
		}

		int iItemsWidth = iColumnWidths + (iColumn-1) * m_iItemMarginHorizontal;
		int iItemsHeight = iRowHeights + (iRow-1) * m_iItemMarginVertical;

		if(m_iItemsWidth != iItemsWidth || m_iItemsHeight != iItemsHeight)
		{
			m_iItemsWidth = iItemsWidth;
			m_iItemsHeight = iItemsHeight;
		}
		
		RecalculateHeaderTextPosition(false);
		RecalculateHeaderIconPosition(false);
		RecalculateTextPosition(false);
		RecalculatePaintOffset();
	}

	void FFQuantityPanel::RecalculatePaintOffset( )
	{
		float flX0 = 0.0f;
		float flY0 = 0.0f;
		float flX1 = m_iItemsWidth;
		float flY1 = m_iItemsHeight;
		
		if( m_bShowHeaderText && m_iHeaderTextPositionX < flX0 )
			flX0 = m_iHeaderTextPositionX;
		if( m_bShowHeaderIcon && m_iHeaderIconPositionX < flX0 )
			flX0 = m_iHeaderIconPositionX;
		if( m_bShowText && m_iTextPositionX < flX0 )
			flX0 = m_iTextPositionX;

		if( m_bShowHeaderText && m_iHeaderTextPositionY < flY0 )
			flY0 = m_iHeaderTextPositionY;
		if( m_bShowHeaderIcon && m_iHeaderIconPositionY < flY0 )
			flY0 = m_iHeaderIconPositionY;
		if( m_bShowText && m_iTextPositionY < flY0 )
			flY0 = m_iTextPositionY;

		if( m_bShowHeaderText && (m_iHeaderTextPositionX + m_iHeaderTextWidth) > flX1 )
			flX1 = m_iHeaderTextPositionX + m_iHeaderTextWidth;
		if( m_bShowHeaderIcon && (m_iHeaderIconPositionX + m_iHeaderIconWidth) > flX1 )
			flX1 = m_iHeaderIconPositionX + m_iHeaderIconWidth;
		if( m_bShowText && (m_iTextPositionX + m_iTextWidth) > flX1 )
			flX1 = m_iTextPositionX + m_iTextWidth;

		if( m_bShowHeaderText && (m_iHeaderTextPositionY + m_iHeaderTextHeight) > flY1 )
			flY1 = m_iHeaderTextPositionY + m_iHeaderTextHeight;
		if( m_bShowHeaderIcon && (m_iHeaderIconPositionY + m_iHeaderIconHeight) > flY1 )
			flY1 = m_iHeaderIconPositionY + m_iHeaderIconHeight;
		if( m_bShowText && (m_iTextPositionY + m_iTextHeight) > flY1 )
			flY1 = m_iTextPositionY + m_iTextHeight;

		flX0 -= m_iPanelMargin;
		flY0 -= m_iPanelMargin;
		flX1 += m_iPanelMargin;
		flY1 += m_iPanelMargin;
		
		if(m_iOffsetX != (int)(-flX0 + 0.6f) || m_iOffsetY != (int)(-flY0 + 0.6f))
		{
			//+0.6 so it always rounds up not down.
			m_iOffsetX = -flX0 + 0.6f;
			m_iOffsetY = -flY0 + 0.6f;

			for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
			{
				m_DarQuantityItems[i]->SetPosOffset( m_iOffsetX, m_iOffsetY );
			}

		}

		if(m_iWidth != (int)(flX1 - flX0 + 0.6f) || m_iHeight != (int)(flY1 - flY0 + 0.6f))
		{
			//+0.6 so it always rounds up not down.
			m_iWidth = flX1 - flX0 + 0.6f;
			m_iHeight = flY1 - flY0 + 0.6f;
			SetSize(m_iWidth, m_iHeight);
		}
	}
	void FFQuantityPanel::RecalculateHeaderIconPosition( bool bRecalculatePaintOffset )
	{
		CalculateTextAnchorPosition(m_iHeaderIconAnchorPositionX, m_iHeaderIconAnchorPositionY, m_iHeaderIconAnchorPosition);
		
		CalculateTextAlignmentOffset(m_iHeaderIconAlignmentOffsetX, m_iHeaderIconAlignmentOffsetY, m_iHeaderIconWidth, m_iHeaderIconHeight, m_iHeaderIconAlignHoriz, m_iHeaderIconAlignVert, m_hfHeaderIcon[m_iHeaderIconSize * 3 + (m_bHeaderIconFontShadow ? 1 : 0)], m_wszHeaderIcon);

		m_iHeaderIconPositionX = m_iHeaderIconAnchorPositionX + m_iHeaderIconAlignmentOffsetX + m_iHeaderIconPositionOffsetX * m_flScale;
		m_iHeaderIconPositionY = m_iHeaderIconAnchorPositionY + m_iHeaderIconAlignmentOffsetY + m_iHeaderIconPositionOffsetY * m_flScale;
		
		if( bRecalculatePaintOffset )
		{
			RecalculatePaintOffset();
		}
	}
	void FFQuantityPanel::RecalculateHeaderTextPosition( bool bRecalculatePaintOffset )
	{
		CalculateTextAnchorPosition(m_iHeaderTextAnchorPositionX, m_iHeaderTextAnchorPositionY, m_iHeaderTextAnchorPosition);
		
		CalculateTextAlignmentOffset(m_iHeaderTextAlignmentOffsetX, m_iHeaderTextAlignmentOffsetY, m_iHeaderTextWidth, m_iHeaderTextHeight, m_iHeaderTextAlignHoriz, m_iHeaderTextAlignVert, m_hfHeaderText[m_iHeaderTextSize*3 + (m_bHeaderTextFontShadow ? 1 : 0)], m_wszHeaderText);
		
		m_iHeaderTextPositionX = m_iHeaderTextAnchorPositionX + m_iHeaderTextAlignmentOffsetX + m_iHeaderTextPositionOffsetX * m_flScale;
		m_iHeaderTextPositionY = m_iHeaderTextAnchorPositionY + m_iHeaderTextAlignmentOffsetY + m_iHeaderTextPositionOffsetY * m_flScale;
		
		if( bRecalculatePaintOffset )
		{
			RecalculatePaintOffset();
		}
	}
	void FFQuantityPanel::RecalculateTextPosition( bool bRecalculatePaintOffset )
	{
		CalculateTextAnchorPosition(m_iTextAnchorPositionX, m_iTextAnchorPositionY, m_iTextAnchorPosition);
		
		CalculateTextAlignmentOffset(m_iTextAlignmentOffsetX, m_iTextAlignmentOffsetY, m_iTextWidth, m_iTextHeight, m_iTextAlignHoriz, m_iTextAlignVert, m_hfText[m_iTextSize*3 + (m_bTextFontShadow ? 1 : 0)], m_wszText);
		
		m_iTextPositionX = m_iTextAnchorPositionX + m_iTextAlignmentOffsetX + m_iTextPositionOffsetX * m_flScale;
		m_iTextPositionY = m_iTextAnchorPositionY + m_iTextAlignmentOffsetY + m_iTextPositionOffsetY * m_flScale;
		
		if( bRecalculatePaintOffset )
		{
			RecalculatePaintOffset();
		}
	}
		
	void FFQuantityPanel::SetHeaderText( wchar_t *newHeaderText, bool bRecalculatePaintOffset )
	{ 
		wcscpy( m_wszHeaderText, newHeaderText ); 
		
		RecalculateHeaderTextPosition(bRecalculatePaintOffset);	
	}
	void FFQuantityPanel::SetHeaderIconChar( char *newHeaderIconChar, bool bRecalculatePaintOffset )
	{
		char szIconChar[5];
		Q_snprintf( szIconChar, 2, "%s%", newHeaderIconChar );
		vgui::localize()->ConvertANSIToUnicode( szIconChar, m_wszHeaderIcon, sizeof( m_wszHeaderIcon ) );

		RecalculateHeaderIconPosition(bRecalculatePaintOffset);
	}
	void FFQuantityPanel::SetText( wchar_t *newText, bool bRecalculatePaintOffset )
	{ 
		wcscpy( m_wszText, newText ); 
		
		RecalculateTextPosition(bRecalculatePaintOffset);
	}
	
	void FFQuantityPanel::SetHeaderTextSize( int iSize, bool bRecalculatePaintOffset )
	{
		if(m_iHeaderTextSize != iSize)
		{
			m_iHeaderTextSize = iSize;
			RecalculateHeaderTextPosition( bRecalculatePaintOffset );
		}
	}
	void FFQuantityPanel::SetHeaderIconSize( int iSize, bool bRecalculatePaintOffset )
	{
		if(m_iHeaderIconSize != iSize)
		{
			m_iHeaderIconSize = iSize;
			RecalculateHeaderIconPosition( bRecalculatePaintOffset );
		}
	}
	void FFQuantityPanel::SetTextSize( int iSize, bool bRecalculatePaintOffset )
	{
		if(m_iTextSize != iSize)
		{
			m_iTextSize = iSize;
			RecalculateTextPosition( bRecalculatePaintOffset );
		}
	}
	
	void FFQuantityPanel::SetHeaderTextVisible( bool bIsVisible, bool bRecalculatePaintOffset ) 
	{ 
		if(m_bShowHeaderText != bIsVisible)
		{
			m_bShowHeaderText = bIsVisible;
			if(bRecalculatePaintOffset)
				RecalculatePaintOffset();
		}
	}
	void FFQuantityPanel::SetHeaderIconVisible( bool bIsVisible, bool bRecalculatePaintOffset ) 
	{ 
		if(m_bShowHeaderIcon != bIsVisible)
		{
			m_bShowHeaderIcon = bIsVisible;
			if(bRecalculatePaintOffset)
				RecalculatePaintOffset();
		}
	}
	void FFQuantityPanel::SetTextVisible( bool bIsVisible, bool bRecalculatePaintOffset )
	{ 
		if(m_bShowText != bIsVisible)
		{
			m_bShowText = bIsVisible;
			if(bRecalculatePaintOffset)
				RecalculatePaintOffset();
		}
	}

	void FFQuantityPanel::SetUseToggleText( bool bUseToggleText )
	{
		m_bUseToggleText = bUseToggleText;
	}
	void FFQuantityPanel::SetToggleTextVisible( bool bIsVisible )
	{
		m_bToggleTextVisible = bIsVisible;
		RecalculatePaintOffset();
	}
	
	void FFQuantityPanel::SetHeaderTextColor( Color newHeaderTextColor ) { m_clrHeaderTextColor = newHeaderTextColor; }
	void FFQuantityPanel::SetHeaderIconColor( Color newHeaderIconColor ) { m_clrHeaderIconColor = newHeaderIconColor; }
	void FFQuantityPanel::SetTextColor( Color newTextColor ) { m_clrTextColor = newTextColor; }
	
	void FFQuantityPanel::SetHeaderTextPositionOffset( int iOffsetXHeaderText, int iOffsetYHeaderText, bool bRecalculatePaintOffset ) 
	{ 
		if(m_iHeaderTextPositionOffsetX != iOffsetXHeaderText || m_iHeaderTextPositionOffsetY != iOffsetYHeaderText)
		{
			m_iHeaderTextPositionOffsetX = iOffsetXHeaderText; 
			m_iHeaderTextPositionOffsetY = iOffsetYHeaderText; 
			RecalculateHeaderTextPosition( bRecalculatePaintOffset );
		}
	}
	void FFQuantityPanel::SetHeaderIconPositionOffset( int iOffsetXHeaderIcon, int iOffsetYHeaderIcon, bool bRecalculatePaintOffset )
	{ 
		if(m_iHeaderIconPositionOffsetX != iOffsetXHeaderIcon || m_iHeaderIconPositionOffsetY != iOffsetYHeaderIcon)
		{
			m_iHeaderIconPositionOffsetX = iOffsetXHeaderIcon; 
			m_iHeaderIconPositionOffsetY = iOffsetYHeaderIcon;
			RecalculateHeaderIconPosition( bRecalculatePaintOffset );
		}
	}
	void FFQuantityPanel::SetTextPositionOffset( int iOffsetXText, int iOffsetYText, bool bRecalculatePaintOffset )
	{ 
		if(m_iTextPositionOffsetX != iOffsetXText || m_iTextPositionOffsetY != iOffsetYText)
		{
			m_iTextPositionOffsetX = iOffsetXText; 
			m_iTextPositionOffsetY = iOffsetYText;
			RecalculateTextPosition( bRecalculatePaintOffset );
		}
	}
	
	void FFQuantityPanel::SetHeaderTextAlignment( int iHeaderTextAlignHoriz, int iHeaderTextAlignVert, bool bRecalculatePaintOffset )
	{
		if(m_iHeaderTextAlignHoriz != iHeaderTextAlignHoriz || m_iHeaderTextAlignVert != iHeaderTextAlignVert)
		{
			m_iHeaderTextAlignHoriz = iHeaderTextAlignHoriz; 
			m_iHeaderTextAlignVert = iHeaderTextAlignVert;
			RecalculateHeaderTextPosition( bRecalculatePaintOffset );
		}
	}

	void FFQuantityPanel::SetHeaderIconAlignment( int iHeaderIconAlignHoriz, int iHeaderIconAlignVert, bool bRecalculatePaintOffset )
	{
		if(m_iHeaderIconAlignHoriz != iHeaderIconAlignHoriz || m_iHeaderIconAlignVert != iHeaderIconAlignVert)
		{
			m_iHeaderIconAlignHoriz = iHeaderIconAlignHoriz; 
			m_iHeaderIconAlignVert = iHeaderIconAlignVert;
			RecalculateHeaderIconPosition( bRecalculatePaintOffset );
		}
	}
	void FFQuantityPanel::SetTextAlignment( int iTextAlignHoriz, int iTextAlignVert, bool bRecalculatePaintOffset )
	{
		if(m_iTextAlignHoriz != iTextAlignHoriz || m_iTextAlignVert != iTextAlignVert)
		{
			m_iTextAlignHoriz = iTextAlignHoriz; 
			m_iTextAlignVert = iTextAlignVert;
			RecalculateTextPosition( bRecalculatePaintOffset );
		}
	}
	
	void FFQuantityPanel::SetHeaderTextAnchorPosition( int iAnchorPositionHeaderText, bool bRecalculatePaintOffset )
	{
		if(m_iHeaderTextAnchorPosition != iAnchorPositionHeaderText)
		{
			m_iHeaderTextAnchorPosition = iAnchorPositionHeaderText;
			RecalculateHeaderTextPosition( bRecalculatePaintOffset );
		}
	}
	void FFQuantityPanel::SetHeaderIconAnchorPosition( int iAnchorPositionHeaderIcon, bool bRecalculatePaintOffset )
	{
		if(m_iHeaderIconAnchorPosition != iAnchorPositionHeaderIcon)
		{
			m_iHeaderIconAnchorPosition = iAnchorPositionHeaderIcon;
			RecalculateHeaderIconPosition( bRecalculatePaintOffset );
		}
	}
	void FFQuantityPanel::SetTextAnchorPosition( int iAnchorPositionText, bool bRecalculatePaintOffset )
	{
		if(m_iTextAnchorPosition != iAnchorPositionText)
		{
			m_iTextAnchorPosition = iAnchorPositionText;
			RecalculateTextPosition( bRecalculatePaintOffset );
		}
	}
	
	void FFQuantityPanel::SetHeaderTextShadow( bool bHasShadow) { m_bHeaderTextFontShadow = bHasShadow; }
	void FFQuantityPanel::SetHeaderIconShadow( bool bHasShadow) { m_bHeaderIconFontShadow = bHasShadow; }
	void FFQuantityPanel::SetTextShadow( bool bHasShadow) { m_bTextFontShadow = bHasShadow; }
	
	void FFQuantityPanel::ApplyStyleData( KeyValues *kvStyleData, bool useDefaults )
	{
		KeyValues* kvDefaultStyleData;

		if(useDefaults)
			kvDefaultStyleData = GetDefaultStyleData();	
		else
			kvDefaultStyleData = new KeyValues("styleData");

		bool bRecalculateItemPositions = false;
		bool bRecalculatePaintOffset = false;

		int iPreviewMode = kvStyleData->GetInt("previewMode", kvDefaultStyleData->GetInt("previewMode", -1));
		if((iPreviewMode == 1 ? true : false) != m_bPreviewMode && iPreviewMode != -1)
		{
			SetPreviewMode((iPreviewMode == 1 ? true : false));
		}

		int iX = kvStyleData->GetInt("x", kvDefaultStyleData->GetInt("x", -1));
		if(m_iX != iX && iX != -1)
			m_iX = iX;

		int iY = kvStyleData->GetInt("y", kvDefaultStyleData->GetInt("y", -1));
		if(m_iY != iY && iY != -1)
			m_iY = iY;

		int iHorizontalAlign = kvStyleData->GetInt("alignH", kvDefaultStyleData->GetInt("alignH", -1));
		if(m_iHorizontalAlign != iHorizontalAlign && iHorizontalAlign != -1)
		{
			m_iHorizontalAlign = iHorizontalAlign;
		}

		int iVerticalAlign = kvStyleData->GetInt("alignV", kvDefaultStyleData->GetInt("alignV", -1));
		if(m_iVerticalAlign != iVerticalAlign && iVerticalAlign != -1)
		{
			m_iVerticalAlign = iVerticalAlign;
		}

		int iColumns = kvStyleData->GetInt("itemColumns", kvDefaultStyleData->GetInt("itemColumns", -1));
		if(m_iItemColumns != iColumns && iColumns != -1)
		{
			m_iItemColumns = iColumns;
			bRecalculateItemPositions = true;
		}

		
		int iPanelMargin = kvStyleData->GetInt("panelMargin", kvDefaultStyleData->GetInt("panelMargin", -1));
		if(m_iPanelMargin != iPanelMargin && iPanelMargin != -1)
		{
			m_iPanelMargin = iPanelMargin;
			bRecalculatePaintOffset = true;
		}

		int iCustomBackroundColor = kvStyleData->GetInt("panelColorMode", kvDefaultStyleData->GetInt("panelColorMode", -1));

		if((iCustomBackroundColor == COLOR_MODE_CUSTOM ? true : false) != m_bCustomBackroundColor && iCustomBackroundColor != -1)
			m_bCustomBackroundColor = !m_bCustomBackroundColor;
		

		int iPanelRed = kvStyleData->GetInt("panelRed", kvDefaultStyleData->GetInt("panelRed", -1));
		int iPanelGreen = kvStyleData->GetInt("panelGreen", kvDefaultStyleData->GetInt("panelGreen", -1));
		int iPanelBlue = kvStyleData->GetInt("panelBlue", kvDefaultStyleData->GetInt("panelBlue", -1));
		int iPanelAlpha = kvStyleData->GetInt("panelAlpha", kvDefaultStyleData->GetInt("panelAlpha", -1));
		if(iPanelRed != -1 && iPanelGreen != -1 && iPanelBlue != -1 && iPanelAlpha != -1)
		{
			Color color = Color(iPanelRed, iPanelGreen, iPanelBlue, iPanelAlpha);
			if(m_ColorPanelBackgroundCustom != color)
			{
					m_ColorPanelBackgroundCustom = color;
			}
		}

		int iShowHeaderText = kvStyleData->GetInt("showHeaderText", kvDefaultStyleData->GetInt("showHeaderText", -1));
		int iHeaderTextShadow = kvStyleData->GetInt("headerTextShadow", kvDefaultStyleData->GetInt("headerTextShadow", -1));
		int iHeaderTextSize = kvStyleData->GetInt("headerTextSize", kvDefaultStyleData->GetInt("headerTextSize", -1));
		int iHeaderTextAnchorPosition = kvStyleData->GetInt("headerTextAnchorPosition", kvDefaultStyleData->GetInt("headerTextAnchorPosition", -1));
		int iHeaderTextAlignHoriz = kvStyleData->GetInt("headerTextAlignHoriz", kvDefaultStyleData->GetInt("headerTextAlignHoriz", -1));
		int iHeaderTextAlignVert = kvStyleData->GetInt("headerTextAlignVert", kvDefaultStyleData->GetInt("headerTextAlignVert", -1));
		int iHeaderTextX = kvStyleData->GetInt("headerTextX", kvDefaultStyleData->GetInt("headerTextX", -9999));
		int iHeaderTextY = kvStyleData->GetInt("headerTextY", kvDefaultStyleData->GetInt("headerTextY", -9999));
		int iHeaderTextColorMode = kvStyleData->GetInt("headerTextColorMode", kvDefaultStyleData->GetInt("headerTextColorMode", -1));
		int iHeaderTextRed = kvStyleData->GetInt("headerTextRed", kvDefaultStyleData->GetInt("headerTextRed", -1));
		int iHeaderTextGreen = kvStyleData->GetInt("headerTextGreen", kvDefaultStyleData->GetInt("headerTextGreen", -1));
		int iHeaderTextBlue = kvStyleData->GetInt("headerTextBlue", kvDefaultStyleData->GetInt("headerTextBlue", -1));
		int iHeaderTextAlpha = kvStyleData->GetInt("headerTextAlpha", kvDefaultStyleData->GetInt("headerTextAlpha", -1));

		if((iShowHeaderText == 1 ? true : false) != m_bShowHeaderText && iShowHeaderText != -1)
		{
			SetHeaderTextVisible(!m_bShowHeaderText, false);
			bRecalculatePaintOffset = true;
		}

		if((iHeaderTextShadow == 1 ? true : false) != m_bHeaderTextFontShadow && iHeaderTextShadow != -1)
			SetHeaderTextShadow(!m_bHeaderTextFontShadow);

		if(m_iHeaderTextSize != iHeaderTextSize && iHeaderTextSize != -1)
		{
			SetHeaderTextSize(iHeaderTextSize, false);
			bRecalculatePaintOffset = true;
		}

		if(m_iHeaderTextAnchorPosition != iHeaderTextAnchorPosition && iHeaderTextAnchorPosition != -1)
		{
			SetHeaderTextAnchorPosition(iHeaderTextAnchorPosition, false);
			bRecalculatePaintOffset = true;
		}
		if((m_iHeaderTextAlignHoriz != iHeaderTextAlignHoriz && iHeaderTextAlignHoriz != -1) || (m_iHeaderTextAlignVert != iHeaderTextAlignVert && iHeaderTextAlignVert != -1))
		{
			SetHeaderTextAlignment(iHeaderTextAlignHoriz, iHeaderTextAlignVert, false);
			bRecalculatePaintOffset = true;
		}
		if((m_iHeaderTextPositionOffsetX != iHeaderTextX && iHeaderTextX != -9999) || (m_iHeaderTextPositionOffsetY != iHeaderTextY && iHeaderTextY != -9999))
		{
			SetHeaderTextPositionOffset(iHeaderTextX, iHeaderTextY, false);
			bRecalculatePaintOffset = true;
		}

		if(m_iColorModeHeaderText != iHeaderTextColorMode && iHeaderTextColorMode != -1)
		{
			m_iColorModeHeaderText = iHeaderTextColorMode;
		}

		if(iHeaderTextRed != -1 && iHeaderTextGreen != -1 && iHeaderTextBlue != -1 && iHeaderTextAlpha != -1)
		{
			Color color = Color(iHeaderTextRed, iHeaderTextGreen, iHeaderTextBlue, iHeaderTextAlpha);
			if(m_clrHeaderTextCustomColor != color)
			{
					m_clrHeaderTextCustomColor = color;
			}
		}

		int iShowHeaderIcon = kvStyleData->GetInt("showHeaderIcon", kvDefaultStyleData->GetInt("showHeaderIcon", -1));
		int iHeaderIconShadow = kvStyleData->GetInt("headerIconShadow", kvDefaultStyleData->GetInt("headerIconShadow", -1));
		int iHeaderIconSize = kvStyleData->GetInt("headerIconSize", kvDefaultStyleData->GetInt("headerIconSize", -1));
		int iHeaderIconAnchorPosition = kvStyleData->GetInt("headerIconAnchorPosition", kvDefaultStyleData->GetInt("headerIconAnchorPosition", -1));
		int iHeaderIconAlignHoriz = kvStyleData->GetInt("headerIconAlignHoriz", kvDefaultStyleData->GetInt("headerIconAlignHoriz", -1));
		int iHeaderIconAlignVert = kvStyleData->GetInt("headerIconAlignVert", kvDefaultStyleData->GetInt("headerIconAlignVert", -1));
		int iHeaderIconX = kvStyleData->GetInt("headerIconX", kvDefaultStyleData->GetInt("headerIconX", -9999));
		int iHeaderIconY = kvStyleData->GetInt("headerIconY", kvDefaultStyleData->GetInt("headerIconY", -9999));
		int iHeaderIconColorMode = kvStyleData->GetInt("headerIconColorMode", kvDefaultStyleData->GetInt("headerIconColorMode", -1));
		int iHeaderIconRed = kvStyleData->GetInt("headerIconRed", kvDefaultStyleData->GetInt("headerIconRed", -1));
		int iHeaderIconGreen = kvStyleData->GetInt("headerIconGreen", kvDefaultStyleData->GetInt("headerIconGreen", -1));
		int iHeaderIconBlue = kvStyleData->GetInt("headerIconBlue", kvDefaultStyleData->GetInt("headerIconBlue", -1));
		int iHeaderIconAlpha = kvStyleData->GetInt("headerIconAlpha", kvDefaultStyleData->GetInt("headerIconAlpha", -1));

		if((iShowHeaderIcon == 1 ? true : false) != m_bShowHeaderIcon && iShowHeaderIcon != -1)
		{
			SetHeaderIconVisible(!m_bShowHeaderIcon, false);
			bRecalculatePaintOffset = true;
		}

		if((iHeaderIconShadow == 1 ? true : false) != m_bHeaderIconFontShadow && iHeaderIconShadow != -1)
			SetHeaderIconShadow(!m_bHeaderIconFontShadow);
		
		if(m_iHeaderIconSize != iHeaderIconSize && iHeaderIconSize != -1)
		{
			SetHeaderIconSize(iHeaderIconSize, false);
			bRecalculatePaintOffset = true;
		}

		if(m_iHeaderIconAnchorPosition != iHeaderIconAnchorPosition && iHeaderIconAnchorPosition != -1)
		{
			SetHeaderIconAnchorPosition(iHeaderIconAnchorPosition, false);
			bRecalculatePaintOffset = true;
		}
		if((m_iHeaderIconAlignHoriz != iHeaderIconAlignHoriz && iHeaderIconAlignHoriz != -1) || (m_iHeaderIconAlignVert != iHeaderIconAlignVert && iHeaderIconAlignVert != -1))
		{
			SetHeaderIconAlignment(iHeaderIconAlignHoriz, iHeaderIconAlignVert, false);
			bRecalculatePaintOffset = true;
		}
		if((m_iHeaderIconPositionOffsetX != iHeaderIconX && iHeaderIconX != -9999) || (m_iHeaderIconPositionOffsetY != iHeaderIconY && iHeaderIconY != -9999))
		{
			SetHeaderIconPositionOffset(iHeaderIconX, iHeaderIconY, false);
			bRecalculatePaintOffset = true;
		}

		if(m_iColorModeHeaderIcon != iHeaderIconColorMode && iHeaderIconColorMode != -1)
		{
			m_iColorModeHeaderIcon = iHeaderIconColorMode;
		}

		if(iHeaderIconRed != -1 && iHeaderIconGreen != -1 && iHeaderIconBlue != -1 && iHeaderIconAlpha != -1)
		{
			Color color = Color(iHeaderIconRed, iHeaderIconGreen, iHeaderIconBlue, iHeaderIconAlpha);
			if(m_clrHeaderIconCustomColor != color)
			{
				m_clrHeaderIconCustomColor = color;
			}
		}

		int iShowText = kvStyleData->GetInt("showText", kvDefaultStyleData->GetInt("showText", -1));
		int iTextShadow = kvStyleData->GetInt("textShadow", kvDefaultStyleData->GetInt("textShadow", -1));
		int iTextSize = kvStyleData->GetInt("textSize", kvDefaultStyleData->GetInt("textSize", -1));
		int iTextAnchorPosition = kvStyleData->GetInt("textAnchorPosition", kvDefaultStyleData->GetInt("textAnchorPosition", -1));
		int iTextAlignHoriz = kvStyleData->GetInt("textAlignHoriz", kvDefaultStyleData->GetInt("textAlignHoriz", -1));
		int iTextAlignVert = kvStyleData->GetInt("textAlignVert", kvDefaultStyleData->GetInt("textAlignVert", -1));
		int iTextX = kvStyleData->GetInt("textX", kvDefaultStyleData->GetInt("textX", -9999));
		int iTextY = kvStyleData->GetInt("textY", kvDefaultStyleData->GetInt("textY", -9999));
		int iTextColorMode = kvStyleData->GetInt("textColorMode", kvDefaultStyleData->GetInt("textColorMode", -1));
		int iTextRed = kvStyleData->GetInt("textRed", kvDefaultStyleData->GetInt("textRed", -1));
		int iTextGreen = kvStyleData->GetInt("textGreen", kvDefaultStyleData->GetInt("textGreen", -1));
		int iTextBlue = kvStyleData->GetInt("textBlue", kvDefaultStyleData->GetInt("textBlue", -1));
		int iTextAlpha = kvStyleData->GetInt("textAlpha", kvDefaultStyleData->GetInt("textAlpha", -1));

		if((iShowText == 1 ? true : false) != m_bShowText && iShowText != -1)
		{
			SetTextVisible(!m_bShowText);
			bRecalculatePaintOffset = true;
		}

		if((iTextShadow == 1 ? true : false) != m_bTextFontShadow && iTextShadow != -1)
			SetTextShadow(!m_bTextFontShadow);

		if(m_iTextSize != iTextSize && iTextSize != -1)
		{
			SetTextSize(iTextSize, false);
			bRecalculatePaintOffset = true;
		}

		if(m_iTextAnchorPosition != iTextAnchorPosition && iTextAnchorPosition != -1)
		{
			SetTextAnchorPosition(iTextAnchorPosition, false);
			bRecalculatePaintOffset = true;
		}
		if((m_iTextAlignHoriz != iTextAlignHoriz && iTextAlignHoriz != -1) || (m_iTextAlignVert != iTextAlignVert && iTextAlignVert != -1))
		{
			SetTextAlignment(iTextAlignHoriz, iTextAlignVert, false);
			bRecalculatePaintOffset = true;
		}
		if((m_iTextPositionOffsetX != iTextX && iTextX != -9999) || (m_iTextPositionOffsetY != iTextY && iTextY != -9999))
		{
			SetTextPositionOffset(iTextX, iTextY, false);
			bRecalculatePaintOffset = true;
		}

		if(m_iColorModeText != iTextColorMode && iTextColorMode != -1)
		{
			m_iColorModeText = iTextColorMode;
		}

		if(iTextRed != -1 && iTextGreen != -1 && iTextBlue != -1 && iTextAlpha != -1)
		{
			Color color = Color(iTextRed, iTextGreen, iTextBlue, iTextAlpha);
			if(m_clrTextCustomColor != color)
			{
				m_clrTextCustomColor = color;
			}
		}

		int iShowPanel = kvStyleData->GetInt("showPanel", kvDefaultStyleData->GetInt("showPanel", -1));
		if((iShowPanel == 1 ? true : false) != m_bShowPanel && iShowPanel != -1)
			m_bShowPanel = !m_bShowPanel;
		
		int iItemMarginHorizontal = kvStyleData->GetInt("itemMarginHorizontal", kvDefaultStyleData->GetInt("itemMarginHorizontal", -1));
		int iItemMarginVertical = kvStyleData->GetInt("itemMarginVertical", kvDefaultStyleData->GetInt("itemMarginVertical", -1));
		if((m_iItemMarginHorizontal != iItemMarginHorizontal && iItemMarginHorizontal != -1) || (m_iItemMarginVertical != iItemMarginVertical && iItemMarginVertical != -1))
		{
			m_iItemMarginHorizontal = iItemMarginHorizontal;
			m_iItemMarginVertical = iItemMarginVertical;
			bRecalculateItemPositions = true;
		}

		for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
			m_DarQuantityItems[i]->SetStyle(kvStyleData, kvDefaultStyleData);

		if(bRecalculateItemPositions)
			RecalculateItemPositions();

		if(bRecalculatePaintOffset)
			RecalculatePaintOffset();
	}

	void FFQuantityPanel::OnStyleDataRecieved( KeyValues *kvStyleData )
	{
		ApplyStyleData( kvStyleData );
	}

	void FFQuantityPanel::OnPresetPreviewDataRecieved( KeyValues *kvPresetPreviewData )
	{
		ApplyStyleData( kvPresetPreviewData, false );
	}
	
	void FFQuantityPanel::OnItemDimentionsChanged( KeyValues *data )
	{
		if(!m_bCheckUpdates)
			m_bCheckUpdates = true;
	}
	void FFQuantityPanel::OnDefaultStyleDataRequested( KeyValues *data )
	{
		g_AP->CreatePresetFromPanelDefault(GetDefaultStyleData());
	}

	FFQuantityItem* FFQuantityPanel::AddItem( const char *pElementName )
	{
		FFQuantityItem* newQBar = new FFQuantityItem(this, pElementName); 
		m_DarQuantityItems.AddElement(newQBar);

		return newQBar;
	}
	void FFQuantityPanel::HideItem( FFQuantityItem* qBar )
	{
		qBar->SetVisible(false);
	}
	void FFQuantityPanel::ShowItem( FFQuantityItem* qBar )
	{
		qBar->SetVisible(true);
	}
	void FFQuantityPanel::DisableItem( FFQuantityItem* qBar )
	{
		qBar->SetDisabled(true);
		RecalculateItemPositions();
	}
	void FFQuantityPanel::EnableItem( FFQuantityItem* qBar )
	{
		qBar->SetDisabled(false);
		RecalculateItemPositions();
	}
	
	void FFQuantityPanel::SetPreviewMode(bool bInPreview) 
	{
		m_bPreviewMode = bInPreview;

		/*
		for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
		{
			 m_DarQuantityItems[i]->SetPreviewMode(bInPreview);
		}
		*/

		if(m_bPreviewMode)
		{
			SetParent(enginevgui->GetPanel( PANEL_ROOT ));
		}
		else
		{
			SetParent(g_pClientMode->GetViewport());
		}
	}
}