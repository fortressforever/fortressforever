#ifndef FF_CUSTOMHUDOPTIONS_H
#define FF_CUSTOMHUDOPTIONS_H

#include "ff_optionspage.h"

#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertySheet.h>

#include "ff_customhudoptions_assignpresets.h"
#include "ff_customhudoptions_positionpresets.h"
#include "ff_customhudoptions_arrangementpresets.h"
#include "ff_customhudoptions_stylepresets.h"

#include "KeyValues.h"

using namespace vgui;

//=============================================================================
// Our quantitypanel options page. This also is massive - like my penis.
//=============================================================================
class CFFCustomHudOptions : public CFFOptionsPage
{
	DECLARE_CLASS_SIMPLE(CFFCustomHudOptions, CFFOptionsPage);
	
public:
	CFFCustomHudOptions(Panel *parent, char const *panelName);

	virtual void AllowChanges(bool state);
	void Apply();
	void Load();
	void Reset();
	
	void SetActivePage(Panel *page);

	void ActivatePositionPresetPage();
	void ActivateArrangementPresetPage();
	void ActivateStylePresetPage();

private:
	MESSAGE_FUNC_PARAMS( OnActivatePositionPage, "ActivatePositionPage", data );
	MESSAGE_FUNC_PARAMS( OnActivateArrangmentPage, "ActivateArrangementPage", data );
	MESSAGE_FUNC_PARAMS( OnActivateStylePage, "ActivateStylePage", data );
	MESSAGE_FUNC_PARAMS( OnActivatePage, "ActivatePage", data );
	

	void UpdateSliders();

	PropertySheet					*m_pPropertyPages;
	CFFCustomHudAssignPresets		*m_pAssignPresets;
	CFFCustomHudPositionPresets		*m_pPositionPresets;
	CFFCustomHudArrangementPresets	*m_pArrangementPresets;
	CFFCustomHudStylePresets		*m_pStylePresets;
};

#endif