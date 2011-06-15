#ifndef FF_HUD_GRENADE1TIMER_H
#define FF_HUD_GRENADE1TIMER_H

#include "cbase.h"
#include "ff_panel.h"


class CHudGrenade1Timer : public CHudElement, public vgui::FFPanel
{
private:
	typedef struct timer_s
	{
		float m_flStartTime;
		float m_flDuration;

		timer_s(float s, float d) 
		{
			m_flStartTime = s;
			m_flDuration = d;
		}

	} timer_t;

	DECLARE_CLASS_SIMPLE(CHudGrenade1Timer, vgui::FFPanel);

	CUtlLinkedList<timer_t> m_Timers;
	int m_iClass;
	bool m_fVisible;
	float m_flLastTime;
	CHudTexture *iconTexture;

	CPanelAnimationVarAliasType(float, bar_xpos, "bar_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_ypos, "bar_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_width, "bar_width", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_height, "bar_height", "1", "proportional_float");
	CPanelAnimationVarAliasType(Color, bar_color, "bar_color", "255 255 255", "color");
	CPanelAnimationVarAliasType(float, icon_xpos, "icon_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_ypos, "icon_ypos", "0", "proportional_float");

public:
	CHudGrenade1Timer(const char *pElementName) : CHudElement(pElementName), vgui::FFPanel(NULL, "HudGrenade1Timer") 
	{
		SetParent( g_pClientMode->GetViewport() );
		SetHiddenBits( HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION );
	}

	~CHudGrenade1Timer();

	void	Init();
	void	Paint();
	void	OnTick();

	void	SetTimer(float duration);
	bool	ActiveTimer( void ) const;
	void	ResetTimer( void );

	// Callback functions for setting
	void	MsgFunc_FF_Grenade1Timer(bf_read &msg);
};

#endif