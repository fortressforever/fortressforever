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

#define MAX_HUD_ELEMENTS	30
#define	DONT_CREATE_NEW		false

using namespace vgui;

enum HudElementType_t
{
	HUD_ICON = 0,
	HUD_TEXT,
	HUD_TIMER,
	HUD_REMOVE
};

typedef struct HudElement_s
{
	char				szIdentifier[128];
	Panel				*pPanel;
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

	void	HudIcon(const char *pszIdentifer, int iX, int iY, const char *pszSource);
	void	HudText(const char *pszIdentifer, int iX, int iY, const char *pszText);
	void	HudTimer(const char *pszIdentifier, int iX, int iY, float flValue, float flSpeed);

private:
	HudElement_t		m_sHudElements[MAX_HUD_ELEMENTS];
	int					m_nHudElements;
};


#endif