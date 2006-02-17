/********************************************************************
	created:	2006/02/12
	created:	12:2:2006   1:04
	filename: 	F:\ff-svn\code\trunk\cl_dll\ff\ff_hud_menu.h
	file path:	F:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_hud_menu
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#include "cbase.h"

#define MENU_SHOW	0
#define MENU_DIM	1
#define MENU_HIDE	2

// Menu option with pointer
typedef struct menuoption_s
{
	const wchar_t *szName;
	const char *szCommand;

	int (*conditionfunc)();

	menuoption_s(const wchar_t *name, const char *command, int (*cfnc)())
	{
		szName = name;
		szCommand = command;
		conditionfunc = cfnc;
	}
} menuoption_t;

#define ADD_MENU_OPTION(id, name, command) \
	int MenuOption##id##();	\
	menuoption_t id##(name, command, &MenuOption##id##);	\
	int MenuOption##id##()

class CHudContextMenu : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE(CHudContextMenu, vgui::Panel);

	// Progress
	float	m_flSelectStart;
	float	m_flDuration;

	// Remember old choice in menu
//	int		m_iPreviousSelection;
	const char	*m_pszPreviousCmd;

	bool	m_fVisible;

	// Which menu to show
	menuoption_t *m_pMenu;

	int		m_nOptions;
	float	m_flPositions[25][2];

	int		m_iSelected;

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

	CHudTexture	*m_pHudElementTexture;
	CHudTexture *m_pHudBuildIcons[4];
	char *m_pszBuildLabels[4];

public:
	CHudContextMenu(const char *pElementName) : CHudElement(pElementName), vgui::Panel(NULL, "HudRadialMenu") 
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits(/*HIDEHUD_HEALTH | */HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);
	}

	~CHudContextMenu();

	void	Init();
	void	VidInit();
	void	Paint();

	void	SetBuildTimer(int type, float duration);

	void	MouseMove(float *x, float *y);

	void	Display(bool state);
	void	SetMenu();
	void	DoCommand(const char *cmd);

	int		m_iIcon;

	float	m_flPosX;
	float	m_flPosY;

	//CUtlLinkedList
};