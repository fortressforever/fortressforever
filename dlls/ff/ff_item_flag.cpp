/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
/// 
/// @file \Steam\SteamApps\SourceMods\FortressForeverCode\dlls\ff\ff_item_flag.cpp
/// @author Kevin Hjelden (FryGuy)
/// @date Jun. 29, 2005
/// @brief Flag Item (generic lua entity)
/// 
/// Implements a basic flag item for the lua scripting system to implement
/// 
/// Revisions
/// ---------
/// Jun. 29, 2005	FryGuy: Initial Creation

#include "cbase.h"
#include "ff_item_flag.h"
#include "ff_entity_system.h"
#include "tier0/memdbgon.h"

#define ITEM_PICKUP_BOX_BLOAT		24
#define FLAG_WEAPON					"ff_weapon_flag"

// --> Mirv: Added for client class
IMPLEMENT_SERVERCLASS_ST( CFFItemFlag, DT_FFItemFlag )
	SendPropFloat( SENDINFO( m_flThrowTime ) )
END_SEND_TABLE( )
// <-- Mirv: Added for client class

BEGIN_DATADESC( CFFItemFlag )
	DEFINE_ENTITYFUNC( OnTouch ),
	DEFINE_THINKFUNC( OnThink ),
	DEFINE_THINKFUNC( OnRespawn ),
END_DATADESC();

LINK_ENTITY_TO_CLASS( info_ff_script, CFFItemFlag );
PRECACHE_REGISTER( info_ff_script );

CFFItemFlag::CFFItemFlag( )
{
	m_spawnflags = 0;
	m_vStartOrigin = Vector();
}

void CFFItemFlag::Precache( void )
{
	PrecacheModel( FLAG_MODEL );
	entsys.RunPredicates( this, NULL, "precache" );
}

bool CFFItemFlag::CreateItemVPhysicsObject( void )
{
	// make the entity stop following the player if necessary
	if (GetOwnerEntity())
	{
		FollowEntity(NULL);
		SetOwnerEntity(NULL);
	}

	// move it to where it's supposed to respawn at
	SetAbsOrigin(m_vStartOrigin);
	SetLocalAngles(m_vStartAngles);

	SetMoveType( MOVETYPE_NONE );

	// Bug #0000131: Ammo, health and armor packs stop rockets
	// Projectiles won't collide with COLLISION_GROUP_WEAPON
	// We don't want to set as not-solid because we need to trace it for sniper rifle dot
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE|FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_WEAPON);

	// If it's not physical, drop it to the floor
	if (UTIL_DropToFloor(this, MASK_SOLID) == 0)
	{
		Warning( "xxxx Item %s fell out of level at %f,%f,%f\n", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		UTIL_Remove( this );
		return false;
	}

	// make it respond to touches
	//SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CollisionProp()->UseTriggerBounds( false, ITEM_PICKUP_BOX_BLOAT );
	SetTouch( &CFFItemFlag::OnTouch );

	return true;
}

void CFFItemFlag::Spawn( void )
{
	Precache();

	// Bug #0000131: Ammo, health and armor packs stop rockets
	// Projectiles won't collide with COLLISION_GROUP_WEAPON
	// We don't want to set as not-solid because we need to trace it for sniper rifle dot
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE|FSOLID_TRIGGER);
	SetCollisionGroup(COLLISION_GROUP_WEAPON);
	SetModel( FLAG_MODEL );

//	DevMsg("[ff_item_flag] Spawn\n");
	entsys.RunPredicates( this, NULL, "spawn" );

	m_vStartOrigin = GetAbsOrigin();
	m_vStartAngles = GetAbsAngles();

	CreateItemVPhysicsObject();

	m_pLastOwner = NULL;
	m_flThrowTime = 0.0f;
}

void CFFItemFlag::OnTouch( CBaseEntity *pOther )
{
	if ( !pOther->IsPlayer() )
		return;

	//DevMsg("[ff_item_flag] Entity Touch");
	CFFPlayer *pFFPlayer = ToFFPlayer(pOther);

	if(pFFPlayer)
	{
		// touch event
		if (m_pLastOwner != pFFPlayer || m_flThrowTime + 7.0f < gpGlobals->curtime)
			entsys.RunPredicates( this, pOther, "touch" );
	}
}

void CFFItemFlag::OnPlayerDied( CFFPlayer *pPlayer )
{
	if (this->GetOwnerEntity() == pPlayer)
	{
		//DevMsg("[ff_item_flag] Player Death\n");
	
		// see when lua wants us to return this item
		entsys.RunPredicates( this, pPlayer, "ownerdie" );
	}
}

void CFFItemFlag::Pickup(CFFPlayer *pFFPlayer)
{
	SetOwnerEntity(pFFPlayer);
	SetTouch(NULL);

	FollowEntity(pFFPlayer, true);

	pFFPlayer->GiveNamedItem(FLAG_WEAPON);

	// stop the return timer
	SetThink( NULL );
	SetNextThink( gpGlobals->curtime );
}

void CFFItemFlag::Respawn(float delay)
{
	CreateItemVPhysicsObject();

	SetTouch(NULL);
	AddEffects( EF_NODRAW );

	SetThink ( &CFFItemFlag::OnRespawn );
	SetNextThink( gpGlobals->curtime + delay );

}

void CFFItemFlag::OnRespawn( void )
{
	CreateItemVPhysicsObject();

	if ( IsEffectActive( EF_NODRAW ) )
	{
		// changing from invisible state to visible.
		RemoveEffects( EF_NODRAW );
		DoMuzzleFlash();
		entsys.RunPredicates( this, NULL, "materialize" );
	}

	SetTouch( &CFFItemFlag::OnTouch );

	m_pLastOwner = NULL;
	m_flThrowTime = 0.0f;
}

void CFFItemFlag::Drop( float delay, float speed )
{
	CFFPlayer *ffplayer = ToFFPlayer(GetFollowedEntity());

	if (ffplayer)
		ffplayer->TakeNamedItem(FLAG_WEAPON);

	// stop following
	FollowEntity(NULL);
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	CollisionRulesChanged();

	CFFPlayer *owner = ToFFPlayer(GetOwnerEntity());

	Assert(owner);

	if (!owner)
		return;

	// inherit the owner's motion
	SetGravity( 1.0 );
	SetAbsOrigin(owner->GetAbsOrigin());

	QAngle ang = owner->EyeAngles();
	ang.y += 90.0f;

	SetAbsAngles(QAngle(0, owner->EyeAngles().y + 90.0f, 0));

	Vector vel = owner->GetAbsVelocity();

	if (speed)
	{
		Vector vecForward;
		owner->EyeVectors(&vecForward);

		vel += vecForward * speed;
	}

	// Mirv: Don't allow a downwards velocity as this will make it float instead
	if (vel.z < 1.0f)
		vel.z = 1.0f;

	SetAbsVelocity(vel);

	// make it respond to touch again
	//SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CollisionProp()->UseTriggerBounds( false, ITEM_PICKUP_BOX_BLOAT );
	SetTouch( &CFFItemFlag::OnTouch );		// |-- Mirv: Account for GCC strictness

	// set to respawn in [delay] seconds
	if (delay > 0)
	{
		SetThink( &CFFItemFlag::OnThink );	// |-- Mirv: Account for GCC strictness
		SetNextThink( gpGlobals->curtime + delay );
	}

	m_pLastOwner = owner;
	m_flThrowTime = gpGlobals->curtime;

	// it's dropped, so don't need a parent anymore
	SetOwnerEntity(NULL);
}

CBaseEntity* CFFItemFlag::Return( void )
{
	CFFPlayer *ffplayer = ToFFPlayer(GetFollowedEntity());

	if (ffplayer)
		ffplayer->TakeNamedItem(FLAG_WEAPON);

	// #0000220: Capping flag simultaneously with gren explosion results in flag touch.
	// set this to curtime so that it will take a few seconds before it becomes
	// eligible to be picked up again.
	CFFPlayer *owner = ToFFPlayer(GetOwnerEntity());

	if (owner)
	{
		m_flThrowTime = gpGlobals->curtime;
		m_pLastOwner = owner;
	}

	CreateItemVPhysicsObject();

	return this;
}

void CFFItemFlag::OnThink( void )
{
	entsys.RunPredicates( this, NULL, "onreturn" );

	Return();
}

void CFFItemFlag::SetSpawnFlags( int flags )
{
	m_spawnflags = flags;
}
