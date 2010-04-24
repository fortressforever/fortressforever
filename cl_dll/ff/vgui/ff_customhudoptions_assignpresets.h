#ifndef FF_CUSTOMHUDOPTIONS_ASSIGNPRESETS_H
#define FF_CUSTOMHUDOPTIONS_ASSIGNPRESETS_H

#include "ff_optionspage.h"
#include "ff_customhudoptions_panelstylepresets.h"
#include "ff_customhudoptions_itemstylepresets.h"
#include "ff_customhudoptions_positionpresets.h"

#include "keyvalues.h"
#include "filesystem.h"
#include "ff_quantitypanel.h"

#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Button.h>

#define	PRESETCOMOBO_ROW_HEIGHT 30
#define	PRESETCOMOBO_X 180
#define	PRESETCOMOBO_Y 80

using namespace vgui;

class CFFCustomHudAssignPresets : public CFFOptionsPage
{
	DECLARE_CLASS_SIMPLE(CFFCustomHudAssignPresets, CFFOptionsPage);
	
public:
	CFFCustomHudAssignPresets(Panel *parent, char const *panelName, CFFCustomHudItemStylePresets* stylePresetsClass, CFFCustomHudPanelStylePresets* arrangementPresetsClass, CFFCustomHudPositionPresets* positionPresetsClass);
	~CFFCustomHudAssignPresets();

	virtual void Load();
	virtual void Reset();
	virtual void Apply();
	
	bool IsReady();
	void ApplyAssignmentToControls(KeyValues *kvAssignment);
	void UpdateSecondaryComboFromParentKey(KeyValues *kvPrimary);

	void ItemStylePresetPreviewUpdated(const char *pszPresetName, KeyValues *kvPresetPreview);
	void ItemStylePresetUpdated(const char *pszPresetName);
	void ItemStylePresetRenamed(const char *pszOldPresetName, const char *pszNewPresetName);
	void ItemStylePresetDeleted(const char *pszDeletedPresetName);
	void ItemStylePresetAdded(const char *pszPresetName, KeyValues *kvPreset);

	void PanelStylePresetPreviewUpdated(const char *pszPresetName, KeyValues *kvPresetPreview);
	void PanelStylePresetUpdated(const char *pszPresetName);
	void PanelStylePresetRenamed(const char *pszOldPresetName, const char *pszNewPresetName);
	void PanelStylePresetDeleted(const char *pszDeletedPresetName);
	void PanelStylePresetAdded(const char *pszPresetName, KeyValues *kvPreset);

	void PositionPresetPreviewUpdated(const char *pszPresetName, KeyValues *kvPresetPreview);
	void PositionPresetUpdated(const char *pszPresetName);
	void PositionPresetRenamed(const char *pszOldPresetName, const char *pszNewPresetName);
	void PositionPresetDeleted(const char *pszDeletedPresetName);
	void PositionPresetAdded(const char *pszPresetName, KeyValues *kvPreset);

	void OnItemStylePresetsClassLoaded();
	void OnPanelStylePresetsClassLoaded();
	void OnPositionPresetsClassLoaded();

	KeyValues* GetPresetAssignments();
	void RegisterAssignmentChange(const char* keyName, ComboBox* comboBox);

	KeyValues* GetStyleDataForAssignment(KeyValues *kvAssignment);
	void SendStyleDataToAssignment(KeyValues *kvAssignment);
	void SendPresetPreviewDataToAssignment(KeyValues *kvAssignment, KeyValues *kvPresetData);
	void CreatePresetFromPanelDefault(KeyValues *kvPresetData);
private:
	//-----------------------------------------------------------------------------
	// Purpose: Catch quantity panels requesting to be added for 
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnAddQuantityPanel, "AddQuantityPanel", data);

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxs changing their selection
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data);

	//-----------------------------------------------------------------------------
	// Purpose: Catch the checkboxes changing their state
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateCheckButton, "CheckButtonChecked", data);
	
	//-----------------------------------------------------------------------------
	// Purpose: Catch the "Copy Default" buttons
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data);

	Panel			*m_pPanelSpecificOptions; 

	ComboBox		*m_pPrimary, *m_pSecondary;
	ComboBox		*m_pItemStylePresets, *m_pPanelStylePresets, *m_pPositionPresets;
	
	CheckButton		*m_pPreviewMode;

	Button			*m_pCopyDefaultItemStyle, *m_pCopyDefaultPanelStyle, *m_pCopyDefaultPosition;

	KeyValues		*m_kvLoadedAssignments;
	KeyValues		*m_kvRequiredUpdates;

	CFFCustomHudPanelStylePresets *m_pPanelStylePresetsClass;
	CFFCustomHudItemStylePresets *m_pItemStylePresetsClass;
	CFFCustomHudPositionPresets *m_pPositionPresetsClass;

	bool m_bCopyDefaultPosition;
	bool m_bCopyDefaultPanelStyle;
	bool m_bCopyDefaultItemStyle;

	bool m_bLoaded;
	int iYCoords;
	
	void RegisterAssignmentChange(KeyValues *kvSelf, KeyValues* kvParent = NULL);
};

extern CFFCustomHudAssignPresets *g_AP;

#endif