/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_grenade.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF grenade projectile code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged


#include "cbase.h"
#include "ff_projectile_goop.h"
#include "ff_buildableobjects_shared.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"
#include "iefx.h"

#define GOOP_MODEL "models/projectiles/pipe/w_pipe.mdl"

#ifdef GAME_DLL
	#include "smoke_trail.h"

//=============================================================================
// CFFProjectileGoop tables
//=============================================================================

	BEGIN_DATADESC(CFFProjectileGoop) 
		DEFINE_THINKFUNC( GoopThink ), 
		DEFINE_ENTITYFUNC( ExplodeTouch ),
	END_DATADESC() 
#endif

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectileGoop, DT_FFProjectileGoop) 

BEGIN_NETWORK_TABLE(CFFProjectileGoop, DT_FFProjectileGoop) 
END_NETWORK_TABLE() 

LINK_ENTITY_TO_CLASS(ff_projectile_goop, CFFProjectileGoop);
PRECACHE_WEAPON_REGISTER(ff_projectile_goop);

//=============================================================================
// CFFProjectileGoop implementation
//=============================================================================

ConVar ffdev_goop_gravity("ffdev_goop_gravity", "1.0", FCVAR_REPLICATED, ""); // used in .h

ConVar ffdev_goopgun_goopsize("ffdev_goopgun_goopsize", "8", FCVAR_REPLICATED, "(int) size of bounding box, 1-5 are good values");
ConVar ffdev_goopgun_healamount("ffdev_goopgun_healamount", "2", FCVAR_REPLICATED, "(int) health to heal per goop");

#define FF_GOOP_SIZE ffdev_goopgun_goopsize.GetInt()
#define FF_GOOP_HEALTHPERGOOP ffdev_goopgun_healamount.GetInt()

#ifdef GAME_DLL

	//----------------------------------------------------------------------------
	// Purpose: Creata a trail of smoke for the grenade
	//----------------------------------------------------------------------------
	void CFFProjectileGoop::CreateProjectileEffects() 
	{
	}

	//----------------------------------------------------------------------------
	// Purpose: Spawn a grenade, set up model, size, etc
	//----------------------------------------------------------------------------
	void CFFProjectileGoop::Spawn() 
	{
		// Setup
		//SetModel(GOOP_MODEL);
		//m_nSkin = 1;	// Blue skin(#2)
		SetEffects(EF_NODRAW);

		//SetSolidFlags(FSOLID_NOT_STANDABLE);
		SetMoveType(MOVETYPE_FLYGRAVITY);
		SetSolid(SOLID_BBOX);	// So it will collide with physics props!
		SetSolidFlags(FSOLID_NOT_STANDABLE);

		// Hits everything but debris
		SetCollisionGroup(COLLISION_GROUP_PROJECTILE);

		// smaller, cube bounding box so we rest on the ground
		SetSize(Vector(-FF_GOOP_SIZE, -FF_GOOP_SIZE, -FF_GOOP_SIZE), Vector(FF_GOOP_SIZE, FF_GOOP_SIZE, FF_GOOP_SIZE));
		
		SetThink(&CFFProjectileGoop::GoopThink);		// |-- Mirv: Account for GCC strictness
		SetTouch(&CFFProjectileGoop::ExplodeTouch);

		SetNextThink(gpGlobals->curtime);

		// Creates the smoke trail
		CreateProjectileEffects();

		BaseClass::Spawn();
	}

#endif

//
// Contact grenade, explode when it touches something
// 
void CFFProjectileGoop::ExplodeTouch( CBaseEntity *pOther )
{
	// Verify our owner is still here!
	if( !GetOwnerEntity() )
	{
		Remove();
		return;
	}

	trace_t		tr;
	Vector		vecSpot;// trace starts here!

	Assert( pOther );
	if ( !pOther->IsSolid() || pOther == this )
		return;

	// --> Mirv: Check collision rules first
	if (!g_pGameRules->ShouldCollide(GetCollisionGroup(), pOther->GetCollisionGroup()))
		return;
	// <-- Mirv: Check collision rules first
	
#ifdef GAME_DLL

	CEffectData	data;

	if (pOther->IsPlayer()) 
	{
		CFFPlayer *pVictim = ToFFPlayer( pOther );
		CFFPlayer *pPlayer = ToFFPlayer( GetOwnerEntity() );

		// check if they are allies
		if (g_pGameRules->PlayerRelationship(pPlayer, pVictim) == GR_TEAMMATE) 
		{
			pVictim->Heal( pPlayer, FF_GOOP_HEALTHPERGOOP, false );
			// HACK: Using m_nDamageType as effect type
			data.m_nDamageType = GOOP_IMPACT_HEAL;
			EmitSound( "goopgun.hitheal" );
		}
		else
		{
			pVictim->TakeDamage( CTakeDamageInfo( this, pPlayer, GetDamage(), DMG_BLAST ) );
			// HACK: Using m_nDamageType as effect type
			data.m_nDamageType = GOOP_IMPACT_DAMAGE;
			EmitSound( "goopgun.hitdamage" );
		}
	}
	else
	{
		// HACK: Using m_nDamageType as effect type
		data.m_nDamageType = GOOP_IMPACT_WORLD;
		EmitSound( "goopgun.hitworld" );
	}

	Vector vForward;
	AngleVectors( GetAbsAngles(), &vForward );
	VectorNormalize( vForward );

	data.m_vOrigin = GetAbsOrigin();
	data.m_vNormal = vForward;
	data.m_nEntIndex = pOther->entindex();

	DispatchEffect( "GoopImpact", data );

	SetThink( &CBaseGrenade::SUB_Remove );
	SetTouch( NULL );
	
	AddEffects( EF_NODRAW );
	SetAbsVelocity( vec3_origin );
	SetNextThink( gpGlobals->curtime );

#endif

}

//----------------------------------------------------------------------------
// Purpose: Precache the grenade model
//----------------------------------------------------------------------------
void CFFProjectileGoop::Precache() 
{
	PrecacheModel(GOOP_MODEL);
	
	PrecacheScriptSound("goopgun.hitworld");
	PrecacheScriptSound("goopgun.hitheal");
	PrecacheScriptSound("goopgun.hitdamage");

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Create a new grenade
//----------------------------------------------------------------------------
CFFProjectileGoop * CFFProjectileGoop::CreateGoop(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed) 
{
	CFFProjectileGoop *pGoop = (CFFProjectileGoop *) CreateEntityByName("ff_projectile_goop");

	UTIL_SetOrigin(pGoop, vecOrigin);
	pGoop->SetAbsAngles(angAngles);
	pGoop->Spawn();
	pGoop->SetOwnerEntity(pentOwner);
	pGoop->m_iSourceClassname = (pSource ? pSource->m_iClassname : NULL_STRING);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pGoop->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pGoop->SetupInitialTransmittedVelocity(vecForward * iSpeed);
#endif
	
	CEffectData data;
	data.m_vOrigin = vecOrigin;
	data.m_vAngles = angAngles;
	data.m_nDamageType = iSpeed;//iSpeed; // AfterShock: HACK: use m_nDamageType to pass the nail speed int

#ifdef GAME_DLL
	data.m_nEntIndex = pentOwner->entindex();
#else
	data.m_hEntity = pentOwner;
#endif

	DispatchEffect("Projectile_Goop", data);

	//pGoop->SetDamage(iDamage); 
	pGoop->SetDamage(iDamage);//AfterShock: cvar for damage while we test direct damage bonus

	pGoop->m_bIsLive = true;

	pGoop->SetGravity(GetGoopGravity());

	return pGoop; 
}

//----------------------------------------------------------------------------
// Purpose: Goop think function
//----------------------------------------------------------------------------
void CFFProjectileGoop::GoopThink() 
{
	// Remove if we're nolonger in the world
	if (!IsInWorld()) 
	{
		Remove();
		return;
	}

	// Bug #0000501: Doors can be blocked by shit that shouldn't block them.
	if( GetGroundEntity() && ( GetAbsVelocity() == vec3_origin ) )
	{
		if( GetGroundEntity()->GetMoveType() != MOVETYPE_PUSH )
			SetMoveType( MOVETYPE_NONE );
	}

	// Next think straight away
	SetNextThink(gpGlobals->curtime);

	// BEG: Mulch - don't slow down in water
//	
//	// Slow down in water(need to fix this, will slow to a halt) 
//	if (GetWaterLevel() != 0) 
//	{
//		SetAbsVelocity(GetAbsVelocity() * 0.5);
//	}
	// END: Mulch
}
