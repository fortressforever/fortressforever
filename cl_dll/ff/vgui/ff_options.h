/********************************************************************
	created:	2006/08/29
	created:	29:8:2006   18:35
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_options.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
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
#include <vgui_controls/HTML.h>
#include "ff_gameui.h"

//using namespace vgui;

class CFFOptionsPage : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CFFOptionsPage, vgui::PropertyPage);

public:
	virtual void	Apply() = 0;
	virtual void	Load() = 0;
	virtual void	Reset() = 0;

	CFFOptionsPage(Panel *parent, char const *panelName) : BaseClass(parent, panelName) {}
};

//-----------------------------------------------------------------------------
// Purpose: Fortress Forever options
//-----------------------------------------------------------------------------
class CFFOptionsPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CFFOptionsPanel, vgui::Frame);

public:
	CFFOptionsPanel(vgui::VPANEL parent);
	void SetVisible(bool state);

private:

	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	
	vgui::PropertySheet		*m_pPropertyPages;
	CFFOptionsPage			*m_pCrosshairOptions;
	CFFOptionsPage			*m_pTimerOptions;
	CFFOptionsPage			*m_pMiscOptions1;
	CFFOptionsPage			*m_pMiscOptions2;
	CFFOptionsPage			*m_pMiscOptions3;
	CFFOptionsPage			*m_pMiscOptions4;
	CFFOptionsPage			*m_pDLightOptions;


	vgui::Button			*m_pOKButton;
	vgui::Button			*m_pCancelButton;
	vgui::Button			*m_pApplyButton;
};

DECLARE_GAMEUI(CFFOptions, CFFOptionsPanel, ffoptions);

class SplashHTML;

//-----------------------------------------------------------------------------
// Purpose: Fortress Forever options
//-----------------------------------------------------------------------------
class CFFSplashPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CFFSplashPanel, vgui::Frame);

public:
	CFFSplashPanel(vgui::VPANEL parent);
	void CheckUpdate(const char *pszServerVersion = NULL);

private:
	SplashHTML	*m_pSplashHTML;
};

DECLARE_GAMEUI(CFFSplash, CFFSplashPanel, ffsplash);


#endif // TESTPANEL_H 