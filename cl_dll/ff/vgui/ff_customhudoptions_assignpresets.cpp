#include "cbase.h"
#include "ff_customhudoptions_assignpresets.h"

#define PRESETASSIGNMENT_FILE "HudPresetAssignments.vdf"

using namespace vgui;

CFFCustomHudAssignPresets *g_AP = NULL;

CFFCustomHudAssignPresets::CFFCustomHudAssignPresets(Panel *parent, char const *panelName, CFFCustomHudStylePresets* stylePresetsClass, CFFCustomHudArrangementPresets* arrangementPresetsClass, CFFCustomHudPositionPresets* positionPresetsClass) : BaseClass(parent, panelName)
{
	m_bIsLoading = false;
	m_bLoaded = false;

	m_pStylePresetsClass = stylePresetsClass;
	m_pArrangementPresetsClass = arrangementPresetsClass;
	m_pPositionPresetsClass = positionPresetsClass;

	m_kvRequestedAssignments = new KeyValues("Assignments");
	m_pPrimary = new ComboBox(this, "ParentCombo", 7, false);
	m_pSecondary = new ComboBox(this, "ChildCombo", 7, false);

	m_pStylePresets = new ComboBox(this, "StyleCombo", 7, false);
	m_pArrangementPresets = new ComboBox(this, "ArrangementCombo", 7, false);
	m_pPositionPresets = new ComboBox(this, "PositionCombo", 7, false);

	m_pUseDefault = new CheckButton(this, "UseDefault", "#GameUI_UseDefault");
	m_pUseDefaultStyle = new CheckButton(this, "UseDefaultStyle", "##GameUI_UseDefault");
	m_pUseDefaultArrangement = new CheckButton(this, "UseDefaultArrangement", "##GameUI_UseDefault");
	m_pUseDefaultPosition = new CheckButton(this, "UseDefaultPosition", "##GameUI_UseDefault");

	m_pUseDefault->SetSelected(true);
	m_pUseDefaultStyle->SetSelected(true);
	m_pUseDefaultArrangement->SetSelected(true);
	m_pUseDefaultPosition->SetSelected(true);
	m_pUseDefaultStyle->SetEnabled(false);
	m_pUseDefaultArrangement->SetEnabled(false);
	m_pUseDefaultPosition->SetEnabled(false);

	m_pStylePresets->SetEnabled(false);
	m_pArrangementPresets->SetEnabled(false);
	m_pPositionPresets->SetEnabled(false);

	iYCoords = PRESETCOMOBO_Y;

	m_bAssignmentUseDefaultChanging = false;
	m_bAssignmentStyleChanging = false;
	m_bAssignmentStyleUseDefaultChanging = false;
	m_bAssignmentArrangementChanging = false;
	m_bAssignmentArrangementUseDefaultChanging = false;
	m_bAssignmentPositionChanging = false;
	m_bAssignmentPositionUseDefaultChanging = false;

	LoadControlSettings("resource/ui/FFOptionsSubCustomHudAssignPresets.res");
	g_AP = this;
}
CFFCustomHudAssignPresets::~CFFCustomHudAssignPresets()
{
	g_AP = NULL;
}
	
void CFFCustomHudAssignPresets::Load()
{
	if(m_bIsLoading)
		return;
	m_bIsLoading = true;

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
		m_kvRequiredUpdates = new KeyValues("Changes");

		if(m_pSecondary->IsVisible())
		{
			//this keyvalue holds the assignmets which were saved for the item we are selecting
			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			ApplyAssignmentToControls(kvSecondary);
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			ApplyAssignmentToControls(kvPrimary);
		}
	}
	
	m_bIsLoading = false;
}
void CFFCustomHudAssignPresets::Reset() {}
void CFFCustomHudAssignPresets::Apply() {
	//update hud elements that need updating
	for (KeyValues *kvPrimaryUpdate = m_kvRequiredUpdates->GetFirstSubKey(); kvPrimaryUpdate != NULL; kvPrimaryUpdate = kvPrimaryUpdate->GetNextKey())
	{
		if(kvPrimaryUpdate->IsEmpty())
		//then this is an assignment and not a parent
		{
			//go through primary and check for assignments which need an update
			for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
			{
				KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
				if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
				//if this is a parent node
				{
					//were not looking for a parent so continue
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
			//go through primary and check for assignments which need an update
			for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
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

	KeyValues *kvPresets = new KeyValues("PresetAssignments");
	for (int i = 0; i < m_pPrimary->GetItemCount(); ++i)
	{
		KeyValues *kvPreset = m_pPrimary->GetItemUserData(i);

		//use a new copied keyvalue
		//crash occurred when saving twice, 
		//something to do with deleting the preset used in the combobox
		KeyValues *kvPresetToSave = new KeyValues(kvPreset->GetName());
		kvPreset->CopySubkeys(kvPresetToSave);				
		kvPresets->AddSubKey(kvPresetToSave);
	}
	kvPresets->SaveToFile(vgui::filesystem(), PRESETASSIGNMENT_FILE);
	kvPresets->deleteThis();
}

	
void CFFCustomHudAssignPresets::SendStyleDataToAssignment(KeyValues *kvAssignment)
{
	KeyValues *kvPositionPresetData = m_pPositionPresetsClass->GetPresetDataByName(kvAssignment->GetString("Position", "global"));
	KeyValues *kvArrangementPresetData = m_pArrangementPresetsClass->GetPresetDataByName(kvAssignment->GetString("Arrangement", "global"));
	KeyValues *kvStylePresetData = m_pStylePresetsClass->GetPresetDataByName(kvAssignment->GetString("Style", "global"));
	
	if(kvPositionPresetData == NULL)
	//if no position data found (preset has been deleted perhaps?)
	{
		//set it to use the default position
		kvAssignment->SetInt("UseDefaultPosition", 1);
	}
	if(kvArrangementPresetData == NULL)
	//if no arrangment data found (preset has been deleted perhaps?)
	{
		//set it to use the default arrangement
		kvAssignment->SetInt("UseDefaultArrangement", 1);
	}
	if(kvStylePresetData == NULL)
	//if no style data found (preset has been deleted perhaps?)
	{
		//set it to use the default arrangement
		kvAssignment->SetInt("UseDefaultStyle", 1);
	}

	KeyValues *kvPresetData = new KeyValues("StyleData");
	KeyValues *kvChainTo = kvPresetData;
	//because copying all keys is retarded (impossible?!) 
	//we'll have to chain....
	if(kvAssignment->GetInt("UseDefault", 1) == 0)
	{
		if(kvAssignment->GetInt("UseDefaultStyle", 1) == 0)
		{
			kvChainTo->ChainKeyValue(kvStylePresetData);
			kvChainTo = kvStylePresetData;
		}
		if(kvAssignment->GetInt("UseDefaultArrangement", 1) == 0)
		{
			kvChainTo->ChainKeyValue(kvArrangementPresetData);
			kvChainTo = kvArrangementPresetData;
		}
		if(kvAssignment->GetInt("UseDefaultPosition", 1) == 0)
		{
			kvChainTo->ChainKeyValue(kvPositionPresetData);
			//kvChainTo = kvPositionPresetData;
		}
	}

	Panel* pSender = (Panel* ) kvAssignment->GetPtr("panel", NULL);
	PostMessage(pSender, kvPresetData);
}
void CFFCustomHudAssignPresets::UpdateSecondaryComboFromParentKey(KeyValues *kvPrimary)
{
	if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
	//if this is a parent node
	{	
		m_pSecondary->SetVisible(true);
		m_pSecondary->RemoveAll();
		for(KeyValues *kvSecondary = kvPrimary->GetFirstTrueSubKey(); kvSecondary != NULL; kvSecondary = kvSecondary->GetNextTrueSubKey())
		{
			m_pSecondary->AddItem(kvSecondary->GetString("DisplayText", kvSecondary->GetName()), kvSecondary);
		}
		//if theres assignments which there should be! (just in case)
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
	bool bPositionMatched = false;
	for (int i = 0; i < m_pPositionPresets->GetItemCount() && !bPositionMatched; ++i)
	{
		//if name exists
		if(Q_stricmp(kvAssignment->GetString("Position"), m_pPositionPresets->GetItemUserData(i)->GetName()) == 0)
		{
			//set matched
			bPositionMatched = true;
			//activate by row
			if(m_pPositionPresets->GetActiveItem() != i)
			{
				m_bAssignmentPositionChanging = true;
				m_pPositionPresets->ActivateItemByRow(i);
			}
			if( (kvAssignment->GetInt("UseDefaultPosition", 1) == 1) != m_pUseDefaultPosition->IsSelected() )
			{
				m_bAssignmentPositionUseDefaultChanging = true;
				m_pUseDefaultPosition->SetSelected(!m_pUseDefaultPosition->IsSelected());	
			}
			break;
		}
	}
	if(!bPositionMatched)
	//if not matched
	//TODO: strickedly speaking we shouldn't get here as assignments are fixed as we add them
	//but what if we delete presets?
	{	
		//So lets just tick the default for this preset! (No styling)
		kvAssignment->SetInt("UseDefaultPosition", 1);
		if(!m_pUseDefaultPosition->IsSelected())
		{
			m_bAssignmentPositionUseDefaultChanging = true;
			m_pUseDefaultPosition->SetSelected(true);	
		}
	}

	bool bArrangementMatched = false;
	for (int i = 0; i < m_pArrangementPresets->GetItemCount() && !bArrangementMatched; ++i)
	{
		//if name exists
		if(Q_stricmp(kvAssignment->GetString("Arrangement"), m_pArrangementPresets->GetItemUserData(i)->GetName()) == 0)
		{
			//set matched
			bArrangementMatched = true;
			//activate by row
			if(m_pArrangementPresets->GetActiveItem() != i)
			{
				m_bAssignmentArrangementChanging = true;
				m_pArrangementPresets->ActivateItemByRow(i);
			}
			if( (kvAssignment->GetInt("UseDefaultArrangement", 1) == 1) != m_pUseDefaultArrangement->IsSelected() )
			{
				m_bAssignmentArrangementUseDefaultChanging = true;
				m_pUseDefaultArrangement->SetSelected(!m_pUseDefaultArrangement->IsSelected());	
			}
			break;
		}
	}
	if(!bArrangementMatched)
	//if not matched
	//TODO: strickedly speaking we shouldn't get here as assignments are fixed as we add them
	//but what if we delete presets?
	{	
		//So lets just tick the default for this preset! (No styling)
		kvAssignment->SetInt("UseDefaultArrangement", 1);
		if(!m_pUseDefaultArrangement->IsSelected())
		{
			m_bAssignmentArrangementUseDefaultChanging = true;
			m_pUseDefaultArrangement->SetSelected(true);	
		}
	}

	bool bStyleMatched = false;
	for (int i = 0; i < m_pStylePresets->GetItemCount() && !bStyleMatched; ++i)
	{
		//if name exists
		if(Q_stricmp(kvAssignment->GetString("Style"), m_pStylePresets->GetItemUserData(i)->GetName()) == 0)
		{
			//set matched
			bStyleMatched = true;
			//activate by row
			if(m_pStylePresets->GetActiveItem() != i)
			{
				m_bAssignmentStyleChanging = true;
				m_pStylePresets->ActivateItemByRow(i);
			}
			if( (kvAssignment->GetInt("UseDefaultStyle", 1) == 1) != m_pUseDefaultStyle->IsSelected() )
			{
				m_bAssignmentStyleUseDefaultChanging = true;
				m_pUseDefaultStyle->SetSelected(!m_pUseDefaultStyle->IsSelected());	
			}
			break;
		}
	}
	if(!bStyleMatched)
	//if not matched
	//TODO: strickedly speaking we shouldn't get here as assignments are fixed as we add them
	//but what if we delete presets?
	{	
		//So lets just tick the default for this preset! (No styling)
		kvAssignment->SetInt("UseDefaultStyle", 1);
		if(!m_pUseDefaultStyle->IsSelected())
		{
			m_bAssignmentStyleUseDefaultChanging = true;
			m_pUseDefaultStyle->SetSelected(true);	
		}
	}

	if( (kvAssignment->GetInt("UseDefault", 1) == 1) != m_pUseDefault->IsSelected() )
	{
		m_bAssignmentUseDefaultChanging = true;
		m_pUseDefault->SetSelected(!m_pUseDefault->IsSelected());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Catch the comboboxs changing their selection
//-----------------------------------------------------------------------------
void CFFCustomHudAssignPresets::OnUpdateCombos(KeyValues *data)
{
	if(m_pPrimary->GetItemCount() > 0 && m_bLoaded )
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
		else if (data->GetPtr("panel") == m_pPositionPresets)
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

				if(m_bAssignmentPositionChanging)
					m_bAssignmentPositionChanging = false;
				else
					RegisterAssignmentChange(kvAssignment, kvPrimary);
			}
			else
			{
				KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
				kvPrimary->SetString("Position", m_pPositionPresets->GetActiveItemUserData()->GetName());

				if(m_bAssignmentPositionChanging)
					m_bAssignmentPositionChanging = false;
				else
					RegisterAssignmentChange(kvPrimary);
			}
		}
		else if (data->GetPtr("panel") == m_pArrangementPresets)
		{
			if(m_pSecondary->IsVisible())
			//if the assignment is in secondary
			{
				//we have to update the assignent in the secondary combo
				//as well as the data in the primary to keep them in sync!

				KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
				kvSecondary->SetString("Arrangement", m_pArrangementPresets->GetActiveItemUserData()->GetName());
				
				KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
				KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());
					
				kvAssignment->SetString("Arrangement", m_pArrangementPresets->GetActiveItemUserData()->GetName());

				if(m_bAssignmentArrangementChanging)
					m_bAssignmentArrangementChanging = false;
				else
					RegisterAssignmentChange(kvAssignment, kvPrimary);
			}
			else
			{
				KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
				kvPrimary->SetString("Arrangement", m_pArrangementPresets->GetActiveItemUserData()->GetName());

				if(m_bAssignmentArrangementChanging)
					m_bAssignmentArrangementChanging = false;
				else
					RegisterAssignmentChange(kvPrimary);
			}
		}
		else if (data->GetPtr("panel") == m_pStylePresets)
		{
			if(m_pSecondary->IsVisible())
			//if the assignment is in secondary
			{
				//we have to update the assignent in the secondary combo
				//as well as the data in the primary to keep them in sync!

				KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
				kvSecondary->SetString("Style", m_pStylePresets->GetActiveItemUserData()->GetName());
				
				KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
				KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());

				kvAssignment->SetString("Style", m_pStylePresets->GetActiveItemUserData()->GetName());

				if(m_bAssignmentStyleChanging)
					m_bAssignmentStyleChanging = false;
				else
					RegisterAssignmentChange(kvAssignment, kvPrimary);	
			}
			else
			{
				KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
				kvPrimary->SetString("Style", m_pStylePresets->GetActiveItemUserData()->GetName());

				if(m_bAssignmentStyleChanging)
					m_bAssignmentStyleChanging = false;
				else
					RegisterAssignmentChange(kvPrimary);
			}
		} 
	}
}

void CFFCustomHudAssignPresets::RegisterAssignmentChange(KeyValues *kvSelf, KeyValues *kvParent)
{
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

//-----------------------------------------------------------------------------
// Purpose: Catch the checkboxes changing their state
//-----------------------------------------------------------------------------
void CFFCustomHudAssignPresets::OnUpdateCheckbox(KeyValues *data)
{
	if(m_bLoaded && data->GetPtr("panel") == m_pUseDefault)
	{
		if(m_pSecondary->IsVisible())
		//if the assignment is in secondary
		{	
			//we have to update the assignent in the secondary combo
			//as well as the data in the primary to keep them in sync!

			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			
			if(m_pUseDefault->IsSelected())
				kvSecondary->SetInt("UseDefault", 1);
			else
				kvSecondary->SetInt("UseDefault", 0);
			
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());
				
			if(m_pUseDefault->IsSelected())
				kvAssignment->SetInt("UseDefault", 1);
			else
				kvAssignment->SetInt("UseDefault", 0);

			if(m_bAssignmentUseDefaultChanging)
				m_bAssignmentUseDefaultChanging = false;
			else
				RegisterAssignmentChange(kvAssignment, kvPrimary);
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			if(m_pUseDefault->IsSelected())
				kvPrimary->SetInt("UseDefault", 1);
			else
				kvPrimary->SetInt("UseDefault", 0);
			
			if(m_bAssignmentUseDefaultChanging)
				m_bAssignmentUseDefaultChanging = false;
			else
				RegisterAssignmentChange(kvPrimary);
		}

		if(m_pUseDefault->IsSelected())
		{
			m_pPositionPresets->SetEnabled(false);
			m_pArrangementPresets->SetEnabled(false);
			m_pStylePresets->SetEnabled(false);
			m_pUseDefaultPosition->SetEnabled(false);
			m_pUseDefaultArrangement->SetEnabled(false);
			m_pUseDefaultStyle->SetEnabled(false);
		}
		else
		{
			m_pUseDefaultPosition->SetEnabled(true);
			m_pUseDefaultArrangement->SetEnabled(true);
			m_pUseDefaultStyle->SetEnabled(true);

			if(m_pUseDefaultPosition->IsSelected())
			{
				m_pPositionPresets->SetEnabled(false);
			}
			else
			{
				m_pPositionPresets->SetEnabled(true);
			}

			if(m_pUseDefaultArrangement->IsSelected())
			{
				m_pArrangementPresets->SetEnabled(false);
			}
			else
			{
				m_pArrangementPresets->SetEnabled(true);
			}

			if(m_pUseDefaultStyle->IsSelected())
			{
				m_pStylePresets->SetEnabled(false);
			}
			else
			{
				m_pStylePresets->SetEnabled(true);
			}
		}
	}
	else if(m_bLoaded && data->GetPtr("panel") == m_pUseDefaultPosition)
	{
		if(m_pSecondary->IsVisible())
		//if the assignment is in secondary
		{	
			//we have to update the assignent in the secondary combo
			//as well as the data in the primary to keep them in sync!

			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			
			if(m_pUseDefaultPosition->IsSelected())
			{
				kvSecondary->SetInt("UseDefaultPosition", 1);
			}
			else
			{
				kvSecondary->SetInt("UseDefaultPosition", 0);
			}
			
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());
				
			if(m_pUseDefaultPosition->IsSelected())
			{
				kvAssignment->SetInt("UseDefaultPosition", 1);
			}
			else
			{
				kvAssignment->SetInt("UseDefaultPosition", 0);
			}

			if(m_bAssignmentPositionUseDefaultChanging)
				m_bAssignmentPositionUseDefaultChanging = false;
			else
				RegisterAssignmentChange(kvAssignment, kvPrimary);
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			if(m_pUseDefaultPosition->IsSelected())
			{
				kvPrimary->SetInt("UseDefaultPosition", 1);
			}
			else
			{
				kvPrimary->SetInt("UseDefaultPosition", 0);
			}

			if(m_bAssignmentPositionUseDefaultChanging)
				m_bAssignmentPositionUseDefaultChanging = false;
			else
				RegisterAssignmentChange(kvPrimary);
		}

		//we don't want to enable the preset dropdown if usedefault is true
		if(!(m_pUseDefault->IsSelected() && !m_pUseDefaultPosition->IsSelected()))
			m_pPositionPresets->SetEnabled(!m_pUseDefaultPosition->IsSelected());
	}
	else if(m_bLoaded && data->GetPtr("panel") == m_pUseDefaultArrangement)
	{
		if(m_pSecondary->IsVisible())
		//if the assignment is in secondary
		{	
			//we have to update the assignent in the secondary combo
			//as well as the data in the primary to keep them in sync!
			
			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			
			if(m_pUseDefaultArrangement->IsSelected())
			{
				kvSecondary->SetInt("UseDefaultArrangement", 1);
			}
			else
			{
				kvSecondary->SetInt("UseDefaultArrangement", 0);
			}
			
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());
				
			if(m_pUseDefaultArrangement->IsSelected())
			{
				kvAssignment->SetInt("UseDefaultArrangement", 1);
			}
			else
			{
				kvAssignment->SetInt("UseDefaultArrangement", 0);
			}
			
			if(m_bAssignmentArrangementUseDefaultChanging)
				m_bAssignmentArrangementUseDefaultChanging = false;
			else
				RegisterAssignmentChange(kvAssignment, kvPrimary);
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			if(m_pUseDefaultArrangement->IsSelected())
			{
				kvPrimary->SetInt("UseDefaultArrangement", 1);
			}
			else
			{
				kvPrimary->SetInt("UseDefaultArrangement", 0);
			}

			if(m_bAssignmentArrangementUseDefaultChanging)
				m_bAssignmentArrangementUseDefaultChanging = false;
			else
				RegisterAssignmentChange(kvPrimary);
		}

		//we don't want to enable the preset dropdown if usedefault is true
		if(!(m_pUseDefault->IsSelected() && !m_pUseDefaultArrangement->IsSelected()))
			m_pArrangementPresets->SetEnabled(!m_pUseDefaultArrangement->IsSelected());
	}
	else if(m_bLoaded && data->GetPtr("panel") == m_pUseDefaultStyle)
	{
		if(m_pSecondary->IsVisible())
		//if the assignment is in secondary
		{	
			//we have to update the assignent in the secondary combo
			//as well as the data in the primary to keep them in sync!

			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			
			if(m_pUseDefaultStyle->IsSelected())
			{
				kvSecondary->SetInt("UseDefaultStyle", 1);
			}
			else
			{
				kvSecondary->SetInt("UseDefaultStyle", 0);
			}
			
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			KeyValues *kvAssignment = kvPrimary->FindKey(kvSecondary->GetName());
				
			if(m_pUseDefaultStyle->IsSelected())
			{
				kvAssignment->SetInt("UseDefaultStyle", 1);
			}
			else
			{
				kvAssignment->SetInt("UseDefaultStyle", 0);
			}

			if(m_bAssignmentStyleUseDefaultChanging)
				m_bAssignmentStyleUseDefaultChanging = false;
			else
				RegisterAssignmentChange(kvAssignment, kvPrimary);

		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			if(m_pUseDefaultStyle->IsSelected())
			{
				kvPrimary->SetInt("UseDefaultStyle", 1);
			}
			else
			{
				kvPrimary->SetInt("UseDefaultStyle", 0);
			}

			if(m_bAssignmentStyleUseDefaultChanging)
				m_bAssignmentStyleUseDefaultChanging = false;
			else
				RegisterAssignmentChange(kvPrimary);
		}

		//we don't want to enable the preset dropdown if usedefault is true
		if(!(m_pUseDefault->IsSelected() && !m_pUseDefaultStyle->IsSelected()))
			m_pStylePresets->SetEnabled(!m_pUseDefaultStyle->IsSelected());
	}
}
	
//-----------------------------------------------------------------------------
// Purpose: Catch quantity panels requesting the ability of customisation
//-----------------------------------------------------------------------------
void CFFCustomHudAssignPresets::OnAddQuantityPanel(KeyValues *data)
{
	//before we get here the quantity panel should have checked IsReady()
	if(!IsReady())
		return;

	//bool bParentMatched = false;
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
		kvParent->SetString("displayText", data->GetString("parentText", "Report-FF-Bug"));
		kvSelf = new KeyValues(data->GetString("selfName", "Report-FF-Bug"));
		kvSelf->SetString("displayText", data->GetString("selfText", "Report-FF-Bug"));
		kvSelf->SetPtr("panel", pSender);
		kvParent->AddSubKey(kvSelf);
	}
	else
	//if its a quantity panel with no other ties
	//such as speedometer, potentially health and armour readouts?!
	{
		kvSelf = new KeyValues(data->GetString("selfName", "Report-FF-Bug"));
		kvSelf->SetString("displayText", data->GetString("selfText", "Report-FF-Bug"));
		kvSelf->SetPtr("panel", pSender);
		kvParent = NULL;
	}

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
						//send style data to the panel
						kvSelf->SetString("Position", kvLoadedAssignment->GetString("Position"));
						kvSelf->SetString("Arrangement", kvLoadedAssignment->GetString("Arrangement"));
						kvSelf->SetString("Style", kvLoadedAssignment->GetString("Style"));
						
						kvSelf->SetInt("UseDefault",kvLoadedAssignment->GetInt("UseDefault", 1));
						kvSelf->SetInt("UseDefaultPosition",kvLoadedAssignment->GetInt("UseDefaultPosition", 1));
						kvSelf->SetInt("UseDefaultArrangement",kvLoadedAssignment->GetInt("UseDefaultArrangement", 1));
						kvSelf->SetInt("UseDefaultStyle",kvLoadedAssignment->GetInt("UseDefaultStyle", 1));

						SendStyleDataToAssignment(kvSelf);

						bool bAddedToCombo = false;
						for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
						{
							if(Q_stricmp(m_pPrimary->GetItemUserData(rowIndex)->GetName(),kvParent->GetName()) == 0)
							{								
								bAddedToCombo = true;
								KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
								kvPrimary->AddSubKey(kvSelf);
								
								if(m_pPrimary->GetActiveItem() == rowIndex)
									UpdateSecondaryComboFromParentKey(kvPrimary);
								break;
							}
						}
						if(!bAddedToCombo)
							m_pPrimary->AddItem(kvParent->GetString("displayText"), kvParent);

						bSelfMatched = true;
						//stop looking for a self match
						break;
					}
				}
			}
		}
		else
		{
			if(Q_stricmp(kvSelf->GetName(), kvLoadedPriAssignment->GetName()) == 0)
			//if we've got a match
			{
				kvSelf->SetString("Position", kvLoadedPriAssignment->GetString("Position"));
				kvSelf->SetString("Arrangement", kvLoadedPriAssignment->GetString("Arrangement"));
				kvSelf->SetString("Style", kvLoadedPriAssignment->GetString("Style"));
				
				kvSelf->SetInt("UseDefault", kvLoadedPriAssignment->GetInt("UseDefault", 1));
				kvSelf->SetInt("UseDefaultPosition", kvLoadedPriAssignment->GetInt("UseDefaultPosition", 1));
				kvSelf->SetInt("UseDefaultArrangement", kvLoadedPriAssignment->GetInt("UseDefaultArrangement", 1));
				kvSelf->SetInt("UseDefaultStyle", kvLoadedPriAssignment->GetInt("UseDefaultStyle", 1));

				SendStyleDataToAssignment(kvSelf);
				
				m_pPrimary->AddItem(kvSelf->GetString("displayText"), kvSelf);
		
				bSelfMatched = true;
			}
		}
		if(bSelfMatched)
			break;
	}

	if(!bSelfMatched)
	{
		//should always have global (but just in case)
		if(m_pPositionPresets->GetItemCount() > 0)
			kvSelf->SetString("Position",  m_pPositionPresets->GetItemUserData(0)->GetName());

		//should always have global (but just in case)
		if(m_pArrangementPresets->GetItemCount() > 0)
			kvSelf->SetString("Arrangement",  m_pArrangementPresets->GetItemUserData(0)->GetName());

		//should always have global (but just in case)
		if(m_pStylePresets->GetItemCount() > 0)
			kvSelf->SetString("Style", m_pStylePresets->GetItemUserData(0)->GetName());

		kvSelf->SetInt("UseDefault", 1);
		kvSelf->SetInt("UseDefaultPosition", 1);
		kvSelf->SetInt("UseDefaultArrangement", 1);
		kvSelf->SetInt("UseDefaultStyle", 1);

		if(kvParent != NULL)
		//if panel has a parent
		{
			bool bAddedToCombo = false;
			for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
			{
				if(Q_stricmp(m_pPrimary->GetItemUserData(rowIndex)->GetName(),kvParent->GetName()) == 0)
				{								
					bAddedToCombo = true;
					KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
					kvPrimary->AddSubKey(kvSelf);
					
					if(m_pPrimary->GetActiveItem() == rowIndex)
						UpdateSecondaryComboFromParentKey(kvPrimary);
					break;
				}
			}
			if(!bAddedToCombo)
				m_pPrimary->AddItem(kvParent->GetString("displayText"), kvParent);

		}
		else
		{
			m_pPrimary->AddItem(kvSelf->GetString("displayText"), kvSelf);
		}
	
		SendStyleDataToAssignment(kvSelf);
	}

	if(m_pPrimary->GetItemCount() == 1)
	{
		m_pPrimary->ActivateItemByRow(0);
		UpdateSecondaryComboFromParentKey(m_pPrimary->GetActiveItemUserData());
	}
}

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
			if(!m_pArrangementPresetsClass->HasLoaded())	
				m_pArrangementPresetsClass->Load();
			if(!m_pStylePresetsClass->HasLoaded())
				m_pStylePresetsClass->Load();
		}
	}

	if(!m_pPositionPresetsClass->HasLoaded())
		return false;
	if(!m_pArrangementPresetsClass->HasLoaded())
		return false;
	if(!m_pStylePresetsClass->HasLoaded())
		return false;

	return true;
}
	
void CFFCustomHudAssignPresets::PositionPresetUpdated(const char* pszPresetName)
{
	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				//REMOVE: DevMsg("%s : %s == %s\n", kvAssignment->GetName(), kvAssignment->GetString("Position", "fail"), pszPresetName);
				if(Q_stricmp(kvAssignment->GetString("Position"), pszPresetName) == 0 && kvAssignment->GetInt("UseDefaultPosition") == 0 && kvAssignment->GetInt("UseDefault") == 0)
				//if update needed
				{
					SendStyleDataToAssignment(kvAssignment);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Position"), pszPresetName) == 0 && kvPrimary->GetInt("UseDefaultPosition") == 0 && kvPrimary->GetInt("UseDefault") == 0)
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
		if(Q_stricmp(itemText, pszOldPresetName) == 0)
		{
			m_pPositionPresets->UpdateItem(rowIndex,pszNewPresetName,NULL);
			break;
		}
	}

	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				//REMOVE: DevMsg("%s : %s == %s\n", kvAssignment->GetName(), kvAssignment->GetString("Position", "fail"), pszPresetName);
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
 	for (int rowIndex = 0; rowIndex < m_pPositionPresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pPositionPresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		if(Q_stricmp(itemText, pszDeletedPresetName) == 0)
		{
			//deleting (->DeleteItem) never shifts indexes so lets do it ourselves!!
			for(int tempIndex = rowIndex; tempIndex < m_pPositionPresets->GetItemCount()-1; ++tempIndex)
			{
				char itemText[128];
				m_pPositionPresets->GetItemText(tempIndex+1, itemText, sizeof(itemText));
				m_pPositionPresets->UpdateItem(tempIndex, itemText, NULL);
			}
			//now delete the end one
			m_pPositionPresets->DeleteItem(m_pPositionPresets->GetItemCount()-1);

			if(rowIndex < m_pPositionPresets->GetItemCount())
			//if the row we deleted wasn't the last one
			{
				//select the same index
				m_pPositionPresets->ActivateItemByRow(rowIndex);
			}
			else
			{
				//select the new last one
				m_pPositionPresets->ActivateItemByRow(m_pPositionPresets->GetItemCount() - 1);
			}
			break;
		}
	}

	//go through primary and check for assignments that were using the deleted preset which now need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				//REMOVE: DevMsg("%s : %s == %s\n", kvAssignment->GetName(), kvAssignment->GetString("Position", "fail"), pszPresetName);
				if(Q_stricmp(kvAssignment->GetString("Position"), pszDeletedPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetInt("UseDefaultPosition",1);
					kvAssignment->SetString("Position", m_pPositionPresets->GetItemUserData(0)->GetName());
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Position"), pszDeletedPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetInt("UseDefaultPosition",1);
			kvPrimary->SetString("Position", m_pPositionPresets->GetItemUserData(0)->GetName());
		}
	}	
}
void CFFCustomHudAssignPresets::PositionPresetAdded(const char* pszPresetName, KeyValues *kvPreset)
{
	m_pPositionPresets->AddItem(pszPresetName, kvPreset);	
}
	
void CFFCustomHudAssignPresets::ArrangementPresetUpdated(const char* pszPresetName)
{
	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				//REMOVE: DevMsg("%s : %s == %s\n", kvAssignment->GetName(), kvAssignment->GetString("Arrangement", "fail"), pszPresetName);
				if(Q_stricmp(kvAssignment->GetString("Arrangement"), pszPresetName) == 0 && kvAssignment->GetInt("UseDefaultArrangement") == 0 && kvAssignment->GetInt("UseDefault") == 0)
				//if update needed
				{
					SendStyleDataToAssignment(kvAssignment);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Arrangement"), pszPresetName) == 0 && kvPrimary->GetInt("UseDefaultArrangement") == 0 && kvPrimary->GetInt("UseDefault") == 0)
		//if update needed
		{
			SendStyleDataToAssignment(kvPrimary);
		}
	}
}

void CFFCustomHudAssignPresets::ArrangementPresetRenamed(const char* pszOldPresetName, const char* pszNewPresetName)
{
	for (int rowIndex = 0; rowIndex < m_pArrangementPresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pArrangementPresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		if(Q_stricmp(itemText, pszOldPresetName) == 0)
		{
			m_pArrangementPresets->UpdateItem(rowIndex,pszNewPresetName,NULL);
			break;
		}
	}

	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				//REMOVE: DevMsg("%s : %s == %s\n", kvAssignment->GetName(), kvAssignment->GetString("Arrangement", "fail"), pszPresetName);
				if(Q_stricmp(kvAssignment->GetString("Arrangement"), pszOldPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetString("Arrangement", pszNewPresetName);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Arrangement"), pszOldPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetString("Arrangement", pszNewPresetName);
		}
	}
}
void CFFCustomHudAssignPresets::ArrangementPresetDeleted(const char* pszDeletedPresetName)
{
 	for (int rowIndex = 0; rowIndex < m_pArrangementPresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pArrangementPresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		if(Q_stricmp(itemText, pszDeletedPresetName) == 0)
		{
			//deleting (->DeleteItem) never shifts indexes so lets do it ourselves!!
			for(int tempIndex = rowIndex; tempIndex < m_pArrangementPresets->GetItemCount()-1; ++tempIndex)
			{
				char itemText[128];
				m_pArrangementPresets->GetItemText(tempIndex+1, itemText, sizeof(itemText));
				m_pArrangementPresets->UpdateItem(tempIndex, itemText, NULL);
			}
			//now delete the end one
			m_pArrangementPresets->DeleteItem(m_pArrangementPresets->GetItemCount()-1);

			if(rowIndex < m_pArrangementPresets->GetItemCount())
			//if the row we deleted wasn't the last one
			{
				//select the same index
				m_pArrangementPresets->ActivateItemByRow(rowIndex);
			}
			else
			{
				//select the new last one
				m_pArrangementPresets->ActivateItemByRow(m_pArrangementPresets->GetItemCount() - 1);
			}
			break;
		}
	}

	//go through primary and check for assignments that were using the deleted preset which now need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				//REMOVE: DevMsg("%s : %s == %s\n", kvAssignment->GetName(), kvAssignment->GetString("Arrangement", "fail"), pszPresetName);
				if(Q_stricmp(kvAssignment->GetString("Arrangement"), pszDeletedPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetInt("UseDefaultArrangement",1);
					kvAssignment->SetString("Arrangement", m_pArrangementPresets->GetItemUserData(0)->GetName());
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Arrangement"), pszDeletedPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetInt("UseDefaultArrangement",1);
			kvPrimary->SetString("Arrangement", m_pArrangementPresets->GetItemUserData(0)->GetName());
		}
	}	
}
void CFFCustomHudAssignPresets::ArrangementPresetAdded(const char* pszPresetName, KeyValues *kvPreset)
{
	m_pArrangementPresets->AddItem(pszPresetName, kvPreset);
}
	
void CFFCustomHudAssignPresets::StylePresetUpdated(const char* pszPresetName)
{
	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("Style"), pszPresetName) == 0 && kvAssignment->GetInt("UseDefaultStyle") == 0 && kvAssignment->GetInt("UseDefault") == 0)
				//if update needed
				{
					SendStyleDataToAssignment(kvAssignment);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Style"), pszPresetName) == 0 && kvPrimary->GetInt("UseDefaultStyle") == 0 && kvPrimary->GetInt("UseDefault") == 0)
		//if update needed
		{
			SendStyleDataToAssignment(kvPrimary);
		}
	}
}
void CFFCustomHudAssignPresets::StylePresetRenamed(const char* pszOldPresetName, const char* pszNewPresetName)
{
	for (int rowIndex = 0; rowIndex < m_pStylePresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pStylePresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		if(Q_stricmp(itemText, pszOldPresetName) == 0)
		{
			m_pStylePresets->UpdateItem(rowIndex, pszNewPresetName, NULL);
			break;
		}
	}

	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				if(Q_stricmp(kvAssignment->GetString("Style"), pszOldPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetString("Style", pszNewPresetName);
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Style"), pszOldPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetString("Style", pszNewPresetName);
		}
	}
}
void CFFCustomHudAssignPresets::StylePresetDeleted(const char* pszDeletedPresetName)
{
	for (int rowIndex = 0; rowIndex < m_pStylePresets->GetItemCount(); ++rowIndex)
	{
		//if name exists
		char itemText[128];
		m_pStylePresets->GetItemText(rowIndex, itemText, sizeof(itemText));
		if(Q_stricmp(itemText, pszDeletedPresetName) == 0)
		{
			//deleting (->DeleteItem) never shifts indexes so lets do it ourselves!!
			for(int tempIndex = rowIndex; tempIndex < m_pStylePresets->GetItemCount()-1; ++tempIndex)
			{
				char itemText[128];
				m_pStylePresets->GetItemText(tempIndex+1, itemText, sizeof(itemText));
				m_pStylePresets->UpdateItem(tempIndex, itemText, NULL);
			}
			//now delete the end one
			m_pStylePresets->DeleteItem(m_pStylePresets->GetItemCount()-1);

			if(rowIndex < m_pStylePresets->GetItemCount())
			//if the row we deleted wasn't the last one
			{
				//select the same index
				m_pStylePresets->ActivateItemByRow(rowIndex);
			}
			else
			{
				//select the new last one
				m_pStylePresets->ActivateItemByRow(m_pStylePresets->GetItemCount() - 1);
			}
			break;
		}
	}

	//go through primary and check for assignments which need an update
	for (int rowIndex = 0; rowIndex < m_pPrimary->GetItemCount(); ++rowIndex)
	{
		KeyValues *kvPrimary = m_pPrimary->GetItemUserData(rowIndex);
		if(Q_stricmp(kvPrimary->GetString("UseDefault"), "") == 0)
		//if this is a parent node
		{
			for(KeyValues *kvAssignment = kvPrimary->GetFirstTrueSubKey(); kvAssignment != NULL; kvAssignment = kvAssignment->GetNextTrueSubKey())
			//go through assignments within the parent node
			{
				//REMOVE: DevMsg("%s : %s == %s\n", kvAssignment->GetName(), kvAssignment->GetString("Arrangement", "fail"), pszPresetName);
				if(Q_stricmp(kvAssignment->GetString("Style"), pszDeletedPresetName) == 0)
				//if update needed
				{
					kvAssignment->SetInt("UseDefaultStyle",1);
					kvAssignment->SetString("Style", m_pStylePresets->GetItemUserData(0)->GetName());
				}
			}
		}
		else if(Q_stricmp(kvPrimary->GetString("Style"), pszDeletedPresetName) == 0)
		//if update needed
		{
			kvPrimary->SetInt("UseDefaultStyle",1);
			kvPrimary->SetString("Style", m_pStylePresets->GetItemUserData(0)->GetName());
		}
	}	
}
void CFFCustomHudAssignPresets::StylePresetAdded(const char* pszPresetName, KeyValues *kvPreset)
{
	m_pStylePresets->AddItem(pszPresetName, kvPreset);
	kvPreset->deleteThis();
}
	
void CFFCustomHudAssignPresets::OnStylePresetsClassLoaded()
{
	m_pStylePresets->RemoveAll();
	KeyValues *kvPresets = m_pStylePresetsClass->GetPresetData();

	// Now go through all the values and add them to the combobox
	for (KeyValues *kvPreset = kvPresets->GetFirstSubKey(); kvPreset != NULL; kvPreset = kvPreset->GetNextKey())
	{
		if(Q_stricmp(kvPreset->GetName(), "global") == 0)
		{
			m_pStylePresets->AddItem("#GameUI_Global", kvPreset); 
		}
		else
		{
			m_pStylePresets->AddItem(kvPreset->GetName(), kvPreset);
		}
	}

	m_pStylePresets->ActivateItemByRow(0);

	if(m_pPrimary->GetItemCount() > 0 && IsReady())
	{
		if(m_pSecondary->IsVisible())
		{
			//this keyvalue holds the assignmets which were saved for the item we are selecting
			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			ApplyAssignmentToControls(kvSecondary);
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			ApplyAssignmentToControls(kvPrimary);
		}
	}
}

void CFFCustomHudAssignPresets::OnArrangementPresetsClassLoaded()
{
	m_pArrangementPresets->RemoveAll();
	KeyValues *kvPresets = m_pArrangementPresetsClass->GetPresetData();
	// Now go through all the values and add them to the combobox
	for (KeyValues *kvPreset = kvPresets->GetFirstSubKey(); kvPreset != NULL; kvPreset = kvPreset->GetNextKey())
	{
		if(Q_stricmp(kvPreset->GetName(), "global") == 0)
		{
			m_pArrangementPresets->AddItem("#GameUI_Global", kvPreset); 
		}
		else
		{
			m_pArrangementPresets->AddItem(kvPreset->GetName(), kvPreset);
		}
	}
	m_pArrangementPresets->ActivateItemByRow(0);

	if(m_pPrimary->GetItemCount() > 0 && IsReady())
	{
		if(m_pSecondary->IsVisible())
		{
			//this keyvalue holds the assignmets which were saved for the item we are selecting
			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			ApplyAssignmentToControls(kvSecondary);
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			ApplyAssignmentToControls(kvPrimary);
		}
	}
}
void CFFCustomHudAssignPresets::OnPositionPresetsClassLoaded()
{
	m_pPositionPresets->RemoveAll();
	KeyValues *kvPresets = m_pPositionPresetsClass->GetPresetData();
	// Now go through all the values and add them to the combobox
	for (KeyValues *kvPreset = kvPresets->GetFirstSubKey(); kvPreset != NULL; kvPreset = kvPreset->GetNextKey())
	{
		if(Q_stricmp(kvPreset->GetName(), "global") == 0)
		{
			m_pPositionPresets->AddItem("#GameUI_Global", kvPreset); 
		}
		else
		{
			m_pPositionPresets->AddItem(kvPreset->GetName(), kvPreset);
		}
	}
	m_pPositionPresets->ActivateItemByRow(0);

	if(m_pPrimary->GetItemCount() > 0 && IsReady())
	{
		if(m_pSecondary->IsVisible())
		{
			//this keyvalue holds the assignmets which were saved for the item we are selecting
			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
			ApplyAssignmentToControls(kvSecondary);
		}
		else
		{
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();
			ApplyAssignmentToControls(kvPrimary);
		}
	}
}