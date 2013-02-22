#include "cbase.h"
#include "ff_customhudoptions_itemstylepresets.h"

#define ITEMSTYLEPRESET_FILE "HudItemStylePresets.vdf"

//this is defined in the quantitybar page too, keep it in sync
#define QUANTITYBARFONTSIZES 15
#define QUANTITYBARICONSIZES 20

#include "ff_customhudoptions.h"

extern CFFCustomHudAssignPresets *g_AP;

namespace vgui
{
	CFFCustomHudItemStylePresets::CFFCustomHudItemStylePresets(Panel *parent, char const *panelName, char const *pszComboBoxName) : BaseClass(parent, panelName, pszComboBoxName, ITEMSTYLEPRESET_FILE)
	{
		m_pColor = new FFColorPicker(this, "Color", this, true);
		m_pColor->SetRedComponentValue(255);
		m_pColor->SetGreenComponentValue(255);
		m_pColor->SetBlueComponentValue(255);
		m_pColor->SetAlphaComponentValue(255);
		
		m_pBarWidth = new CFFInputSlider(this, "BarWidth", "BarWidthInput");
		m_pBarWidth->SetRange(0, 120);
		m_pBarWidth->SetValue(0);
		
		m_pBarHeight = new CFFInputSlider(this, "BarHeight", "BarHeightInput");
		m_pBarHeight->SetRange(0, 120);
		m_pBarHeight->SetValue(0);
		
		m_pBarBorderWidth = new CFFInputSlider(this, "BarBorderWidth", "BarBorderWidthInput");
		m_pBarBorderWidth->SetRange(1, 5);
		m_pBarBorderWidth->SetValue(1);
		
		m_pItemMarginHorizontal = new CFFInputSlider(this, "ItemMarginHorizontal", "ItemMarginHorizontalInput");
		m_pItemMarginHorizontal->SetRange(0, 20);
		m_pItemMarginHorizontal->SetValue(5);
		
		m_pItemMarginVertical = new CFFInputSlider(this, "ItemMarginVertical", "ItemMarginVerticalInput");
		m_pItemMarginVertical->SetRange(0, 20);
		m_pItemMarginVertical->SetValue(5);
		
		m_pItemColumns = new CFFInputSlider(this, "Columns", "ColumnsInput");
		m_pItemColumns->SetRange(1, 12);
		m_pItemColumns->SetValue(1);

		m_pSizeLabel = new Label(this, "SizeLabel", "#GameUI_Size");
		m_pTextLabel = new Label(this, "TextLabel", "#GameUI_Text");
		m_pOffsetXLabel = new Label(this, "OffsetXLabel", "#GameUI_X");
		m_pOffsetYLabel = new Label(this, "OffsetYLabel", "#GameUI_Y");
		m_pOffsetLabel = new Label(this, "PositionOffsetLabel", "#GameUI_PositionOffset");
		m_pAlignHLabel = new Label(this, "AlignHorizontallyLabel", "#GameUI_Horizontal");
		m_pAlignVLabel = new Label(this, "AlignVerticallyLabel", "#GameUI_Vertical");
		m_pAlignLabel = new Label(this, "AlignmentLabel", "#GameUI_Alignment");
		m_pAnchorPositionLabel = new Label(this, "AnchorPositionLabel", "#GameUI_Position");

		m_pOffsetX = new CFFInputSlider(this, "OffsetX", "OffsetXInput");
		m_pOffsetX->SetRange(-120, 120);
		m_pOffsetX->SetValue(0);
		
		m_pOffsetY = new CFFInputSlider(this, "OffsetY", "OffsetYInput");
		m_pOffsetY->SetRange(-120, 120);
		m_pOffsetY->SetValue(0);
		
		m_pSize = new CFFInputSlider(this, "Size", "SizeInput");
		m_pSize->SetRange(1, QUANTITYBARFONTSIZES);
		m_pSize->SetValue(2);
		
		m_pShadow = new CheckButton(this, "Shadow", "#GameUI_DropShadow");
		m_pShow = new CheckButton(this, "Show", "#GameUI_Show");
		m_pFontTahoma = new CheckButton(this, "FontTahoma", "#GameUI_UseTahomaFont");

		m_pAnchorPositionTopLeft = new CheckButton(this, "AnchorPositionTopLeft", "");
		m_pAnchorPositionTopCenter = new CheckButton(this, "AnchorPositionTopCenter", "");
		m_pAnchorPositionTopRight = new CheckButton(this, "AnchorPositionTopRight", "");
		m_pAnchorPositionMiddleLeft = new CheckButton(this, "AnchorPositionMiddleLeft", "");
		m_pAnchorPositionMiddleCenter = new CheckButton(this, "AnchorPositionMiddleCenter", "");
		m_pAnchorPositionMiddleRight = new CheckButton(this, "AnchorPositionMiddleRight", "");
		m_pAnchorPositionBottomLeft = new CheckButton(this, "AnchorPositionBottomLeft", "");
		m_pAnchorPositionBottomCenter = new CheckButton(this, "AnchorPositionBottomCenter", "");
		m_pAnchorPositionBottomRight = new CheckButton(this, "AnchorPositionBottomRight", "");

		KeyValues *kv;

		m_pAlignH = new ComboBox(this, "AlignHorizontallyCombo", 3, false);
		kv = new KeyValues("AH");
		kv->SetInt("Left", FFQuantityItem::ALIGN_LEFT);
		m_pAlignH->AddItem("#GameUI_Left", kv);
		kv->deleteThis();
		kv = new KeyValues("AH");
		kv->SetInt("Center", FFQuantityItem::ALIGN_CENTER);
		m_pAlignH->AddItem("#GameUI_Center", kv);
		kv->deleteThis();
		kv = new KeyValues("AH");
		kv->SetInt("Right", FFQuantityItem::ALIGN_RIGHT);
		m_pAlignH->AddItem("#GameUI_Right", kv);
		kv->deleteThis();
		m_pAlignH->ActivateItemByRow(0);
		
		m_pAlignV = new ComboBox(this, "AlignVerticallyCombo", 3, false);
		kv = new KeyValues("AV");
		kv->SetInt("Top", FFQuantityItem::ALIGN_TOP);
		m_pAlignV->AddItem("#GameUI_Top", kv);
		kv->deleteThis();
		kv = new KeyValues("AV");
		kv->SetInt("Middle", FFQuantityItem::ALIGN_MIDDLE);
		m_pAlignV->AddItem("#GameUI_Middle", kv);
		kv->deleteThis();
		kv = new KeyValues("AV");
		kv->SetInt("Bottom", FFQuantityItem::ALIGN_BOTTOM);
		m_pAlignV->AddItem("#GameUI_Bottom", kv);
		kv->deleteThis();
		m_pAlignV->ActivateItemByRow(0);

		m_pBarOrientation = new ComboBox(this, "BarOrientationCombo", 4, false);
		kv = new KeyValues("BO");
		kv->SetInt("Horizontal", FFQuantityItem::ORIENTATION_HORIZONTAL);
		m_pBarOrientation->AddItem("#GameUI_Horizontal", kv);
		kv->deleteThis();
		kv = new KeyValues("BO");
		kv->SetInt("Vertical", FFQuantityItem::ORIENTATION_VERTICAL);
		m_pBarOrientation->AddItem("#GameUI_Vertical", kv);
		kv->deleteThis();
		kv = new KeyValues("BO");
		kv->SetInt("InvertHorizontal", FFQuantityItem::ORIENTATION_HORIZONTAL_INVERTED);
		m_pBarOrientation->AddItem("#GameUI_InvertHorizontal", kv);
		kv->deleteThis();
		kv = new KeyValues("BO");
		kv->SetInt("InvertVertical", FFQuantityItem::ORIENTATION_VERTICAL_INVERTED);
		m_pBarOrientation->AddItem("#GameUI_InvertVertical", kv);
		kv->deleteThis();
		m_pBarOrientation->ActivateItemByRow(0);

		m_pComponentSelection = new ComboBox(this, "ComponentSelectionCombo", 6, false);
		//we add these in ApplyPresetToControls
		
		LoadControlSettings("resource/ui/FFCustomHudOptionsItemStylePresets.res");

		kv = NULL;
	}

	void CFFCustomHudItemStylePresets::ActivatePresetPage()
	{
		KeyValues *kvAction = new KeyValues("ActivateItemStylePage");
		PostActionSignal ( kvAction );
	}
	
	void CFFCustomHudItemStylePresets::SetControlsEnabled(bool bEnabled)
	{
		m_pItemColumns->SetEnabled( bEnabled );
		m_pBarWidth->SetEnabled( bEnabled );
		m_pBarHeight->SetEnabled( bEnabled );
		m_pBarBorderWidth->SetEnabled( bEnabled );
		m_pItemMarginHorizontal->SetEnabled( bEnabled );
		m_pItemMarginVertical->SetEnabled( bEnabled );
		m_pBarOrientation->SetEnabled( bEnabled );

		m_pComponentSelection->SetEnabled( bEnabled );
		m_pShow->SetEnabled( bEnabled );
		m_pColor->SetEnabled( bEnabled );

		//if we're disabling
		if(!bEnabled)
		//disable all
		{
			m_pOffsetX->SetEnabled( bEnabled );
			m_pOffsetY->SetEnabled( bEnabled );
			m_pSize->SetEnabled( bEnabled );
			m_pShadow->SetEnabled( bEnabled );
			m_pFontTahoma->SetEnabled( bEnabled );
			m_pAlignH->SetEnabled( bEnabled );
			m_pAlignV->SetEnabled( bEnabled );
			m_pAnchorPositionTopLeft->SetEnabled( bEnabled );
			m_pAnchorPositionTopCenter->SetEnabled( bEnabled );
			m_pAnchorPositionTopRight->SetEnabled( bEnabled );
			m_pAnchorPositionMiddleLeft->SetEnabled( bEnabled );
			m_pAnchorPositionMiddleCenter->SetEnabled( bEnabled );
			m_pAnchorPositionMiddleRight->SetEnabled( bEnabled );
			m_pAnchorPositionBottomLeft->SetEnabled( bEnabled );
			m_pAnchorPositionBottomCenter->SetEnabled( bEnabled );
			m_pAnchorPositionBottomRight->SetEnabled( bEnabled );
		}
		//else we're enabling
		else
		//only enable based upon what is selected in the component dropdown
		{
			if(m_pComponentSelection->GetItemCount() > 0)
			{			
				int activeItem = m_pComponentSelection->GetActiveItem();
				m_pComponentSelection->ActivateItem(activeItem);
			}
		}
	}


	//-----------------------------------------------------------------------------
	// Purpose: Tells the assignment class that this preset class is loaded
	//-----------------------------------------------------------------------------	
	void CFFCustomHudItemStylePresets::RegisterSelfForPresetAssignment()
	{
		if(g_AP != NULL)
			g_AP->OnItemStylePresetsClassLoaded();
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: Cleans up preset.. we might add remove alter values in the future
	//-----------------------------------------------------------------------------	
	KeyValues* CFFCustomHudItemStylePresets::RemoveNonEssentialValues(KeyValues *kvPreset)
	{
		KeyValues *kvTemp = new KeyValues(kvPreset->GetName());

		kvTemp->SetInt("itemColumns", kvPreset->GetInt("itemColumns", 2));
		kvTemp->SetInt("barWidth", kvPreset->GetInt("barWidth", 60));
		kvTemp->SetInt("barHeight", kvPreset->GetInt("barHeight", 10));
		kvTemp->SetInt("barBorderWidth", kvPreset->GetInt("barBorderWidth", 1));
		kvTemp->SetInt("itemMarginHorizontal", kvPreset->GetInt("itemMarginHorizontal", 10));
		kvTemp->SetInt("itemMarginVertical", kvPreset->GetInt("itemMarginVertical", 10));
		kvTemp->SetInt("barOrientation", kvPreset->GetInt("barOrientation", 0));

		//see if component data exists
		KeyValues *kvComponent = kvPreset->FindKey("Bar");
		if(kvComponent)	
		//if it does
		{
			//create the component and strip
			KeyValues *kvTempComponent = new KeyValues("Bar");

			//strip as normal	
			kvTempComponent->SetInt("show", kvComponent->GetInt("show", 1));
			kvTempComponent->SetInt("colorMode", kvComponent->GetInt("colorMode", 2));
			kvTempComponent->SetInt("red", kvComponent->GetInt("red", 255));
			kvTempComponent->SetInt("green", kvComponent->GetInt("green", 255));
			kvTempComponent->SetInt("blue", kvComponent->GetInt("blue", 255));
			kvTempComponent->SetInt("alpha", kvComponent->GetInt("alpha", 255));

			kvTemp->AddSubKey(kvTempComponent);
		}
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
			
			//also add component data to the preset
			kvTemp->AddSubKey(kvComponent);
		}
		
		kvComponent = kvPreset->FindKey("BarBorder");
		if(kvComponent)	
		//if it does
		{
			//create the component and strip
			KeyValues *kvTempComponent = new KeyValues("BarBorder");

			//strip as normal	
			kvTempComponent->SetInt("show", kvComponent->GetInt("show", 1));
			kvTempComponent->SetInt("colorMode", kvComponent->GetInt("colorMode", 0));
			kvTempComponent->SetInt("red", kvComponent->GetInt("red", 255));
			kvTempComponent->SetInt("green", kvComponent->GetInt("green", 255));
			kvTempComponent->SetInt("blue", kvComponent->GetInt("blue", 255));
			kvTempComponent->SetInt("alpha", kvComponent->GetInt("alpha", 255));

			kvTemp->AddSubKey(kvTempComponent);
		}
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

			//also add component data to the preset
			kvTemp->AddSubKey(kvComponent);
		}
		
		kvComponent = kvPreset->FindKey("BarBackground");
		if(kvComponent)	
		//if it does
		{
			//create the component and strip
			KeyValues *kvTempComponent = new KeyValues("BarBackground");

			//strip as normal	
			kvTempComponent->SetInt("show", kvComponent->GetInt("show", 1));
			kvTempComponent->SetInt("colorMode", kvComponent->GetInt("colorMode", 2));
			kvTempComponent->SetInt("red", kvComponent->GetInt("red", 255));
			kvTempComponent->SetInt("green", kvComponent->GetInt("green", 255));
			kvTempComponent->SetInt("blue", kvComponent->GetInt("blue", 255));
			kvTempComponent->SetInt("alpha", kvComponent->GetInt("alpha", 96));

			kvTemp->AddSubKey(kvTempComponent);
		}
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
			
			//also add component data to the preset
			kvTemp->AddSubKey(kvComponent);
		}
		kvComponent = kvPreset->FindKey("Icon");
		if(kvComponent)	
		//if it does
		{
			//create the component and strip
			KeyValues *kvTempComponent = new KeyValues("Icon");

			//strip as normal	
			kvTempComponent->SetInt("show", kvComponent->GetInt("show", 1));
			kvTempComponent->SetInt("colorMode", kvComponent->GetInt("colorMode", 0));
			kvTempComponent->SetInt("red", kvComponent->GetInt("red", 255));
			kvTempComponent->SetInt("green", kvComponent->GetInt("green", 255));
			kvTempComponent->SetInt("blue", kvComponent->GetInt("blue", 255));
			kvTempComponent->SetInt("alpha", kvComponent->GetInt("alpha", 255));
			kvTempComponent->SetInt("shadow", kvComponent->GetInt("shadow", 0));
			kvTempComponent->SetInt("size", kvComponent->GetInt("size", 4));
			kvTempComponent->SetInt("anchorPosition", kvComponent->GetInt("anchorPosition", FFQuantityItem::ANCHORPOS_MIDDLELEFT));
			kvTempComponent->SetInt("alignH", kvComponent->GetInt("alignH", FFQuantityItem::ALIGN_RIGHT));
			kvTempComponent->SetInt("alignV", kvComponent->GetInt("alignV", FFQuantityItem::ALIGN_MIDDLE));
			kvTempComponent->SetInt("offsetX", kvComponent->GetInt("offsetX", -2));
			kvTempComponent->SetInt("offsetY", kvComponent->GetInt("offsetY", 0));

			kvTemp->AddSubKey(kvTempComponent);
		}
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
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_MIDDLELEFT);
			kvComponent->SetInt("alignH", FFQuantityItem::ALIGN_RIGHT);
			kvComponent->SetInt("alignV", FFQuantityItem::ALIGN_MIDDLE);
			kvComponent->SetInt("offsetX", -2);
			kvComponent->SetInt("offsetY", 0);
			
			//also add component data to the preset
			kvTemp->AddSubKey(kvComponent);
		}
		
		kvComponent = kvPreset->FindKey("Label");
		if(kvComponent)
		//if it does
		{
			//create the component and strip
			KeyValues *kvTempComponent = new KeyValues("Label");

			//strip as normal	
			kvTempComponent->SetInt("show", kvComponent->GetInt("show", 1));
			kvTempComponent->SetInt("colorMode", kvComponent->GetInt("colorMode", 0));
			kvTempComponent->SetInt("red", kvComponent->GetInt("red", 255));
			kvTempComponent->SetInt("green", kvComponent->GetInt("green", 255));
			kvTempComponent->SetInt("blue", kvComponent->GetInt("blue", 255));
			kvTempComponent->SetInt("alpha", kvComponent->GetInt("alpha", 255));
			kvTempComponent->SetInt("shadow", kvComponent->GetInt("shadow", 0));
			kvTempComponent->SetInt("size", kvComponent->GetInt("size", 4));
			kvTempComponent->SetInt("anchorPosition", kvComponent->GetInt("anchorPosition", FFQuantityItem::ANCHORPOS_TOPLEFT));
			kvTempComponent->SetInt("alignH", kvComponent->GetInt("alignH", FFQuantityItem::ALIGN_LEFT));
			kvTempComponent->SetInt("alignV", kvComponent->GetInt("alignV", FFQuantityItem::ALIGN_BOTTOM));
			kvTempComponent->SetInt("offsetX", kvComponent->GetInt("offsetX", 0));
			kvTempComponent->SetInt("offsetY", kvComponent->GetInt("offsetY", 0));
			kvTempComponent->SetInt("fontTahoma", kvComponent->GetInt("fontTahoma", 0));

			kvTemp->AddSubKey(kvTempComponent);
		}
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
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_TOPLEFT);
			kvComponent->SetInt("alignH", FFQuantityItem::ALIGN_LEFT);
			kvComponent->SetInt("alignV", FFQuantityItem::ALIGN_BOTTOM);
			kvComponent->SetInt("offsetX", 0);
			kvComponent->SetInt("offsetY", 0);
			kvComponent->SetInt("fontTahoma", 0);
			
			//also add component data to the preset
			kvTemp->AddSubKey(kvComponent);
		}
		
		kvComponent = kvPreset->FindKey("Amount");
		if(kvComponent)	
		//if it does
		{
			//create the component and strip
			KeyValues *kvTempComponent = new KeyValues("Amount");

			//strip as normal	
			kvTempComponent->SetInt("show", kvComponent->GetInt("show", 1));
			kvTempComponent->SetInt("colorMode", kvComponent->GetInt("colorMode", 0));
			kvTempComponent->SetInt("red", kvComponent->GetInt("red", 255));
			kvTempComponent->SetInt("green", kvComponent->GetInt("green", 255));
			kvTempComponent->SetInt("blue", kvComponent->GetInt("blue", 255));
			kvTempComponent->SetInt("alpha", kvComponent->GetInt("alpha", 255));
			kvTempComponent->SetInt("shadow", kvComponent->GetInt("shadow", 0));
			kvTempComponent->SetInt("size", kvComponent->GetInt("size", 4));
			kvTempComponent->SetInt("anchorPosition", kvComponent->GetInt("anchorPosition", FFQuantityItem::ANCHORPOS_TOPRIGHT));
			kvTempComponent->SetInt("alignH", kvComponent->GetInt("alignH", FFQuantityItem::ALIGN_RIGHT));
			kvTempComponent->SetInt("alignV", kvComponent->GetInt("alignV", FFQuantityItem::ALIGN_BOTTOM));
			kvTempComponent->SetInt("offsetX", kvComponent->GetInt("offsetX", 0));
			kvTempComponent->SetInt("offsetY", kvComponent->GetInt("offsetY", 0));
			kvTempComponent->SetInt("fontTahoma", kvComponent->GetInt("fontTahoma", 0));

			kvTemp->AddSubKey(kvTempComponent);
		}
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
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_TOPRIGHT);
			kvComponent->SetInt("alignH", FFQuantityItem::ALIGN_RIGHT);
			kvComponent->SetInt("alignV", FFQuantityItem::ALIGN_BOTTOM);
			kvComponent->SetInt("offsetX", 0);
			kvComponent->SetInt("offsetY", 0);
			kvComponent->SetInt("fontTahoma", 0);
			
			//also add component data to the preset
			kvTemp->AddSubKey(kvComponent);	
		}

		return kvTemp;
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: Update the currently selected preset from the controls
	//-----------------------------------------------------------------------------	
	void CFFCustomHudItemStylePresets::UpdatePresetFromControls(KeyValues *kvPreset)
	{
		//update the selected component (bar,barBorder,icon,label) with the current values
		KeyValues *kvComponent = m_pComponentSelection->GetActiveItemUserData();
		
		kvComponent->SetInt("show", m_pShow->IsSelected());
		kvComponent->SetInt("colorMode", m_pColor->GetColorMode());

		int iRed, iGreen, iBlue, iAlpha;
		m_pColor->GetValue( iRed, iGreen, iBlue, iAlpha );

		kvComponent->SetInt("red", iRed );
		kvComponent->SetInt("green", iGreen );
		kvComponent->SetInt("blue", iBlue );
		kvComponent->SetInt("alpha", iAlpha );

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

		if(m_pAnchorPositionTopLeft->IsEnabled() && m_pAnchorPositionTopLeft->IsSelected())
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_TOPLEFT);
		else if(m_pAnchorPositionTopCenter->IsEnabled() && m_pAnchorPositionTopCenter->IsSelected())
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_TOPCENTER);
		else if(m_pAnchorPositionTopRight->IsEnabled() && m_pAnchorPositionTopRight->IsSelected())
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_TOPRIGHT);
		else if(m_pAnchorPositionMiddleLeft->IsEnabled() && m_pAnchorPositionMiddleLeft->IsSelected())
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_MIDDLELEFT);
		else if(m_pAnchorPositionMiddleCenter->IsEnabled() && m_pAnchorPositionMiddleCenter->IsSelected())
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_MIDDLECENTER);
		else if(m_pAnchorPositionMiddleRight->IsEnabled() && m_pAnchorPositionMiddleRight->IsSelected())
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_MIDDLERIGHT);
		else if(m_pAnchorPositionBottomLeft->IsEnabled() && m_pAnchorPositionBottomLeft->IsSelected())
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMLEFT);
		else if(m_pAnchorPositionBottomCenter->IsEnabled() && m_pAnchorPositionBottomCenter->IsSelected())
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMCENTER);
		else if(m_pAnchorPositionBottomRight->IsEnabled() && m_pAnchorPositionBottomRight->IsSelected())
			kvComponent->SetInt("anchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMRIGHT);

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
			kvPreset->SetInt("itemMarginHorizontal", m_pItemMarginHorizontal->GetValue());
			kvPreset->SetInt("itemMarginVertical", m_pItemMarginVertical->GetValue());
			kvPreset->SetInt("barOrientation", m_pBarOrientation->GetActiveItem());
			kvPreset->SetInt("itemColumns", m_pItemColumns->GetValue());
		}

		BaseClass::UpdatePresetFromControls(kvPreset);
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: Apply the selected preset to the contols
	//-----------------------------------------------------------------------------	
	void CFFCustomHudItemStylePresets::ApplyPresetToControls(KeyValues *kvPreset)
	{
		//this disables the preset options from registering these changes as a preset update!
		m_pBarWidth->RemoveActionSignalTarget(this);
		m_pBarHeight->RemoveActionSignalTarget(this);
		m_pBarBorderWidth->RemoveActionSignalTarget(this);
		m_pBarOrientation->RemoveActionSignalTarget(this);
		m_pItemMarginHorizontal->RemoveActionSignalTarget(this);
		m_pItemMarginVertical->RemoveActionSignalTarget(this);
		m_pItemColumns->RemoveActionSignalTarget(this);
		m_pComponentSelection->RemoveActionSignalTarget(this);

		m_pBarWidth->SetValue(kvPreset->GetInt("barWidth", 60));
		m_pBarHeight->SetValue(kvPreset->GetInt("barHeight", 10));
		m_pBarBorderWidth->SetValue(kvPreset->GetInt("barBorderWidth", 1));
		m_pBarOrientation->ActivateItemByRow(kvPreset->GetInt("barOrientation", 0));
		m_pItemMarginHorizontal->SetValue(kvPreset->GetInt("itemMarginHorizontal", 10));
		m_pItemMarginVertical->SetValue(kvPreset->GetInt("itemMarginVertical", 10));
		m_pItemColumns->SetValue(kvPreset->GetInt("itemColumns", 2));

		int iMenuItemToShow = 0;

		if(m_pComponentSelection->GetItemCount() > 0)
			iMenuItemToShow = m_pComponentSelection->GetActiveItem();
		//remove and readd the items so we can detect if there's something wrong with the loading preset
	
		//lets remove the current component items
		m_pComponentSelection->RemoveAll();
		
		// now lets rebuild the dropdown with current data

		//see if component data exists
		KeyValues *kvComponent = kvPreset->FindKey("Bar");

		//if it does exist
		if(kvComponent)	
		{
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_Bar", kvComponent);		
		}
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
		
		//see if component data exists
		kvComponent = kvPreset->FindKey("BarBorder");

		//if it does exist
		if(kvComponent)	
		{
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_BarBorder", kvComponent);
		}
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
		
		//see if component data exists
		kvComponent = kvPreset->FindKey("BarBackground");

		//if it does exist
		if(kvComponent)	
		{
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_BarBackground", kvComponent);
		}
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
		
		//see if component data exists
		kvComponent = kvPreset->FindKey("Icon");

		//if it does exist
		if(kvComponent)	
		{
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_Icon", kvComponent);
		}
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
		
		//see if component data exists
		kvComponent = kvPreset->FindKey("Label");

		//if it does exist
		if(kvComponent)
		{
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_Label", kvComponent);
		}
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
		//see if component exists
		kvComponent = kvPreset->FindKey("Amount");

		//if it does exist
		if(kvComponent)	
		{
			//add it to the drop down
			m_pComponentSelection->AddItem("#GameUI_Amount", kvComponent);
		}
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

		if(m_pComponentSelection->GetActiveItem() == iMenuItemToShow)
		{
			m_pOffsetX->RemoveActionSignalTarget(this);
			m_pOffsetY->RemoveActionSignalTarget(this);
			m_pSize->RemoveActionSignalTarget(this);

			m_pColor->RemoveActionSignalTarget(this);
			
			m_pAlignH->RemoveActionSignalTarget(this);
			m_pAlignV->RemoveActionSignalTarget(this);
			
			m_pShadow->RemoveActionSignalTarget(this);
			m_pShow->RemoveActionSignalTarget(this);
			m_pFontTahoma->RemoveActionSignalTarget(this);
			
			m_pAnchorPositionTopLeft->RemoveActionSignalTarget(this);
			m_pAnchorPositionTopCenter->RemoveActionSignalTarget(this);
			m_pAnchorPositionTopRight->RemoveActionSignalTarget(this);
			m_pAnchorPositionMiddleLeft->RemoveActionSignalTarget(this);
			m_pAnchorPositionMiddleCenter->RemoveActionSignalTarget(this);
			m_pAnchorPositionMiddleRight->RemoveActionSignalTarget(this);
			m_pAnchorPositionBottomLeft->RemoveActionSignalTarget(this);
			m_pAnchorPositionBottomCenter->RemoveActionSignalTarget(this);
			m_pAnchorPositionBottomRight->RemoveActionSignalTarget(this);

			//force an update of component controls because the dropdown won't register a change to do it itself
			UpdateComponentControls(m_pComponentSelection->GetActiveItemUserData());
			
			m_pColor->AddActionSignalTarget(this);

			m_pOffsetX->AddActionSignalTarget(this);
			m_pOffsetY->AddActionSignalTarget(this);
			m_pSize->AddActionSignalTarget(this);
			
			m_pAlignH->AddActionSignalTarget(this);
			m_pAlignV->AddActionSignalTarget(this);
			
			m_pShadow->AddActionSignalTarget(this);
			m_pShow->AddActionSignalTarget(this);
			m_pFontTahoma->AddActionSignalTarget(this);

			m_pAnchorPositionTopLeft->AddActionSignalTarget(this);
			m_pAnchorPositionTopCenter->AddActionSignalTarget(this);
			m_pAnchorPositionTopRight->AddActionSignalTarget(this);
			m_pAnchorPositionMiddleLeft->AddActionSignalTarget(this);
			m_pAnchorPositionMiddleCenter->AddActionSignalTarget(this);
			m_pAnchorPositionMiddleRight->AddActionSignalTarget(this);
			m_pAnchorPositionBottomLeft->AddActionSignalTarget(this);
			m_pAnchorPositionBottomCenter->AddActionSignalTarget(this);
			m_pAnchorPositionBottomRight->AddActionSignalTarget(this);
		}
		//select the item which was previously selected (might be default 0)
		m_pComponentSelection->ActivateItemByRow(iMenuItemToShow);
		
		m_pBarWidth->AddActionSignalTarget(this);
		m_pBarHeight->AddActionSignalTarget(this);
		m_pBarBorderWidth->AddActionSignalTarget(this);
		m_pBarOrientation->AddActionSignalTarget(this);
		m_pItemMarginHorizontal->AddActionSignalTarget(this);
		m_pItemMarginVertical->AddActionSignalTarget(this);
		m_pItemColumns->AddActionSignalTarget(this);
		m_pComponentSelection->AddActionSignalTarget(this);
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: Update the component controls from the selected Component
	//-----------------------------------------------------------------------------	
	void CFFCustomHudItemStylePresets::UpdateComponentControls(KeyValues *kvComponent)
	{
		m_pShow->SetSelected(kvComponent->GetInt("show", 1));
		m_pColor->SetColorMode(kvComponent->GetInt("colorMode", 0));

		m_pColor->SetValue(
			kvComponent->GetInt("red", 255), 
			kvComponent->GetInt("green", 255),
			kvComponent->GetInt("blue", 255),
			kvComponent->GetInt("alpha", 255) );

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

		if(m_pAnchorPositionTopLeft->IsEnabled())
			m_pAnchorPositionTopLeft->SetSelected(kvComponent->GetInt("anchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPLEFT);
		else
			m_pAnchorPositionTopLeft->SetSelected(false);

		if(m_pAnchorPositionTopCenter->IsEnabled())
			m_pAnchorPositionTopCenter->SetSelected(kvComponent->GetInt("anchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPCENTER);
		else
			m_pAnchorPositionTopCenter->SetSelected(false);

		if(m_pAnchorPositionTopRight->IsEnabled())
			m_pAnchorPositionTopRight->SetSelected(kvComponent->GetInt("anchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPRIGHT);
		else
			m_pAnchorPositionTopRight->SetSelected(false);

		if(m_pAnchorPositionMiddleLeft->IsEnabled())
			m_pAnchorPositionMiddleLeft->SetSelected(kvComponent->GetInt("anchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLELEFT);
		else
			m_pAnchorPositionMiddleLeft->SetSelected(false);

		if(m_pAnchorPositionMiddleCenter->IsEnabled())
			m_pAnchorPositionMiddleCenter->SetSelected(kvComponent->GetInt("anchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLECENTER);
		else
			m_pAnchorPositionMiddleCenter->SetSelected(false);

		if(m_pAnchorPositionMiddleRight->IsEnabled())
			m_pAnchorPositionMiddleRight->SetSelected(kvComponent->GetInt("anchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLERIGHT);
		else
			m_pAnchorPositionMiddleRight->SetSelected(false);

		if(m_pAnchorPositionBottomLeft->IsEnabled())
			m_pAnchorPositionBottomLeft->SetSelected(kvComponent->GetInt("anchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMLEFT);
		else
			m_pAnchorPositionBottomLeft->SetSelected(false);

		if(m_pAnchorPositionBottomCenter->IsEnabled())
			m_pAnchorPositionBottomCenter->SetSelected(kvComponent->GetInt("anchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMCENTER);
		else
			m_pAnchorPositionBottomCenter->SetSelected(false);

		if(m_pAnchorPositionBottomRight->IsEnabled())
			m_pAnchorPositionBottomRight->SetSelected(kvComponent->GetInt("anchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMRIGHT);
		else
			m_pAnchorPositionBottomRight->SetSelected(false);
	}
	
	void CFFCustomHudItemStylePresets::OnUpdateComponentSelection()
	{
		m_pOffsetX->RemoveActionSignalTarget(this);
		m_pOffsetY->RemoveActionSignalTarget(this);
		m_pSize->RemoveActionSignalTarget(this);

		m_pColor->RemoveActionSignalTarget(this);
		
		m_pAlignH->RemoveActionSignalTarget(this);
		m_pAlignV->RemoveActionSignalTarget(this);
		
		m_pShadow->RemoveActionSignalTarget(this);
		m_pShow->RemoveActionSignalTarget(this);
		m_pFontTahoma->RemoveActionSignalTarget(this);
		
		m_pAnchorPositionTopLeft->RemoveActionSignalTarget(this);
		m_pAnchorPositionTopCenter->RemoveActionSignalTarget(this);
		m_pAnchorPositionTopRight->RemoveActionSignalTarget(this);
		m_pAnchorPositionMiddleLeft->RemoveActionSignalTarget(this);
		m_pAnchorPositionMiddleCenter->RemoveActionSignalTarget(this);
		m_pAnchorPositionMiddleRight->RemoveActionSignalTarget(this);
		m_pAnchorPositionBottomLeft->RemoveActionSignalTarget(this);
		m_pAnchorPositionBottomCenter->RemoveActionSignalTarget(this);
		m_pAnchorPositionBottomRight->RemoveActionSignalTarget(this);

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
			m_pAnchorPositionLabel->SetEnabled(true);
			m_pAnchorPositionTopLeft->SetEnabled(true);
			m_pAnchorPositionTopCenter->SetEnabled(true);
			m_pAnchorPositionTopRight->SetEnabled(true);
			m_pAnchorPositionMiddleLeft->SetEnabled(true);
			m_pAnchorPositionMiddleCenter->SetEnabled(true);
			m_pAnchorPositionMiddleRight->SetEnabled(true);
			m_pAnchorPositionBottomLeft->SetEnabled(true);
			m_pAnchorPositionBottomCenter->SetEnabled(true);
			m_pAnchorPositionBottomRight->SetEnabled(true);
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
			m_pAnchorPositionLabel->SetEnabled(false);
			m_pAnchorPositionTopLeft->SetEnabled(false);
			m_pAnchorPositionTopCenter->SetEnabled(false);
			m_pAnchorPositionTopRight->SetEnabled(false);
			m_pAnchorPositionMiddleLeft->SetEnabled(false);
			m_pAnchorPositionMiddleCenter->SetEnabled(false);
			m_pAnchorPositionMiddleRight->SetEnabled(false);
			m_pAnchorPositionBottomLeft->SetEnabled(false);
			m_pAnchorPositionBottomCenter->SetEnabled(false);
			m_pAnchorPositionBottomRight->SetEnabled(false);
		}

		UpdateComponentControls(m_pComponentSelection->GetActiveItemUserData());

		m_pColor->AddActionSignalTarget(this);

		m_pOffsetX->AddActionSignalTarget(this);
		m_pOffsetY->AddActionSignalTarget(this);
		m_pSize->AddActionSignalTarget(this);
		
		m_pAlignH->AddActionSignalTarget(this);
		m_pAlignV->AddActionSignalTarget(this);
		
		m_pShadow->AddActionSignalTarget(this);
		m_pShow->AddActionSignalTarget(this);
		m_pFontTahoma->AddActionSignalTarget(this);

		m_pAnchorPositionTopLeft->AddActionSignalTarget(this);
		m_pAnchorPositionTopCenter->AddActionSignalTarget(this);
		m_pAnchorPositionTopRight->AddActionSignalTarget(this);
		m_pAnchorPositionMiddleLeft->AddActionSignalTarget(this);
		m_pAnchorPositionMiddleCenter->AddActionSignalTarget(this);
		m_pAnchorPositionMiddleRight->AddActionSignalTarget(this);
		m_pAnchorPositionBottomLeft->AddActionSignalTarget(this);
		m_pAnchorPositionBottomCenter->AddActionSignalTarget(this);
		m_pAnchorPositionBottomRight->AddActionSignalTarget(this);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxs changing their selection
	//-----------------------------------------------------------------------------
	void CFFCustomHudItemStylePresets::OnUpdateCombos(KeyValues *data)
	{
		if(m_bLoaded && data->GetPtr("panel") == m_pComponentSelection)
		//if we're changing component selection box (bar, barBorder, barBackground, icon, label, amount)
		{
			OnUpdateComponentSelection();
		}
		else if (m_bLoaded && (data->GetPtr("panel") == m_pAlignV))
		{
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		}
		else if (m_bLoaded && (data->GetPtr("panel") == m_pAlignH))
		{
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		}
		else if (m_bLoaded && (data->GetPtr("panel") == m_pBarOrientation))
		{
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		}
		else
		{
			BaseClass::OnUpdateCombos(data);
		}
	}
	
	void CFFCustomHudItemStylePresets::RefreshAnchorPositionCheckButtons(CheckButton *pCheckButton)
	{
		if(pCheckButton == m_pAnchorPositionTopLeft 
			|| pCheckButton == m_pAnchorPositionTopCenter
			|| pCheckButton == m_pAnchorPositionTopRight
			|| pCheckButton == m_pAnchorPositionMiddleLeft
			|| pCheckButton == m_pAnchorPositionMiddleCenter
			|| pCheckButton == m_pAnchorPositionMiddleRight
			|| pCheckButton == m_pAnchorPositionBottomLeft
			|| pCheckButton == m_pAnchorPositionBottomCenter
			|| pCheckButton == m_pAnchorPositionBottomRight)
		{	
			m_pAnchorPositionTopLeft->RemoveActionSignalTarget(this);
			m_pAnchorPositionTopCenter->RemoveActionSignalTarget(this);
			m_pAnchorPositionTopRight->RemoveActionSignalTarget(this);
			m_pAnchorPositionMiddleLeft->RemoveActionSignalTarget(this);
			m_pAnchorPositionMiddleCenter->RemoveActionSignalTarget(this);
			m_pAnchorPositionMiddleRight->RemoveActionSignalTarget(this);
			m_pAnchorPositionBottomLeft->RemoveActionSignalTarget(this);
			m_pAnchorPositionBottomCenter->RemoveActionSignalTarget(this);
			m_pAnchorPositionBottomRight->RemoveActionSignalTarget(this);

			if(!pCheckButton->IsSelected())
			{
				pCheckButton->SetSelected(true);
			}

			m_pAnchorPositionTopLeft->SetSelected(pCheckButton == m_pAnchorPositionTopLeft);
			m_pAnchorPositionTopCenter->SetSelected(pCheckButton == m_pAnchorPositionTopCenter);
			m_pAnchorPositionTopRight->SetSelected(pCheckButton == m_pAnchorPositionTopRight);
			m_pAnchorPositionMiddleLeft->SetSelected(pCheckButton == m_pAnchorPositionMiddleLeft);
			m_pAnchorPositionMiddleCenter->SetSelected(pCheckButton == m_pAnchorPositionMiddleCenter);
			m_pAnchorPositionMiddleRight->SetSelected(pCheckButton == m_pAnchorPositionMiddleRight);
			m_pAnchorPositionBottomLeft->SetSelected(pCheckButton == m_pAnchorPositionBottomLeft);
			m_pAnchorPositionBottomCenter->SetSelected(pCheckButton == m_pAnchorPositionBottomCenter);
			m_pAnchorPositionBottomRight->SetSelected(pCheckButton == m_pAnchorPositionBottomRight);

			m_pAnchorPositionTopLeft->AddActionSignalTarget(this);
			m_pAnchorPositionTopCenter->AddActionSignalTarget(this);
			m_pAnchorPositionTopRight->AddActionSignalTarget(this);
			m_pAnchorPositionMiddleLeft->AddActionSignalTarget(this);
			m_pAnchorPositionMiddleCenter->AddActionSignalTarget(this);
			m_pAnchorPositionMiddleRight->AddActionSignalTarget(this);
			m_pAnchorPositionBottomLeft->AddActionSignalTarget(this);
			m_pAnchorPositionBottomCenter->AddActionSignalTarget(this);
			m_pAnchorPositionBottomRight->AddActionSignalTarget(this);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the checkboxes changing their state
	//-----------------------------------------------------------------------------
	void CFFCustomHudItemStylePresets::OnUpdateCheckButton(KeyValues *data)
	{
		RefreshAnchorPositionCheckButtons(reinterpret_cast<CheckButton*>(data->GetPtr("panel")));

		if(m_bLoaded)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: Catch the sliders moving
	//-----------------------------------------------------------------------------
	void CFFCustomHudItemStylePresets::OnUpdateSliders(KeyValues *data)
	{
		if(m_bLoaded)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}

	void CFFCustomHudItemStylePresets::OnColorChanged(KeyValues *data)
	{
		if(m_bLoaded)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}
	void CFFCustomHudItemStylePresets::OnColorModeChanged(KeyValues *data)
	{
		if(m_bLoaded)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}

	void CFFCustomHudItemStylePresets::SendUpdatedPresetPreviewToPresetAssignment(const char *pszPresetName, KeyValues *kvPresetPreview)
	{
		g_AP->ItemStylePresetPreviewUpdated(pszPresetName, kvPresetPreview);
	}
	void CFFCustomHudItemStylePresets::SendUpdatedPresetToPresetAssignment(const char *pszPresetName)
	{
		g_AP->ItemStylePresetUpdated(pszPresetName);
	}
	void CFFCustomHudItemStylePresets::SendRenamedPresetToPresetAssignment(const char *pszOldPresetName, const char *pszNewPresetName)
	{
		g_AP->ItemStylePresetRenamed(pszOldPresetName, pszNewPresetName);
	}
	void CFFCustomHudItemStylePresets::SendDeletedPresetToPresetAssignment(const char *pszDeletedPresetName)
	{
		g_AP->ItemStylePresetDeleted(pszDeletedPresetName);
	}
	void CFFCustomHudItemStylePresets::SendNewPresetToPresetAssignment(const char *pszPresetName, KeyValues *kvPreset)
	{
		g_AP->ItemStylePresetAdded(pszPresetName, kvPreset);
	}
};