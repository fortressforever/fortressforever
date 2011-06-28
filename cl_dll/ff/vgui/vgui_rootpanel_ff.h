//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef VGUI_ROOTPANEL_FF_H
#define VGUI_ROOTPANEL_FF_H
#ifdef _WIN32
#pragma once
#endif


#include <vgui_controls/Panel.h>
#include <vgui_controls/EditablePanel.h>
#include "UtlVector.h"


class CPanelEffect;


// Serial under of effect, for safe lookup
typedef unsigned int EFFECT_HANDLE;

//-----------------------------------------------------------------------------
// Purpose: Sits between engine and client .dll panels
//  Responsible for drawing screen overlays
//-----------------------------------------------------------------------------
class C_FFRootPanel : public vgui::Panel
{
	typedef vgui::Panel BaseClass;
public:
						C_FFRootPanel( vgui::VPANEL parent );
	virtual				~C_FFRootPanel( void );

	// Draw Panel effects here
	virtual void		PostChildPaint();

	// Clear list of Panel Effects
	virtual void		LevelInit( void );
	virtual void		LevelShutdown( void );

private:

	// Render all panel effects
	void		RenderPanelEffects( void );

	// List of current panel effects
	CUtlVector< CPanelEffect *> m_Effects;
};


#endif // VGUI_ROOTPANEL_FF_H
