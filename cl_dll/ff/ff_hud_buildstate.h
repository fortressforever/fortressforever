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

#include "hudelement.h"
//#include "ff_hud_hint.h"
#include "hud_macros.h"

#include "iclientmode.h" //to set panel parent as the cliends viewport
#include "c_ff_player.h" //required to cast base player

#include <vgui_controls/Panel.h>
#include <vgui/ISurface.h>
#include <vgui/IVGui.h>
#include <vgui/ILocalize.h>
//#include <vgui/ISystem.h>
//#include <vgui_controls/AnimationController.h>

//#include "c_ff_buildableobjects.h"
//#include "ff_buildableobjects_shared.h"

using namespace vgui;

enum {
	RESET_PIPES=0,
	INCREMENT_PIPES,
	DECREMENT_PIPES
};

class CHudBuildState : public CHudElement, public Panel
{
private:
	DECLARE_CLASS_SIMPLE(CHudBuildState, Panel);

	// Stuff we need to know
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "ChatFont");

	CPanelAnimationVarAliasType(float, text1_xpos, "text1_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text1_ypos, "text1_ypos", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "20", "proportional_float");
	CPanelAnimationVarAliasType(float, icon1_xpos, "icon1_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon1_ypos, "icon1_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon1_width, "icon1_width", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, icon1_height, "icon1_height", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, icon2_xpos, "icon2_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon2_ypos, "icon2_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, icon2_width, "icon2_width", "1", "proportional_float");
	CPanelAnimationVarAliasType(float, icon2_height, "icon2_height", "1", "proportional_float");

	CHudTexture	*m_pHudElementTexture;

	// Results of localising strings
	wchar_t m_szHealth[32];
	wchar_t m_szArmor[32];
	wchar_t m_szAmmo[32];
	//wchar_t m_szNoRockets[32];

	// Icons
	CHudTexture *m_pHudSentryLevel1;
	CHudTexture *m_pHudSentryLevel2;
	CHudTexture *m_pHudSentryLevel3;
	CHudTexture *m_pHudDispenser;
	CHudTexture *m_pHudManCannon;
	CHudTexture *m_pHudDetpack;
	CHudTexture *m_pHudPipes;

	// Lines of information
	wchar_t m_szDispenser[128];
	wchar_t m_szSentry[128];
	wchar_t m_szManCannon[128];
	wchar_t m_szDetpack[128];
	wchar_t m_szPipes[128];

	bool m_bDrawDispenser;
	bool m_bDrawSentry;
	bool m_bDrawManCannon;
	bool m_bDrawDetpack;
	bool m_bDrawPipes;

	int m_iSentryLevel;
    float m_flManCannonTimeoutTime;
    float m_flDetpackDetonateTime;
	int m_iNumPipes;

	void MsgFunc_DispenserMsg(bf_read &msg);
	void MsgFunc_SentryMsg(bf_read &msg);
	void MsgFunc_ManCannonMsg(bf_read &msg);
	void MsgFunc_DetpackMsg(bf_read &msg);
	void MsgFunc_PipeMsg(bf_read &msg);

public:
	CHudBuildState(const char *pElementName);

	~CHudBuildState();

	void	Init();
	void	VidInit();
	void	Paint();

	void	OnTick();
};

DECLARE_HUDELEMENT(CHudBuildState);
DECLARE_HUD_MESSAGE(CHudBuildState, DispenserMsg);
DECLARE_HUD_MESSAGE(CHudBuildState, SentryMsg);
DECLARE_HUD_MESSAGE(CHudBuildState, ManCannonMsg);
DECLARE_HUD_MESSAGE(CHudBuildState, DetpackMsg);
DECLARE_HUD_MESSAGE(CHudBuildState, PipeMsg);