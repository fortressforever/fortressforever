/********************************************************************
	created:	2020/11
	filename: 	cl_dll\ff\vgui\ff_quantitytext.cpp
	file path:	cl_dll\ff\vgui
	file base:	ff_quantitytext
	file ext:	cpp
	author:		Elmo

	purpose:	Quanitity panel component to render customised text
*********************************************************************/

#include "cbase.h"
#include "ff_quantitytext.h"
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
	FFQuantityText::FFQuantityText(Panel *parent, const char *panelName) : BaseClass(parent, panelName)
	{
		m_flScaleX = 1.0f;
		m_flScaleY = 1.0f;

		m_iPositionOffsetX = 0;
		m_iPositionOffsetY = 0;

		m_iAnchorPosition = FFQuantityHelper::ANCHORPOS_TOPRIGHT;
		m_iAnchorWidth = 0;
		m_iAnchorHeight = 0;

		m_iAlignHoriz = FFQuantityHelper::ALIGN_RIGHT;
		m_iAlignVert = FFQuantityHelper::ALIGN_BOTTOM;

		m_iColorMode = FFQuantityHelper::COLOR_MODE_CUSTOM;
		m_clrCustom = Color(255, 255, 255, 255);
		m_clrDisplay = Color(255, 255, 255, 255);

		m_iSize = 4;
		m_bShow = true;
		m_bDropShadow = false;
	}

	KeyValues* FFQuantityText::AssignDefaultStyleData(KeyValues *kvStyleData)
	{
		kvStyleData->SetInt("show", 1);
		kvStyleData->SetInt("size", 4);
		kvStyleData->SetInt("shadow", 0);
		kvStyleData->SetInt("anchorPosition",  FFQuantityHelper::ANCHORPOS_TOPLEFT);
		kvStyleData->SetInt("alignHorizontal",  FFQuantityHelper::ALIGN_LEFT);
		kvStyleData->SetInt("alignVertical",  FFQuantityHelper::ALIGN_TOP);
		kvStyleData->SetInt("x", 0);
		kvStyleData->SetInt("y", 0);
		kvStyleData->SetInt("colorMode", FFQuantityHelper::COLOR_MODE_CUSTOM);
		kvStyleData->SetInt("colorRed", 255);
		kvStyleData->SetInt("colorGreen", 255);
		kvStyleData->SetInt("colorBlue", 255);
		kvStyleData->SetInt("colorAlpha", 255);
	}

	void FFQuantityText::Paint()
	{
		VPROF_BUDGET("FFQuantityText::Paint","QuantityText");

		BaseClass::Paint();

		if (m_bShow && m_wszText)
		{
			DrawText(
				m_wszText,
				m_hfText,
				m_clrText,
				m_iOffsetX + m_iPositionX,
				m_iOffsetY + m_iPositionY);
		}
	}

	void FFQuantityText::DrawText(
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

	void FFQuantityText::RecalculateHeaderIconFont()
	{
		HFont font
			= FFQuantityHelper
				::GetFont(
					m_hfFamily,
					m_iSize,
					m_bShadow);

		m_hfText = font;
	}

	vgui::HFont FFQuantityText::GetFont(
		vgui::HFont* hfFamily,
		int iSize,
		bool bUseModifier)
	{
		return ;
	}

	void FFQuantityText::CalculateAnchorPosition(
		int &iX,
		int &iY,
		int iAnchorPos)
	{
		int iAlignHoriz, iAlignVert;

		FFQuantityHelper
			::ConvertToAlignment(
				iAnchorPos,
				iAlignHoriz,
				iAlignVert);
		
		int iWide = m_iAnchorWidth * m_flScaleX;
		int iTall = m_iAnchorHeight * m_flScaleY;

		FFQuantityHelper
			::CalculatePositionOffset(
				iX, 
				iY, 
				iWide, 
				iTall,
				iAlignHoriz,
				iAlignVert);
	}

	void FFQuantityText::CalculateAlignmentOffset(
		int &iX,
		int &iY,
		int &iWide,
		int &iTall,
		int iAlignHoriz,
		int iAlignVert,
		HFont hfFont,
		wchar_t* wszString)
	{
		surface()
			->GetTextSize(
				hfFont,
				wszString,
				iWide,
				iTall);

		FFQuantityHelper
			::CalculatePositionOffset(
				iX, 
				iY, 
				iWide, 
				iTall,
				iAlignHoriz,
				iAlignVert);

		iX = iX * -1;
		iY = iY * -1;
	}

	bool FFQuantityText::SetTeamColor(Color clrTeam)
	{
		bool bHasChanged 
			= FFQuantityHelper
				::Change(
					m_clrTeam, 
					clrTeam);
		
		if (bHasChanged)
		{
			RecalculateColor();
		}

		return bHasChanged;
	}

	void FFQuantityText::OnTick()
	{
		VPROF_BUDGET("FFQuantityText::OnTick","QuantityItems");

		if (m_bAddToHud && !m_bAddToHudSent)
		{
			//put the global check separately from the above case
			//just in case it uses more resources
			if (g_AP != NULL && g_AP->IsReady())
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

		C_BasePlayer *pBasePlayer 
			= CBasePlayer::GetLocalPlayer()

		C_FFPlayer *pPlayer 
			= ToFFPlayer(pBasePlayer);

		int iTeamNumber 
			= pPlayer->GetTeamNumber();

		Color clrTeam 
			= g_PR->GetTeamColor(iTeamNumber);

		SetTeamColor(clrTeam);

		// Get the screen width/height
		int iScreenWide, iScreenTall;
		surface()
			->GetScreenSize(
				iScreenWide,
				iScreenTall);

		// "map" screen res to 640/480
		float flScaleX 
			= iScreenWide / 640.0f;

		float flScaleY 
			= iScreenTall / 480.0f;

		if ( m_flScaleX != flScaleX 
			|| m_flScaleY != flScaleY)
		{
			m_flScaleX = flScaleX;
			m_flScaleY = flScaleY;
		}
	}

	KeyValues* FFQuantityText::AddPanelSpecificOptions(KeyValues *kvPanelSpecificOptions)
	{
		return NULL;
	}

	KeyValues* FFQuantityText::AddItemStyleList(KeyValues *kvItemStyleList)
	{
		for(int i = 0; i < m_DarQuantityItems.Count(); ++i)
		{
			kvItemStyleList->SetString(m_DarQuantityItems[i]->GetName(), "Default");
		}

		return kvItemStyleList;
	}

	void FFQuantityText::RecalculatePaintOffset(
		fl &flX0,
		int &flX1,
		int &flY0,
		int &flY1)
	{
		if (m_bShow) 
		{
			return;
		}

		if (m_iPositionX < flX0)
		{
			flX0 = m_iPositionX;
		}

		if (m_iPositionY < flY0)
		{
			flX0 = m_iPositionX;
		}

		int x1 
			= m_iPositionX + m_iWidth

		if ( x1 > flX1 )
		{
			flX1 = x1;
		}

		int y1 
			= m_iPositionY + m_iHeight

		if ( y1 > flY1 )
		{
			flX1 = x1;
		}
	}

	void FFQuantityText::RecalculatePosition( )
	{
		CalculateTextAnchorPosition(m_iAnchorPositionX, m_iAnchorPositionY, m_iAnchorPosition);

		CalculateTextAlignmentOffset(m_iAlignmentOffsetX, m_iAlignmentOffsetY, m_iWidth, m_iHeight, m_iAlignHoriz, m_iAlignVert, m_hfText, m_wszText);

		m_iPositionX = m_iAnchorPositionX + m_iAlignmentOffsetX + m_iPositionOffsetX * m_flScaleX;
		m_iPositionY = m_iAnchorPositionY + m_iAlignmentOffsetY + m_iPositionOffsetY * m_flScaleY;
	}

	void FFQuantityText::SetText(
		wchar_t *newText,
		bool bRecalculatePaintOffset)
	{
		wcscpy(m_wszText, newText);

		RecalculateTextPosition();

		if (bRecalculatePaintOffset)
		{
			RecalculatePaintOffset();
		}
	}

	bool FFQuantityText::SetTextSize( int iSize )
	{
		bool bHasChanged = FFQuantityHelper::Change(m_iTextSize, iSize);

		if (bHasChanged)
		{
			RecalculateTextFont();
			RecalculateTextPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityText::SetShowText( bool bShow )
	{
		return FFQuantityHelper::Change(m_bShowText, bShow);
	}

	bool FFQuantityText::SetTextPositionOffset(
		int iOffsetX,
		int iOffsetY)
	{
		bool bOffsetXChanged = FFQuantityHelper::Change(m_iTextPositionOffsetX, iOffsetX);
		bool bOffsetYChanged = FFQuantityHelper::Change(m_iTextPositionOffsetY, iOffsetY);

		if (bOffsetXChanged || bOffsetYChanged)
		{
			RecalculateTextPosition();
		}

		return bOffsetXChanged 
			|| bOffsetYChanged;
	}

	bool FFQuantityText::SetTextAlignment(
		int iAlignHoriz,
		int iAlignVert )
	{
		bool bHorizChanged 
			= FFQuantityHelper
				::Change(
					m_iTextAlignHoriz, 
					iAlignHoriz);

		bool bVertChanged 
			= FFQuantityHelper
				::Change(
					m_iTextAlignVert,
					iAlignVert);

		if (bHorizChanged || bVertChanged)
		{
			RecalculateTextPosition();
		}

		return bHorizChanged 
			|| bVertChanged;
	}

	bool FFQuantityText::SetTextAnchorPosition(
		int iAnchorPosition)
	{
		bool bHasChanged 
			= FFQuantityHelper
				::Change(
					m_iTextAnchorPosition,
					iAnchorPosition);

		if (bHasChanged)
		{
			RecalculateTextPosition();
		}

		return bHasChanged;
	}

	bool FFQuantityText::SetTextShadow( bool bDropShadow )
	{
		bool bHasChanged 
			= FFQuantityHelper
				::Change(
					m_bDropShadow,
					bDropShadow);

		if (bHasChanged)
		{
			RecalculateTextFont();
			RecalculateTextPosition();
		}

		return bHasChanged;
	}

	void FFQuantityText::ApplyStyleData(
		KeyValues *kvStyleData,
		bool useDefaults)
	{
		KeyValues* kvDefaultStyleData 
			= useDefaults 
				? GetDefaultStyleData()
				: new KeyValues("styleData");

		bool bRecalculatePaintOffset = false;

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

		if (iShowText != -1 && SetTextVisible(iShowText == 1))
		{
			bRecalculatePaintOffset = true;
		}

		if (iTextShadow != -1
			&& SetTextShadow(iTextShadow == 1))
		{
			bRecalculatePaintOffset = true;
		}

		if (iTextSize != -1
			&& SetTextSize(iTextSize))
		{
			bRecalculatePaintOffset = true;
		}

		if (iTextAnchorPosition != -1 
			&& SetTextAnchorPosition(iTextAnchorPosition))
		{
			bRecalculatePaintOffset = true;
		}

		if ((iTextAlignHoriz != -1 || iTextAlignVert != -1) 
			&& SetTextAlignment(iTextAlignHoriz, iTextAlignVert))
		{
			bRecalculatePaintOffset = true;
		}
		
		if ((iTextX != -9999 || iTextY != -9999) 
			&& SetTextPositionOffset(iTextX, iTextY))
		{
			bRecalculatePaintOffset = true;
		}

		if (iTextColorMode != -1)
		{
			SetTextColorMode(iTextColorMode);
		}

		if (iTextRed != -1 && iTextGreen != -1 && iTextBlue != -1 && iTextAlpha != -1)
		{
			SetCustomTextColor(iTextRed, iTextGreen, iTextBlue, iTextAlpha);
		}

		if (bRecalculatePaintOffset)
		{
			RecalculatePaintOffset();
		}
	}

	bool FFQuantityText::SetColorMode(
		int iColorMode)
	{
		bool bHasChanged 
			= FFQuantityHelper
				::Change(
					m_iColorMode,
					iColorMode);

		if (bHasChanged)
		{
			RecalculateColor();
		}

		return bHasChanged;
	}

	bool FFQuantityText::SetCustomColor( int iRed, int iGreen, int iBlue, int iAlpha )
	{
		Color clrCustom = Color(iRed, iGreen, iBlue, iAlpha);
		bool bHasChanged = FFQuantityHelper::Change(m_clrCustom, clrCustom);

		if (bHasChanged)
		{
			RecalculateColor();
		}

		return bHasChanged;
	}

	void FFQuantityText::RecalculateDisplayColor()
	{
		Color rgbColor 
			= GetRgbColor(m_iColorMode, m_clrCustom);

		SetColor(m_clrDisplay, rgbColor, m_clrCustom);
	}

	Color FFQuantityText::GetRgbColor(
		int iColorMode,
		Color &clrCustom)
	{
		switch (iColorMode)
		{
		case COLOR_MODE_TEAM:
			return m_clrTeam;

		case COLOR_MODE_CUSTOM:
		default:
			return clrCustom;
		}
	}

	void FFQuantityText::SetColor(
		Color &clrDestination,
		Color clrRgb,
		Color clrAlpha)
	{
		clrDestination.SetColor(
			clrRgb.r(),
			clrRgb.g(),
			clrRgb.b(),
			clrAlpha.a());
	}

	int FFQuantityText::GetInt(const char *keyName, KeyValues *kvStyleData, KeyValues *kvDefaultStyleData, int iDefaultValue)
	{
		return FFQuantityHelper::GetInt(keyName, kvStyleData, kvDefaultStyleData, iDefaultValue);
	}

	void FFQuantityText::OnStyleDataRecieved( KeyValues *kvStyleData )
	{
		ApplyStyleData( kvStyleData );
	}

	void FFQuantityText::OnPresetPreviewDataRecieved( KeyValues *kvPresetPreviewData )
	{
		ApplyStyleData( kvPresetPreviewData, false );
	}

	void FFQuantityText::OnDefaultStyleDataRequested( KeyValues *data )
	{
		g_AP->CreatePresetFromPanelDefault(GetDefaultStyleData());
	}
}