/********************************************************************
	created:	2006/08/14
	created:	14:8:2006   21:37
	filename: 	f:\ff-svn\code\trunk\game_shared\ff\ff_grenade_parse.cpp
	file path:	f:\ff-svn\code\trunk\game_shared\ff
	file base:	ff_grenade_parse
	file ext:	cpp
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/


#include "cbase.h"
#include <KeyValues.h>
#include <tier0/mem.h>
#include "filesystem.h"
#include "utldict.h"
#include "ff_grenade_parse.h"

#ifdef CLIENT_DLL
#include "hud.h"
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Here are all our grenade things
static CUtlDict<CFFGrenadeInfo *, unsigned short> m_GrenadeInfoDatabase;

// This is our unassigned grenade... still needed?
static CFFGrenadeInfo g_NullClass;

//----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
// Output : CFFGrenadeInfo
//----------------------------------------------------------------------------
static GRENADE_FILE_INFO_HANDLE FindGrenadeInfoSlot(const char *name) 
{
	// Complain about duplicately defined metaclass names...
	unsigned short lookup = m_GrenadeInfoDatabase.Find(name);
	if (lookup != m_GrenadeInfoDatabase.InvalidIndex()) 
	{
		return lookup;
	}

	CFFGrenadeInfo *insert = CreateGrenadeInfo();

	lookup = m_GrenadeInfoDatabase.Insert(name, insert);
	Assert(lookup != m_GrenadeInfoDatabase.InvalidIndex());
	return lookup;
}

//----------------------------------------------------------------------------
// Purpose: Find a playerclass slot, assuming the playerclass's data has already been loaded.
//----------------------------------------------------------------------------
GRENADE_FILE_INFO_HANDLE LookupGrenadeInfoSlot(const char *name) 
{
	return m_GrenadeInfoDatabase.Find(name);
}

// FIXME, handle differently?
static CFFGrenadeInfo gNullGrenadeInfo;

//----------------------------------------------------------------------------
// Purpose: 
// Input  : handle - 
// Output : CFFGrenadeInfo
//----------------------------------------------------------------------------
CFFGrenadeInfo *GetFileGrenadeInfoFromHandle(GRENADE_FILE_INFO_HANDLE handle) 
{
	if (handle < 0 || handle >= m_GrenadeInfoDatabase.Count()) 
	{
		return &gNullGrenadeInfo;
	}

	if (handle == m_GrenadeInfoDatabase.InvalidIndex()) 
	{
		return &gNullGrenadeInfo;
	}

	return m_GrenadeInfoDatabase[ handle ];
}

//----------------------------------------------------------------------------
// Purpose: 
// Output : GRENADE_FILE_INFO_HANDLE
//----------------------------------------------------------------------------
GRENADE_FILE_INFO_HANDLE GetInvalidGrenadeInfoHandle() 
{
	return (GRENADE_FILE_INFO_HANDLE) m_GrenadeInfoDatabase.InvalidIndex();
}

//----------------------------------------------------------------------------
// Purpose: Precaches all the playerclass_ *.txt files
//----------------------------------------------------------------------------
void PrecacheFileGrenadeInfoDatabase(IFileSystem *filesystem, const unsigned char *pICEKey) 
{
	if (m_GrenadeInfoDatabase.Count()) 
		return;

	FileFindHandle_t findHandle;
	const char *pFilename = filesystem->FindFirstEx("scripts/ff_grenade_*.txt", "MOD", &findHandle);
	while (pFilename != NULL) 
	{
		char fileBase[512];
		Q_FileBase(pFilename, fileBase, sizeof(fileBase));
		GRENADE_FILE_INFO_HANDLE tmp;
		ReadGrenadeDataFromFileForSlot(filesystem, fileBase, &tmp, pICEKey);
		pFilename = filesystem->FindNext(findHandle);
	}
	filesystem->FindClose(findHandle);
}

//----------------------------------------------------------------------------
// Purpose: Reads the file for KeyValues
//----------------------------------------------------------------------------
KeyValues *ReadEncryptedKVGrenadeFile(IFileSystem *filesystem, const char *szFilenameWithoutExtension, const unsigned char *pICEKey) 
{
	Assert(strchr(szFilenameWithoutExtension, '.') == NULL);
	char szFullName[512];

	const char *pSearchPath = "MOD";

	if (pICEKey == NULL) 
	{
		pSearchPath = "GAME";
	}

	// Open the playerclass data file, and abort if we can't
	KeyValues *pKV = new KeyValues("GrenadeDatafile");

	Q_snprintf(szFullName, sizeof(szFullName), "%s.txt", szFilenameWithoutExtension);

	if (!pKV->LoadFromFile(filesystem, szFullName, pSearchPath)) // try to load the normal .txt file first
	{
		if (pICEKey) 
		{
			Q_snprintf(szFullName, sizeof(szFullName), "%s.ctx", szFilenameWithoutExtension); // fall back to the .ctx file

			FileHandle_t f = filesystem->Open(szFullName, "rb", pSearchPath);

			if (!f) 
			{
				pKV->deleteThis();
				return NULL;
			}
			// load file into a null-terminated buffer
			int fileSize = filesystem->Size(f);
			char *buffer = (char *) MemAllocScratch(fileSize + 1);
		
			Assert(buffer);
		
			filesystem->Read(buffer, fileSize, f); // read into local buffer
			buffer[fileSize] = 0; // null terminate file as EOF
			filesystem->Close(f);	// close file after reading

			UTIL_DecodeICE((unsigned char *) buffer, fileSize, pICEKey);

			bool retOK = pKV->LoadFromBuffer(szFullName, buffer, filesystem);

			MemFreeScratch();

			if (!retOK) 
			{
				pKV->deleteThis();
				return NULL;
			}
		}
		else
		{
				pKV->deleteThis();
				return NULL;
		}
	}

	return pKV;
}


//-----------------------------------------------------------------------------
// Purpose: Read data on playerclass from script file
// Output:  true  - if data2 successfully read
//			false - if data load fails
//-----------------------------------------------------------------------------
bool ReadGrenadeDataFromFileForSlot(IFileSystem *filesystem, const char *szGrenadeName, GRENADE_FILE_INFO_HANDLE *phandle, const unsigned char *pICEKey) 
{
	if (!phandle) 
	{
		Assert(0);
		return false;
	}

	*phandle = FindGrenadeInfoSlot(szGrenadeName);
	CFFGrenadeInfo *pFileInfo = GetFileGrenadeInfoFromHandle(*phandle);
	Assert(pFileInfo);

	if (pFileInfo->bParsedScript) 
		return true;

	char sz[128];
	Q_snprintf(sz, sizeof(sz), "scripts/%s", szGrenadeName);
	KeyValues *pKV = ReadEncryptedKVFile(filesystem, sz, pICEKey);
	if (!pKV) 
		return false;

	pFileInfo->Parse(pKV, szGrenadeName);

	pKV->deleteThis();

	return true;
}


//============================================================================
// CFFGrenadeInfo implementation.
//============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFGrenadeInfo::CFFGrenadeInfo() 
{
	bParsedScript = false;

	// Added defaults here
	iconHud = NULL;
	iconAmmo = NULL;

	Q_strncpy(szPrintName, "None", MAX_GRENADE_STRING);
}

void FreeHudTextureList(CUtlDict<CHudTexture *, int>& list);
CHudTexture *FindHudTextureInDict(CUtlDict< CHudTexture *, int>& list, const char *psz);

//----------------------------------------------------------------------------
// Purpose: Parse all the values for a class
//----------------------------------------------------------------------------
void CFFGrenadeInfo::Parse(KeyValues *pKeyValuesData, const char *szGrenadeName) 
{
	// Okay, we tried at least once to look this up...
	bParsedScript = true;

	Q_strncpy(szPrintName, pKeyValuesData->GetString("printname", GRENADE_PRINTNAME_MISSING), MAX_GRENADE_STRING);

#ifdef CLIENT_DLL
	char sz[256];
	Q_snprintf(sz, 255, "scripts/%s", szGrenadeName);

	// It's a bit wasteful to open the file again, but screw it it's late
	CUtlDict<CHudTexture *, int> tempList;
	LoadHudTextures(tempList, sz, g_pGameRules->GetEncryptionKey());

	// Point our iconHud to this texture
	CHudTexture *p = FindHudTextureInDict(tempList, "grenade");
	if (p)
	{
		iconHud = gHUD.AddUnsearchableHudIconToList(*p);
	}
	
	// Point our iconHud to this texture
	p = FindHudTextureInDict(tempList, "ammo");
	if (p)
	{
		iconAmmo = gHUD.AddUnsearchableHudIconToList(*p);
	}

	// Add a deathnotice that we can search for later
	p = FindHudTextureInDict(tempList, "deathnotice");
	if (p)
	{
		if (strlen(szGrenadeName) > 3)
		{
			Q_snprintf(p->szShortName, 63, "death_%s", szGrenadeName + 3);
			gHUD.AddSearchableHudIconToList(*p);
		}
	}
	FreeHudTextureList(tempList);
#endif
}

//----------------------------------------------------------------------------
// Purpose: Create a new class info for this player
//----------------------------------------------------------------------------
CFFGrenadeInfo *CreateGrenadeInfo() 
{
	return new CFFGrenadeInfo;
}
