//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "gameinterface.h"
#include "mapentities.h"
#include "ff_mapfilter.h" // Mulch: 9/6/2007: Added


// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameClients implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameClients::GetPlayerLimits( int& minplayers, int& maxplayers, int &defaultMaxPlayers ) const
{
	minplayers = 2;  // Force multiplayer.
	maxplayers = MAX_PLAYERS;
	defaultMaxPlayers = 16;
}


// -------------------------------------------------------------------------------------------- //
// Mod-specific CServerGameDLL implementation.
// -------------------------------------------------------------------------------------------- //

void CServerGameDLL::LevelInit_ParseAllEntities( const char *pMapEntities )
{
	// Mulch: 9/6/2007: Old code:
	/*MapEntity_ParseAllEntities( pMapEntities, NULL );*/

	// Mulch: 9/6/2007: New blatantly stolen code from: http://developer.valvesoftware.com/wiki/Resetting_Maps_and_Entities
	// Load the entities and build up a new list of the map entities and their starting state in here.
	g_MapEntityRefs.Purge();
	CFFMapLoadEntityFilter filter;
	MapEntity_ParseAllEntities( pMapEntities, &filter );
}


