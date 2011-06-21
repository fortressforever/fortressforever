#ifndef FF_HUD_BUILDTIMER_H
#define FF_HUD_BUILDTIMER_H

#include "cbase.h"
#include <vgui_controls/Panel.h>

using namespace vgui;

class CHudBuildTimer : public CHudElement, public Panel
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildTimer, Panel);

	CPanelAnimationVarAliasType(float, bar_xpos, "bar_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_ypos, "bar_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_width, "bar_width", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, bar_height, "bar_height", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, icon_offset, "icon_offset", "2", "proportional_float");

	Color m_HudForegroundColour;
	Color m_HudBackgroundColour;
	Color m_TeamColorHudBackgroundColour;

	float	m_flStartTime, m_flDuration;
	bool	m_fVisible;
	int		m_iBuildType, m_iTeam;

	CHudTexture *m_pIconTexture; // the one being used

	CHudTexture *m_pDispenserIconTexture;
	CHudTexture *m_pSentrygunIconTexture;
	CHudTexture *m_pDetpackIconTexture;
	CHudTexture *m_pMancannonIconTexture;

public:
	CHudBuildTimer(const char *pElementName);

	~CHudBuildTimer();

	virtual void	Init();
	virtual void	VidInit();
	virtual void	Paint();
	virtual void	PaintBackground();
	virtual void	OnTick();
	virtual void	ApplySchemeSettings(IScheme *pScheme);

	void	SetBuildTimer(int iBuildType, float flDuration);

	// Callback functions for setting
	void	MsgFunc_FF_BuildTimer(bf_read &msg);
};

#endif