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
#include "tier0/memdbgon.h"

#define ITEM_PICKUP_BOX_BLOAT		24

int ACT_INFO_IDLE;
int ACT_INFO_ROLL;

LINK_ENTITY_TO_CLASS( info_ff_script_animator, CFFInfoScriptAnimator );
PRECACHE_REGISTER( CFFInfoScriptAnimator );

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
			/*
			m_pFFScript->ResetSequenceInfo();
			//Warning( "[m_pffScript] %s\n", STRING( m_pFFScript->GetModelName() ) );
			int nSequence = m_pFFScript->SelectWeightedSequence( ( Activity )ACT_INFO_IDLE );
			m_pFFScript->SetSequence( nSequence );
			*/
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
END_SEND_TABLE()
// <-- Mirv: Added for client class

BEGIN_DATADESC( CFFInfoScript )
	DEFINE_ENTITYFUNC( OnTouch ),
	DEFINE_THINKFUNC( OnThink ),
	DEFINE_THINKFUNC( OnRespawn ),
	DEFINE_THINKFUNC( TempThink ),
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

	m_vecOffset.SetX( entsys.RunPredicates( this, NULL, "attachoffsetforward" ) );
	m_vecOffset.SetY( entsys.RunPredicates( this, NULL, "attachoffsetright" ) );
	m_vecOffset.SetZ( entsys.RunPredicates( this, NULL, "attachoffsetup" ) );	

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

	entsys.RunPredicates( this, NULL, "spawn" );

	m_vStartOrigin = GetAbsOrigin();
	m_vStartAngles = GetAbsAngles();
	m_atStart = true;

	if( entsys.RunPredicates( this, NULL, "hasanimation" ) && !m_bHasAnims )
	{
		m_bHasAnims = true;

		ADD_CUSTOM_ACTIVITY( CFFInfoScript, ACT_INFO_IDLE );
		ADD_CUSTOM_ACTIVITY( CFFInfoScript, ACT_INFO_ROLL );

		m_pAnimator = ( CFFInfoScriptAnimator * )CreateEntityByName( "info_ff_script_animator" );
		if( m_pAnimator )
		{
			//Warning( "[info_ff_script] created animator!\n" );
			m_pAnimator->Spawn();
			m_pAnimator->m_pFFScript = this;
		}		
	}

	if( entsys.RunPredicates( this, NULL, "usephysics" ) && !m_bUsePhysics )
	{
		m_bUsePhysics = true;
	}

	CreateItemVPhysicsObject();

	m_pLastOwner = NULL;
	m_flThrowTime = 0.0f;

	PlayIdleAnim();

	/*
	SetThink( &CFFInfoScript::TempThink );
	SetNextThink( gpGlobals->curtime );
	*/
}

void CFFInfoScript::TempThink( void )
{
	Warning( "[TempThink]\n" );
	StudioFrameAdvance();
	SetNextThink( gpGlobals->curtime );
}

void CFFInfoScript::PlayIdleAnim( void )
{
	if( m_bHasAnims )
	{		
		ResetSequenceInfo();
		m_Activity = ( Activity )ACT_INFO_IDLE;
		m_iSequence = SelectWeightedSequence( m_Activity );
		SetSequence( m_iSequence );
	}
}

void CFFInfoScript::PlayActiveAnim( void )
{
	if( m_bHasAnims )
	{
		ResetSequenceInfo();
		m_Activity = ( Activity )ACT_INFO_ROLL;
		m_iSequence = SelectWeightedSequence( m_Activity );		
		SetSequence( m_iSequence );
	}
}

void CFFInfoScript::OnTouch( CBaseEntity *pOther )
{
	if( !pOther->IsPlayer() )
		return;

	CFFPlayer *pPlayer = ToFFPlayer( pOther );

	if( !pPlayer )
		return;

	entsys.RunPredicates( this, pPlayer, "touch" );
}

void CFFInfoScript::OnPlayerDied( CFFPlayer *pPlayer )
{
	if( GetOwnerEntity() == pPlayer )
	{
		entsys.RunPredicates( this, pPlayer, "ownerdie" );
	}
}

void CFFInfoScript::Pickup(CFFPlayer *pFFPlayer)
{
	SetOwnerEntity( pFFPlayer );
	SetTouch( NULL );

	if( m_bUsePhysics )
	{
		if( VPhysicsGetObject() )
			VPhysicsDestroyObject();
	}

	FollowEntity( pFFPlayer, true );

	// stop the return timer
	SetThink( NULL );
	SetNextThink( gpGlobals->curtime );

	PlayActiveAnim();
}

void CFFInfoScript::Respawn(float delay)
{
	CreateItemVPhysicsObject();

	SetTouch( NULL );
	AddEffects( EF_NODRAW );

	SetThink( &CFFInfoScript::OnRespawn );
	SetNextThink( gpGlobals->curtime + delay );
}

void CFFInfoScript::OnRespawn( void )
{
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

	PlayIdleAnim();
}

void CFFInfoScript::Drop( float delay, float speed )
{
	// stop following
	FollowEntity( NULL );	
	SetMoveType( MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE );
	CollisionRulesChanged();
	
	CFFPlayer *pOwner = ToFFPlayer( GetOwnerEntity() );

	Assert( pOwner );

	if( !pOwner )
		return;

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

		AngularImpulse angImpulse;
		QAngleToAngularImpulse( pOwner->EyeAngles(), angImpulse );

		// This needs to be based on where the player is looking
		// and the angle they're looking (so they can throw upwards)
		Vector vecVelocity = vecDir * ( vel * vel );
		pPhysics->SetVelocity( &vecVelocity, &angImpulse );

		// Stop the sequence if playing
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
	if( delay > 0 )
	{
		SetThink( &CFFInfoScript::OnThink );	// |-- Mirv: Account for GCC strictness
		SetNextThink( gpGlobals->curtime + delay );
	}

	m_pLastOwner = pOwner;
	m_flThrowTime = gpGlobals->curtime;

	SetOwnerEntity( NULL );

	PlayIdleAnim();

	entsys.RunPredicates( this, m_pLastOwner, "ondrop" );
	entsys.RunPredicates( this, m_pLastOwner, "onloseitem" );
}

CBaseEntity* CFFInfoScript::Return( void )
{
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

	PlayIdleAnim();

	return this;
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

//* This stuff is part of baseentity... need to remove
void CFFInfoScript::LUA_SetModel( const char *model )
{
	UTIL_SetModel(this, model);
}
void CFFInfoScript::LUA_SetSkin( int skin )
{
	this->m_nSkin = skin;
}
//*/
