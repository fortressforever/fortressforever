/********************************************************************
	created:	2010/09/02
	filename: 	cl_dll\ff\ff_optionspresetpage.h
	file path:	cl_dll\ff
	file base:	ff_optionspresetpage
	file ext:	h
	author:		Elmo
	
	purpose:	Preset editor panel (for cusomtisable hud stuff)
*********************************************************************/

#ifndef FF_OPTIONSPRESETPAGE_H
#define FF_OPTIONSPRESETPAGE_H

#include "ff_optionspage.h"

#include "filesystem.h"
extern IFileSystem **pFilesystem;

#include "keyvalues.h"

#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/InputDialog.h>
#include <vgui_controls/MessageBox.h>

namespace vgui
{
	class CFFOptionsPresetPage : public CFFOptionsPage
	{
	DECLARE_CLASS_SIMPLE( CFFOptionsPresetPage, CFFOptionsPage );
	public:
		CFFOptionsPresetPage(Panel *parent, const char *panelName, const char *pszComboBoxName, const char *pszSourceFile);
		~CFFOptionsPresetPage();

		virtual void SetControlsEnabled(bool bEnabled) = 0;
		virtual void ActivatePresetPage() = 0;

		bool HasLoaded();
		virtual void UpdatePresetFromControls(KeyValues *kvPreset);
		virtual void ApplyPresetToControls(KeyValues *kvPreset) = 0;
		virtual void RegisterSelfForPresetAssignment() = 0;
		virtual KeyValues* RemoveNonEssentialValues(KeyValues *kvPreset) = 0;

		virtual void SendUpdatedPresetPreviewToPresetAssignment(const char *pszPresetName, KeyValues *kvPresetPreview) = 0;
		virtual void SendUpdatedPresetToPresetAssignment(const char *pszPresetName) = 0;
		virtual void SendRenamedPresetToPresetAssignment(const char *pszOldPresetName, const char *pszNewPresetName) = 0;
		virtual void SendDeletedPresetToPresetAssignment(const char *pszDeletedPresetName) = 0;
		virtual void SendNewPresetToPresetAssignment(const char *pszPresetName, KeyValues *kvPreset) = 0;

		void AddPreset(KeyValues *kvPreset);

		virtual void Load();
		virtual void Reset();
		virtual void Apply();

		
		void CreatePresetFromPanelDefault(KeyValues *kvPreset);

		KeyValues *GetPresetDataByName(char const *styleName);
		KeyValues *GetPresetData();

		//-----------------------------------------------------------------------------
		// Purpose: Catch the comboboxs changing their selection
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);

		//-----------------------------------------------------------------------------
		// Purpose: Catch name input dialog's okay button and process the text
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnInputDialogCommand, "InputCompleted", data);

		//-----------------------------------------------------------------------------
		// Purpose: Catch the buttons. New Copy Delete Rename
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);
		
	protected:
		char m_szSourceFile[128];
		char m_szPresetTypeName[128];
		bool m_bLoaded, m_bIsLoading;
		bool m_bCopy, m_bRename;
		ComboBox		*m_pPresets;
		
		Button			*m_pNewPreset, *m_pCopyPreset;
		Button			*m_pRenamePreset, *m_pDeletePreset;

		KeyValues	*m_kvChanges;
		KeyValues	*m_kvUpdates;
		KeyValues	*m_kvPanelDefaultCopy;

		InputDialog		*m_pPresetNameInput;
		MessageBox		*m_pPresetNameInputError;
	};
};

#endif