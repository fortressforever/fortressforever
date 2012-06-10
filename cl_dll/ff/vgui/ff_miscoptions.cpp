#include "cbase.h"
#include "ff_miscoptions.h"

#include "filesystem.h"
extern IFileSystem **pFilesystem;

#include "KeyValues.h"

#include <vgui_controls/CheckButton.h>
#include <vgui_controls/ComboBox.h>
#include "ff_inputslider.h"
#include <vgui_controls/Slider.h>
#include <vgui/ILocalize.h>
/*
#include "ff_inputslider.h"
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/Button.h>
*/

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: Populate all the menu stuff
//-----------------------------------------------------------------------------
CFFMiscOptions::CFFMiscOptions(Panel *parent, char const *panelName, const char *pszSourceFile) : BaseClass(parent, panelName)
{
	LoadControlSettings("resource/ui/FFOptionsSubMisc.res");

	Q_strncpy(m_szSourceFile, pszSourceFile, 127);
}

//-----------------------------------------------------------------------------
// Purpose: Apply the player's changes
//			
//-----------------------------------------------------------------------------
void CFFMiscOptions::Apply()
{
	/*if ( m_pHintsConVar )
		m_pHintsConVar->SetValue( m_pHints->IsSelected() );

	if ( m_pAutoRLConVar )
		m_pAutoRLConVar->SetValue( m_pARCheck->IsSelected() );

	if ( m_pAutoKillConVar )
		m_pAutoKillConVar->SetValue( m_pAutoKillCheck->IsSelected() );

	if ( m_pBlurConVar )
		m_pBlurConVar->SetValue( m_pBlurCheck->IsSelected() );*/

	KeyValues *kvOptions = new KeyValues("Options");
	kvOptions->LoadFromFile(*pFilesystem, m_szSourceFile);

	// Loop through creating new options for each one
	for (KeyValues *kvOption = kvOptions->GetFirstSubKey(); kvOption != NULL; kvOption = kvOption->GetNextKey())
	{
		const char *pszCvar = kvOption->GetString("cvar");
		const char *pszName = kvOption->GetName();

		Panel *pChild = FindChildByName(pszName);

		if (!pChild)
			continue;

		ConVar *pCvar = cvar->FindVar(pszCvar);

		if (!pCvar)
			continue;

		// This is a bad show old chap
		if (CheckButton *cb = dynamic_cast <CheckButton *> (pChild))
		{
			pCvar->SetValue(cb->IsSelected());
		}
		else if (ComboBox *cb = dynamic_cast <ComboBox *> (pChild))
		{
			// Only replace the cvar with this option if it is not a custom one
			const char *pszValue = cb->GetActiveItemUserData()->GetString("value");
			if (Q_strncmp(pszValue, "custom", 6) != 0)
				pCvar->SetValue(pszValue);
		}
		else if (CFFInputSlider *slider = dynamic_cast <CFFInputSlider *> (pChild))
		{
			pCvar->SetValue(slider->GetValue());
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Just load again to reset
//-----------------------------------------------------------------------------
void CFFMiscOptions::Reset()
{
	Load();
}

//-----------------------------------------------------------------------------
// Purpose: Find the appropriate ConVar states and update the Check Boxes
//-----------------------------------------------------------------------------
void CFFMiscOptions::Load()
{
	//remove all exisiting specific options
	for(int i = GetChildCount() - 1; i >= 0; --i)
	//count backwards cause we're removing stuff (counting forwards will FAIL EPICLY!!)
	{
		Panel* childPanel = GetChild(i);
		childPanel->DeletePanel();
	}

	int iYCoords = TITLE_SPACER;

	// Put all our options stuff in a keyfile now
	KeyValues *kvOptions = new KeyValues("Options");
	kvOptions->LoadFromFile(*pFilesystem, m_szSourceFile);

	// Loop through creating new options for each one
	for (KeyValues *kvOption = kvOptions->GetFirstSubKey(); kvOption != NULL; kvOption = kvOption->GetNextKey())
	{
		const char *pszType = kvOption->GetString("type", "boolean");

		const char* pszName = kvOption->GetName();
		const char* pszCaption = kvOption->GetString("caption");
		wchar_t* wszCaption = vgui::localize()->Find(pszCaption);
		char szCaption[128];
		if(wszCaption)
			vgui::localize()->ConvertUnicodeToANSI(wszCaption, szCaption, sizeof(szCaption));
		else
			Q_strcpy(szCaption, pszCaption);

		// A little separator
		if (Q_strncmp(pszName, "heading", 7) == 0)
		{
			Label *l = new Label(this, "label", szCaption);

			if (l)
			{
				l->SetPos(25, iYCoords + TITLE_SPACER);
				l->SetSize(250, ROW_HEIGHT);
				iYCoords += ROW_HEIGHT + TITLE_SPACER;	// Add extra bit on
			}
		}

		// Boolean is just a simple checkbox
		else if (Q_strncmp(pszType, "boolean", 7) == 0)
		{
			CheckButton *cb = new CheckButton(this, pszName, szCaption);

			if (!cb)
				continue;

			cb->SetPos(30, iYCoords);
			cb->SetSize(450, ROW_HEIGHT);

			iYCoords += ROW_HEIGHT;
		}
		// Discrete is a combobox with a label
		else if (Q_strncmp(pszType, "discrete", 8) == 0)
		{
			KeyValues *kvValues = kvOption->FindKey("values", false);
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

			ComboBox *cb = new ComboBox(this, pszName, nValues, false);

			if (!cb)
				continue;

			kvValues = kvOption->FindKey("values", false);

			if (!kvValues)
				continue;

			// Now go through all the values and add them to the combobox
			kvValue = kvValues->GetFirstSubKey();
			while (kvValue)
			{
				const char *pszValue = kvValue->GetName();
				const char *pszItemCaption = kvValues->GetString(pszValue);
				wchar_t* wszItemCaption = vgui::localize()->Find(pszItemCaption);
				char szItemCaption[128];
				if(wszItemCaption)
					vgui::localize()->ConvertUnicodeToANSI(wszItemCaption, szItemCaption, sizeof(szItemCaption));
				else
					Q_strcpy(szItemCaption, pszItemCaption);

				kvValue = kvValue->GetNextKey();

				KeyValues *kvItem = new KeyValues("kvItem");
				kvItem->SetString("value", pszValue);
				cb->AddItem(szItemCaption, kvItem);
				kvItem->deleteThis();
			}

			cb->SetPos(30, iYCoords);
			cb->SetSize(80, ROW_HEIGHT - 4);
			cb->ActivateItemByRow(0);

			// Create a handy label too so we know what this is
			Label *l = new Label(this, "label", szCaption);

			if (l)
			{
				l->SetPos(120, iYCoords);
				l->SetSize(450, ROW_HEIGHT);
			}

			iYCoords += ROW_HEIGHT;
		}
		// Slider is a slider with a label
		else if (Q_strncmp(pszType, "slider", 6) == 0)
		{
			CFFInputSlider *slider = new CFFInputSlider(this, pszName, VarArgs("%sInput", pszName));

			if (!slider)
				continue;
			
			int minval = kvOption->GetInt("minval");
			int maxval = kvOption->GetInt("maxval");

			slider->SetRange(minval,maxval);
			slider->SetValue(minval);

			slider->SetSize(80, ROW_HEIGHT - 4);
			slider->SetPos(30, iYCoords);

			// Create a handy label too so we know what this is
			Label *l = new Label(this, "label", szCaption);

			if (l)
			{
				l->SetPos(180, iYCoords);
				l->SetSize(250, ROW_HEIGHT);
			}

			iYCoords += ROW_HEIGHT;
		}
	}

	// Loop through creating new options for each one
	for (KeyValues *kvOption = kvOptions->GetFirstSubKey(); kvOption != NULL; kvOption = kvOption->GetNextKey())
	{
		const char *pszCvar = kvOption->GetString("cvar");
		const char *pszName = kvOption->GetName();

		Panel *pChild = FindChildByName(pszName);

		if (!pChild)
			continue;

		ConVar *pCvar = cvar->FindVar(pszCvar);

		if (!pCvar)
			continue;

		// This is a bad show old chap
		if (CheckButton *cb = dynamic_cast <CheckButton *> (pChild))
		{
			cb->SetSelected(pCvar->GetBool());
		}
		else if (ComboBox *cb = dynamic_cast <ComboBox *> (pChild))
		{
			int option = GetComboBoxOption(cb, pCvar->GetString());

			// Option doesn't exist, so add a "custom field"
			if (option == -1)
			{
				int custom = GetComboBoxOption(cb, "custom");

				// Need to add the custom field
				if (custom == -1)
				{
					KeyValues *kvItem = new KeyValues("kvItem");
					kvItem->SetString("value", "custom");
					cb->AddItem("custom", kvItem);
					kvItem->deleteThis();
				}

				custom = GetComboBoxOption(cb, "custom");
				cb->ActivateItem(custom);
			}
			else
			{
				// We've found this item, so activate it
				cb->ActivateItem(option);

				// However lets remove the custom row if it exists
				int custom = GetComboBoxOption(cb, "custom");
				if (custom != -1)
					cb->DeleteItem(custom);
			}

			// Stuff below replaced by GetComboBoxOption()
			/*const char *pszCvarValue = pCvar->GetString();
			for (int i = 0; i < cb->GetItemCount(); i++)
			{
				KeyValues *kvItem = cb->GetItemUserData(i);
				const char *pszItemValue = kvItem->GetString("value");
				if (kvItem && Q_strncmp(pszItemValue, pszCvarValue, 5) == 0) {
					cb->ActivateItem(i);
					break;
				}
			}*/
		}
		else if (CFFInputSlider *slider = dynamic_cast <CFFInputSlider *> (pChild))
		{
			slider->SetValue(pCvar->GetInt());
		}
	}


	/*if ( !m_pHintsConVar )
		m_pHintsConVar = cvar->FindVar( "cl_hints" );
	if ( m_pHintsConVar )
		m_pHints->SetSelected( m_pHintsConVar->GetBool() );

	if ( !m_pAutoRLConVar )
		m_pAutoRLConVar = cvar->FindVar( "cl_autoreload" );
	if ( m_pAutoRLConVar )
		m_pARCheck->SetSelected( m_pAutoRLConVar->GetBool() );

	if ( !m_pAutoKillConVar )
		m_pAutoKillConVar = cvar->FindVar( "cl_classautokill" );
	if ( m_pAutoKillConVar )
		m_pAutoKillCheck->SetSelected( m_pAutoKillConVar->GetBool() );

	if ( !m_pBlurConVar )
		m_pBlurConVar = cvar->FindVar( "cl_dynamicblur" );
	if ( m_pBlurConVar )
		m_pBlurCheck->SetSelected( m_pBlurConVar->GetBool() );*/
}

int CFFMiscOptions::GetComboBoxOption(ComboBox *cb, const char *value, const char *keyname)
{
	int n = cb->GetItemCount();
	int l = strlen(value);
	for (int i = 0; i < n; i++)
	{
		KeyValues *kvItem = cb->GetItemUserData(i);
		const char *pszItemValue = kvItem->GetString(keyname);
		if (kvItem && Q_strncmp(pszItemValue, value, l) == 0) {
			return i;
		}
	}
	return -1;
}