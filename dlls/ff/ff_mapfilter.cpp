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
//
//	9/6/2007, Mulchman:
//		Blatantly ripping off http://developer.valvesoftware.com/wiki/Resetting_Maps_and_Entities
//		to try and fix the crashing problem on large maps with lots of entities

#include "cbase.h"
#include "ff_mapfilter.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//=============================================================================
//
// Class CFFMapEntityRef
//
//=============================================================================

// Singleton
CUtlLinkedList< CFFMapEntityRef, unsigned short > g_MapEntityRefs;

//=============================================================================
//
// Class CFFMapEntityFilter
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFMapEntityFilter::CFFMapEntityFilter( void )
{
	/*Initialize();*/
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CFFMapEntityFilter::~CFFMapEntityFilter( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Used to check if we should reset an entity or not
//-----------------------------------------------------------------------------
bool CFFMapEntityFilter::ShouldCreateEntity( const char *pszClassname )
{
	/*if( m_vKeepList.Find( pszClassname ) >= 0 )
		return false;
	else
		return true;*/

	// Don't recreate the preserved entities.
	if( !FindInList( g_MapEntityFilterKeepList, pszClassname ) )
	{
		return true;
	}
	else
	{
		// Increment our iterator since it's not going to call CreateNextEntity for this ent.
		if( m_iIterator != g_MapEntityRefs.InvalidIndex() )
			m_iIterator = g_MapEntityRefs.Next( m_iIterator );

		return false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Creates the next entity in our stored list
//-----------------------------------------------------------------------------
CBaseEntity *CFFMapEntityFilter::CreateNextEntity( const char *pszClassname )
{
	/*return CreateEntityByName( pszClassname );*/

	if( m_iIterator == g_MapEntityRefs.InvalidIndex() )
	{
		// We should never reach this point - g_MapEntityRefs should have been filled
		// when we loaded the map due to the use of CHDNMapLoadFilter, but we cover ourselves
		// by checking here.
		Assert( m_iIterator != g_MapEntityRefs.InvalidIndex() );
		return NULL;
	}
	else
	{
		CFFMapEntityRef &ref = g_MapEntityRefs[m_iIterator];
		m_iIterator = g_MapEntityRefs.Next( m_iIterator ); // Seek to the next entity.

		if( (ref.m_iEdict == -1) || engine->PEntityOfEntIndex( ref.m_iEdict ) )
		{
			// the entities previous edict has been used for whatever reason,
			// so just create it and use any spare edict slot
			return CreateEntityByName( pszClassname );
		}
		else
		{
			// The entity's edict slot was free, so we put it back where it came from.
			return CreateEntityByName( pszClassname, ref.m_iEdict );
		}
	}
}

////-----------------------------------------------------------------------------
//// Purpose: Add an entity to our list
////-----------------------------------------------------------------------------
//void CFFMapEntityFilter::AddKeep( const char *pszClassname )
//{
//	m_vKeepList.Insert( pszClassname );
//}
//
////-----------------------------------------------------------------------------
//// Purpose: Loads all the entities we want to keep
////-----------------------------------------------------------------------------
//void CFFMapEntityFilter::Initialize( void )
//{
//	int iCount = 0;
//	while( g_MapFilterKeepList[ iCount ] )
//	{
//		AddKeep( g_MapFilterKeepList[ iCount ] );
//		iCount++;
//	}
//}

//-----------------------------------------------------------------------------
// Purpose: Searches the provided array for the compare string
// Remarks:	Mulch: 9/6/2007: Blatantly stolen from: 
//			http://developer.valvesoftware.com/wiki/Resetting_Maps_and_Entities
//-----------------------------------------------------------------------------
bool FindInList( const char *s_List[], const char *compare )
{
	int index = 0;

	while( s_List[index] )
	{
		if( Q_strcmp( s_List[index], compare ) == 0 )
			return true;

		index++;
	}

	return false;
}
