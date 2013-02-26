#include "cbase.h"
#include "ff_customhudoptions_panelstylepresets.h"

#define ARRANGEMENTPRESET_FILE "HudPanelStylePresets.vdf"

//this is defined in the quantitypanel page too, keep it in sync
#define QUANTITYPANELFONTSIZES 15
#define QUANTITYBARICONSIZES 20

#include "ff_customhudoptions.h"

extern CFFCustomHudAssignPresets *g_AP;

namespace vgui
{
	CFFCustomHudPanelStylePresets::CFFCustomHudPanelStylePresets(Panel *parent, char const *panelName, char const *pszComboBoxName) : BaseClass(parent, panelName, pszComboBoxName, ARRANGEMENTPRESET_FILE)
	{
		m_pHeaderText = new PropertyPage( this, "HeaderText");
		m_pHeaderIcon = new PropertyPage( this, "HeaderIcon");
		m_pText = new PropertyPage( this, "Text");
		m_pPanel = new PropertyPage( this, "Panel");

		m_pPropertyPages = new PropertySheet(this, "PanelStylePages", true);
		m_pPropertyPages->AddPage(m_pPanel, "#GameUI_Panel");
		m_pPropertyPages->AddPage(m_pHeaderText, "#GameUI_HeaderText");
		m_pPropertyPages->AddPage(m_pHeaderIcon, "#GameUI_HeaderIcon");
		m_pPropertyPages->AddPage(m_pText, "#GameUI_Text");
		m_pPropertyPages->SetActivePage(m_pPanel);

		m_pPropertyPages->SetDragEnabled(false);

		m_pPanel->RemoveActionSignalTarget( m_pPropertyPages );
		m_pHeaderText->RemoveActionSignalTarget( m_pPropertyPages );
		m_pHeaderIcon->RemoveActionSignalTarget( m_pPropertyPages );
		m_pText->RemoveActionSignalTarget( m_pPropertyPages );

		m_pPanelColor = new FFColorPicker(m_pPanel, "PanelColor", this);
		m_pPanelColor->SetRedComponentValue(255);
		m_pPanelColor->SetGreenComponentValue(255);
		m_pPanelColor->SetBlueComponentValue(255);
		m_pPanelColor->SetAlphaComponentValue(255);

		m_pPanelMargin = new CFFInputSlider(m_pPanel, "PanelMargin", "PanelMarginInput");
		m_pPanelMargin->SetRange(0, 20);
		m_pPanelMargin->SetValue(0);
		m_pPanelMargin->RemoveActionSignalTarget(m_pPanel);
		m_pPanelMargin->AddActionSignalTarget(this);

		m_pHeaderTextColor = new FFColorPicker(m_pHeaderText, "HeaderTextColor", this);
		m_pHeaderTextColor->SetRedComponentValue(255);
		m_pHeaderTextColor->SetGreenComponentValue(255);
		m_pHeaderTextColor->SetBlueComponentValue(255);
		m_pHeaderTextColor->SetAlphaComponentValue(255);

		m_pHeaderIconColor = new FFColorPicker(m_pHeaderIcon, "HeaderIconColor", this);
		m_pHeaderIconColor->SetRedComponentValue(255);
		m_pHeaderIconColor->SetGreenComponentValue(255);
		m_pHeaderIconColor->SetBlueComponentValue(255);
		m_pHeaderIconColor->SetAlphaComponentValue(255);

		m_pTextColor = new FFColorPicker(m_pText, "TextColor", this);
		m_pTextColor->SetRedComponentValue(255);
		m_pTextColor->SetGreenComponentValue(255);
		m_pTextColor->SetBlueComponentValue(255);
		m_pTextColor->SetAlphaComponentValue(255);
		
		m_pHeaderTextSize = new CFFInputSlider(m_pHeaderText, "HeaderTextSize", "HeaderTextSizeInput");
		m_pHeaderTextSize->SetRange(1, QUANTITYPANELFONTSIZES);
		m_pHeaderTextSize->SetValue(2);
		m_pHeaderTextSize->RemoveActionSignalTarget(m_pHeaderText);
		m_pHeaderTextSize->AddActionSignalTarget(this);

		m_pHeaderTextX = new CFFInputSlider(m_pHeaderText, "HeaderTextX", "HeaderTextXInput");
		m_pHeaderTextX->SetRange(-40, 40);
		m_pHeaderTextX->SetValue(0);
		m_pHeaderTextX->RemoveActionSignalTarget(m_pHeaderText);
		m_pHeaderTextX->AddActionSignalTarget(this);
		
		m_pHeaderTextY = new CFFInputSlider(m_pHeaderText, "HeaderTextY", "HeaderTextYInput");
		m_pHeaderTextY->SetRange(-40, 40);
		m_pHeaderTextY->SetValue(0);
		m_pHeaderTextX->RemoveActionSignalTarget(m_pHeaderText);
		m_pHeaderTextX->AddActionSignalTarget(this);
		
		m_pHeaderTextAnchorPositionTopLeft = new CheckButton(m_pHeaderText, "HeaderTextAnchorPositionTopLeft", "");
		m_pHeaderTextAnchorPositionTopCenter = new CheckButton(m_pHeaderText, "HeaderTextAnchorPositionTopCenter", "");
		m_pHeaderTextAnchorPositionTopRight = new CheckButton(m_pHeaderText, "HeaderTextAnchorPositionTopRight", "");
		m_pHeaderTextAnchorPositionMiddleLeft = new CheckButton(m_pHeaderText, "HeaderTextAnchorPositionMiddleLeft", "");
		m_pHeaderTextAnchorPositionMiddleCenter = new CheckButton(m_pHeaderText, "HeaderTextAnchorPositionMiddleCenter", "");
		m_pHeaderTextAnchorPositionMiddleRight = new CheckButton(m_pHeaderText, "HeaderTextAnchorPositionMiddleRight", "");
		m_pHeaderTextAnchorPositionBottomLeft = new CheckButton(m_pHeaderText, "HeaderTextAnchorPositionBottomLeft", "");
		m_pHeaderTextAnchorPositionBottomCenter = new CheckButton(m_pHeaderText, "HeaderTextAnchorPositionBottomCenter", "");
		m_pHeaderTextAnchorPositionBottomRight = new CheckButton(m_pHeaderText, "HeaderTextAnchorPositionBottomRight", "");

		KeyValues *kv;

		m_pHeaderTextAlignHoriz = new ComboBox(m_pHeaderText, "HeaderTextAlignHorizontallyCombo", 3, false);
		kv = new KeyValues("AH");
		kv->SetInt("Left", FFQuantityItem::ALIGN_LEFT);
		m_pHeaderTextAlignHoriz->AddItem("#GameUI_Left", kv);
		kv->deleteThis();
		kv = new KeyValues("AH");
		kv->SetInt("Center", FFQuantityItem::ALIGN_CENTER);
		m_pHeaderTextAlignHoriz->AddItem("#GameUI_Center", kv);
		kv->deleteThis();
		kv = new KeyValues("AH");
		kv->SetInt("Right", FFQuantityItem::ALIGN_RIGHT);
		m_pHeaderTextAlignHoriz->AddItem("#GameUI_Right", kv);
		kv->deleteThis();
		m_pHeaderTextAlignHoriz->ActivateItemByRow(0);
		
		m_pHeaderTextAlignVert = new ComboBox(m_pHeaderText, "HeaderTextAlignVerticallyCombo", 3, false);
		kv = new KeyValues("AV");
		kv->SetInt("Top", FFQuantityItem::ALIGN_TOP);
		m_pHeaderTextAlignVert->AddItem("#GameUI_Top", kv);
		kv->deleteThis();
		kv = new KeyValues("AV");
		kv->SetInt("Middle", FFQuantityItem::ALIGN_MIDDLE);
		m_pHeaderTextAlignVert->AddItem("#GameUI_Middle", kv);
		kv->deleteThis();
		kv = new KeyValues("AV");
		kv->SetInt("Bottom", FFQuantityItem::ALIGN_BOTTOM);
		m_pHeaderTextAlignVert->AddItem("#GameUI_Bottom", kv);
		kv->deleteThis();
		m_pHeaderTextAlignVert->ActivateItemByRow(0);

		m_pHeaderIconSize = new CFFInputSlider(m_pHeaderIcon, "HeaderIconSize", "HeaderIconSizeInput");
		m_pHeaderIconSize->SetRange(1, QUANTITYBARICONSIZES);
		m_pHeaderIconSize->SetValue(2);
		m_pHeaderIconSize->RemoveActionSignalTarget(m_pHeaderIcon);
		m_pHeaderIconSize->AddActionSignalTarget(this);

		m_pHeaderIconX = new CFFInputSlider(m_pHeaderIcon, "HeaderIconX", "HeaderIconXInput");
		m_pHeaderIconX->SetRange(-40, 40);
		m_pHeaderIconX->SetValue(0);
		m_pHeaderIconX->RemoveActionSignalTarget(m_pHeaderIcon);
		m_pHeaderIconX->AddActionSignalTarget(this);
		
		m_pHeaderIconY = new CFFInputSlider(m_pHeaderIcon, "HeaderIconY", "HeaderIconYInput");
		m_pHeaderIconY->SetRange(-40, 40);
		m_pHeaderIconY->SetValue(0);
		m_pHeaderIconY->RemoveActionSignalTarget(m_pHeaderIcon);
		m_pHeaderIconY->AddActionSignalTarget(this);
		
		m_pHeaderIconAnchorPositionTopLeft = new CheckButton(m_pHeaderIcon, "HeaderIconAnchorPositionTopLeft", "");
		m_pHeaderIconAnchorPositionTopCenter = new CheckButton(m_pHeaderIcon, "HeaderIconAnchorPositionTopCenter", "");
		m_pHeaderIconAnchorPositionTopRight = new CheckButton(m_pHeaderIcon, "HeaderIconAnchorPositionTopRight", "");
		m_pHeaderIconAnchorPositionMiddleLeft = new CheckButton(m_pHeaderIcon, "HeaderIconAnchorPositionMiddleLeft", "");
		m_pHeaderIconAnchorPositionMiddleCenter = new CheckButton(m_pHeaderIcon, "HeaderIconAnchorPositionMiddleCenter", "");
		m_pHeaderIconAnchorPositionMiddleRight = new CheckButton(m_pHeaderIcon, "HeaderIconAnchorPositionMiddleRight", "");
		m_pHeaderIconAnchorPositionBottomLeft = new CheckButton(m_pHeaderIcon, "HeaderIconAnchorPositionBottomLeft", "");
		m_pHeaderIconAnchorPositionBottomCenter = new CheckButton(m_pHeaderIcon, "HeaderIconAnchorPositionBottomCenter", "");
		m_pHeaderIconAnchorPositionBottomRight = new CheckButton(m_pHeaderIcon, "HeaderIconAnchorPositionBottomRight", "");

		m_pHeaderIconAlignHoriz = new ComboBox(m_pHeaderIcon, "HeaderIconAlignHorizontallyCombo", 3, false);
		kv = new KeyValues("AH");
		kv->SetInt("Left", FFQuantityItem::ALIGN_LEFT);
		m_pHeaderIconAlignHoriz->AddItem("#GameUI_Left", kv);
		kv->deleteThis();
		kv = new KeyValues("AH");
		kv->SetInt("Center", FFQuantityItem::ALIGN_CENTER);
		m_pHeaderIconAlignHoriz->AddItem("#GameUI_Center", kv);
		kv->deleteThis();
		kv = new KeyValues("AH");
		kv->SetInt("Right", FFQuantityItem::ALIGN_RIGHT);
		m_pHeaderIconAlignHoriz->AddItem("#GameUI_Right", kv);
		kv->deleteThis();
		m_pHeaderIconAlignHoriz->ActivateItemByRow(0);
		
		m_pHeaderIconAlignVert = new ComboBox(m_pHeaderIcon, "HeaderIconAlignVerticallyCombo", 3, false);
		kv = new KeyValues("AV");
		kv->SetInt("Top", FFQuantityItem::ALIGN_TOP);
		m_pHeaderIconAlignVert->AddItem("#GameUI_Top", kv);
		kv->deleteThis();
		kv = new KeyValues("AV");
		kv->SetInt("Middle", FFQuantityItem::ALIGN_MIDDLE);
		m_pHeaderIconAlignVert->AddItem("#GameUI_Middle", kv);
		kv->deleteThis();
		kv = new KeyValues("AV");
		kv->SetInt("Bottom", FFQuantityItem::ALIGN_BOTTOM);
		m_pHeaderIconAlignVert->AddItem("#GameUI_Bottom", kv);
		kv->deleteThis();
		m_pHeaderIconAlignVert->ActivateItemByRow(0);

		m_pTextSize = new CFFInputSlider(m_pText, "TextSize", "TextSizeInput");
		m_pTextSize->SetRange(1, QUANTITYPANELFONTSIZES);
		m_pTextSize->SetValue(2);
		m_pTextSize->RemoveActionSignalTarget(m_pText);
		m_pTextSize->AddActionSignalTarget(this);

		m_pTextX = new CFFInputSlider(m_pText, "TextX", "TextXInput");
		m_pTextX->SetRange(-40, 40);
		m_pTextX->SetValue(0);
		m_pTextX->RemoveActionSignalTarget(m_pText);
		m_pTextX->AddActionSignalTarget(this);
		
		m_pTextY = new CFFInputSlider(m_pText, "TextY", "TextYInput");
		m_pTextY->SetRange(-40, 40);
		m_pTextY->SetValue(0);
		m_pTextY->RemoveActionSignalTarget(m_pText);
		m_pTextY->AddActionSignalTarget(this);

		m_pTextAnchorPositionTopLeft = new CheckButton(m_pText, "TextAnchorPositionTopLeft", "");
		m_pTextAnchorPositionTopCenter = new CheckButton(m_pText, "TextAnchorPositionTopCenter", "");
		m_pTextAnchorPositionTopRight = new CheckButton(m_pText, "TextAnchorPositionTopRight", "");
		m_pTextAnchorPositionMiddleLeft = new CheckButton(m_pText, "TextAnchorPositionMiddleLeft", "");
		m_pTextAnchorPositionMiddleCenter = new CheckButton(m_pText, "TextAnchorPositionMiddleCenter", "");
		m_pTextAnchorPositionMiddleRight = new CheckButton(m_pText, "TextAnchorPositionMiddleRight", "");
		m_pTextAnchorPositionBottomLeft = new CheckButton(m_pText, "TextAnchorPositionBottomLeft", "");
		m_pTextAnchorPositionBottomCenter = new CheckButton(m_pText, "TextAnchorPositionBottomCenter", "");
		m_pTextAnchorPositionBottomRight = new CheckButton(m_pText, "TextAnchorPositionBottomRight", "");

		m_pTextAlignHoriz = new ComboBox(m_pText, "TextAlignHorizontallyCombo", 3, false);
		kv = new KeyValues("AH");
		kv->SetInt("Left", FFQuantityItem::ALIGN_LEFT);
		m_pTextAlignHoriz->AddItem("#GameUI_Left", kv);
		kv->deleteThis();
		kv = new KeyValues("AH");
		kv->SetInt("Center", FFQuantityItem::ALIGN_CENTER);
		m_pTextAlignHoriz->AddItem("#GameUI_Center", kv);
		kv->deleteThis();
		kv = new KeyValues("AH");
		kv->SetInt("Right", FFQuantityItem::ALIGN_RIGHT);
		m_pTextAlignHoriz->AddItem("#GameUI_Right", kv);
		kv->deleteThis();
		m_pTextAlignHoriz->ActivateItemByRow(0);
		
		m_pTextAlignVert = new ComboBox(m_pText, "TextAlignVerticallyCombo", 3, false);
		kv = new KeyValues("AV");
		kv->SetInt("Top", FFQuantityItem::ALIGN_TOP);
		m_pTextAlignVert->AddItem("#GameUI_Top", kv);
		kv->deleteThis();
		kv = new KeyValues("AV");
		kv->SetInt("Middle", FFQuantityItem::ALIGN_MIDDLE);
		m_pTextAlignVert->AddItem("#GameUI_Middle", kv);
		kv->deleteThis();
		kv = new KeyValues("AV");
		kv->SetInt("Bottom", FFQuantityItem::ALIGN_BOTTOM);
		m_pTextAlignVert->AddItem("#GameUI_Bottom", kv);
		kv->deleteThis();
		m_pTextAlignVert->ActivateItemByRow(0);
		
		m_pShowHeaderIcon = new CheckButton(m_pHeaderIcon, "ShowHeaderIcon", "#GameUI_Show");
		m_pShowHeaderIcon->RemoveActionSignalTarget(m_pHeaderIcon);
		m_pShowHeaderIcon->AddActionSignalTarget(this);
		m_pShowHeaderText = new CheckButton(m_pHeaderText, "ShowHeaderText", "#GameUI_Show");
		m_pShowHeaderText->RemoveActionSignalTarget(m_pHeaderText);
		m_pShowHeaderText->AddActionSignalTarget(this);
		m_pShowText = new CheckButton(m_pText, "ShowText", "#GameUI_Show");
		m_pShowText->RemoveActionSignalTarget(m_pText);
		m_pShowText->AddActionSignalTarget(this);
		m_pHeaderIconShadow = new CheckButton(m_pHeaderIcon, "HeaderIconShadow", "#GameUI_DropShadow");
		m_pHeaderIconShadow->RemoveActionSignalTarget(m_pHeaderIcon);
		m_pHeaderIconShadow->AddActionSignalTarget(this);
		m_pHeaderTextShadow = new CheckButton(m_pHeaderText, "HeaderTextShadow", "#GameUI_DropShadow");
		m_pHeaderTextShadow->RemoveActionSignalTarget(m_pHeaderText);
		m_pHeaderTextShadow->AddActionSignalTarget(this);
		m_pTextShadow = new CheckButton(m_pText, "TextShadow", "#GameUI_DropShadow");
		m_pTextShadow->RemoveActionSignalTarget(m_pText);
		m_pTextShadow->AddActionSignalTarget(this);
		m_pShowPanel = new CheckButton(m_pPanel, "ShowPanel", "#GameUI_Show");
		m_pShowPanel->RemoveActionSignalTarget(m_pPanel);
		m_pShowPanel->AddActionSignalTarget(this);

		m_pHeaderText->LoadControlSettings("resource/ui/FFCustomHudOptionsPanelStyleHeaderText.res");
		m_pHeaderIcon->LoadControlSettings("resource/ui/FFCustomHudOptionsPanelStyleHeaderIcon.res");
		m_pText->LoadControlSettings("resource/ui/FFCustomHudOptionsPanelStyleText.res");
		m_pPanel->LoadControlSettings("resource/ui/FFCustomHudOptionsPanelStylePanel.res");
		LoadControlSettings("resource/ui/FFCustomHudOptionsPanelStylePresets.res");
	}
	
	void CFFCustomHudPanelStylePresets::ActivatePresetPage()
	{
		KeyValues *kvAction = new KeyValues("ActivatePanelStylePage");
		PostActionSignal ( kvAction );
	}
	
	void CFFCustomHudPanelStylePresets::SetControlsEnabled(bool bEnabled)
	{
		m_pHeaderTextAnchorPositionTopLeft->SetEnabled(bEnabled);
		m_pHeaderTextAnchorPositionTopCenter->SetEnabled(bEnabled);
		m_pHeaderTextAnchorPositionTopRight->SetEnabled(bEnabled);
		m_pHeaderTextAnchorPositionMiddleLeft->SetEnabled(bEnabled);
		m_pHeaderTextAnchorPositionMiddleCenter->SetEnabled(bEnabled);
		m_pHeaderTextAnchorPositionMiddleRight->SetEnabled(bEnabled);
		m_pHeaderTextAnchorPositionBottomLeft->SetEnabled(bEnabled);
		m_pHeaderTextAnchorPositionBottomCenter->SetEnabled(bEnabled);
		m_pHeaderTextAnchorPositionBottomRight->SetEnabled(bEnabled);
		
		m_pHeaderIconAnchorPositionTopLeft->SetEnabled(bEnabled);
		m_pHeaderIconAnchorPositionTopCenter->SetEnabled(bEnabled);
		m_pHeaderIconAnchorPositionTopRight->SetEnabled(bEnabled);
		m_pHeaderIconAnchorPositionMiddleLeft->SetEnabled(bEnabled);
		m_pHeaderIconAnchorPositionMiddleCenter->SetEnabled(bEnabled);
		m_pHeaderIconAnchorPositionMiddleRight->SetEnabled(bEnabled);
		m_pHeaderIconAnchorPositionBottomLeft->SetEnabled(bEnabled);
		m_pHeaderIconAnchorPositionBottomCenter->SetEnabled(bEnabled);
		m_pHeaderIconAnchorPositionBottomRight->SetEnabled(bEnabled);
		
		m_pTextAnchorPositionTopLeft->SetEnabled(bEnabled);
		m_pTextAnchorPositionTopCenter->SetEnabled(bEnabled);
		m_pTextAnchorPositionTopRight->SetEnabled(bEnabled);
		m_pTextAnchorPositionMiddleLeft->SetEnabled(bEnabled);
		m_pTextAnchorPositionMiddleCenter->SetEnabled(bEnabled);
		m_pTextAnchorPositionMiddleRight->SetEnabled(bEnabled);
		m_pTextAnchorPositionBottomLeft->SetEnabled(bEnabled);
		m_pTextAnchorPositionBottomCenter->SetEnabled(bEnabled);
		m_pTextAnchorPositionBottomRight->SetEnabled(bEnabled);

		m_pHeaderTextAlignHoriz->SetEnabled(bEnabled);
		m_pHeaderTextAlignVert->SetEnabled(bEnabled);
		m_pHeaderIconAlignHoriz->SetEnabled(bEnabled);
		m_pHeaderIconAlignVert->SetEnabled(bEnabled);
		m_pTextAlignHoriz->SetEnabled(bEnabled);
		m_pTextAlignVert->SetEnabled(bEnabled);

		m_pHeaderTextX->SetEnabled( bEnabled );
		m_pHeaderTextY->SetEnabled( bEnabled );
		m_pHeaderIconX->SetEnabled( bEnabled );
		m_pHeaderIconY->SetEnabled( bEnabled );
		m_pTextX->SetEnabled( bEnabled );
		m_pTextY->SetEnabled( bEnabled );

		m_pHeaderTextSize->SetEnabled( bEnabled );
		m_pHeaderIconSize->SetEnabled( bEnabled );
		m_pTextSize->SetEnabled( bEnabled );

		m_pShowHeaderIcon->SetEnabled( bEnabled );
		m_pShowHeaderText->SetEnabled( bEnabled );
		m_pShowText->SetEnabled( bEnabled );

		m_pHeaderIconShadow->SetEnabled( bEnabled );
		m_pHeaderTextShadow->SetEnabled( bEnabled );
		m_pTextShadow->SetEnabled( bEnabled );

		m_pShowPanel->SetEnabled( bEnabled );

		m_pPanelMargin->SetEnabled( bEnabled );

		m_pPanelColor->SetEnabled( bEnabled );
		m_pHeaderTextColor->SetEnabled( bEnabled );
		m_pHeaderIconColor->SetEnabled( bEnabled );
		m_pTextColor->SetEnabled( bEnabled );
	}

	//-----------------------------------------------------------------------------
	// Purpose: Tells the assignment class that this preset class is loaded
	//-----------------------------------------------------------------------------	
	void CFFCustomHudPanelStylePresets::RegisterSelfForPresetAssignment()
	{
		if(g_AP != NULL)
			g_AP->OnPanelStylePresetsClassLoaded();
	}
	
	//-----------------------------------------------------------------------------
	// Purpose: Cleans up a preset.. we might add remove alter values in the future
	//			Also used for new preset settings
	//-----------------------------------------------------------------------------	
	KeyValues* CFFCustomHudPanelStylePresets::RemoveNonEssentialValues(KeyValues *kvPreset)
	{
		KeyValues *kvTemp = new KeyValues(kvPreset->GetName());

		kvTemp->SetInt("showPanel", kvPreset->GetInt("showPanel", 1));
		kvTemp->SetInt("panelMargin", kvPreset->GetInt("panelMargin", 5));
		kvTemp->SetInt("panelColorMode", kvPreset->GetInt("panelColorMode", FFQuantityPanel::COLOR_MODE_TEAMCOLORED));
		kvTemp->SetInt("panelRed", kvPreset->GetInt("panelRed", 255));
		kvTemp->SetInt("panelGreen", kvPreset->GetInt("panelGreen", 255));
		kvTemp->SetInt("panelBlue", kvPreset->GetInt("panelBlue", 255));
		kvTemp->SetInt("panelAlpha", kvPreset->GetInt("panelAlpha", 255));

		kvTemp->SetInt("showHeaderText", kvPreset->GetInt("showHeaderText", 1));
		kvTemp->SetInt("headerTextSize", kvPreset->GetInt("headerTextSize", 3));
		kvTemp->SetInt("headerTextShadow", kvPreset->GetInt("headerTextShadow", 1));
		kvTemp->SetInt("headerTextAnchorPosition", kvPreset->GetInt("headerTextAnchorPosition", FFQuantityPanel::ANCHORPOS_TOPLEFT));
		kvTemp->SetInt("headerTextAlignHoriz", kvPreset->GetInt("headerTextAlignHoriz", FFQuantityPanel::ALIGN_LEFT));
		kvTemp->SetInt("headerTextAlignVert", kvPreset->GetInt("headerTextAlignVert", FFQuantityPanel::ALIGN_BOTTOM));
		kvTemp->SetInt("headerTextX", kvPreset->GetInt("headerTextX", 11));
		kvTemp->SetInt("headerTextY", kvPreset->GetInt("headerTextY", -4));
		kvTemp->SetInt("headerTextColorMode", kvPreset->GetInt("headerTextColorMode", FFQuantityPanel::COLOR_MODE_CUSTOM));
		kvTemp->SetInt("headerTextRed", kvPreset->GetInt("headerTextRed", 255));
		kvTemp->SetInt("headerTextGreen", kvPreset->GetInt("headerTextGreen", 255));
		kvTemp->SetInt("headerTextBlue", kvPreset->GetInt("headerTextBlue", 255));
		kvTemp->SetInt("headerTextAlpha", kvPreset->GetInt("headerTextAlpha", 255));

		kvTemp->SetInt("showHeaderIcon", kvPreset->GetInt("showHeaderIcon", 1));
		kvTemp->SetInt("headerIconSize", kvPreset->GetInt("headerIconSize", 3));
		kvTemp->SetInt("headerIconShadow", kvPreset->GetInt("headerIconShadow", 1));
		kvTemp->SetInt("headerIconAnchorPosition", kvPreset->GetInt("headerIconAnchorPosition", FFQuantityPanel::ANCHORPOS_TOPLEFT));
		kvTemp->SetInt("headerIconAlignHoriz", kvPreset->GetInt("headerIconAlignHoriz", FFQuantityPanel::ALIGN_LEFT));
		kvTemp->SetInt("headerIconAlignVert", kvPreset->GetInt("headerIconAlignVert", FFQuantityPanel::ALIGN_BOTTOM));
		kvTemp->SetInt("headerIconX", kvPreset->GetInt("headerIconX", -5));
		kvTemp->SetInt("headerIconY", kvPreset->GetInt("headerIconY", -2));
		kvTemp->SetInt("headerIconColorMode", kvPreset->GetInt("headerIconColorMode", FFQuantityPanel::COLOR_MODE_CUSTOM));
		kvTemp->SetInt("headerIconRed", kvPreset->GetInt("headerIconRed", 255));
		kvTemp->SetInt("headerIconGreen", kvPreset->GetInt("headerIconGreen", 255));
		kvTemp->SetInt("headerIconBlue", kvPreset->GetInt("headerIconBlue", 255));
		kvTemp->SetInt("headerIconAlpha", kvPreset->GetInt("headerIconAlpha", 255));

		kvTemp->SetInt("showText", kvPreset->GetInt("showText", 1));
		kvTemp->SetInt("textSize", kvPreset->GetInt("textSize", 3));
		kvTemp->SetInt("textShadow", kvPreset->GetInt("textShadow", 1));
		kvTemp->SetInt("textAnchorPosition", kvPreset->GetInt("textAnchorPosition", FFQuantityPanel::ANCHORPOS_TOPLEFT));
		kvTemp->SetInt("textAlignHoriz", kvPreset->GetInt("textAlignHoriz", FFQuantityPanel::ALIGN_LEFT));
		kvTemp->SetInt("textAlignVert", kvPreset->GetInt("textAlignVert", FFQuantityPanel::ALIGN_TOP));
		kvTemp->SetInt("textX", kvPreset->GetInt("textX", 5));
		kvTemp->SetInt("textY", kvPreset->GetInt("textY", 5));
		kvTemp->SetInt("textColorMode", kvPreset->GetInt("textColorMode", FFQuantityPanel::COLOR_MODE_CUSTOM));
		kvTemp->SetInt("textRed", kvPreset->GetInt("textRed", 255));
		kvTemp->SetInt("textGreen", kvPreset->GetInt("textGreen", 255));
		kvTemp->SetInt("textBlue", kvPreset->GetInt("textBlue", 255));
		kvTemp->SetInt("textAlpha", kvPreset->GetInt("textAlpha", 255));

		return kvTemp;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the currently selected preset from the controls
	//-----------------------------------------------------------------------------	
	void CFFCustomHudPanelStylePresets::UpdatePresetFromControls(KeyValues *kvPreset)
	{
		if(m_pHeaderTextAnchorPositionTopLeft->IsSelected())
			kvPreset->SetInt("headerTextAnchorPosition", FFQuantityItem::ANCHORPOS_TOPLEFT);
		else if(m_pHeaderTextAnchorPositionTopCenter->IsSelected())
			kvPreset->SetInt("headerTextAnchorPosition", FFQuantityItem::ANCHORPOS_TOPCENTER);
		else if(m_pHeaderTextAnchorPositionTopRight->IsSelected())
			kvPreset->SetInt("headerTextAnchorPosition", FFQuantityItem::ANCHORPOS_TOPRIGHT);
		else if(m_pHeaderTextAnchorPositionMiddleLeft->IsSelected())
			kvPreset->SetInt("headerTextAnchorPosition", FFQuantityItem::ANCHORPOS_MIDDLELEFT);
		else if(m_pHeaderTextAnchorPositionMiddleCenter->IsSelected())
			kvPreset->SetInt("headerTextAnchorPosition", FFQuantityItem::ANCHORPOS_MIDDLECENTER);
		else if(m_pHeaderTextAnchorPositionMiddleRight->IsSelected())
			kvPreset->SetInt("headerTextAnchorPosition", FFQuantityItem::ANCHORPOS_MIDDLERIGHT);
		else if(m_pHeaderTextAnchorPositionBottomLeft->IsSelected())
			kvPreset->SetInt("headerTextAnchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMLEFT);
		else if(m_pHeaderTextAnchorPositionBottomCenter->IsSelected())
			kvPreset->SetInt("headerTextAnchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMCENTER);
		else if(m_pHeaderTextAnchorPositionBottomRight->IsSelected())
			kvPreset->SetInt("headerTextAnchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMRIGHT);
		
		if(m_pHeaderIconAnchorPositionTopLeft->IsSelected())
			kvPreset->SetInt("headerIconAnchorPosition", FFQuantityItem::ANCHORPOS_TOPLEFT);
		else if(m_pHeaderIconAnchorPositionTopCenter->IsSelected())
			kvPreset->SetInt("headerIconAnchorPosition", FFQuantityItem::ANCHORPOS_TOPCENTER);
		else if(m_pHeaderIconAnchorPositionTopRight->IsSelected())
			kvPreset->SetInt("headerIconAnchorPosition", FFQuantityItem::ANCHORPOS_TOPRIGHT);
		else if(m_pHeaderIconAnchorPositionMiddleLeft->IsSelected())
			kvPreset->SetInt("headerIconAnchorPosition", FFQuantityItem::ANCHORPOS_MIDDLELEFT);
		else if(m_pHeaderIconAnchorPositionMiddleCenter->IsSelected())
			kvPreset->SetInt("headerIconAnchorPosition", FFQuantityItem::ANCHORPOS_MIDDLECENTER);
		else if(m_pHeaderIconAnchorPositionMiddleRight->IsSelected())
			kvPreset->SetInt("headerIconAnchorPosition", FFQuantityItem::ANCHORPOS_MIDDLERIGHT);
		else if(m_pHeaderIconAnchorPositionBottomLeft->IsSelected())
			kvPreset->SetInt("headerIconAnchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMLEFT);
		else if(m_pHeaderIconAnchorPositionBottomCenter->IsSelected())
			kvPreset->SetInt("headerIconAnchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMCENTER);
		else if(m_pHeaderIconAnchorPositionBottomRight->IsSelected())
			kvPreset->SetInt("headerIconAnchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMRIGHT);
		
		if(m_pTextAnchorPositionTopLeft->IsSelected())
			kvPreset->SetInt("textAnchorPosition", FFQuantityItem::ANCHORPOS_TOPLEFT);
		else if(m_pTextAnchorPositionTopCenter->IsSelected())
			kvPreset->SetInt("textAnchorPosition", FFQuantityItem::ANCHORPOS_TOPCENTER);
		else if(m_pTextAnchorPositionTopRight->IsSelected())
			kvPreset->SetInt("textAnchorPosition", FFQuantityItem::ANCHORPOS_TOPRIGHT);
		else if(m_pTextAnchorPositionMiddleLeft->IsSelected())
			kvPreset->SetInt("textAnchorPosition", FFQuantityItem::ANCHORPOS_MIDDLELEFT);
		else if(m_pTextAnchorPositionMiddleCenter->IsSelected())
			kvPreset->SetInt("textAnchorPosition", FFQuantityItem::ANCHORPOS_MIDDLECENTER);
		else if(m_pTextAnchorPositionMiddleRight->IsSelected())
			kvPreset->SetInt("textAnchorPosition", FFQuantityItem::ANCHORPOS_MIDDLERIGHT);
		else if(m_pTextAnchorPositionBottomLeft->IsSelected())
			kvPreset->SetInt("textAnchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMLEFT);
		else if(m_pTextAnchorPositionBottomCenter->IsSelected())
			kvPreset->SetInt("textAnchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMCENTER);
		else if(m_pTextAnchorPositionBottomRight->IsSelected())
			kvPreset->SetInt("textAnchorPosition", FFQuantityItem::ANCHORPOS_BOTTOMRIGHT);

		kvPreset->SetInt("headerTextAlignHoriz", m_pHeaderTextAlignHoriz->GetActiveItem());
		kvPreset->SetInt("headerTextAlignVert", m_pHeaderTextAlignVert->GetActiveItem());
		kvPreset->SetInt("headerIconAlignHoriz", m_pHeaderIconAlignHoriz->GetActiveItem());
		kvPreset->SetInt("headerIconAlignVert", m_pHeaderIconAlignVert->GetActiveItem());
		kvPreset->SetInt("textAlignHoriz", m_pTextAlignHoriz->GetActiveItem());
		kvPreset->SetInt("textAlignVert", m_pTextAlignVert->GetActiveItem());

		kvPreset->SetInt("panelMargin", m_pPanelMargin->GetValue());

		kvPreset->SetInt("headerTextX", m_pHeaderTextX->GetValue());
		kvPreset->SetInt("headerTextY", m_pHeaderTextY->GetValue());
		kvPreset->SetInt("headerIconX", m_pHeaderIconX->GetValue());
		kvPreset->SetInt("headerIconY", m_pHeaderIconY->GetValue());
		kvPreset->SetInt("textX", m_pTextX->GetValue());
		kvPreset->SetInt("textY", m_pTextY->GetValue());
		
		kvPreset->SetInt("headerTextSize", m_pHeaderTextSize->GetValue() - 1);
		kvPreset->SetInt("headerIconSize", m_pHeaderIconSize->GetValue() - 1);
		kvPreset->SetInt("textSize", m_pTextSize->GetValue() - 1);

		kvPreset->SetInt("showHeaderText", m_pShowHeaderText->IsSelected());
		kvPreset->SetInt("showHeaderIcon", m_pShowHeaderIcon->IsSelected());
		kvPreset->SetInt("showText", m_pShowText->IsSelected());

		kvPreset->SetInt("headerTextShadow", m_pHeaderTextShadow->IsSelected());
		kvPreset->SetInt("headerIconShadow", m_pHeaderIconShadow->IsSelected());
		kvPreset->SetInt("textShadow", m_pTextShadow->IsSelected());

		kvPreset->SetInt("headerTextColorMode", m_pHeaderTextColor->GetColorMode());
		kvPreset->SetInt("headerIconColorMode", m_pHeaderIconColor->GetColorMode());
		kvPreset->SetInt("textColorMode", m_pTextColor->GetColorMode());
		kvPreset->SetInt("panelColorMode", m_pPanelColor->GetColorMode());

		kvPreset->SetInt("showPanel", m_pShowPanel->IsSelected());

		int iRed, iGreen, iBlue, iAlpha;

		m_pPanelColor->GetValue( iRed, iGreen, iBlue, iAlpha );
		kvPreset->SetInt("panelRed", iRed );
		kvPreset->SetInt("panelGreen", iGreen );
		kvPreset->SetInt("panelBlue", iBlue );
		kvPreset->SetInt("panelAlpha", iAlpha );

		m_pHeaderTextColor->GetValue( iRed, iGreen, iBlue, iAlpha );
		kvPreset->SetInt("headerTextRed", iRed );
		kvPreset->SetInt("headerTextGreen", iGreen );
		kvPreset->SetInt("headerTextBlue", iBlue );
		kvPreset->SetInt("headerTextAlpha", iAlpha );

		m_pHeaderIconColor->GetValue( iRed, iGreen, iBlue, iAlpha );
		kvPreset->SetInt("headerIconRed", iRed );
		kvPreset->SetInt("headerIconGreen", iGreen );
		kvPreset->SetInt("headerIconBlue", iBlue );
		kvPreset->SetInt("headerIconAlpha", iAlpha );

		m_pTextColor->GetValue( iRed, iGreen, iBlue, iAlpha );
		kvPreset->SetInt("textRed", iRed );
		kvPreset->SetInt("textGreen", iGreen );
		kvPreset->SetInt("textBlue", iBlue );
		kvPreset->SetInt("textAlpha", iAlpha );
		
		BaseClass::UpdatePresetFromControls(kvPreset);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Apply the selected preset to the contols
	//-----------------------------------------------------------------------------	
	void CFFCustomHudPanelStylePresets::ApplyPresetToControls(KeyValues *kvPreset)
	{
		m_pHeaderTextAnchorPositionTopLeft->RemoveActionSignalTarget(this);
		m_pHeaderTextAnchorPositionTopCenter->RemoveActionSignalTarget(this);
		m_pHeaderTextAnchorPositionTopRight->RemoveActionSignalTarget(this);
		m_pHeaderTextAnchorPositionMiddleLeft->RemoveActionSignalTarget(this);
		m_pHeaderTextAnchorPositionMiddleCenter->RemoveActionSignalTarget(this);
		m_pHeaderTextAnchorPositionMiddleRight->RemoveActionSignalTarget(this);
		m_pHeaderTextAnchorPositionBottomLeft->RemoveActionSignalTarget(this);
		m_pHeaderTextAnchorPositionBottomCenter->RemoveActionSignalTarget(this);
		m_pHeaderTextAnchorPositionBottomRight->RemoveActionSignalTarget(this);
		
		m_pHeaderIconAnchorPositionTopLeft->RemoveActionSignalTarget(this);
		m_pHeaderIconAnchorPositionTopCenter->RemoveActionSignalTarget(this);
		m_pHeaderIconAnchorPositionTopRight->RemoveActionSignalTarget(this);
		m_pHeaderIconAnchorPositionMiddleLeft->RemoveActionSignalTarget(this);
		m_pHeaderIconAnchorPositionMiddleCenter->RemoveActionSignalTarget(this);
		m_pHeaderIconAnchorPositionMiddleRight->RemoveActionSignalTarget(this);
		m_pHeaderIconAnchorPositionBottomLeft->RemoveActionSignalTarget(this);
		m_pHeaderIconAnchorPositionBottomCenter->RemoveActionSignalTarget(this);
		m_pHeaderIconAnchorPositionBottomRight->RemoveActionSignalTarget(this);
		
		m_pTextAnchorPositionTopLeft->RemoveActionSignalTarget(this);
		m_pTextAnchorPositionTopCenter->RemoveActionSignalTarget(this);
		m_pTextAnchorPositionTopRight->RemoveActionSignalTarget(this);
		m_pTextAnchorPositionMiddleLeft->RemoveActionSignalTarget(this);
		m_pTextAnchorPositionMiddleCenter->RemoveActionSignalTarget(this);
		m_pTextAnchorPositionMiddleRight->RemoveActionSignalTarget(this);
		m_pTextAnchorPositionBottomLeft->RemoveActionSignalTarget(this);
		m_pTextAnchorPositionBottomCenter->RemoveActionSignalTarget(this);
		m_pTextAnchorPositionBottomRight->RemoveActionSignalTarget(this);

		m_pHeaderTextAlignHoriz->RemoveActionSignalTarget(this);
		m_pHeaderTextAlignVert->RemoveActionSignalTarget(this);
		m_pHeaderIconAlignHoriz->RemoveActionSignalTarget(this);
		m_pHeaderIconAlignVert->RemoveActionSignalTarget(this);
		m_pTextAlignHoriz->RemoveActionSignalTarget(this);
		m_pTextAlignVert->RemoveActionSignalTarget(this);

		m_pHeaderTextX->RemoveActionSignalTarget(this);
		m_pHeaderTextY->RemoveActionSignalTarget(this);
		m_pHeaderIconX->RemoveActionSignalTarget(this);
		m_pHeaderIconY->RemoveActionSignalTarget(this);
		m_pTextX->RemoveActionSignalTarget(this);
		m_pTextY->RemoveActionSignalTarget(this);

		m_pPanelMargin->RemoveActionSignalTarget(this);

		m_pShowPanel->RemoveActionSignalTarget(this);
		m_pShowHeaderText->RemoveActionSignalTarget(this);
		m_pShowHeaderIcon->RemoveActionSignalTarget(this);
		m_pShowText->RemoveActionSignalTarget(this);

		m_pHeaderTextSize->RemoveActionSignalTarget(this);
		m_pHeaderIconSize->RemoveActionSignalTarget(this);
		m_pTextSize->RemoveActionSignalTarget(this);

		m_pHeaderTextShadow->RemoveActionSignalTarget(this);
		m_pHeaderIconShadow->RemoveActionSignalTarget(this);
		m_pTextShadow->RemoveActionSignalTarget(this);

		m_pPanelColor->RemoveActionSignalTarget(this);
		m_pHeaderTextColor->RemoveActionSignalTarget(this);
		m_pHeaderIconColor->RemoveActionSignalTarget(this);
		m_pTextColor->RemoveActionSignalTarget(this);

		//this disables the preset options from registering these changes as a preset update!

		m_pHeaderTextAnchorPositionTopLeft->SetSelected(kvPreset->GetInt("headerTextAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPLEFT);
		m_pHeaderTextAnchorPositionTopCenter->SetSelected(kvPreset->GetInt("headerTextAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPCENTER);
		m_pHeaderTextAnchorPositionTopRight->SetSelected(kvPreset->GetInt("headerTextAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPRIGHT);
		m_pHeaderTextAnchorPositionMiddleLeft->SetSelected(kvPreset->GetInt("headerTextAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLELEFT);
		m_pHeaderTextAnchorPositionMiddleCenter->SetSelected(kvPreset->GetInt("headerTextAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLECENTER);
		m_pHeaderTextAnchorPositionMiddleRight->SetSelected(kvPreset->GetInt("headerTextAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLERIGHT);
		m_pHeaderTextAnchorPositionBottomLeft->SetSelected(kvPreset->GetInt("headerTextAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMLEFT);
		m_pHeaderTextAnchorPositionBottomCenter->SetSelected(kvPreset->GetInt("headerTextAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMCENTER);
		m_pHeaderTextAnchorPositionBottomRight->SetSelected(kvPreset->GetInt("headerTextAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMRIGHT);

		m_pHeaderIconAnchorPositionTopLeft->SetSelected(kvPreset->GetInt("headerIconAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPLEFT);
		m_pHeaderIconAnchorPositionTopCenter->SetSelected(kvPreset->GetInt("headerIconAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPCENTER);
		m_pHeaderIconAnchorPositionTopRight->SetSelected(kvPreset->GetInt("headerIconAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPRIGHT);
		m_pHeaderIconAnchorPositionMiddleLeft->SetSelected(kvPreset->GetInt("headerIconAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLELEFT);
		m_pHeaderIconAnchorPositionMiddleCenter->SetSelected(kvPreset->GetInt("headerIconAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLECENTER);
		m_pHeaderIconAnchorPositionMiddleRight->SetSelected(kvPreset->GetInt("headerIconAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLERIGHT);
		m_pHeaderIconAnchorPositionBottomLeft->SetSelected(kvPreset->GetInt("headerIconAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMLEFT);
		m_pHeaderIconAnchorPositionBottomCenter->SetSelected(kvPreset->GetInt("headerIconAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMCENTER);
		m_pHeaderIconAnchorPositionBottomRight->SetSelected(kvPreset->GetInt("headerIconAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMRIGHT);

		m_pTextAnchorPositionTopLeft->SetSelected(kvPreset->GetInt("textAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPLEFT);
		m_pTextAnchorPositionTopCenter->SetSelected(kvPreset->GetInt("textAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPCENTER);
		m_pTextAnchorPositionTopRight->SetSelected(kvPreset->GetInt("textAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_TOPRIGHT);
		m_pTextAnchorPositionMiddleLeft->SetSelected(kvPreset->GetInt("textAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLELEFT);
		m_pTextAnchorPositionMiddleCenter->SetSelected(kvPreset->GetInt("textAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLECENTER);
		m_pTextAnchorPositionMiddleRight->SetSelected(kvPreset->GetInt("textAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_MIDDLERIGHT);
		m_pTextAnchorPositionBottomLeft->SetSelected(kvPreset->GetInt("textAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMLEFT);
		m_pTextAnchorPositionBottomCenter->SetSelected(kvPreset->GetInt("textAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMCENTER);
		m_pTextAnchorPositionBottomRight->SetSelected(kvPreset->GetInt("textAnchorPosition", -1) == FFQuantityItem::ANCHORPOS_BOTTOMRIGHT);

		m_pHeaderTextAlignHoriz->ActivateItemByRow(kvPreset->GetInt("headerTextAlignHoriz", -1));
		m_pHeaderTextAlignVert->ActivateItemByRow(kvPreset->GetInt("headerTextAlignVert", -1));
		m_pHeaderIconAlignHoriz->ActivateItemByRow(kvPreset->GetInt("headerIconAlignHoriz", -1));
		m_pHeaderIconAlignVert->ActivateItemByRow(kvPreset->GetInt("headerIconAlignVert", -1));
		m_pTextAlignHoriz->ActivateItemByRow(kvPreset->GetInt("textAlignHoriz", -1));
		m_pTextAlignVert->ActivateItemByRow(kvPreset->GetInt("textAlignVert", -1));

		m_pHeaderTextX->SetValue(kvPreset->GetInt("headerTextX", 20));
		m_pHeaderTextY->SetValue(kvPreset->GetInt("headerTextY", 7));
		m_pHeaderIconX->SetValue(kvPreset->GetInt("headerIconX", 3));
		m_pHeaderIconY->SetValue(kvPreset->GetInt("headerIconY", 3));
		m_pTextX->SetValue(kvPreset->GetInt("textX", 25));
		m_pTextY->SetValue(kvPreset->GetInt("textY", 20));

		m_pPanelMargin->SetValue(kvPreset->GetInt("panelMargin", 5));

		m_pShowPanel->SetSelected(kvPreset->GetInt("showPanel", 1));
		m_pShowHeaderText->SetSelected(kvPreset->GetInt("showHeaderText", 1));
		m_pShowHeaderIcon->SetSelected(kvPreset->GetInt("showHeaderIcon", 1));
		m_pShowText->SetSelected(kvPreset->GetInt("showText", 1));

		m_pHeaderTextSize->SetValue(kvPreset->GetInt("headerTextSize", 3) + 1);
		m_pHeaderIconSize->SetValue(kvPreset->GetInt("headerIconSize", 3) + 1);
		m_pTextSize->SetValue(kvPreset->GetInt("textSize", 3) + 1);

		m_pHeaderTextShadow->SetSelected(kvPreset->GetInt("headerTextShadow", 0));
		m_pHeaderIconShadow->SetSelected(kvPreset->GetInt("headerIconShadow", 0));
		m_pTextShadow->SetSelected(kvPreset->GetInt("textShadow", 0));

		m_pHeaderTextColor->SetColorMode(kvPreset->GetInt("headerTextColorMode", 0));
		m_pHeaderIconColor->SetColorMode(kvPreset->GetInt("headerIconColorMode", 0));
		m_pTextColor->SetColorMode(kvPreset->GetInt("textColorMode", 0));
		m_pPanelColor->SetColorMode(kvPreset->GetInt("panelColorMode", 0));
		
		int iRed, iGreen, iBlue, iAlpha;

		iRed = kvPreset->GetInt("panelRed", 255);
		iGreen = kvPreset->GetInt("panelGreen", 255);
		iBlue = kvPreset->GetInt("panelBlue", 255);
		iAlpha = kvPreset->GetInt("panelAlpha", 255);
		m_pPanelColor->SetValue( iRed, iGreen, iBlue, iAlpha );
		
		iRed = kvPreset->GetInt("headerTextRed", 255);
		iGreen = kvPreset->GetInt("headerTextGreen", 255);
		iBlue = kvPreset->GetInt("headerTextBlue", 255);
		iAlpha = kvPreset->GetInt("headerTextAlpha", 255);
		m_pHeaderTextColor->SetValue( iRed, iGreen, iBlue, iAlpha );
		
		iRed = kvPreset->GetInt("headerIconRed", 255);
		iGreen = kvPreset->GetInt("headerIconGreen", 255);
		iBlue = kvPreset->GetInt("headerIconBlue", 255);
		iAlpha = kvPreset->GetInt("headerIconAlpha", 255);
		m_pHeaderIconColor->SetValue( iRed, iGreen, iBlue, iAlpha );
		
		iRed = kvPreset->GetInt("textRed", 255);
		iGreen = kvPreset->GetInt("textGreen", 255);
		iBlue = kvPreset->GetInt("textBlue", 255);
		iAlpha = kvPreset->GetInt("textAlpha", 255);
		m_pTextColor->SetValue( iRed, iGreen, iBlue, iAlpha );
		
		//this enables the preset options to register follwoing changes as a preset update!
		m_pHeaderTextX->AddActionSignalTarget(this);
		m_pHeaderTextY->AddActionSignalTarget(this);
		m_pHeaderIconX->AddActionSignalTarget(this);
		m_pHeaderIconY->AddActionSignalTarget(this);
		m_pTextX->AddActionSignalTarget(this);
		m_pTextY->AddActionSignalTarget(this);

		m_pPanelMargin->AddActionSignalTarget(this);

		m_pShowPanel->AddActionSignalTarget(this);
		m_pShowHeaderText->AddActionSignalTarget(this);
		m_pShowHeaderIcon->AddActionSignalTarget(this);
		m_pShowText->AddActionSignalTarget(this);

		m_pHeaderTextSize->AddActionSignalTarget(this);
		m_pHeaderIconSize->AddActionSignalTarget(this);
		m_pTextSize->AddActionSignalTarget(this);

		m_pHeaderTextShadow->AddActionSignalTarget(this);
		m_pHeaderIconShadow->AddActionSignalTarget(this);
		m_pTextShadow->AddActionSignalTarget(this);

		m_pPanelColor->AddActionSignalTarget(this);
		m_pHeaderTextColor->AddActionSignalTarget(this);
		m_pHeaderIconColor->AddActionSignalTarget(this);
		m_pTextColor->AddActionSignalTarget(this);

		m_pHeaderTextAlignHoriz->AddActionSignalTarget(this);
		m_pHeaderTextAlignVert->AddActionSignalTarget(this);
		m_pHeaderIconAlignHoriz->AddActionSignalTarget(this);
		m_pHeaderIconAlignVert->AddActionSignalTarget(this);
		m_pTextAlignHoriz->AddActionSignalTarget(this);
		m_pTextAlignVert->AddActionSignalTarget(this);
	
		m_pHeaderTextAnchorPositionTopLeft->AddActionSignalTarget(this);
		m_pHeaderTextAnchorPositionTopCenter->AddActionSignalTarget(this);
		m_pHeaderTextAnchorPositionTopRight->AddActionSignalTarget(this);
		m_pHeaderTextAnchorPositionMiddleLeft->AddActionSignalTarget(this);
		m_pHeaderTextAnchorPositionMiddleCenter->AddActionSignalTarget(this);
		m_pHeaderTextAnchorPositionMiddleRight->AddActionSignalTarget(this);
		m_pHeaderTextAnchorPositionBottomLeft->AddActionSignalTarget(this);
		m_pHeaderTextAnchorPositionBottomCenter->AddActionSignalTarget(this);
		m_pHeaderTextAnchorPositionBottomRight->AddActionSignalTarget(this);
		
		m_pHeaderIconAnchorPositionTopLeft->AddActionSignalTarget(this);
		m_pHeaderIconAnchorPositionTopCenter->AddActionSignalTarget(this);
		m_pHeaderIconAnchorPositionTopRight->AddActionSignalTarget(this);
		m_pHeaderIconAnchorPositionMiddleLeft->AddActionSignalTarget(this);
		m_pHeaderIconAnchorPositionMiddleCenter->AddActionSignalTarget(this);
		m_pHeaderIconAnchorPositionMiddleRight->AddActionSignalTarget(this);
		m_pHeaderIconAnchorPositionBottomLeft->AddActionSignalTarget(this);
		m_pHeaderIconAnchorPositionBottomCenter->AddActionSignalTarget(this);
		m_pHeaderIconAnchorPositionBottomRight->AddActionSignalTarget(this);
		
		m_pTextAnchorPositionTopLeft->AddActionSignalTarget(this);
		m_pTextAnchorPositionTopCenter->AddActionSignalTarget(this);
		m_pTextAnchorPositionTopRight->AddActionSignalTarget(this);
		m_pTextAnchorPositionMiddleLeft->AddActionSignalTarget(this);
		m_pTextAnchorPositionMiddleCenter->AddActionSignalTarget(this);
		m_pTextAnchorPositionMiddleRight->AddActionSignalTarget(this);
		m_pTextAnchorPositionBottomLeft->AddActionSignalTarget(this);
		m_pTextAnchorPositionBottomCenter->AddActionSignalTarget(this);
		m_pTextAnchorPositionBottomRight->AddActionSignalTarget(this);
	}
	
	void CFFCustomHudPanelStylePresets::OnColorChanged(KeyValues *data)
	{
		if(m_bLoaded)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}

	void CFFCustomHudPanelStylePresets::OnColorModeChanged(KeyValues *data)
	{
		if(m_bLoaded)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the component controls from the selected Component
	//-----------------------------------------------------------------------------	
	void CFFCustomHudPanelStylePresets::OnUpdateCombos(KeyValues *data)
	{
		if (m_bLoaded && 
			(data->GetPtr("panel") == m_pHeaderTextAlignHoriz ||
			data->GetPtr("panel") == m_pHeaderTextAlignVert ||
			data->GetPtr("panel") == m_pHeaderIconAlignHoriz ||
			data->GetPtr("panel") == m_pHeaderIconAlignVert ||
			data->GetPtr("panel") == m_pTextAlignHoriz ||
			data->GetPtr("panel") == m_pTextAlignVert))
		{
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		}
		else
		{
			BaseClass::OnUpdateCombos(data);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the sliders moving
	//-----------------------------------------------------------------------------
	void CFFCustomHudPanelStylePresets::OnUpdateSliders(KeyValues *data)
	{
		if(m_bLoaded)
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		
	} 
	//-----------------------------------------------------------------------------
	// Purpose: Catch the checkbox selection changes
	//-----------------------------------------------------------------------------
	void CFFCustomHudPanelStylePresets::OnUpdateCheckButton(KeyValues *data)
	{
		RefreshAnchorPositionCheckButtons(reinterpret_cast<CheckButton*>(data->GetPtr("panel")));

		//TODO remove this loaded statement - should not be required.
		if(m_bLoaded)
		{	
			if(data->GetPtr("panel") == m_pShowHeaderText)
			{
				m_pHeaderTextX->SetEnabled(m_pShowHeaderText->IsSelected());
				m_pHeaderTextY->SetEnabled(m_pShowHeaderText->IsSelected());
				m_pHeaderTextSize->SetEnabled(m_pShowHeaderText->IsSelected());
				m_pHeaderTextColor->SetEnabled(m_pShowPanel->IsSelected());
			}
			else if (data->GetPtr("panel") == m_pShowHeaderIcon)
			{
				m_pHeaderIconX->SetEnabled(m_pShowHeaderIcon->IsSelected());
				m_pHeaderIconY->SetEnabled(m_pShowHeaderIcon->IsSelected());
				m_pHeaderIconSize->SetEnabled(m_pShowHeaderIcon->IsSelected());
				m_pHeaderIconColor->SetEnabled(m_pShowPanel->IsSelected());
			}
			else if(data->GetPtr("panel") == m_pShowText)
			{
				m_pTextX->SetEnabled(m_pShowText->IsSelected());
				m_pTextY->SetEnabled(m_pShowText->IsSelected());
				m_pTextSize->SetEnabled(m_pShowText->IsSelected());
				m_pTextColor->SetEnabled(m_pShowPanel->IsSelected());
			}
			else if(data->GetPtr("panel") == m_pShowPanel)
			{
				m_pPanelColor->SetEnabled(m_pShowPanel->IsSelected());
			}
			UpdatePresetFromControls(m_pPresets->GetActiveItemUserData());
		}
	}


	void CFFCustomHudPanelStylePresets::RefreshAnchorPositionCheckButtons(CheckButton *pCheckButton)
	{
		if(pCheckButton == m_pHeaderTextAnchorPositionTopLeft 
			|| pCheckButton == m_pHeaderTextAnchorPositionTopCenter
			|| pCheckButton == m_pHeaderTextAnchorPositionTopRight
			|| pCheckButton == m_pHeaderTextAnchorPositionMiddleLeft
			|| pCheckButton == m_pHeaderTextAnchorPositionMiddleCenter
			|| pCheckButton == m_pHeaderTextAnchorPositionMiddleRight
			|| pCheckButton == m_pHeaderTextAnchorPositionBottomLeft
			|| pCheckButton == m_pHeaderTextAnchorPositionBottomCenter
			|| pCheckButton == m_pHeaderTextAnchorPositionBottomRight)
		{	
			m_pHeaderTextAnchorPositionTopLeft->RemoveActionSignalTarget(this);
			m_pHeaderTextAnchorPositionTopCenter->RemoveActionSignalTarget(this);
			m_pHeaderTextAnchorPositionTopRight->RemoveActionSignalTarget(this);
			m_pHeaderTextAnchorPositionMiddleLeft->RemoveActionSignalTarget(this);
			m_pHeaderTextAnchorPositionMiddleCenter->RemoveActionSignalTarget(this);
			m_pHeaderTextAnchorPositionMiddleRight->RemoveActionSignalTarget(this);
			m_pHeaderTextAnchorPositionBottomLeft->RemoveActionSignalTarget(this);
			m_pHeaderTextAnchorPositionBottomCenter->RemoveActionSignalTarget(this);
			m_pHeaderTextAnchorPositionBottomRight->RemoveActionSignalTarget(this);

			if(!pCheckButton->IsSelected())
			{
				pCheckButton->SetSelected(true);
			}

			m_pHeaderTextAnchorPositionTopLeft->SetSelected(pCheckButton == m_pHeaderTextAnchorPositionTopLeft);
			m_pHeaderTextAnchorPositionTopCenter->SetSelected(pCheckButton == m_pHeaderTextAnchorPositionTopCenter);
			m_pHeaderTextAnchorPositionTopRight->SetSelected(pCheckButton == m_pHeaderTextAnchorPositionTopRight);
			m_pHeaderTextAnchorPositionMiddleLeft->SetSelected(pCheckButton == m_pHeaderTextAnchorPositionMiddleLeft);
			m_pHeaderTextAnchorPositionMiddleCenter->SetSelected(pCheckButton == m_pHeaderTextAnchorPositionMiddleCenter);
			m_pHeaderTextAnchorPositionMiddleRight->SetSelected(pCheckButton == m_pHeaderTextAnchorPositionMiddleRight);
			m_pHeaderTextAnchorPositionBottomLeft->SetSelected(pCheckButton == m_pHeaderTextAnchorPositionBottomLeft);
			m_pHeaderTextAnchorPositionBottomCenter->SetSelected(pCheckButton == m_pHeaderTextAnchorPositionBottomCenter);
			m_pHeaderTextAnchorPositionBottomRight->SetSelected(pCheckButton == m_pHeaderTextAnchorPositionBottomRight);

			m_pHeaderTextAnchorPositionTopLeft->AddActionSignalTarget(this);
			m_pHeaderTextAnchorPositionTopCenter->AddActionSignalTarget(this);
			m_pHeaderTextAnchorPositionTopRight->AddActionSignalTarget(this);
			m_pHeaderTextAnchorPositionMiddleLeft->AddActionSignalTarget(this);
			m_pHeaderTextAnchorPositionMiddleCenter->AddActionSignalTarget(this);
			m_pHeaderTextAnchorPositionMiddleRight->AddActionSignalTarget(this);
			m_pHeaderTextAnchorPositionBottomLeft->AddActionSignalTarget(this);
			m_pHeaderTextAnchorPositionBottomCenter->AddActionSignalTarget(this);
			m_pHeaderTextAnchorPositionBottomRight->AddActionSignalTarget(this);
		}

		if(pCheckButton == m_pHeaderIconAnchorPositionTopLeft 
			|| pCheckButton == m_pHeaderIconAnchorPositionTopCenter
			|| pCheckButton == m_pHeaderIconAnchorPositionTopRight
			|| pCheckButton == m_pHeaderIconAnchorPositionMiddleLeft
			|| pCheckButton == m_pHeaderIconAnchorPositionMiddleCenter
			|| pCheckButton == m_pHeaderIconAnchorPositionMiddleRight
			|| pCheckButton == m_pHeaderIconAnchorPositionBottomLeft
			|| pCheckButton == m_pHeaderIconAnchorPositionBottomCenter
			|| pCheckButton == m_pHeaderIconAnchorPositionBottomRight)
		{	
			m_pHeaderIconAnchorPositionTopLeft->RemoveActionSignalTarget(this);
			m_pHeaderIconAnchorPositionTopCenter->RemoveActionSignalTarget(this);
			m_pHeaderIconAnchorPositionTopRight->RemoveActionSignalTarget(this);
			m_pHeaderIconAnchorPositionMiddleLeft->RemoveActionSignalTarget(this);
			m_pHeaderIconAnchorPositionMiddleCenter->RemoveActionSignalTarget(this);
			m_pHeaderIconAnchorPositionMiddleRight->RemoveActionSignalTarget(this);
			m_pHeaderIconAnchorPositionBottomLeft->RemoveActionSignalTarget(this);
			m_pHeaderIconAnchorPositionBottomCenter->RemoveActionSignalTarget(this);
			m_pHeaderIconAnchorPositionBottomRight->RemoveActionSignalTarget(this);

			if(!pCheckButton->IsSelected())
			{
				pCheckButton->SetSelected(true);
			}

			m_pHeaderIconAnchorPositionTopLeft->SetSelected(pCheckButton == m_pHeaderIconAnchorPositionTopLeft);
			m_pHeaderIconAnchorPositionTopCenter->SetSelected(pCheckButton == m_pHeaderIconAnchorPositionTopCenter);
			m_pHeaderIconAnchorPositionTopRight->SetSelected(pCheckButton == m_pHeaderIconAnchorPositionTopRight);
			m_pHeaderIconAnchorPositionMiddleLeft->SetSelected(pCheckButton == m_pHeaderIconAnchorPositionMiddleLeft);
			m_pHeaderIconAnchorPositionMiddleCenter->SetSelected(pCheckButton == m_pHeaderIconAnchorPositionMiddleCenter);
			m_pHeaderIconAnchorPositionMiddleRight->SetSelected(pCheckButton == m_pHeaderIconAnchorPositionMiddleRight);
			m_pHeaderIconAnchorPositionBottomLeft->SetSelected(pCheckButton == m_pHeaderIconAnchorPositionBottomLeft);
			m_pHeaderIconAnchorPositionBottomCenter->SetSelected(pCheckButton == m_pHeaderIconAnchorPositionBottomCenter);
			m_pHeaderIconAnchorPositionBottomRight->SetSelected(pCheckButton == m_pHeaderIconAnchorPositionBottomRight);

			m_pHeaderIconAnchorPositionTopLeft->AddActionSignalTarget(this);
			m_pHeaderIconAnchorPositionTopCenter->AddActionSignalTarget(this);
			m_pHeaderIconAnchorPositionTopRight->AddActionSignalTarget(this);
			m_pHeaderIconAnchorPositionMiddleLeft->AddActionSignalTarget(this);
			m_pHeaderIconAnchorPositionMiddleCenter->AddActionSignalTarget(this);
			m_pHeaderIconAnchorPositionMiddleRight->AddActionSignalTarget(this);
			m_pHeaderIconAnchorPositionBottomLeft->AddActionSignalTarget(this);
			m_pHeaderIconAnchorPositionBottomCenter->AddActionSignalTarget(this);
			m_pHeaderIconAnchorPositionBottomRight->AddActionSignalTarget(this);
		}

		if(pCheckButton == m_pTextAnchorPositionTopLeft 
			|| pCheckButton == m_pTextAnchorPositionTopCenter
			|| pCheckButton == m_pTextAnchorPositionTopRight
			|| pCheckButton == m_pTextAnchorPositionMiddleLeft
			|| pCheckButton == m_pTextAnchorPositionMiddleCenter
			|| pCheckButton == m_pTextAnchorPositionMiddleRight
			|| pCheckButton == m_pTextAnchorPositionBottomLeft
			|| pCheckButton == m_pTextAnchorPositionBottomCenter
			|| pCheckButton == m_pTextAnchorPositionBottomRight)
		{	
			m_pTextAnchorPositionTopLeft->RemoveActionSignalTarget(this);
			m_pTextAnchorPositionTopCenter->RemoveActionSignalTarget(this);
			m_pTextAnchorPositionTopRight->RemoveActionSignalTarget(this);
			m_pTextAnchorPositionMiddleLeft->RemoveActionSignalTarget(this);
			m_pTextAnchorPositionMiddleCenter->RemoveActionSignalTarget(this);
			m_pTextAnchorPositionMiddleRight->RemoveActionSignalTarget(this);
			m_pTextAnchorPositionBottomLeft->RemoveActionSignalTarget(this);
			m_pTextAnchorPositionBottomCenter->RemoveActionSignalTarget(this);
			m_pTextAnchorPositionBottomRight->RemoveActionSignalTarget(this);

			if(!pCheckButton->IsSelected())
			{
				pCheckButton->SetSelected(true);
			}

			m_pTextAnchorPositionTopLeft->SetSelected(pCheckButton == m_pTextAnchorPositionTopLeft);
			m_pTextAnchorPositionTopCenter->SetSelected(pCheckButton == m_pTextAnchorPositionTopCenter);
			m_pTextAnchorPositionTopRight->SetSelected(pCheckButton == m_pTextAnchorPositionTopRight);
			m_pTextAnchorPositionMiddleLeft->SetSelected(pCheckButton == m_pTextAnchorPositionMiddleLeft);
			m_pTextAnchorPositionMiddleCenter->SetSelected(pCheckButton == m_pTextAnchorPositionMiddleCenter);
			m_pTextAnchorPositionMiddleRight->SetSelected(pCheckButton == m_pTextAnchorPositionMiddleRight);
			m_pTextAnchorPositionBottomLeft->SetSelected(pCheckButton == m_pTextAnchorPositionBottomLeft);
			m_pTextAnchorPositionBottomCenter->SetSelected(pCheckButton == m_pTextAnchorPositionBottomCenter);
			m_pTextAnchorPositionBottomRight->SetSelected(pCheckButton == m_pTextAnchorPositionBottomRight);

			m_pTextAnchorPositionTopLeft->AddActionSignalTarget(this);
			m_pTextAnchorPositionTopCenter->AddActionSignalTarget(this);
			m_pTextAnchorPositionTopRight->AddActionSignalTarget(this);
			m_pTextAnchorPositionMiddleLeft->AddActionSignalTarget(this);
			m_pTextAnchorPositionMiddleCenter->AddActionSignalTarget(this);
			m_pTextAnchorPositionMiddleRight->AddActionSignalTarget(this);
			m_pTextAnchorPositionBottomLeft->AddActionSignalTarget(this);
			m_pTextAnchorPositionBottomCenter->AddActionSignalTarget(this);
			m_pTextAnchorPositionBottomRight->AddActionSignalTarget(this);
		}
	}
		
	void CFFCustomHudPanelStylePresets::SendUpdatedPresetPreviewToPresetAssignment(const char *pszPresetName, KeyValues *kvPresetPreview)
	{
		g_AP->PanelStylePresetPreviewUpdated(pszPresetName, kvPresetPreview);
	}	
	void CFFCustomHudPanelStylePresets::SendUpdatedPresetToPresetAssignment(const char *pszPresetName)
	{
		g_AP->PanelStylePresetUpdated(pszPresetName);
	}
	void CFFCustomHudPanelStylePresets::SendRenamedPresetToPresetAssignment(const char *pszOldPresetName, const char *pszNewPresetName)
	{
		g_AP->PanelStylePresetRenamed(pszOldPresetName, pszNewPresetName);
	}
	void CFFCustomHudPanelStylePresets::SendDeletedPresetToPresetAssignment(const char *pszDeletedPresetName)
	{
		g_AP->PanelStylePresetDeleted(pszDeletedPresetName);
	}
	void CFFCustomHudPanelStylePresets::SendNewPresetToPresetAssignment(const char *pszPresetName, KeyValues *kvPreset)
	{
		g_AP->PanelStylePresetAdded(pszPresetName, kvPreset);
	}
};