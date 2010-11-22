// =============== Fortress Forever ===============
// ======== A modification for Half-Life 2 ========
// 
// @file game_shared\ff\ff_triggerclip.h
// @author Patrick (Mulchman) O'Leary
// @date March 27, 2007
// @brief Lua controlled clip brush
// 
// Implements a basic Lua controlled clip brush
// 
// Revisions
// ---------
// 03/27/2007:	Mulchman: Initial creation

#ifndef FF_TRIGGERCLIP_H
#define FF_TRIGGERCLIP_H

#ifdef _WIN32
#pragma once
#endif

#include "cbase.h"

#ifdef CLIENT_DLL
#define CFFTriggerClip C_FFTriggerClip
#else

namespace luabind
{
	namespace adl
	{
		class object;
	}
}

#endif

//-----------------------------------------------------------------------------
// ClipFlags for Lua
//-----------------------------------------------------------------------------
enum FF_ClipFlags
{

	LUA_CLIP_FLAG_PLAYERS		= 1 << 0,
	LUA_CLIP_FLAG_TEAMBLUE		= 1 << 1,
	LUA_CLIP_FLAG_TEAMRED		= 1 << 2,
	LUA_CLIP_FLAG_TEAMYELLOW	= 1 << 3,
	LUA_CLIP_FLAG_TEAMGREEN		= 1 << 4,
	LUA_CLIP_FLAG_GRENADES		= 1 << 5,
	LUA_CLIP_FLAG_TEAM_ENTITIES	= 1 << 6
};

//-----------------------------------------------------------------------------
// Class CFFTriggerClip
//-----------------------------------------------------------------------------
class CFFTriggerClip : public CBaseEntity
{
public:
	DECLARE_CLASS( CFFTriggerClip, CBaseEntity );
	DECLARE_NETWORKCLASS();

public:
	CFFTriggerClip( void );
	~CFFTriggerClip( void );

	virtual void	Spawn( void );
	virtual Class_T	Classify( void )			{ return CLASS_TRIGGER_CLIP; }

	bool IsClipMaskSet( int iClipMask )	const	{ return ((m_iClipMask & iClipMask) != 0); }
	int GetClipMask( void ) const				{ return m_iClipMask; }

#ifdef CLIENT_DLL
	virtual void	OnDataChanged( DataUpdateType_t updateType );
#else
	void AddClipMask( int iClipMask )			{ m_iClipMask |= iClipMask; }
	void RemoveClipMask( int iClipMask )		{ m_iClipMask &= (~iClipMask); }
	void ToggleClipMask( int iClipMask )		{ m_iClipMask ^= iClipMask; }	
	void SetClipMask( int iClipMask )			{ m_iClipMask = iClipMask; }

	// These functions give Lua access to adjust stuff
	void LUA_SetClipFlags( const luabind::adl::object& hTable );
#endif

private:
	CNetworkVar( int, m_iClipMask );
};

#endif
