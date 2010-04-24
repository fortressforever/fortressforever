#ifndef FF_CUSTOMHUDOPTIONS_ARRANGEMENTPRESETS_H
#define FF_CUSTOMHUDOPTIONS_ARRANGEMENTPRESETS_H

#include "ff_optionspresetpage.h"

#include "ff_inputslider.h"
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/TextEntry.h>
#include "ff_colorpicker.h"

namespace vgui
{
	class CFFCustomHudPanelStylePresets : public CFFOptionsPresetPage
	{
		DECLARE_CLASS_SIMPLE(CFFCustomHudPanelStylePresets, CFFOptionsPresetPage);

	public:
		CFFCustomHudPanelStylePresets(Panel *parent, char const *panelName, char const *pszComboBoxName);

		virtual void ActivatePresetPage();
		virtual void SetControlsEnabled(bool bEnabled);
		void RefreshAnchorPositionCheckButtons(CheckButton *pCheckButton);
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
		//-----------------------------------------------------------------------------
		// Purpose: Catch the checkbox selection changes
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnUpdateCheckButton, "CheckButtonChecked", data);		
		//-----------------------------------------------------------------------------
		// Purpose: Catch the color pickers changing color
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnColorChanged, "ColorChanged", data);
		//-----------------------------------------------------------------------------
		// Purpose: Catch the color mode changing mode
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnColorModeChanged, "ColorModeChanged", data);

	private:
		PropertySheet	*m_pPropertyPages;
		PropertyPage *m_pHeaderText;
		PropertyPage *m_pHeaderIcon;
		PropertyPage *m_pText;
		PropertyPage *m_pPanel;

		CFFInputSlider	*m_pPanelMargin;
		ComboBox		*m_pPanelType;

		CFFInputSlider	*m_pHeaderTextX, *m_pHeaderTextY;
		CFFInputSlider	*m_pHeaderIconX, *m_pHeaderIconY;
		CFFInputSlider	*m_pTextX, *m_pTextY;

		CFFInputSlider	*m_pHeaderIconSize;
		CFFInputSlider	*m_pHeaderTextSize;
		CFFInputSlider	*m_pTextSize;
		
		ComboBox		*m_pHeaderTextAlignHoriz, *m_pHeaderTextAlignVert;
		ComboBox		*m_pHeaderIconAlignHoriz, *m_pHeaderIconAlignVert;
		ComboBox		*m_pTextAlignHoriz, *m_pTextAlignVert;
		
		CheckButton		*m_pHeaderTextAnchorPositionTopLeft;
		CheckButton		*m_pHeaderTextAnchorPositionTopCenter;
		CheckButton		*m_pHeaderTextAnchorPositionTopRight;
		CheckButton		*m_pHeaderTextAnchorPositionMiddleLeft;
		CheckButton		*m_pHeaderTextAnchorPositionMiddleCenter;
		CheckButton		*m_pHeaderTextAnchorPositionMiddleRight;
		CheckButton		*m_pHeaderTextAnchorPositionBottomLeft;
		CheckButton		*m_pHeaderTextAnchorPositionBottomCenter;
		CheckButton		*m_pHeaderTextAnchorPositionBottomRight;		

		CheckButton		*m_pHeaderIconAnchorPositionTopLeft;
		CheckButton		*m_pHeaderIconAnchorPositionTopCenter;
		CheckButton		*m_pHeaderIconAnchorPositionTopRight;
		CheckButton		*m_pHeaderIconAnchorPositionMiddleLeft;
		CheckButton		*m_pHeaderIconAnchorPositionMiddleCenter;
		CheckButton		*m_pHeaderIconAnchorPositionMiddleRight;
		CheckButton		*m_pHeaderIconAnchorPositionBottomLeft;
		CheckButton		*m_pHeaderIconAnchorPositionBottomCenter;
		CheckButton		*m_pHeaderIconAnchorPositionBottomRight;

		CheckButton		*m_pTextAnchorPositionTopLeft;
		CheckButton		*m_pTextAnchorPositionTopCenter;
		CheckButton		*m_pTextAnchorPositionTopRight;
		CheckButton		*m_pTextAnchorPositionMiddleLeft;
		CheckButton		*m_pTextAnchorPositionMiddleCenter;
		CheckButton		*m_pTextAnchorPositionMiddleRight;
		CheckButton		*m_pTextAnchorPositionBottomLeft;
		CheckButton		*m_pTextAnchorPositionBottomCenter;
		CheckButton		*m_pTextAnchorPositionBottomRight;

		CheckButton		*m_pShowHeaderIcon;
		CheckButton		*m_pShowHeaderText;
		CheckButton		*m_pShowText;
		CheckButton		*m_pHeaderIconShadow;
		CheckButton		*m_pHeaderTextShadow;
		CheckButton		*m_pTextShadow;
		CheckButton		*m_pShowPanel;
				
		FFColorPicker	*m_pHeaderTextColor;
		FFColorPicker	*m_pHeaderIconColor;
		FFColorPicker	*m_pTextColor;
		FFColorPicker	*m_pPanelColor;
	};
};
#endif