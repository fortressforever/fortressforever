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

class MouseOverButton;
class LoadoutLabel;
class ClassPropertiesLabel;

namespace vgui
{
	class TextEntry;
	class PlayerModelPanel;
	class FFButton;
	class ProgressBar;
	class Section;
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

	MESSAGE_FUNC_PARAMS(OnMouseOverMessage, "MouseOverEvent", data);

public:

protected:	

	// vgui overrides
	virtual void OnCommand(const char *command);

	void UpdateClassInfo(const char *pszClassName);
	void SetClassInfoVisible( bool state );

	IViewPort		*m_pViewPort;

	vgui::Panel		*m_pPanel;

	float			m_flNextUpdate;

	MouseOverButton	*m_pClassButtons[10];
	vgui::FFButton	*m_pCancelButton;
	vgui::FFButton	*m_pRandomButton;

	LoadoutLabel *m_pPrimaryGren;
	LoadoutLabel *m_pSecondaryGren;

	LoadoutLabel *m_WepSlots[8];

	ClassPropertiesLabel *m_pSpeed;
	ClassPropertiesLabel *m_pFirepower;
	ClassPropertiesLabel *m_pHealth;

	vgui::PlayerModelPanel	*m_pModelView;
	
	vgui::Section *m_pGrenadesSection;
	vgui::Section *m_pWeaponsSection;
	vgui::Section *m_pClassInfoSection;

	//virtual vgui::Panel *CreateControlByName(const char *controlName);
	//MouseOverPanelButton * CreateNewMouseOverPanelButton(vgui::Panel *panel);
};


#endif // CLASSMENU_H
