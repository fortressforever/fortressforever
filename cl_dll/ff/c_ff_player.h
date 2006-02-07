//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_FF_PLAYER_H
#define C_FF_PLAYER_H
#ifdef _WIN32
#pragma once
#endif


#include "ff_playeranimstate.h"
#include "c_baseplayer.h"
#include "ff_shareddefs.h"
#include "baseparticleentity.h"


class C_FFPlayer : public C_BasePlayer, public IFFPlayerAnimStateHelpers
{
public:
	DECLARE_CLASS( C_FFPlayer, C_BasePlayer );
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
	DECLARE_INTERPOLATION();

	C_FFPlayer();
	~C_FFPlayer();

	static C_FFPlayer* GetLocalFFPlayer();

	virtual const QAngle& GetRenderAngles();
	virtual void UpdateClientSideAnimation();
	virtual void PostDataUpdate( DataUpdateType_t updateType );
	virtual void OnDataChanged( DataUpdateType_t updateType );


// Called by shared code.
public:
	
	// IFFPlayerAnimState overrides.
	virtual CWeaponFFBase* FFAnim_GetActiveWeapon();
	virtual bool FFAnim_CanMove();

	void DoAnimationEvent( PlayerAnimEvent_t event );
	bool ShouldDraw();

	IFFPlayerAnimState *m_PlayerAnimState;

	QAngle	m_angEyeAngles;
	CInterpolatedVar< QAngle >	m_iv_angEyeAngles;

	CNetworkVar( int, m_iThrowGrenadeCounter );	// used to trigger grenade throw animations.
	CNetworkVar( int, m_iShotsFired );	// number of shots fired recently

	EHANDLE	m_hRagdoll;

	CWeaponFFBase *GetActiveFFWeapon() const;

	C_BaseAnimating *BecomeRagdollOnClient( bool bCopyEntity);
	IRagdoll* C_FFPlayer::GetRepresentativeRagdoll() const;

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
	C_FFPlayer( const C_FFPlayer & );
};


inline C_FFPlayer* ToFFPlayer( CBaseEntity *pPlayer )
{
	Assert( dynamic_cast< C_FFPlayer* >( pPlayer ) != NULL );
	return static_cast< C_FFPlayer* >( pPlayer );
}


#endif // C_FF_PLAYER_H
