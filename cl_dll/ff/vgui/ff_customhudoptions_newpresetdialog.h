#ifndef FF_CUSTOMHUDOPTIONS_NEWPRESETDIALOG_H
#define FF_CUSTOMHUDOPTIONS_NEWPRESETDIALOG_H

#include <vgui_controls/Frame.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/TextEntry.h>

namespace vgui
{
	class CFFCustomHudNewPresetDialog : public Frame
	{
	DECLARE_CLASS_SIMPLE(CFFCustomHudNewPresetDialog, Frame);
	public:
		CFFCustomHudNewPresetDialog(Panel *parent, const char *panelName);

		void OK();
		void Cancel();

		void UpdateSecondaryComboFromParentKey(KeyValues *kvPrimary);
	private:
		ComboBox	*m_pPrimary, *m_pSecondary;
		TextEntry	*m_pName;

		Button	*m_pOKButton;
		Button	*m_pCancelButton;

		Panel *m_pColor;
		ImagePanel *m_pColorBackground;
		
		MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);
	};
};

#endif