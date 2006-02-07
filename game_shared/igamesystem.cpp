//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Deals with singleton  
//
// $Revision: $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "igamesystem.h"
#include "engine/IVEngineCache.h"
#include "utlvector.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Pointer to a member method of IGameSystem
typedef void (IGameSystem::*GameSystemFunc_t)();

// Used to invoke a method of all added Game systems in order
static void InvokeMethod( GameSystemFunc_t f );

// Used to invoke a method of all added Game systems in reverse order
static void InvokeMethodReverseOrder( GameSystemFunc_t f );

static bool s_bSystemsInitted = false; 

// List of all installed Game systems
static CUtlVector<IGameSystem*> s_GameSystems( 0, 4 );

// The map name
static char* s_pMapName = 0;


//-----------------------------------------------------------------------------
// Auto-registration of game systems
//-----------------------------------------------------------------------------
static	CAutoGameSystem *s_pSystemList = NULL;

CAutoGameSystem::CAutoGameSystem()
{
	// If s_GameSystems hasn't been initted yet, then add ourselves to the global list
	// because we don't know if the constructor for s_GameSystems has happened yet.
	// Otherwise, we can add ourselves right into that list.
	if ( s_bSystemsInitted )
	{
		s_GameSystems.AddToTail( this );
	}
	else
	{
		m_pNext = s_pSystemList;
		s_pSystemList = this;
	}
}


//-----------------------------------------------------------------------------
// destructor, cleans up automagically....
//-----------------------------------------------------------------------------
IGameSystem::~IGameSystem()
{
	Remove( this );
}


//-----------------------------------------------------------------------------
// Adds a system to the list of systems to run
//-----------------------------------------------------------------------------
void IGameSystem::Add( IGameSystem* pSys )
{
	s_GameSystems.AddToTail( pSys );
}


//-----------------------------------------------------------------------------
// Removes a system from the list of systems to update
//-----------------------------------------------------------------------------
void IGameSystem::Remove( IGameSystem* pSys )
{
	s_GameSystems.FindAndRemove( pSys );
}

//-----------------------------------------------------------------------------
// Removes *all* systems from the list of systems to update
//-----------------------------------------------------------------------------
void IGameSystem::RemoveAll(  )
{
	s_GameSystems.RemoveAll();
}


//-----------------------------------------------------------------------------
// Client systems can use this to get at the map name
//-----------------------------------------------------------------------------
char const*	IGameSystem::MapName()
{
	return s_pMapName;
}


//-----------------------------------------------------------------------------
// Invokes methods on all installed game systems
//-----------------------------------------------------------------------------
bool IGameSystem::InitAllSystems()
{
	int i;

	// first add any auto systems to the end
	CAutoGameSystem *pSystem = s_pSystemList;
	while ( pSystem )
	{
		if ( s_GameSystems.Find( pSystem ) == s_GameSystems.InvalidIndex() )
		{
			Add( pSystem );
		}
		else
		{
			DevWarning( 1, "AutoGameSystem already added to game system list!!!\n" );
		}
		pSystem = pSystem->m_pNext;
	}
	s_pSystemList = NULL;

	// Now remember that we are initted so new CAutoGameSystems will add themselves automatically.
	s_bSystemsInitted = true;

	for ( i = 0; i < s_GameSystems.Count(); ++i )
	{
		if ( !s_GameSystems[i]->Init() )
			return false;
	}

	return true;
}

void IGameSystem::ShutdownAllSystems()
{
	InvokeMethodReverseOrder( &IGameSystem::Shutdown );
}

void IGameSystem::LevelInitPreEntityAllSystems( char const* pMapName )
{
	// Store off the map name
	if ( s_pMapName )
	{
		delete[] s_pMapName;
	}

	int len = Q_strlen(pMapName) + 1;
	s_pMapName = new char [ len ];
	Q_strncpy( s_pMapName, pMapName, len );

	InvokeMethod( &IGameSystem::LevelInitPreEntity );
}

void IGameSystem::LevelInitPostEntityAllSystems( void )
{
	InvokeMethod( &IGameSystem::LevelInitPostEntity );
}

void IGameSystem::LevelShutdownPreEntityAllSystems()
{
	InvokeMethodReverseOrder( &IGameSystem::LevelShutdownPreEntity );
}

void IGameSystem::LevelShutdownPostEntityAllSystems()
{
	InvokeMethodReverseOrder( &IGameSystem::LevelShutdownPostEntity );

	if ( s_pMapName )
	{
		delete[] s_pMapName;
		s_pMapName = 0;
	}
}

void IGameSystem::OnSaveAllSystems()
{
	InvokeMethod( &IGameSystem::OnSave );
}

void IGameSystem::OnRestoreAllSystems()
{
	InvokeMethod( &IGameSystem::OnRestore );
}

void IGameSystem::SafeRemoveIfDesiredAllSystems()
{
	InvokeMethodReverseOrder( &IGameSystem::SafeRemoveIfDesired );
}

#ifdef CLIENT_DLL

void IGameSystem::PreRenderAllSystems()
{
	InvokeMethod( &IGameSystem::PreRender );
}

void IGameSystem::UpdateAllSystems( float frametime )
{
	SafeRemoveIfDesiredAllSystems();

	int i;
	for ( i = 0; i < s_GameSystems.Count(); ++i )
	{
		s_GameSystems[i]->Update( frametime );
	}
}

#else

void IGameSystem::FrameUpdatePreEntityThinkAllSystems()
{
	InvokeMethod( &IGameSystem::FrameUpdatePreEntityThink );
}

void IGameSystem::FrameUpdatePostEntityThinkAllSystems()
{
	SafeRemoveIfDesiredAllSystems();

	InvokeMethod( &IGameSystem::FrameUpdatePostEntityThink );
}

void IGameSystem::PreClientUpdateAllSystems() 
{
	InvokeMethod( &IGameSystem::PreClientUpdate );
}

#endif


//-----------------------------------------------------------------------------
// Invokes a method on all installed game systems in proper order
//-----------------------------------------------------------------------------
void InvokeMethod( GameSystemFunc_t f )
{
	int i;
	int c = s_GameSystems.Count();
	for ( i = 0; i < c ; ++i )
	{
		IGameSystem *sys = s_GameSystems[i];
		engineCache->EnterCriticalSection();
		(sys->*f)();
		engineCache->ExitCriticalSection();
	}
}

//-----------------------------------------------------------------------------
// Invokes a method on all installed game systems in reverse order
//-----------------------------------------------------------------------------
void InvokeMethodReverseOrder( GameSystemFunc_t f )
{
	int i;
	int c = s_GameSystems.Count();
	for ( i = c; --i >= 0; )
	{
		IGameSystem *sys = s_GameSystems[i];
		engineCache->EnterCriticalSection();
		(sys->*f)();
		engineCache->ExitCriticalSection();
	}
}


