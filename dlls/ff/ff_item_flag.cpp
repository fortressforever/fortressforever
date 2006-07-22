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
#include "debugoverlay_shared.h"

// Lua includes
extern "C"
{
	#include "lua.h"
	#include "lualib.h"
	#include "lauxlib.h"
}

#include "luabind/luabind.hpp"
#include "luabind/object.hpp"

#include "tier0/memdbgon.h"

#define ITEM_PICKUP_BOX_BLOAT		24

int ACT_INFO_RETURNED;
int ACT_INFO_CARRIED;
int ACT_INFO_DROPPED;

LINK_ENTITY_TO_CLASS( info_ff_script_animator, CFFInfoScriptAnimator );
PRECACHE_REGISTER( info_ff_script_animator );

BEGIN_DATADESC( CFFInfoScriptAnimator )
	DEFINE_THINKFUNC( OnThink ),
END_DATADESC()

void CFFInfoScriptAnimator::Spawn( void )
{
	SetThink( &CFFInfoScriptAnimator::OnThink );
	SetNextThink( gpGlobals->curtime );
}

void CFFInfoScriptAnimator::OnThink( void )
{
	if( m_pFFScript )
	{
		if( m_pFFScript->HasAnimations() )
		{
			m_pFFScript->StudioFrameAdvance();
		}
	}

	SetNextThink( gpGlobals->curtime );
}

// -----------------

// --> Mirv: Added for client class
IMPLEMENT_SERVERCLASS_ST( CFFInfoScript, DT_FFInfoScript )
	SendPropFloat( SENDINFO( m_flThrowTime ) ),
	SendPropVector( SENDINFO( m_vecOffset ), SPROP_NOSCALE ),
	SendPropInt( SENDINFO( m_iGoalState ), 4 ),
	SendPropInt( SENDINFO( m_iPosState ), 4 ),
END_SEND_TABLE()
// <-- Mirv: Added for client class

BEGIN_DATADESC( CFFInfoScript )
	DEFINE_ENTITYFUNC( OnTouch ),
	DEFINE_THINKFUNC( OnThink ),
	DEFINE_THINKFUNC( OnRespawn ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( info_ff_script, CFFInfoScript );
PRECACHE_REGISTER( info_ff_script );

CFFInfoScript::CFFInfoScript( void )
{
	m_pAnimator = NULL;
	m_pLastOwner = NULL;
	m_spawnflags = 0;
	m_vStartOrigin = Vector( 0, 0, 0 );
	m_bHasAnims = false;
	m_bUsePhysics = false;

	// Init the goal state
	m_iGoalState = GS_INACTIVE;
	// Init the position state
	m_iPosState = PS_RETURNED;
}

CFFInfoScript::~CFFInfoScript( void )
{
	m_spawnflags = 0;
	m_vStartOrigin = Vector();
}

void CFFInfoScript::Precache( void )
{
	PrecacheModel( FLAG_MODEL );
	entsys.RunPredicates( this, NULL, "precache" );
}

bool CFFInfoScript::CreateItemVPhysicsObject( void )
{
	if( GetOwnerEntity() )
	{
		FollowEntity( NULL );
		SetOwnerEntity( NULL );
	}

	// move it to where it's supposed to respawn at
	m_atStart = true;
	SetAbsOrigin( m_vStartOrigin );
	SetLocalAngles( m_vStartAngles );

	SetMoveType( MOVETYPE_NONE );

	// Update goal state
	SetInactive();
	// Update position state
	SetReturned();

	// Bug #0000131: Ammo, health and armor packs stop rockets
	// Projectiles won't collide with COLLISION_GROUP_WEAPON
	// We don't want to set as not-solid because we need to trace it for sniper rifle dot
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE | FSOLID_TRIGGER );
	SetCollisionGroup( COLLISION_GROUP_WEAPON );

	// See if the info_ff_script should drop to the ground or not
	if( entsys.RunPredicates( this, NULL, "dropatspawn" ) )
	{
		// If it's not physical, drop it to the floor
		if( UTIL_DropToFloor( this, MASK_SOLID ) == 0 )
		{
			Warning( "[InfoFFScript] Item %s fell out of level at %f,%f,%f\n", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
			UTIL_Remove( this );
			return false;
		}
	}

	// make it respond to touches
	//SetCollisionGroup( COLLISION_GROUP_WEAPON );
	SetTouch( &CFFInfoScript::OnTouch );

	return true;
}

void CFFInfoScript::Spawn( void )
{
	Precache();

	// Check if this object has an attachoffset function and get the value if it does
	//entsys.RunPredicates_Vector( this, NULL, "attachoffset", m_vecOffset.GetForModify() );
	
	// Bug #0000131: Ammo, health and armor packs stop rockets
	// Projectiles won't collide with COLLISION_GROUP_WEAPON
	// We don't want to set as not-solid because we need to trace it for sniper rifle dot
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE | FSOLID_TRIGGER );
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	SetModel( FLAG_MODEL );

	SetBlocksLOS( false );

	// Try and make the flags easier to grab
	CollisionProp()->UseTriggerBounds( true, ITEM_PICKUP_BOX_BLOAT );

	//entsys.RunPredicates( this, NULL, "spawn" );
	//entsys.RunPredicates_Void( this, NULL, "spawn" );
	luabind::adl::object hSpawn;
	entsys.RunPredicates_LUA( this, NULL, "spawn", hSpawn );

	m_vStartOrigin = GetAbsOrigin();
	m_vStartAngles = GetAbsAngles();
	m_atStart = true;

	// Check to see if this object has animations
	entsys.RunPredicates_Bool( this, NULL, "hasanimation", &m_bHasAnims );
	//luabind::adl::object hAnims;
	//entsys.RunPredicates_LUA( this, NULL, "hasanimation", hAnims );

	//Warning( "[ff_item_flag] entity: %s - m_bHasAnims: %s\n", STRING( GetEntityName() ), m_bHasAnims ? "TRUE" : "FALSE" );

	// If using anims, set them up!
	// TODO: Only do this if the "spawn" call was successful so
	// it won't break maps
	if( m_bHasAnims )
	{
		ADD_CUSTOM_ACTIVITY( CFFInfoScript, ACT_INFO_RETURNED );
		ADD_CUSTOM_ACTIVITY( CFFInfoScript, ACT_INFO_DROPPED );
		ADD_CUSTOM_ACTIVITY( CFFInfoScript, ACT_INFO_CARRIED );

		// Set up the animator helper
		m_pAnimator = ( CFFInfoScriptAnimator * )CreateEntityByName( "info_ff_script_animator" );
		if( m_pAnimator )
		{
			m_pAnimator->Spawn();
			m_pAnimator->m_pFFScript = this;
		}
	}

	// Check to see if this object uses physics
	entsys.RunPredicates_Bool( this, NULL, "usephysics", &m_bUsePhysics );
	//luabind::adl::object hOutput;
	//entsys.RunPredicates_LUA( this, NULL, "usephysics", hOutput );

	CreateItemVPhysicsObject();

	m_pLastOwner = NULL;
	m_flThrowTime = 0.0f;

	PlayReturnedAnim();
}

//-----------------------------------------------------------------------------
// Purpose: Play an animation specific to this info_ff_script being dropped
//-----------------------------------------------------------------------------
void CFFInfoScript::PlayDroppedAnim( void )
{
	InternalPlayAnim( ( Activity )ACT_INFO_DROPPED );
}

//-----------------------------------------------------------------------------
// Purpose: Play an animation specific to this info_ff_script being carried
//-----------------------------------------------------------------------------
void CFFInfoScript::PlayCarriedAnim( void )
{
	InternalPlayAnim( ( Activity )ACT_INFO_CARRIED );
}

//-----------------------------------------------------------------------------
// Purpose: Play an animation specific to this info_ff_script being returned
//-----------------------------------------------------------------------------
void CFFInfoScript::PlayReturnedAnim( void )
{
	InternalPlayAnim( ( Activity )ACT_INFO_RETURNED );
}

//-----------------------------------------------------------------------------
// Purpose: Play an animation
//-----------------------------------------------------------------------------
void CFFInfoScript::InternalPlayAnim( Activity hActivity )
{
	if( m_bHasAnims )
	{
		ResetSequenceInfo();
		SetSequence( SelectWeightedSequence( hActivity ) );
	}
}

void CFFInfoScript::OnTouch( CBaseEntity *pEntity )
{
	if( !pEntity )
		return;

	entsys.RunPredicates( this, pEntity, "touch" );
}

void CFFInfoScript::OnOwnerDied( CBaseEntity *pEntity )
{
	if( GetOwnerEntity() == pEntity )
	{
		// Update position state
		SetDropped();
		entsys.RunPredicates( this, pEntity, "onownerdie" );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Added so that players getting forced respawn by ApplyToAll/Team/Player
//			can give their objects a chance to be dropped so we don't run into
//			a situation where you respawn with an info_ff_script and you're not
//			supposed to because forcing a respawn used to not ask lua what to
//			do with objects players were carrying.
//-----------------------------------------------------------------------------
void CFFInfoScript::OnOwnerForceRespawn( CBaseEntity *pEntity )
{
	if( GetOwnerEntity() == pEntity )
	{
		// Update position state
		SetDropped();
		entsys.RunPredicates( this, pEntity, "onownerforcerespawn" );
	}
}

void CFFInfoScript::Pickup( CBaseEntity *pEntity )
{
	// Don't do anything if removed
	if( IsRemoved() )
		return;

	SetOwnerEntity( pEntity );
	SetTouch( NULL );

	if( m_bUsePhysics )
	{
		if( VPhysicsGetObject() )
			VPhysicsDestroyObject();
	}

	FollowEntity( pEntity, true );

	// Goal is active when carried	
	SetActive();
	// Update position state
	SetCarried();

	// stop the return timer
	SetThink( NULL );
	SetNextThink( gpGlobals->curtime );

	PlayCarriedAnim();
}

void CFFInfoScript::Respawn(float delay)
{
	// Don't do anything if removed
	if( IsRemoved() )
		return;

	CreateItemVPhysicsObject();

	SetTouch( NULL );
	AddEffects( EF_NODRAW );

	// During this period until
	// we respawn we're active
	SetActive();

	SetThink( &CFFInfoScript::OnRespawn );
	SetNextThink( gpGlobals->curtime + delay );
}

void CFFInfoScript::OnRespawn( void )
{
	// Don't do anything if removed
	if( IsRemoved() )
		return;

	CreateItemVPhysicsObject();

	if( IsEffectActive( EF_NODRAW ) )
	{
		// changing from invisible state to visible.
		RemoveEffects( EF_NODRAW );
		DoMuzzleFlash();
		entsys.RunPredicates( this, NULL, "materialize" );
	}

	SetTouch( &CFFInfoScript::OnTouch );

	m_pLastOwner = NULL;
	m_flThrowTime = 0.0f;

	PlayReturnedAnim();
}

void CFFInfoScript::Drop( float delay, float speed )
{
	// Don't do anything if removed
	if( IsRemoved() )
		return;

	// stop following
	FollowEntity( NULL );	
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	CollisionRulesChanged();
	
	CFFPlayer *pOwner = ToFFPlayer( GetOwnerEntity() );

	Assert( pOwner );

	if( !pOwner )
		return;

	// Update goal state
	SetActive();
	// Update position state
	SetDropped();

	Vector vecOrigin = pOwner->GetAbsOrigin();

	Vector vecForward, vecRight, vecUp;
	pOwner->GetVectors( &vecForward, &vecRight, &vecUp );

	VectorNormalize( vecForward );
	VectorNormalize( vecRight );
	VectorNormalize( vecUp );

	// Bug #0000429: Flags dropped on death move with the same velocity as the dead player
	Vector vel = Vector( 0, 0, 20.0f ); // owner->GetAbsVelocity();

	if( speed )
		vel += vecForward * speed;

	// Mirv: Don't allow a downwards velocity as this will make it float instead
	if( vel.z < 1.0f )
		vel.z = 1.0f;

	if( m_bUsePhysics )
	{
		// No physics object exists, create it
		if( !VPhysicsGetObject() )
			VPhysicsInitNormal( SOLID_VPHYSICS, GetSolidFlags(), false );

		// No physics object can be created, use normal stuff
		if( !VPhysicsGetObject() )
			m_bUsePhysics = false;
	}

	if( m_bUsePhysics )
	{
		Warning( "[Physics!]\n" );

		IPhysicsObject *pPhysics = VPhysicsGetObject();

		// If we're here, pPhysics won't be NULL
		Assert( pPhysics );

		pPhysics->EnableGravity( true );
		pPhysics->EnableMotion( true );
		pPhysics->EnableCollisions( true );
		pPhysics->EnableDrag( true );

		Vector vecDir;
		AngleVectors( pOwner->EyeAngles(), &vecDir );

		// NOTE: This is the right angle shit... the ball just
		// doesn't want to be thrown that way
		// NDebugOverlay::Line( pOwner->EyePosition(), pOwner->EyePosition() + ( 256.0f * vecDir ), 0, 0, 255, false, 5.0f );

		pPhysics->ApplyForceCenter( ( ( pOwner->GetAbsOrigin() + ( 256.0f * vecDir ) ) - pOwner->GetAbsOrigin() ) * vel * vel );

		//AngularImpulse angImpulse;
		//QAngleToAngularImpulse( pOwner->EyeAngles(), angImpulse );

		// This needs to be based on where the player is looking
		// and the angle they're looking (so they can throw upwards)
		//Vector vecVelocity = vecDir * ( vel * vel );
		//pPhysics->SetVelocity( &vecVelocity, &angImpulse );

		// Stop the sequence if playing
		// NOTE: This doesn't seem to work yet either...
		if( m_bHasAnims )
		{
			ResetSequenceInfo();
			SetCycle( 0 );
			SetSequence( -1 );
		}		
	}
	else
	{
		Warning( "[Physics!] FAILED\n" );

		// inherit the owner's motion
		SetGravity( 1.0 );
		//SetAbsOrigin(owner->GetAbsOrigin());
		SetAbsOrigin( vecOrigin + ( vecForward * m_vecOffset.GetX() ) + ( vecRight * m_vecOffset.GetY() ) + ( vecUp * m_vecOffset.GetZ() ) );

		QAngle vecAngles = pOwner->EyeAngles();
		SetAbsAngles( QAngle( 0, vecAngles.y + 90.0f, 0 ) );

		SetAbsVelocity( vel );
	}

	// make it respond to touch again
	//SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CollisionProp()->UseTriggerBounds( true, ITEM_PICKUP_BOX_BLOAT );
	SetTouch( &CFFInfoScript::OnTouch );		// |-- Mirv: Account for GCC strictness

	// set to respawn in [delay] seconds
	if( delay >= 0 )
	{
		SetThink( &CFFInfoScript::OnThink );	// |-- Mirv: Account for GCC strictness
		SetNextThink( gpGlobals->curtime + delay );
	}

	m_pLastOwner = pOwner;
	m_flThrowTime = gpGlobals->curtime;

	SetOwnerEntity( NULL );

	PlayDroppedAnim();

	entsys.RunPredicates( this, m_pLastOwner, "ondrop" );
	entsys.RunPredicates( this, m_pLastOwner, "onloseitem" );
}

void CFFInfoScript::Return( void )
{
	// Don't do anything if removed
	if( IsRemoved() )
		return;

	// #0000220: Capping flag simultaneously with gren explosion results in flag touch.
	// set this to curtime so that it will take a few seconds before it becomes
	// eligible to be picked up again.
	//*
	CFFPlayer *pOwner = ToFFPlayer( GetOwnerEntity() );

	if( pOwner )
	{
		m_flThrowTime = gpGlobals->curtime;
		m_pLastOwner = pOwner;

		entsys.RunPredicates( this, m_pLastOwner, "onloseitem" );
	}

	CreateItemVPhysicsObject();

	PlayReturnedAnim();
}

void CFFInfoScript::OnThink( void )
{
	entsys.RunPredicates( this, NULL, "onreturn" );

	Return();
}

void CFFInfoScript::SetSpawnFlags( int flags )
{
	m_spawnflags = flags;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CFFInfoScript::ShouldTransmit( const CCheckTransmitInfo *pInfo )
{
	// Force sending even if no model. By default objects
	// without a model aren't sent to the client. And,
	// sometimes we don't want a model.
	return FL_EDICT_ALWAYS;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script currently carried?
//-----------------------------------------------------------------------------
bool CFFInfoScript::IsCarried( void )
{
	return m_iPosState == PS_CARRIED;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script currently dropped?
//-----------------------------------------------------------------------------
bool CFFInfoScript::IsDropped( void )
{
	return m_iPosState == PS_DROPPED;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script currently returned?
//-----------------------------------------------------------------------------
bool CFFInfoScript::IsReturned( void )
{
	return m_iPosState == PS_RETURNED;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script "active"?
//-----------------------------------------------------------------------------
bool CFFInfoScript::IsActive( void )
{
	return m_iGoalState == GS_ACTIVE;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script "inactive"?
//-----------------------------------------------------------------------------
bool CFFInfoScript::IsInactive( void )
{
	return m_iGoalState == GS_INACTIVE;
}

//-----------------------------------------------------------------------------
// Purpose: Is this info_ff_script "removed"?
//-----------------------------------------------------------------------------
bool CFFInfoScript::IsRemoved( void )
{
	return ( m_iGoalState == GS_REMOVED ) || ( m_iPosState == PS_REMOVED );
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFInfoScript::SetActive( void )
{
	m_iGoalState = GS_ACTIVE;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFInfoScript::SetInactive( void )
{
	m_iGoalState = GS_INACTIVE;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFInfoScript::SetRemoved( void )
{
	m_iGoalState = GS_REMOVED;
	m_iPosState = PS_REMOVED;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFInfoScript::SetCarried( void )
{
	m_iPosState = PS_CARRIED;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFInfoScript::SetDropped( void )
{
	m_iPosState = PS_DROPPED;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFInfoScript::SetReturned( void )
{
	m_iPosState = PS_RETURNED;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFInfoScript::LUA_Remove( void )
{
	// This sets the object as "removed"	
	FollowEntity( NULL );
	SetOwnerEntity( NULL );

	SetTouch( NULL );

	SetThink( NULL );
	SetNextThink( gpGlobals->curtime );

	SetRemoved();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CFFInfoScript::LUA_Restore( void )
{
	// This "restores" the item

	// Set some flags so we can call respawn
	SetInactive();
	SetReturned();

	// Respawn the item back at it's starting spot
	Respawn( 0.0f );
}
