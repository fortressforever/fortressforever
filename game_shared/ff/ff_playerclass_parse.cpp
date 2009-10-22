/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_playerclass_parse.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date March 30, 2005
/// @brief The FF player class script parser implementation
///
/// REVISIONS
/// ---------
/// Mar 30, 2005 Mirv: Initial implementation
//
//	12/6/2007, Mulchman:
//		Added man cannon stuff

#include "cbase.h"
#include <KeyValues.h>
#include <tier0/mem.h>
#include "filesystem.h"
#include "utldict.h"
#include "ff_playerclass_parse.h"
#include "ff_weapon_base.h"

#define MAX_PLAYERCLASS_SLOTS		10

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar ffdev_spy_maxcloakspeed( "ffdev_spy_maxcloakspeed", "220", FCVAR_REPLICATED | FCVAR_CHEAT );

static CUtlDict< CFFPlayerClassInfo *, unsigned short > m_PlayerClassInfoDatabase;

// used to track whether or not two player classes have been mistakenly assigned the wrong slot
bool g_bUsedPlayerClassSlots[MAX_PLAYERCLASS_SLOTS] = { 0 };

// this is our unassigned class
static CFFPlayerClassInfo g_NullClass;

//----------------------------------------------------------------------------
// Purpose: 
// Input  : *name - 
// Output : CFFPlayerClassInfo
//----------------------------------------------------------------------------
static PLAYERCLASS_FILE_INFO_HANDLE FindPlayerClassInfoSlot(const char *name) 
{
	// Complain about duplicately defined metaclass names...
	unsigned short lookup = m_PlayerClassInfoDatabase.Find(name);
	if (lookup != m_PlayerClassInfoDatabase.InvalidIndex()) 
	{
		return lookup;
	}

	CFFPlayerClassInfo *insert = CreatePlayerClassInfo();

	lookup = m_PlayerClassInfoDatabase.Insert(name, insert);
	Assert(lookup != m_PlayerClassInfoDatabase.InvalidIndex());
	return lookup;
}

//----------------------------------------------------------------------------
// Purpose: Find a playerclass slot, assuming the playerclass's data has already been loaded.
//----------------------------------------------------------------------------
PLAYERCLASS_FILE_INFO_HANDLE LookupPlayerClassInfoSlot(const char *name) 
{
	return m_PlayerClassInfoDatabase.Find(name);
}

// FIXME, handle differently?
static CFFPlayerClassInfo gNullPlayerClassInfo;

//----------------------------------------------------------------------------
// Purpose: 
// Input  : handle - 
// Output : CFFPlayerClassInfo
//----------------------------------------------------------------------------
CFFPlayerClassInfo *GetFilePlayerClassInfoFromHandle(PLAYERCLASS_FILE_INFO_HANDLE handle) 
{
	if (handle < 0 || handle >= m_PlayerClassInfoDatabase.Count()) 
	{
		return &gNullPlayerClassInfo;
	}

	if (handle == m_PlayerClassInfoDatabase.InvalidIndex()) 
	{
		return &gNullPlayerClassInfo;
	}

	return m_PlayerClassInfoDatabase[ handle ];
}

//----------------------------------------------------------------------------
// Purpose: 
// Output : PLAYERCLASS_FILE_INFO_HANDLE
//----------------------------------------------------------------------------
PLAYERCLASS_FILE_INFO_HANDLE GetInvalidPlayerClassInfoHandle() 
{
	return (PLAYERCLASS_FILE_INFO_HANDLE) m_PlayerClassInfoDatabase.InvalidIndex();
}


//----------------------------------------------------------------------------
// Purpose: 
//----------------------------------------------------------------------------
#if 0
void ResetFilePlayerClassInfoDatabase() 
{
	int c = m_PlayerClassInfoDatabase.Count(); 
	for (int i = 0; i < c; ++i) 
	{
		delete m_PlayerClassInfoDatabase[ i ];
	}
	m_PlayerClassInfoDatabase.RemoveAll();

#ifdef _DEBUG
	memset(g_bUsedPlayerClassSlots, 0, sizeof(g_bUsedPlayerClassSlots));
#endif
}
#endif

//----------------------------------------------------------------------------
// Purpose: Precaches all the playerclass_ *.txt files
//----------------------------------------------------------------------------
void PrecacheFilePlayerClassInfoDatabase(IFileSystem *filesystem, const unsigned char *pICEKey) 
{
	if (m_PlayerClassInfoDatabase.Count()) 
		return;

	FileFindHandle_t findHandle;
	const char *pFilename = filesystem->FindFirstEx("scripts/ff_playerclass_*.txt", "MOD", &findHandle);
	while (pFilename != NULL) 
	{
		char fileBase[512];
		Q_FileBase(pFilename, fileBase, sizeof(fileBase));
		PLAYERCLASS_FILE_INFO_HANDLE tmp;
		ReadPlayerClassDataFromFileForSlot(filesystem, fileBase, &tmp, pICEKey);
		pFilename = filesystem->FindNext(findHandle);
	}
	filesystem->FindClose(findHandle);
}

//----------------------------------------------------------------------------
// Purpose: Reads the file for KeyValues
//----------------------------------------------------------------------------
KeyValues * ReadEncryptedKVPlayerClassFile(IFileSystem *filesystem, const char *szFilenameWithoutExtension, const unsigned char *pICEKey) 
{
	Assert(strchr(szFilenameWithoutExtension, '.') == NULL);
	char szFullName[512];

	const char *pSearchPath = "MOD";

	if (pICEKey == NULL) 
	{
		pSearchPath = "GAME";
	}

	// Open the playerclass data file, and abort if we can't
	KeyValues *pKV = new KeyValues("PlayerClassDatafile");

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
bool ReadPlayerClassDataFromFileForSlot(IFileSystem * filesystem, const char *szPlayerClassName, PLAYERCLASS_FILE_INFO_HANDLE *phandle, const unsigned char *pICEKey) 
{
	if (!phandle) 
	{
		Assert(0);
		return false;
	}

	char szRealPlayerClassName[128] = "";

	if (Q_strncmp(szPlayerClassName, "ff_playerclass_", 15) != 0)
	{
		Q_strncpy(szRealPlayerClassName, "ff_playerclass_", 100);
	}

	Q_strncat(szRealPlayerClassName, szPlayerClassName, 50);

	*phandle = FindPlayerClassInfoSlot(szRealPlayerClassName);
	CFFPlayerClassInfo *pFileInfo = GetFilePlayerClassInfoFromHandle(*phandle);
	Assert(pFileInfo);

	if (pFileInfo->bParsedScript) 
		return true;

	pFileInfo->m_iSlot = 0;

	char sz[128];
	Q_snprintf(sz, sizeof(sz), "scripts/%s", szRealPlayerClassName);
	KeyValues *pKV = ReadEncryptedKVFile(filesystem, sz, pICEKey);
	if (!pKV) 
		return false;

	pFileInfo->Parse(pKV, szRealPlayerClassName);

	pKV->deleteThis();

	return true;
}


//============================================================================
// CFFPlayerClassInfo implementation.
//============================================================================

//----------------------------------------------------------------------------
// Purpose: Constructor
//----------------------------------------------------------------------------
CFFPlayerClassInfo::CFFPlayerClassInfo() 
{
	bParsedScript = false;
	bLoadedHudElements = false;

	m_iSlot = 0;

	// 
	Q_strncpy(m_szClassName, "unselected", MAX_PLAYERCLASS_STRING);
	Q_strncpy(m_szPrintName, "unselected class", MAX_PLAYERCLASS_STRING);
	Q_strncpy(m_szDescription, "You need to select a class!", MAX_PLAYERCLASS_STRING_LONG);
	Q_strncpy(m_szModel, "models/player/scout/scout.mdl", MAX_PLAYERCLASS_STRING);
	
	m_iMaxArmour = 0;
	m_iInitialArmour = 0;
	m_iArmourType = 0;

	m_iHealth = 0;
	
	m_iSpeed = 500;

	m_iNumSkills = m_iNumWeapons = m_iNumAmmos = 0;

	m_szPrimaryClassName[0] = 0;
	m_iPrimaryInitial = 0;
	m_iPrimaryMax = 0;

	m_szSecondaryClassName[0];
	m_iSecondaryInitial = 0;
	m_iSecondaryMax = 0;
}

//----------------------------------------------------------------------------
// Purpose: Parse all the values for a class
//----------------------------------------------------------------------------
void CFFPlayerClassInfo::Parse(KeyValues *pKeyValuesData, const char *szPlayerClassName) 
{
	// Okay, we tried at least once to look this up...
	bParsedScript = true;

	// Player number / selection slot
	m_iSlot = pKeyValuesData->GetInt("slot", 0);

	// Classname
	Q_strncpy(m_szClassName, szPlayerClassName, MAX_PLAYERCLASS_STRING);

	// Printable name & description
	Q_strncpy(m_szPrintName, pKeyValuesData->GetString("printname", PLAYERCLASS_PRINTNAME_MISSING), MAX_PLAYERCLASS_STRING);
	Q_strncpy(m_szDescription, pKeyValuesData->GetString("description", PLAYERCLASS_PRINTNAME_MISSING), MAX_PLAYERCLASS_STRING_LONG);

	// View model & world model
	Q_strncpy(m_szModel, pKeyValuesData->GetString("model"), MAX_PLAYERCLASS_STRING);

	// Health and armour values
	m_iMaxArmour		= pKeyValuesData->GetInt("max_armour", 0);
	m_iInitialArmour	= pKeyValuesData->GetInt("initial_armour", 0);
	m_iArmourType		= pKeyValuesData->GetInt("armour_type", 0);
	m_iHealth			= pKeyValuesData->GetInt("health", 0);

	// Speed
	m_iSpeed			= pKeyValuesData->GetInt("speed", 0);

	// Grenade types
	Q_strncpy(m_szPrimaryClassName, pKeyValuesData->GetString("primary_classname", PLAYERCLASS_PRINTNAME_MISSING), MAX_PLAYERCLASS_STRING);
	Q_strncpy(m_szSecondaryClassName, pKeyValuesData->GetString("secondary_classname", PLAYERCLASS_PRINTNAME_MISSING), MAX_PLAYERCLASS_STRING);

	// Grenade numbers
	m_iPrimaryInitial	= pKeyValuesData->GetInt("primary_initial", 0);
	m_iPrimaryMax		= pKeyValuesData->GetInt("primary_max", 0);
	m_iSecondaryInitial	= pKeyValuesData->GetInt("secondary_initial", 0);
	m_iSecondaryMax		= pKeyValuesData->GetInt("secondary_max", 0);

	// Make sure two player classes aren't in the same slot
	if (g_bUsedPlayerClassSlots[m_iSlot]) 
	{
		Msg("PlayerClass slot info: %s(%d) \n", m_szPrintName, m_iSlot);
		Warning("Duplicately assigned playerclass to slots in selection hud\n");
	}
	g_bUsedPlayerClassSlots[m_iSlot] = true;

	KeyValues *pWeaponData = pKeyValuesData->FindKey("ArmamentsData");

	// This will go through the values for weapon and abilities
	for (KeyValues *pArmaments = pWeaponData->GetFirstValue(); pArmaments; pArmaments = pArmaments->GetNextValue()) 
	{
		// This is a weapon
		if (strcmp(pArmaments->GetName(), "weapon") == 0) 
		{
			//m_vecWeapons.push_back(pArmaments->GetString());
			if (m_iNumWeapons < MAX_WEAPONS_PER_CLASS) 
				Q_strncpy(m_aWeapons[m_iNumWeapons++], pArmaments->GetString(), MAX_PLAYERCLASS_STRING);
		}
		else if (strcmp(pArmaments->GetName(), "skill") == 0) 
		{
			//m_vecSkills.push_back(pArmaments->GetString());
			if (m_iNumSkills < MAX_WEAPONS_PER_CLASS) 
				Q_strncpy(m_aSkills[m_iNumSkills++], pArmaments->GetString(), MAX_PLAYERCLASS_STRING);
		}
	}

	KeyValues *pAmmoData = pKeyValuesData->FindKey("AmmoData");

	// This will go through the values for ammo amounts the player spawns with
	for (KeyValues *pAmmo = pAmmoData->GetFirstValue(); pAmmo; pAmmo = pAmmo->GetNextValue()) 
	{
		//DevMsg("Ammo: %s Amount: %i\n", pAmmo->GetName(), pAmmo->GetInt());

		if (m_iNumAmmos < MAX_WEAPONS_PER_CLASS) 
		{
			Q_strncpy(m_aAmmos[ m_iNumAmmos ].m_szAmmoType, pAmmo->GetName(), MAX_PLAYERCLASS_STRING);
			m_aAmmos[ m_iNumAmmos ].m_iAmount = pAmmo->GetInt();

			//DevMsg("Ammo: %s Ammount: %i\n", m_aAmmos[ m_iNumAmmos ].m_szAmmoType, m_aAmmos[ m_iNumAmmos ].m_iAmount);

			m_iNumAmmos++;
		}
	}

	KeyValues *pMaxAmmoData = pKeyValuesData->FindKey("MaxAmmoData");
	int iMaxNumAmmos = 0;

	// This will go through the values for ammo amounts the player spawns with
	for (KeyValues *pAmmo = pMaxAmmoData->GetFirstValue(); pAmmo; pAmmo = pAmmo->GetNextValue()) 
	{
		//DevMsg("Ammo: %s Amount: %i\n", pAmmo->GetName(), pAmmo->GetInt());

		if (iMaxNumAmmos < MAX_WEAPONS_PER_CLASS) 
		{			
			const char *pszAmmo = pAmmo->GetName();
			int iVal = pAmmo->GetInt();

			if (Q_strcmp(AMMO_SHELLS, pszAmmo) == 0) 
				m_iMaxShells = iVal;
			else if (Q_strcmp(AMMO_CELLS, pszAmmo) == 0) 
				m_iMaxCells = iVal;
			else if (Q_strcmp(AMMO_NAILS, pszAmmo) == 0) 
				m_iMaxNails = iVal;
			else if (Q_strcmp(AMMO_ROCKETS, pszAmmo) == 0) 
				m_iMaxRockets = iVal;
			else if (Q_strcmp(AMMO_DETPACK, pszAmmo) == 0) 
				m_iMaxDetpack = iVal;
			else if (Q_strcmp(AMMO_MANCANNON, pszAmmo) == 0)
				m_iMaxManCannon = iVal;

			iMaxNumAmmos++;
		}
	}
}

//----------------------------------------------------------------------------
// Purpose: Create a new class info for this player
//----------------------------------------------------------------------------
CFFPlayerClassInfo * CreatePlayerClassInfo() 
{
	return new CFFPlayerClassInfo;
}
