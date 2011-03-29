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
	LUA_CLIP_FLAG_TEAMBLUE					= 1 << 0,
	LUA_CLIP_FLAG_TEAMRED					= 1 << 1,
	LUA_CLIP_FLAG_TEAMYELLOW				= 1 << 2,
	LUA_CLIP_FLAG_TEAMGREEN					= 1 << 3,
	LUA_CLIP_FLAG_PLAYERS					= 1 << 4,
	LUA_CLIP_FLAG_GRENADES					= 1 << 5,
	LUA_CLIP_FLAG_PROJECTILES				= 1 << 6,
	LUA_CLIP_FLAG_BULLETS					= 1 << 7,
	LUA_CLIP_FLAG_BUILDABLES				= 1 << 8,
	LUA_CLIP_FLAG_BUILDABLEWEAPONS			= 1 << 9,
	LUA_CLIP_FLAG_BACKPACKS					= 1 << 10,
	LUA_CLIP_FLAG_INFOSCRIPTS				= 1 << 11,
	LUA_CLIP_FLAG_SPAWNTURRETS				= 1 << 12,
	LUA_CLIP_FLAG_NONPLAYERS				= 1 << 13,
	LUA_CLIP_FLAG_PLAYERSBYTEAM				= 1 << 14,
	LUA_CLIP_FLAG_GRENADESBYTEAM			= 1 << 15,
	LUA_CLIP_FLAG_PROJECTILESBYTEAM			= 1 << 16,
	LUA_CLIP_FLAG_BULLETSBYTEAM				= 1 << 17,
	LUA_CLIP_FLAG_BUILDABLESBYTEAM			= 1 << 18,
	LUA_CLIP_FLAG_BUILDABLEWEAPONSBYTEAM	= 1 << 19,
	LUA_CLIP_FLAG_BACKPACKSBYTEAM			= 1 << 20,
	LUA_CLIP_FLAG_INFOSCRIPTSBYTEAM			= 1 << 21,
	LUA_CLIP_FLAG_SPAWNTURRETSBYTEAM		= 1 << 22,
	LUA_CLIP_FLAG_NONPLAYERSBYTEAM			= 1 << 23
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

//-----------------------------------------------------------------------------
// Standard collision test for trigger_ff_clip collisions
//-----------------------------------------------------------------------------
bool ShouldFFTriggerClipBlock( CFFTriggerClip *pTriggerClip, int iTeam, int iClipMask, int iTeamClipMask );
bool ShouldFFTriggerClipBlock( CFFTriggerClip *pTriggerClip, int iTeam, int iClipMask, int iAllClipMask, int iTeamClipMask );

#endif
