
// ff_lualib_util.cpp

//---------------------------------------------------------------------------
// includes
//---------------------------------------------------------------------------
// includes
#include "cbase.h"
#include "ff_lualib.h"
#include "ff_player.h"
#include "ff_projectile_base.h"
#include "ff_item_flag.h"
#include "ff_triggerclip.h"
#include "ff_utils.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"
#include "luabind/iterator_policy.hpp"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------------------------
using namespace luabind;

typedef std::vector< CBaseEntity * > CollectionContainer;

//============================================================================
// CFFEntity_CollectionFilter
// Purpose: this is a fake class to expose collection filter enums to lua
//============================================================================
class CFFEntity_CollectionFilter
{
public:
};

enum CollectionFilter
{
	CF_NONE = 0,

	CF_PLAYERS,
	CF_HUMAN_PLAYERS,
	CF_BOT_PLAYERS,
	CF_PLAYER_SCOUT,
	CF_PLAYER_SNIPER,
	CF_PLAYER_SOLDIER,
	CF_PLAYER_DEMOMAN,
	CF_PLAYER_MEDIC,
	CF_PLAYER_HWGUY,
	CF_PLAYER_PYRO,
	CF_PLAYER_SPY,
	CF_PLAYER_ENGY,
	CF_PLAYER_CIVILIAN,

	CF_TEAMS,
	CF_TEAM_SPECTATOR,
	CF_TEAM_BLUE,
	CF_TEAM_RED,
	CF_TEAM_YELLOW,
	CF_TEAM_GREEN,

	CF_PROJECTILES,
	CF_GRENADES,
	CF_INFOSCRIPTS,

	CF_INFOSCRIPT_CARRIED,
	CF_INFOSCRIPT_DROPPED,
	CF_INFOSCRIPT_RETURNED,
	CF_INFOSCRIPT_ACTIVE,
	CF_INFOSCRIPT_INACTIVE,
	CF_INFOSCRIPT_REMOVED,

	CF_BUILDABLES,
	CF_BUILDABLE_DISPENSER,
	CF_BUILDABLE_SENTRYGUN,
	CF_BUILDABLE_DETPACK,

	CF_TRACE_BLOCK_WALLS,

	CF_MAX_FLAG
};

//-----------------------------------------------------------------------------
// Purpose: Parse collection filter flags
//-----------------------------------------------------------------------------
bool CollectionFilterParseFlags( const luabind::adl::object& table, bool *pbFlags )
{
	if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( table ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			if( luabind::type( val ) == LUA_TNUMBER )
			{
				// Set to out of bounds in case the cast fails
				int iIndex = -1;

				try
				{
					iIndex = luabind::object_cast< int >( val );
				}
				catch( ... )
				{
				}

				// Make sure within bounds
				if( ( iIndex >= 0 ) && ( iIndex < CF_MAX_FLAG ) )
					pbFlags[ iIndex ] = true;
			}
		}

		return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PassesCollectionFilter_Players( CBaseEntity *pEntity, bool _nobots = false, bool _nohumans = false )
{
	if( !pEntity )
		return false;

	if( !pEntity->IsPlayer() )
		return false;

	if(_nobots && ToBasePlayer(pEntity)->IsBot())
		return false;

	if(_nohumans && !ToBasePlayer(pEntity)->IsBot())
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PassesCollectionFilter_PlayerClass( CBaseEntity *pEntity, int iClassSlot )
{
	if( pEntity && pEntity->IsPlayer() )
	{
		CFFPlayer *pPlayer = ToFFPlayer( pEntity );
		if( pPlayer )
			return pPlayer->GetClassSlot() == iClassSlot;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PassesCollectionFilter_Team( CBaseEntity *pEntity )
{
	if( pEntity )
	{
		if( ( pEntity->GetTeamNumber() >= TEAM_SPECTATOR ) && ( pEntity->GetTeamNumber() <= TEAM_GREEN ) )
			return true;
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PassesCollectionFilter_TeamNum( CBaseEntity *pEntity, int iTeamNum )
{
	if( pEntity )
		return pEntity->GetTeamNumber() == iTeamNum;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Check an entity against some collection filter flags
//-----------------------------------------------------------------------------
bool PassesCollectionFilter( CBaseEntity *pEntity, bool *pbFlags )
{
	// Bail now if none is set
	if( pbFlags[ CF_NONE ] )
		return true;

	if( pbFlags[ CF_PLAYERS ] )
	{
		if( !PassesCollectionFilter_Players( pEntity ) )
			return false;
	}
	else if( pbFlags[ CF_HUMAN_PLAYERS ] )
	{
		if( !PassesCollectionFilter_Players( pEntity, true, false ) )
			return false;
	} 
	else if( pbFlags[ CF_BOT_PLAYERS ] )
	{
		if( !PassesCollectionFilter_Players( pEntity, false, true ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_SCOUT ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_SCOUT ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_SNIPER ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_SNIPER ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_SOLDIER ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_SOLDIER ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_DEMOMAN ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_DEMOMAN ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_MEDIC ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_MEDIC ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_HWGUY ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_HWGUY ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_PYRO ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_PYRO ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_SPY ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_SPY ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_ENGY ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_ENGINEER ) )
			return false;
	}

	if( pbFlags[ CF_PLAYER_CIVILIAN ] )
	{
		if( !PassesCollectionFilter_PlayerClass( pEntity, CLASS_CIVILIAN ) )
			return false;
	}

	if( pbFlags[ CF_TEAMS ] )
	{
		if( !PassesCollectionFilter_Team( pEntity ) )
			return false;
	}

	if( pbFlags[ CF_TEAM_SPECTATOR ] )
	{
		if( !PassesCollectionFilter_TeamNum( pEntity, TEAM_SPECTATOR ) )
			return false;
	}

	if( pbFlags[ CF_TEAM_BLUE ] )
	{
		if( !PassesCollectionFilter_TeamNum( pEntity, TEAM_BLUE ) )
			return false;
	}

	if( pbFlags[ CF_TEAM_RED ] )
	{
		if( !PassesCollectionFilter_TeamNum( pEntity, TEAM_RED ) )
			return false;
	}

	if( pbFlags[ CF_TEAM_YELLOW ] )
	{
		if( !PassesCollectionFilter_TeamNum( pEntity, TEAM_YELLOW ) )
			return false;
	}

	if( pbFlags[ CF_TEAM_GREEN ] )
	{
		if( !PassesCollectionFilter_TeamNum( pEntity, TEAM_GREEN ) )
			return false;
	}

	if( pbFlags[ CF_PROJECTILES ] )
	{
		if( dynamic_cast< CFFProjectileBase * >( pEntity ) == NULL )
			return false;
	}

	if( pbFlags[ CF_GRENADES ] )
	{
		if( !( pEntity->GetFlags() & FL_GRENADE ) )
			return false;
	}

	if( pbFlags[ CF_INFOSCRIPTS ] )
	{
		if( pEntity->Classify() != CLASS_INFOSCRIPT )
			return false;
	}

	if( pbFlags[ CF_INFOSCRIPT_CARRIED ] )
	{
		if( pEntity->Classify() != CLASS_INFOSCRIPT )
			return false;

		CFFInfoScript *pInfoScript = dynamic_cast< CFFInfoScript * >( pEntity );
		if( !pInfoScript->IsCarried() )
			return false;
	}

	if( pbFlags[ CF_INFOSCRIPT_DROPPED ] )
	{
		if( pEntity->Classify() != CLASS_INFOSCRIPT )
			return false;

		CFFInfoScript *pInfoScript = dynamic_cast< CFFInfoScript * >( pEntity );
		if( !pInfoScript->IsDropped() )
			return false;
	}

	if( pbFlags[ CF_INFOSCRIPT_RETURNED ] )
	{
		if( pEntity->Classify() != CLASS_INFOSCRIPT )
			return false;

		CFFInfoScript *pInfoScript = dynamic_cast< CFFInfoScript * >( pEntity );
		if( !pInfoScript->IsReturned() )
			return false;
	}

	if( pbFlags[ CF_INFOSCRIPT_ACTIVE ] )
	{
		if( pEntity->Classify() != CLASS_INFOSCRIPT )
			return false;

		CFFInfoScript *pInfoScript = dynamic_cast< CFFInfoScript * >( pEntity );
		if( !pInfoScript->IsActive() )
			return false;
	}

	if( pbFlags[ CF_INFOSCRIPT_INACTIVE ] )
	{
		if( pEntity->Classify() != CLASS_INFOSCRIPT )
			return false;

		CFFInfoScript *pInfoScript = dynamic_cast< CFFInfoScript * >( pEntity );
		if( !pInfoScript->IsInactive() )
			return false;
	}

	if( pbFlags[ CF_INFOSCRIPT_REMOVED ] )
	{
		if( pEntity->Classify() != CLASS_INFOSCRIPT )
			return false;

		CFFInfoScript *pInfoScript = dynamic_cast< CFFInfoScript * >( pEntity );
		if( !pInfoScript->IsRemoved() )
			return false;
	}

	if( pbFlags[ CF_BUILDABLES ] )
	{
		if( ( pEntity->Classify() != CLASS_DISPENSER ) ||
			( pEntity->Classify() != CLASS_SENTRYGUN ) ||
			( pEntity->Classify() != CLASS_DETPACK ) )
			return false;
	}

	if( pbFlags[ CF_BUILDABLE_DISPENSER ] )
	{
		if( pEntity->Classify() != CLASS_DISPENSER )
			return false;
	}

	if( pbFlags[ CF_BUILDABLE_SENTRYGUN ] )
	{
		if( pEntity->Classify() != CLASS_SENTRYGUN )
			return false;
	}

	if( pbFlags[ CF_BUILDABLE_DETPACK ] )
	{
		if( pEntity->Classify() != CLASS_DETPACK )
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool PassesCollectionFilter_Trace( CBaseEntity *pEntity, bool *pbFlags, const Vector& vecTraceOrigin )
{
	if( PassesCollectionFilter( pEntity, pbFlags ) )
	{
		if( pbFlags[ CF_TRACE_BLOCK_WALLS ] )
		{
			trace_t tr;
			UTIL_TraceLine( vecTraceOrigin, pEntity->GetAbsOrigin(), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr );

			if( !FF_TraceHitWorld( &tr ) )
				return true;
		}
	}

	return false;
}

//============================================================================
// CFFEntity_Collection
// Purpose: basically a list/vector of entities that lua can have access to
//============================================================================
class CFFEntity_Collection
{
public:
	CFFEntity_Collection( void );
	CFFEntity_Collection( const luabind::adl::object& table );

	~CFFEntity_Collection( void );

	void AddItem( CBaseEntity *pItem = NULL );
	void AddItem( const luabind::adl::object& table );
	
	void RemoveItem( CBaseEntity *pItem = NULL );
	void RemoveItem( const luabind::adl::object& table );
	
	void RemoveAllItems( void );
	
	bool IsEmpty( void ) const		{ return m_vCollection.empty(); }
	
	int Count( void ) const			{ return ( int )m_vCollection.size(); }

	bool HasItem( CBaseEntity *pItem = NULL );
	bool HasItem( const luabind::adl::object& table );

	CBaseEntity *GetItem( CBaseEntity *pItem = NULL );
	CBaseEntity *GetItem( const luabind::adl::object& table );

	void GetInSphere( CBaseEntity *pObject, float flRadius, const luabind::adl::object& hFilterTable );
	void GetInSphere( const Vector& vecOrigin, float flRadius, const luabind::adl::object& hFilterTable );
	void GetTouching( CBaseEntity *pTouchee, const luabind::adl::object& hFilterTable );
	void GetByName( const luabind::adl::object& hNameTable );
	void GetByName( const luabind::adl::object& hNameTable, const luabind::adl::object& hFilterTable );	
	void GetByFilter( const luabind::adl::object& hFilterTable );

	CBaseEntity *Element( int iElement );

	CollectionContainer	m_vCollection;

protected:
	CBaseEntity *InternalFindPtr( CBaseEntity *pItem = NULL );
	CollectionContainer::iterator InternalFindItr( CBaseEntity *pItem = NULL );
};

//-----------------------------------------------------------------------------
// Purpose: Constructor - Create an empty collection
//-----------------------------------------------------------------------------
CFFEntity_Collection::CFFEntity_Collection( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Constructor - Create a collection from table
//-----------------------------------------------------------------------------
CFFEntity_Collection::CFFEntity_Collection( const luabind::adl::object& table )
{
	AddItem( table );
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFFEntity_Collection::~CFFEntity_Collection( void )
{
	RemoveAllItems();
}

//-----------------------------------------------------------------------------
// Purpose: Add an item to the collection
//-----------------------------------------------------------------------------
void CFFEntity_Collection::AddItem( CBaseEntity *pItem )
{
	if( !pItem )
		return;

	if( !InternalFindPtr( pItem ) )
		m_vCollection.push_back( pItem );
}

//-----------------------------------------------------------------------------
// Purpose: Add an item to the collection
//-----------------------------------------------------------------------------
void CFFEntity_Collection::AddItem( const luabind::adl::object& table )
{
	if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( table ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			try
			{
				CBaseEntity *pEntity = luabind::object_cast< CBaseEntity * >( val );

				if( pEntity )
					AddItem( pEntity );
			}
			catch( ... )
			{
				Warning( "[Collection] Error in AddItem!\n" );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove an item from the collection
//-----------------------------------------------------------------------------
void CFFEntity_Collection::RemoveItem( CBaseEntity *pItem )
{
	if( !pItem )
		return;

	CollectionContainer::iterator vs = InternalFindItr( pItem );

	if( vs != m_vCollection.end() )
		m_vCollection.erase( vs );
}

//-----------------------------------------------------------------------------
// Purpose: Remove an item from the collection
//-----------------------------------------------------------------------------
void CFFEntity_Collection::RemoveItem( const luabind::adl::object& table )
{
	if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( table ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			try
			{
				CBaseEntity *pEntity = luabind::object_cast< CBaseEntity * >( val );

				if( pEntity )
					RemoveItem( pEntity );
			}
			catch( ... )
			{
				Warning( "[Collection] Error in RemoveItem!\n" );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Remove all items from the collection
//-----------------------------------------------------------------------------
void CFFEntity_Collection::RemoveAllItems( void )
{
	m_vCollection.clear();
}

//-----------------------------------------------------------------------------
// Purpose: See if an item exists
//-----------------------------------------------------------------------------
bool CFFEntity_Collection::HasItem( CBaseEntity *pItem )
{
	if( !pItem )
		return false;

	return InternalFindPtr( pItem ) ? true : false;
}

//-----------------------------------------------------------------------------
// Purpose: See if an item exists
//-----------------------------------------------------------------------------
bool CFFEntity_Collection::HasItem( const luabind::adl::object& table )
{
	if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( table ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			try
			{
				CBaseEntity *pEntity = luabind::object_cast< CBaseEntity * >( val );

				// TODO: Support seeing if all values in table were found
				if( pEntity )
					return HasItem( pEntity );
			}
			catch( ... )
			{
				Warning( "[Collection] Error in HasItem!\n" );
			}
		}
	}

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Return an item from the collection
//-----------------------------------------------------------------------------
CBaseEntity *CFFEntity_Collection::GetItem( CBaseEntity *pItem )
{
	if( !pItem )
		return NULL;

	return InternalFindPtr( pItem );
}

//-----------------------------------------------------------------------------
// Purpose: Return an item from the collection.
//-----------------------------------------------------------------------------
CBaseEntity *CFFEntity_Collection::GetItem( const luabind::adl::object& table )
{
	if( table.is_valid() && ( luabind::type( table ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( table ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			try
			{
				CBaseEntity *pEntity = luabind::object_cast< CBaseEntity * >( val );

				if( pEntity )
				{
					CBaseEntity *pFoundEntity = GetItem( pEntity );
					if( pFoundEntity )
						return pFoundEntity;
				}
			}
			catch( ... )
			{
				Warning( "[Collection] Error in GetItem!\n" );
			}
		}
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Find an item in the collection & return a reference to it
//-----------------------------------------------------------------------------
CBaseEntity *CFFEntity_Collection::InternalFindPtr( CBaseEntity *pItem )
{
	if( !pItem )
		return NULL;

	CollectionContainer::iterator vb = m_vCollection.begin(), ve = m_vCollection.end();

	// Search for pItem
	for( ; vb != ve; vb++ )
	{
		if( *vb == pItem )
			return *vb;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Find an item in the collection & return a iterator to it
//-----------------------------------------------------------------------------
CollectionContainer::iterator CFFEntity_Collection::InternalFindItr( CBaseEntity *pItem )
{
	if( !pItem )
		return m_vCollection.end();

	CollectionContainer::iterator vb = m_vCollection.begin(), ve = m_vCollection.end();

	// Search for pItem
	for( ; vb != ve; vb++ )
	{
		if( *vb == pItem )
			return vb;
	}

	return m_vCollection.end();
}

//-----------------------------------------------------------------------------
// Purpose: Get entities inside a sphere filtered by hFilterTable
//-----------------------------------------------------------------------------
void CFFEntity_Collection::GetInSphere( CBaseEntity *pObject, float flRadius, const luabind::adl::object& hFilterTable )
{
	// Waste!
	if( flRadius < 0 )
		return;

	if( !pObject )
		return;

	bool bFlags[ CF_MAX_FLAG ] = { false };
	if( CollectionFilterParseFlags( hFilterTable, bFlags ) )
	{
		Vector vecOrigin = pObject->GetAbsOrigin();

		if( pObject->IsPlayer() )
			vecOrigin = ToFFPlayer( pObject )->GetAbsOrigin();

		CBaseEntity *pEntity = NULL;
		for( CEntitySphereQuery sphere( vecOrigin, flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( !pEntity )
				continue;

			if( PassesCollectionFilter_Trace( pEntity, bFlags, vecOrigin ) )
				m_vCollection.push_back( pEntity );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get entities inside a sphere filtered by hFilterTable
//-----------------------------------------------------------------------------
void CFFEntity_Collection::GetInSphere( const Vector& vecOrigin, float flRadius, const luabind::adl::object& hFilterTable )
{
	// Waste!
	if( flRadius < 0 )
		return;

	bool bFlags[ CF_MAX_FLAG ] = { false };
	if( CollectionFilterParseFlags( hFilterTable, bFlags ) )
	{
		CBaseEntity *pEntity = NULL;
		for( CEntitySphereQuery sphere( vecOrigin, flRadius ); ( pEntity = sphere.GetCurrentEntity() ) != NULL; sphere.NextEntity() )
		{
			if( !pEntity )
				continue;

			if( PassesCollectionFilter_Trace( pEntity, bFlags, vecOrigin ) )
				m_vCollection.push_back( pEntity );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get any entities that are currently touching pTouchee filter by hFilterTable
//-----------------------------------------------------------------------------
void CFFEntity_Collection::GetTouching( CBaseEntity *pTouchee, const luabind::adl::object& hFilterTable )
{
	if( !pTouchee )
		return;

	bool bFlags[ CF_MAX_FLAG ] = { false };
	if( CollectionFilterParseFlags( hFilterTable, bFlags ) )
	{
		// TODO: Hrm, don't know how possible this is
		// Might have to be trigger_ff_script only (?)
		/*
		CBaseEntity *pEntity = gEntList.FirstEnt();
		while( pEntity )
		{
			if( pEntity->IsCurrentlyTouching() )
			{
				
			}

			pEntity = gEntList.NextEnt( pEntity );
		}
		*/
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get any entities that are named certain things
//-----------------------------------------------------------------------------
void CFFEntity_Collection::GetByName( const luabind::adl::object& hNameTable )
{
	std::vector< std::string > hNames;

	// Grab all the strings out of hNameTable
	if( hNameTable.is_valid() && ( luabind::type( hNameTable ) == LUA_TTABLE ) )
	{
		// Iterate through the table
		for( iterator ib( hNameTable ), ie; ib != ie; ++ib )
		{			
			luabind::adl::object val = *ib;

			try
			{
				std::string szString = luabind::object_cast< std::string >( val );

				if( !szString.empty() )
					hNames.push_back( szString );
			}
			catch( ... )
			{
				Warning( "[Collection] Error in GetByName - item not a string!\n" );
			}
		}
	}

	// Iterate through the entity list looking for items in hNames
	std::vector< std::string >::iterator sb = hNames.begin(), se = hNames.end();

	for( ; sb != se; sb++ )
	{
		// Find the item of name in the entity list
		CBaseEntity *pEntity = gEntList.FindEntityByName( NULL, sb->c_str() );
		while( pEntity )
		{
			m_vCollection.push_back( pEntity );
			pEntity = gEntList.FindEntityByName( pEntity, sb->c_str() );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get any entities that are named certain things
//-----------------------------------------------------------------------------
void CFFEntity_Collection::GetByName( const luabind::adl::object& hNameTable, const luabind::adl::object& hFilterTable )
{
	bool bFlags[ CF_MAX_FLAG ] = { false };
	if( CollectionFilterParseFlags( hFilterTable, bFlags ) )
	{
		std::vector< std::string > hNames;

		// Grab all the strings out of hNameTable
		if( hNameTable.is_valid() && ( luabind::type( hNameTable ) == LUA_TTABLE ) )
		{
			// Iterate through the table
			for( iterator ib( hNameTable ), ie; ib != ie; ++ib )
			{			
				luabind::adl::object val = *ib;

				try
				{
					std::string szString = luabind::object_cast< std::string >( val );

					if( !szString.empty() )
						hNames.push_back( szString );
				}
				catch( ... )
				{
					Warning( "[Collection] Error in GetByName - item not a string!\n" );
				}
			}
		}

		// Iterate through the entity list looking for items in hNames
		std::vector< std::string >::iterator sb = hNames.begin(), se = hNames.end();

		for( ; sb != se; sb++ )
		{
			// Find the item of name in the entity list
			CBaseEntity *pEntity = gEntList.FindEntityByName( NULL, sb->c_str() );
			while( pEntity )
			{
				// See if object passes the filter...
				if( PassesCollectionFilter( pEntity, bFlags ) )
					m_vCollection.push_back( pEntity );

				pEntity = gEntList.FindEntityByName( pEntity, sb->c_str() );
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get entities based on some flags
//-----------------------------------------------------------------------------
void CFFEntity_Collection::GetByFilter( const luabind::adl::object& hFilterTable )
{
	bool bFlags[ CF_MAX_FLAG ] = { false };
	if( CollectionFilterParseFlags( hFilterTable, bFlags ) )
	{
		// NOTE: This might be a little ridiculous...
		// ...iterating through the entire entity list...
		CBaseEntity *pEntity = gEntList.FirstEnt();
		while( pEntity )
		{
			// See if object passes the filter...
			if( PassesCollectionFilter( pEntity, bFlags ) )
				m_vCollection.push_back( pEntity );

			pEntity = gEntList.NextEnt( pEntity );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Return item[ iElement ]
//-----------------------------------------------------------------------------
CBaseEntity *CFFEntity_Collection::Element( int iElement )
{
	if( iElement < 0 )
		return NULL;

	if( iElement > ( Count() - 1 ) )
		return NULL;

	return m_vCollection[ iElement ];
}

//---------------------------------------------------------------------------
void CFFLuaLib::InitUtil(lua_State* L)
{
	ASSERT(L);

	module(L)
	[
		// CFFEntity_Collection
		class_<CFFEntity_Collection>("Collection")
			.def(constructor<>())
			.def(constructor<const luabind::adl::object&>())
			.def_readwrite("items",		&CFFEntity_Collection::m_vCollection, return_stl_iterator)
			.def("AddItem",				(void(CFFEntity_Collection::*)(CBaseEntity*))&CFFEntity_Collection::AddItem)
			.def("AddItem",				(void(CFFEntity_Collection::*)(const luabind::adl::object&))&CFFEntity_Collection::AddItem)
			.def("RemoveItem",			(void(CFFEntity_Collection::*)(CBaseEntity*))&CFFEntity_Collection::RemoveItem)
			.def("RemoveItem",			(void(CFFEntity_Collection::*)(const luabind::adl::object&))&CFFEntity_Collection::RemoveItem)
			.def("RemoveAllItems",		&CFFEntity_Collection::RemoveAllItems)
			.def("IsEmpty",				&CFFEntity_Collection::IsEmpty)
			.def("Count",				&CFFEntity_Collection::Count)
			.def("NumItems",			&CFFEntity_Collection::Count)
			.def("HasItem",				(bool(CFFEntity_Collection::*)(CBaseEntity*))&CFFEntity_Collection::HasItem)
			.def("HasItem",				(bool(CFFEntity_Collection::*)(const luabind::adl::object&))&CFFEntity_Collection::HasItem)
			.def("GetItem",				(CBaseEntity*(CFFEntity_Collection::*)(CBaseEntity*))&CFFEntity_Collection::GetItem)
			.def("GetItem",				(CBaseEntity*(CFFEntity_Collection::*)(const luabind::adl::object&))&CFFEntity_Collection::GetItem)
			.def("Element",				&CFFEntity_Collection::Element)
			.def("GetByName",			(void(CFFEntity_Collection::*)(const luabind::adl::object&))&CFFEntity_Collection::GetByName)
			.def("GetByName",			(void(CFFEntity_Collection::*)(const luabind::adl::object&, const luabind::adl::object&))&CFFEntity_Collection::GetByName)
			.def("GetInSphere",			(void(CFFEntity_Collection::*)(CBaseEntity*, float, const luabind::adl::object&))&CFFEntity_Collection::GetInSphere)
			.def("GetInSphere",			(void(CFFEntity_Collection::*)(const Vector&, float, const luabind::adl::object&))&CFFEntity_Collection::GetInSphere)
			.def("GetByFilter",			&CFFEntity_Collection::GetByFilter),

		// CFFEntity_CollectionFilter
		class_<CFFEntity_CollectionFilter>("CF")
			.enum_("FilterId")
			[
				value("kNone",				CF_NONE),

				value("kPlayers",			CF_PLAYERS),
				value("kHumanPlayers",		CF_HUMAN_PLAYERS),
				value("kBotPlayers",		CF_BOT_PLAYERS),
				value("kPlayerScout",		CF_PLAYER_SCOUT),
				value("kPlayerSniper",		CF_PLAYER_SNIPER),
				value("kPlayerSoldier",		CF_PLAYER_SOLDIER),
				value("kPlayerDemoman",		CF_PLAYER_DEMOMAN),
				value("kPlayerMedic",		CF_PLAYER_DEMOMAN),
				value("kPlayerHWGuy",		CF_PLAYER_HWGUY),
				value("kPlayerPyro",		CF_PLAYER_PYRO),
				value("kPlayerSpy",			CF_PLAYER_SPY),
				value("kPlayerEngineer",	CF_PLAYER_ENGY),
				value("kPlayerCivilian",	CF_PLAYER_CIVILIAN),

				value("kTeams",				CF_TEAMS),
				value("kTeamSpec",			CF_TEAM_SPECTATOR),
				value("kTeamBlue",			CF_TEAM_BLUE),
				value("kTeamRed",			CF_TEAM_RED),
				value("kTeamYellow",		CF_TEAM_YELLOW),
				value("kTeamGreen",			CF_TEAM_GREEN),

				value("kProjectiles",		CF_PROJECTILES),
				value("kGrenades",			CF_GRENADES),
				value("kInfoScipts",		CF_INFOSCRIPTS),

				value("kInfoScript_Carried",	CF_INFOSCRIPT_CARRIED),
				value("kInfoScript_Dropped",	CF_INFOSCRIPT_DROPPED),
				value("kInfoScript_Returned",	CF_INFOSCRIPT_RETURNED),
				value("kInfoScript_Active",		CF_INFOSCRIPT_ACTIVE),
				value("kInfoScript_Inactive",	CF_INFOSCRIPT_INACTIVE),
				value("kInfoScript_Removed",	CF_INFOSCRIPT_REMOVED),

				value("kTraceBlockWalls",	CF_TRACE_BLOCK_WALLS),
				
				value("kBuildables",		CF_BUILDABLES),
				value("kDispenser",			CF_BUILDABLE_DISPENSER),
				value("kSentrygun",			CF_BUILDABLE_SENTRYGUN),
				value("kDetpack",			CF_BUILDABLE_DETPACK)
			]
	];
};
