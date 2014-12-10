#include "cbase.h"
#include "ff_cdll_client_int.h"

#include "filesystem.h"
#include "ff_utils.h"

#define CLASSCFG_PATH				"cfg/%s.cfg"
#define CLASSCFG_DEFAULT_PATH		"cfg/classcfg_default.cfg"
// userconfig is a helper cfg for the default class configs
#define USERCONFIG_PATH				"cfg/userconfig.cfg"
#define USERCONFIG_DEFAULT_PATH		"cfg/userconfig_default.cfg"

int CFFClient::Init( CreateInterfaceFn appSystemFactory, CreateInterfaceFn physicsFactory, CGlobalVarsBase *pGlobals )
{
	int ret = BaseClass::Init( appSystemFactory, physicsFactory, pGlobals );

	PopulateMissingClassConfigs();
	PopulateMissingUserConfig();
	
	return ret;
}

void CFFClient::PopulateMissingClassConfigs()
{
	bool bIsBufferPopulated = false;
	CUtlBuffer bufferDefaultConfig;
	char szConfigPath[MAX_PATH] = {0};

	// it's still worth creating empty class .cfg files if the default cfg
	// doesn't exist, so just mark the buffer as populated and continue as normal
	if (!filesystem->FileExists(CLASSCFG_DEFAULT_PATH, "MOD"))
		bIsBufferPopulated = true;

	for (int iClass=CLASS_SCOUT; iClass<=CLASS_CIVILIAN; iClass++ )
	{
		Q_snprintf(szConfigPath, sizeof(szConfigPath), CLASSCFG_PATH, Class_IntToString(iClass));

		// don't overwrite existing cfgs
		if (filesystem->FileExists(szConfigPath))
			continue;
		
		// only read the file if we know theres a use for its contents
		if (!bIsBufferPopulated)
		{
			// read in binary mode
			FileHandle_t fileDefaultConfig = filesystem->Open(CLASSCFG_DEFAULT_PATH, "rb");
			
			// if we fail to open the file, act like the file doesn't exist
			// and generate blank class cfgs
			if ( fileDefaultConfig != FILESYSTEM_INVALID_HANDLE )
			{
				filesystem->ReadToBuffer( fileDefaultConfig, bufferDefaultConfig );
				filesystem->Close(fileDefaultConfig);
			}

			bIsBufferPopulated = true;
		}

		filesystem->WriteFile(szConfigPath, "MOD", bufferDefaultConfig);
	}
}

void CFFClient::PopulateMissingUserConfig()
{
	// don't overwrite existing cfgs
	if (filesystem->FileExists(USERCONFIG_PATH, "MOD"))
		return;

	CUtlBuffer bufferDefaultConfig;

	// if the default doesn't exist or fails to get read, then a blank
	// userconfig.cfg will be created
	if (filesystem->FileExists(USERCONFIG_DEFAULT_PATH, "MOD"))
	{
		FileHandle_t fileDefaultConfig = filesystem->Open(USERCONFIG_DEFAULT_PATH, "rb");

		if ( fileDefaultConfig != FILESYSTEM_INVALID_HANDLE )
		{
			filesystem->ReadToBuffer( fileDefaultConfig, bufferDefaultConfig );
			filesystem->Close(fileDefaultConfig);
		}
	}

	filesystem->WriteFile(USERCONFIG_PATH, "MOD", bufferDefaultConfig);
}

CFFClient gFFClient;
IBaseClientDLL *clientdll = &gFFClient;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CFFClient, IBaseClientDLL, CLIENT_DLL_INTERFACE_VERSION, gFFClient );