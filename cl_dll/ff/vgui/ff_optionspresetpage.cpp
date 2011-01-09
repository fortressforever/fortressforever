/********************************************************************
	created:	2010/09/02
	filename: 	cl_dll\ff\ff_optionspresetpage.cpp
	file path:	cl_dll\ff
	file base:	ff_optionspresetpage
	file ext:	cpp
	author:		Elmo
	
	purpose:	Preset editor panel (for cusomtisable hud stuff)
*********************************************************************/

#include "cbase.h"
#include "ff_optionspresetpage.h"

#include "filesystem.h"
extern IFileSystem **pFilesystem;

#include "ff_customhudoptions_assignpresets.h"
extern CFFCustomHudAssignPresets *g_AP;

namespace vgui {
	CFFOptionsPresetPage::CFFOptionsPresetPage(Panel *parent, const char *panelName, char const *pszPresetType, const char *pszSourceFile) : BaseClass(parent, panelName) {
		Q_strncpy( m_szSourceFile, pszSourceFile, 127 );
		Q_strncpy( m_szPresetTypeName, pszPresetType, 127 );

		//preset changes (new delete rename
		m_kvChanges = new KeyValues("Changes");
		//updates to changes which need to be 
		m_kvUpdates = new KeyValues("Updates");

		m_bLoaded = false;
		m_bIsLoading = false;
		m_bPresetLoading = false;
		
		char comboBoxName[128];
		Q_strncpy( comboBoxName, m_szPresetTypeName, 127 );
		Q_strncat( comboBoxName, "PresetSelectionCombo", 127, COPY_ALL_CHARACTERS );
		m_pPresets = new ComboBox(this, comboBoxName, 6, false);
		m_pNewPreset = new Button(this, "NewPresetButton", "", this, "NewPreset");
		m_pCopyPreset = new Button(this, "CopyPresetButton", "", this, "CopyPreset");
		m_pDeletePreset = new Button(this, "DeletePresetButton", "", this, "DeletePreset");
		m_pRenamePreset = new Button(this, "RenamePresetButton", "", this, "RenamePreset");
	}
	CFFOptionsPresetPage::~CFFOptionsPresetPage()
	{
		m_kvChanges->deleteThis();
		m_kvUpdates->deleteThis();
	}
	void CFFOptionsPresetPage::Load()
	{
		if(m_bIsLoading)
			return;

		m_bIsLoading = true;

		m_pPresets->RemoveAll();
		KeyValues *kvPresets = new KeyValues("Presets");
		kvPresets->LoadFromFile(*pFilesystem, m_szSourceFile);

		if(kvPresets->GetFirstSubKey() == NULL)
		//if no presets were in the file (or file not found)
		{
			KeyValues *kvPreset = new KeyValues("global");
			//use this to remove/add keys using defaults
			kvPreset = RemoveNonEssentialValues(kvPreset);
			m_pPresets->AddItem("#GameUI_Global", kvPreset);
		}
		else
		//if presets exist
		{
			for (KeyValues *kvPreset = kvPresets->GetFirstSubKey(); kvPreset != NULL; kvPreset = kvPreset->GetNextKey())
				{
			//use this to remove/add keys using defaults
				kvPreset = RemoveNonEssentialValues(kvPreset);
				if(Q_strncmp(kvPreset->GetName(), "global", 7) == 0)
				{
					m_pPresets->AddItem("#GameUI_Global", kvPreset); 
				}
				else
				{
					m_pPresets->AddItem(kvPreset->GetName(), kvPreset);
				}
			}
		}
		
		// Default to the global preset
		m_pPresets->ActivateItemByRow(0);
	
		ApplyPresetToControls(m_pPresets->GetActiveItemUserData());

		kvPresets->deleteThis();

		RegisterSelfForPresetAssignment();

		m_bIsLoading = false;
		m_bLoaded = true;
	}

	void CFFOptionsPresetPage::Reset()
	{
		m_bLoaded = false;
		Load();
	}

	void CFFOptionsPresetPage::Apply()
	{
		for (int i = 0; i < m_kvChanges->GetInt("Count",0); ++i)
		{
			char item[5];
			Q_snprintf( item, 5, "%i", i );
			KeyValues *kvCommand = m_kvChanges->FindKey(item, true);
			
			KeyValues *kvRename = kvCommand->FindKey("Rename");
			if( kvRename )
			{
				SendRenamedPresetNameToPresetAssignment(kvRename->GetString("OldName"), kvRename->GetString("NewName"));
			}
			else if( Q_stricmp(kvCommand->GetString("New"), "") != 0 )
			{
				SendNewPresetNameToPresetAssignment( kvCommand->GetString("New"), GetPresetDataByName( kvCommand->GetString("New") ) );
			}
			else if( Q_stricmp(kvCommand->GetString("Delete"), "") != 0 )
			{
				SendDeletedPresetNameToPresetAssignment(kvCommand->GetString("Delete"));
			}
		}
		m_kvChanges->Clear();

		for (KeyValues *kvPreset = m_kvUpdates->GetFirstSubKey(); kvPreset != NULL; kvPreset = kvPreset->GetNextKey())
		{
			SendUpdatedPresetNameToPresetAssignment(kvPreset->GetName());
		}
		m_kvUpdates->Clear();

		KeyValues *kvPresets = GetPresetData();
		kvPresets->SaveToFile(*pFilesystem, m_szSourceFile);	
		kvPresets->deleteThis();
	}

	bool CFFOptionsPresetPage::HasLoaded()
	{
		return m_bLoaded;
	}

	void CFFOptionsPresetPage::UpdatePresetFromControls(KeyValues *kvPreset)
	{
		if(!m_kvUpdates->FindKey(kvPreset->GetName()))
		{
			m_kvUpdates->AddSubKey(new KeyValues(kvPreset->GetName()));
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Give the keyvalue data associated with the given name
	//-----------------------------------------------------------------------------
	KeyValues *CFFOptionsPresetPage::GetPresetDataByName(char const *styleName)
	{
		for (int i = 0; i < m_pPresets->GetItemCount(); ++i)
		{
			//if name exists
			if(Q_strcmp(styleName, m_pPresets->GetItemUserData(i)->GetName()) == 0)
			{
				return m_pPresets->GetItemUserData(i)->MakeCopy();
			}
		}
		//if we got here then it wasn't found so return null
		return NULL;
	}
	//-----------------------------------------------------------------------------
	KeyValues *CFFOptionsPresetPage::GetPresetData()
	{
		KeyValues *kvPresets = new KeyValues("Presets");
		for (int i = 0; i < m_pPresets->GetItemCount(); ++i)
		{
			KeyValues *kvPreset = m_pPresets->GetItemUserData(i);

			//use a new copied keyvalue
			//crash occurred when saving twice, 
			//something to do with deleting the preset used in the combobox
			KeyValues *kvPresetToSave = new KeyValues(kvPreset->GetName());
			kvPreset->CopySubkeys(kvPresetToSave);				
			kvPresets->AddSubKey(kvPresetToSave);
		}
		return kvPresets;
	}
	//-----------------------------------------------------------------------------
	// Purpose: Catch the comboboxs changing their selection
	//-----------------------------------------------------------------------------
	void CFFOptionsPresetPage::OnUpdateCombos(KeyValues *data)
	{
		if (m_bLoaded && data->GetPtr("panel") == m_pPresets)
		{
			if(m_pPresets->GetActiveItem() == 0)
			{
				m_pDeletePreset->SetEnabled(false);
				m_pRenamePreset->SetEnabled(false);
			}
			else
			{
				m_pDeletePreset->SetEnabled(true);
				m_pRenamePreset->SetEnabled(true);
			}
			ApplyPresetToControls(m_pPresets->GetActiveItemUserData());
		}
	}
	//-----------------------------------------------------------------------------
	// Purpose: Catch name input dialog's okay button and process the text
	//-----------------------------------------------------------------------------
	void CFFOptionsPresetPage::OnInputDialogCommand(KeyValues *data)
	{
		//if name empty
		if(Q_strcmp(data->GetString("text"), "") == 0)
		{
			//tell the user what's wrong
			m_pPresetNameInputError = new MessageBox("#GameUI_ErrorPresetNameTitle","#GameUI_ErrorPresetNameEmpty", this);
			m_pPresetNameInputError->DoModal();
			//cant seem catch the close of the messagebox to reopen the input dialog
			//dont continue
			return;
		}
		
		for (int i = 0; i < m_pPresets->GetItemCount(); ++i)
		{
			//if name exists
			if(Q_strcmp(data->GetString("text"), m_pPresets->GetItemUserData(i)->GetName()) == 0)
			{
				//if renaming to the same name as it was then just continue without any error
				if(!(m_bRename && m_pPresets->GetActiveItem() == i))
				{
					//tell the user what's wrong
					m_pPresetNameInputError = new MessageBox("#GameUI_ErrorPresetNameTitle","#GameUI_ErrorPresetNameDuplicate", this);
					m_pPresetNameInputError->DoModal();
				}
				//cant seem catch the close of the messagebox to reopen the input dialog
				//dont continue
				return;
			}
		}
		if(m_bRename)
		{
			const char* szOldName = m_pPresets->GetActiveItemUserData()->GetName();

			KeyValues *kvPreset = new KeyValues(data->GetString("text"));
			m_pPresets->GetActiveItemUserData()->CopySubkeys(kvPreset);
			m_pPresets->UpdateItem(m_pPresets->GetActiveItem(), data->GetString("text"), kvPreset);
			//makes the name update in the combobox
			m_pPresets->ActivateItem(m_pPresets->GetActiveItem());
			kvPreset->deleteThis();
			
			int iCount = m_kvChanges->GetInt("Count",0);
			const char* szCount = m_kvChanges->GetString("Count","0");
			//the idea is that the current count key doesn't exist and gets created
			KeyValues *kvCommand = new KeyValues(szCount);
			m_kvChanges->SetInt("Count",iCount+1);

			KeyValues *kvRename = new KeyValues("Rename");
			kvRename->SetString("NewName", data->GetString("text"));
			kvRename->SetString("OldName", szOldName);
			kvCommand->AddSubKey(kvRename);
			m_kvChanges->AddSubKey(kvCommand);
		}
		else
		{			
			KeyValues *kvPreset = new KeyValues(data->GetString("text"));
			if(m_bCopy)
			{	
				//copy from current
				m_pPresets->GetActiveItemUserData()->CopySubkeys(kvPreset);
			}
			else
			{
				//copy from global
				m_pPresets->GetItemUserData(0)->CopySubkeys(kvPreset);
			}
			
			//add the new item to the presets dropdown
			m_pPresets->AddItem(kvPreset->GetName(), kvPreset);
			m_pPresets->ActivateItemByRow(m_pPresets->GetItemCount() - 1);
			kvPreset->deleteThis();

			int iCount = m_kvChanges->GetInt("Count",0);
			const char* szCount = m_kvChanges->GetString("Count","0");
			//the idea is that the current count key doesn't exist and gets created
			KeyValues *kvCommand = new KeyValues(szCount);
			m_kvChanges->SetInt("Count",iCount+1);
			kvCommand->SetString("New", data->GetString("text"));
			m_kvChanges->AddSubKey(kvCommand);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the buttons. New Copy Delete Rename
	//-----------------------------------------------------------------------------
	void CFFOptionsPresetPage::OnButtonCommand(KeyValues *data)
	{
		const char *pszCommand = data->GetString("command");

		if (Q_strcmp(pszCommand, "NewPreset") == 0 || Q_strcmp(pszCommand, "CopyPreset") == 0 || Q_strcmp(pszCommand, "RenamePreset") == 0)
		{
			if(Q_strcmp(pszCommand, "CopyPreset") == 0)
			{
				m_bCopy = true;
				m_bRename = false;
			}
			else if(Q_strcmp(pszCommand, "RenamePreset") == 0)
			{
				m_bCopy = false;
				m_bRename = true;
			}
			else
			{
				m_bCopy = false;
				m_bRename = false;
			}

			if(m_bRename)
				m_pPresetNameInput = new InputDialog(this, "#GameUI_EnterPresetName", "", m_pPresets->GetActiveItemUserData()->GetName());
			else
				m_pPresetNameInput = new InputDialog(this, "#GameUI_EnterPresetName", "");
			m_pPresetNameInput->DoModal();
		}
		else if (Q_strcmp(pszCommand, "DeletePreset") == 0)
		{
			int index = m_pPresets->GetActiveItem();

			int iCount = m_kvChanges->GetInt("Count",0);
			const char* szCount = m_kvChanges->GetString("Count","0");
			//the idea is that the current count key doesn't exist and gets created
			KeyValues *kvCommand = new KeyValues(szCount);
			m_kvChanges->SetInt("Count",iCount+1);
			kvCommand->SetString("Delete", m_pPresets->GetActiveItemUserData()->GetName());

			//deleting never shifts indexes so lets do it ourselves!!
			for(int tempIndex = index; tempIndex < m_pPresets->GetItemCount()-1; ++tempIndex)
			{
				m_pPresets->UpdateItem(tempIndex, m_pPresets->GetItemUserData(tempIndex+1)->GetName(), m_pPresets->GetItemUserData(tempIndex+1));
			}
			//now delete the end one
			m_pPresets->DeleteItem(m_pPresets->GetItemCount()-1);

			if(index < m_pPresets->GetItemCount())
				m_pPresets->ActivateItemByRow(index);
			else
				m_pPresets->ActivateItemByRow(m_pPresets->GetItemCount() - 1);

			m_kvChanges->AddSubKey(kvCommand);
		}
	}
};