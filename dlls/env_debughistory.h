//====== Copyright � 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#ifndef ENV_DEBUGHISTORY_H
#define ENV_DEBUGHISTORY_H
#ifdef _WIN32
#pragma once
#endif

enum debughistorycategories_t
{
	HISTORY_ENTITY_IO,
	HISTORY_AI_DECISIONS,
	HISTORY_SCENE_PRINT,
	HISTORY_ALYX_BLIND,		// TEMP: until we find and fix this bug

	// Add new categories here

	MAX_HISTORY_CATEGORIES,
};

#define DISABLE_DEBUG_HISTORY

#ifdef DISABLE_DEBUG_HISTORY
#define ADD_DEBUG_HISTORY( category, line )		((void)0)
#else
#define ADD_DEBUG_HISTORY( category, line )		AddDebugHistoryLine( category, line )
void AddDebugHistoryLine( int iCategory, const char *pszLine );
#endif

#endif // ENV_DEBUGHISTORY_H
