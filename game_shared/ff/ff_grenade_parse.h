/********************************************************************
	created:	2006/08/14
	created:	14:8:2006   21:37
	filename: 	f:\ff-svn\code\trunk\game_shared\ff\ff_grenade_parse.h
	file path:	f:\ff-svn\code\trunk\game_shared\ff
	file base:	ff_grenade_parse
	file ext:	h
	author:		Gavin "Mirvin_Monkey" Bramhill
	
	purpose:	
*********************************************************************/


#ifndef FF_GRENADE_PARSE_H
#define FF_GRENADE_PARSE_H
#ifdef _WIN32
#pragma once
#endif

#include "shareddefs.h"

class IFileSystem;

typedef unsigned short GRENADE_FILE_INFO_HANDLE;

#define MAX_GRENADE_STRING			80
#define MAX_GRENADE_STRING_LONG		512
#define MAX_WEAPONS_PER_CLASS			32 //lol

#define GRENADE_PRINTNAME_MISSING "!!! Missing printname on playerclass"

class KeyValues;

//============================================================================
// CFFGrenadeInfo
//============================================================================
class CFFGrenadeInfo
{
public:

	CFFGrenadeInfo();
	
	void Parse(KeyValues *pKeyValuesData, const char *szGrenadeName);

public:	
	bool bParsedScript;

	CHudTexture	*iconHud;
	CHudTexture *iconAmmo;
};

// The playerclass parse function
bool ReadGrenadeDataFromFileForSlot(IFileSystem *filesystem, const char *szGrenadeName, 
	GRENADE_FILE_INFO_HANDLE *phandle, const unsigned char *pICEKey = NULL);

// If playerclass info has been loaded for the specified class name, this returns it.
GRENADE_FILE_INFO_HANDLE LookupGrenadeInfoSlot(const char *name);

CFFGrenadeInfo *GetFileGrenadeInfoFromHandle(GRENADE_FILE_INFO_HANDLE handle);
GRENADE_FILE_INFO_HANDLE GetInvalidGrenadeInfoHandle();
void PrecacheFileGrenadeInfoDatabase(IFileSystem *filesystem, const unsigned char *pICEKey);


// 
// Read a possibly-encrypted KeyValues file in. 
// If pICEKey is NULL, then it appends .txt to the filename and loads it as an unencrypted file.
// If pICEKey is non-NULL, then it appends .ctx to the filename and loads it as an encrypted file.
//
// (This should be moved into a more appropriate place).
//
KeyValues *ReadEncryptedKVGrenadeFile(IFileSystem *filesystem, const char *szFilenameWithoutExtension, const unsigned char *pICEKey);

extern CFFGrenadeInfo *CreateGrenadeInfo();

#endif // FF_GRENADE_PARSE_H
