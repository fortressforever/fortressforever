//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose:		Player for FF Game
//
// $NoKeywords: $
//=============================================================================//

#ifndef FF_PLAYER_H
#define FF_PLAYER_H
#pragma once


#include "player.h"
#include "server_class.h"
#include "ff_playeranimstate.h"
#include "ff_shareddefs.h"


//=============================================================================
// >> FF Game player
//=============================================================================
class CFFPlayer : public CBasePlayer, public IFFPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( CFFPlayer, CBasePlayer );
	DECLARE_SERVERCLASS();
	DECLARE_PREDICTABLE();

	CFFPlayer();
	~CFFPlayer();

	static CFFPlayer *CreatePlayer( const char *className, edict_t *ed );
	static CFFPlayer* Instance( int iEnt );

	// This passes the event to the client's and server's CPlayerAnimState.
	void DoAnimationEvent( PlayerAnimEvent_t event );

	virtual void FlashlightTurnOn( void );
	virtual void FlashlightTurnOff( void );
	virtual int FlashlightIsOn( void );

	virtual void PreThink();
	virtual void PostThink();
	virtual void Spawn();
	virtual void InitialSpawn();
	virtual void Precache();
	virtual void Event_Killed( const CTakeDamageInfo &info );
	virtual void LeaveVehicle( const Vector &vecExitPoint, const QAngle &vecExitAngles );
	
	CWeaponFFBase* GetActiveFFWeapon() const;
	virtual void	CreateViewModel( int viewmodelindex = 0 );

	virtual void	CheatImpulseCommands( int iImpulse );

	CNetworkVar( int, m_iThrowGrenadeCounter );	// used to trigger grenade throw animations.
	CNetworkQAngle( m_angEyeAngles );	// Copied from EyeAngles() so we can send it to the client.
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	// Tracks our ragdoll entity.
	CNetworkHandle( CBaseEntity, m_hRagdoll );	// networked entity handle 

// In shared code.
public:
	// IFFPlayerAnimState overrides.
	virtual CWeaponFFBase* FFAnim_GetActiveWeapon();
	virtual bool FFAnim_CanMove();
	

	void FireBullet( 
		Vector vecSrc, 
		const QAngle &shootAngles, 
		float vecSpread, 
		int iDamage, 
		int iBulletType,
		CBaseEntity *pevAttacker,
		bool bDoEffects,
		float x,
		float y );

private:

	void CreateRagdollEntity();

	IFFPlayerAnimState *m_PlayerAnimState;
};


inline CFFPlayer *ToFFPlayer( CBaseEntity *pEntity )
{
	if ( !pEntity || !pEntity->IsPlayer() )
		return NULL;

#ifdef _DEBUG
	Assert( dynamic_cast<CFFPlayer*>( pEntity ) != 0 );
#endif
	return static_cast< CFFPlayer* >( pEntity );
}


#endif	// FF_PLAYER_H
