#ifndef FF_HUD_GRENADE1TIMER_H
#define FF_HUD_GRENADE1TIMER_H

#include "cbase.h"
#include "ff_panel.h"

using namespace vgui;

class CHudGrenade1Timer : public CHudElement, public FFPanel
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

	DECLARE_CLASS_SIMPLE(CHudGrenade1Timer, FFPanel);

	CUtlLinkedList<timer_t> m_Timers;
	int m_iClass;
	int m_iPlayerTeam;
	bool m_fVisible;
	float m_flLastTime;

	CHudTexture *m_pIconTexture;

	CPanelAnimationVarAliasType(float, bar_xpos, "bar_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_ypos, "bar_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_width, "bar_width", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_height, "bar_height", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_offset, "icon_offset", "2", "proportional_float");
	CPanelAnimationVar( Color, icon_color, "icon_color", "HUD_Tone_Default" );

public:
	CHudGrenade1Timer(const char *pElementName);

	~CHudGrenade1Timer();

	virtual void	Init();
	virtual void	Paint();
	virtual void	PaintBackground();
	virtual void	OnTick();

	void	SetTimer(float duration);
	bool	ActiveTimer( void ) const;
	void	ResetTimer( void );

	// Callback functions for setting
	void	MsgFunc_FF_Grenade1Timer(bf_read &msg);
};

#endif