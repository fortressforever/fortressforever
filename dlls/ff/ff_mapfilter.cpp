// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_mapfilter.cpp
// @author Patrick O'Leary (Mulchman) 
// @date 8/17/2006
// @brief Map filter
//
// REVISIONS
// ---------
//	8/17/2006, Mulchman: 
//		First created - my birthday!

#include "cbase.h"
#include "ff_mapfilter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// TODO: Add stuff you don't want deleted upon ff_restartround
const char *g_MapFilterKeepList[] =
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
	"func_brush",
	"func_wall",
	"func_illusionary",
	"func_rotating",
	"ff_gamerules",
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
	"ff_mapguide",
	NULL
};

//=============================================================================
//
// Class CFFMapFilter
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFMapFilter::CFFMapFilter( void )
{
	Initialize();
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CFFMapFilter::~CFFMapFilter( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Used to check if we should reset an entity or not
//-----------------------------------------------------------------------------
bool CFFMapFilter::ShouldCreateEntity( const char *pszClassname )
{
	if( m_vKeepList.Find( pszClassname ) >= 0 )
		return false;
	else
		return true;
}

//-----------------------------------------------------------------------------
// Purpose: Creates the next entity in our stored list
//-----------------------------------------------------------------------------
CBaseEntity *CFFMapFilter::CreateNextEntity( const char *pszClassname )
{
	return CreateEntityByName( pszClassname );
}

//-----------------------------------------------------------------------------
// Purpose: Add an entity to our list
//-----------------------------------------------------------------------------
void CFFMapFilter::AddKeep( const char *pszClassname )
{
	m_vKeepList.Insert( pszClassname );
}

//-----------------------------------------------------------------------------
// Purpose: Loads all the entities we want to keep
//-----------------------------------------------------------------------------
void CFFMapFilter::Initialize( void )
{
	int iCount = 0;
	while( g_MapFilterKeepList[ iCount ] )
	{
		AddKeep( g_MapFilterKeepList[ iCount ] );
		iCount++;
	}
}
