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

//=============================================================================
//
// Class CFFMapFilter
//
//=============================================================================
const char *g_MapFilterKeepList[] =
{
	"CFFPlayer",
	NULL
};

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
