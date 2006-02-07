//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
#include "cbase.h"
#include "player_pickup.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// player pickup utility routine
void Pickup_ForcePlayerToDropThisObject( CBaseEntity *pTarget )
{
	if ( pTarget == NULL )
		return;

	IPhysicsObject *pPhysics = pTarget->VPhysicsGetObject();
	
	if ( pPhysics == NULL )
		return;

	if ( pPhysics->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
	{
		CBasePlayer *pPlayer = UTIL_GetLocalPlayer();
		pPlayer->ForceDropOfCarriedPhysObjects( pTarget );
	}
}


void Pickup_OnPhysGunDrop( CBaseEntity *pDroppedObject, CBasePlayer *pPlayer, PhysGunDrop_t Reason )
{
	IPlayerPickupVPhysics *pPickup = dynamic_cast<IPlayerPickupVPhysics *>(pDroppedObject);
	if ( pPickup )
	{
		pPickup->OnPhysGunDrop( pPlayer, Reason );
	}
}


void Pickup_OnPhysGunPickup( CBaseEntity *pPickedUpObject, CBasePlayer *pPlayer, PhysGunPickup_t reason )
{
	IPlayerPickupVPhysics *pPickup = dynamic_cast<IPlayerPickupVPhysics *>(pPickedUpObject);
	if ( pPickup )
	{
		pPickup->OnPhysGunPickup( pPlayer, reason );
	}
}

bool Pickup_OnAttemptPhysGunPickup( CBaseEntity *pPickedUpObject, CBasePlayer *pPlayer, PhysGunPickup_t reason )
{
	IPlayerPickupVPhysics *pPickup = dynamic_cast<IPlayerPickupVPhysics *>(pPickedUpObject);
	if ( pPickup )
	{
		return pPickup->OnAttemptPhysGunPickup( pPlayer, reason );
	}
	return true;
}

bool Pickup_GetPreferredCarryAngles( CBaseEntity *pObject, CBasePlayer *pPlayer, matrix3x4_t &localToWorld, QAngle &outputAnglesWorldSpace )
{
	IPlayerPickupVPhysics *pPickup = dynamic_cast<IPlayerPickupVPhysics *>(pObject);
	if ( pPickup )
	{
		if ( pPickup->HasPreferredCarryAnglesForPlayer( pPlayer ) )
		{
			outputAnglesWorldSpace = TransformAnglesToWorldSpace( pPickup->PreferredCarryAngles(), localToWorld );
			return true;
		}
	}
	return false;
}

bool Pickup_ForcePhysGunOpen( CBaseEntity *pObject, CBasePlayer *pPlayer )
{
	IPlayerPickupVPhysics *pPickup = dynamic_cast<IPlayerPickupVPhysics *>(pObject);
	if ( pPickup )
	{
		return pPickup->ForcePhysgunOpen( pPlayer );
	}
	return false;
}

AngularImpulse Pickup_PhysGunLaunchAngularImpulse( CBaseEntity *pObject )
{
	IPlayerPickupVPhysics *pPickup = dynamic_cast<IPlayerPickupVPhysics *>(pObject);
	if ( pPickup )
	{
		return pPickup->PhysGunLaunchAngularImpulse();
	}
	return RandomAngularImpulse( -600, 600 );
}

bool Pickup_ShouldPuntUseLaunchForces( CBaseEntity *pObject )
{
	IPlayerPickupVPhysics *pPickup = dynamic_cast<IPlayerPickupVPhysics *>(pObject);
	if ( pPickup )
	{
		return pPickup->ShouldPuntUseLaunchForces();
	}
	return false;
}

