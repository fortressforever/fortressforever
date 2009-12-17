#ifndef FF_IRC_H
#define FF_IRC_H

#ifdef _WIN32
	#pragma once
#endif

#include "vgui_helpers.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include "ff_gameui.h"

using namespace vgui;


class CFFIRCLobbyTab : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CFFIRCLobbyTab, PropertyPage);

public:
	CFFIRCLobbyTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName) {}

};

class CFFIRCGameTab : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CFFIRCGameTab, PropertyPage);

public:
	CFFIRCGameTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName) {}

};

class CFFIRCPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFIRCPanel,Frame);

public:
	CFFIRCPanel( vgui::VPANEL parent );


private:

	//MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	
	vgui::PropertySheet		*m_pIRCTabs;
	CFFIRCLobbyTab			*m_pLobbyTab;
	CFFIRCGameTab			*m_pGame1Tab;

	//vgui::Button			*m_pOKButton;
	//vgui::Button			*m_pCancelButton;
	//vgui::Button			*m_pApplyButton;

};

DECLARE_GAMEUI(CFFIRC, CFFIRCPanel, ffirc);

#endif // FF_IRC_H 