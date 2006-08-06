//	=============== Fortress Forever ==============
//	======== A modification for Half-Life 2 =======
//
//	@file ff_hud_boxes.cpp
//	@author Patrick O'Leary (Mulchman)
//	@date 8/6/2006
//	@brief Loads up information for our vector bg boxes
//
//	REVISIONS
//	---------
//	08/06/2006, Mulchman: 
//		First created

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"
#include "ff_hud_boxes.h"

#include <KeyValues.h>
#include <filesystem.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FF_HUD_BOXES_FILE "scripts/ff_hud_boxes.txt"

IFileSystem **pHudBoxFilesystem = &filesystem;

// TODO: Might need to look up x/y & width/height in the future
bool GetVectorBgBoxInfo( const char *pszBoxName, char *pszFontFile, char& cFontChar )
{
	KeyValues *kv = new KeyValues( "HudBoxes" );
	if( kv->LoadFromFile( ( *pHudBoxFilesystem ), FF_HUD_BOXES_FILE, "MOD" ) )
	{
		for( KeyValues *pEntry = kv->GetFirstSubKey(); pEntry; pEntry = pEntry->GetNextKey() )
		{
			if( !Q_stricmp( pEntry->GetName(), pszBoxName ) )
			{
				for( KeyValues *pItems = pEntry->GetFirstSubKey(); pItems; pItems = pItems->GetNextKey() )
				{
					if( !Q_stricmp( pItems->GetName(), "font" ) )
						Q_snprintf( pszFontFile, sizeof( pszFontFile ), "%s", pItems->GetString() );
					else if( !Q_stricmp( pItems->GetName(), "character" ) )
						cFontChar = pItems->GetString()[ 0 ];
					// TODO: x/y, width/height
				}
			}
		}
	}

	kv->deleteThis();

	return true;
}
