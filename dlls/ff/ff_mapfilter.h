// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_mapfilter.h
// @author Patrick O'Leary (Mulchman) 
// @date 8/17/2006
// @brief Map filter
//
// REVISIONS
// ---------
//	8/17/2006, Mulchman: 
//		First created - my birthday!
//
//	9/6/2007, Mulchman:
//		Blatantly ripping off http://developer.valvesoftware.com/wiki/Resetting_Maps_and_Entities
//		to try and fix the crashing problem on large maps with lots of entities

#ifndef FF_MAPFILTER_H
#define FF_MAPFILTER_H

#ifdef _WIN32
#pragma once
#endif

#include "mapentities.h"
#include "UtlSortVector.h"

//=============================================================================
//
// Class CFFMapEntityRef
//
//=============================================================================
class CFFMapEntityRef
{
public:
	int m_iEdict;
	int m_iSerialNumber;
};

// Singleton
extern CUtlLinkedList< CFFMapEntityRef, unsigned short > g_MapEntityRefs;

//=============================================================================
//
// Class CFFMapLoadEntityFilter
//
//=============================================================================
class CFFMapLoadEntityFilter : public IMapEntityFilter
{
public:
	virtual bool ShouldCreateEntity( const char *pClassname )
	{
		// During map load, create all the entities.
		return true;
	}

	virtual CBaseEntity* CreateNextEntity( const char *pClassname )
	{
		// create each entity in turn and an instance of CFFMapEntityRef
		CBaseEntity *pRet = CreateEntityByName( pClassname );

		CFFMapEntityRef ref;
		ref.m_iEdict = -1;
		ref.m_iSerialNumber = -1;

		// if the new entity is valid store the entity information in ref
		if ( pRet )
		{
			ref.m_iEdict = pRet->entindex();

			if ( pRet->edict() )
				ref.m_iSerialNumber = pRet->edict()->m_NetworkSerialNumber;
		}

		// add the new ref to the linked list and return the entity
		g_MapEntityRefs.AddToTail( ref );
		return pRet;
	}
};

//=============================================================================
//
// Class CFFMapEntityFilter
//
//=============================================================================
class CFFMapEntityFilter : public IMapEntityFilter
{
public:
	CFFMapEntityFilter( void );
	virtual ~CFFMapEntityFilter( void );

//public:
//	typedef const char* szPtr;
//	class CFFMapFilterLess
//	{
//	public:
//		bool Less( const szPtr &src1, const szPtr &src2, void *pCtx )
//		{
//			if( Q_strcmp( src1, src2 ) >= 0 )
//				return false;
//			else
//				return true;
//		}
//	};

public:
	virtual bool ShouldCreateEntity( const char *pszClassname );
	virtual CBaseEntity *CreateNextEntity( const char *pszClassname );
	/*void AddKeep( const char *pszClassname );*/

public:
	int m_iIterator; // Iterator into g_MapEntityRefs.

//private:
//	void Initialize( void );

//private:
//	CUtlSortVector< const char *, CFFMapFilterLess > m_vKeepList;

};

// TODO: Add stuff you don't want deleted upon ff_restartround
static const char *g_MapEntityFilterKeepList[] =
{	
	"ai_network",
	"ai_hint",
	"ambient_generic",
	"ff_gamerules",
	"ff_team_manager",
	"player_manager",
	"env_soundscape",
	"env_soundscape_proxy",
	"env_soundscape_triggerable",
	"env_sun",
	"env_wind",
	"env_fog_controller",
	"ff_entity_system_helper",	// ignore this (entity system spawns it)
	"ff_mapguide",
	"func_brush",
	"func_wall",
	"func_illusionary",
	"func_rotating",
	"ff_gamerules",
	"ff_radiotagdata",
	"infodecal",
	"info_projecteddecal",
	"info_node",
	"info_target",
	"info_node_hint",
	"info_spectator",
	"info_map_parameters",
	"keyframe_rope",
	"move_rope",
	"info_ladder",
	"player",
	"point_viewcontrol",
	"scene_manager",
	"shadow_control",
	"sky_camera",
	"soundent",
	"trigger_soundscape",
	"viewmodel",
	"predicted_viewmodel",
	"worldspawn",
	"point_devshot_camera",
	"light",
	"light_spot",
	"env_soundscape",
	"env_lightglow",
	"water_lod_control",
	"info_target",
	"func_door",
	"func_illusionary",
	"path_mapguide",
	"",
	NULL
};

// Mulch: 9/6/2007: Blatantly stolen from: http://developer.valvesoftware.com/wiki/Resetting_Maps_and_Entities
bool FindInList( const char *s_List[], const char *compare );

#endif // FF_MAPFILTER_H
