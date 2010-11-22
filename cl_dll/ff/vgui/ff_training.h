/********************************************************************
	created:	2006/09/28
	created:	28:9:2006   13:03
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_gamemodes.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_gamemodes
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/

#ifndef FF_TRAINING_H
#define FF_TRAINING_H

#ifdef _WIN32
	#pragma once
#endif

#include "vgui_helpers.h"
#include <vgui_controls/Frame.h>
#include "ff_gameui.h"

using namespace vgui;

class CFFTrainingPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFTrainingPanel,Frame);

public:
	CFFTrainingPanel( vgui::VPANEL parent );
	void SetVisible(bool state);

private:

	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);

	vgui::Button			*m_pOKButton;
	vgui::Button			*m_pCancelButton;
	vgui::Label				*m_pStatusLabel;

};

DECLARE_GAMEUI(CFFTrainingUI, CFFTrainingPanel, fftraining);

#endif // FF_TRAINING_H 