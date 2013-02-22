#include "cbase.h"
#include "ff_customhudoptions_assignpresets.h"

#define PRESETASSIGNMENT_FILE "HudPresetAssignments.vdf"

using namespace vgui;

CFFCustomHudAssignPresets *g_AP = NULL;


CFFCustomHudAssignPresets::CFFCustomHudAssignPresets(Panel *parent, char const *panelName, CFFCustomHudItemStylePresets* stylePresetsClass, CFFCustomHudPanelStylePresets* arrangementPresetsClass, CFFCustomHudPositionPresets* positionPresetsClass) : BaseClass(parent, panelName)
{
	m_bLoaded = false;

	m_pItemStylePresetsClass = stylePresetsClass;
	m_pPanelStylePresetsClass = arrangementPresetsClass;
	m_pPositionPresetsClass = positionPresetsClass;

	m_pPreviewMode = new CheckButton(this, "PreviewMode", "#GameUI_PreviewMode");

	m_pPrimary = new ComboBox(this, "ParentCombo", 7, false);
	m_pSecondary = new ComboBox(this, "ChildCombo", 7, false);

	m_pItemStylePresets = new ComboBox(this, "ItemStyleCombo", 7, false);
	m_pPanelStylePresets = new ComboBox(this, "PanelStyleCombo", 7, false);
	m_pPositionPresets = new ComboBox(this, "PositionCombo", 7, false);
	
	m_pItemStylePresets->RemoveActionSignalTarget(this);
	m_pPanelStylePresets->RemoveActionSignalTarget(this);
	m_pPositionPresets->RemoveActionSignalTarget(this);

	m_pCopyDefaultPosition = new Button(this, "CopyDefaultPosition", "#GameUI_CopyDefault", this, "CopyDefaultPosition");
	m_pCopyDefaultPanelStyle = new Button(this, "CopyDefaultPanelStyle", "#GameUI_CopyDefault", this, "CopyDefaultPanelStyle");
	m_pCopyDefaultItemStyle = new Button(this, "CopyDefaultItemStyle", "#GameUI_CopyDefault", this, "CopyDefaultItemStyle");

	m_pPanelSpecificOptions = new Panel(this, "SpecificOptions");
	m_pPanelSpecificOptions->SetPos(10, 135);
	m_pPanelSpecificOptions->SetSize(585, 130);

	m_bCopyDefaultPosition = false;
	m_bCopyDefaultPanelStyle = false;
	m_bCopyDefaultItemStyle = false;

	LoadControlSettings("resource/ui/FFCustomHudOptionsAssignPresets.res");
	g_AP = this;
}
CFFCustomHudAssignPresets::~CFFCustomHudAssignPresets()
{
	m_kvRequiredUpdates->deleteThis();
	m_kvRequiredUpdates = NULL;
	m_kvLoadedAssignments->deleteThis();
	m_kvLoadedAssignments = NULL;
	g_AP = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFCustomHudAssignPresets::Load()
{
	static bool bIsLoading = false;

	if(bIsLoading)
		return;

	bIsLoading = true;

	//it wouldnt do anything if we reloaded the assignments file after 
	//the hud elements have initialised so just don't allow it
	if(!m_bLoaded)
	{
		m_pPrimary->RemoveAll();
		m_pSecondary->RemoveAll();
		m_pSecondary->SetVisible(false);
		m_kvLoadedAssignments = new KeyValues("Assignments");
		m_kvLoadedAssignments->LoadFromFile(vgui::filesystem(), PRESETASSIGNMENT_FILE);

		m_kvRequiredUpdates = new KeyValues("Changes");

		m_bLoaded = true;
	}
	else if(m_pPrimary->GetItemCount() > 0)
	{
		m_kvRequiredUpdates->deleteThis();
		m_kvRequiredUpdates = NULL;
		m_kvRequiredUpdates = new KeyValues("Changes");

		if(m_pSecondary->IsVisible())
		{
			//this keyvalue holds the assignments which were saved for the item we are selecting
			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			ApplyAssignmentToControls(kvSecondary);
			SendStyleDataToAssignment(kvSecondary);
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			ApplyAssignmentToControls(kvPrimary);
			SendStyleDataToAssignment(kvPrimary);
		}
	}
	
	bIsLoading = false;
}
void CFFCustomHudAssignPresets::Reset() { Load(); }
void CFFCustomHudAssignPresets::Apply() {
	//update hud elements that need updating
	for(KeyValues *kvPrimaryUpdate = m_kvRequiredUpdates->GetFirstSubKey(); kvPrimaryUpdate != NULL; kvPrimaryUpdate = kvPrimaryUpdate->GetNextKey())
	{
		if(kvPrimaryUpdate->IsEmpty())
		//then this is an assignment and not a parent
		{
			//go through primary and check for assignments which need an update
			for(int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
			{
				KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
				if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
				//if this is a parent node
				{
					//we're not looking for a parent so continue
					continue;
				}
				else
				{
					if(Q_stricmp(kvPrimaryUpdate->GetName(), kvPrimary->GetName()) == 0)
					//if this is an assignment needing an update
					{
						SendStyleDataToAssignment(kvPrimary);

						//we've dealt with this one so lets break out
						break;
					}
				}
			}
		}
		else
		//we're dealing with an assignment that has a parent
		{
			//go through primary
			for(int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
			{
				KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
				if(Q_stricmp(kvPrimary->GetName(), kvPrimaryUpdate->GetName()) == 0)
				//if we've matched the parent
				{
					for(KeyValues *kvUpdate = kvPrimaryUpdate->GetFirstTrueSubKey(); kvUpdate != NULL; kvUpdate = kvUpdate->GetNextTrueSubKey())
					{
						for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
						//go through assignments within the parent node
						{
							if(Q_stricmp(kvUpdate->GetName(), kvAssignment->GetName()) == 0)
							//if this is an assignment needing an update
							{
								SendStyleDataToAssignment(kvAssignment);
								//we've dealt with this one so lets break out
								break;
							}
						}
					}
				}
			}
		}
	}
	m_kvRequiredUpdates->Clear();

	KeyValues *kvPresets = GetPresetAssignments();

	kvPresets->SaveToFile(vgui::filesystem(), PRESETASSIGNMENT_FILE);
	kvPresets->deleteThis();
}


KeyValues* CFFCustomHudAssignPresets::GetPresetAssignments()
{
	KeyValues *kvPresets = new KeyValues("PresetAssignments");
	
	for (int i = 0; i < m_pPrimary->GetItemCount(); ++i)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(i);

		//use new copied keyvalues
		//crash occurred when saving twice, 
		//something to do with deleting the preset used in the combobox (I think)
		KeyValues *kvPresetToSave = new KeyValues(kvPrimary->GetName());
		kvPrimary->CopySubkeys(kvPresetToSave);	

		if(Q_stricmp(kvPresetToSave->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{	
			for(KeyValues *kvSecondary = kvPresetToSave->GetFirstTrueSubKey(); kvSecondary != NULL; kvSecondary = kvSecondary->GetNextTrueSubKey())
			{
				KeyValues *kvPanelSpecificOptions = kvSecondary->FindKey("PanelSpecificOptions");
				if(kvPanelSpecificOptions)
				{
					kvSecondary->RemoveSubKey(kvPanelSpecificOptions);
				}
			}
		}
		else
		//if this is an assignment
		{
			KeyValues *kvPanelSpecificOptions = kvPresetToSave->FindKey("PanelSpecificOptions");
			if(kvPanelSpecificOptions)
			{
				kvPresetToSave->RemoveSubKey(kvPanelSpecificOptions);
			}
		}
			
		kvPresets->AddSubKey(kvPresetToSave);
	}

	return kvPresets;
}

	
void CFFCustomHudAssignPresets::SendStyleDataToAssignment(KeyValues *kvAssignment)
{
	KeyValues* kvPresetData = GetStyleDataForAssignment(kvAssignment);

	Panel* pSender = (Panel* ) kvAssignment->GetPtr("panel", NULL);
	PostMessage(pSender, kvPresetData);
}

void CFFCustomHudAssignPresets::SendPresetPreviewDataToAssignment(KeyValues *kvAssignment, KeyValues *kvPreset)
{
	//we need the SetPresetPreviewData as the keyvalue 
	//name will not call the quantitypanel function
	KeyValues* kvPresetData = new KeyValues("SetPresetPreviewData");

	//because copying all keys is retarded (impossible?!) 
	//we'll have to chain....
	kvPresetData->ChainKeyValue(kvPreset);

	Panel* pSender = (Panel* ) kvAssignment->GetPtr("panel", NULL);
	PostMessage(pSender, kvPresetData);
}

KeyValues* CFFCustomHudAssignPresets::GetStyleDataForAssignment(KeyValues *kvAssignment)
{
	KeyValues *kvPositionPresetData;
	KeyValues *kvPanelStylePresetData;
	KeyValues *kvItemStylePresetData;
	KeyValues *kvPanelSpecificValues = kvAssignment->FindKey("PanelSpecificValues");

	//get localisation of default
	wchar_t *wszDefault = vgui::localize()->Find("#GameUI_Default");
	char szDefault[32];
	localize()->ConvertUnicodeToANSI( wszDefault, szDefault, sizeof( szDefault ) );

	//if its not using default
	if(Q_stricmp(kvAssignment->GetString("Position"), szDefault) != 0)
	{
		//Get the preset
		kvPositionPresetData = m_pPositionPresetsClass->GetPresetDataByName(kvAssignment->GetString("Position"));

		//if no position data found (preset has been deleted perhaps?)
		if(kvPositionPresetData == NULL)
		{
			//TODO: consider if this is correct...  
			//pehapse shouldn't get here but if we do then are we only setting the assignment in primary, secondary or both??

			//set the assignment to use default
			kvAssignment->SetString("Position", szDefault);
		}
	}
	//else we're using default
	else
	{
		kvPositionPresetData = NULL;
	}

	//if its not using default
	if(Q_stricmp(kvAssignment->GetString("PanelStyle"),szDefault) != 0)
	{
		//Get the preset
		kvPanelStylePresetData = m_pPanelStylePresetsClass->GetPresetDataByName(kvAssignment->GetString("PanelStyle"));

		//if no arrangement data found (preset has been deleted perhaps?)
		if(kvPanelStylePresetData == NULL)
		{
			//TODO: consider if this is correct...  
			//pehapse shouldn't get here but if we do then are we only setting the assignment in primary, secondary or both??

			//set the assignment to use default
			kvAssignment->SetString("PanelStyle", szDefault);
		}
	}
	//else we're using default
	else
	{
		kvPanelStylePresetData = NULL;
	}

	//if its not using default
	if(Q_stricmp(kvAssignment->GetString("ItemStyle"),szDefault) != 0)
	{
		//Get the preset
		kvItemStylePresetData = m_pItemStylePresetsClass->GetPresetDataByName(kvAssignment->GetString("ItemStyle"));

		//if no arrangement data found (preset has been deleted perhaps?)
		if(kvItemStylePresetData == NULL)
		{
			//TODO: consider if this is correct...  
			//pehapse shouldn't get here but if we do then are we only setting the assignment in primary, secondary or both??

			//set the assignment to use default
			kvAssignment->SetString("ItemStyle", szDefault);
		}
	}
	//else we're using default
	else
	{
		kvItemStylePresetData = NULL;
	}

	if(kvPanelSpecificValues)
	{
		kvPanelSpecificValues = kvPanelSpecificValues->MakeCopy();
	}

	KeyValues *kvPresetData = new KeyValues("SetStyleData");
	KeyValues *kvChainTo = kvPresetData;
	//because copying all keys is retarded (impossible?!) 
	//we'll have to chain....
	if(kvItemStylePresetData != NULL)
	{
		kvChainTo->ChainKeyValue(kvItemStylePresetData);
		kvChainTo = kvItemStylePresetData;
	}
	if(kvPanelStylePresetData != NULL)
	{
		kvChainTo->ChainKeyValue(kvPanelStylePresetData);
		kvChainTo = kvPanelStylePresetData;
	}
	if(kvPositionPresetData != NULL)
	{
		kvChainTo->ChainKeyValue(kvPositionPresetData);
		//kvChainTo = kvPositionPresetData;
	}
	if(kvPanelSpecificValues != NULL)
	{
		kvPresetData->AddSubKey(kvPanelSpecificValues);
	}

	kvPresetData->SetInt("previewMode", kvAssignment->GetInt("previewMode", -1));

	return kvPresetData;
}

void CFFCustomHudAssignPresets::UpdateSecondaryComboFromParentKey(KeyValues *kvPrimary)
{
	if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
	//if this is a parent node
	{	
		m_pSecondary->SetVisible(true);
		m_pSecondary->RemoveAll();
		for(KeyValues *kvSecondary = kvPrimary->GetFirstTrueSubKey(); kvSecondary != NULL; kvSecondary = kvSecondary->GetNextTrueSubKey())
		{
			m_pSecondary->AddItem(kvSecondary->GetString("AssignmentName", kvSecondary->GetName()), kvSecondary);
		}
		//if theres assignments which there should now be! (just in case)
		if(m_pSecondary->GetItemCount() > 0)
		{
			m_pSecondary->ActivateItemByRow(0);
		}
	}
	else
	//if this is an assignment
	{
		m_pSecondary->SetVisible(false);
		ApplyAssignmentToControls(kvPrimary);
	}
}

void CFFCustomHudAssignPresets::ApplyAssignmentToControls(KeyValues *kvAssignment)
{
	m_pPositionPresets->RemoveActionSignalTarget(this);
	m_pPanelStylePresets->RemoveActionSignalTarget(this);
	m_pItemStylePresets->RemoveActionSignalTarget(this);

	//get localisation of default
	wchar_t *wszDefault = vgui::localize()->Find("#GameUI_Default");
	char szDefault[32];
	localize()->ConvertUnicodeToANSI( wszDefault, szDefault, sizeof( szDefault ) );

	int iPreviewMode = kvAssignment->GetInt("previewMode", kvAssignment->GetInt("previewMode", -1));

	m_pPreviewMode->SetSelected((iPreviewMode == 1 ? true : false));

	bool bPositionMatched = false;
	//for:
	//1. It's not matched
	//2. hasn't reached the last Position preset
	//3. assignment has a Position to look for
	for (int i = 0; Q_stricmp(kvAssignment->GetString("Position"), "") != 0 && i < m_pPositionPresets->GetItemCount() && !bPositionMatched; ++i)
	{
		//if name exists
		if(Q_stricmp(kvAssignment->GetString("Position"), m_pPositionPresets->GetItemUserData(i)->GetName()) == 0)
		{
			//set matched
			bPositionMatched = true;
			//activate by row
			m_pPositionPresets->ActivateItemByRow(i);
			break;
		}
	}
	if(!bPositionMatched)
	{	
		//So lets just use default for this preset! (No custom styling)
		kvAssignment->SetString("Position", szDefault);
		m_pPositionPresets->ActivateItemByRow(0);
	}

	bool bPanelStyleMatched = false;
	//for:
	//1. It's not matched
	//2. hasn't reached the last PanelStyle preset
	//3. assignment has a PanelStyle to look for
	for (int i = 0; Q_stricmp(kvAssignment->GetString("PanelStyle"), "") != 0 && i < m_pPanelStylePresets->GetItemCount() && !bPanelStyleMatched; ++i)
	{
		//if name exists
		if(Q_stricmp(kvAssignment->GetString("PanelStyle"), m_pPanelStylePresets->GetItemUserData(i)->GetName()) == 0)
		{
			//set matched
			bPanelStyleMatched = true;
			//activate by row
			m_pPanelStylePresets->ActivateItemByRow(i);
			break;
		}
	}
	if(!bPanelStyleMatched)
	{	
		//So lets just tick the default for this preset! (No custom styling)
		kvAssignment->SetString("PanelStyle", szDefault);
			m_pPanelStylePresets->ActivateItemByRow(0);
	}

	bool bItemStyleMatched = false;
	//for:
	//1. It's not matched
	//2. hasn't reached the last ItemStyle preset
	//3. assignment has a ItemStyle to look for
	for (int i = 0; Q_stricmp(kvAssignment->GetString("ItemStyle"), "") != 0 && i < m_pItemStylePresets->GetItemCount() && !bItemStyleMatched; ++i)
	{
		//if name exists
		if(Q_stricmp(kvAssignment->GetString("ItemStyle"), m_pItemStylePresets->GetItemUserData(i)->GetName()) == 0)
		{
			//set matched
			bItemStyleMatched = true;
			//activate by row
			m_pItemStylePresets->ActivateItemByRow(i);
			break;
		}
	}
	if(!bItemStyleMatched)
	{	
		//So lets just tick the default for this preset! (No custom styling)
		kvAssignment->SetString("ItemStyle", szDefault);
		m_pItemStylePresets->ActivateItemByRow(0);
	}

	//remove all exisiting specific options
	for(int i = m_pPanelSpecificOptions->GetChildCount() - 1; i >= 0; --i)
	//count backwards cause we're removing stuff (counting forwards will FAIL EPICLY!!)
	{
		Panel* childPanel = m_pPanelSpecificOptions->GetChild(i);
		childPanel->DeletePanel();
	}
	
	//deal with assignment specific options
	KeyValues *kvPanelSpecificOptions = kvAssignment->FindKey("PanelSpecificOptions");
	KeyValues *kvPanelSpecificValues = kvAssignment->FindKey("PanelSpecificValues");
	if(kvPanelSpecificOptions)
	{
		int  iYCoords = 0;
		int  iRowHeight = 30;
		// Loop through creating new options for each one
		for (KeyValues *kvPanelSpecificOption = kvPanelSpecificOptions->GetFirstSubKey(); kvPanelSpecificOption != NULL; kvPanelSpecificOption = kvPanelSpecificOption->GetNextKey())
		{
			// Boolean is just a simple checkbox
			if (Q_strnicmp(kvPanelSpecificOption->GetName(), "boolean", 7) == 0)
			{
				CheckButton *cb = new CheckButton(m_pPanelSpecificOptions, kvPanelSpecificOption->GetString("name"), kvPanelSpecificOption->GetString("text"));

				if (!cb)
					continue;

				//remove panel specific options from receiving action signals
				cb->RemoveActionSignalTarget(m_pPanelSpecificOptions);

				cb->SetPos(0, iYCoords);
				cb->SetSize(120, iRowHeight);

				if(kvPanelSpecificValues)
					cb->SetSelected(kvPanelSpecificValues->GetInt(kvPanelSpecificOption->GetString("name"),kvPanelSpecificOption->GetInt("defaultValue")));
				else
					cb->SetSelected(kvPanelSpecificOption->GetInt("defaultValue") == 1 ? true : false );

				//add this main preset class as the receiver of actions signals
				cb->AddActionSignalTarget(this);

				iYCoords += iRowHeight;
			}
			else if (Q_strnicmp(kvPanelSpecificOption->GetName(), "combobox", 8) == 0)
			{
				KeyValues *kvValues = kvPanelSpecificOption->FindKey("values");
				int nValues = 0;

				if (!kvValues)
					continue;

				// First count all the values so we know how many lines are
				// needed for the combobox
				nValues = 0;
				KeyValues *kvValue = kvValues->GetFirstSubKey();
				while (kvValue)
				{
					nValues++;
					kvValue = kvValue->GetNextKey();
				}

				//create the combobox
				ComboBox *cb = new ComboBox(m_pPanelSpecificOptions, kvPanelSpecificOption->GetString("name"), nValues, false);

				//just in case
				if (!cb)
					continue;

				//remove panel specific options from receiving action signals
				cb->RemoveActionSignalTarget(m_pPanelSpecificOptions);

				kvValues = kvPanelSpecificOption->FindKey("values", false);

				if (!kvValues)
					continue;

				int iActiveItemId = -1;

				// Now go through all the values and add them to the combobox
				kvValue = kvValues->GetFirstSubKey();
				while (kvValue)
				{
					const char *pszValue = kvValue->GetName();
					const char *pszCaption = kvValues->GetString(pszValue);

					KeyValues *kvItem = new KeyValues("kvItem");
					//this pszValue is actually an int but dont bother converting.
					kvItem->SetString("value", pszValue);
					int iItemId = cb->AddItem(pszCaption, kvItem);
					kvItem->deleteThis();
					kvItem = NULL;

					//if this option should be used or if nothing is currently chosen and this matches the default value
					if(kvPanelSpecificValues)
					{
						if(Q_stricmp(kvPanelSpecificValues->GetString(kvPanelSpecificOption->GetString("name"), kvPanelSpecificOption->GetString("defaultValue")), pszValue) == 0)
						{
							iActiveItemId = iItemId;
						}
					}
					else
					{
						if(Q_stricmp(kvPanelSpecificOption->GetString("defaultValue"), pszValue) == 0)
						{
							iActiveItemId = iItemId;
						}
					}

					kvValue = kvValue->GetNextKey();
				}

				cb->SetPos(0, iYCoords);
				cb->SetSize(85, iRowHeight);

				if(iActiveItemId != -1)
				{
					cb->ActivateItem(iActiveItemId);
				}
				else
				{
					cb->ActivateItemByRow(0);
				}

				//add this main preset class as the receiver of actions signals
				cb->AddActionSignalTarget(this);


				// label the combobox!
				char labelName[128];
				Q_snprintf( labelName, sizeof( labelName ), "%sLabel", kvPanelSpecificOption->GetString("name") );
				Label *l = new Label(m_pPanelSpecificOptions, labelName, kvPanelSpecificOption->GetString("text"));
				if (l)
				{
					l->SetPos(95, iYCoords);
					l->SetSize(110, iRowHeight);
				}

				iYCoords += iRowHeight;
			}
		}
	}

	m_pPositionPresets->AddActionSignalTarget(this);
	m_pPanelStylePresets->AddActionSignalTarget(this);
	m_pItemStylePresets->AddActionSignalTarget(this);
}

void CFFCustomHudAssignPresets::RegisterAssignmentChange(KeyValues *kvSelf, KeyValues *kvParent)
{
	//TODO BOOBS
	//If kvSelf is in preview mode then send the data for the assignment
	if(kvParent != NULL)
	{
		KeyValues *kvRequiredUpdateParent = m_kvRequiredUpdates->FindKey(kvParent->GetName());
		if(!kvRequiredUpdateParent)
		//if the parent doesnt already exist in the required updates
		{
			//create the parent update keyvalue
			kvRequiredUpdateParent = new KeyValues(kvParent->GetName());

			//create the update keyvalue
			KeyValues *kvUpdate = new KeyValues(kvSelf->GetName());

			//add it to the parent update keyvalue
			kvRequiredUpdateParent->AddSubKey(kvUpdate);

			//add the parent to the required updates
			m_kvRequiredUpdates->AddSubKey(kvRequiredUpdateParent);
		}
		else
		{
			if(!kvRequiredUpdateParent->FindKey(kvSelf->GetName()))
			//if the update doesnt already exist in the parent
			{
				//create the update keyvalue
				KeyValues *kvUpdate = new KeyValues(kvSelf->GetName());

				//add the update to the parent
				kvRequiredUpdateParent->AddSubKey(kvUpdate);
			}
		}
	}
	else
	{
		KeyValues *kvRequiredUpdate = m_kvRequiredUpdates->FindKey(kvSelf->GetName());
		//if the update doesnt already exist
		if(!kvRequiredUpdate)
		{
			//create the update keyvalue
			kvRequiredUpdate = new KeyValues(kvSelf->GetName());

			//add the update to the required updates
			m_kvRequiredUpdates->AddSubKey(kvRequiredUpdate);
		}
	}
}

void CFFCustomHudAssignPresets::CreatePresetFromPanelDefault(KeyValues *kvPresetData)
{
	if(m_bCopyDefaultItemStyle)
	{
		m_pItemStylePresetsClass->CreatePresetFromPanelDefault(kvPresetData);
		m_bCopyDefaultItemStyle = false;
	}

	if(m_bCopyDefaultPanelStyle)
	{
		m_pPanelStylePresetsClass->CreatePresetFromPanelDefault(kvPresetData);
		m_bCopyDefaultPanelStyle = false;
	}

	if(m_bCopyDefaultPosition)
	{
		m_pPositionPresetsClass->CreatePresetFromPanelDefault(kvPresetData);
		m_bCopyDefaultPosition = false;
	}
}
 
//-----------------------------------------------------------------------------
// Purpose: Catch the checkboxes changing their state
//-----------------------------------------------------------------------------
void CFFCustomHudAssignPresets::OnUpdateCheckButton(KeyValues *data)
{
	if(data->GetPtr("panel") == m_pPreviewMode)
	{
		if(m_pSecondary->IsVisible())
		//if the assignment is in secondary
		{
			//we have to update the assignent in the secondary combo
			//as well as the data in the primary to keep them in sync!

			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			
			kvSecondary->SetInt("previewMode", (m_pPreviewMode->IsSelected() ? 1 : 0));

			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());

			kvAssignment->SetInt("previewMode", (m_pPreviewMode->IsSelected() ? 1 : 0));

			RegisterAssignmentChange(kvAssignment, kvPrimary);
		}
		else
		{
			KeyValues *kvAssignment = m_pPrimary->GetActiveItemUserData();		

			kvAssignment->SetInt("previewMode", (m_pPreviewMode->IsSelected() ? 1 : 0));

			RegisterAssignmentChange(kvAssignment);
		}
	}
	else
	//panel specific options
	{
		CheckButton* selectedCheckButton = NULL;

		for(int i = m_pPanelSpecificOptions->GetChildCount() - 1; i >= 0; --i)
		{
			Panel* childPanel = m_pPanelSpecificOptions->GetChild(i);
			if(data->GetPtr("panel") == childPanel)
			{
				selectedCheckButton = (CheckButton*)childPanel;
			}
		}

		if(!selectedCheckButton)
			return;

		KeyValues *kvPanelSpecificValues;
		const char* panelSpecificOptionName = selectedCheckButton->GetName();
		const int panelSpecificOptionValue = selectedCheckButton->IsSelected() ? 1 : 0;

		if(m_pSecondary->IsVisible())
		//if the assignment is in secondary
		{
			//we have to update the assignent in the secondary combo
			//as well as the data in the primary to keep them in sync!

			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			
			kvPanelSpecificValues = kvSecondary->FindKey("PanelSpecificValues");

			if(!kvPanelSpecificValues)
			{
				kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
				kvPanelSpecificValues->SetInt(panelSpecificOptionName, panelSpecificOptionValue);
				kvSecondary->AddSubKey(kvPanelSpecificValues);
			}
			else
			{
				kvPanelSpecificValues->SetInt(panelSpecificOptionName, panelSpecificOptionValue);
			}

			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());

			kvPanelSpecificValues = kvAssignment->FindKey("PanelSpecificValues");

			if(!kvPanelSpecificValues)
			{
				kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
				kvPanelSpecificValues->SetInt(panelSpecificOptionName, panelSpecificOptionValue);
				kvAssignment->AddSubKey(kvPanelSpecificValues);
			}
			else
			{
				kvPanelSpecificValues->SetInt(panelSpecificOptionName, panelSpecificOptionValue);
			}

			RegisterAssignmentChange(kvAssignment, kvPrimary);
		}
		else
		{
			KeyValues *kvAssignment = m_pPrimary->GetActiveItemUserData();		

			kvPanelSpecificValues = kvAssignment->FindKey("PanelSpecificValues");

			if(!kvPanelSpecificValues)
			{
				kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
				kvPanelSpecificValues->SetInt(panelSpecificOptionName, panelSpecificOptionValue);
				kvAssignment->AddSubKey(kvPanelSpecificValues);
			}
			else
			{
				kvPanelSpecificValues->SetInt(panelSpecificOptionName, panelSpecificOptionValue);
			}

			RegisterAssignmentChange(kvAssignment);
		}
	}
}
	 
//-----------------------------------------------------------------------------
// Purpose: Catch the "Copy Default" buttons
//-----------------------------------------------------------------------------
void CFFCustomHudAssignPresets::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	if (Q_strcmp(pszCommand, "CopyDefaultPosition") == 0)
	{
		m_bCopyDefaultPosition = true;
	}
	else if(Q_strcmp(pszCommand, "CopyDefaultPanelStyle") == 0)
	{
		m_bCopyDefaultPanelStyle = true;
	}
	else if(Q_strcmp(pszCommand, "CopyDefaultItemStyle") == 0)
	{
		m_bCopyDefaultItemStyle = true;
	}

	KeyValues *kvPresetData = new KeyValues("GetStyleData");
	KeyValues *kvAssignment;

	if(m_pSecondary->IsVisible())
	//if the assignment is in secondary
	{
		kvAssignment = m_pSecondary->GetActiveItemUserData();
	}
	else
	{
		kvAssignment = m_pPrimary->GetActiveItemUserData();
	}

	Panel* pSender = (Panel* ) kvAssignment->GetPtr("panel", NULL);
	PostMessage(pSender, kvPresetData);	
}
	 
//-----------------------------------------------------------------------------
// Purpose: Catch quantity panels requesting the ability of customisation
//-----------------------------------------------------------------------------
	
void CFFCustomHudAssignPresets::OnAddQuantityPanel(KeyValues *data)
{
	bool bFirstPanel = false;
	bool bSelfMatched = false;

	Panel* pSender = (Panel* ) data->GetPtr("panel", NULL);
	KeyValues *kvParent, *kvSelf;
	if(!(Q_stricmp(data->GetString("parentName"), "") == 0))
	//if it has a parent 
	//like SG, Dispenser, ManCannon might all be considered a buildstate
	//like Player, SG, Dispenser is all part of crosshairinfo
	{
		//Just in case: Report-FF-Bug to show that the keyvlaues for the requesting panel are not set up right
		kvParent = new KeyValues(data->GetString("parentName", "Report-FF-Bug"));
		kvParent->SetString("CategoryName", data->GetString("parentText", "Report-FF-Bug"));
		kvSelf = new KeyValues(data->GetString("selfName", "Report-FF-Bug"));
		kvSelf->SetString("AssignmentName", data->GetString("selfText", "Report-FF-Bug"));
		kvSelf->SetPtr("panel", pSender);
		kvParent->AddSubKey(kvSelf);
	}
	else
	//if its a quantity panel with no other ties
	//such as speedometer, potentially health and armour readouts?!
	{
		kvSelf = new KeyValues(data->GetString("selfName", "Report-FF-Bug"));
		kvSelf->SetString("AssignmentName", data->GetString("selfText", "Report-FF-Bug"));
		kvSelf->SetPtr("panel", pSender);
		kvParent = NULL;
	}

	//get the panel specific options from the data
	KeyValues *kvPanelSpecificOptions = data->FindKey("PanelSpecificOptions");
	if(kvPanelSpecificOptions)
	{
		//make copy!! Data keyvalues are destroyed afterwards which causes memory leaks if assigned directly!
		kvSelf->AddSubKey(kvPanelSpecificOptions->MakeCopy());
	}

	if(m_pPrimary->GetItemCount() == 0)
	{
		bFirstPanel = true;
	}

	//get localisation of default
	wchar_t *wszDefault = vgui::localize()->Find("#GameUI_Default");
	char szDefault[32];
	localize()->ConvertUnicodeToANSI( wszDefault, szDefault, sizeof( szDefault ) );

	for (KeyValues *kvLoadedPriAssignment = m_kvLoadedAssignments->GetFirstTrueSubKey(); kvLoadedPriAssignment != NULL && !bSelfMatched; kvLoadedPriAssignment = kvLoadedPriAssignment->GetNextTrueSubKey())
	//go through loaded assignments and look for a match
	//will break out on self match
	{
		if(kvParent != NULL)
		//if it has a parent we need to match both parent and child
		{
			if(Q_stricmp(kvParent->GetName(), kvLoadedPriAssignment->GetName()) == 0)
			//if we've got a match
			{
				//go through children and look for the assignments
				for (KeyValues *kvLoadedAssignment = kvLoadedPriAssignment->GetFirstTrueSubKey(); kvLoadedAssignment != NULL; kvLoadedAssignment = kvLoadedAssignment->GetNextTrueSubKey())
				{
					if(Q_stricmp(kvSelf->GetName(), kvLoadedAssignment->GetName()) == 0)
					//if we've got a match
					{
						const char *szPosition = kvLoadedAssignment->GetString("Position", szDefault);
						const char *szPanelStyle = kvLoadedAssignment->GetString("PanelStyle", szDefault);
						const char *szItemStyle = kvLoadedAssignment->GetString("ItemStyle", szDefault);

						//The presets that were last saved for this assignment might have been deleted from the file manually 
						//so check they're still presets before doing anything...

						//if it's the string for "default" then don't bother looking
						if(Q_strcmp(szDefault, szPosition) != 0 && m_pPositionPresetsClass->GetPresetDataByName(szPosition))
						{
							kvSelf->SetString("Position", szPosition);
						}
						else
						{
							kvSelf->SetString("Position", szDefault);
						}

						//if it's the string for "default" then don't bother looking
						if(Q_strcmp(szDefault, szPanelStyle) != 0 && m_pPanelStylePresetsClass->GetPresetDataByName(szPanelStyle))
						{
							kvSelf->SetString("PanelStyle", szPanelStyle);
						}
						else
						{
							kvSelf->SetString("PanelStyle", szDefault);
						}

						//if it's the string for "default" then don't bother looking
						if(Q_strcmp(szDefault, szItemStyle) != 0 && m_pItemStylePresetsClass->GetPresetDataByName(szItemStyle))
						{
							kvSelf->SetString("ItemStyle", szItemStyle);
						}
						else
						{
							kvSelf->SetString("ItemStyle", szDefault);
						}

						KeyValues *kvPanelSpecificValues = kvLoadedAssignment->FindKey("PanelSpecificValues");

						if(kvPanelSpecificValues)
						{
							kvSelf->AddSubKey(kvPanelSpecificValues->MakeCopy());
						}

						bool bAddedToCombo = false;
						for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
						{
							if(Q_stricmp(m_pPrimary->GetItemUserData(rowIndex)->GetName(),kvParent->GetName()) == 0)
							{	
								KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);

								kvPrimary->AddSubKey(kvSelf->MakeCopy());
								
								if(m_pPrimary->GetActiveItem() == rowIndex)
									UpdateSecondaryComboFromParentKey(kvPrimary);

								bAddedToCombo = true;
								break;
							}
						}
						if(!bAddedToCombo)
						{
							m_pPrimary->AddItem(kvParent->GetString("CategoryName"), kvParent);
						}

						SendStyleDataToAssignment(kvSelf->MakeCopy());

						bSelfMatched = true;
						//stop looking for a self match
						break;
					}
				}
			}
		}
		else
		//just match self in the primary assignents.
		{
			if(Q_stricmp(kvSelf->GetName(), kvLoadedPriAssignment->GetName()) == 0)
			//if we've got a match
			{
				const char *szPosition = kvLoadedPriAssignment->GetString("Position", szDefault);
				const char *szPanelStyle = kvLoadedPriAssignment->GetString("PanelStyle", szDefault);
				const char *szItemStyle = kvLoadedPriAssignment->GetString("ItemStyle", szDefault);

				//The presets that were last saved for this assignment might have been deleted from the file manually 
				//so check they're still presets before doing anything...

				//if it's the string for "default" then don't bother looking
				if(Q_strcmp(szDefault, szPosition) != 0 && m_pPositionPresetsClass->GetPresetDataByName(szPosition))
				{
					kvSelf->SetString("Position", szPosition);
				}
				else
				{
					kvSelf->SetString("Position", szDefault);
				}

				//if it's the string for "default" then don't bother looking
				if(Q_strcmp(szDefault, szPanelStyle) != 0 && m_pPanelStylePresetsClass->GetPresetDataByName(szPanelStyle))
				{
					kvSelf->SetString("PanelStyle", kvLoadedPriAssignment->GetString(szPanelStyle));
				}
				else
				{
					kvSelf->SetString("PanelStyle", szDefault);
				}

				//if it's the string for "default" then don't bother looking
				if(Q_strcmp(szDefault, szItemStyle) != 0 && m_pItemStylePresetsClass->GetPresetDataByName(szItemStyle))
				{
					kvSelf->SetString("ItemStyle", szItemStyle);
				}
				else
				{
					kvSelf->SetString("ItemStyle", szDefault);
				}

				KeyValues *kvPanelSpecificValues = kvLoadedPriAssignment->FindKey("PanelSpecificValues");

				if(kvPanelSpecificValues)
				{
					kvSelf->AddSubKey(kvPanelSpecificValues->MakeCopy());
				}

				m_pPrimary->AddItem(kvSelf->GetString("AssignmentName"), kvSelf);

				SendStyleDataToAssignment(kvSelf->MakeCopy());
		
				bSelfMatched = true;
				break;
			}
		}
	}

	if(!bSelfMatched)
	{
		//Just use that quantity panel's defaults 
		kvSelf->SetString("Position", szDefault);
		kvSelf->SetString("PanelStyle", szDefault);
		kvSelf->SetString("ItemStyle", szDefault);

		if(kvParent != NULL)
		//if panel has a parent
		{
			bool bAddedToCombo = false;
			for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
			{
				if(Q_stricmp(m_pPrimary->GetItemUserData(rowIndex)->GetName(),kvParent->GetName()) == 0)
				{	
					KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);

					kvPrimary->AddSubKey(kvSelf->MakeCopy());
					if(m_pPrimary->GetActiveItem() == rowIndex)
						UpdateSecondaryComboFromParentKey(kvPrimary);

					bAddedToCombo = true;
					break;
				}
			}
			if(!bAddedToCombo)
				m_pPrimary->AddItem(kvParent->GetString("CategoryName"), kvParent);

		}
		else
		//it is itself an assignment
		{
			m_pPrimary->AddItem(kvSelf->GetString("AssignmentName"), kvSelf);
		}
	
		SendStyleDataToAssignment(kvSelf->MakeCopy());
	}

	if(kvParent != NULL)
	{
		kvParent->deleteThis();
		kvParent = NULL;

		//self becomes a child of parent... so we shouldn't need to deal with self aswell
	}
	else
	{
		kvSelf->deleteThis();
		kvSelf = NULL;
	}

	//if this is the first one we've added and the presets are loaded
	if(bFirstPanel)
	//select it and apply it to the controls
	{
		m_pPrimary->ActivateItemByRow(0);
		UpdateSecondaryComboFromParentKey(m_pPrimary->GetActiveItemUserData());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Catch the comboboxs changing their selection
//-----------------------------------------------------------------------------
void CFFCustomHudAssignPresets::OnUpdateCombos(KeyValues *data)
{
	if (data->GetPtr("panel") == m_pPrimary)
	{	
		KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();

		UpdateSecondaryComboFromParentKey(kvPrimary);
	}
	else if (data->GetPtr("panel") == m_pSecondary)
	{
		//this keyvalue holds the assignmets which were saved for the item we are selecting
		KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();

		ApplyAssignmentToControls(kvSecondary);
	}
	else if(data->GetPtr("panel") == m_pPositionPresets)
	{
		if(m_pSecondary->IsVisible())
		//if the assignment is in secondary
		{	
			//we have to update the assignent in the secondary combo
			//as well as the data in the primary to keep them in sync!

			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			kvSecondary->SetString("Position", m_pPositionPresets->GetActiveItemUserData()->GetName());
			
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());

			kvAssignment->SetString("Position", m_pPositionPresets->GetActiveItemUserData()->GetName());

			RegisterAssignmentChange(kvAssignment, kvPrimary);
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			kvPrimary->SetString("Position", m_pPositionPresets->GetActiveItemUserData()->GetName());

			RegisterAssignmentChange(kvPrimary);
		}
	}
	else if (data->GetPtr("panel") == m_pPanelStylePresets)
	{
		if(m_pSecondary->IsVisible())
		//if the assignment is in secondary
		{
			//we have to update the assignent in the secondary combo
			//as well as the data in the primary to keep them in sync!

			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			kvSecondary->SetString("PanelStyle", m_pPanelStylePresets->GetActiveItemUserData()->GetName());
			
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());
				
			kvAssignment->SetString("PanelStyle", m_pPanelStylePresets->GetActiveItemUserData()->GetName());

			RegisterAssignmentChange(kvAssignment, kvPrimary);
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			kvPrimary->SetString("PanelStyle", m_pPanelStylePresets->GetActiveItemUserData()->GetName());

			RegisterAssignmentChange(kvPrimary);
		}
	}
	else if (data->GetPtr("panel") == m_pItemStylePresets)
	{
		if(m_pSecondary->IsVisible())
		//if the assignment is in secondary
		{
			//we have to update the assignent in the secondary combo
			//as well as the data in the primary to keep them in sync!

			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			kvSecondary->SetString("ItemStyle", m_pItemStylePresets->GetActiveItemUserData()->GetName());
			
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());

			kvAssignment->SetString("ItemStyle", m_pItemStylePresets->GetActiveItemUserData()->GetName());

			RegisterAssignmentChange(kvAssignment, kvPrimary);	
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			kvPrimary->SetString("ItemStyle", m_pItemStylePresets->GetActiveItemUserData()->GetName());

			RegisterAssignmentChange(kvPrimary);
		}
	}
	//panel specific options
	else
	{
		ComboBox* selectedCombo = NULL;

		for(int i = m_pPanelSpecificOptions->GetChildCount() - 1; i >= 0; --i)
		//count backwards cause we're removing stuff (counting forwards will FAIL EPICLY!!)
		{
			Panel* childPanel = m_pPanelSpecificOptions->GetChild(i);
			if(data->GetPtr("panel") == childPanel)
			{
				selectedCombo = (ComboBox*)childPanel;
				break;
			}
		}

		if(!selectedCombo)
			return;
		
		KeyValues *kvPanelSpecificValues;
		const char* panelSpecificOptionName = selectedCombo->GetName();
		const char* panelSpecificOptionValue = selectedCombo->GetActiveItemUserData()->GetString("value");

		if(m_pSecondary->IsVisible())
		//if the assignment is in secondary
		{
			//we have to update the assignent in the secondary combo
			//as well as the data in the primary to keep them in sync!

			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			
			kvPanelSpecificValues = kvSecondary->FindKey("PanelSpecificValues");

			if(!kvPanelSpecificValues)
			{
				kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
				kvPanelSpecificValues->SetString(panelSpecificOptionName, panelSpecificOptionValue);
				kvSecondary->AddSubKey(kvPanelSpecificValues);
			}
			else
			{
				kvPanelSpecificValues->SetString(panelSpecificOptionName, panelSpecificOptionValue);
			}

			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());

			kvPanelSpecificValues = kvAssignment->FindKey("PanelSpecificValues");

			if(!kvPanelSpecificValues)
			{
				kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
				kvPanelSpecificValues->SetString(panelSpecificOptionName, panelSpecificOptionValue);
				kvAssignment->AddSubKey(kvPanelSpecificValues);
			}
			else
			{
				kvPanelSpecificValues->SetString(panelSpecificOptionName, panelSpecificOptionValue);
			}

			RegisterAssignmentChange(kvAssignment, kvPrimary);
		}
		else
		{
			KeyValues *kvAssignment = m_pPrimary->GetActiveItemUserData();		

			kvPanelSpecificValues = kvAssignment->FindKey("PanelSpecificValues");

			if(!kvPanelSpecificValues)
			{
				kvPanelSpecificValues = new KeyValues("PanelSpecificValues");
				kvPanelSpecificValues->SetString(panelSpecificOptionName, panelSpecificOptionValue);
				kvAssignment->AddSubKey(kvPanelSpecificValues);
			}
			else
			{
				kvPanelSpecificValues->SetString(panelSpecificOptionName, panelSpecificOptionValue);
			}

			RegisterAssignmentChange(kvAssignment);
		}
	}
}
	
//-----------------------------------------------------------------------------
// Purpose: This method gets called by all quantity panels to check whether the
// custom options pages are ready to receive knowledge of them. This in turn 
// loads all preset option pages before returning true
//-----------------------------------------------------------------------------
bool CFFCustomHudAssignPresets::IsReady()
{
	if(vgui::filesystem())
	{
		if(!m_bLoaded)
		{
			Load();
		}
		else
		{
			if(!m_pPositionPresetsClass->HasLoaded())	
				m_pPositionPresetsClass->Load();

			if(!m_pPanelStylePresetsClass->HasLoaded())	
				m_pPanelStylePresetsClass->Load();

			if(!m_pItemStylePresetsClass->HasLoaded())
				m_pItemStylePresetsClass->Load();
		}
	}

	if(!m_pPositionPresetsClass->HasLoaded())
		return false;
	if(!m_pPanelStylePresetsClass->HasLoaded())
		return false;
	if(!m_pItemStylePresetsClass->HasLoaded())
		return false;

	return true;
}
	
void CFFCustomHudAssignPresets::PositionPresetPreviewUpdated(const char* pszPresetName, KeyValues *kvPresetPreview)
{
	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				bool bPreviewMode = ( kvAssignment->GetInt("previewMode", -1) == 1 );

				if(bPreviewMode && Q_stricmp(kvAssignment->GetString("Position"), pszPresetName) == 0)
				//if update needed
				{
					SendPresetPreviewDataToAssignment(kvAssignment, kvPresetPreview->MakeCopy());
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Position"), pszPresetName) == 0)
		//if update needed
		{
			bool bPreviewMode = ( kvPrimary->GetInt("previewMode", -1) == 1 );

			if(bPreviewMode)
			{
				SendPresetPreviewDataToAssignment(kvPrimary, kvPresetPreview->MakeCopy());
			}
		}
	}
}

void CFFCustomHudAssignPresets::PositionPresetUpdated(const char* pszPresetName)
{
	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("Position"), pszPresetName) == 0)
				//if update needed
				{
					SendStyleDataToAssignment(kvAssignment);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Position"), pszPresetName) == 0)
		//if update needed
		{
			SendStyleDataToAssignment(kvPrimary);
		}
	}
}

void CFFCustomHudAssignPresets::PositionPresetRenamed(const char* pszOldPresetName, const char* pszNewPresetName)
{
	for (int rowIndex = 0; rowIndex < m_pPositionPresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pPositionPresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		KeyValues *kvPreset = m_pPositionPresets->GetItemUserData(rowIndex);
		if(Q_stricmp(itemText, pszOldPresetName) == 0)
		{
			m_pPositionPresets->RemoveActionSignalTarget(this);

			if(m_pPositionPresets->GetActiveItem() == rowIndex)
				m_pPositionPresets->SetText(pszNewPresetName);

			kvPreset->SetName(pszNewPresetName);
			m_pPositionPresets->UpdateItem(rowIndex, pszNewPresetName, kvPreset);

			m_pPositionPresets->AddActionSignalTarget(this);
			break;
		}
	}

	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("Position"), pszOldPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetString("Position", pszNewPresetName);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Position"), pszOldPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetString("Position", pszNewPresetName);
		}
	}
}
void CFFCustomHudAssignPresets::PositionPresetDeleted(const char* pszDeletedPresetName)
{
	//get localisation of default
	wchar_t *wszDefault = vgui::localize()->Find("#GameUI_Default");
	char szDefault[32];
	localize()->ConvertUnicodeToANSI( wszDefault, szDefault, sizeof( szDefault ) );

 	for (int rowIndex = 0; rowIndex < m_pPositionPresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pPositionPresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		if(Q_stricmp(itemText, pszDeletedPresetName) == 0)
		{
			bool bDeletedItemWasActive = (m_pPositionPresets->GetActiveItem() == rowIndex);

			m_pPositionPresets->RemoveActionSignalTarget(this);
			//deleting (->DeleteItem) never shifts indexes so lets do it ourselves!!
			for(int tempIndex = rowIndex; tempIndex < m_pPositionPresets->GetItemCount()-1; ++tempIndex)
			{
				char itemText[128];
				m_pPositionPresets->GetItemText(tempIndex+1, itemText, sizeof(itemText));
				KeyValues *kvPreset = m_pPositionPresets->GetItemUserData(tempIndex+1);
				m_pPositionPresets->UpdateItem(tempIndex, itemText, kvPreset);
			}
			//now delete the end one
			m_pPositionPresets->DeleteItem(m_pPositionPresets->GetItemCount()-1);

			if(bDeletedItemWasActive)
			{
				//select "Default"
				m_pPanelStylePresets->ActivateItemByRow(0);				
			}
			else if(rowIndex < m_pPositionPresets->GetItemCount())
			//if the row we deleted wasn't the last one
			{
				//select the same index
			}
			else
			{
				//select the new last one
				m_pPositionPresets->ActivateItemByRow(m_pPositionPresets->GetItemCount() - 1);
			}

			m_pPositionPresets->AddActionSignalTarget(this);

			break;
		}
	}

	//go through primary and check for assignments that were using the now deleted preset and so need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("Position"), pszDeletedPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetString("Position", szDefault);
					SendStyleDataToAssignment(kvAssignment);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Position"), pszDeletedPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetString("Position", szDefault);
			SendStyleDataToAssignment(kvPrimary);
		}
	}
}
void CFFCustomHudAssignPresets::PositionPresetAdded(const char* pszPresetName, KeyValues *kvPreset)
{
	m_pPositionPresets->AddItem(pszPresetName, kvPreset);	
	kvPreset->deleteThis();
	kvPreset = NULL;
}
	
void CFFCustomHudAssignPresets::PanelStylePresetPreviewUpdated(const char* pszPresetName, KeyValues *kvPresetPreview)
{
	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				bool bPreviewMode = ( kvAssignment->GetInt("previewMode", -1) == 1 );

				if( bPreviewMode && Q_stricmp(kvAssignment->GetString("PanelStyle"), pszPresetName) == 0 )
				//if update needed
				{
					SendPresetPreviewDataToAssignment(kvAssignment, kvPresetPreview->MakeCopy());
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("PanelStyle"), pszPresetName) == 0)
		//if update needed
		{	
			bool bPreviewMode = ( kvPrimary->GetInt("previewMode", -1) == 1 );

			if(bPreviewMode)
			{		
				SendPresetPreviewDataToAssignment(kvPrimary, kvPresetPreview->MakeCopy());
			}
		}
	}
}


void CFFCustomHudAssignPresets::PanelStylePresetUpdated(const char* pszPresetName)
{
	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("PanelStyle"), pszPresetName) == 0)
				//if update needed
				{
					SendStyleDataToAssignment(kvAssignment);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("PanelStyle"), pszPresetName) == 0)
		//if update needed
		{
			SendStyleDataToAssignment(kvPrimary);
		}
	}
}

void CFFCustomHudAssignPresets::PanelStylePresetRenamed(const char* pszOldPresetName, const char* pszNewPresetName)
{
	for (int rowIndex = 0; rowIndex < m_pPanelStylePresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pPanelStylePresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		KeyValues *kvPreset = m_pPanelStylePresets->GetItemUserData(rowIndex);
		if(Q_stricmp(itemText, pszOldPresetName) == 0)
		{
			m_pPanelStylePresets->RemoveActionSignalTarget(this);

			if(m_pPanelStylePresets->GetActiveItem() == rowIndex)
				m_pPanelStylePresets->SetText(pszNewPresetName);

			kvPreset->SetName(pszNewPresetName);
			m_pPanelStylePresets->UpdateItem(rowIndex, pszNewPresetName, NULL);
		
			m_pPanelStylePresets->AddActionSignalTarget(this);
			break;
		}
	}

	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("PanelStyle"), pszOldPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetString("PanelStyle", pszNewPresetName);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("PanelStyle"), pszOldPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetString("PanelStyle", pszNewPresetName);
		}
	}
}
void CFFCustomHudAssignPresets::PanelStylePresetDeleted(const char* pszDeletedPresetName)
{
	//get localisation of default
	wchar_t *wszDefault = vgui::localize()->Find("#GameUI_Default");
	char szDefault[32];
	localize()->ConvertUnicodeToANSI( wszDefault, szDefault, sizeof( szDefault ) );

 	for (int rowIndex = 0; rowIndex < m_pPanelStylePresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pPanelStylePresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		if(Q_stricmp(itemText, pszDeletedPresetName) == 0)
		{
			bool bDeletedItemWasActive = (m_pPanelStylePresets->GetActiveItem() == rowIndex);

			m_pPanelStylePresets->RemoveActionSignalTarget(this);
			//deleting (->DeleteItem) never shifts indexes so lets do it ourselves!!
			for(int tempIndex = rowIndex; tempIndex < m_pPanelStylePresets->GetItemCount()-1; ++tempIndex)
			{
				char itemText[128];
				m_pPanelStylePresets->GetItemText(tempIndex+1, itemText, sizeof(itemText));
				KeyValues *kvPreset = m_pPanelStylePresets->GetItemUserData(tempIndex+1);
				m_pPanelStylePresets->UpdateItem(tempIndex, itemText, kvPreset);
			}
			//now delete the end one
			m_pPanelStylePresets->DeleteItem(m_pPanelStylePresets->GetItemCount()-1);

			if(bDeletedItemWasActive)
			{
				//select "Default"
				m_pPanelStylePresets->ActivateItemByRow(0);				
			}
			else if(rowIndex < m_pPanelStylePresets->GetItemCount())
			//if the row we deleted wasn't the last one
			{
				//select the same index
				m_pPanelStylePresets->ActivateItemByRow(rowIndex);
			}
			else
			{
				//select the new last one
				m_pPanelStylePresets->ActivateItemByRow(m_pPanelStylePresets->GetItemCount() - 1);
			}

			m_pPanelStylePresets->AddActionSignalTarget(this);

			break;
		}
	}

	//go through primary and check for assignments that were using the now deleted preset and so need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("PanelStyle"), pszDeletedPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetString("PanelStyle", szDefault);
					SendStyleDataToAssignment(kvAssignment);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("PanelStyle"), pszDeletedPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetString("PanelStyle", szDefault);
			SendStyleDataToAssignment(kvPrimary);
		}
	}
}
void CFFCustomHudAssignPresets::PanelStylePresetAdded(const char* pszPresetName, KeyValues *kvPreset)
{
	m_pPanelStylePresets->AddItem(pszPresetName, kvPreset);
	kvPreset->deleteThis();
	kvPreset = NULL;
}
	
void CFFCustomHudAssignPresets::ItemStylePresetPreviewUpdated(const char* pszPresetName, KeyValues *kvPresetPreview)
{
	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				bool bPreviewMode = ( kvAssignment->GetInt("previewMode", -1) == 1 );

				if ( bPreviewMode && Q_stricmp(kvAssignment->GetString("ItemStyle"), pszPresetName) == 0)
				//if update needed
				{
					SendPresetPreviewDataToAssignment(kvAssignment, kvPresetPreview->MakeCopy());
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("ItemStyle"), pszPresetName) == 0)
		//if update needed
		{
			bool bPreviewMode = ( kvPrimary->GetInt("previewMode", -1) == 1 );

			if(bPreviewMode)
			{
				SendPresetPreviewDataToAssignment(kvPrimary, kvPresetPreview->MakeCopy());
			}
		}
	}
}
void CFFCustomHudAssignPresets::ItemStylePresetUpdated(const char* pszPresetName)
{
	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("ItemStyle"), pszPresetName) == 0)
				//if update needed
				{
					SendStyleDataToAssignment(kvAssignment);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("ItemStyle"), pszPresetName) == 0)
		//if update needed
		{
			SendStyleDataToAssignment(kvPrimary);
		}
	}
}

void CFFCustomHudAssignPresets::ItemStylePresetRenamed(const char* pszOldPresetName, const char* pszNewPresetName)
{
	for (int rowIndex = 0; rowIndex < m_pItemStylePresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pItemStylePresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		KeyValues *kvPreset = m_pItemStylePresets->GetItemUserData(rowIndex);
		if(Q_stricmp(itemText, pszOldPresetName) == 0)
		{
			m_pItemStylePresets->RemoveActionSignalTarget(this);

			if(m_pItemStylePresets->GetActiveItem() == rowIndex)
				m_pItemStylePresets->SetText(pszNewPresetName);

			kvPreset->SetName(pszNewPresetName);
			m_pItemStylePresets->UpdateItem(rowIndex, pszNewPresetName, kvPreset);
			m_pItemStylePresets->AddActionSignalTarget(this);

			break;
		}
	}

	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("ItemStyle"), pszOldPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetString("ItemStyle", pszNewPresetName);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("ItemStyle"), pszOldPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetString("ItemStyle", pszNewPresetName);
		}
	}
}
void CFFCustomHudAssignPresets::ItemStylePresetDeleted(const char* pszDeletedPresetName)
{
	//get localisation of default
	wchar_t *wszDefault = vgui::localize()->Find("#GameUI_Default");
	char szDefault[32];
	localize()->ConvertUnicodeToANSI( wszDefault, szDefault, sizeof( szDefault ) );

	for (int rowIndex = 0; rowIndex < m_pItemStylePresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pItemStylePresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		if(Q_stricmp(itemText, pszDeletedPresetName) == 0)
		{
			bool bDeletedItemWasActive = (m_pItemStylePresets->GetActiveItem() == rowIndex);

			m_pItemStylePresets->RemoveActionSignalTarget(this);
			//deleting (->DeleteItem) never shifts indexes so lets do it ourselves!!
			for(int tempIndex = rowIndex; tempIndex < m_pItemStylePresets->GetItemCount()-1; ++tempIndex)
			{
				char itemText[128];
				m_pItemStylePresets->GetItemText(tempIndex+1, itemText, sizeof(itemText));
				KeyValues *kvPreset = m_pItemStylePresets->GetItemUserData(tempIndex+1);
				m_pItemStylePresets->UpdateItem(tempIndex, itemText, kvPreset);
			}
			//now delete the end one
			m_pItemStylePresets->DeleteItem(m_pItemStylePresets->GetItemCount()-1);

			if(bDeletedItemWasActive)
			{
				//select "Default"
				m_pItemStylePresets->ActivateItemByRow(0);				
			}
			else if(rowIndex < m_pItemStylePresets->GetItemCount())
			//if the row we deleted wasn't the last one
			{
				//select the same index
				m_pItemStylePresets->ActivateItemByRow(rowIndex);
			}
			else
			{
				//select the new last one
				m_pItemStylePresets->ActivateItemByRow(m_pItemStylePresets->GetItemCount() - 1);
			}

			m_pItemStylePresets->AddActionSignalTarget(this);

			break;
		}
	}

	//go through primary and check for assignments that were using the now deleted preset and so need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("ItemStyle"), pszDeletedPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetString("ItemStyle", szDefault);
					SendStyleDataToAssignment(kvAssignment);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("ItemStyle"), pszDeletedPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetString("ItemStyle", szDefault);
			SendStyleDataToAssignment(kvPrimary);
		}
	}
}
void CFFCustomHudAssignPresets::ItemStylePresetAdded(const char* pszPresetName, KeyValues *kvPreset)
{
	m_pItemStylePresets->AddItem(pszPresetName, kvPreset);
	kvPreset->deleteThis();
	kvPreset = NULL;
}
	
void CFFCustomHudAssignPresets::OnItemStylePresetsClassLoaded()
{
	static bool bFirstLoad = true;

	if(!bFirstLoad)
		m_pPanelStylePresets->RemoveActionSignalTarget(this);

	//get localisation of default
	wchar_t *wszDefault = vgui::localize()->Find("#GameUI_Default");
	char szDefault[32];
	localize()->ConvertUnicodeToANSI( wszDefault, szDefault, sizeof( szDefault ) );

	//clear the presets dropdown
	m_pItemStylePresets->RemoveAll();
	//Add the default option
	m_pItemStylePresets->AddItem("#GameUI_Default", new KeyValues(szDefault));
	//Activate that option
	m_pItemStylePresets->ActivateItemByRow(0);

	//Get the available arrangment presets
	KeyValues *kvPresets = m_pItemStylePresetsClass->GetPresetData();

	// Now go through all the values and add them to the combobox
	for (KeyValues *kvPreset = kvPresets->GetFirstSubKey(); kvPreset != NULL; kvPreset = kvPreset->GetNextKey())
	{
		m_pItemStylePresets->AddItem(kvPreset->GetName(), kvPreset);
	}

	//remove KeyValues that is no longer needed!
	kvPresets->deleteThis();
	kvPresets = NULL;

	if(!bFirstLoad)
		m_pPanelStylePresets->AddActionSignalTarget(this);

	bFirstLoad = false;
}

void CFFCustomHudAssignPresets::OnPanelStylePresetsClassLoaded()
{
	static bool bFirstLoad = true;

	if(!bFirstLoad)
		m_pPanelStylePresets->RemoveActionSignalTarget(this);

	//get localisation of default
	wchar_t *wszDefault = vgui::localize()->Find("#GameUI_Default");
	char szDefault[32];
	localize()->ConvertUnicodeToANSI( wszDefault, szDefault, sizeof( szDefault ) );

	//clear the presets dropdown
	m_pPanelStylePresets->RemoveAll();
	//Add the default option
	m_pPanelStylePresets->AddItem("#GameUI_Default", new KeyValues(szDefault));
	//Activate that option
	m_pPanelStylePresets->ActivateItemByRow(0);

	//Get the available arrangment presets
	KeyValues *kvPresets = m_pPanelStylePresetsClass->GetPresetData();

	// Now go through all the values and add them to the combobox
	for (KeyValues *kvPreset = kvPresets->GetFirstSubKey(); kvPreset != NULL; kvPreset = kvPreset->GetNextKey())
	{
		m_pPanelStylePresets->AddItem(kvPreset->GetName(), kvPreset);
	}

	//remove KeyValues that is no longer needed!
	kvPresets->deleteThis();
	kvPresets = NULL;

	if(!bFirstLoad)
		m_pPanelStylePresets->AddActionSignalTarget(this);

	bFirstLoad = false;
}
void CFFCustomHudAssignPresets::OnPositionPresetsClassLoaded()
{
	static bool bFirstLoad = true;

	if(!bFirstLoad)
		m_pPositionPresets->RemoveActionSignalTarget(this);

	//get localisation of default
	wchar_t *wszDefault = vgui::localize()->Find("#GameUI_Default");
	char szDefault[30];
	localize()->ConvertUnicodeToANSI( wszDefault, szDefault, sizeof( szDefault ) );

	//clear the presets dropdown
	m_pPositionPresets->RemoveAll();
	//Add the default option
	m_pPositionPresets->AddItem("#GameUI_Default", new KeyValues(szDefault));
	//Activate that option
	m_pPositionPresets->ActivateItemByRow(0);

	//Get the available position presets
	KeyValues *kvPresets = m_pPositionPresetsClass->GetPresetData();

	// Now go through all the values and add them to the combobox
	for (KeyValues *kvPreset = kvPresets->GetFirstSubKey(); kvPreset != NULL; kvPreset = kvPreset->GetNextKey())
	{
		m_pPositionPresets->AddItem(kvPreset->GetName(), kvPreset);
	}

	//remove KeyValues that is no longer needed!
	kvPresets->deleteThis();
	kvPresets = NULL;

	if(!bFirstLoad)
		m_pPositionPresets->AddActionSignalTarget(this);

	bFirstLoad = false;
}