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

#include "ff_utils.h"

#include <vgui_controls/Panel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Label.h>
#include "vgui/ff_luabox.h"
#include "vgui/ff_vgui_timer.h"

#define MAX_HUD_ELEMENTS	128
#define	DONT_CREATE_NEW		false

using namespace vgui;

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

	Panel	*GetHudElement(const char *pszIdentifier, HudElementType_t iType);
	void	RemoveElement(const char *pszIdentifier);

	void	HudIcon(const char *pszIdentifier, int iX, int iY, const char *pszSource, int iWidth, int iHeight, int iAlignX, int iAlignY);
	void	HudBox(const char *pszIdentifier, int iX, int iY, int iWidth, int iHeight, Color clr, Color clrBorder, int iBorderWidth, int iAlignX, int iAlignY);
	void	HudText(const char *pszIdentifier, int iX, int iY, const char *pszText, int iAlignX, int iAlignY, int iSize);

	bool	TranslateKeyCommand( const char *szMessage, char *szTranslated, int iBufferSizeInBytes );

	void	HudTimer(const char *pszIdentifier, int iX, int iY, float flValue, float flSpeed, int iAlignX, int iAlignY, int iSize);

	void	FireGameEvent( IGameEvent *pEvent );

private:
	HudElement_t		m_sHudElements[MAX_HUD_ELEMENTS];
	int					m_nHudElements;
};


#endif
