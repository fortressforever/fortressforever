/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   15:58
	filename: 	F:\cvs\code\cl_dll\ff\ff_hud_buildtimer.h
	file path:	F:\cvs\code\cl_dll\ff
	file base:	ff_hud_buildtimer
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"

class CHudBuildTimer : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildTimer, vgui::Panel);

	// Progress
	float	m_flStartTime, m_flDuration;
	wchar_t m_LabelText[32];
	bool	m_fVisible;

	// Stuff we need to know
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");

	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_xpos, "icon_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_ypos, "icon_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_width, "icon_width", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_height, "icon_height", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_xpos, "bar_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_ypos, "bar_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_width, "bar_width", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_height, "bar_height", "1", "proportional_float");
	CPanelAnimationVarAliasType(Color, bar_color, "bar_color", "255 255 255", "color");

	CHudTexture	*m_pHudElementTexture;
	CHudTexture *m_pHudBuildIcons[4];
	char *m_pszBuildLabels[4];

public:
	CHudBuildTimer(const char *pElementName) : CHudElement(pElementName), vgui::Panel(NULL, "HudBuildTimer") 
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits(/*HIDEHUD_HEALTH | */HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
	}

	~CHudBuildTimer();

	void	Init();
	void	VidInit();
	void	Paint();

	void	SetLabelText(const wchar_t *text);
	
	
	void	SetBuildTimer(int type, float duration);

	int		m_iIcon;

	// Callback functions for setting
	void	MsgFunc_FF_BuildTimer(bf_read &msg);
};