/********************************************************************
	created:	2006/07/07
	created:	7:7:2006   15:46
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\ff_hud_lua.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff
	file base:	ff_hud_lua
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Lua controlled hut items
*********************************************************************/

#ifndef FF_HUD_LUA_H
#define FF_HUD_LUA_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "hudelement.h"
#include "iclientmode.h"

#include <vgui_controls/Panel.h>

#define MAX_HUD_ELEMENTS	128
#define	DONT_CREATE_NEW		false

//using namespace vgui;

enum HudElementType_t
{
	HUD_ICON = 0,
	HUD_TEXT,
	HUD_TIMER,
	HUD_REMOVE,
	HUD_ICON_ALIGNXY,
	HUD_TEXT_ALIGN,
	HUD_TEXT_ALIGNXY,
	HUD_TIMER_ALIGN,
	HUD_TIMER_ALIGNXY,
};

typedef struct HudElement_s
{
	char				szIdentifier[128];
	vgui::Panel			*pPanel;
	HudElementType_t	iType;
} HudElement_t;

class CHudLua : public CHudElement, public vgui::Panel
{
private:
	DECLARE_CLASS_SIMPLE(CHudLua, vgui::Panel);

public:

	CHudLua(const char *pElementName);
	virtual ~CHudLua();

	void	Init();
	void	VidInit();

	void	MsgFunc_FF_HudLua(bf_read &msg);

	Panel	*GetHudElement(const char *pszIdentifier, HudElementType_t iType, bool bCreateNew = true);
	void	RemoveElement(const char *pszIdentifier);
	void	HudIcon(const char *pszIdentifier, int iX, int iY, const char *pszSource, int iWidth, int iHeight, int iAlign);
	void	HudIcon(const char *pszIdentifier, int iX, int iY, const char *pszSource, int iWidth, int iHeight, int iAlignX, int iAlignY); // added y alignment
	void	HudText(const char *pszIdentifier, int iX, int iY, const char *pszText);
	void	HudText(const char *pszIdentifier, int iX, int iY, const char *pszText, int iAlign);
	void	HudText(const char *pszIdentifier, int iX, int iY, const char *pszText, int iAlignX, int iAlignY);
	void	HudTimer(const char *pszIdentifier, int iX, int iY, float flValue, float flSpeed);
	void	HudTimer(const char *pszIdentifier, int iX, int iY, float flValue, float flSpeed, int iAlign);
	void	HudTimer(const char *pszIdentifier, int iX, int iY, float flValue, float flSpeed, int iAlignX, int iAlignY);

	void	FireGameEvent( IGameEvent *pEvent );

private:
	HudElement_t		m_sHudElements[MAX_HUD_ELEMENTS];
	int					m_nHudElements;
};


#endif
