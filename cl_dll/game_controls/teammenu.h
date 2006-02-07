/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file teammenu2.h
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date August 15, 2005
/// @brief New team selection menu
///
/// REVISIONS
/// ---------
/// Aug 15, 2005 Mirv: First creation

#ifndef TEAMMENU_H
#define TEAMMENU_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include <cl_dll/iviewport.h>
#include <vgui/KeyCode.h>

namespace vgui
{
	class TextEntry;
}

//-----------------------------------------------------------------------------
// Purpose: displays the team menu
//-----------------------------------------------------------------------------

class CTeamMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CTeamMenu, vgui::Frame);

public:
	CTeamMenu(IViewPort *pViewPort);
	virtual ~CTeamMenu();

	virtual const char *GetName() { return PANEL_TEAM; }
	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual void ShowPanel(bool bShow);

	virtual void OnKeyCodePressed(vgui::KeyCode code);

	virtual bool IsVisible() 						{ return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) 	{ BaseClass::SetParent(parent); }
	virtual bool NeedsUpdate() 				{ return false; }
	virtual bool HasInputElements() 			{ return true; }


	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel() 					{ return BaseClass::GetVPanel(); }

public:

protected:	
	// vgui overrides
	virtual void OnCommand(const char *command);

	IViewPort	*m_pViewPort;

	vgui::RichText	*m_pMapInfo;
	vgui::Label		*m_pMapName;
	vgui::Button	*m_pCancel;
};


#endif // TEAMMENU_H
