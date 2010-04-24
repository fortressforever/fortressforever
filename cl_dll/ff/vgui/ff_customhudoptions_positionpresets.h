#ifndef FF_CUSTOMHUDOPTIONS_POSITIONPRESETS_H
#define FF_CUSTOMHUDOPTIONS_POSITIONPRESETS_H

#include "ff_optionspresetpage.h"

#include "ff_inputslider.h"
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/TextEntry.h>

namespace vgui
{
	class CFFCustomHudPositionPresets : public CFFOptionsPresetPage
	{
		DECLARE_CLASS_SIMPLE(CFFCustomHudPositionPresets, CFFOptionsPresetPage);

	public:
		CFFCustomHudPositionPresets(Panel *parent, char const *panelName, char const *pszComboBoxName);

		virtual void ActivatePresetPage();
		virtual void SetControlsEnabled(bool bEnabled);

		virtual void UpdatePresetFromControls(KeyValues *kvPreset);
		virtual void ApplyPresetToControls(KeyValues *kvPreset);
		virtual void RegisterSelfForPresetAssignment();
		virtual KeyValues* RemoveNonEssentialValues(KeyValues *kvPreset);
		
		virtual void SendUpdatedPresetPreviewToPresetAssignment(const char *pszPresetName, KeyValues *kvPresetPreview);
		virtual void SendUpdatedPresetToPresetAssignment(const char *pszPresetName);
		virtual void SendRenamedPresetToPresetAssignment(const char *pszOldPresetName, const char *pszNewPresetName);
		virtual void SendDeletedPresetToPresetAssignment(const char *pszDeletedPresetName);
		virtual void SendNewPresetToPresetAssignment(const char *pszPresetName, KeyValues *kvPreset);

		//-----------------------------------------------------------------------------
		// Purpose: Catch the comboboxs changing their selection
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);

		//-----------------------------------------------------------------------------
		// Purpose: Catch the sliders moving
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnUpdateSliders, "SliderMoved", data);

	private:
		CFFInputSlider	*m_pX, *m_pY;

		ComboBox		*m_pAlignH, *m_pAlignV;
	};
};
#endif