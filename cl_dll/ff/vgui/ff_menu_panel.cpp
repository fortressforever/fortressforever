/********************************************************************
	created:	2006/09/13
	created:	13:9:2006   13:04
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_menu_panel.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_menu_panel
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	A menu thing. This could probably be merged with ffpanel
*********************************************************************/


#include "cbase.h"

#include <KeyValues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui/IScheme.h>

#include "c_ff_player.h"
#include "ff_menu_panel.h"
#include "ff_utils.h"

#define ORIENT_NORMAL	0
#define ORIENT_FLIPH	1
#define ORIENT_FLIPV	2

namespace vgui
{
	//-----------------------------------------------------------------------------
	// Purpose: Load the textures
	//-----------------------------------------------------------------------------
	void FFMenuPanel::ApplySettings(KeyValues *inResourceData)
	{
		const char *pszCornerBorder		= inResourceData->GetString("CornerBorderTexture", "Border_Corner");
		const char *pszHorizontalBorder	= inResourceData->GetString("HorizontalBorderTexture", "Border_TB");
		const char *pszVerticalBorder	= inResourceData->GetString("VerticalBorderTexture", "Border_LR");
		const char *pszBackground		= inResourceData->GetString("BackgroundTexture", "BG_Fill");

		LoadTextures(pszCornerBorder, m_sCornerBorder);
		LoadTextures(pszHorizontalBorder, m_sHorizontalBorder);
		LoadTextures(pszVerticalBorder, m_sVerticalBorder);

		LoadTexture(VarArgs("vgui/%s.vtf", pszBackground), &m_pBackgroundTexture);

		Panel::ApplySettings(inResourceData);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load the colours
	//-----------------------------------------------------------------------------
	void FFMenuPanel::ApplySchemeSettings(IScheme *pScheme)
	{
		//m_HudForegroundColour = GetSchemeColor("HudItem.Foreground", pScheme);
		//m_HudBackgroundColour = GetSchemeColor("HudItem.Background", pScheme);

		Panel::ApplySchemeSettings(pScheme);
	}

	//-----------------------------------------------------------------------------
	// Purpose: The background consists of a separate foreground and background
	//			texture.
	//-----------------------------------------------------------------------------
	void FFMenuPanel::PaintBackground()
	{
		int iStyle = 0;	// Normal for now

		int iFarRight = GetWide() - m_iRightBorderWidth;
		int iFarBottom = GetTall() - m_iBottomBorderWidth;
		int iHorizontalLineLength = iFarRight - m_iLeftBorderWidth;
		int iVerticalLineLength = iFarBottom - m_iTopBorderWidth;

		//
		// Background pieces
		//
		surface()->DrawSetColor(GetBgColor());

		// Corners (TL, TR, BL, BR)
		DrawTexture(m_sCornerBorder.m_pBackTexture[iStyle], 0, 0, m_iLeftBorderWidth, m_iTopBorderWidth, ORIENT_NORMAL);
		DrawTexture(m_sCornerBorder.m_pBackTexture[iStyle], iFarRight, 0, m_iRightBorderWidth, m_iTopBorderWidth, ORIENT_FLIPH);
		DrawTexture(m_sCornerBorder.m_pBackTexture[iStyle], 0, iFarBottom, m_iLeftBorderWidth, m_iBottomBorderWidth, ORIENT_FLIPV);
		DrawTexture(m_sCornerBorder.m_pBackTexture[iStyle], iFarRight, iFarBottom, m_iRightBorderWidth, m_iBottomBorderWidth, ORIENT_FLIPH|ORIENT_FLIPV);

		// Lines (T, B, L, R)
		DrawTexture(m_sHorizontalBorder.m_pBackTexture[iStyle], m_iLeftBorderWidth, 0, iHorizontalLineLength, m_iTopBorderWidth, ORIENT_NORMAL);
		DrawTexture(m_sHorizontalBorder.m_pBackTexture[iStyle], m_iLeftBorderWidth, iFarBottom, iHorizontalLineLength, m_iBottomBorderWidth, ORIENT_FLIPV);
		DrawTexture(m_sVerticalBorder.m_pBackTexture[iStyle], 0, m_iTopBorderWidth, m_iLeftBorderWidth, iVerticalLineLength, ORIENT_NORMAL);
		DrawTexture(m_sVerticalBorder.m_pBackTexture[iStyle], iFarRight, m_iTopBorderWidth, m_iRightBorderWidth, iVerticalLineLength, ORIENT_FLIPH);

		// Middle
		DrawTexture(m_pBackgroundTexture, m_iLeftBorderWidth, m_iRightBorderWidth, iHorizontalLineLength, iVerticalLineLength, ORIENT_NORMAL);

		//
		// Shadow pieces
		//
		if (m_iShadowOffset > 0 && !m_bHideShadow)
		{
			surface()->DrawSetColor(Color(0, 0, 0, 80));

			// We have to draw the shadow on our parent since we're drawing
			// outside of our clipping box.
			surface()->PushMakeCurrent(GetVParent(), false);

			int iShadowOffsetX, iShadowOffsetY;
			GetPos(iShadowOffsetX, iShadowOffsetY);

			iShadowOffsetX += m_iShadowOffset;
			iShadowOffsetY += m_iShadowOffset;

			// Corners (TR, BL, BR)
			DrawTexture(m_sCornerBorder.m_pShadowTexture[iStyle], iFarRight + iShadowOffsetX, iShadowOffsetY, m_iRightBorderWidth, m_iTopBorderWidth, ORIENT_FLIPH);
			DrawTexture(m_sCornerBorder.m_pShadowTexture[iStyle], iShadowOffsetX, iFarBottom + iShadowOffsetY, m_iLeftBorderWidth, m_iBottomBorderWidth, ORIENT_FLIPV);
			DrawTexture(m_sCornerBorder.m_pShadowTexture[iStyle], iFarRight + iShadowOffsetX, iFarBottom + iShadowOffsetY, m_iRightBorderWidth, m_iBottomBorderWidth, ORIENT_FLIPH|ORIENT_FLIPV);

			// Lines (B, R)
			DrawTexture(m_sHorizontalBorder.m_pShadowTexture[iStyle], m_iLeftBorderWidth + iShadowOffsetX, iFarBottom + iShadowOffsetY, iHorizontalLineLength, m_iBottomBorderWidth, ORIENT_FLIPV);
			DrawTexture(m_sVerticalBorder.m_pShadowTexture[iStyle], iFarRight + iShadowOffsetX, m_iTopBorderWidth + iShadowOffsetY, m_iRightBorderWidth, iVerticalLineLength, ORIENT_FLIPH);

			// Go back to drawing on this element
			surface()->PopMakeCurrent(GetVParent());
		}

		//
		// Foreground pieces
		//
		surface()->DrawSetColor(GetFgColor());

		// Corners (TL, TR, BL, BR)
		DrawTexture(m_sCornerBorder.m_pFrontTexture[iStyle], 0, 0, m_iLeftBorderWidth, m_iTopBorderWidth, ORIENT_NORMAL);
		DrawTexture(m_sCornerBorder.m_pFrontTexture[iStyle], iFarRight, 0, m_iRightBorderWidth, m_iTopBorderWidth, ORIENT_FLIPH);
		DrawTexture(m_sCornerBorder.m_pFrontTexture[iStyle], 0, iFarBottom, m_iLeftBorderWidth, m_iBottomBorderWidth, ORIENT_FLIPV);
		DrawTexture(m_sCornerBorder.m_pFrontTexture[iStyle], iFarRight, iFarBottom, m_iRightBorderWidth, m_iBottomBorderWidth, ORIENT_FLIPH|ORIENT_FLIPV);

		// Lines (T, B, L, R)
		DrawTexture(m_sHorizontalBorder.m_pFrontTexture[iStyle], m_iLeftBorderWidth, 0, iHorizontalLineLength, m_iTopBorderWidth, ORIENT_NORMAL);
		DrawTexture(m_sHorizontalBorder.m_pFrontTexture[iStyle], m_iLeftBorderWidth, iFarBottom, iHorizontalLineLength, m_iBottomBorderWidth, ORIENT_FLIPV);
		DrawTexture(m_sVerticalBorder.m_pFrontTexture[iStyle], 0, m_iTopBorderWidth, m_iLeftBorderWidth, iVerticalLineLength, ORIENT_NORMAL);
		DrawTexture(m_sVerticalBorder.m_pFrontTexture[iStyle], iFarRight, m_iTopBorderWidth, m_iRightBorderWidth, iVerticalLineLength, ORIENT_FLIPH);

		// Don't chain this.
		//Panel::PaintBackground();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void FFMenuPanel::LoadTextures(const char *pszName, BorderTexture_t &sTextures)
	{
		const char *pszResolutions[] = { "1024" };

		for (int i = 0; i < ARRAYSIZE(pszResolutions); i++)
		{
			LoadTexture(VarArgs("vgui/%s_BG%s.vtf", pszResolutions[i], pszName), &sTextures.m_pBackTexture[0]);
			LoadTexture(VarArgs("vgui/%s_%s.vtf", pszResolutions[i], pszName), &sTextures.m_pFrontTexture[0]);
			LoadTexture(VarArgs("vgui/%s_Shadow%s.vtf", pszResolutions[i], pszName), &sTextures.m_pShadowTexture[0]);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Create a new texture with pTexture
	//-----------------------------------------------------------------------------
	void FFMenuPanel::LoadTexture(const char *pszName, CHudTexture **pTexture)
	{
		if( *pTexture )
		{
			delete *pTexture;
			*pTexture = NULL;
		}

		Assert(*pTexture == NULL);

		*pTexture = new CHudTexture();
		(*pTexture)->textureId = surface()->CreateNewTextureID();
		surface()->DrawSetTextureFile((*pTexture)->textureId, pszName, true, false);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Draw the texture. Can be flipped and rotated 90 degrees
	//-----------------------------------------------------------------------------
	void FFMenuPanel::DrawTexture(const CHudTexture *pTexture, int x, int y, int w, int h, int sOrientation)
	{
		if (!pTexture)
			return;

		int dx = x + w;
		int dy = y + h;

		float s0 = 0.0f, t0 = 0.0f;
		float s1 = 1.0f, t1 = 1.0f;

		if (sOrientation & ORIENT_FLIPH)
		{
			s0 = 1.0f;
			s1 = 0.0f;
		}
		if (sOrientation & ORIENT_FLIPV)
		{
			t0 = 1.0f;
			t1 = 0.0f;
		}

		surface()->DrawSetTexture(pTexture->textureId);
		surface()->DrawTexturedSubRect(x, y, dx, dy, s0, t0, s1, t1);
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void FFMenuPanel::SetBorderWidth(int iBorderWidth)
	{
		m_iBorderWidth = iBorderWidth;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Set the shadow offset.
	//			The size of the box will need to be readjusted for this
	//-----------------------------------------------------------------------------
	void FFMenuPanel::SetShadowOffset(int iShadowOffset)
	{
		m_iShadowOffset = iShadowOffset;

		//BaseClass::SetSize(m_iRealWide + m_iShadowOffset, m_iRealTall + m_iShadowOffset);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Clean out the textures
	//-----------------------------------------------------------------------------
	void FFMenuPanel::InitFFMenuPanel()
	{
		memset(&m_sCornerBorder, 0, sizeof(m_sCornerBorder));
		memset(&m_sHorizontalBorder, 0, sizeof(m_sHorizontalBorder));
		memset(&m_sVerticalBorder, 0, sizeof(m_sVerticalBorder));

		m_pBackgroundTexture = NULL;
		m_iShadowOffset = 0;
		m_bHideShadow = false;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Temporarily hide a shadow (e.g. if disabled)
	//-----------------------------------------------------------------------------
	void FFMenuPanel::SetHideShadow(bool state)
	{
		m_bHideShadow = state;
	}
}
