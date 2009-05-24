/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   18:34
	filename: 	f:\cvs\code\cl_dll\ff\ff_hud_buildstate.h
	file path:	f:\cvs\code\cl_dll\ff
	file base:	ff_hud_buildstate
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/


#include "cbase.h"

class CHudBuildState : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildState, vgui::Panel);

	

	// Stuff we need to know
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");

	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_xpos, "icon_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_ypos, "icon_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_width, "icon_width", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_height, "icon_height", "1", "proportional_float");

	CHudTexture	*m_pHudElementTexture;

	int m_iHealthPerc, m_iAmmoPerc;
	bool m_fNoRockets;

public:
	CHudBuildState(const char *pElementName) : CHudElement(pElementName), vgui::Panel(NULL, "HudBuildState") 
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits(/*HIDEHUD_HEALTH | */HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
	}

	~CHudBuildState();

	void	Init();
	void	VidInit();
	void	Paint();

	void	OnTick();
};