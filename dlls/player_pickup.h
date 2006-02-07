//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: APIs for player pickup of physics objects
//
//=============================================================================//

#ifndef PLAYER_PICKUP_H
#define PLAYER_PICKUP_H
#ifdef _WIN32
#pragma once
#endif

// Reasons behind a pickup
enum PhysGunPickup_t
{
	PICKED_UP_BY_CANNON,
	PUNTED_BY_CANNON
};

// Reasons behind a drop
enum PhysGunDrop_t
{
	DROPPED_BY_PLAYER,
	THROWN_BY_PLAYER,
	DROPPED_BY_CANNON,
	LAUNCHED_BY_CANNON,
};

void PlayerPickupObject( CBasePlayer *pPlayer, CBaseEntity *pObject );
void Pickup_ForcePlayerToDropThisObject( CBaseEntity *pTarget );

void Pickup_OnPhysGunDrop( CBaseEntity *pDroppedObject, CBasePlayer *pPlayer, PhysGunDrop_t reason );
void Pickup_OnPhysGunPickup( CBaseEntity *pPickedUpObject, CBasePlayer *pPlayer, PhysGunPickup_t reason = PICKED_UP_BY_CANNON );
bool Pickup_OnAttemptPhysGunPickup( CBaseEntity *pPickedUpObject, CBasePlayer *pPlayer, PhysGunPickup_t reason = PICKED_UP_BY_CANNON );
bool Pickup_GetPreferredCarryAngles( CBaseEntity *pObject, CBasePlayer *pPlayer, matrix3x4_t &localToWorld, QAngle &outputAnglesWorldSpace );
bool Pickup_ForcePhysGunOpen( CBaseEntity *pObject, CBasePlayer *pPlayer );
bool Pickup_ShouldPuntUseLaunchForces( CBaseEntity *pObject );
AngularImpulse Pickup_PhysGunLaunchAngularImpulse( CBaseEntity *pObject );

class IPlayerPickupVPhysics
{
public:
	// Callbacks for the physgun/cannon picking up an entity
	virtual bool			OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason = PICKED_UP_BY_CANNON ) = 0;
	virtual void			OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason = PICKED_UP_BY_CANNON ) = 0;
	virtual void			OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason ) = 0;
	virtual bool			HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer = NULL ) = 0;
	virtual const QAngle	&PreferredCarryAngles( void )  = 0;
	virtual bool			ForcePhysgunOpen( CBasePlayer *pPlayer ) = 0;
	virtual AngularImpulse	PhysGunLaunchAngularImpulse() = 0;
	virtual bool			ShouldPuntUseLaunchForces() = 0;
};

class CDefaultPlayerPickupVPhysics : public IPlayerPickupVPhysics
{
public:
	virtual bool			OnAttemptPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason = PICKED_UP_BY_CANNON ) { return true; }
	virtual void			OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason = PICKED_UP_BY_CANNON ) {}
	virtual void			OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t reason ) {}
	virtual bool			HasPreferredCarryAnglesForPlayer( CBasePlayer *pPlayer ) { return false; }
	virtual const QAngle	&PreferredCarryAngles( void ) { return vec3_angle; }
	virtual bool			ForcePhysgunOpen( CBasePlayer *pPlayer ) { return false; }
	virtual AngularImpulse	PhysGunLaunchAngularImpulse() { return RandomAngularImpulse( -600, 600 ); }
	virtual bool			ShouldPuntUseLaunchForces() { return false; }
};

#endif // PLAYER_PICKUP_H
