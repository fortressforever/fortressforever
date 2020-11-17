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

		m_iAnchorPosition = FFQuantityHelper::ANCHORPOS_TOPRIGHT;
		m_iAnchorWidth = 0;
		m_iAnchorHeight = 0;

		m_iAlignHorizontally = FFQuantityHelper::ALIGN_RIGHT;
		m_iAlignVertically = FFQuantityHelper::ALIGN_BOTTOM;

		m_iColorMode = FFQuantityHelper::COLOR_MODE_CUSTOM;
		m_clrCustom = Color(255, 255, 255, 255);
		m_clrText = Color(255, 255, 255, 255);

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
				m_iPositionX,
				m_iPositionY);
		}
	}

	void FFQuantityText::DrawText(
		wchar_t* wszText,
		HFont hfText,
		Color clrText,
		int iPositionX,
		int iPositionY)
	{
		surface()->DrawSetTextPos(iPositionX, iPositionY);
		surface()->DrawSetTextColor(clrText);
		surface()->DrawSetTextFont(hfText);
		surface()->DrawUnicodeString(wszText);
	}

	void FFQuantityText::RecalculateFont()
	{
		m_hfText 
			= FFQuantityHelper
				::GetFont(
					m_hfFamily,
					m_iSize,
					m_bDropShadow);
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
		float &flX0,
		float &flX1,
		float &flY0,
		float &flY1)
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
		int iAnchorOffsetX, iAnchorOffsetY;

		FFQuantityHelper
			::CalculateAnchorOffset(
				m_iAnchorPosition,
				iAnchorOffsetX,
				iAnchorOffsetY,
				m_iAnchorWidth,
				m_iAnchorHeight);

		surface()
			->GetTextSize(
				m_hfText,
				m_wszText,
				m_iWidth,
				m_iHeight);

		int iAlignmentOffsetX, iAlignmentOffsetY;

		FFQuantityHelper
			::CalculatePositionOffset(
				iAlignmentOffsetX,
				iAlignmentOffsetY,
				m_iWidth,
				m_iHeight,
				m_iAlignHorizontally,
				m_iAlignVertically);

		m_iPositionX 
			= m_iPositionOffsetX * m_flScaleX
				+ iAnchorOffsetX * m_flScaleX
				+ iAlignmentOffsetX;

		m_iPositionY 
			= m_iPositionOffsetY * m_flScaleY
				+ iAnchorOffsetY * m_flScaleY
				+ iAlignmentOffsetY;
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

	bool FFQuantityText::SetTextSize(int iSize)
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

		int iShow = GetInt("show", kvStyleData, kvDefaultStyleData);
		int iTextShadow = GetInt("dropShadow", kvStyleData, kvDefaultStyleData);
		int iTextSize = GetInt("size", kvStyleData, kvDefaultStyleData);
		int iTextAnchorPosition = GetInt("atextAnchorPosition", kvStyleData, kvDefaultStyleData);
		int iTextAlignHoriz = GetInt("alignHorizontally", kvStyleData, kvDefaultStyleData);
		int iTextAlignVert = GetInt("alignVertically", kvStyleData, kvDefaultStyleData);
		int iTextX = GetInt("X", kvStyleData, kvDefaultStyleData, -9999);
		int iTextY = GetInt("Y", kvStyleData, kvDefaultStyleData, -9999);
		int iTextColorMode = GetInt("colorMode", kvStyleData, kvDefaultStyleData);
		int iRed = GetInt("red", kvStyleData, kvDefaultStyleData);
		int iGreen = GetInt("green", kvStyleData, kvDefaultStyleData);
		int iBlue = GetInt("blue", kvStyleData, kvDefaultStyleData);
		int iAlpha = GetInt("alpha", kvStyleData, kvDefaultStyleData);

		if (iShow != -1 && SetTextVisible(iShow == 1))
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

		if (iColorMode != -1)
		{
			SetColorMode(iColorMode);
		}

		if (iRed != -1 && iGreen != -1 && iBlue != -1 && iAlpha != -1)
		{
			Color clrCustom 
				= Color(iRed, iGreen, iBlue, iAlpha);

			SetCustomColor(clrCustom);
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

	bool FFQuantityText::SetCustomColor(Color clrCustom)
	{
		bool bHasChanged = FFQuantityHelper::Change(m_clrCustom, clrCustom);

		if (bHasChanged)
		{
			RecalculateColor();
		}

		return bHasChanged;
	}

	void FFQuantityText::RecalculateColor()
	{
		Color clrRgb;

		switch (m_iColorMode)
		{
		case FFQuantityHelper::COLOR_MODE_TEAM:
			clrRgb = m_clrTeam;
			break;

		case FFQuantityHelper::COLOR_MODE_CUSTOM:
		default:
			clrRgb = m_clrCustom;
		}

		m_clrText
			.SetColor(
				clrRgb.r(),
				clrRgb.g(),
				clrRgb.b(),
				m_clrCustom.a());
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