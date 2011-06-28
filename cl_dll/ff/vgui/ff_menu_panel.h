/********************************************************************
	created:	2006/09/13
	created:	13:9:2006   13:02
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_menu_panel.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_menu_panel
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#ifndef FF_MENU_PANEL_H
#define FF_MENU_PANEL_H

#include "cbase.h"
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/Panel.h>
#include "color.h"

#define m_iTopBorderWidth m_iBorderWidth
#define m_iBottomBorderWidth m_iBorderWidth
#define m_iLeftBorderWidth m_iBorderWidth
#define m_iRightBorderWidth m_iBorderWidth

typedef struct BorderTexture_s
{
	CHudTexture *m_pFrontTexture[3];
	CHudTexture *m_pBackTexture[3];
	CHudTexture *m_pShadowTexture[3];

} BorderTexture_t;

namespace vgui
{
	class FFMenuPanel : public Panel
	{
		DECLARE_CLASS_SIMPLE(FFMenuPanel, Panel);

	public:
		FFMenuPanel() : BaseClass() { InitFFMenuPanel(); }
		FFMenuPanel(Panel *parent) : BaseClass(parent) { InitFFMenuPanel(); }
		FFMenuPanel(Panel *parent, const char *panelName) : BaseClass(parent, panelName) { InitFFMenuPanel(); }
		FFMenuPanel(Panel *parent, const char *panelName, HScheme scheme) : BaseClass(parent, panelName, scheme) { InitFFMenuPanel(); }

		void ApplySettings(KeyValues *inResourceData);
		void ApplySchemeSettings(IScheme *pScheme);
		void PaintBackground();

	public:

		//int GetTopOffset() { return m_iTopBorderWidth; }
		//int GetBottomOffset() { return m_iBottomBorderWidth; }
		//int GetLeftOffset() { return m_iLeftBorderWidth; }
		//int GetRightOffset() { return m_iRightBorderWidth; }

		void SetBorderWidth(int iBorderWidth);
		int GetBorderWidth() { return m_iBorderWidth; }

		void SetShadowOffset(int iShadowOffset);

		int GetInsetWidth() { return (m_iBorderWidth * 0.6f); }

		void SetHideShadow(bool state);

	private:

		void InitFFMenuPanel();
		void LoadTextures(const char *pszName, BorderTexture_t &sTextures);
		void LoadTexture(const char *pszName, CHudTexture **pTexture);

		void DrawTexture(const CHudTexture *pszTexture, int x, int y, int w, int h, int sOrientation);


	private:

		BorderTexture_t m_sCornerBorder;
		BorderTexture_t m_sHorizontalBorder;
		BorderTexture_t m_sVerticalBorder;

		CHudTexture *m_pBackgroundTexture;

	private:

		//CPanelAnimationVarAliasType(float, m_iTopBorderWidth, "topborderwidth", "12.8", "proportional_float");
		//CPanelAnimationVarAliasType(float, m_iBottomBorderWidth, "bottomborderwidth", "12.8", "proportional_float");
		//CPanelAnimationVarAliasType(float, m_iLeftBorderWidth, "leftborderwidth", "12.8", "proportional_float");
		//CPanelAnimationVarAliasType(float, m_iRightBorderWidth, "rightborderwidth", "12.8", "proportional_float");

		CPanelAnimationVarAliasType(float, m_iBorderWidth, "borderWidth", "10", "proportional_float");
		//CPanelAnimationVarAliasType(float, m_iShadowOffset, "shadowOffset", "10", "proportional_float");

		int		m_iShadowOffset;
		bool	m_bHideShadow;
	};
}

#endif