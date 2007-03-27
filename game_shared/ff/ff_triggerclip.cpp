// =============== Fortress Forever ===============
// ======== A modification for Half-Life 2 ========
// 
// @file game_shared\ff\ff_triggerclip.cpp
// @author Patrick (Mulchman) O'Leary
// @date March 27, 2007
// @brief Lua controlled clip brush
// 
// Implements a basic Lua controlled clip brush
// 
// Revisions
// ---------
// 03/27/2007:	Mulchman: Initial creation

#include "cbase.h"
#include "ff_triggerclip.h"

#ifdef GAME_DLL
#include "ff_scriptman.h"
#include "ff_luacontext.h"
//#include "ff_player.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#undef MINMAX_H
#undef min
#undef max

#include "luabind/luabind.hpp"
#include "luabind/iterator_policy.hpp"
#endif

#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Network tables
//-----------------------------------------------------------------------------
IMPLEMENT_NETWORKCLASS_ALIASED( FFTriggerClip, DT_FFTriggerClip ) 

BEGIN_NETWORK_TABLE( CFFTriggerClip, DT_FFTriggerClip )
#ifdef CLIENT_DLL 
	RecvPropInt( RECVINFO( m_iClipMask ) ),
#else
	SendPropInt( SENDINFO( m_iClipMask ) ),
#endif
END_NETWORK_TABLE() 

#ifdef GAME_DLL
LINK_ENTITY_TO_CLASS( trigger_ff_clip, CFFTriggerClip );
PRECACHE_REGISTER( trigger_ff_clip );
#endif

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFTriggerClip::CFFTriggerClip( void )
{
	m_iClipMask = 0;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CFFTriggerClip::~CFFTriggerClip( void )
{
}

void CFFTriggerClip::Spawn( void )
{
#ifdef GAME_DLL
	SetLocalAngles( vec3_angle );
	SetMoveType( MOVETYPE_PUSH );  // so it doesn't get pushed by anything
	SetModel( STRING( GetModelName() ) );

	// If it can't move/go away, it's really part of the world
	AddFlag( FL_WORLDBRUSH );

	SetSolid( SOLID_BSP );
	VPhysicsInitStatic();

	CFFLuaSC hContext;
	_scriptman.RunPredicates_LUA( this, &hContext, "spawn" );
#endif
}

#ifdef CLIENT_DLL

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFTriggerClip::OnDataChanged( DataUpdateType_t updateType ) 
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	if( updateType == DATA_UPDATE_CREATED ) 
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

#else

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFTriggerClip::LUA_SetClipFlags( const luabind::adl::object& hTable )
{
	SetClipMask( 0 );

	if( hTable.is_valid() && ( luabind::type( hTable ) == LUA_TTABLE ) )
	{
		// Iterate through table
		for( luabind::iterator ib( hTable ), ie; ib != ie; ++ib )
		{
			luabind::adl::object value = *ib;

			if( luabind::type( value ) == LUA_TNUMBER )
			{
				try
				{
					int iFlag = luabind::object_cast<int>( value );
					AddClipMask( iFlag );
				}
				catch( ... )
				{
					// Don't care
				}
			}
		}
	}
}

#endif
