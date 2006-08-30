/********************************************************************
	created:	2006/08/29
	created:	29:8:2006   18:35
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff_options.h
	file path:	f:\ff-svn\code\trunk\cl_dll
	file base:	ff_options
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#ifndef FF_OPTIONS_H
#define FF_OPTIONS_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_helpers.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include "ff_gameui.h"

using namespace vgui;

class CFFOptionsPage : public PropertyPage
{
	DECLARE_CLASS_SIMPLE(CFFOptionsPage, PropertyPage);

public:
	virtual void	Apply() = 0;
	virtual void	Load() = 0;
	virtual void	Reset() = 0;

	CFFOptionsPage(Panel *parent, char const *panelName) : BaseClass(parent, panelName) {}
};

//-----------------------------------------------------------------------------
// Purpose: Fortress Forever options
//-----------------------------------------------------------------------------
class CFFOptionsPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFOptionsPanel, Frame);

public:
	CFFOptionsPanel(vgui::VPANEL parent);
	void SetVisible(bool state);

private:

	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	
	PropertySheet	*m_pPropertyPages;
	CFFOptionsPage	*m_pCrosshairOptions;
	CFFOptionsPage	*m_pTimerOptions;

	Button			*m_pOKButton;
	Button			*m_pCancelButton;
	Button			*m_pApplyButton;
};

DECLARE_GAMEUI(CFFOptions, CFFOptionsPanel, ffoptions);

#endif // TESTPANEL_H 