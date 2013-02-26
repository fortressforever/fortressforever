
#include "cbase.h"
#include "autoupdate.h"
#include "filesystem.h"

#include "vgui/ff_updates.h"
extern CFFUpdatesPanel *g_pUpdatePanel;

// supressing macro redefinition warnings
#undef ARRAYSIZE
#undef GetCommandLine
#undef ReadConsoleInput
#undef RegCreateKey
#undef RegCreateKeyEx
#undef RegOpenKey
#undef RegOpenKeyEx
#undef RegQueryValue
#undef RegQueryValueEx
#undef RegSetValue
#undef RegSetValueEx

#include "Shlwapi.h"
#pragma comment (lib, "Shlwapi.lib")

int CFFUpdateThread::Run() 
{
	m_bIsRunning = true;

	while (g_pUpdatePanel == NULL)
	{
		DevMsg("[update] g_pUpdatePanel is NULL\n");
	}

	g_pUpdatePanel->UpdateAvailable( wyUpdateAvailable() );
	
	/*
	// just to test the thread
	int i=0;
	while(IsAlive())
	{
		i++;
		Sleep(1000);
		DevMsg("[UpdateThread] %d...\n", i);
		if (i>20)
			break;
	}
	*/

	m_bIsRunning = false;

	return 1;
}

bool wyUpdateAvailable()
{
	if (vgui::filesystem()->FileExists( "wyUpdate.exe", "MOD" ))
	{
		// Get the real path to wyUpdate.exe
		char szPath[MAX_PATH] = {0};

		// start with "
		szPath[0] = '\"';

		// add full local path to wyUpdate
		vgui::filesystem()->GetLocalPath( "wyUpdate.exe", &szPath[1], MAX_PATH-1 );

		// add params
		Q_strncat(szPath, "\" /quickcheck /justcheck", MAX_PATH);
		//PathAppend(szPath, _T("wyUpdate.exe\" /quickcheck /justcheck")); // only usable if the path is to a directory, not a file
		//Msg("[update] command: %s\n", szPath);

		Msg("[update] Checking for updates\n", szPath);

		STARTUPINFO si = {0}; si.cb = sizeof(si);
		PROCESS_INFORMATION pi = {0};

		// start wyUpdate
		if (!CreateProcess(NULL, szPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			Msg("[update] ERROR: Failed to launch wyUpdate\n");
			//MessageBox(0, _T("Failed to launch wyUpdate."), _T("Error..."), MB_OK);
			return false;
		}

		// Wait until child process exits.
		VCRHook_WaitForSingleObject(pi.hProcess, INFINITE);

		// Get the exit code
		DWORD exitcode = 0;
		GetExitCodeProcess(pi.hProcess, &exitcode);

		// Close process and thread handles.
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);

		// exitcode of 2 means update available
		return exitcode == 2;
	}
	else
	{
		Msg("[update] ERROR: wyUpdate.exe not found\n");
		return false;
	}
}

void wyInstallUpdate()
{
	if (vgui::filesystem()->FileExists( "wyUpdate.exe", "MOD" ))
	{
		TCHAR szUpdatePath[MAX_PATH] = {0};
		vgui::filesystem()->GetLocalPath( "wyUpdate.exe", szUpdatePath, MAX_PATH );

		// Start the wyUpdate and Quit
		ShellExecute(NULL, NULL, szUpdatePath, NULL, NULL, SW_SHOWNORMAL);
		engine->ClientCmd("exit\n");
	}
	else
	{
		Msg("[update] ERROR: wyUpdate.exe not found\n");
	}
}