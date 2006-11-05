/********************************************************************
	created:	2006/02/04
	created:	4:2:2006   15:58
	filename: 	F:\cvs\code\cl_dll\ff\ff_hud_buildtimer.h
	file path:	F:\cvs\code\cl_dll\ff
	file base:	ff_hud_buildtimer
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	For now this is just a straight copy
*********************************************************************/

#ifndef FF_HUD_GRENADE2TIMER_H
#define FF_HUD_GRENADE2TIMER_H

#include "cbase.h"
#include "ff_hud_grenade1timer.h"
#include "ff_panel.h"

class CHudGrenade2Timer : public CHudElement, public vgui::FFPanel
{
private:
	DECLARE_CLASS_SIMPLE(CHudGrenade2Timer, vgui::FFPanel);

	CUtlLinkedList<timer_t> m_Timers;
	bool m_fVisible;
	float m_flLastTime;

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

	//CHudTexture	*m_pHudElementTexture;

public:
	CHudGrenade2Timer(const char *pElementName) : CHudElement(pElementName), vgui::FFPanel(NULL, "HudGrenade2Timer") 
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits(/*HIDEHUD_HEALTH | */HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
	}

	~CHudGrenade2Timer();

	void	Init();
	void	VidInit();
	void	Paint();

	void	SetTimer(float duration);
	bool	ActiveTimer( void ) const;
	void	ResetTimer( void );

	// Callback functions for setting
	void	MsgFunc_FF_Grenade1Timer(bf_read &msg);
};

#endif