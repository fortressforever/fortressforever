/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file mapscreen.cpp
/// @author Christopher "Jiggles" Boylan
/// @date August 30, 2007
/// @brief To display the map screenshot

#ifndef MAPSCREEN_H
#define MAPSCREEN_H
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
	class FFButton;
}

//---------------------------------------------------------------------------------
// Purpose: Displays a map screenshot to help the player learn the map's objectives
//---------------------------------------------------------------------------------

class CMapScreen : public vgui::Frame, public IViewPortPanel
{
private:
	DECLARE_CLASS_SIMPLE(CMapScreen, vgui::Frame);

public:
	CMapScreen(IViewPort *pViewPort);
	virtual ~CMapScreen();

	virtual void SetData(KeyValues *data);
	virtual void Reset();
	virtual void Update();
	virtual void ShowPanel(bool bShow);

	void	KeyDown( void );
	void	KeyUp( void );

	// both vgui::Frame and IViewPortPanel define these, so explicitly define them here as passthroughs to vgui
	vgui::VPANEL GetVPanel() 					{ return BaseClass::GetVPanel(); }

  	virtual bool IsVisible() 						{ return BaseClass::IsVisible(); }
  	virtual void SetParent(vgui::VPANEL parent) 	{ BaseClass::SetParent(parent); }
	virtual const char *GetName() 				{ return PANEL_MAP; }
	virtual bool NeedsUpdate() 				{ return gpGlobals->curtime > m_flNextUpdate; }
	virtual bool HasInputElements() 			{ return true; }

protected:	

	// vgui overrides
	virtual void OnCommand(const char *command);

	IViewPort		*m_pViewPort;

	float			m_flNextUpdate;

	bool			m_bMapKeyPressed;

	vgui::FFButton	*m_pCloseButton;

};


// Global
extern CMapScreen *g_pMapScreen;

#endif // MAPSCREEN_H
