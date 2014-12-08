#include "cbase.h"
#include "ff_cdll_client_int.h"

#include "filesystem.h"
#include "ff_utils.h"

#define CLASSCFG_PATH				"cfg/%s.cfg"
#define CLASSCFG_DEFAULT_PATH		"cfg/classcfg_default.cfg"
#define USERCONFIG_PATH				"cfg/userconfig.cfg"
#define USERCONFIG_DEFAULT_PATH		"cfg/userconfig_default.cfg"

int CFFClient::Init( CreateInterfaceFn appSystemFactory, CreateInterfaceFn physicsFactory, CGlobalVarsBase *pGlobals )
{
	int ret = BaseClass::Init( appSystemFactory, physicsFactory, pGlobals );

	PopulateMissingConfigs();
	
	return ret;
}

void CFFClient::PopulateMissingConfigs()
{
	// populate class configs
	if (filesystem->FileExists(CLASSCFG_DEFAULT_PATH, "MOD"))
	{
		char szConfigPath[MAX_PATH] = {0};
		CUtlBuffer buf(0,0,false);
		bool bIsBufferPopulated = false;

		for (int iClass=CLASS_SCOUT; iClass<=CLASS_CIVILIAN; iClass++ )
		{
			Q_snprintf(szConfigPath, sizeof(szConfigPath), CLASSCFG_PATH, Class_IntToString(iClass));

			if (!filesystem->FileExists(szConfigPath))
			{
				if (!bIsBufferPopulated)
				{
					FileHandle_t f = filesystem->Open(CLASSCFG_DEFAULT_PATH, "rb");
					
					if ( f == FILESYSTEM_INVALID_HANDLE )
						continue;

					filesystem->ReadToBuffer( f, buf );
					filesystem->Close(f);

					bIsBufferPopulated = true;
				}

				filesystem->WriteFile(szConfigPath, "MOD", buf);
			}
		}
	}

	// populate userconfig (helper cfg for class cfgs)
	if (filesystem->FileExists(USERCONFIG_DEFAULT_PATH, "MOD") && !filesystem->FileExists(USERCONFIG_PATH, "MOD"))
	{
		CUtlBuffer buf(0,0,false);
		FileHandle_t f = filesystem->Open(USERCONFIG_DEFAULT_PATH, "rb");

		if ( f != FILESYSTEM_INVALID_HANDLE )
		{
			filesystem->ReadToBuffer( f, buf );
			filesystem->Close(f);

			filesystem->WriteFile(USERCONFIG_PATH, "MOD", buf);
		}
	}
}

CFFClient gFFClient;
IBaseClientDLL *clientdll = &gFFClient;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR( CFFClient, IBaseClientDLL, CLIENT_DLL_INTERFACE_VERSION, gFFClient );