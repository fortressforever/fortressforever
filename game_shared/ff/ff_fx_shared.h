//========= Copyright � 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#ifndef FX_CS_SHARED_H
#define FX_CS_SHARED_H
#ifdef _WIN32
#pragma once
#endif

// This runs on both the client and the server.
// On the server, it only does the damage calculations.
// On the client, it does all the effects.
void FX_FireBullets( 
	int	iPlayer,
	const Vector &vOrigin,
	const QAngle &vAngles,
	int	iWeaponID,
	int	iMode,
	int iSeed,
	float flSpread,
	float flSniperRifleCharge = 0.0f	// Extra shiz by Mulchman 9/20/2005
	);									// |-- Mirv: Modified a bit


#endif // FX_CS_SHARED_H
