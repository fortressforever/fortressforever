#include "cbase.h"
#include "ff_customhudoptions_positionpresets.h"

#define POSITIONPRESET_FILE "HudPositionPresets.vdf"

#include "ff_customhudoptions.h"

extern CFFCustomHudAssignPresets *g_AP;

namespace vgui
{
	CFFCustomHudPositionPresets::CFFCustomHudPositionPresets(Panel *parent, char const *panelName, char const *pszComboBoxName) : BaseClass(parent, panelName, pszComboBoxName, POSITIONPRESET_FILE)
	{
		m_pAlignH = new ComboBox(this, "AlignHorizontallyCombo", 3, false);
		KeyValues *kv = new KeyValues("AH");
		kv->SetInt("Left", 0);
		m_pAlignH->AddItem("#GameUI_Left", kv);
		kv->deleteThis();
		kv = new KeyValues("AH");
		kv->SetInt("Center", 1);
		m_pAlignH->AddItem("#GameUI_Center", kv);
		kv->deleteThis();
		kv = new KeyValues("AH");
		kv->SetInt("Right", 2);
		m_pAlignH->AddItem("#GameUI_Right", kv);
		kv->deleteThis();
		m_pAlignH->ActivateItemByRow(0);
		
		m_pAlignV = new ComboBox(this, "AlignVerticallyCombo", 3, false);
		kv = new KeyValues("AV");
		kv->SetInt("Top", 0);
		m_pAlignV->AddItem("#GameUI_Top", kv);
		kv->deleteThis();
		kv = new KeyValues("AV");
		kv->SetInt("Middle", 1);
		m_pAlignV->AddItem("#GameUI_Middle", kv);
		kv->deleteThis();
		kv = new KeyValues("AV");
		kv->SetInt("Bottom", 2);
		m_pAlignV->AddItem("#GameUI_Bottom", kv);
		kv->deleteThis();
		m_pAlignV->ActivateItemByRow(0);
		
		m_pX = new CFFInputSlider(this, "X", "XInput");
		m_pX->SetRange(0, 640);
		m_pX->SetValue(0);
		
		m_pY = new CFFInputSlider(this, "Y", "YInput");
		m_pY->SetRange(0, 480);
		m_pY->SetValue(0);
		
		LoadControlSettings("resource/ui/FFOptionsSubCustomHudPositionPresets.res");
	}
	
	void CFFCustomHudPositionPresets::ActivatePresetPage()
	{
		KeyValues *kvAction = new KeyValues("ActivatePositionPage");
		PostActionSignal ( kvAction );
	}

	void CFFCustomHudPositionPresets::RegisterSelfForPresetAssignment()
	{
		if(g_AP != NULL)
			g_AP->OnPositionPresetsClassLoaded();
	}
	
	KeyValues* CFFCustomHudPositionPresets::RemoveNonEssentialValues(KeyValues *kvPreset)
	{
		KeyValues *kvTemp = new KeyValues(kvPreset->GetName());

		kvTemp->SetInt("x", kvPreset->GetInt("x", 320));
		kvTemp->SetInt("y", kvPreset->GetInt("y", 300));
		kvTemp->SetInt("alignH", kvPreset->GetInt("alignH", 1));
		kvTemp->SetInt("alignV", kvPreset->GetInt("alignV", 1));

		return kvTemp;
	}

	void CFFCustomHudPositionPresets::UpdatePresetFromControls(KeyValues *kvPreset)
	{
		BaseClass::UpdatePresetFromControls(kvPreset);

		kvPreset->SetInt("x", m_pX->GetValue());
		kvPreset->SetInt("y", m_pY->GetValue());
		kvPreset->SetInt("alignH", m_pAlignH->GetActiveItem());
		kvPreset->SetInt("alignV", m_pAlignV->GetActiveItem());
	}

	void CFFCustomHudPositionPresets::ApplyPresetToControls(KeyValues *kvPreset)
	{
		m_bPresetLoading = true;
		//this disables the preset options from registering these changes as a preset update!

		m_pX->SetValue(kvPreset->GetInt("x", 320));
		m_pY->SetValue(kvPreset->GetInt("y", 300));
		m_pAlignH->ActivateItemByRow(kvPreset->GetInt("alignH", 1));
		m_pAlignV->ActivateItemByRow(kvPreset->GetInt("alignV", 1));
		
		m_bPresetLoading = false;
		//this enables the preset options to register follwoing changes as a preset update!
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxs changing their selection
	//-----------------------------------------------------------------------------
	void CFFCustomHudPositionPresets::OnUpdateCombos(KeyValues *data)
	{
		if (m_bLoaded && !m_bPresetLoading && (data->GetPtr("panel") == m_pAlignH || data->GetPtr("panel") == m_pAlignV))
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		else
			BaseClass::OnUpdateCombos(data);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the sliders moving
	//-----------------------------------------------------------------------------
	void CFFCustomHudPositionPresets::OnUpdateSliders(KeyValues *data)
	{
		if(m_bLoaded && !m_bPresetLoading)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}

	void CFFCustomHudPositionPresets::SendUpdatedPresetNameToPresetAssignment(const char *pszPresetName)
	{
		g_AP->PositionPresetUpdated(pszPresetName);
	}
	void CFFCustomHudPositionPresets::SendRenamedPresetNameToPresetAssignment(const char *pszOldPresetName, const char *pszNewPresetName)
	{
		g_AP->PositionPresetRenamed(pszOldPresetName, pszNewPresetName);
	}
	void CFFCustomHudPositionPresets::SendDeletedPresetNameToPresetAssignment(const char *pszDeletedPresetName)
	{
		g_AP->PositionPresetDeleted(pszDeletedPresetName);
	}
	void CFFCustomHudPositionPresets::SendNewPresetNameToPresetAssignment(const char *pszPresetName, KeyValues *kvPreset)
	{
		g_AP->PositionPresetAdded(pszPresetName, kvPreset);
	}
};