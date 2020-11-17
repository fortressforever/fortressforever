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
#include "ff_quantityhelper.h"

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

namespace vgui
{
	FFQuantityPanel::FFQuantityPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
		m_flScale = 1.0f;
		m_flScaleX = 1.0f;
		m_flScaleY = 1.0f;

		m_iPositionalHashCode = 0;
		m_iHeaderTextPositionOffsetX = 0;
		m_iHeaderTextPositionOffsetY = -2;
		m_iHeaderIconPositionOffsetX = 0;
		m_iHeaderIconPositionOffsetY = -2;
		m_iTextPositionOffsetX = 0;
		m_iTextPositionOffsetY = 0;

		m_iHeaderTextAnchorPosition = FFQuantityHelper::ANCHORPOS_TOPRIGHT;
		m_iHeaderIconAnchorPosition = FFQuantityHelper::ANCHORPOS_TOPLEFT;
		m_iTextAnchorPosition = FFQuantityHelper::ANCHORPOS_TOPLEFT;

		m_iHeaderTextAlignHoriz = FFQuantityHelper::ALIGN_RIGHT;
		m_iHeaderIconAlignHoriz = FFQuantityHelper::ALIGN_LEFT;
		m_iTextAlignHoriz = FFQuantityHelper::ALIGN_LEFT;

		m_iHeaderTextAlignVert = FFQuantityHelper::ALIGN_BOTTOM;
		m_iHeaderIconAlignVert = FFQuantityHelper::ALIGN_BOTTOM;
		m_iTextAlignVert = FFQuantityHelper::ALIGN_TOP;

		m_iPanelColorMode = FFQuantityHelper::COLOR_MODE_TEAM;
		m_iHeaderTextColorMode = FFQuantityHelper::COLOR_MODE_CUSTOM;
		m_iHeaderIconColorMode = FFQuantityHelper::COLOR_MODE_CUSTOM;
		m_iTextColorMode = FFQuantityHelper::COLOR_MODE_CUSTOM;

		m_clrHeaderText = Color(255, 255, 255, 255);
		m_clrHeaderIcon = Color(255, 255, 255, 255);
		m_clrText = Color(255, 255, 255, 255);

		m_iX = 640;
		m_iY = 300;
		m_iWidth = 0;
		m_iHeight = 0;
		m_iHorizontalAlign = FFQuantityHelper::ALIGN_RIGHT;
		m_iVerticalAlign = FFQuantityHelper::ALIGN_TOP;
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
	}

	void FFQuantityPanel::ApplySchemeSettings( vgui::IScheme *pScheme )
	{
		BaseClass::ApplySchemeSettings( pScheme );

		vgui::HScheme quantityPanelScheme = vgui::scheme()->LoadSchemeFromFile("resource/QuantityPanelScheme.res", "QuantityPanelScheme");
		vgui::IScheme *qbScheme = vgui::scheme()->GetIScheme(quantityPanelScheme);

		for(int i = 0; i < QUANTITYITEMICONSIZES; ++i)
		{
			m_hfHeaderIconFamily[i*3] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderIcon%d",i), true );
			m_hfHeaderIconFamily[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderIconShadow%d",i), true );
			m_hfHeaderIconFamily[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderIcon%d",i), false );
		}

		for(int i = 0; i < QUANTITYPANELTEXTSIZES; ++i)
		{
			m_hfHeaderTextFamily[i*3] = qbScheme->GetFont( VarArgs("QuantityPanelHeader%d",i), true );
			m_hfHeaderTextFamily[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityPanelHeaderShadow%d",i), true );
			m_hfHeaderTextFamily[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityPanelHeader%d",i), false );

			m_hfTextFamily[i*3] = qbScheme->GetFont( VarArgs("QuantityPanel%d",i), true );
			m_hfTextFamily[i*3 + 1] = qbScheme->GetFont( VarArgs("QuantityPanelShadow%d",i), true );
			m_hfTextFamily[i*3 + 2] = qbScheme->GetFont( VarArgs("QuantityPanel%d",i), false );
		}

		SetPaintBackgroundEnabled(true);
		SetPaintBackgroundType(FFQuantityPanel::CORNERS_ROUND);
		SetPaintBorderEnabled(true);
		SetPaintEnabled(true);
	}

	KeyValues* FFQuantityPanel::GetDefaultStyleData()
	{
		KeyValues *kvPreset = new KeyValues("StyleData");

		kvPreset->SetInt("x", 320);
		kvPreset->SetInt("y", 330);
		kvPreset->SetInt("alignH", FFQuantityHelper::ALIGN_CENTER);
		kvPreset->SetInt("alignV", FFQuantityHelper::ALIGN_TOP);

		kvPreset->SetInt("showPanel", 1);
		kvPreset->SetInt("panelMargin", 5);
		kvPreset->SetInt("panelType", FFQuantityPanel::CORNERS_ROUND);
		kvPreset->SetInt("panelColorMode", FFQuantityHelper::COLOR_MODE_TEAM);
		kvPreset->SetInt("panelRed", 255);
		kvPreset->SetInt("panelGreen", 255);
		kvPreset->SetInt("panelBlue", 255);
		kvPreset->SetInt("panelAlpha", 255);

		kvPreset->SetInt("showHeaderText", 1);
		kvPreset->SetInt("headerTextSize", 3);
		kvPreset->SetInt("headerTextShadow", 1);
		kvPreset->SetInt("headerTextAnchorPosition",  FFQuantityHelper::ANCHORPOS_TOPLEFT);
		kvPreset->SetInt("headerTextAlignHoriz",  FFQuantityHelper::ALIGN_LEFT);
		kvPreset->SetInt("headerTextAlignVert",  FFQuantityHelper::ALIGN_BOTTOM);
		kvPreset->SetInt("headerTextX", 16);
		kvPreset->SetInt("headerTextY", -2);
		kvPreset->SetInt("headerTextColorMode", FFQuantityHelper::COLOR_MODE_CUSTOM);
		kvPreset->SetInt("headerTextRed", 255);
		kvPreset->SetInt("headerTextGreen", 255);
		kvPreset->SetInt("headerTextBlue", 255);
		kvPreset->SetInt("headerTextAlpha", 255);

		kvPreset->SetInt("showHeaderIcon", 1);
		kvPreset->SetInt("headerIconSize", 1);
		kvPreset->SetInt("headerIconShadow", 1);
		kvPreset->SetInt("headerIconAnchorPosition",  FFQuantityHelper::ANCHORPOS_TOPLEFT);
		kvPreset->SetInt("headerIconAlignHoriz",  FFQuantityHelper::ALIGN_LEFT);
		kvPreset->SetInt("headerIconAlignVert",  FFQuantityHelper::ALIGN_BOTTOM);
		kvPreset->SetInt("headerIconX", 0);
		kvPreset->SetInt("headerIconY", 0);
		kvPreset->SetInt("headerIconColorMode", FFQuantityHelper::COLOR_MODE_CUSTOM);
		kvPreset->SetInt("headerIconRed", 255);
		kvPreset->SetInt("headerIconGreen", 255);
		kvPreset->SetInt("headerIconBlue", 255);
		kvPreset->SetInt("headerIconAlpha", 255);

		kvPreset->SetInt("showText", 1);
		kvPreset->SetInt("textSize", 3);
		kvPreset->SetInt("textShadow", 1);
		kvPreset->SetInt("textAnchorPosition",  FFQuantityHelper::ANCHORPOS_TOPLEFT);
		kvPreset->SetInt("textAlignHoriz",  FFQuantityHelper::ALIGN_LEFT);
		kvPreset->SetInt("textAlignVert",  FFQuantityHelper::ALIGN_TOP);
		kvPreset->SetInt("textX", 5);
		kvPreset->SetInt("textY", 5);
		kvPreset->SetInt("textColorMode", FFQuantityHelper::COLOR_MODE_CUSTOM);
		kvPreset->SetInt("textRed", 255);
		kvPreset->SetInt("textGreen", 255);
		kvPreset->SetInt("textBlue", 255);
		kvPreset->SetInt("textAlpha", 255);

		kvPreset->SetInt("barOrientation", FFQuantityItem::ORIENTATION_HORIZONTAL);
		kvPreset->SetInt("barWidth", 70);
		kvPreset->SetInt("barHeight", 5);
		kvPreset->SetInt("barBorderWidth", 1);
		kvPreset->SetInt("itemColumns", 1);
		kvPreset->SetInt("itemMarginHorizontal", 0);
		kvPreset->SetInt("itemMarginVertical", 0);

		KeyValues *kvComponent = new KeyValues("Bar");
		kvComponent->SetInt("show", 1);
		kvComponent->SetInt("colorMode", FFQuantityHelper::COLOR_MODE_FADED);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 130);

		kvPreset->AddSubKey(kvComponent);

		kvComponent = new KeyValues("BarBorder");
		kvComponent->SetInt("show", 0);
		kvComponent->SetInt("colorMode", FFQuantityHelper::COLOR_MODE_CUSTOM);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 155);

		kvPreset->AddSubKey(kvComponent);

		kvComponent = new KeyValues("BarBackground");
		kvComponent->SetInt("show", 1);
		kvComponent->SetInt("colorMode", FFQuantityHelper::COLOR_MODE_FADED);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 40);

		kvPreset->AddSubKey(kvComponent);

		kvComponent = new KeyValues("Icon");
		kvComponent->SetInt("show", 0);
		kvComponent->SetInt("colorMode", FFQuantityHelper::COLOR_MODE_CUSTOM);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 255);
		kvComponent->SetInt("shadow", 1);
		kvComponent->SetInt("size", 2);
		kvComponent->SetInt("anchorPosition",  FFQuantityHelper::ANCHORPOS_MIDDLELEFT);
		kvComponent->SetInt("alignH", FFQuantityHelper::ALIGN_RIGHT);
		kvComponent->SetInt("alignV", FFQuantityHelper::ALIGN_MIDDLE);
		kvComponent->SetInt("offsetX", -2);
		kvComponent->SetInt("offsetY", 0);

		kvPreset->AddSubKey(kvComponent);

		kvComponent = new KeyValues("Label");
		kvComponent->SetInt("show", 1);
		kvComponent->SetInt("colorMode", FFQuantityHelper::COLOR_MODE_CUSTOM);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 255);
		kvComponent->SetInt("fontTahoma", 1);
		kvComponent->SetInt("shadow", 1);
		kvComponent->SetInt("size", 1);
		kvComponent->SetInt("anchorPosition",  FFQuantityHelper::ANCHORPOS_TOPLEFT);
		kvComponent->SetInt("alignH", FFQuantityHelper::ALIGN_LEFT);
		kvComponent->SetInt("alignV", FFQuantityHelper::ALIGN_BOTTOM);
		kvComponent->SetInt("offsetX", 0);
		kvComponent->SetInt("offsetY", 0);

		kvPreset->AddSubKey(kvComponent);

		kvComponent = new KeyValues("Amount");
		kvComponent->SetInt("show", 1);
		kvComponent->SetInt("sticky", 1);
		kvComponent->SetInt("colorMode", FFQuantityHelper::COLOR_MODE_CUSTOM);
		kvComponent->SetInt("red", 255);
		kvComponent->SetInt("green", 255);
		kvComponent->SetInt("blue", 255);
		kvComponent->SetInt("alpha", 255);
		kvComponent->SetInt("fontTahoma", 1);
		kvComponent->SetInt("shadow", 1);
		kvComponent->SetInt("size", 1);
		kvComponent->SetInt("anchorPosition",  FFQuantityHelper::ANCHORPOS_TOPRIGHT);
		kvComponent->SetInt("alignH", FFQuantityHelper::ALIGN_RIGHT);
		kvComponent->SetInt("alignV", FFQuantityHelper::ALIGN_BOTTOM);
		kvComponent->SetInt("offsetX", 0);
		kvComponent->SetInt("offsetY", 0);

		kvPreset->AddSubKey(kvComponent);

		return kvPreset;
	}

	void FFQuantityPanel::Paint()
	{
		VPROF_BUDGET("FFQuantityPanel::Paint","QuantityItems");

		BaseClass::Paint();

		if(m_bShowHeaderText && m_wszHeaderText)
		{
			DrawText(
				m_wszHeaderText,
				m_hfHeaderText,
				m_clrHeaderText,
				m_iOffsetX + m_iHeaderTextPositionX,
				m_iOffsetY + m_iHeaderTextPositionY);
		}

		if(m_bShowHeaderIcon && m_wszHeaderIcon)
		{
			DrawText(
				m_wszHeaderIcon,
				m_hfHeaderIcon,
				m_clrHeaderIcon,
				m_iOffsetX + m_iHeaderIconPositionX,
				m_iOffsetY + m_iHeaderIconPositionY);
		}

		if(m_bShowText && m_wszText && m_bUseToggleText && m_bToggleTextVisible)
		{
			DrawText(
				m_wszText,
				m_hfText,
				m_clrText,
				m_iOffsetX + m_iTextPositionX,
				m_iOffsetY + m_iTextPositionY);
		}
	}

	void FFQuantityPanel::DrawText(
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

	void FFQuantityPanel::RecalculateHeaderIconFont()
	{
		HFont font
			= GetFont(
				m_hfHeaderIconFamily,
				m_iHeaderIconSize,
				m_bHeaderIconFontShadow);

		m_hfHeaderIcon = font;
	}

	void FFQuantityPanel::RecalculateHeaderTextFont()
	{
		HFont font
			= GetFont(
				m_hfHeaderTextFamily,
				m_iHeaderTextSize,
				m_bHeaderTextFontShadow);

		m_hfHeaderText = font;
	}

	void FFQuantityPanel::RecalculateTextFont()
	{
		HFont font
			= GetFont(
				m_hfTextFamily,
				m_iTextSize,
				m_bTextFontShadow);

		m_hfText = font;
	}

	vgui::HFont FFQuantityPanel::GetFont(
		vgui::HFont* hfFamily,
		int iSize,
		bool bUseModifier)
	{
		return FFQuantityHelper::GetFont(
			hfFamily,
			iSize,
			bUseModifier);
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

	Color FFQuantityPanel::GetTeamColor()
	{
		C_FFPlayer *pPlayer = ToFFPlayer(CBasePlayer::GetLocalPlayer());

		return g_PR->GetTeamColor( pPlayer->GetTeamNumber() );

	}

	void FFQuantityPanel::CalculateTextAnchorPosition(
		int &iX,
		int &iY,
		int iAnchorPos)
	{
		int iAlignHoriz, iAlignVert;

		FFQuantityHelper::ConvertToAlignment(
			iAnchorPos,
			iAlignHoriz,
			iAlignVert);
		
		int iWide = m_iItemsWidth * m_flScaleX;
		int iTall = m_iItemsHeight * m_flScaleY;

		FFQuantityHelper::CalculatePositionOffset(
			iX,
			iY,
			iWide,
			iTall,
			iAlignHoriz,
			iAlignVert);
	}

	void FFQuantityPanel::CalculateTextAlignmentOffset( int &iX, int &iY, int &iWide, int &iTall, int iAlignHoriz, int iAlignVert, HFont hfFont, wchar_t* wszString )
	{
		surface()->GetTextSize(hfFont, wszString,  iWide, iTall);

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

	bool FFQuantityPanel::SetTeamColor(Color clrTeam)
	{
		bool bHasChanged = FFQuantityHelper::Change(m_clrTeam, clrTeam);
		
		if (!bHasChanged)
		{
			return false;
		}

		for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
		{
			m_DarQuantityItems[i]->SetTeamColor(clrTeam);
		}

		RecalculateColor(m_clrPanel, m_iPanelColorMode, m_clrPanelCustom);
		RecalculateColor(m_clrHeaderIcon, m_iHeaderIconColorMode, m_clrHeaderIconCustom);
		RecalculateColor(m_clrHeaderText, m_iHeaderTextColorMode, m_clrHeaderTextCustom);
		RecalculateColor(m_clrText, m_iTextColorMode, m_clrTextCustom);

		return true;
	}

	void FFQuantityPanel::OnTick()
	{
		VPROF_BUDGET("FFQuantityPanel::OnTick","QuantityItems");

		if(m_bAddToHud && !m_bAddToHudSent)
		{
			//put the global check separately from the above case
			//just in case it uses more resources
			if(g_AP != NULL && g_AP->IsReady())
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

		if (!engine->IsInGame())
			return;

		SetPaintBackgroundEnabled(m_bShowPanel);
		SetBgColor(m_clrPanel);
		SetTeamColor(GetTeamColor());

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
			m_flScale = (m_flScaleX>=m_flScaleY ? m_flScaleX : m_flScaleY);
			//recalculate text sizes
			RecalculateItemPositions();
		}

		int panelAlignmentOffsetX, panelAlignmentOffsetY;

		FFQuantityHelper::CalculatePositionOffset(
			panelAlignmentOffsetX,
			panelAlignmentOffsetY,
			m_iWidth,
			m_iHeight,
			m_iHorizontalAlign,
			m_iVerticalAlign);

		SetPos(
			m_flScaleX * m_iX - panelAlignmentOffsetX,
			m_flScaleY * m_iY - panelAlignmentOffsetY);

        int iPositionalHashCode = 17;
		for( int i = 0; i < m_DarQuantityItems.GetCount(); ++i )
		{
			FFQuantityHelper::CombineHash(
				iPositionalHashCode,
				m_DarQuantityItems[i]->GetPositionalHashCode());
		}

		if(m_iPositionalHashCode != iPositionalHashCode)
		{
			m_iPositionalHashCode = iPositionalHashCode;

			RecalculateItemPositions();
		}
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

	void FFQuantityPanel::RecalculateItemPositions()
	{
		int iQuantityItems = m_DarQuantityItems.GetCount();

		int* iColumnWidth = new int[iQuantityItems];
		int* iColumnOffset = new int[iQuantityItems];
		int* iRowHeight = new int[iQuantityItems];
		int* iRowOffset = new int[iQuantityItems];

		int iEnabled = 0;

		int iRow = 0;
		int iColumn = 0;

		for(int i = 0; i < iQuantityItems; ++i)
		{
			iColumnOffset[i] = 0;
			iColumnWidth[i] = 0;

			iRowOffset[i] = 0;
			iRowHeight[i] = 0;

			if(m_DarQuantityItems[i]->IsDisabled())
			{
				continue;
			}

			if(iColumn >= m_iItemColumns)
			{
				iRow++;
				iColumn = 0;
			}

			int iWidth, iHeight, iOffsetX, iOffsetY;
			m_DarQuantityItems[i]->GetPanelPositioningData(iWidth, iHeight, iOffsetX, iOffsetY);

			iColumnOffset[iColumn] = max(iOffsetX, iColumnOffset[iColumn]);
			iColumnWidth[iColumn] = max(iWidth, iColumnWidth[iColumn]);

			iRowOffset[iRow] = max(iOffsetY, iRowOffset[iRow]);
			iRowHeight[iRow] = max(iHeight, iRowHeight[iRow]);
			
			iEnabled++;
			iColumn++;
		}

		iRow = 0;
		iColumn = 0;

		int* iColumnWidths = new int[iQuantityItems];
		int* iRowHeights = new int[iQuantityItems];

		for(int i = 0; i < iQuantityItems; ++i)
		{
			iColumnWidths[i] = 0;
			iRowHeights[i] = 0;

			if(m_DarQuantityItems[i]->IsDisabled())
			{
				continue;
			}

			if(iColumn >= m_iItemColumns)
			{
				iRow++;
				iColumn = 0;
			}
			
			int iAccumulativeWidths
				= iColumn == 0
					? 0
					: iColumnWidths[iColumn - 1];

			int iAccumulativeHeights
				= iRow == 0
					? 0
					: iRowHeights[iRow - 1];

			int iAccumulativeHorizontalMargins
				= iColumn * m_iItemMarginHorizontal;

			int iAccumulativeVerticalMargins
				= iRow * m_iItemMarginVertical;

			int iPosX = iAccumulativeWidths + iAccumulativeHorizontalMargins;
			int iPosY = iAccumulativeHeights + iAccumulativeVerticalMargins;
			int iWidth = iColumnWidth[iColumn];
			int iHeight = iRowHeight[iRow];

			m_DarQuantityItems[i]->SetPos( iPosX, iPosY );
			m_DarQuantityItems[i]->SetPaintOffset( iColumnOffset[iColumn], iRowOffset[iRow] );
			m_DarQuantityItems[i]->SetSize( iWidth, iHeight);

			iColumnWidths[iColumn] = iAccumulativeWidths + iWidth;
			iRowHeights[iRow] = iAccumulativeHeights + iHeight;

			iColumn++;
		}
		
		int iColumns
			= iEnabled < m_iItemColumns
				? iEnabled
				: m_iItemColumns;
		
		int iRows = ceil((float)iEnabled / iColumns);

		int iHorizontalMargins = iColumns - 1;
		int iVerticalMargins = iRows - 1;
		int iItemsWidth = iColumnWidths[iColumns - 1] + iHorizontalMargins * m_iItemMarginHorizontal;
		int iItemsHeight = iRowHeights[iRows - 1] + iVerticalMargins * m_iItemMarginVertical;

		if(m_iItemsWidth != iItemsWidth || m_iItemsHeight != iItemsHeight)
		{
			m_iItemsWidth = iItemsWidth;
			m_iItemsHeight = iItemsHeight;
		}

		RecalculateHeaderTextPosition();
		RecalculateHeaderIconPosition();
		RecalculateTextPosition();
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

		int iOffsetX = -flX0;
		int iOffsetY = -flY0;
		if(m_iOffsetX != iOffsetX
			|| m_iOffsetY != iOffsetY)
		{
			m_iOffsetX = iOffsetX;
			m_iOffsetY = iOffsetY;

			for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
			{
				m_DarQuantityItems[i]->SetPosOffset( m_iOffsetX, m_iOffsetY );
			}
		}

		int iWidth = (int)(flX1 - flX0);
		int iHeight = (int)(flY1 - flY0);
		if(m_iWidth != iWidth
			|| m_iHeight != iHeight)
		{
			m_iWidth = iWidth;
			m_iHeight = iHeight;

			SetSize(m_iWidth, m_iHeight);
		}
	}

	void FFQuantityPanel::RecalculateHeaderIconPosition( )
	{
		CalculateTextAnchorPosition(m_iHeaderIconAnchorPositionX, m_iHeaderIconAnchorPositionY, m_iHeaderIconAnchorPosition);

		CalculateTextAlignmentOffset(m_iHeaderIconAlignmentOffsetX, m_iHeaderIconAlignmentOffsetY, m_iHeaderIconWidth, m_iHeaderIconHeight, m_iHeaderIconAlignHoriz, m_iHeaderIconAlignVert, m_hfHeaderIconFamily[m_iHeaderIconSize * 3 + (m_bHeaderIconFontShadow ? 1 : 0)], m_wszHeaderIcon);

		m_iHeaderIconPositionX = m_iHeaderIconAnchorPositionX + m_iHeaderIconAlignmentOffsetX + m_iHeaderIconPositionOffsetX * m_flScaleX;
		m_iHeaderIconPositionY = m_iHeaderIconAnchorPositionY + m_iHeaderIconAlignmentOffsetY + m_iHeaderIconPositionOffsetY * m_flScaleY;
	}

	void FFQuantityPanel::RecalculateHeaderTextPosition()
	{
		CalculateTextAnchorPosition(m_iHeaderTextAnchorPositionX, m_iHeaderTextAnchorPositionY, m_iHeaderTextAnchorPosition);

		CalculateTextAlignmentOffset(m_iHeaderTextAlignmentOffsetX, m_iHeaderTextAlignmentOffsetY, m_iHeaderTextWidth, m_iHeaderTextHeight, m_iHeaderTextAlignHoriz, m_iHeaderTextAlignVert, m_hfHeaderTextFamily[m_iHeaderTextSize*3 + (m_bHeaderTextFontShadow ? 1 : 0)], m_wszHeaderText);

		m_iHeaderTextPositionX = m_iHeaderTextAnchorPositionX + m_iHeaderTextAlignmentOffsetX + m_iHeaderTextPositionOffsetX * m_flScaleX;
		m_iHeaderTextPositionY = m_iHeaderTextAnchorPositionY + m_iHeaderTextAlignmentOffsetY + m_iHeaderTextPositionOffsetY * m_flScaleY;
	}

	void FFQuantityPanel::RecalculateTextPosition( )
	{
		CalculateTextAnchorPosition(m_iTextAnchorPositionX, m_iTextAnchorPositionY, m_iTextAnchorPosition);

		CalculateTextAlignmentOffset(m_iTextAlignmentOffsetX, m_iTextAlignmentOffsetY, m_iTextWidth, m_iTextHeight, m_iTextAlignHoriz, m_iTextAlignVert, m_hfTextFamily[m_iTextSize*3 + (m_bTextFontShadow ? 1 : 0)], m_wszText);

		m_iTextPositionX = m_iTextAnchorPositionX + m_iTextAlignmentOffsetX + m_iTextPositionOffsetX * m_flScaleX;
		m_iTextPositionY = m_iTextAnchorPositionY + m_iTextAlignmentOffsetY + m_iTextPositionOffsetY * m_flScaleY;
	}

	void FFQuantityPanel::SetHeaderText( wchar_t *newHeaderText, bool bRecalculatePaintOffset )
	{
		wcscpy( m_wszHeaderText, newHeaderText );

		RecalculateHeaderTextPosition();

		if(bRecalculatePaintOffset)
		{
			RecalculatePaintOffset();
		}
	}

	void FFQuantityPanel::SetHeaderIconChar( char *newHeaderIconChar, bool bRecalculatePaintOffset )
	{
		char szIconChar[5];
		Q_snprintf( szIconChar, 2, "%s%", newHeaderIconChar );
		vgui::localize()->ConvertANSIToUnicode( szIconChar, m_wszHeaderIcon, sizeof( m_wszHeaderIcon ) );

		RecalculateHeaderIconPosition();

		if(bRecalculatePaintOffset)
		{
			RecalculatePaintOffset();
		}
	}

	void FFQuantityPanel::SetText( wchar_t *newText, bool bRecalculatePaintOffset )
	{
		wcscpy( m_wszText, newText );

		RecalculateTextPosition();

		if(bRecalculatePaintOffset)
		{
			RecalculatePaintOffset();
		}
	}

	bool FFQuantityPanel::SetHeaderIconSize( int iSize )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iHeaderIconSize, iSize);

		if(bHasChanged)
		{
			RecalculateHeaderIconFont();
			RecalculateHeaderIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetHeaderTextSize( int iSize )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iHeaderTextSize, iSize);

		if(bHasChanged)
		{
			RecalculateHeaderTextFont();
			RecalculateHeaderTextPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetTextSize( int iSize )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iTextSize, iSize);

		if(bHasChanged)
		{
			RecalculateTextFont();
			RecalculateTextPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetHeaderTextVisible( bool bIsVisible )
	{
		return FFQuantityHelper::Change(m_bShowHeaderText, bIsVisible);
	}

	bool FFQuantityPanel::SetHeaderIconVisible( bool bIsVisible )
	{
		return FFQuantityHelper::Change(m_bShowHeaderIcon, bIsVisible);
	}

	bool FFQuantityPanel::SetTextVisible( bool bIsVisible )
	{
		return FFQuantityHelper::Change(m_bShowText, bIsVisible);
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

	bool FFQuantityPanel::SetHeaderIconPositionOffset( int iOffsetXHeaderIcon, int iOffsetYHeaderIcon )
	{
		bool bOffsetXChanged = FFQuantityHelper::Change(m_iHeaderIconPositionOffsetX, iOffsetXHeaderIcon);
		bool bOffsetYChanged = FFQuantityHelper::Change(m_iHeaderIconPositionOffsetY, iOffsetYHeaderIcon);

		if(bOffsetXChanged || bOffsetYChanged)
		{
			RecalculateHeaderIconPosition();
		}

		return bOffsetXChanged || bOffsetYChanged;
	}

	bool FFQuantityPanel::SetHeaderTextPositionOffset( int iOffsetXHeaderText, int iOffsetYHeaderText )
	{
		bool bOffsetXChanged = FFQuantityHelper::Change(m_iHeaderTextPositionOffsetX, iOffsetXHeaderText);
		bool bOffsetYChanged = FFQuantityHelper::Change(m_iHeaderTextPositionOffsetY, iOffsetYHeaderText);

		if(bOffsetXChanged || bOffsetYChanged)
		{
			RecalculateHeaderTextPosition();
		}

		return bOffsetXChanged || bOffsetYChanged;
	}

	bool FFQuantityPanel::SetTextPositionOffset( int iOffsetXText, int iOffsetYText )
	{
		bool bOffsetXChanged = FFQuantityHelper::Change(m_iTextPositionOffsetX, iOffsetXText);
		bool bOffsetYChanged = FFQuantityHelper::Change(m_iTextPositionOffsetY, iOffsetYText);

		if(bOffsetXChanged || bOffsetYChanged)
		{
			RecalculateTextPosition();
		}

		return bOffsetXChanged || bOffsetYChanged;
	}

	bool FFQuantityPanel::SetHeaderTextAlignment( int iHeaderTextAlignHoriz, int iHeaderTextAlignVert )
	{
		bool bHorizChanged = FFQuantityHelper::Change(m_iHeaderTextAlignHoriz, iHeaderTextAlignHoriz);
		bool bVertChanged = FFQuantityHelper::Change(m_iHeaderTextAlignVert, iHeaderTextAlignVert);

		if(bHorizChanged || bVertChanged)
		{
			RecalculateHeaderTextPosition();
		}

		return bHorizChanged || bVertChanged;
	}

	bool FFQuantityPanel::SetHeaderIconAlignment( int iHeaderIconAlignHoriz, int iHeaderIconAlignVert )
	{
		bool bHorizChanged = FFQuantityHelper::Change(m_iHeaderIconAlignHoriz, iHeaderIconAlignHoriz);
		bool bVertChanged = FFQuantityHelper::Change(m_iHeaderIconAlignVert, iHeaderIconAlignVert);

		if(bHorizChanged || bVertChanged)
		{
			RecalculateHeaderIconPosition();
		}

		return bHorizChanged || bVertChanged;
	}

	bool FFQuantityPanel::SetTextAlignment( int iTextAlignHoriz, int iTextAlignVert )
	{
		bool bHorizChanged = FFQuantityHelper::Change(m_iTextAlignHoriz, iTextAlignHoriz);
		bool bVertChanged = FFQuantityHelper::Change(m_iTextAlignVert, iTextAlignVert);

		if(bHorizChanged || bVertChanged)
		{
			RecalculateTextPosition();
		}

		return bHorizChanged || bVertChanged;
	}

	bool FFQuantityPanel::SetHeaderTextAnchorPosition( int iAnchorPositionHeaderText )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iHeaderTextAnchorPosition, iAnchorPositionHeaderText);

		if(bHasChanged)
		{
			RecalculateHeaderTextPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetHeaderIconAnchorPosition( int iAnchorPositionHeaderIcon )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iHeaderIconAnchorPosition, iAnchorPositionHeaderIcon);

		if(bHasChanged)
		{
			RecalculateHeaderIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetTextAnchorPosition( int iAnchorPositionText )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iTextAnchorPosition, iAnchorPositionText);

		if(bHasChanged)
		{
			RecalculateTextPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetHeaderIconShadow( bool bUseShadow )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bHeaderIconFontShadow, bUseShadow);

		if(bHasChanged)
		{
			RecalculateHeaderIconFont();
			RecalculateHeaderIconPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetHeaderTextShadow( bool bUseShadow )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bHeaderTextFontShadow, bUseShadow);

		if(bHasChanged)
		{
			RecalculateHeaderTextFont();
			RecalculateHeaderTextPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetTextShadow( bool bUseShadow )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_bTextFontShadow, bUseShadow);

		if(bHasChanged)
		{
			RecalculateTextFont();
			RecalculateTextPosition();
		}

		return bHasChanged;
	}

	void FFQuantityPanel::ApplyStyleData( KeyValues *kvStyleData, bool useDefaults )
	{
		KeyValues* kvDefaultStyleData
			= useDefaults
				? GetDefaultStyleData()
				: new KeyValues("styleData");

		bool bRecalculateItemPositions = false;
		bool bRecalculatePaintOffset = false;

		int iPreviewMode = GetInt("previewMode", kvStyleData, kvDefaultStyleData);
		if(iPreviewMode != -1)
		{
			SetPreviewMode(iPreviewMode == 1);
		}

		int iX = GetInt("x", kvStyleData, kvDefaultStyleData);
		if(iX != -1)
		{
			m_iX = iX;
		}

		int iY = GetInt("y", kvStyleData, kvDefaultStyleData);
		if(iY != -1)
		{
			m_iY = iY;
		}

		int iHorizontalAlign = GetInt("alignH", kvStyleData, kvDefaultStyleData);
		if(iHorizontalAlign != -1)
		{
			m_iHorizontalAlign = iHorizontalAlign;
		}

		int iVerticalAlign = GetInt("alignV", kvStyleData, kvDefaultStyleData);
		if(iVerticalAlign != -1)
		{
			m_iVerticalAlign = iVerticalAlign;
		}

		int iColumns = GetInt("itemColumns", kvStyleData, kvDefaultStyleData);
		if(m_iItemColumns != iColumns && iColumns != -1)
		{
			m_iItemColumns = iColumns;
			bRecalculateItemPositions = true;
		}

		int iPanelMargin = GetInt("panelMargin", kvStyleData, kvDefaultStyleData);
		if(m_iPanelMargin != iPanelMargin && iPanelMargin != -1)
		{
			m_iPanelMargin = iPanelMargin;
			bRecalculatePaintOffset = true;
		}

		int iPanelType = GetInt("panelType", kvStyleData, kvDefaultStyleData);
		if(iPanelType != -1)
		{
			SetPaintBackgroundType(iPanelType);
		}

		int iPanelColorMode = GetInt("panelColorMode", kvStyleData, kvDefaultStyleData);
		if(iPanelColorMode != -1)
		{
			SetPanelColorMode(iPanelColorMode);
		}

		int iPanelRed = GetInt("panelRed", kvStyleData, kvDefaultStyleData);
		int iPanelGreen = GetInt("panelGreen", kvStyleData, kvDefaultStyleData);
		int iPanelBlue = GetInt("panelBlue", kvStyleData, kvDefaultStyleData);
		int iPanelAlpha = GetInt("panelAlpha", kvStyleData, kvDefaultStyleData);
		if(iPanelRed != -1 && iPanelGreen != -1 && iPanelBlue != -1 && iPanelAlpha != -1)
		{
			SetCustomPanelColor(iPanelRed, iPanelGreen, iPanelBlue, iPanelAlpha);
		}

		int iShowHeaderIcon = GetInt("showHeaderIcon", kvStyleData, kvDefaultStyleData);
		int iHeaderIconShadow = GetInt("headerIconShadow", kvStyleData, kvDefaultStyleData);
		int iHeaderIconSize = GetInt("headerIconSize", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAnchorPosition = GetInt("headerIconAnchorPosition", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAlignHoriz = GetInt("headerIconAlignHoriz", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAlignVert = GetInt("headerIconAlignVert", kvStyleData, kvDefaultStyleData);
		int iHeaderIconX = GetInt("headerIconX", kvStyleData, kvDefaultStyleData, -9999);
		int iHeaderIconY = GetInt("headerIconY", kvStyleData, kvDefaultStyleData, -9999);
		int iHeaderIconColorMode = GetInt("headerIconColorMode", kvStyleData, kvDefaultStyleData);
		int iHeaderIconRed = GetInt("headerIconRed", kvStyleData, kvDefaultStyleData);
		int iHeaderIconGreen = GetInt("headerIconGreen", kvStyleData, kvDefaultStyleData);
		int iHeaderIconBlue = GetInt("headerIconBlue", kvStyleData, kvDefaultStyleData);
		int iHeaderIconAlpha = GetInt("headerIconAlpha", kvStyleData, kvDefaultStyleData);

		if(iShowHeaderIcon != -1
			&& SetHeaderIconVisible(iShowHeaderIcon == 1))
		{
			bRecalculatePaintOffset = true;
		}

		if(iHeaderIconShadow != -1
			&& SetHeaderIconShadow(iHeaderIconShadow == 1))
		{
			bRecalculatePaintOffset = true;
		}

		if(iHeaderIconSize != -1
			&& SetHeaderIconSize(iHeaderIconSize))
		{
			bRecalculatePaintOffset = true;
		}

		if(iHeaderIconAnchorPosition != -1
			&& SetHeaderIconAnchorPosition(iHeaderIconAnchorPosition))
		{
			bRecalculatePaintOffset = true;
		}

		if((iHeaderIconAlignHoriz != -1 || iHeaderIconAlignVert != -1)
			&& SetHeaderIconAlignment(iHeaderIconAlignHoriz, iHeaderIconAlignVert))
		{
			bRecalculatePaintOffset = true;
		}

		if((iHeaderIconX != -9999 || iHeaderIconY != -9999)
			&& SetHeaderIconPositionOffset(iHeaderIconX, iHeaderIconY))
		{
			bRecalculatePaintOffset = true;
		}

		if(iHeaderIconColorMode != -1)
		{
			SetHeaderIconColorMode(iHeaderIconColorMode);
		}

		if(iHeaderIconRed != -1 && iHeaderIconGreen != -1 && iHeaderIconBlue != -1 && iHeaderIconAlpha != -1)
		{
			SetCustomHeaderIconColor(iHeaderIconRed, iHeaderIconGreen, iHeaderIconBlue, iHeaderIconAlpha);
		}

		int iShowHeaderText = GetInt("showHeaderText", kvStyleData, kvDefaultStyleData);
		int iHeaderTextShadow = GetInt("headerTextShadow", kvStyleData, kvDefaultStyleData);
		int iHeaderTextSize = GetInt("headerTextSize", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAnchorPosition = GetInt("headerTextAnchorPosition", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAlignHoriz = GetInt("headerTextAlignHoriz", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAlignVert = GetInt("headerTextAlignVert", kvStyleData, kvDefaultStyleData);
		int iHeaderTextX = GetInt("headerTextX", kvStyleData, kvDefaultStyleData, -9999);
		int iHeaderTextY = GetInt("headerTextY", kvStyleData, kvDefaultStyleData, -9999);
		int iHeaderTextColorMode = GetInt("headerTextColorMode", kvStyleData, kvDefaultStyleData);
		int iHeaderTextRed = GetInt("headerTextRed", kvStyleData, kvDefaultStyleData);
		int iHeaderTextGreen = GetInt("headerTextGreen", kvStyleData, kvDefaultStyleData);
		int iHeaderTextBlue = GetInt("headerTextBlue", kvStyleData, kvDefaultStyleData);
		int iHeaderTextAlpha = GetInt("headerTextAlpha", kvStyleData, kvDefaultStyleData);

		if(iShowHeaderText != -1
			&& SetHeaderTextVisible(iShowHeaderText == 1))
		{
			bRecalculatePaintOffset = true;
		}

		if(iHeaderTextShadow != -1
			&& SetHeaderTextShadow(iHeaderTextShadow == 1))
		{
			bRecalculatePaintOffset = true;
		}

		if(iHeaderTextSize != -1
			&& SetHeaderTextSize(iHeaderTextSize))
		{
			bRecalculatePaintOffset = true;
		}

		if(iHeaderTextAnchorPosition != -1)
		{
			SetHeaderTextAnchorPosition(iHeaderTextAnchorPosition);
			bRecalculatePaintOffset = true;
		}

		if((iHeaderTextAlignHoriz != -1 || iHeaderTextAlignVert != -1)
			&& SetHeaderTextAlignment(iHeaderTextAlignHoriz, iHeaderTextAlignVert))
		{
			bRecalculatePaintOffset = true;
		}

		if((iHeaderTextX != -9999 || iHeaderTextY != -9999)
			&& SetHeaderTextPositionOffset(iHeaderTextX, iHeaderTextY))
		{
			bRecalculatePaintOffset = true;
		}

		if(iHeaderTextColorMode != -1)
		{
			SetHeaderTextColorMode(iHeaderTextColorMode);
		}

		if(iHeaderTextRed != -1 && iHeaderTextGreen != -1 && iHeaderTextBlue != -1 && iHeaderTextAlpha != -1)
		{
			SetCustomHeaderTextColor(iHeaderTextRed, iHeaderTextGreen, iHeaderTextBlue, iHeaderTextAlpha);
		}

		int iShowText = GetInt("showText", kvStyleData, kvDefaultStyleData);
		int iTextShadow = GetInt("textShadow", kvStyleData, kvDefaultStyleData);
		int iTextSize = GetInt("textSize", kvStyleData, kvDefaultStyleData);
		int iTextAnchorPosition = GetInt("textAnchorPosition", kvStyleData, kvDefaultStyleData);
		int iTextAlignHoriz = GetInt("textAlignHoriz", kvStyleData, kvDefaultStyleData);
		int iTextAlignVert = GetInt("textAlignVert", kvStyleData, kvDefaultStyleData);
		int iTextX = GetInt("textX", kvStyleData, kvDefaultStyleData, -9999);
		int iTextY = GetInt("textY", kvStyleData, kvDefaultStyleData, -9999);
		int iTextColorMode = GetInt("textColorMode", kvStyleData, kvDefaultStyleData);
		int iTextRed = GetInt("textRed", kvStyleData, kvDefaultStyleData);
		int iTextGreen = GetInt("textGreen", kvStyleData, kvDefaultStyleData);
		int iTextBlue = GetInt("textBlue", kvStyleData, kvDefaultStyleData);
		int iTextAlpha = GetInt("textAlpha", kvStyleData, kvDefaultStyleData);

		if(iShowText != -1 && SetTextVisible(iShowText == 1))
		{
			bRecalculatePaintOffset = true;
		}

		if(iTextShadow != -1
			&& SetTextShadow(iTextShadow == 1))
		{
			bRecalculatePaintOffset = true;
		}

		if(iTextSize != -1
			&& SetTextSize(iTextSize))
		{
			bRecalculatePaintOffset = true;
		}

		if(iTextAnchorPosition != -1
			&& SetTextAnchorPosition(iTextAnchorPosition))
		{
			bRecalculatePaintOffset = true;
		}

		if((iTextAlignHoriz != -1 || iTextAlignVert != -1)
			&& SetTextAlignment(iTextAlignHoriz, iTextAlignVert))
		{
			bRecalculatePaintOffset = true;
		}
		
		if((iTextX != -9999 || iTextY != -9999)
			&& SetTextPositionOffset(iTextX, iTextY))
		{
			bRecalculatePaintOffset = true;
		}

		if(iTextColorMode != -1)
		{
			SetTextColorMode(iTextColorMode);
		}

		if(iTextRed != -1 && iTextGreen != -1 && iTextBlue != -1 && iTextAlpha != -1)
		{
			SetCustomTextColor(iTextRed, iTextGreen, iTextBlue, iTextAlpha);
		}

		int iShowPanel = GetInt("showPanel", kvStyleData, kvDefaultStyleData);
		if(iShowPanel != -1)
		{
			m_bShowPanel = iShowPanel == 1;
		}

		int iItemMarginHorizontal = GetInt("itemMarginHorizontal", kvStyleData, kvDefaultStyleData);
		int iItemMarginVertical = GetInt("itemMarginVertical", kvStyleData, kvDefaultStyleData);
		if((m_iItemMarginHorizontal != iItemMarginHorizontal && iItemMarginHorizontal != -1) || (m_iItemMarginVertical != iItemMarginVertical && iItemMarginVertical != -1))
		{
			m_iItemMarginHorizontal = iItemMarginHorizontal;
			m_iItemMarginVertical = iItemMarginVertical;
			bRecalculateItemPositions = true;
		}

		for(int i = 0; i < m_DarQuantityItems.GetCount(); ++i)
		{
			m_DarQuantityItems[i]->SetStyle(kvStyleData, kvDefaultStyleData);
		}

		if(bRecalculateItemPositions)
		{
			RecalculateItemPositions();
		}

		if(bRecalculatePaintOffset)
		{
			RecalculatePaintOffset();
		}
	}

	bool FFQuantityPanel::SetPanelColorMode( int iColorMode )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iPanelColorMode, iColorMode);

		if(bHasChanged)
		{
			RecalculateColor(m_clrPanel, m_iPanelColorMode, m_clrPanelCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetHeaderIconColorMode( int iColorMode )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iHeaderIconColorMode, iColorMode);

		if(bHasChanged)
		{
			RecalculateColor(m_clrHeaderIcon, m_iHeaderIconColorMode, m_clrHeaderIconCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetHeaderTextColorMode( int iColorMode )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iHeaderTextColorMode, iColorMode);

		if(bHasChanged)
		{
			RecalculateColor(m_clrHeaderText, m_iHeaderTextColorMode, m_clrHeaderTextCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetTextColorMode( int iColorMode )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iTextColorMode, iColorMode);

		if(bHasChanged)
		{
			RecalculateColor(m_clrText, m_iTextColorMode, m_clrTextCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetCustomPanelColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrPanelCustom, clrCustom);

		if(bHasChanged)
		{
			RecalculateColor(m_clrPanel, m_iPanelColorMode, m_clrPanelCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetCustomHeaderIconColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrHeaderIconCustom, clrCustom);

		if(bHasChanged)
		{
			RecalculateColor(m_clrHeaderIcon, m_iHeaderIconColorMode, m_clrHeaderIconCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetCustomHeaderTextColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrHeaderTextCustom, clrCustom);

		if(bHasChanged)
		{
			RecalculateColor(m_clrHeaderText, m_iHeaderTextColorMode, m_clrHeaderTextCustom);
		}

		return bHasChanged;
	}

	bool FFQuantityPanel::SetCustomTextColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrTextCustom, clrCustom);

		if(bHasChanged)
		{
			RecalculateColor(m_clrText, m_iTextColorMode, m_clrTextCustom);
		}

		return bHasChanged;
	}

	void FFQuantityPanel::RecalculateColor( Color &clr, int iColorMode, Color &clrCustom )
	{
		Color rgbColor = GetRgbColor(iColorMode, clrCustom);

		SetColor(clr, rgbColor, clrCustom);
	}

	Color FFQuantityPanel::GetRgbColor( int iColorMode, Color &clrCustom )
	{
		switch (iColorMode)
		{
		case FFQuantityHelper::COLOR_MODE_TEAM:
			return m_clrTeam;

		case FFQuantityHelper::COLOR_MODE_CUSTOM:
		default:
			return clrCustom;
		}
	}

	void FFQuantityPanel::SetColor( Color &clr, Color &clrRgb, Color &clrAlpha )
	{
		clr.SetColor(
			clrRgb.r(),
			clrRgb.g(),
			clrRgb.b(),
			clrAlpha.a());
	}

	int FFQuantityPanel::GetInt(const char *keyName, KeyValues *kvStyleData, KeyValues *kvDefaultStyleData, int iDefaultValue)
	{
		return FFQuantityHelper::GetInt(keyName, kvStyleData, kvDefaultStyleData, iDefaultValue);
	}

	void FFQuantityPanel::OnStyleDataRecieved( KeyValues *kvStyleData )
	{
		ApplyStyleData( kvStyleData );
	}

	void FFQuantityPanel::OnPresetPreviewDataRecieved( KeyValues *kvPresetPreviewData )
	{
		ApplyStyleData( kvPresetPreviewData, false );
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