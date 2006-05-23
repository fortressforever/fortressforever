// =============== Fortress Forever ==============
// ======== A modification for Half-Life 2 =======
//
// @file ff_minecart.cpp
// @author Patrick O'Leary (Mulchman) 
// @date 5/21/2006
// @brief Mine cart (vphysics vrooom!!!)
//
// REVISIONS
// ---------
//	5/21/2006, Mulchman: 
//		First created

#include "cbase.h"
#include "ff_minecart.h"
#include "ff_player.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar minecart_pushvelmul( "ffdev_minecart_pushvelmul", "3500", FCVAR_NONE, "Mine Cart push velocity multiplier." );

//=============================================================================
//
// Class CFFMineCart tables
//
//=============================================================================

LINK_ENTITY_TO_CLASS( ff_minecart, CFFMineCart );
PRECACHE_REGISTER( ff_minecart );

BEGIN_DATADESC( CFFMineCart )
	DEFINE_KEYFIELD( m_bDisabled, FIELD_BOOLEAN, "start_disabled" ),
	DEFINE_FUNCTION( OnUse ),
END_DATADESC()

//=============================================================================
//
// Class CFFMineCart
//
//=============================================================================

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CFFMineCart::CFFMineCart( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
CFFMineCart::~CFFMineCart( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Precaache stuff
//-----------------------------------------------------------------------------
void CFFMineCart::Precache( void )
{
	PrecacheModel( FF_MINECART_MODEL );

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFFMineCart::Spawn( void )
{
	Precache();
	BaseClass::Spawn();

	SetMoveType( MOVETYPE_VPHYSICS );
	SetSolid( SOLID_VPHYSICS );
	SetCollisionGroup( COLLISION_GROUP_PLAYER );
	SetModel( FF_MINECART_MODEL );	

	m_takedamage = DAMAGE_EVENTS_ONLY;

	if( CreateVPhysics() )
	{
		IPhysicsObject *pPhysics = VPhysicsGetObject();
		if( pPhysics )
		{
			if( !m_bDisabled )
				pPhysics->Wake();

			pPhysics->EnableCollisions( !m_bDisabled );
			pPhysics->EnableMotion( !m_bDisabled );
			pPhysics->EnableDrag( !m_bDisabled );
			pPhysics->EnableGravity( !m_bDisabled );

			pPhysics->SetMass( 800.0f );
		}
		else
		{
			Warning( "[Mine Cart] Physics init'd but failed to get a physics object!\n" );
		}
	}

	SetUse( &CFFMineCart::OnUse );
}

//-----------------------------------------------------------------------------
// Purpose: Init physics
//-----------------------------------------------------------------------------
bool CFFMineCart::CreateVPhysics( void )
{
	// If not reacting to physics, drop to ground on spawn
	if( m_bDisabled )
	{
		if( UTIL_DropToFloor( this, MASK_SOLID ) == 0 )
		{
			Warning( "Item %s fell out of level at %f,%f,%f\n", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z );
			UTIL_Remove( this );
			return false;
		}
	}

	VPhysicsInitNormal( SOLID_VPHYSICS, 0, true );

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: When the object gets +use'd
//-----------------------------------------------------------------------------
void CFFMineCart::OnUse( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	if( !PhysicsEnabled() )
		return;

	if( pActivator && pActivator->IsPlayer() )
	{
		VPhysicsGetObject()->ApplyForceCenter( ( GetAbsOrigin() - ToFFPlayer( pActivator )->GetAbsOrigin() ) * minecart_pushvelmul.GetFloat() );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Check if we can react to physics
//-----------------------------------------------------------------------------
bool CFFMineCart::PhysicsEnabled( void ) const
{
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if( !pPhysics )
		return false;

	if( !pPhysics->IsAsleep() )
		return false;

	if( !pPhysics->IsMoveable() )
		return false;

	return true;
}
