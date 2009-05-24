/********************************************************************
	created:	2006/09/28
	created:	28:9:2006   13:03
	filename: 	f:\ff-svn\code\trunk\cl_dll\ff\vgui\ff_gamemodes.cpp
	file path:	f:\ff-svn\code\trunk\cl_dll\ff\vgui
	file base:	ff_gamemodes
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/


#include "cbase.h"
#include "ff_gamemodes.h"

#include "KeyValues.h"
#include "filesystem.h"

#include "ff_weapon_base.h"
extern const char *s_WeaponAliasInfo[];

#include <vgui_controls/PropertySheet.h>
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Slider.h>
#include <vgui_controls/TextEntry.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Button.h>
#include <vgui_controls/CheckButton.h>

using namespace vgui;
extern IFileSystem **pFilesystem;

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

#define MAX_TITLE		128
#define MAX_DESCRIPTION	128

//=============================================================================
// Training script loader
//=============================================================================
class CFFTrainingGameMode : public CFFGameModesPage
{
	DECLARE_CLASS_SIMPLE(CFFTrainingGameMode, CFFGameModesPage);

public:

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	CFFTrainingGameMode(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
	{
		m_pTitle = new Label(this, "TrainingTitle", "");
		m_pDescription = new Label(this, "TrainingDescription", "");
		m_pTrainingScripts = new ComboBox(this, "TrainingScriptsCombo", 8, "");

		LoadControlSettings("resource/ui/FFGameModesTraining.res");
	}

	//-----------------------------------------------------------------------------
	// Purpose: Set the bot_training cvar, start the map
	//-----------------------------------------------------------------------------
	void Play()
	{
		KeyValues *kv = m_pTrainingScripts->GetActiveItemUserData();

		if (kv == NULL)
			return;

		engine->ClientCmd(VarArgs("botrules_training %s\n", kv->GetString("script")));
		engine->ClientCmd(VarArgs("map %s\n", kv->GetString("map")));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load the descriptions and titles into the combobox
	//-----------------------------------------------------------------------------
	void Load()
	{
		m_pTrainingScripts->DeleteAllItems();

		FileFindHandle_t findHandle;
		const char *pFilename = (*pFilesystem)->FindFirstEx("maps/*.gm", "MOD", &findHandle);

		while (pFilename != NULL) 
		{
			char szFilename[MAX_PATH];
			Q_strncpy(szFilename, pFilename, MAX_PATH - 1);

			char *pszEndOfFilename = strstr(szFilename, "_scenario");

			// Invalid name
			if (!pszEndOfFilename)
			{
				pFilename = (*pFilesystem)->FindNext(findHandle);
				continue;
			}

			pszEndOfFilename[0] = '\0';

			// There is a corresponding bsp for ths file
			if ((*pFilesystem)->FileExists(VarArgs("maps/%s.bsp", szFilename), "MOD"))
			{
				char szTitle[MAX_TITLE];
				char szDescription[MAX_DESCRIPTION];
				ReadHeader(VarArgs("maps/%s", pFilename), szTitle, szDescription);

				KeyValues *kv = new KeyValues("maps");
				kv->SetString("script", pFilename);
				kv->SetString("map", VarArgs("%s", szFilename));
				kv->SetString("title", szTitle);
				kv->SetString("description", szDescription);
				m_pTrainingScripts->AddItem(szTitle, kv);
				kv->deleteThis();
			}

			pFilename = (*pFilesystem)->FindNext(findHandle);
		}

		(*pFilesystem)->FindClose(findHandle);

		m_pTrainingScripts->ActivateItemByRow(0);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Quick + hacky header reading script. Assumes that header starts
	//			on the second line and is one line containing the title
	//			and the other line containing the description (both localised)
	//			TODO: Fix this up when possible.
	//-----------------------------------------------------------------------------
	bool ReadHeader(const char *pszFilename, char szTitle[MAX_TITLE], char szDescription[MAX_DESCRIPTION])
	{
		FileHandle_t hFile = (*pFilesystem)->Open(pszFilename, "r");

		char szLineBuffer[1024];

		// The first line just starts the comment, ignore
		(*pFilesystem)->ReadLine(szLineBuffer, 1023, hFile);

		// Second line has the title
		(*pFilesystem)->ReadLine(szLineBuffer, 1023, hFile);
		Q_strncpy(szTitle, szLineBuffer, min(strlen(szLineBuffer), MAX_TITLE - 1));

		// And 3rd has the description. These should be localised ideally.
		(*pFilesystem)->ReadLine(szLineBuffer, 1023, hFile);
		Q_strncpy(szDescription, szLineBuffer, min(strlen(szLineBuffer), MAX_DESCRIPTION - 1));

		(*pFilesystem)->Close(hFile);

		return false;
	}

	void Reset()
	{
	}

private:

	//-----------------------------------------------------------------------------
	// Purpose: Catch the combo box changing training script
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data)
	{
		if (data->GetPtr("panel") == m_pTrainingScripts)
		{
			KeyValues *kv = m_pTrainingScripts->GetActiveItemUserData();

			if (kv == NULL)
				return;
			
			m_pTitle->SetText(kv->GetString("title"));
			m_pDescription->SetText(kv->GetString("description"));
		}
	}

private:

	Label		*m_pDescription;
	Label		*m_pTitle;
	ComboBox	*m_pTrainingScripts;
};

//=============================================================================
// A simple scenario builder
//=============================================================================
class CFFScenarioGameMode : public CFFGameModesPage
{
	DECLARE_CLASS_SIMPLE(CFFScenarioGameMode, CFFGameModesPage);

private:

	//=============================================================================
	// Popup window that allows classes to be ticked to enable/disable
	//=============================================================================
	class CFFAllowedClasses : public Frame
	{
		DECLARE_CLASS_SIMPLE(CFFAllowedClasses, Frame);
	
	public:
		//-----------------------------------------------------------------------------
		// Purpose: 
		//-----------------------------------------------------------------------------
		CFFAllowedClasses(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
		{
			const char *pszAllowedCheckboxes[] = { "ScoutChk", "SniperChk", "SoldierChk", "DemomanChk", "MedicChk", "HwguyChk", "PyroChk", "SpyChk", "EngineerChk" };

			for (int i = 0; i < ARRAYSIZE(pszAllowedCheckboxes); i++)
			{
				m_pClasses[i] = new CheckButton(this, pszAllowedCheckboxes[i], pszAllowedCheckboxes[i]);
			}

			new Button(this, "OKButton", "", this, "OK");
			new Button(this, "CancelButton", "", this, "Cancel");

			SetSizeable(false);
			LoadControlSettings("resource/ui/FFGameModesScenarioAllowedClasses.res");
		}

		//-----------------------------------------------------------------------------
		// Purpose: Use the bitfields to select/disable the correct checkboxes
		//-----------------------------------------------------------------------------
		void PopulateCheckBoxes(int iUserSettings, int iMapSettings)
		{
			for (int i = 0; i < 9; i++)
			{
				bool bSelected = (iUserSettings & (1 << i));
				bool bEnabled = (iMapSettings & (1 << i));

				m_pClasses[i]->SetEnabled(bEnabled);
				m_pClasses[i]->SetSelected((bSelected && bEnabled));
			}
		}

		//-----------------------------------------------------------------------------
		// Purpose: Create a bitfield representing the selected checkboxes
		//-----------------------------------------------------------------------------
		int GetCheckBoxes()
		{
			int iRet = 0;

			for (int i = 0; i < 9; i++)
			{
				iRet += (m_pClasses[i]->IsSelected() ? (1 << i) : 0);
			}

			return iRet;
		}

		//-----------------------------------------------------------------------------
		// Purpose: Set the correct team for this
		//-----------------------------------------------------------------------------
		void SetCurrentTeam(int iTeam)
		{
			Assert(iTeam >= 0 && iTeam < 4);
			m_iCurrentTeam = iTeam;
		}

	private:

		//-----------------------------------------------------------------------------
		// Purpose: Catches button presses
		//-----------------------------------------------------------------------------
		MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data)
		{
			const char *pszCommand = data->GetString("command");

			if (Q_strcmp(pszCommand, "OK") == 0)
			{
				CFFScenarioGameMode *parent = dynamic_cast <CFFScenarioGameMode *> (GetParent());

				if (parent)
				{
					parent->UpdateAllowedClasses(m_iCurrentTeam, GetCheckBoxes());
				}
			}

			CloseModal();
			SetVisible(false);
		}

	private:
		CheckButton *m_pClasses[9];
		int			m_iCurrentTeam;
	};

public:

	//-----------------------------------------------------------------------------
	// Purpose: Populate all the menu stuff
	//-----------------------------------------------------------------------------
	CFFScenarioGameMode(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
	{
		m_pAllowed = new CFFAllowedClasses(this, "AllowedClass");
		m_pMaps = new ComboBox(this, "MapCombo", 5, false);

		const char *pszTeams[] = { "BlueTeam", "RedTeam", "YellowTeam", "GreenTeam" };

		for (int i = 0; i < ARRAYSIZE(pszTeams); i++)
		{
			m_pBotNumbers[i] = new ComboBox(this, VarArgs("%sNumbers", pszTeams[i]), 8, false);
			m_pAllowedButtons[i] = new Button(this, VarArgs("%sAllowed", pszTeams[i]), "", this, "");
			m_pTeamTitles[i] = new Label(this, VarArgs("%sTitle", pszTeams[i]), "");
			m_pBotsLabels[i] = new Label(this, VarArgs("%sBotsLabel", pszTeams[i]), "");
		}

		memset(&m_iAllowedClasses, 0, sizeof(m_iAllowedClasses));
		memset(&m_iMapEnabledClasses, 0, sizeof(m_iMapEnabledClasses));
		memset(&m_nMaxPlayers, 0, sizeof(m_nMaxPlayers));

		LoadControlSettings("resource/ui/FFGameModesScenario.res");
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void Play()
	{
#define ARRAYEXPAND(a) (a)[0], (a)[1], (a)[2], (a)[3]

		engine->ClientCmd(VarArgs("botrules_classlimits %d %d %d %d\n", ARRAYEXPAND(m_iAllowedClasses)));
		engine->ClientCmd(VarArgs("botrules_teamlimits %d %d %d %d\n", ARRAYEXPAND(m_nMaxPlayers)));
		engine->ClientCmd(VarArgs("botrules_teamroles %s %s %s %s\n", ARRAYEXPAND(m_szRoles)));

		engine->ClientCmd(VarArgs("map %s\n", m_pMaps->GetActiveItemUserData()->GetString("name")));
	}

	//-----------------------------------------------------------------------------
	// Purpose: Just load again to reset
	//-----------------------------------------------------------------------------
	void Reset()
	{
		Load();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load possible maps
	//-----------------------------------------------------------------------------
	void Load()
	{
		m_pMaps->DeleteAllItems();

		FileFindHandle_t findHandle;
		const char *pFilename = (*pFilesystem)->FindFirstEx("maps/*.scn", "MOD", &findHandle);

		while (pFilename != NULL) 
		{
			char szBuffer[MAX_PATH];
			Q_strncpy(szBuffer, pFilename, MAX_PATH);
			szBuffer[strlen(pFilename) - 4] = '\0';

			// There is a corresponding bsp for ths file
			if ((*pFilesystem)->FileExists(VarArgs("maps/%s.bsp", szBuffer), "MOD"))
			{
				KeyValues *kv = new KeyValues("maps");
				kv->SetString("name", szBuffer);
				m_pMaps->AddItem(szBuffer, kv);
				kv->deleteThis();
			}

			pFilename = (*pFilesystem)->FindNext(findHandle);
		}

		(*pFilesystem)->FindClose(findHandle);

		m_pMaps->ActivateItemByRow(0);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Load all the timers into the combobox
	//-----------------------------------------------------------------------------
	void LoadSettingsForMap(const char *pszMapName)
	{
		KeyValues *kv = new KeyValues("MapScenario");
		kv->LoadFromFile(*pFilesystem, VarArgs("maps/%s.scn", pszMapName));

		const char *pszTeams[] = { "BlueTeam", "RedTeam", "YellowTeam", "GreenTeam" };

		// Loop through teams
		for (int i = 0; i < ARRAYSIZE(pszTeams); i++)
		{
			KeyValues *k = kv->FindKey(pszTeams[i]);

			if (k)
			{
				SetTeamEnabled(i, true);

				m_nMaxPlayers[i] = k->GetInt("maxplayers");
				m_iMapEnabledClasses[i] = k->GetInt("classes");
				m_iAllowedClasses[i] = 511;
				
				m_pTeamTitles[i]->SetText(k->GetString("title"));

				Q_strncpy(m_szRoles[i], k->GetString("role"), 19);

				SetBotSizeOptions(i, m_nMaxPlayers[i]);
			}
			else
			{
				m_nMaxPlayers[i] = m_iMapEnabledClasses[i] = m_iAllowedClasses[i] = 0;
				Q_strcpy(m_szRoles[i], "0");
                SetTeamEnabled(i, false);
			}
		}

		kv->deleteThis();
	}

private:

	//-----------------------------------------------------------------------------
	// Purpose: Catches button press when the player clicks on the
	//			"Allowed Classes" button
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnButtonCommand, "Command", data)
	{
		Button *pButton = (Button *) data->GetPtr("panel", NULL);

		// Could not determine where this came from
		if (pButton == NULL)
			return;

		ShowAllowedClasses(pButton->GetName());
	}

	//-----------------------------------------------------------------------------
	// Purpose: Catch the combo box changing map
	//-----------------------------------------------------------------------------
	MESSAGE_FUNC_PARAMS(OnUpdateCombos, "TextChanged", data)
	{
		if (data->GetPtr("panel") == m_pMaps)
		{
			LoadSettingsForMap(m_pMaps->GetActiveItemUserData()->GetString("name"));
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Display the allowed classes window
	//-----------------------------------------------------------------------------
	void ShowAllowedClasses(const char *pszTeamString)
	{
		int iTeam = GetTeamIndex(pszTeamString);

		m_pAllowed->SetCurrentTeam(iTeam);
		m_pAllowed->PopulateCheckBoxes(m_iAllowedClasses[iTeam], m_iMapEnabledClasses[iTeam]);

		m_pAllowed->Activate();
		m_pAllowed->DoModal();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Get a TeamIndex from a team string
	//-----------------------------------------------------------------------------
	int GetTeamIndex(const char *pszTeamString)
	{
		if (Q_strncmp(pszTeamString, "Blue", 4) == 0)
			return 0;
		
		if (Q_strncmp(pszTeamString, "Red", 3) == 0)
			return 1;

		if (Q_strncmp(pszTeamString, "Yellow", 6) == 0)
			return 2;

		if (Q_strncmp(pszTeamString, "Green", 5) == 0)
			return 3;

		Assert(0);
		return 0;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Update the allowed classes bitfields
	//-----------------------------------------------------------------------------
	void UpdateAllowedClasses(int iTeamIndex, int iAllowed)
	{
		Assert(iTeamIndex >= 0 && iTeamIndex < 4);

		m_iAllowedClasses[iTeamIndex] = iAllowed;

		// Make sure we're still within map bounds
		m_iAllowedClasses[iTeamIndex] &= m_iMapEnabledClasses[iTeamIndex];
	}

	//-----------------------------------------------------------------------------
	// Purpose: Handy function to fully enable or disable a team on the ui
	//-----------------------------------------------------------------------------
	void SetTeamEnabled(int iTeamIndex, bool state)
	{
		m_pTeamTitles[iTeamIndex]->SetVisible(state);
		m_pAllowedButtons[iTeamIndex]->SetVisible(state);
		m_pBotNumbers[iTeamIndex]->SetVisible(state);
		m_pBotsLabels[iTeamIndex]->SetVisible(state);
	}

	//-----------------------------------------------------------------------------
	// Purpose: Set the correct number of options for the team size combo
	//-----------------------------------------------------------------------------
	void SetBotSizeOptions(int iTeamIndex, int nSize)
	{
		m_pBotNumbers[iTeamIndex]->DeleteAllItems();

		for (int i = 0; i <= nSize; i++)
		{
			KeyValues *kv = new KeyValues("Numbers");
			kv->SetInt("n", i);
			m_pBotNumbers[iTeamIndex]->AddItem(VarArgs("%d", i), kv);
			kv->deleteThis();
		}

		m_pBotNumbers[iTeamIndex]->ActivateItemByRow(nSize);
	}

private:

    CFFAllowedClasses	*m_pAllowed;

	ComboBox	*m_pMaps;
	ComboBox	*m_pBotNumbers[4];
	Button		*m_pAllowedButtons[4];
	Label		*m_pTeamTitles[4];
	
	Label		*m_pBotsLabels[4];

	int			m_iAllowedClasses[4];
	int			m_iMapEnabledClasses[4];
	int			m_nMaxPlayers[4];
	char		m_szRoles[4][20];
};

DEFINE_GAMEUI(CFFGameModes, CFFGameModesPanel, ffgamemodes);

//-----------------------------------------------------------------------------
// Purpose: Display the ff options
//-----------------------------------------------------------------------------
CON_COMMAND(ff_gamemodes, NULL)
{
	ffgamemodes->GetPanel()->SetVisible(true);
}

//-----------------------------------------------------------------------------
// Purpose: Set up our main options screen. This involves creating all the
//			tabbed pages too.
//-----------------------------------------------------------------------------
CFFGameModesPanel::CFFGameModesPanel(vgui::VPANEL parent) : BaseClass(NULL, "FFGameModesPanel")
{
	SetParent(parent);
	vgui::HScheme scheme = vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme");
	SetScheme(scheme);

	// Centre this panel on the screen for consistency.
	int nWide = GetWide();
	int nTall = GetTall();

	SetPos((ScreenWidth() - nWide) / 2, (ScreenHeight() - nTall) / 2);

	// This should be visible since we're only showing it when selected in the
	// main menu.
	SetVisible(false);

	m_pScenarioGameMode = new CFFScenarioGameMode(this, "ScenarioGameMode");
	m_pTrainingGameMode = new CFFTrainingGameMode(this, "TrainingGameMode");

	m_pPropertyPages = new PropertySheet(this, "GameModesPages", true);
	m_pPropertyPages->AddPage(m_pScenarioGameMode, "#GameUI_Scenarios");
	m_pPropertyPages->AddPage(m_pTrainingGameMode, "#GameUI_Training");
	m_pPropertyPages->SetActivePage(m_pScenarioGameMode);
	m_pPropertyPages->SetDragEnabled(false);

	m_pOKButton = new Button(this, "PlayButton", "", this, "Play");
	m_pCancelButton = new Button(this, "CancelButton", "", this, "Cancel");

	SetSizeable(false);
	
	LoadControlSettings("resource/ui/FFGameModes.res");
}

//-----------------------------------------------------------------------------
// Purpose: Catch the buttons. We have OK (save + close), Cancel (close) and
//			Apply (save).
//-----------------------------------------------------------------------------
void CFFGameModesPanel::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	// Play starts the mode
	if (Q_strcmp(pszCommand, "OK") == 0)
	{
		CFFGameModesPage *pActivePanel = dynamic_cast <CFFGameModesPage *> (m_pPropertyPages->GetActivePage());

		if (pActivePanel)
		{
			pActivePanel->Play();
		}
	}
	else
	{
		// Cancelled, so reset the settings
		m_pScenarioGameMode->Reset();
		m_pTrainingGameMode->Reset();
	}

	// Now make invisible
	SetVisible(false);
}

//-----------------------------------------------------------------------------
// Purpose: Catch this options menu coming on screen and load up the info needed
//			for the option pages.
//-----------------------------------------------------------------------------
void CFFGameModesPanel::SetVisible(bool state)
{
	if (state)
	{
		m_pScenarioGameMode->Load();
		m_pTrainingGameMode->Load();

		RequestFocus();
		MoveToFront();
	}

	BaseClass::SetVisible(state);
}