#ifndef FF_CUSTOMHUDOPTIONS_H
#define FF_CUSTOMHUDOPTIONS_H

#ifdef _WIN32
	#pragma once
#endif

#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Button.h>
#include "vstdlib/icommandline.h"
#include "ff_gameui.h"

#include "ff_customhudoptions_assignpresets.h"
#include "ff_customhudoptions_positionpresets.h"
#include "ff_customhudoptions_panelstylepresets.h"
#include "ff_customhudoptions_itemstylepresets.h"

#include "KeyValues.h"

using namespace vgui;

//=============================================================================
// Our quantitypanel options page. This also is massive - like my penis.
//=============================================================================
class CFFCustomHudOptionsPanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CFFCustomHudOptionsPanel, Frame);
	
public:
	CFFCustomHudOptionsPanel( VPANEL parent );

	void SetVisible(bool state);
	void Apply();
	void Load();
	void Reset();
	
	void SetActivePage(Panel *page);

	void ActivatePositionPresetPage();
	void ActivatePanelStylePresetPage();
	void ActivateItemStylePresetPage();

private:
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
	MESSAGE_FUNC_PARAMS( OnActivatePositionPage, "ActivatePositionPage", data );
	MESSAGE_FUNC_PARAMS( OnActivatePanelStylePage, "ActivatePanelStylePage", data );
	MESSAGE_FUNC_PARAMS( OnActivateItemStylePage, "ActivateItemStylePage", data );
	
	void UpdateSliders();

	PropertySheet					*m_pPropertyPages;
	CFFCustomHudAssignPresets		*m_pAssignPresets;
	CFFCustomHudPositionPresets		*m_pPositionPresets;
	CFFCustomHudPanelStylePresets	*m_pPanelStylePresets;
	CFFCustomHudItemStylePresets	*m_pItemStylePresets;

	Button					*m_pOKButton;
	Button					*m_pCancelButton;
	Button					*m_pApplyButton;
};

DECLARE_GAMEUI(CFFCustomHudOptions, CFFCustomHudOptionsPanel, ffcustomhudoptions);

#endif