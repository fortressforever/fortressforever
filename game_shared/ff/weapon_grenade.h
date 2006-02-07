//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef WEAPON_GRENADE_H
#define WEAPON_GRENADE_H
#ifdef _WIN32
#pragma once
#endif

#include "weapon_baseffgrenade.h"


#ifdef CLIENT_DLL
	
	#define CFFGrenade C_FFGrenade

#endif

//-----------------------------------------------------------------------------
// Fragmentation grenades
//-----------------------------------------------------------------------------
class CFFGrenade : public CBaseFFGrenade
{
public:
	DECLARE_CLASS( CFFGrenade, CBaseFFGrenade );
	DECLARE_NETWORKCLASS();
	DECLARE_PREDICTABLE();

	CFFGrenade() {}

	virtual FFWeaponID GetWeaponID( void ) const		{ return WEAPON_GRENADE; }

#ifdef CLIENT_DLL

#else
	DECLARE_DATADESC();

	virtual void EmitGrenade( Vector vecSrc, QAngle vecAngles, Vector vecVel, AngularImpulse angImpulse, CBasePlayer *pPlayer );
	
#endif

	CFFGrenade( const CFFGrenade & ) {}
};


#endif // WEAPON_GRENADE_H
