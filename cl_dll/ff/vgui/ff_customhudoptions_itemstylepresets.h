#ifndef FF_CUSTOMHUDOPTIONS_ITEMPRESETS_H
#define FF_CUSTOMHUDOPTIONS_ITEMPRESETS_H

#include "ff_optionspresetpage.h"

#include "ff_inputslider.h"
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include "ff_colorpicker.h"

namespace vgui
{
	class CFFCustomHudItemStylePresets : public CFFOptionsPresetPage
	{
		DECLARE_CLASS_SIMPLE(CFFCustomHudItemStylePresets, CFFOptionsPresetPage);

	public:
		CFFCustomHudItemStylePresets(Panel *parent, char const *panelName, char const *pszComboBoxName);

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
		
		void RefreshAnchorPositionCheckButtons(CheckButton *pCheckButton);
		void OnUpdateComponentSelection();
		void UpdateComponentControls(KeyValues *kvComponent);

	private:
	
		//-----------------------------------------------------------------------------
		// Purpose: Catch the color pickers changing color
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnColorChanged, "ColorChanged", data);
		//-----------------------------------------------------------------------------
		// Purpose: Catch the color mode changing mode
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnColorModeChanged, "ColorModeChanged", data);

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
		MESSAGE_FUNC_PARAMS(OnUpdateCheckButton, "CheckButtonChecked", data);

		CFFInputSlider	*m_pBarWidth;
		CFFInputSlider	*m_pBarHeight;
		CFFInputSlider	*m_pBarBorderWidth;

		CFFInputSlider	*m_pItemMarginHorizontal;
		CFFInputSlider	*m_pItemMarginVertical;
		CFFInputSlider	*m_pItemColumns;

		FFColorPicker	*m_pColor;

		Label			*m_pSizeLabel, *m_pTextLabel;
		Label			*m_pAnchorPositionLabel;
		Label			*m_pOffsetXLabel, *m_pOffsetYLabel, *m_pOffsetLabel;
		Label			*m_pAlignHLabel, *m_pAlignVLabel, *m_pAlignLabel;
		CFFInputSlider	*m_pOffsetX;
		CFFInputSlider	*m_pOffsetY;
		CFFInputSlider	*m_pSize;
		
		ComboBox		*m_pComponentSelection;
		ComboBox		*m_pAlignH, *m_pAlignV;
		ComboBox		*m_pBarOrientation;
		
		CheckButton		*m_pShadow;
		CheckButton		*m_pShow;
		CheckButton		*m_pFontTahoma;

		CheckButton		*m_pAnchorPositionTopLeft;
		CheckButton		*m_pAnchorPositionTopCenter;
		CheckButton		*m_pAnchorPositionTopRight;
		CheckButton		*m_pAnchorPositionMiddleLeft;
		CheckButton		*m_pAnchorPositionMiddleCenter;
		CheckButton		*m_pAnchorPositionMiddleRight;
		CheckButton		*m_pAnchorPositionBottomLeft;
		CheckButton		*m_pAnchorPositionBottomCenter;
		CheckButton		*m_pAnchorPositionBottomRight;
	};
};

#endif