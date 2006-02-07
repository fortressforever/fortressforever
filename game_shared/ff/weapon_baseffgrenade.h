//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_BASEFFGRENADE_H
#define WEAPON_BASEFFGRENADE_H
#ifdef _WIN32
#pragma once
#endif


#include "weapon_ffbase.h"


#ifdef CLIENT_DLL
	
	#define CBaseFFGrenade C_BaseFFGrenade

#endif


class CBaseFFGrenade : public CWeaponFFBase
{
public:
	DECLARE_CLASS( CBaseFFGrenade, CWeaponFFBase );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CBaseFFGrenade();

	virtual void	Precache();

	bool			Deploy();
	bool			Holster( CBaseCombatWeapon *pSwitchingTo );

	void			PrimaryAttack();
	void			SecondaryAttack();

	bool			Reload();

	virtual void	ItemPostFrame();
	
	void			DecrementAmmo( CBaseCombatCharacter *pOwner );
	virtual void	StartGrenadeThrow();
	virtual void	ThrowGrenade();
	virtual void	DropGrenade();

	bool IsPinPulled() const;

#ifndef CLIENT_DLL
	DECLARE_DATADESC();

	virtual bool AllowsAutoSwitchFrom( void ) const;

	int		CapabilitiesGet();
	
	// Each derived grenade class implements this.
	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer );
#endif

protected:
	CNetworkVar( bool, m_bRedraw );	// Draw the weapon again after throwing a grenade
	CNetworkVar( bool, m_bPinPulled );	// Set to true when the pin has been pulled but the grenade hasn't been thrown yet.
	CNetworkVar( float, m_fThrowTime ); // the time at which the grenade will be thrown.  If this value is 0 then the time hasn't been set yet.

private:
	CBaseFFGrenade( const CBaseFFGrenade & ) {}
};


inline bool CBaseFFGrenade::IsPinPulled() const
{
	return m_bPinPulled;
}


#endif // WEAPON_BASEFFGRENADE_H
