#ifndef FF_CUSTOMHUDOPTIONS_ARRANGEMENTPRESETS_H
#define FF_CUSTOMHUDOPTIONS_ARRANGEMENTPRESETS_H

#include "ff_optionspresetpage.h"

#include "ff_inputslider.h"
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/TextEntry.h>

namespace vgui
{
	class CFFCustomHudArrangementPresets : public CFFOptionsPresetPage
	{
		DECLARE_CLASS_SIMPLE(CFFCustomHudArrangementPresets, CFFOptionsPresetPage);

	public:
		CFFCustomHudArrangementPresets(Panel *parent, char const *panelName, char const *pszComboBoxName);

		virtual void ActivatePresetPage();
		virtual void UpdatePresetFromControls(KeyValues *kvPreset);
		virtual void ApplyPresetToControls(KeyValues *kvPreset);
		virtual void RegisterSelfForPresetAssignment();
		virtual KeyValues* RemoveNonEssentialValues(KeyValues *kvPreset);
		virtual void SendUpdatedPresetNameToPresetAssignment(const char *pszPresetName);
		virtual void SendRenamedPresetNameToPresetAssignment(const char *pszOldPresetName, const char *pszNewPresetName);
		virtual void SendDeletedPresetNameToPresetAssignment(const char *pszDeletedPresetName);
		virtual void SendNewPresetNameToPresetAssignment(const char *pszPresetName, KeyValues *kvPreset);

		//-----------------------------------------------------------------------------
		// Purpose: Catch the comboboxs changing their selection
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);

		//-----------------------------------------------------------------------------
		// Purpose: Catch the sliders moving
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnUpdateSliders, "SliderMoved", data);
		//-----------------------------------------------------------------------------
		// Purpose: Catch the checkbox selection changes
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnUpdateCheckbox, "CheckButtonChecked", data);
	private:
		CFFInputSlider	*m_pColumns;

		ComboBox		*m_pPanelColorMode;

		CFFInputSlider	*m_pItemsX, *m_pItemsY;
		CFFInputSlider	*m_pHeaderTextX, *m_pHeaderTextY;
		CFFInputSlider	*m_pHeaderIconX, *m_pHeaderIconY;
		CFFInputSlider	*m_pTextX, *m_pTextY;
		CFFInputSlider	*m_pHeaderIconSize;
		CFFInputSlider	*m_pHeaderTextSize;
		CFFInputSlider	*m_pTextSize;

		CFFInputSlider	*m_pPanelRed;
		CFFInputSlider	*m_pPanelGreen;
		CFFInputSlider	*m_pPanelBlue;
		CFFInputSlider	*m_pPanelAlpha;

		CheckButton		*m_pShowHeaderIcon;
		CheckButton		*m_pShowHeaderText;
		CheckButton		*m_pShowText;
		CheckButton		*m_pHeaderIconShadow;
		CheckButton		*m_pHeaderTextShadow;
		CheckButton		*m_pTextShadow;
		CheckButton		*m_pShowPanel;
		CheckButton		*m_pPanelColorCustom;
	};
};
#endif