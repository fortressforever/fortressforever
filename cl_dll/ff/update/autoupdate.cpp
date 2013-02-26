
#include "cbase.h"
#include "autoupdate.h"
#include "filesystem.h"

#include "irc/ff_socks.h"
#include "vgui/ff_updates.h"
extern CFFUpdatesPanel *g_pUpdatePanel;
char *GetModVersion();

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

	// try checking for update with sockets
	eUpdateResponse response = sockUpdateAvailable( m_szServerVersion );

	// if sockets failed, try wyUpdate; it's slower, so only use it as a last resort
	if (response == UPDATE_ERROR)
		response = wyUpdateAvailable();

	if (g_pUpdatePanel)
		g_pUpdatePanel->UpdateAvailable( response );
	
	/*
	int i=0;
	while(IsAlive() && i<0)
	{
		i++;
		Sleep(100);
		Msg("[update] %d...\n", i);
	}
	*/

	m_bIsRunning = false;

	return 1;
}

eUpdateResponse wyUpdateAvailable()
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

		STARTUPINFO si = {0}; si.cb = sizeof(si);
		PROCESS_INFORMATION pi = {0};

		// start wyUpdate
		if (!CreateProcess(NULL, szPath, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			Msg("[update] ERROR: Failed to launch wyUpdate\n");
			//MessageBox(0, _T("Failed to launch wyUpdate."), _T("Error..."), MB_OK);
			return UPDATE_ERROR;
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
		return (exitcode == 2 ? UPDATE_FOUND : UPDATE_NOTFOUND);
	}
	else
	{
		Msg("[update] ERROR: wyUpdate.exe not found\n");
		return UPDATE_ERROR;
	}
}

eUpdateResponse sockUpdateAvailable( const char *pszServerVersion )
{
	static Socks sock;
	static char buf[1024];
	int a;
	
	// Open up a socket
	if (!sock.Open( 1, 0)) 
	{
		Warning("[update] Could not open socket\n");
		sock.Close();
		return UPDATE_ERROR;
	}
	// Connect to remote host
	if (!sock.Connect("www.fortress-forever.com", 80)) 
	{
		Warning("[update] Could not connect to remote host\n");
		sock.Close();
		return UPDATE_ERROR;
	}
	
	Q_snprintf(buf, sizeof(buf),
		"GET /notifier/test.php?c=%s&s=%s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: FortressForever\r\n"
		"Connection: close\r\n"
		"Accept-Charset: ISO-8859-1,UTF-8;q=0.7,*;q=0.7\r\n"
		"Cache-Control: no-cache\r\n"
		"\r\n",
		
		GetModVersion(), (pszServerVersion ? pszServerVersion : ""),
		"www.fortress-forever.com");

	// Send data
	if (!sock.Send(buf)) 
	{
		Warning("[update] Could not send data to remote host\n");
		sock.Close();
		return UPDATE_ERROR;
	}

	// Send data
	if ((a = sock.Recv(buf, sizeof(buf)-1)) == 0) 
	{
		Warning("[update] Did not get response from server\n");
		sock.Close();
		return UPDATE_ERROR;
	}

	buf[a] = '\0';
	
	// response header first line determines the status:
	// HTTP/1.1 ??? Code
	//	201 Created = client patch available
	//	409 Conflict = server out of date
	//	304 Not Modified = up to date
	char *response = strtok(buf,"\r\n");
	if (!response)
	{
		Warning("[update] Unknown response received (NULL)\n");
		return UPDATE_ERROR;
	}
	else if (strlen(response) <= 10)
	{
		Warning("[update] Unknown response received (%s)\n", response);
		return UPDATE_ERROR;
	}
	response = response+9; // skip the HTTP/1.1 part

	//DevMsg("[update] Successfully sent update check request. Response code: %s\n Full response:\n---\n%s\n---\n", response, buf);

	eUpdateResponse ret;

	if (Q_strcmp(response, "304 Not Modified") == 0)
	{
		ret = UPDATE_NOTFOUND;
	}
	else if (Q_strcmp(response, "409 Conflict") == 0)
	{
		ret = UPDATE_SERVER_OUTOFDATE;
	}
	else if (Q_strcmp(response, "201 Created") == 0)
	{
		ret = UPDATE_FOUND;
	}
	else
	{
		Warning("[update] Unknown response code received (%s)\n", response);
		ret = UPDATE_ERROR;
	}

	// Close socket
	sock.Close();
	
	return ret;
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