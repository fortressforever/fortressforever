/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life2 ========
///
/// @file ff_playerclass_parse.j
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date March 30, 2005
/// @brief The FF player class script parser declaration
///
/// REVISIONS
/// ---------
/// Mar 30, 2005 Mirv: Initial implementation
//
//	12/6/2007, Mulchman:
//		Added man cannon stuff

#ifndef FF_PLAYERCLASS_PARSE_H
#define FF_PLAYERCLASS_PARSE_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"

class IFileSystem;

typedef unsigned short PLAYERCLASS_FILE_INFO_HANDLE;

#define MAX_PLAYERCLASS_STRING			80
#define MAX_PLAYERCLASS_STRING_LONG		512
#define MAX_WEAPONS_PER_CLASS			32 //lol

#define PLAYERCLASS_PRINTNAME_MISSING "!!! Missing printname on playerclass"

class KeyValues;

// Beg: Mulchman
struct PlayerClassInfo_AmmoInfo_s
{
	char	m_szAmmoType[ MAX_PLAYERCLASS_STRING ];
	int		m_iAmount;
};
// End: Mulchman

//============================================================================
// CFFPlayerClassInfo
//============================================================================
class CFFPlayerClassInfo
{
public:

	CFFPlayerClassInfo();
	
	void Parse(KeyValues *pKeyValuesData, const char *szPlayerClassName);


public:	
	bool					bParsedScript;
	bool					bLoadedHudElements;

	// Variables for each class
	int						m_iSlot;

	char					m_szClassName[MAX_PLAYERCLASS_STRING];
	char					m_szPrintName[MAX_PLAYERCLASS_STRING];
	char					m_szRole[MAX_PLAYERCLASS_STRING];
	char					m_szDescription[MAX_PLAYERCLASS_STRING_LONG];
	char					m_szModel[MAX_PLAYERCLASS_STRING];
	
	int						m_iMaxArmour;
	int						m_iInitialArmour;
	int						m_iArmourType;

	int						m_iHealth;
	
	int						m_iSpeed;

	int						m_iFirepower;

	// std::vector<const char *>		m_vecWeapons;
	// std::vector<const char *>		m_vecSkills;

	char					m_aWeapons[MAX_WEAPONS_PER_CLASS][MAX_PLAYERCLASS_STRING];
	char					m_aSkills[MAX_WEAPONS_PER_CLASS][MAX_PLAYERCLASS_STRING];
	PlayerClassInfo_AmmoInfo_s	m_aAmmos[ MAX_WEAPONS_PER_CLASS ];

	int						m_iNumWeapons, m_iNumSkills, m_iNumAmmos;

	char					m_szPrimaryClassName[MAX_PLAYERCLASS_STRING];
	int						m_iPrimaryInitial;
	int						m_iPrimaryMax;

	char					m_szSecondaryClassName[MAX_PLAYERCLASS_STRING];
	int						m_iSecondaryInitial;
	int						m_iSecondaryMax;

	// Added by Mulchman
	int						m_iMaxShells;
	int						m_iMaxNails;
	int						m_iMaxCells;
	int						m_iMaxRockets;
	int						m_iMaxDetpack;
	int						m_iMaxManCannon;
};

// The playerclass parse function
bool ReadPlayerClassDataFromFileForSlot(IFileSystem * filesystem, const char *szPlayerClassName, 
	PLAYERCLASS_FILE_INFO_HANDLE *phandle, const unsigned char *pICEKey = NULL);

// If playerclass info has been loaded for the specified class name, this returns it.
PLAYERCLASS_FILE_INFO_HANDLE LookupPlayerClassInfoSlot(const char *name);

CFFPlayerClassInfo *GetFilePlayerClassInfoFromHandle(PLAYERCLASS_FILE_INFO_HANDLE handle);
PLAYERCLASS_FILE_INFO_HANDLE GetInvalidPlayerClassInfoHandle();
void PrecacheFilePlayerClassInfoDatabase(IFileSystem *filesystem, const unsigned char *pICEKey);


// 
// Read a possibly-encrypted KeyValues file in. 
// If pICEKey is NULL, then it appends .txt to the filename and loads it as an unencrypted file.
// If pICEKey is non-NULL, then it appends .ctx to the filename and loads it as an encrypted file.
//
// (This should be moved into a more appropriate place).
//
KeyValues * ReadEncryptedKVPlayerClassFile(IFileSystem *filesystem, const char *szFilenameWithoutExtension, const unsigned char *pICEKey);

extern CFFPlayerClassInfo * CreatePlayerClassInfo();

#endif // FF_PLAYERCLASS_PARSE_H
