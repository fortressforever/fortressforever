/********************************************************************
	created:	2006/09/28
	created:	28:9:2006   13:03
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_gamemodes.h
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_gamemodes
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/


#ifndef FF_GAMEMODES_H
#define FF_GAMEMODES_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_helpers.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include "ff_gameui.h"

//using namespace vgui;

class CFFGameModesPage : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CFFGameModesPage, PropertyPage);

public:
	virtual void	Play() = 0;
	virtual void	Load() = 0;
	virtual void	Reset() = 0;

	CFFGameModesPage(Panel *parent, char const *panelName) : BaseClass(parent, panelName) {}
};

//-----------------------------------------------------------------------------
// Purpose: Fortress Forever GameModes
//-----------------------------------------------------------------------------
class CFFGameModesPanel : public vgui::Frame
{
	DECLARE_CLASS_SIMPLE(CFFGameModesPanel, vgui::Frame);

public:
	CFFGameModesPanel(vgui::VPANEL parent);
	void SetVisible(bool state);

private:

	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	
	vgui::PropertySheet		*m_pPropertyPages;
	CFFGameModesPage		*m_pScenarioGameMode;
	CFFGameModesPage		*m_pTrainingGameMode;

	vgui::Button			*m_pOKButton;
	vgui::Button			*m_pCancelButton;
	vgui::Button			*m_pApplyButton;
};

DECLARE_GAMEUI(CFFGameModes, CFFGameModesPanel, ffgamemodes);

#endif // TESTPANEL_H 