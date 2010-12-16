#include "cbase.h"
#include "ff_customhudoptions_arrangementpresets.h"

#define ARRANGEMENTPRESET_FILE "HudArrangementPresets.vdf"

//this is defined in the quantitypanel page too, keep it in sync
#define QUANTITYPANELFONTSIZES 10

#include "ff_customhudoptions_assignpresets.h"
extern CFFCustomHudAssignPresets *g_AP;

namespace vgui
{
	CFFCustomHudArrangementPresets::CFFCustomHudArrangementPresets(Panel *parent, char const *panelName, char const *pszComboBoxName) : BaseClass(parent, panelName, pszComboBoxName, ARRANGEMENTPRESET_FILE)
	{
		m_pPanelRed = new CFFInputSlider(this, "PanelRed", "PanelRedInput");
		m_pPanelRed->SetRange(0, 255);
		m_pPanelRed->SetValue(255);
		
		m_pPanelGreen = new CFFInputSlider(this, "PanelGreen", "PanelGreenInput");
		m_pPanelGreen->SetRange(0, 255);
		m_pPanelGreen->SetValue(255);
		
		m_pPanelBlue = new CFFInputSlider(this, "PanelBlue", "PanelBlueInput");
		m_pPanelBlue->SetRange(0, 255);
		m_pPanelBlue->SetValue(255);
		
		m_pPanelAlpha = new CFFInputSlider(this, "PanelAlpha", "PanelAlphaInput");
		m_pPanelAlpha->SetRange(0, 255);
		m_pPanelAlpha->SetValue(255);

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
		
		m_pItemsX = new CFFInputSlider(this, "ItemsX", "ItemsXInput");
		m_pItemsX->SetRange(0, 640);
		m_pItemsX->SetValue(0);
		
		m_pItemsY = new CFFInputSlider(this, "ItemsY", "ItemsYInput");
		m_pItemsY->SetRange(0, 480);
		m_pItemsY->SetValue(0);
		
		m_pHeaderTextSize = new CFFInputSlider(this, "HeaderTextSize", "HeaderTextSizeInput");
		m_pHeaderTextSize->SetRange(1, QUANTITYPANELFONTSIZES);
		m_pHeaderTextSize->SetValue(2);

		m_pHeaderTextX = new CFFInputSlider(this, "HeaderTextX", "HeaderTextXInput");
		m_pHeaderTextX->SetRange(0, 640);
		m_pHeaderTextX->SetValue(0);
		
		m_pHeaderTextY = new CFFInputSlider(this, "HeaderTextY", "HeaderTextYInput");
		m_pHeaderTextY->SetRange(0, 480);
		m_pHeaderTextY->SetValue(0);
		
		m_pHeaderIconSize = new CFFInputSlider(this, "HeaderIconSize", "HeaderIconSizeInput");
		m_pHeaderIconSize->SetRange(1, QUANTITYPANELFONTSIZES);
		m_pHeaderIconSize->SetValue(2);

		m_pHeaderIconX = new CFFInputSlider(this, "HeaderIconX", "HeaderIconXInput");
		m_pHeaderIconX->SetRange(0, 640);
		m_pHeaderIconX->SetValue(0);
		
		m_pHeaderIconY = new CFFInputSlider(this, "HeaderIconY", "HeaderIconYInput");
		m_pHeaderIconY->SetRange(0, 480);
		m_pHeaderIconY->SetValue(0);

		m_pShowHeaderIcon = new CheckButton(this, "ShowHeaderIcon", "#GameUI_Show");
		m_pShowHeaderText = new CheckButton(this, "ShowHeaderText", "#GameUI_Show");
		m_pHeaderIconShadow = new CheckButton(this, "HeaderIconShadow", "#GameUI_DropShadow");
		m_pHeaderTextShadow = new CheckButton(this, "HeaderTextShadow", "#GameUI_DropShadow");
		m_pShowPanel = new CheckButton(this, "ShowPanel", "#GameUI_Show");
		m_pPanelColorCustom = new CheckButton(this, "PanelColorCustom", "#GameUI_CustomColor");

		m_pColumns = new CFFInputSlider(this, "Columns", "ColumnsInput");
		m_pColumns->SetRange(1, 6);
		m_pColumns->SetValue(1);
		
		LoadControlSettings("resource/ui/FFOptionsSubCustomHudArrangementPresets.res");
	}

	void CFFCustomHudArrangementPresets::RegisterSelfForPresetAssignment()
	{
		if(g_AP != NULL)
			g_AP->OnArrangementPresetsClassLoaded();
	}
	
	void CFFCustomHudArrangementPresets::UpdatePresetFromControls(KeyValues *kvPreset)
	{
		BaseClass::UpdatePresetFromControls(kvPreset);

		kvPreset->SetInt("x", m_pX->GetValue());
		kvPreset->SetInt("y", m_pY->GetValue());
		kvPreset->SetInt("alignH", m_pAlignH->GetActiveItem());
		kvPreset->SetInt("alignV", m_pAlignV->GetActiveItem());
		kvPreset->SetInt("columns", m_pColumns->GetValue());
		kvPreset->SetInt("itemsX", m_pItemsX->GetValue());
		kvPreset->SetInt("itemsY", m_pItemsY->GetValue());
		kvPreset->SetInt("showHeaderText", m_pShowHeaderText->IsSelected());
		kvPreset->SetInt("headerTextShadow", m_pHeaderTextShadow->IsSelected());
		kvPreset->SetInt("headerTextSize", m_pHeaderTextSize->GetValue() - 1);
		kvPreset->SetInt("headerTextX", m_pHeaderTextX->GetValue());
		kvPreset->SetInt("headerTextY", m_pHeaderTextY->GetValue());
		kvPreset->SetInt("showHeaderIcon", m_pShowHeaderIcon->IsSelected());
		kvPreset->SetInt("headerIconShadow", m_pHeaderIconShadow->IsSelected());
		kvPreset->SetInt("headerIconSize", m_pHeaderIconSize->GetValue() - 1);
		kvPreset->SetInt("headerIconX", m_pHeaderIconX->GetValue());
		kvPreset->SetInt("headerIconY", m_pHeaderIconY->GetValue());
		kvPreset->SetInt("showPanel", m_pShowPanel->IsSelected());
		kvPreset->SetInt("panelColorCustom", m_pPanelColorCustom->IsSelected());
		kvPreset->SetInt("panelRed", m_pPanelRed->GetValue());
		kvPreset->SetInt("panelGreen", m_pPanelGreen->GetValue());
		kvPreset->SetInt("panelBlue", m_pPanelBlue->GetValue());
		kvPreset->SetInt("panelAlpha", m_pPanelAlpha->GetValue());
	}

	void CFFCustomHudArrangementPresets::ApplyPresetToControls(KeyValues *kvPreset)
	{
		m_bPresetLoading = true;
		//this disables the preset options from registering these changes as a preset update!

		m_pX->SetValue(kvPreset->GetInt("x", 320));
		m_pY->SetValue(kvPreset->GetInt("y", 300));
		m_pAlignH->ActivateItemByRow(kvPreset->GetInt("alignH", 1));
		m_pAlignV->ActivateItemByRow(kvPreset->GetInt("alignV", 1));
		m_pColumns->SetValue(kvPreset->GetInt("columns", 2));
		m_pItemsX->SetValue(kvPreset->GetInt("itemsX", 5));
		m_pItemsY->SetValue(kvPreset->GetInt("itemsY", 20));
		m_pShowHeaderText->SetSelected(kvPreset->GetInt("showHeaderText", 1));
		m_pHeaderTextShadow->SetSelected(kvPreset->GetInt("headerTextShadow", 0));
		m_pHeaderTextSize->SetValue(kvPreset->GetInt("headerTextSize", 3) + 1);
		m_pHeaderTextX->SetValue(kvPreset->GetInt("headerTextX", 20));
		m_pHeaderTextY->SetValue(kvPreset->GetInt("headerTextY", 7));
		m_pShowHeaderIcon->SetSelected(kvPreset->GetInt("showHeaderIcon", 1));
		m_pHeaderIconShadow->SetSelected(kvPreset->GetInt("headerIconShadow", 0));
		m_pHeaderIconSize->SetValue(kvPreset->GetInt("headerIconSize", 3) + 1);
		m_pHeaderIconX->SetValue(kvPreset->GetInt("headerIconX", 3));
		m_pHeaderIconY->SetValue(kvPreset->GetInt("headerIconY", 3));
		m_pShowPanel->SetSelected(kvPreset->GetInt("showPanel", 1));
		m_pPanelColorCustom->SetSelected(kvPreset->GetInt("panelColorCustom", 0));
		m_pPanelRed->SetValue(kvPreset->GetInt("panelRed", 255));
		m_pPanelGreen->SetValue(kvPreset->GetInt("panelGreen", 255));
		m_pPanelBlue->SetValue(kvPreset->GetInt("panelBlue", 255));
		m_pPanelAlpha->SetValue(kvPreset->GetInt("panelAlpha", 255));
		
		m_bPresetLoading = false;
		//this enables the preset options to register follwoing changes as a preset update!
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxs changing their selection
	//-----------------------------------------------------------------------------
	void CFFCustomHudArrangementPresets::OnUpdateCombos(KeyValues *data)
	{
		if (m_bLoaded && !m_bPresetLoading && (data->GetPtr("panel") == m_pAlignH || data->GetPtr("panel") == m_pAlignV))
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		else
			BaseClass::OnUpdateCombos(data);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the sliders moving
	//-----------------------------------------------------------------------------
	void CFFCustomHudArrangementPresets::OnUpdateSliders(KeyValues *data)
	{
		if(m_bLoaded && !m_bPresetLoading)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the checkbox selection changes
	//-----------------------------------------------------------------------------
	void CFFCustomHudArrangementPresets::OnUpdateCheckbox(KeyValues *data)
	{
		if(m_bLoaded && !m_bPresetLoading)
		{	
			if (data->GetPtr("panel") == m_pShowHeaderIcon)
			{
				m_pHeaderIconX->SetEnabled(m_pShowHeaderIcon->IsSelected());
				m_pHeaderIconY->SetEnabled(m_pShowHeaderIcon->IsSelected());
				m_pHeaderIconSize->SetEnabled(m_pShowHeaderIcon->IsSelected());
			}
			else if(data->GetPtr("panel") == m_pShowHeaderText)
			{
				m_pHeaderTextX->SetEnabled(m_pShowHeaderText->IsSelected());
				m_pHeaderTextY->SetEnabled(m_pShowHeaderText->IsSelected());
				m_pHeaderTextSize->SetEnabled(m_pShowHeaderText->IsSelected());
			}
			else if(data->GetPtr("panel") == m_pShowPanel)
			{
				m_pPanelRed->SetEnabled(m_pShowPanel->IsSelected());
				m_pPanelGreen->SetEnabled(m_pShowPanel->IsSelected());
				m_pPanelBlue->SetEnabled(m_pShowPanel->IsSelected());
				m_pPanelAlpha->SetEnabled(m_pShowPanel->IsSelected());
			}
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		}
	}

	void CFFCustomHudArrangementPresets::SendUpdatedPresetNameToPresetAssignment(const char *pszPresetName)
	{
		g_AP->ArrangementPresetUpdated(pszPresetName);
	}
	void CFFCustomHudArrangementPresets::SendRenamedPresetNameToPresetAssignment(const char *pszOldPresetName, const char *pszNewPresetName)
	{
		g_AP->ArrangementPresetRenamed(pszOldPresetName, pszNewPresetName);
	}
	void CFFCustomHudArrangementPresets::SendDeletedPresetNameToPresetAssignment(const char *pszDeletedPresetName)
	{
		g_AP->ArrangementPresetDeleted(pszDeletedPresetName);
	}
	void CFFCustomHudArrangementPresets::SendNewPresetNameToPresetAssignment(const char *pszPresetName, KeyValues *kvPreset)
	{
		g_AP->ArrangementPresetAdded(pszPresetName, kvPreset);
	}
};