#ifndef FF_CUSTOMHUDOPTIONS_ITEMPRESETS_H
#define FF_CUSTOMHUDOPTIONS_ITEMPRESETS_H

#include "ff_optionspresetpage.h"

#include "ff_inputslider.h"
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>

namespace vgui
{
	class CFFCustomHudStylePresets : public CFFOptionsPresetPage
	{
		DECLARE_CLASS_SIMPLE(CFFCustomHudStylePresets, CFFOptionsPresetPage);

	public:
		CFFCustomHudStylePresets(Panel *parent, char const *panelName, char const *pszComboBoxName);

		virtual void ActivatePresetPage();
		virtual void SetControlsEnabled(bool bEnabled);

		virtual void UpdatePresetFromControls(KeyValues *kvPreset);
		virtual void ApplyPresetToControls(KeyValues *kvPreset);
		virtual void RegisterSelfForPresetAssignment();		
		virtual KeyValues* RemoveNonEssentialValues(KeyValues *kvPreset);
		virtual void SendUpdatedPresetNameToPresetAssignment(const char *pszPresetName);
		virtual void SendRenamedPresetNameToPresetAssignment(const char *pszOldPresetName, const char *pszNewPresetName);
		virtual void SendDeletedPresetNameToPresetAssignment(const char *pszDeletedPresetName);
		virtual void SendNewPresetNameToPresetAssignment(const char *pszPresetName, KeyValues *kvPreset);

		void UpdateComponentControls(KeyValues *kvComponent);

	private:

		//-----------------------------------------------------------------------------
		// Purpose: Catch the sliders moving
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnUpdateSliders, "SliderMoved", data);

		//-----------------------------------------------------------------------------
		// Purpose: Catch the comboboxs changing their selection
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);

		//-----------------------------------------------------------------------------
		// Purpose: Catch the checkboxes changing their state
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnUpdateCheckbox, "CheckButtonChecked", data);

		CFFInputSlider	*m_pBarWidth;
		CFFInputSlider	*m_pBarHeight;
		CFFInputSlider	*m_pBarBorderWidth;

		CFFInputSlider	*m_pItemMarginHorizontal;
		CFFInputSlider	*m_pItemMarginVertical;
		CFFInputSlider	*m_pItemColumns;

		Label			*m_pSizeLabel, *m_pTextLabel;
		Label			*m_pOffsetXLabel, *m_pOffsetYLabel, *m_pOffsetLabel;
		Label			*m_pAlignHLabel, *m_pAlignVLabel, *m_pAlignLabel;
		CFFInputSlider	*m_pOffsetX;
		CFFInputSlider	*m_pOffsetY;
		CFFInputSlider	*m_pSize;
		CFFInputSlider	*m_pRed;
		CFFInputSlider	*m_pGreen;
		CFFInputSlider	*m_pBlue;
		CFFInputSlider	*m_pAlpha;
		
		ComboBox		*m_pColorMode;
		ComboBox		*m_pComponentSelection;
		ComboBox		*m_pAlignH, *m_pAlignV;
		ComboBox		*m_pBarOrientation;
		
		CheckButton		*m_pShadow;
		CheckButton		*m_pShow;
		CheckButton		*m_pFontTahoma;
	};
};

#endif