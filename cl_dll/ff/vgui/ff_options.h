/********************************************************************
	created:	2006/08/29
	created:	29:8:2006   18:35
	filename: 	cl_dll\ff\vgui\ff_options\ff_options.h
	file path:	cl_dll\ff\vgui\ff_options
	file base:	ff_options
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	Main "Fortress Options" panel with tabs contining option pages
*********************************************************************/

#ifndef FF_OPTIONS_H
#define FF_OPTIONS_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_helpers.h"
#include <vgui_controls/Frame.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/HTML.h>

#include "ff_timeroptions.h"
#include "ff_miscoptions.h"
#include "ff_crosshairoptions.h"
#include "ff_dlightoptions.h"

#include "ff_gameui.h"

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Fortress Forever options
//-----------------------------------------------------------------------------
class CFFOptionsPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFOptionsPanel, Frame);

public:
	CFFOptionsPanel(VPANEL parent);
	void SetVisible(bool state);

private:
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	
	PropertySheet			*m_pPropertyPages;

	CFFOptionsPage			*m_pCrosshairOptions;
	CFFOptionsPage			*m_pTimerOptions;
	CFFOptionsPage			*m_pMiscOptions1;
	CFFOptionsPage			*m_pMiscOptions2;
	CFFOptionsPage			*m_pMiscOptions3;
	CFFOptionsPage			*m_pMiscOptions4;
	CFFOptionsPage			*m_pDLightOptions;

	Button					*m_pOKButton;
	Button					*m_pCancelButton;
	Button					*m_pApplyButton;
};

DECLARE_GAMEUI(CFFOptions, CFFOptionsPanel, ffoptions);

#endif