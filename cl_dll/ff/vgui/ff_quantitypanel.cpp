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

#include "ff_customhudoptions_assignpresets.h"
extern CFFCustomHudAssignPresets *g_AP;

extern ConVar cl_teamcolourhud;

#define BARUPDATE_WAITTIME 2 //2 seconds

namespace vgui
{
	FFQuantityPanel::FFQuantityPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
		m_flScale = 1.0f;
		m_flScaleX = 1.0f;
		m_flScaleY = 1.0f;

		m_iItems = 0;
	
		m_iHeaderTextX = 20; 
		m_iHeaderTextY = 7;
		m_iHeaderIconX = 3; 
		m_iHeaderIconY = 3;

		m_iX = 640;
		m_iY = 300;
		m_iWidth = 0;
		m_iHeight = 0;
		m_iHorizontalAlign = 2;
		m_iVerticalAlign = 0;
		m_iHeaderTextSize = 4;
		m_iHeaderIconSize = 4;
		m_bShowHeaderText = true;
		m_bShowHeaderIcon = true;
		m_bHeaderIconShadow = false;
		m_bHeaderTextShadow = false;

		m_bShowPanel = true;

		m_iItemMarginHorizontal = 5;
		m_iItemMarginVertical = 5;
		
		m_iItemPositionX = 5;
		m_iItemPositionY = 25;
		m_iItemColumns = 1;

		m_flCheckUpdateFlagTime = -1;
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
		
		
		BaseClass::ApplySchemeSettings( pScheme );
	}
	void FFQuantityPanel::Paint() 
	{
		if(m_bShowHeaderText)
		{
			//Paint Header Text
			vgui::surface()->DrawSetTextFont( m_hfHeaderText[m_iHeaderTextSize * 3 + (m_bHeaderTextShadow ? 1 : 0)] );
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

		//SetPaintBackgroundEnabled(true);
		//SetPaintBackgroundType(2);
		//SetPaintBorderEnabled(false);
		//SetPaintEnabled(true);

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

	Color FFQuantityPanel::GetTeamColor()
	{
		C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

		return g_PR->GetTeamColor( pPlayer->GetTeamNumber() );

	}

	void FFQuantityPanel::SetTeamColor(Color teamColor)
	{
		if(m_ColorTeam != teamColor)
		{
			m_ColorTeam = teamColor;
			for(int i = 0; i < m_iItems; ++i)
			{		
				m_QBars[i]->SetTeamColor(teamColor);
			}	
		}
	}

	void FFQuantityPanel::OnTick()
	{
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
					kv->SetPtr("panel", this);
					PostMessage(g_AP, kv);
					m_bAddToHudSent = true;
				}
			}
		}

		if (!engine->IsInGame()) 
			return;

		//TODO: this is a work around for the SetVisible not working properly - need to figure out why it does what it does
		if(!m_bDraw)
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
		}
		else
		{
			SetPaintEnabled(true);
			SetPaintBackgroundEnabled(m_bShowPanel);
		}

		SetTeamColor(GetTeamColor());

		if(m_bShowPanel)
		{
			if(m_bCustomBackroundColor)
				SetBgColor(m_ColorBackgroundCustom);
			else if(cl_teamcolourhud.GetBool())
				SetBgColor(Color(m_ColorTeam.r(), m_ColorTeam.g(), m_ColorTeam.b(), m_iTeamColourAlpha));
			else
				SetBgColor(m_ColorBackground);
		}

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
			UpdateQBPositions();
		}

		SetPos(m_flScaleX * m_iX + alignOffsetX,  m_flScaleY * m_iY + alignOffsetY);

		if(m_bCheckUpdates)
		{
			if(m_flCheckUpdateFlagTime == -1) 
				m_flCheckUpdateFlagTime = gpGlobals->curtime;

			if(m_flCheckUpdateFlagTime + BARUPDATE_WAITTIME > gpGlobals->curtime)
			{
				for(int i = 0; i < m_iItems; ++i)
				{
					if(!m_bUpdateRecieved[i])
						return;
				}
			}
			m_flCheckUpdateFlagTime = -1;

			//if we got here then they have all been updated
			for(int i = 0; i < m_iItems; ++i)
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

		for(int i = 0; i < m_iItems; ++i)
		{
			int iWidth, iHeight, iOffsetX, iOffsetY;
			m_QBars[i]->GetPanelPositioningData(iWidth, iHeight, iOffsetX, iOffsetY);

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

		for(int i = 0; i < m_iItems; ++i)
		{
			int iPosX, iPosY;
	
			iPosX = (m_iItemPositionX + iColumnWidths + iColumn * m_iItemMarginHorizontal) * m_flScale;
			iPosY = (m_iItemPositionY + iRowHeights + iRow * m_iItemMarginVertical) * m_flScale;

			m_QBars[i]->SetPos( iPosX, iPosY );
			m_QBars[i]->SetPosOffset( m_iColumnOffset[iColumn], m_iRowOffset[iRow] );
			m_QBars[i]->SetSize( m_iColumnWidth[iColumn] * m_flScale, m_iRowHeight[iRow] * m_flScale);
					
			if(i != m_iItems -1)
				//if not the last one
			{
				iColumnWidths += m_iColumnWidth[iColumn++];
				if(iColumn >= m_iItemColumns)	
				{
					iColumnWidths = 0;
					iColumn = 0;
					iRowHeights += m_iRowHeight[iRow++];
				}
			}
		}

		//now we can calculate the overall size of the quantity panel for automatic resizing!
		for(;iColumn < ( m_iItems < m_iItemColumns ? m_iItems : m_iItemColumns );)
		//make sure its calculated from the last (used) column incase we started a new row
			iColumnWidths += m_iColumnWidth[iColumn++];
		//include the height of the row
		iRowHeights += m_iRowHeight[iRow++];

		int iX1 = 0;
		int iY1 = 0;

		if(iColumnWidths > 0)
			iX1 = m_iItemPositionX + iColumnWidths + iColumn * m_iItemMarginHorizontal;
			
		if(iRowHeights > 0)
			iY1 = m_iItemPositionY + iRowHeights + iRow * m_iItemMarginVertical;

		//these are used to get equal padding on either side
		bool HeaderTextLeft = false;
		bool HeaderTextTop = false;
		bool HeaderIconLeft = false;
		bool HeaderIconTop = false;

		if( m_iHeaderTextX < m_iItemPositionX || m_iHeaderIconX < m_iItemPositionX )
			if(m_iHeaderTextX < m_iHeaderIconX && m_bShowHeaderText)
				HeaderTextLeft = true;
			else if(m_bShowHeaderIcon)
				HeaderIconLeft = true;
		if( m_iHeaderTextY < m_iItemPositionY || m_iHeaderIconY < m_iItemPositionY )
			if(m_iHeaderTextY < m_iHeaderIconY && m_bShowHeaderText)
				HeaderTextTop = true;
			else if(m_bShowHeaderIcon)
				HeaderIconTop = true;
	

		if( (m_iHeaderTextX + m_iHeaderTextWidth) > iX1 && m_bShowHeaderText || (m_iHeaderIconX + m_iHeaderIconWidth) > iX1 && m_bShowHeaderIcon )
		//if header text or header icon is furthest right
		{
			if( (m_iHeaderTextX + m_iHeaderTextWidth) > (m_iHeaderIconX + m_iHeaderIconWidth) && m_bShowHeaderText )
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
					iX1 = m_iHeaderTextX + m_iHeaderTextWidth + m_iItemPositionX; //then use the xpadding of the bars
			}
			else if(m_bShowHeaderIcon)
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
					iX1 = m_iHeaderIconX + m_iHeaderIconWidth + m_iItemPositionX; //then use the xpadding of the bars on the right
			}
		}
		else
		{
			if(HeaderTextLeft)
			//if bars are furthest right and text furthest left
				iX1 = iX1 - m_iItemMarginHorizontal + m_iHeaderTextX; //then use the xpadding of the header text on the right
			else if(HeaderIconLeft)
			//if bars are furthest right and icon furthest left
				iX1 = iX1 - m_iItemMarginHorizontal + m_iHeaderIconX; //then use the xpadding of the header icon on the right
			else
			//if bars are furthest right and left
				iX1 = iX1 + m_iItemPositionX; //then use the xpadding of the bars
		}

			/***************************/
			/***************************/
			/***************************/
			/***************************/

		if( (m_iHeaderTextY + m_iHeaderTextHeight) > iY1 && m_bShowHeaderText || (m_iHeaderIconY + m_iHeaderIconHeight) > iY1 && m_bShowHeaderIcon )
		//if header text or header icon is furthest bottom
		{
			if((m_iHeaderTextY + m_iHeaderTextHeight) > (m_iHeaderIconY + m_iHeaderIconHeight) && m_bShowHeaderText)
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
					iY1 = m_iHeaderTextY + m_iHeaderTextHeight + m_iItemPositionY; //then use the xpadding of the bars
			}
			else if(m_bShowHeaderIcon)
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
					iY1 = m_iHeaderIconY + m_iHeaderIconHeight + m_iItemPositionY; //then use the xpadding of the bars on the right
			}
		}
		else
		{
			if(HeaderTextTop)
			//if bars are furthest bottom and text furthest top
				iY1 = iY1 - m_iItemMarginVertical + m_iHeaderTextY; //then use the xpadding of the header text on the right
			else if(HeaderIconTop)
			//if bars are furthest bottom and icon furthest top
				iY1 = iY1 - m_iItemMarginVertical + m_iHeaderIconY; //then use the xpadding of the header icon on the right
			else
			//if bars are furthest bottom and top
				iY1 = iY1 + m_iItemPositionY; //then use the xpadding of the bars
		}		

		if(m_iHeight != iY1 * m_flScale || m_iWidth != iX1 * m_flScale)
		{
			m_iWidth = iX1 * m_flScale; 
			m_iHeight = iY1 * m_flScale;
			SetSize(m_iWidth, m_iHeight);
		}
	}
	
	void FFQuantityPanel::SetHeaderText( wchar_t *newHeaderText, bool bUpdateQBPositions ) { 
		wcscpy( m_wszHeaderText, newHeaderText ); 
		//RecalculateHeaderTextDimentions
		vgui::surface()->GetTextSize(m_hfHeaderText[m_iHeaderTextSize*3 + 2], m_wszHeaderText, m_iHeaderTextWidth, m_iHeaderTextHeight);
		if(bUpdateQBPositions)
			UpdateQBPositions();
		
	}
	void FFQuantityPanel::SetHeaderIconChar( char *newHeaderIconChar, bool bUpdateQBPositions )
	{
		char szIconChar[5];
		Q_snprintf( szIconChar, 2, "%s%", newHeaderIconChar );
		vgui::localize()->ConvertANSIToUnicode( szIconChar, m_wszHeaderIcon, sizeof( m_wszHeaderIcon ) );

		//RecalculateHeaderIconDimentions
		vgui::surface()->GetTextSize(m_hfHeaderIcon[m_iHeaderIconSize*3 + 2], m_wszHeaderIcon, m_iHeaderIconWidth, m_iHeaderIconHeight);
		if(bUpdateQBPositions)
			UpdateQBPositions();
	}

	void FFQuantityPanel::SetHeaderTextSize( int iSize, bool bUpdateQBPositions )
	{
		if(m_iHeaderTextSize != iSize)
		{
			m_iHeaderTextSize = iSize;
			if(bUpdateQBPositions)
				UpdateQBPositions();
		}
	}
	void FFQuantityPanel::SetHeaderIconSize( int iSize, bool bUpdateQBPositions )
	{
		if(m_iHeaderIconSize != iSize)
		{
			m_iHeaderIconSize = iSize;
			if(bUpdateQBPositions)
				UpdateQBPositions();
		}
	}

	void FFQuantityPanel::SetHeaderTextVisible( bool bIsVisible, bool bUpdateQBPositions ) 
	{ 
		if(m_bShowHeaderText != bIsVisible)
		{
			m_bShowHeaderText = bIsVisible;
			if(bUpdateQBPositions)
				UpdateQBPositions();
		}
	}
	void FFQuantityPanel::SetHeaderIconVisible( bool bIsVisible, bool bUpdateQBPositions ) 
	{ 
		if(m_bShowHeaderIcon != bIsVisible)
		{
			m_bShowHeaderIcon = bIsVisible;
			if(bUpdateQBPositions)
				UpdateQBPositions();
		}
	}

	void FFQuantityPanel::SetHeaderTextColor( Color newHeaderTextColor ) { m_ColorHeaderText = newHeaderTextColor; }
	void FFQuantityPanel::SetHeaderIconColor( Color newHeaderIconColor ) { m_ColorHeaderIcon = newHeaderIconColor; }

	void FFQuantityPanel::SetHeaderTextPosition( int iPositionX, int iPositionY, bool bUpdateQBPositions ) 
	{ 
		m_iHeaderTextX = iPositionX; 
		m_iHeaderTextY = iPositionY; 
		if(bUpdateQBPositions)
			UpdateQBPositions();
	}
	void FFQuantityPanel::SetHeaderIconPosition( int iPositionX, int iPositionY, bool bUpdateQBPositions )
	{ 
		m_iHeaderIconX = iPositionX; 
		m_iHeaderIconY = iPositionY;
		if(bUpdateQBPositions)
			UpdateQBPositions(); 
	}

	void FFQuantityPanel::SetHeaderTextShadow( bool bHasShadow) { m_bHeaderTextShadow = bHasShadow; }
	void FFQuantityPanel::SetHeaderIconShadow( bool bHasShadow) { m_bHeaderIconShadow = bHasShadow; }
	
	void FFQuantityPanel::SetBarsVisible( bool bIsVisible, bool bUpdateQBPositions )
	{
		for(int i = 0; i < m_iItems; ++i)
			m_QBars[i]->SetVisible(bIsVisible);

		if(bUpdateQBPositions)
			UpdateQBPositions();
	}


	
	void FFQuantityPanel::OnStyleDataRecieved( KeyValues *kvStyleData )
	{
		KeyValues* kvDefaultStyleData = GetDefaultStyleData();	
		bool bUpdateQBPositions = false;

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
			bUpdateQBPositions = true;
		}

		int iVerticalAlign = kvStyleData->GetInt("alignV", kvDefaultStyleData->GetInt("alignV", -1));
		if(m_iVerticalAlign != iVerticalAlign && iVerticalAlign != -1)
		{
			m_iVerticalAlign = iVerticalAlign;
			bUpdateQBPositions = true;
		}

		int iColumns = kvStyleData->GetInt("columns", kvDefaultStyleData->GetInt("columns", -1));
		if(m_iItemColumns != iColumns && iColumns != -1)
		{
			m_iItemColumns = iColumns;
			bUpdateQBPositions = true;
		}

		int iItemsX = kvStyleData->GetInt("itemsX", kvDefaultStyleData->GetInt("itemsX", -1));
		if(m_iItemPositionX != iItemsX && iItemsX != -1)
		{
			m_iItemPositionX = iItemsX;
			bUpdateQBPositions = true;
		}

		int iItemsY = kvStyleData->GetInt("itemsY", kvDefaultStyleData->GetInt("itemsY", -1));
		if(m_iItemPositionY != iItemsY && iItemsY != -1)
		{
			m_iItemPositionY = iItemsY;
			bUpdateQBPositions = true;
		}

		int iPanelColorCustom = kvStyleData->GetInt("panelColorCustom", kvDefaultStyleData->GetInt("panelColorCustom", -1));
		if((iPanelColorCustom == 1 ? true : false) != m_bCustomBackroundColor && iPanelColorCustom != -1)
			m_bCustomBackroundColor = !m_bCustomBackroundColor;

		int iPanelRed = kvStyleData->GetInt("panelRed", kvDefaultStyleData->GetInt("panelRed", -1));
		int iPanelGreen = kvStyleData->GetInt("panelGreen", kvDefaultStyleData->GetInt("panelGreen", -1));
		int iPanelBlue = kvStyleData->GetInt("panelBlue", kvDefaultStyleData->GetInt("panelBlue", -1));
		int iPanelAlpha = kvStyleData->GetInt("panelAlpha", kvDefaultStyleData->GetInt("panelAlpha", -1));

		bool bValidColor = false;

		Color color = Color(iPanelRed, iPanelGreen, iPanelBlue, iPanelAlpha);
		if(iPanelRed != -1 && iPanelGreen != -1 && iPanelBlue != -1 && iPanelAlpha != -1)
			bValidColor = true;

		if(m_ColorBackgroundCustom != color && bValidColor)
				m_ColorBackgroundCustom = color;

		int iShowHeaderText = kvStyleData->GetInt("showHeaderText", kvDefaultStyleData->GetInt("showHeaderText", -1));
		if((iShowHeaderText == 1 ? true : false) != m_bShowHeaderText && iShowHeaderText != -1)
		{
			SetHeaderTextVisible(!m_bShowHeaderText, false);
			bUpdateQBPositions = true;
		}

		int iHeaderTextShadow = kvStyleData->GetInt("headerTextShadow", kvDefaultStyleData->GetInt("headerTextShadow", -1));
		if((iHeaderTextShadow == 1 ? true : false) != m_bHeaderTextShadow && iHeaderTextShadow != -1)
			SetHeaderTextShadow(!m_bHeaderTextShadow);

		int iHeaderTextSize = kvStyleData->GetInt("headerTextSize", kvDefaultStyleData->GetInt("headerTextSize", -1));
		if(m_iHeaderTextSize != iHeaderTextSize && iHeaderTextSize != -1)
		{
			SetHeaderTextSize(iHeaderTextSize, false);
			bUpdateQBPositions = true;
		}

		int iHeaderTextX = kvStyleData->GetInt("headerTextX", kvDefaultStyleData->GetInt("headerTextX", -1));
		int iHeaderTextY = kvStyleData->GetInt("headerTextY", kvDefaultStyleData->GetInt("headerTextY", -1));
		if((m_iHeaderTextX != iHeaderTextX && iHeaderTextX != -1) || (m_iHeaderTextY != iHeaderTextY && iHeaderTextY != -1))
		{
			SetHeaderTextPosition(iHeaderTextX, iHeaderTextY, false);
			bUpdateQBPositions = true;
		}

		int iShowHeaderIcon = kvStyleData->GetInt("showHeaderIcon", kvDefaultStyleData->GetInt("showHeaderIcon", -1));
		if((iShowHeaderIcon == 1 ? true : false) != m_bShowHeaderIcon && iShowHeaderIcon != -1)
		{
			SetHeaderIconVisible(!m_bShowHeaderIcon, false);
			bUpdateQBPositions = true;
		}

		int iHeaderIconShadow = kvStyleData->GetInt("headerIconShadow", kvDefaultStyleData->GetInt("headerIconShadow", -1));
		if((iHeaderIconShadow == 1 ? true : false) != m_bHeaderIconShadow && iHeaderIconShadow != -1)
			SetHeaderIconShadow(!m_bHeaderIconShadow);

		int iHeaderIconSize = kvStyleData->GetInt("headerIconSize", kvDefaultStyleData->GetInt("headerIconSize", -1));
		if(m_iHeaderIconSize != iHeaderIconSize && iHeaderIconSize != -1)
		{
			SetHeaderIconSize(iHeaderIconSize, false);
			bUpdateQBPositions = true;
		}

		int iHeaderIconX = kvStyleData->GetInt("headerIconX", kvDefaultStyleData->GetInt("headerIconX", -1));
		int iHeaderIconY = kvStyleData->GetInt("headerIconY", kvDefaultStyleData->GetInt("headerIconY", -1));
		if((m_iHeaderIconX != iHeaderIconX && iHeaderIconX != -1) || (m_iHeaderIconY != iHeaderIconY && iHeaderIconY != -1))
			SetHeaderIconPosition(iHeaderIconX, iHeaderIconY);

		int iShowPanel = kvStyleData->GetInt("showPanel", kvDefaultStyleData->GetInt("showPanel", -1));
		if((iShowPanel == 1 ? true : false) != m_bShowPanel && iShowPanel != -1)
			m_bShowPanel = !m_bShowPanel;
		
		int iItemMarginHorizontal = kvStyleData->GetInt("barMarginHorizontal", kvDefaultStyleData->GetInt("barMarginHorizontal", -1));
		int iItemMarginVertical = kvStyleData->GetInt("barMarginVertical", kvDefaultStyleData->GetInt("barMarginVertical", -1));
		if((m_iItemMarginHorizontal != iItemMarginHorizontal && iItemMarginHorizontal != -1) || (m_iItemMarginVertical != iItemMarginVertical && iItemMarginVertical != -1))
		{
			m_iItemMarginHorizontal = iItemMarginHorizontal;
			m_iItemMarginVertical = iItemMarginVertical;
			bUpdateQBPositions = true;
		}		

		for(int i = 0; i < m_iItems; ++i)
			m_QBars[i]->SetStyle(kvStyleData, kvDefaultStyleData);

		if(bUpdateQBPositions)
			UpdateQBPositions();
	}

	void FFQuantityPanel::OnChildDimentionsChanged( KeyValues *data )
	{
		int id = data->GetInt("id",-1);

		//loop and see if it matches the pointer
		//data->GetPtr("panel") 
		if(id != -1 && id < m_iItems)
		{
			m_bUpdateRecieved[id] = true;

			//set the wait for update flag
			if(!m_bCheckUpdates)
				m_bCheckUpdates = true;
		}
	}

	FFQuantityBar* FFQuantityPanel::AddChild( const char *pElementName )
	{
		FFQuantityBar* newQBar = new FFQuantityBar(this, pElementName, m_iItems); 
		m_QBars[m_iItems++] = newQBar;

		return newQBar;
	}
}