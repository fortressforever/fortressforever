#include "cbase.h"
#include "ff_customhudoptions_newpresetdialog.h"

#include "ff_customhudoptions.h"

extern CFFCustomHudAssignPresets *g_AP;

namespace vgui
{
	CFFCustomHudNewPresetDialog::CFFCustomHudNewPresetDialog( Panel *parent, const char* panelName ) : BaseClass(parent, panelName )
	{
		m_pPrimary = new ComboBox(this, "ParentCombo", 7, false);
		m_pSecondary = new ComboBox(this, "ChildCombo", 7, false);

		m_pName = new TextEntry(this, "NameEntry");

		KeyValues *kvPresets;
		
		if(g_AP != NULL)
		{
			kvPresets = g_AP->GetPresetAssignments();
			
			for(KeyValues *kvPrimary = kvPresets->GetFirstTrueSubKey(); kvPrimary != NULL; kvPrimary = kvPrimary->GetNextTrueSubKey())
			{
				if(Q_stricmp(kvPrimary->GetString("CategoryName"), "") != 0)
				//if this is a parent node
				{
					m_pPrimary->AddItem(kvPrimary->GetString("CategoryName"), kvPrimary);
				}
				else
				{
					m_pPrimary->AddItem(kvPrimary->GetString("AssignmentName"), kvPrimary);
				}
			}
		}
		
	}

	void CFFCustomHudNewPresetDialog::OnUpdateCombos(KeyValues *data)
	{
		if (data->GetPtr("panel") == m_pPrimary)
		{	
			KeyValues *kvPrimary = m_pPrimary->GetActiveItemUserData();

			UpdateSecondaryComboFromParentKey(kvPrimary);
		}
		/*
		else if (data->GetPtr("panel") == m_pSecondary)
		{
			//this keyvalue holds the assignmets which were saved for the item we are selecting
			KeyValues *kvSecondary = m_pSecondary->GetActiveItemUserData();
		}
		*/
	}

	void CFFCustomHudNewPresetDialog::UpdateSecondaryComboFromParentKey(KeyValues *kvPrimary)
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
		}
	}
}