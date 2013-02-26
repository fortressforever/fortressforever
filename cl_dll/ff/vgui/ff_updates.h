/********************************************************************
	created:	2006/09/28
	created:	28:9:2006   13:03
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_gamemodes.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_gamemodes
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#ifndef FF_UPDATES_H
#define FF_UPDATES_H

#ifdef _WIN32
	#pragma once
#endif

#include "vgui_helpers.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/HTML.h>
#include "ff_gameui.h"
#include "update/autoupdate.h"

class CFFUpdateInfo;

using namespace vgui;

//-----------------------------------------------------------------------------
// parent panel, holds all the update UI elements
//-----------------------------------------------------------------------------
class CFFUpdatesPanel : public Panel
{
	DECLARE_CLASS_SIMPLE(CFFUpdatesPanel,Panel);

public:
	CFFUpdatesPanel( vgui::VPANEL parent );
	~CFFUpdatesPanel( );
	void SetVisible(bool state);
	void CheckUpdate(const char *pszServerVersion = NULL);
	void UpdateAvailable(eUpdateResponse status);
	
	virtual void OnTick();
	virtual void Paint( void );
	
	// called when scheme settings need to be applied; called the first time before the panel is painted
	virtual void ApplySchemeSettings(IScheme *pScheme);

private:

	CFFUpdateInfo			*m_pUpdateInfo;
	vgui::Label				*m_pUpdateStatus;
	vgui::Label				*m_pCurrentVersion;

	float					m_flStatusFadeTime;

public:

	eUpdateResponse			m_UpdateStatus;

};

DECLARE_GAMEUI(CFFUpdatesUI, CFFUpdatesPanel, ffupdates);

//-----------------------------------------------------------------------------
// update info, shows if an update is available
//-----------------------------------------------------------------------------
class CFFUpdateInfo : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFUpdateInfo,Frame);

public:
	CFFUpdateInfo(Panel *parent, const char *panelName, bool showTaskbarIcon = true);
	void SetVisible(bool state);

private:

	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);

	vgui::Button			*m_pOKButton;
	vgui::Button			*m_pCancelButton;

};

#endif // FF_UPDATES_H 