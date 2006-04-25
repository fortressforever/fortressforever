/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file classmenu2.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date August 15, 2005
/// @brief New class selection menu
///
/// REVISIONS
/// ---------
/// Aug 15, 2005 Mirv: First creation

#ifndef CLASSMENU_H
#define CLASSMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include <cl_dll/iviewport.h>
#include <vgui/KeyCode.h>

#include "mouseoverpanelbutton.h"

namespace vgui
{
	class TextEntry;
}

//-----------------------------------------------------------------------------
// Purpose: displays the MOTD
//-----------------------------------------------------------------------------

class CClassMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CClassMenu, vgui::Frame);

public:
	CClassMenu(IViewPort *pViewPort);
	virtual ~CClassMenu();

	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual void ShowPanel(bool bShow);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel() 					{ return BaseClass::GetVPanel(); }

  	virtual bool IsVisible() 						{ return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) 	{ BaseClass::SetParent(parent); }
	virtual const char *GetName() 				{ return PANEL_CLASS; }
	virtual bool NeedsUpdate() 				{ return gpGlobals->curtime > m_flNextUpdate; }
	virtual bool HasInputElements() 			{ return true; }

	void OnKeyCodePressed(vgui::KeyCode code);
	void OnKeyCodeReleased(vgui::KeyCode code);

public:

protected:	

	// vgui overrides
	virtual void OnCommand(const char *command);

	IViewPort		*m_pViewPort;

	vgui::Button	*m_pCancel;
	vgui::Panel		*m_pPanel;

	float			m_flNextUpdate;

	virtual vgui::Panel *CreateControlByName(const char *controlName);
	MouseOverPanelButton * CreateNewMouseOverPanelButton(vgui::Panel *panel);
};


#endif // CLASSMENU_H
