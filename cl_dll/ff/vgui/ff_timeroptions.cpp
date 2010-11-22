#include "cbase.h"
#include "ff_timeroptions.h"

#include "filesystem.h"
extern IFileSystem **pFilesystem;

#include <vgui_controls/Button.h>

// memdbgon must be the last include file in a .cpp file!!! 
#include "tier0/memdbgon.h"

extern ConVar cl_timerwav;
extern ConVar cl_killbeepwav;

//-----------------------------------------------------------------------------
// Purpose: Populate all the menu stuff
//-----------------------------------------------------------------------------
CFFTimerOptions::CFFTimerOptions(Panel *parent, char const *panelName) : BaseClass(parent, panelName)
{
	m_pTimers = new ComboBox(this, "TimerList", 0, false);
	m_pPlayButton = new Button(this, "PlayButton", "", this, "Play");

	m_pBeeps = new ComboBox(this, "BeepList", 0, false);
	m_pPlayButton2 = new Button(this, "PlayButton2", "", this, "Play2");

	LoadControlSettings("resource/ui/FFOptionsSubTimer.res");
}

//-----------------------------------------------------------------------------
// Purpose: This is a bit messy. We need to save the filename without the
//			extension to the cvar.
//-----------------------------------------------------------------------------
void CFFTimerOptions::Apply()
{
	const char *pszTimer = m_pTimers->GetActiveItemUserData()->GetString("file");

	if (!pszTimer)
		return;

	int nLen = strlen(pszTimer);

	if (nLen < 5)
		return;

	char buf[128];
	Q_snprintf(buf, 127, "%s", pszTimer);
	buf[nLen - 4] = 0;

	cl_timerwav.SetValue(buf);

	// same for death beep

	const char *pszBeep = m_pBeeps->GetActiveItemUserData()->GetString("file");

	if (!pszBeep)
		return;

	int nLen2 = strlen(pszBeep);

	if (nLen2 < 5)
		return;

	char buf2[128];
	Q_snprintf(buf2, 127, "%s", pszBeep);
	buf2[nLen2 - 4] = 0;

	cl_killbeepwav.SetValue(buf2);
}

//-----------------------------------------------------------------------------
// Purpose: Just load again to reset
//-----------------------------------------------------------------------------
void CFFTimerOptions::Reset()
{
	Load();
}

//-----------------------------------------------------------------------------
// Purpose: Load all the timers into the combobox
//-----------------------------------------------------------------------------
void CFFTimerOptions::Load()
{
	int iCurrent = 0;
	m_pTimers->DeleteAllItems();

	FileFindHandle_t findHandle;
	const char *pFilename = (*pFilesystem)->FindFirstEx("sound/timers/*.wav", "MOD", &findHandle);
	
	while (pFilename != NULL) 
	{
		KeyValues *kv = new KeyValues("timers");
		kv->SetString("file", pFilename);
		int iNew= m_pTimers->AddItem(pFilename, kv);
		kv->deleteThis();

		int nLength = strlen(cl_timerwav.GetString());

		// This is our timer file
		if (Q_strncmp(pFilename, cl_timerwav.GetString(), nLength) == 0)
		{
			iCurrent = iNew;
		}

		pFilename = (*pFilesystem)->FindNext(findHandle);
	}

	(*pFilesystem)->FindClose(findHandle);

	m_pTimers->ActivateItemByRow(iCurrent);

	// same for death beeps
	int iCurrent2 = 0;
	m_pBeeps->DeleteAllItems();

	FileFindHandle_t findHandle2;
	const char *pFilename2 = (*pFilesystem)->FindFirstEx("sound/player/deathbeep/*.wav", "MOD", &findHandle2);
	
	while (pFilename2 != NULL) 
	{
		KeyValues *kv = new KeyValues("Beeps");
		kv->SetString("file", pFilename2);
		int iNew= m_pBeeps->AddItem(pFilename2, kv);
		kv->deleteThis();

		int nLength = strlen(cl_killbeepwav.GetString());

		// This is our timer file
		if (Q_strncmp(pFilename2, cl_killbeepwav.GetString(), nLength) == 0)
		{
			iCurrent2 = iNew;
		}

		pFilename2 = (*pFilesystem)->FindNext(findHandle2);
	}

	(*pFilesystem)->FindClose(findHandle2);

	m_pBeeps->ActivateItemByRow(iCurrent2);
}

void CFFTimerOptions::OnButtonCommand(KeyValues *data)
{
	const char *pszCommand = data->GetString("command");

	if (Q_strcmp(pszCommand, "Play") == 0)
	{
		const char *pszTimer = m_pTimers->GetActiveItemUserData()->GetString("file");

		if (pszTimer)
		{
			engine->ClientCmd(VarArgs("play timers/%s\n", pszTimer));
		}
	}
	else if (Q_strcmp(pszCommand, "Play2") == 0)
	{
		const char *pszBeep = m_pBeeps->GetActiveItemUserData()->GetString("file");

		if (pszBeep)
		{
			engine->ClientCmd(VarArgs("play player/deathbeep/%s\n", pszBeep));
		}
	}
}