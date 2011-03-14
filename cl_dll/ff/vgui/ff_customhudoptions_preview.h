#ifndef FF_CUSTOMHUDOPTIONS_PREVIEW_H
#define FF_CUSTOMHUDOPTIONS_PREVIEW_H

#ifdef _WIN32
	#pragma once
#endif

#include "vgui_helpers.h"
#include <vgui_controls/Frame.h>
#include "ff_gameui.h"

using namespace vgui;

class CFFCustomHudPreviewPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFCustomHudPreviewPanel, Frame);

public:
	CFFCustomHudPreviewPanel( VPANEL parent );
	void SetVisible(bool state);

private:

};

DECLARE_GAMEUI(CFFCustomHudPreview, CFFCustomHudPreviewPanel, ffcustomhudpreview);

#endif