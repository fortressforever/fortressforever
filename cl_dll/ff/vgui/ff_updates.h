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
#include "ff_gameui.h"

using namespace vgui;

class CFFUpdatesPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFUpdatesPanel,Frame);

public:
	CFFUpdatesPanel( vgui::VPANEL parent );
	void SetVisible(bool state);
	void UpdateAvailable(bool state);

private:

	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);

	vgui::Button			*m_pOKButton;
	vgui::Button			*m_pCancelButton;

};

DECLARE_GAMEUI(CFFUpdatesUI, CFFUpdatesPanel, ffupdates);

#endif // FF_UPDATES_H 