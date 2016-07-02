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
#ifndef FF_HUD_MENU_H
#define FF_HUD_MENU_H


#include "cbase.h"

#define MENU_SHOW	0
#define MENU_DIM	1
#define MENU_HIDE	2

// Menu option with pointer
class MenuOption
{
public:
	const char		*szName;
	wchar_t			*wszText;
	const char		*szCommand;
	void			*pNextMenu;
	char			chIcon;

	int (*conditionfunc)();

public:
	MenuOption(const char *name, char icon, const char *command, int (*cfnc)(), void *nextmenu)
	{
		szName			= name;
		szCommand		= command;
		conditionfunc	= cfnc;
		pNextMenu		= nextmenu;
		chIcon			= icon;
		wszText			= NULL;
	}

	~MenuOption()
	{
		if (wszText)
			delete wszText;
	}
};

// Information about a menu
typedef struct menu_s {
	int size;
	MenuOption *options;
	const char *default_cmd;
} menu_t;

#define ADD_MENU_OPTION(id, name, icon, command) \
	int MenuOption##id##();	\
	MenuOption id##(name, icon, command, &MenuOption##id##, NULL);	\
	int MenuOption##id##()

#define ADD_MENU_BRANCH(id, name, icon, command, nextmenu) \
	int MenuOption##id##();	\
	MenuOption id##(name, icon, command, &MenuOption##id##, nextmenu);	\
	int MenuOption##id##()

class CHudContextMenu : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE(CHudContextMenu, vgui::Panel);

	// Progress
	float	m_flSelectStart;
	float	m_flMenuStart;

	bool	m_fVisible;

	menu_t	*m_pMenu;

	int		m_nOptions;
	float	m_flPositions[25][2];
	int		m_nLayer;

	int		m_iSelected;

	// Stuff we need to know
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
	CPanelAnimationVar(vgui::HFont, m_hMenuIcon, "DisguiseFont", "ClassGlyphs");

public:
	CHudContextMenu(const char *pElementName) : CHudElement(pElementName), vgui::Panel(NULL, "HudRadialMenu") 
	{
		SetParent(g_pClientMode->GetViewport());
		SetHiddenBits(HIDEHUD_PLAYERDEAD | HIDEHUD_SPECTATING | HIDEHUD_UNASSIGNED);
	}

	~CHudContextMenu();

	void	Init();
	void	VidInit();
	void	Paint();

	void	MouseMove(float *x, float *y);

	void	Display(bool state);
	void	SetMenu();
	void	DoCommand(const char *cmd);

	int		KeyEvent(int down, int keynum, const char *pszCurrentBinding);

	float	m_flPosX;
	float	m_flPosY;

	int		GetLayerNumber() const { return m_nLayer; }

private:
	void ProgressToNextMenu(int iOption);


	//CUtlLinkedList
};

#endif