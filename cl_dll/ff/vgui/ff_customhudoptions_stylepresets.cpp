#include "cbase.h"
#include "ff_customhudoptions_stylepresets.h"

#define STYLEPRESET_FILE "HudStylePresets.vdf"

//this is defined in the quantitybar page too, keep it in sync
#define QUANTITYBARFONTSIZES 10
#define QUANTITYBARICONSIZES 15

#include "ff_customhudoptions_assignpresets.h"
extern CFFCustomHudAssignPresets *g_AP;

namespace vgui
{
	CFFCustomHudStylePresets::CFFCustomHudStylePresets(Panel *parent, char const *panelName, char const *pszComboBoxName) : BaseClass(parent, panelName, pszComboBoxName, STYLEPRESET_FILE)
	{
		m_pBarWidth = new CFFInputSlider(this, "BarWidth", "BarWidthInput");
		m_pBarWidth->SetRange(0, 120);
		m_pBarWidth->SetValue(0);
		
		m_pBarHeight = new CFFInputSlider(this, "BarHeight", "BarHeightInput");
		m_pBarHeight->SetRange(0, 120);
		m_pBarHeight->SetValue(0);
		
		m_pBarBorderWidth = new CFFInputSlider(this, "BarBorderWidth", "BarBorderWidthInput");
		m_pBarBorderWidth->SetRange(1, 5);
		m_pBarBorderWidth->SetValue(1);
		
		m_pBarMarginHorizontal = new CFFInputSlider(this, "BarMarginHorizontal", "BarMarginHorizontalInput");
		m_pBarMarginHorizontal->SetRange(0, 20);
		m_pBarMarginHorizontal->SetValue(5);
		
		m_pBarMarginVertical = new CFFInputSlider(this, "BarMarginVertical", "BarMarginVerticalInput");
		m_pBarMarginVertical->SetRange(0, 20);
		m_pBarMarginVertical->SetValue(5);

		m_pSizeLabel = new Label(this, "SizeLabel", "#GameUI_Size");
		m_pTextLabel = new Label(this, "TextLabel", "#GameUI_Text");
		m_pOffsetXLabel = new Label(this, "OffsetXLabel", "##GameUI_X");
		m_pOffsetYLabel = new Label(this, "OffsetYLabel", "#GameUI_Y");
		m_pOffsetLabel = new Label(this, "OffsetLabel", "#GameUI_Offset");
		m_pAlignHLabel = new Label(this, "AlignHorizontallyLabel", "#GameUI_Horizontal");
		m_pAlignVLabel = new Label(this, "AlignVerticallyLabel", "#GameUI_Vertical");
		m_pAlignLabel = new Label(this, "AlignmentLabel", "#GameUI_Alignment");

		m_pOffsetX = new CFFInputSlider(this, "OffsetX", "OffsetXInput");
		m_pOffsetX->SetRange(-120, 120);
		m_pOffsetX->SetValue(0);
		
		m_pOffsetY = new CFFInputSlider(this, "OffsetY", "OffsetYInput");
		m_pOffsetY->SetRange(-120, 120);
		m_pOffsetY->SetValue(0);
		
		m_pSize = new CFFInputSlider(this, "Size", "SizeInput");
		m_pSize->SetRange(1, QUANTITYBARFONTSIZES);
		m_pSize->SetValue(2);
		
		m_pRed = new CFFInputSlider(this, "Red", "RedInput");
		m_pRed->SetRange(0, 255);
		m_pRed->SetValue(255);
		
		m_pGreen = new CFFInputSlider(this, "Green", "GreenInput");
		m_pGreen->SetRange(0, 255);
		m_pGreen->SetValue(255);
		
		m_pBlue = new CFFInputSlider(this, "Blue", "BlueInput");
		m_pBlue->SetRange(0, 255);
		m_pBlue->SetValue(255);
		
		m_pAlpha = new CFFInputSlider(this, "Alpha", "AlphaInput");
		m_pAlpha->SetRange(0, 255);
		m_pAlpha->SetValue(255);

		m_pShadow = new CheckButton(this, "Shadow", "#GameUI_DropShadow");
		m_pShow = new CheckButton(this, "Show", "#GameUI_Show");
		m_pFontTahoma = new CheckButton(this, "FontTahoma", "#GameUI_UseTahomaFont");

		m_pColorMode = new ComboBox(this, "ColorModeCombo", 4, false);
		KeyValues *kv = new KeyValues("Custom");
		kv->SetInt("Custom", 0);
		m_pColorMode->AddItem("#GameUI_Custom", kv);
		kv->deleteThis();
		kv = new KeyValues("Stepped");
		kv->SetInt("Stepped", 1);
		m_pColorMode->AddItem("#GameUI_Stepped", kv);
		kv->deleteThis();
		kv = new KeyValues("Faded");
		kv->SetInt("Faded", 2);
		m_pColorMode->AddItem("#GameUI_Faded", kv);
		kv->deleteThis();
		kv = new KeyValues("TeamColored");
		kv->SetInt("TeamColored", 3);
		m_pColorMode->AddItem("#GameUI_TeamColored", kv);
		kv->deleteThis();
		m_pColorMode->ActivateItemByRow(0);

		m_pAlignH = new ComboBox(this, "AlignHorizontallyCombo", 3, false);
		kv = new KeyValues("AH");
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

		m_pBarOrientation = new ComboBox(this, "BarOrientationCombo", 4, false);
		kv = new KeyValues("BO");
		kv->SetInt("Horizontal", 0);
		m_pBarOrientation->AddItem("#GameUI_Horizontal", kv);
		kv->deleteThis();
		kv = new KeyValues("BO");
		kv->SetInt("Vertical", 1);
		m_pBarOrientation->AddItem("#GameUI_Vertical", kv);
		kv->deleteThis();
		kv = new KeyValues("BO");
		kv->SetInt("InvertHorizontal", 2);
		m_pBarOrientation->AddItem("#GameUI_InvertHorizontal", kv);
		kv->deleteThis();
		kv = new KeyValues("BO");
		kv->SetInt("InvertVertical", 3);
		m_pBarOrientation->AddItem("#GameUI_InvertVertical", kv);
		kv->deleteThis();
		m_pBarOrientation->ActivateItemByRow(0);

		m_pComponentSelection = new ComboBox(this, "ComponentSelectionCombo", 6, false);
		//we add these in ApplyPresetToControls
		
		LoadControlSettings("resource/ui/FFOptionsSubCustomHudStylePresets.res");
	}

	
	void CFFCustomHudStylePresets::RegisterSelfForPresetAssignment()
	{
		if(g_AP != NULL)
			g_AP->OnStylePresetsClassLoaded();
	}

	void CFFCustomHudStylePresets::UpdatePresetFromControls(KeyValues *kvPreset)
	{
		BaseClass::UpdatePresetFromControls(kvPreset);
		//update the selected component (bar,barBorder,icon,label) with the current values
		KeyValues *kvComponent = m_pComponentSelection->GetActiveItemUserData();
		
		kvComponent->SetInt("show", m_pShow->IsSelected());
		kvComponent->SetInt("colorMode", m_pColorMode->GetActiveItem());
		kvComponent->SetInt("red", m_pRed->GetValue());
		kvComponent->SetInt("green", m_pGreen->GetValue());
		kvComponent->SetInt("blue", m_pBlue->GetValue());
		kvComponent->SetInt("alpha", m_pAlpha->GetValue());
		if(m_pShadow->IsEnabled())
			kvComponent->SetInt("shadow", m_pShadow->IsSelected());
		if(m_pSize->IsEnabled())
			kvComponent->SetInt("size", m_pSize->GetValue() - 1);
		if(m_pAlignH->IsEnabled())
			kvComponent->SetInt("alignH", m_pAlignH->GetActiveItem());
		if(m_pAlignV->IsEnabled())
			kvComponent->SetInt("alignV", m_pAlignV->GetActiveItem());
		if(m_pOffsetX->IsEnabled())
			kvComponent->SetInt("offsetX", m_pOffsetX->GetValue());
		if(m_pOffsetY->IsEnabled())
			kvComponent->SetInt("offsetY", m_pOffsetY->GetValue());
		if(m_pFontTahoma->IsEnabled())
			kvComponent->SetInt("fontTahoma", m_pFontTahoma->IsSelected());

		//update the main preset, damn thing copies rather than keeping the same pointer
		//
		//only save the preset if it has all the components
		//they should get fixed when loaded so we shouln't really ever have a problem anyway
		kvPreset->Clear();
		if(m_pComponentSelection->GetItemCount() == 6)
		{
			//might not need to use same name??
			//get the 6 components (bar, barborder, barBackground, icon, label,amount) and add them as subkeys
			for(int i = 0; i < 6; ++i)
			{
				KeyValues *kvComponent = new KeyValues(m_pComponentSelection->GetItemUserData(i)->GetName());
				m_pComponentSelection->GetItemUserData(i)->CopySubkeys(kvComponent);
				kvPreset->AddSubKey(kvComponent);
			}

			kvPreset->SetInt("barWidth", m_pBarWidth->GetValue());
			kvPreset->SetInt("barHeight", m_pBarHeight->GetValue());
			kvPreset->SetInt("barBorderWidth", m_pBarBorderWidth->GetValue());
			kvPreset->SetInt("barMarginHorizontal", m_pBarMarginHorizontal->GetValue());
			kvPreset->SetInt("barMarginVertical", m_pBarMarginVertical->GetValue());
			kvPreset->SetInt("barOrientation", m_pBarOrientation->GetActiveItem());
		}
	}

	void CFFCustomHudStylePresets::ApplyPresetToControls(KeyValues *kvPreset)
	{
		m_bPresetLoading = true;
		//this disables the preset options from registering these changes as a preset update!

		m_pBarWidth->SetValue(kvPreset->GetInt("barWidth", 60));
		m_pBarHeight->SetValue(kvPreset->GetInt("barHeight", 10));
		m_pBarBorderWidth->SetValue(kvPreset->GetInt("barBorderWidth", 1));
		m_pBarMarginHorizontal->SetValue(kvPreset->GetInt("barMarginHorizontal", 10));
		m_pBarMarginVertical->SetValue(kvPreset->GetInt("barMarginVertical", 10));
		m_pBarOrientation->ActivateItemByRow(kvPreset->GetInt("barOrientation", 0));

		int iMenuItemToShow = 0;

		if(m_pComponentSelection->GetItemCount() > 0)
			iMenuItemToShow = m_pComponentSelection->GetActiveItem();
		//remove and readd the items so we can detect if there's something wrong with the loading preset
	
		//lets remove the current component items
		m_pComponentSelection->RemoveAll();
		
		// now lets rebuild the dropdown with current data

		//see if component data exists
		KeyValues *kvComponent = kvPreset->FindKey("Bar");
		if(kvComponent)	
		//if it does
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_Bar", kvComponent);
		else
		{
			//create the component using these defaults
			kvComponent = new KeyValues("Bar");
			kvComponent->SetInt("show", 1);
			kvComponent->SetInt("colorMode", 2);
			kvComponent->SetInt("red", 255);
			kvComponent->SetInt("green", 255);
			kvComponent->SetInt("blue", 255);
			kvComponent->SetInt("alpha", 255);
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_Bar", kvComponent);
			
			//also add component data to the preset
			kvPreset->AddSubKey(kvComponent);
		}
		
		kvComponent = kvPreset->FindKey("BarBorder");
		if(kvComponent)	
		//if it does
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_BarBorder", kvComponent);
		else
		{
			//create the component using these defaults
			kvComponent = new KeyValues("BarBorder");
			kvComponent->SetInt("show", 1);
			kvComponent->SetInt("colorMode", 0);
			kvComponent->SetInt("red", 255);
			kvComponent->SetInt("green", 255);
			kvComponent->SetInt("blue", 255);
			kvComponent->SetInt("alpha", 255);
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_BarBorder", kvComponent);
			
			//also add component data to the preset
			kvPreset->AddSubKey(kvComponent);
		}
		
		kvComponent = kvPreset->FindKey("BarBackground");
		if(kvComponent)	
			m_pComponentSelection->AddItem("#GameUI_BarBackground", kvComponent);
		else
		{
			//create the component using these defaults
			kvComponent = new KeyValues("BarBackground");
			kvComponent->SetInt("show", 1);
			kvComponent->SetInt("colorMode", 2);
			kvComponent->SetInt("red", 255);
			kvComponent->SetInt("green", 255);
			kvComponent->SetInt("blue", 255);
			kvComponent->SetInt("alpha", 96);
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_BarBackground", kvComponent);
			
			//also add component data to the preset
			kvPreset->AddSubKey(kvComponent);
		}
		kvComponent = kvPreset->FindKey("Icon");
		if(kvComponent)	
			m_pComponentSelection->AddItem("#GameUI_Icon", kvComponent);
		else
		{
			//create the component using these defaults
			kvComponent = new KeyValues("Icon");
			kvComponent->SetInt("show", 1);
			kvComponent->SetInt("colorMode", 0);
			kvComponent->SetInt("red", 255);
			kvComponent->SetInt("green", 255);
			kvComponent->SetInt("blue", 255);
			kvComponent->SetInt("alpha", 255);
			kvComponent->SetInt("shadow", 0);
			kvComponent->SetInt("size", 4);
			kvComponent->SetInt("alignH", 2);
			kvComponent->SetInt("alignV", 1);
			kvComponent->SetInt("offsetX", 2);
			kvComponent->SetInt("offsetY", 0);
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_Icon", kvComponent);
			
			//also add component data to the preset
			kvPreset->AddSubKey(kvComponent);
		}
		
		kvComponent = kvPreset->FindKey("Label");
		if(kvComponent)
			m_pComponentSelection->AddItem("#GameUI_Label", kvComponent);
		else
		{
			//create the component using these defaults
			kvComponent = new KeyValues("Label");
			kvComponent->SetInt("show", 1);
			kvComponent->SetInt("colorMode", 0);
			kvComponent->SetInt("red", 255);
			kvComponent->SetInt("green", 255);
			kvComponent->SetInt("blue", 255);
			kvComponent->SetInt("alpha", 255);
			kvComponent->SetInt("shadow", 0);
			kvComponent->SetInt("size", 4);
			kvComponent->SetInt("alignH", 0);
			kvComponent->SetInt("alignV", 1);
			kvComponent->SetInt("offsetX", -2);
			kvComponent->SetInt("offsetY", 0);
			kvComponent->SetInt("fontTahoma", 0);
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_Label", kvComponent);
			
			//also add component data to the preset
			kvPreset->AddSubKey(kvComponent);
		}
		
		kvComponent = kvPreset->FindKey("Amount");
		if(kvComponent)	
			m_pComponentSelection->AddItem("#GameUI_Amount", kvComponent);
		else
		{
			//create the component using these defaults
			kvComponent = new KeyValues("Amount");
			kvComponent->SetInt("show", 1);
			kvComponent->SetInt("colorMode", 0);
			kvComponent->SetInt("red", 0);
			kvComponent->SetInt("green", 0);
			kvComponent->SetInt("blue", 0);
			kvComponent->SetInt("alpha", 255);
			kvComponent->SetInt("shadow", 0);
			kvComponent->SetInt("size", 4);
			kvComponent->SetInt("alignH", 1);
			kvComponent->SetInt("alignV", 1);
			kvComponent->SetInt("offsetX", 0);
			kvComponent->SetInt("offsetY", 1);
			kvComponent->SetInt("fontTahoma", 0);
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_Amount", kvComponent);
			
			//also add component data to the preset
			kvPreset->AddSubKey(kvComponent);	
		}

		//select the item which was previously selected (might be default 0)
		m_pComponentSelection->ActivateItemByRow(iMenuItemToShow);

		UpdateComponentControls(m_pComponentSelection->GetActiveItemUserData());

		m_bPresetLoading = false;
		//this enables the preset options to register follwoing changes as a preset update!
	}

	void CFFCustomHudStylePresets::UpdateComponentControls(KeyValues *kvComponent)
	{
		m_pShow->SetSelected(kvComponent->GetInt("show", 1));
		m_pColorMode->ActivateItemByRow(kvComponent->GetInt("colorMode", 0));
		m_pRed->SetValue(kvComponent->GetInt("red", 255));
		m_pGreen->SetValue(kvComponent->GetInt("green", 255));
		m_pBlue->SetValue(kvComponent->GetInt("blue", 255));
		m_pAlpha->SetValue(kvComponent->GetInt("alpha", 255));

		if(m_pShadow->IsEnabled())
			m_pShadow->SetSelected(kvComponent->GetInt("shadow", 0));
		else
			m_pShadow->SetSelected(false);

		if(m_pSize->IsEnabled())
			m_pSize->SetValue(kvComponent->GetInt("size", 5) + 1);
		else
			m_pSize->SetValue(0);

		if(m_pAlignH->IsEnabled())
			m_pAlignH->ActivateItemByRow(kvComponent->GetInt("alignH", 0));
		else
			m_pAlignH->ActivateItemByRow(0);

		if(m_pAlignV->IsEnabled())
			m_pAlignV->ActivateItemByRow(kvComponent->GetInt("alignV", 0));
		else
			m_pAlignV->ActivateItemByRow(0);
		
		if(m_pOffsetX->IsEnabled())
			m_pOffsetX->SetValue(kvComponent->GetInt("offsetX", 0));
		else
			m_pOffsetX->SetValue(0);

		if(m_pOffsetY->IsEnabled())
			m_pOffsetY->SetValue(kvComponent->GetInt("offsetY", 0));
		else
			m_pOffsetY->SetValue(0);

		if(m_pFontTahoma->IsEnabled())
			m_pFontTahoma->SetSelected(kvComponent->GetInt("fontTahoma", 0));
		else
			m_pFontTahoma->SetSelected(false);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the sliders moving
	//-----------------------------------------------------------------------------
	void CFFCustomHudStylePresets::OnUpdateSliders(KeyValues *data)
	{
		if(m_bLoaded && !m_bPresetLoading)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxs changing their selection
	//-----------------------------------------------------------------------------
	void CFFCustomHudStylePresets::OnUpdateCombos(KeyValues *data)
	{
		if(m_bLoaded && !m_bPresetLoading && data->GetPtr("panel") == m_pComponentSelection)
		//if we're changing component selection box (bar, barBorder, barBackground, icon, label, amount)
		{
			const char* m_szName = m_pComponentSelection->GetActiveItemUserData()->GetName();
			if(!(Q_stricmp(m_szName, "Bar") == 0 || Q_stricmp(m_szName, "BarBorder") == 0 || Q_stricmp(m_szName, "BarBackground") == 0))
			{
				if(Q_stricmp(m_szName, "Icon") == 0)
				{
					m_pFontTahoma->SetEnabled(false);
					m_pSize->SetRange(1, QUANTITYBARICONSIZES);
				}
				else
				{
					m_pFontTahoma->SetEnabled(true);
					m_pSize->SetRange(1, QUANTITYBARFONTSIZES);
				}

				m_pShadow->SetEnabled(true);
				m_pSize->SetEnabled(true);
				m_pSizeLabel->SetEnabled(true);
				m_pOffsetX->SetEnabled(true);
				m_pOffsetXLabel->SetEnabled(true);
				m_pOffsetY->SetEnabled(true);
				m_pOffsetYLabel->SetEnabled(true);
				m_pOffsetLabel->SetEnabled(true);
				m_pTextLabel->SetEnabled(true);
				m_pAlignH->SetEnabled(true);
				m_pAlignHLabel->SetEnabled(true);
				m_pAlignV->SetEnabled(true);
				m_pAlignVLabel->SetEnabled(true);
				m_pAlignLabel->SetEnabled(true);
			}
			else
			{
				m_pFontTahoma->SetEnabled(false);
				m_pShadow->SetEnabled(false);
				m_pSize->SetEnabled(false);
				m_pSizeLabel->SetEnabled(false);
				m_pOffsetX->SetEnabled(false);
				m_pOffsetXLabel->SetEnabled(false);
				m_pOffsetY->SetEnabled(false);
				m_pOffsetYLabel->SetEnabled(false);
				m_pOffsetLabel->SetEnabled(false);
				m_pTextLabel->SetEnabled(false);
				m_pAlignH->SetEnabled(false);
				m_pAlignHLabel->SetEnabled(false);
				m_pAlignV->SetEnabled(false);
				m_pAlignVLabel->SetEnabled(false);
				m_pAlignLabel->SetEnabled(false);
			}
			UpdateComponentControls(m_pComponentSelection->GetActiveItemUserData());
		}
		else if (m_bLoaded && !m_bPresetLoading && (data->GetPtr("panel") == m_pColorMode))
		{
			const char* m_szName = m_pColorMode->GetActiveItemUserData()->GetName();
			if(Q_stricmp(m_szName, "Custom") == 0)
			{
				m_pRed->SetEnabled(true);
				m_pGreen->SetEnabled(true);
				m_pBlue->SetEnabled(true);
			}
			else
			{
				m_pRed->SetEnabled(false);
				m_pGreen->SetEnabled(false);
				m_pBlue->SetEnabled(false);
			}
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		}
		else if (m_bLoaded && !m_bPresetLoading && (data->GetPtr("panel") == m_pAlignV))
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		else if (m_bLoaded && !m_bPresetLoading && (data->GetPtr("panel") == m_pAlignH))
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		else if (m_bLoaded && !m_bPresetLoading && (data->GetPtr("panel") == m_pBarOrientation))
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		else
			BaseClass::OnUpdateCombos(data);
	}

	void CFFCustomHudStylePresets::OnUpdateCheckbox(KeyValues *data)
	{
		if(m_bLoaded && !m_bPresetLoading)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}

	void CFFCustomHudStylePresets::SendUpdatedPresetNameToPresetAssignment(const char *pszPresetName)
	{
		g_AP->StylePresetUpdated(pszPresetName);
	}
	void CFFCustomHudStylePresets::SendRenamedPresetNameToPresetAssignment(const char *pszOldPresetName, const char *pszNewPresetName)
	{
		g_AP->StylePresetRenamed(pszOldPresetName, pszNewPresetName);
	}
	void CFFCustomHudStylePresets::SendDeletedPresetNameToPresetAssignment(const char *pszDeletedPresetName)
	{
		g_AP->StylePresetDeleted(pszDeletedPresetName);
	}
	void CFFCustomHudStylePresets::SendNewPresetNameToPresetAssignment(const char *pszPresetName, KeyValues *kvPreset)
	{
		g_AP->StylePresetAdded(pszPresetName, kvPreset);
	}
};