//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "grenade_hopwire.h"
#include "rope.h"
#include "rope_shared.h"
#include "beam_shared.h"
#include "physics.h"
#include "physics_saverestore.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define TETHERHOOK_MODEL	"models/Weapons/w_hopwire.mdl"

//-----------------------------------------------------------------------------
// Tether hook
//-----------------------------------------------------------------------------

class CTetherHook : public CBaseAnimating
{
	DECLARE_CLASS( CTetherHook, CBaseAnimating );
public:
	typedef CBaseAnimating BaseClass;

	bool	CreateVPhysics( void );
	void	Spawn( void );

	virtual void Precache();

	void	SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	void	StartTouch( CBaseEntity *pOther );

	static CTetherHook	*Create( const Vector &origin, const QAngle &angles, CGrenadeHopWire *pOwner );

	void	CreateRope( void );
	void	HookThink( void );

	DECLARE_DATADESC();

private:
	CHandle<CGrenadeHopWire>	m_hTetheredOwner;
	IPhysicsSpring				*m_pSpring;
	CRopeKeyframe				*m_pRope;
	CSprite						*m_pGlow;
	CBeam						*m_pBeam;
	bool						m_bAttached;
};

BEGIN_DATADESC( CTetherHook )
	DEFINE_FIELD( m_hTetheredOwner, FIELD_EHANDLE ),
	DEFINE_PHYSPTR( m_pSpring ),
	DEFINE_FIELD( m_pRope, FIELD_CLASSPTR ),
	DEFINE_FIELD( m_pGlow, FIELD_CLASSPTR ),
	DEFINE_FIELD( m_pBeam, FIELD_CLASSPTR ),
	DEFINE_FIELD( m_bAttached, FIELD_BOOLEAN ),
	DEFINE_FUNCTION( HookThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( tetherhook, CTetherHook );

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CTetherHook::CreateVPhysics()
{
	// Create the object in the physics system
	IPhysicsObject *pPhysicsObject = VPhysicsInitNormal( SOLID_BBOX, 0, false );
	
	// Make sure I get touch called for static geometry
	if ( pPhysicsObject )
	{
		int flags = pPhysicsObject->GetCallbackFlags();
		flags |= CALLBACK_GLOBAL_TOUCH_STATIC;
		pPhysicsObject->SetCallbackFlags(flags);
	}

	return true;
}

void CTetherHook::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "TripwireGrenade.ShootRope" );
	PrecacheScriptSound( "TripwireGrenade.Hook" );

	PrecacheModel( "sprites/rollermine_shock.vmt" );
	PrecacheModel( "sprites/blueflare1.vmt" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTetherHook::Spawn( void )
{
	m_bAttached = false;

	Precache();
	SetModel( TETHERHOOK_MODEL );

	UTIL_SetSize( this, vec3_origin, vec3_origin );

	CreateVPhysics();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTetherHook::CreateRope( void )
{
	//Make sure it's not already there
	if ( m_pRope == NULL )
	{
		//Creat it between ourselves and the owning grenade
		m_pRope = CRopeKeyframe::Create( this, m_hTetheredOwner, 0, 0 );
		
		if ( m_pRope != NULL )
		{
			m_pRope->m_Width		= 0.75;
			m_pRope->m_RopeLength	= 32;
			m_pRope->m_Slack		= 64;

			CPASAttenuationFilter filter( this,"TripwireGrenade.ShootRope" );
			EmitSound( filter, entindex(),"TripwireGrenade.ShootRope" );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void CTetherHook::StartTouch( CBaseEntity *pOther )
{
	if ( m_bAttached == false )
	{
		m_bAttached = true;

		SetVelocity( vec3_origin, vec3_origin );
		SetMoveType( MOVETYPE_NONE );
		VPhysicsDestroyObject();

		EmitSound( "TripwireGrenade.Hook" );

		StopSound( entindex(),"TripwireGrenade.ShootRope" );

		//Make a physics constraint between us and the owner
		if ( m_pSpring == NULL )
		{
			springparams_t spring;

			//FIXME: Make these real
			spring.constant			= 150.0f;
			spring.damping			= 24.0f;
			spring.naturalLength	= 32;
			spring.relativeDamping	= 0.1f;
			spring.startPosition	= GetAbsOrigin();
			spring.endPosition		= m_hTetheredOwner->WorldSpaceCenter();
			spring.useLocalPositions= false;
			
			IPhysicsObject *pEnd	= m_hTetheredOwner->VPhysicsGetObject();

			m_pSpring = physenv->CreateSpring( g_PhysWorldObject, pEnd, &spring );
		}

		SetThink( HookThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
		//UTIL_Remove(this);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &velocity - 
//			&angVelocity - 
//-----------------------------------------------------------------------------
void CTetherHook::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();

	if ( pPhysicsObject != NULL )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CTetherHook::HookThink( void )
{
	if ( m_pBeam == NULL )
	{
		m_pBeam = CBeam::BeamCreate( "sprites/rollermine_shock.vmt", 1.0f );
		m_pBeam->EntsInit( this, m_hTetheredOwner );

		m_pBeam->SetNoise( 0.5f );
		m_pBeam->SetColor( 255, 255, 255 );
		m_pBeam->SetScrollRate( 25 );
		m_pBeam->SetBrightness( 128 );
		m_pBeam->SetWidth( 4.0f );
		m_pBeam->SetEndWidth( 1.0f );
	}

	if ( m_pGlow == NULL )
	{
		m_pGlow = CSprite::SpriteCreate( "sprites/blueflare1.vmt", GetLocalOrigin(), false );

		if ( m_pGlow != NULL )
		{
			m_pGlow->SetParent( this );
			m_pGlow->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
			m_pGlow->SetBrightness( 128, 0.1f );
			m_pGlow->SetScale( 0.5f, 0.1f );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &origin - 
//			&angles - 
//			*pOwner - 
//-----------------------------------------------------------------------------
CTetherHook	*CTetherHook::Create( const Vector &origin, const QAngle &angles, CGrenadeHopWire *pOwner )
{
	CTetherHook *pHook = CREATE_ENTITY( CTetherHook, "tetherhook" );
	
	if ( pHook != NULL )
	{
		pHook->m_hTetheredOwner = pOwner;
		pHook->SetAbsOrigin( origin );
		pHook->SetAbsAngles( angles );
		pHook->SetOwnerEntity( (CBaseEntity *) pOwner );
		pHook->Spawn();
	}

	return pHook;
}

//-----------------------------------------------------------------------------

#define GRENADE_MODEL "models/Weapons/w_hopwire.mdl"

BEGIN_DATADESC( CGrenadeHopWire )
	DEFINE_FIELD( m_nHooksShot, FIELD_INTEGER ),
	DEFINE_FIELD( m_pGlow, FIELD_CLASSPTR ),

	DEFINE_FUNCTION( TetherThink ),
	DEFINE_FUNCTION( CombatThink ),
END_DATADESC()

LINK_ENTITY_TO_CLASS( npc_grenade_hopwire, CGrenadeHopWire );

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopWire::Spawn( void )
{
	Precache();

	SetModel( GRENADE_MODEL );
	SetCollisionGroup( COLLISION_GROUP_PROJECTILE );
	m_nHooksShot	= 0;
	m_pGlow			= NULL;

	CreateVPhysics();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CGrenadeHopWire::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, 0, false );
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopWire::Precache( void )
{
	PrecacheModel( GRENADE_MODEL );
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : timer - 
//-----------------------------------------------------------------------------
void CGrenadeHopWire::SetTimer( float timer )
{
	SetThink( PreDetonate );
	SetNextThink( gpGlobals->curtime + timer );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopWire::CombatThink( void )
{
	if ( m_pGlow == NULL )
	{
		m_pGlow = CSprite::SpriteCreate( "sprites/blueflare1.vmt", GetLocalOrigin(), false );

		if ( m_pGlow != NULL )
		{
			m_pGlow->SetParent( this );
			m_pGlow->SetTransparency( kRenderTransAdd, 255, 255, 255, 255, kRenderFxNone );
			m_pGlow->SetBrightness( 128, 0.1f );
			m_pGlow->SetScale( 1.0f, 0.1f );
		}
	}

	//TODO: Go boom... or something	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopWire::TetherThink( void )
{
	CTetherHook *pHook = NULL;
	static Vector velocity = RandomVector( -1.0f, 1.0f );

	//Create a tether hook
	pHook = (CTetherHook *) CTetherHook::Create( GetLocalOrigin(), GetLocalAngles(), this );

	if ( pHook == NULL )
		return;

	pHook->CreateRope();

	if ( m_nHooksShot % 2 )
	{
		velocity.Negate();
	}
	else
	{
		velocity = RandomVector( -1.0f, 1.0f );
	}

	pHook->SetVelocity( velocity * 1500.0f, vec3_origin );

	m_nHooksShot++;

	if ( m_nHooksShot == 8 )
	{
		//TODO: Play a startup noise
		SetThink( CombatThink );
		SetNextThink( gpGlobals->curtime + 0.1f );
	}
	else
	{
		SetThink( TetherThink );
		SetNextThink( gpGlobals->curtime + random->RandomFloat( 0.1f, 0.3f ) );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &velocity - 
//			&angVelocity - 
//-----------------------------------------------------------------------------
void CGrenadeHopWire::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	
	if ( pPhysicsObject != NULL )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeHopWire::Detonate( void )
{
	Vector	hopVel = RandomVector( -8, 8 );
	hopVel[2] += 800.0f;

	AngularImpulse	hopAngle = RandomAngularImpulse( -300, 300 );

	//FIXME: We should find out how tall the ceiling is and always try to hop halfway

	//Add upwards velocity for the "hop"
	SetVelocity( hopVel, hopAngle );

	//Shoot 4-8 cords out to grasp the surroundings
	SetThink( TetherThink );
	SetNextThink( gpGlobals->curtime + 0.6f );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CBaseGrenade *HopWire_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer )
{
	CGrenadeHopWire *pGrenade = (CGrenadeHopWire *) CBaseEntity::Create( "npc_grenade_hopwire", position, angles, pOwner );
	
	pGrenade->SetTimer( timer );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );

	return pGrenade;
}
