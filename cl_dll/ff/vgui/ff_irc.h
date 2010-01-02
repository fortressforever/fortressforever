#ifndef FF_IRC_H
#define FF_IRC_H

#ifdef _WIN32
	#pragma once
#endif

#include "vgui_helpers.h"
#include "KeyValues.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/RichText.h>
#include "ff_gameui.h"

using namespace vgui;


class CFFIRCTab : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CFFIRCTab, PropertyPage);

public:
	CFFIRCTab(Panel *parent, char const *panelName) : BaseClass(parent, panelName) {}

};

//-----------------------------------------------------------------------------
// IRC Panel
//-----------------------------------------------------------------------------

class CFFIRCPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFIRCPanel,Frame);

public:
	CFFIRCPanel( vgui::VPANEL parent );

	void AddGameTab();
	void RemoveGameTab( CFFIRCTab *pTab );

private:

	//MESSAGE_FUNC_PARAMS( OnNewLineMessage, "TextNewLine",data); // When TextEntry sends a TextNewLine message (when user presses enter), trigger the function OnNewLineMessage
	//MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	//MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	
	//void CFFIRCPanel::OnNewLineMessage(KeyValues *data);

	vgui::PropertySheet		*m_pIRCTabs;
	//CFFIRCLobbyTab		*m_pLobbyTab;
	//CFFIRCGameTab			*m_pGame1Tab;
	vgui::PropertyPage		*m_pGameTab;
	vgui::PropertyPage		*m_pLobbyTab;

	vgui::TextEntry*		m_pTextEntry_ChatEntry;
	vgui::RichText*		m_pRichText_LobbyChat;

	//vgui::Button			*m_pOKButton;
	//vgui::Button			*m_pCancelButton;
	//vgui::Button			*m_pApplyButton;

};

DECLARE_GAMEUI(CFFIRC, CFFIRCPanel, ffirc);

#endif // FF_IRC_H 