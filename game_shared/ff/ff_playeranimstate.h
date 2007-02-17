//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FF_PLAYERANIMSTATE_H
#define FF_PLAYERANIMSTATE_H
#ifdef _WIN32
#pragma once
#endif


#include "convar.h"
#include "iplayeranimstate.h"
#include "base_playeranimstate.h"


#ifdef CLIENT_DLL
	class C_BaseAnimatingOverlay;
	class C_FFWeaponBase;
	class C_FFPlayer;
	#define CBaseAnimatingOverlay C_BaseAnimatingOverlay
	#define CFFWeaponBase C_FFWeaponBase
	#define CFFPlayer C_FFPlayer
#else
	class CBaseAnimatingOverlay;
	class CFFWeaponBase; 
	class CFFPlayer;
#endif


// When moving this fast, he plays run anim.
#define ARBITRARY_RUN_SPEED		175.0f


enum PlayerAnimEvent_t
{
	PLAYERANIMEVENT_FIRE_GUN_PRIMARY=0,
	PLAYERANIMEVENT_FIRE_GUN_SECONDARY,
	PLAYERANIMEVENT_JUMP,
	PLAYERANIMEVENT_RELOAD,
	
	PLAYERANIMEVENT_COUNT
};


class IFFPlayerAnimState : virtual public IPlayerAnimState
{
public:
	// This is called by both the client and the server in the same way to trigger events for
	// players firing, jumping, etc.
	virtual void DoAnimationEvent( PlayerAnimEvent_t event ) = 0;
};


// We need these from the player
class IFFPlayerAnimStateHelpers
{
public:
	virtual CFFWeaponBase* FFAnim_GetActiveWeapon() = 0;
	virtual bool FFAnim_CanMove() = 0;
	virtual CFFPlayer* FFAnim_GetPlayer() = 0;
};


IFFPlayerAnimState* CreatePlayerAnimState( CBaseAnimatingOverlay *pEntity, IFFPlayerAnimStateHelpers *pHelpers, LegAnimType_t legAnimType, bool bUseAimSequences );

// If this is set, then the game code needs to make sure to send player animation events
// to the local player if he's the one being watched.
extern ConVar cl_showanimstate;


#endif // FF_PLAYERANIMSTATE_H
