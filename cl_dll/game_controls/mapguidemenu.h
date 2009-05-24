/********************************************************************
	created:	2006/02/13
	created:	13:2:2006   18:47
	filename: 	f:\ff-svn\code\trunk\cl_dll\game_controls\mapguidemenu.h
	file path:	f:\ff-svn\code\trunk\cl_dll\game_controls
	file base:	mapguidemenu
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#ifndef MAPGUIDEMENU_H
#define MAPGUIDEMENU_H
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

// Forward declaration
class GuideMouseOverPanelButton;

class CMapGuideMenu : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CMapGuideMenu, vgui::Frame);

public:
	CMapGuideMenu(IViewPort *pViewPort);
	virtual ~CMapGuideMenu();

	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual void ShowPanel(bool bShow);

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel() 					{ return BaseClass::GetVPanel(); }

  	virtual bool IsVisible() 						{ return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) 	{ BaseClass::SetParent(parent); }
	virtual const char *GetName() 				{ return PANEL_MAPGUIDE; }
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
	vgui::ImagePanel	*m_pImagePanel;

	float			m_flNextUpdate;

	virtual vgui::Panel *CreateControlByName(const char *controlName);
	GuideMouseOverPanelButton * CreateNewMouseOverPanelButton(vgui::Panel *panel);
};


#endif // MAPGUIDEMENU_H
