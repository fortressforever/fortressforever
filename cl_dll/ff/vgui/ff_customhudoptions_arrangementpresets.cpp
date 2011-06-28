#include "cbase.h"
#include "ff_customhudoptions_arrangementpresets.h"

#define ARRANGEMENTPRESET_FILE "HudArrangementPresets.vdf"

//this is defined in the quantitypanel page too, keep it in sync
#define QUANTITYPANELFONTSIZES 10
#define QUANTITYBARICONSIZES 15

#include "ff_customhudoptions.h"

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
		
		m_pItemsX = new CFFInputSlider(this, "ItemsX", "ItemsXInput");
		m_pItemsX->SetRange(0, 50);
		m_pItemsX->SetValue(0);
		
		m_pItemsY = new CFFInputSlider(this, "ItemsY", "ItemsYInput");
		m_pItemsY->SetRange(0, 50);
		m_pItemsY->SetValue(0);
		
		m_pHeaderTextSize = new CFFInputSlider(this, "HeaderTextSize", "HeaderTextSizeInput");
		m_pHeaderTextSize->SetRange(1, QUANTITYPANELFONTSIZES);
		m_pHeaderTextSize->SetValue(2);

		m_pHeaderTextX = new CFFInputSlider(this, "HeaderTextX", "HeaderTextXInput");
		m_pHeaderTextX->SetRange(0, 160);
		m_pHeaderTextX->SetValue(0);
		
		m_pHeaderTextY = new CFFInputSlider(this, "HeaderTextY", "HeaderTextYInput");
		m_pHeaderTextY->SetRange(0, 120);
		m_pHeaderTextY->SetValue(0);
		
		m_pHeaderIconSize = new CFFInputSlider(this, "HeaderIconSize", "HeaderIconSizeInput");
		m_pHeaderIconSize->SetRange(1, QUANTITYBARICONSIZES);
		m_pHeaderIconSize->SetValue(2);

		m_pHeaderIconX = new CFFInputSlider(this, "HeaderIconX", "HeaderIconXInput");
		m_pHeaderIconX->SetRange(0, 160);
		m_pHeaderIconX->SetValue(0);
		
		m_pHeaderIconY = new CFFInputSlider(this, "HeaderIconY", "HeaderIconYInput");
		m_pHeaderIconY->SetRange(0, 120);
		m_pHeaderIconY->SetValue(0);

		m_pTextSize = new CFFInputSlider(this, "TextSize", "TextSizeInput");
		m_pTextSize->SetRange(1, QUANTITYPANELFONTSIZES);
		m_pTextSize->SetValue(2);

		m_pTextX = new CFFInputSlider(this, "TextX", "TextXInput");
		m_pTextX->SetRange(0, 160);
		m_pTextX->SetValue(0);
		
		m_pTextY = new CFFInputSlider(this, "TextY", "TextYInput");
		m_pTextY->SetRange(0, 120);
		m_pTextY->SetValue(0);
		
		m_pShowHeaderIcon = new CheckButton(this, "ShowHeaderIcon", "#GameUI_Show");
		m_pShowHeaderText = new CheckButton(this, "ShowHeaderText", "#GameUI_Show");
		m_pShowText = new CheckButton(this, "ShowText", "#GameUI_Show");
		m_pHeaderIconShadow = new CheckButton(this, "HeaderIconShadow", "#GameUI_DropShadow");
		m_pHeaderTextShadow = new CheckButton(this, "HeaderTextShadow", "#GameUI_DropShadow");
		m_pTextShadow = new CheckButton(this, "TextShadow", "#GameUI_DropShadow");
		m_pShowPanel = new CheckButton(this, "ShowPanel", "#GameUI_Show");
		m_pPanelColorCustom = new CheckButton(this, "PanelColorCustom", "#GameUI_CustomColor");

		m_pColumns = new CFFInputSlider(this, "Columns", "ColumnsInput");
		m_pColumns->SetRange(1, 6);
		m_pColumns->SetValue(1);
		
		LoadControlSettings("resource/ui/FFOptionsSubCustomHudArrangementPresets.res");
	}
	
	void CFFCustomHudArrangementPresets::ActivatePresetPage()
	{
		KeyValues *kvAction = new KeyValues("ActivateArrangementPage");
		PostActionSignal ( kvAction );
	}
	
	void CFFCustomHudArrangementPresets::SetControlsEnabled(bool bEnabled)
	{
		m_pColumns->SetEnabled( bEnabled );
		m_pItemsX->SetEnabled( bEnabled );
		m_pItemsY->SetEnabled( bEnabled );
		m_pHeaderTextSize->SetEnabled( bEnabled );
		m_pHeaderTextX->SetEnabled( bEnabled );
		m_pHeaderTextY->SetEnabled( bEnabled );
		m_pHeaderIconSize->SetEnabled( bEnabled );
		m_pHeaderIconX->SetEnabled( bEnabled );
		m_pHeaderIconY->SetEnabled( bEnabled );
		m_pTextSize->SetEnabled( bEnabled );
		m_pTextX->SetEnabled( bEnabled );
		m_pTextY->SetEnabled( bEnabled );
		m_pShowHeaderIcon->SetEnabled( bEnabled );
		m_pShowHeaderText->SetEnabled( bEnabled );
		m_pShowText->SetEnabled( bEnabled );
		m_pHeaderIconShadow->SetEnabled( bEnabled );
		m_pHeaderTextShadow->SetEnabled( bEnabled );
		m_pTextShadow->SetEnabled( bEnabled );
		m_pShowPanel->SetEnabled( bEnabled );
		m_pPanelColorCustom->SetEnabled( bEnabled );


		//if we're disabling
		if(!bEnabled)
		//disable all
		{
			m_pPanelRed->SetEnabled( bEnabled );
			m_pPanelGreen->SetEnabled( bEnabled );
			m_pPanelBlue->SetEnabled( bEnabled );
			m_pPanelAlpha->SetEnabled( bEnabled );
		}
		//else we're enabling
		else
		//only enable based upon what is selected for custom panel colour
		{
			if(m_pPanelColorCustom->IsSelected())
			{
				m_pPanelRed->SetEnabled( bEnabled );
				m_pPanelGreen->SetEnabled( bEnabled );
				m_pPanelBlue->SetEnabled( bEnabled );
				m_pPanelAlpha->SetEnabled( bEnabled );
			}
			//TODO: else might need to force false - might be sensible but not necessary??
		}
	}
	//-----------------------------------------------------------------------------
	// Purpose: Tells the assignment class that this preset class is loaded
	//-----------------------------------------------------------------------------	
	void CFFCustomHudArrangementPresets::RegisterSelfForPresetAssignment()
	{
		if(g_AP != NULL)
			g_AP->OnArrangementPresetsClassLoaded();
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: Cleans up preset.. we might add remove alter values in the future
	//-----------------------------------------------------------------------------	
	KeyValues* CFFCustomHudArrangementPresets::RemoveNonEssentialValues(KeyValues *kvPreset)
	{
		KeyValues *kvTemp = new KeyValues(kvPreset->GetName());

		kvTemp->SetInt("columns", kvPreset->GetInt("columns", 2));
		kvTemp->SetInt("itemsX", kvPreset->GetInt("itemsX", 5));
		kvTemp->SetInt("itemsY", kvPreset->GetInt("itemsY", 20));
		kvTemp->SetInt("showHeaderText", kvPreset->GetInt("showHeaderText", 1));
		kvTemp->SetInt("headerTextShadow", kvPreset->GetInt("headerTextShadow", 0));
		kvTemp->SetInt("headerTextSize", kvPreset->GetInt("headerTextSize", 3));
		kvTemp->SetInt("headerTextX", kvPreset->GetInt("headerTextX", 20));
		kvTemp->SetInt("headerTextY", kvPreset->GetInt("headerTextY", 7));
		kvTemp->SetInt("showHeaderIcon", kvPreset->GetInt("showHeaderIcon", 1));
		kvTemp->SetInt("headerIconShadow", kvPreset->GetInt("headerIconShadow", 0));
		kvTemp->SetInt("headerIconSize", kvPreset->GetInt("headerIconSize", 3));
		kvTemp->SetInt("headerIconX", kvPreset->GetInt("headerIconX", 3));
		kvTemp->SetInt("headerIconY", kvPreset->GetInt("headerIconY", 3));
		kvTemp->SetInt("showText", kvPreset->GetInt("showText", 1));
		kvTemp->SetInt("textShadow", kvPreset->GetInt("textShadow", 0));
		kvTemp->SetInt("textSize", kvPreset->GetInt("textSize", 3));
		kvTemp->SetInt("textX", kvPreset->GetInt("textX", 25));
		kvTemp->SetInt("textY", kvPreset->GetInt("textY", 20));
		kvTemp->SetInt("showPanel", kvPreset->GetInt("showPanel", 1));
		kvTemp->SetInt("panelColorCustom", kvPreset->GetInt("panelColorCustom", 0));
		kvTemp->SetInt("panelRed", kvPreset->GetInt("panelRed", 255));
		kvTemp->SetInt("panelGreen", kvPreset->GetInt("panelGreen", 255));
		kvTemp->SetInt("panelBlue", kvPreset->GetInt("panelBlue", 255));
		kvTemp->SetInt("panelAlpha", kvPreset->GetInt("panelAlpha", 255));

		return kvTemp;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the currently selected preset from the controls
	//-----------------------------------------------------------------------------	
	void CFFCustomHudArrangementPresets::UpdatePresetFromControls(KeyValues *kvPreset)
	{
		BaseClass::UpdatePresetFromControls(kvPreset);

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
		kvPreset->SetInt("showText", m_pShowText->IsSelected());
		kvPreset->SetInt("textShadow", m_pTextShadow->IsSelected());
		kvPreset->SetInt("textSize", m_pTextSize->GetValue() - 1);
		kvPreset->SetInt("textX", m_pTextX->GetValue());
		kvPreset->SetInt("textY", m_pTextY->GetValue());
		kvPreset->SetInt("showPanel", m_pShowPanel->IsSelected());
		kvPreset->SetInt("panelColorCustom", m_pPanelColorCustom->IsSelected());
		kvPreset->SetInt("panelRed", m_pPanelRed->GetValue());
		kvPreset->SetInt("panelGreen", m_pPanelGreen->GetValue());
		kvPreset->SetInt("panelBlue", m_pPanelBlue->GetValue());
		kvPreset->SetInt("panelAlpha", m_pPanelAlpha->GetValue());

		//TODO: send preset to preview
	}

	//-----------------------------------------------------------------------------
	// Purpose: Apply the selected preset to the contols
	//-----------------------------------------------------------------------------	
	void CFFCustomHudArrangementPresets::ApplyPresetToControls(KeyValues *kvPreset)
	{
		m_bPresetLoading = true;
		//this disables the preset options from registering these changes as a preset update!

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
		m_pShowText->SetSelected(kvPreset->GetInt("showText", 1));
		m_pTextShadow->SetSelected(kvPreset->GetInt("textShadow", 0));
		m_pTextSize->SetValue(kvPreset->GetInt("textSize", 3) + 1);
		m_pTextX->SetValue(kvPreset->GetInt("textX", 25));
		m_pTextY->SetValue(kvPreset->GetInt("textY", 20));
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
	// Purpose: Update the component controls from the selected Component
	//-----------------------------------------------------------------------------	
	void CFFCustomHudArrangementPresets::OnUpdateCombos(KeyValues *data)
	{
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
			else if(data->GetPtr("panel") == m_pShowText)
			{
				m_pTextX->SetEnabled(m_pShowText->IsSelected());
				m_pTextY->SetEnabled(m_pShowText->IsSelected());
				m_pTextSize->SetEnabled(m_pShowText->IsSelected());
			}
			else if(data->GetPtr("panel") == m_pShowPanel)
			{
				m_pPanelColorCustom->SetEnabled(m_pShowPanel->IsSelected());
				if(m_pPanelColorCustom->IsSelected())
				{
					m_pPanelRed->SetEnabled(m_pShowPanel->IsSelected());
					m_pPanelGreen->SetEnabled(m_pShowPanel->IsSelected());
					m_pPanelBlue->SetEnabled(m_pShowPanel->IsSelected());
					m_pPanelAlpha->SetEnabled(m_pShowPanel->IsSelected());
				}
			}
			else if(data->GetPtr("panel") == m_pPanelColorCustom)
			{	
				m_pPanelRed->SetEnabled(m_pPanelColorCustom->IsSelected());
				m_pPanelGreen->SetEnabled(m_pPanelColorCustom->IsSelected());
				m_pPanelBlue->SetEnabled(m_pPanelColorCustom->IsSelected());
				m_pPanelAlpha->SetEnabled(m_pPanelColorCustom->IsSelected());	
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