/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_rail.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF rail projectile code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged
//
//	11/11/2006 Mulchman: Tweaking a bit

#include "cbase.h"
#include "ff_projectile_rail.h"
#include "effect_dispatch_data.h"
#include "IEffects.h"

#define RAIL_MODEL "models/projectiles/rail/w_rail.mdl"
#define RAIL_GLOW "sprites/redglow1.vmt"
#define RAIL_TRAIL "sprites/bluelaser1.vmt"

#ifdef CLIENT_DLL
	#include "c_te_effect_dispatch.h"
	#include "c_world.h"
#else
	#include "te_effect_dispatch.h"
#endif

IMPLEMENT_NETWORKCLASS_ALIASED( FFProjectileRail, DT_FFProjectileRail )

BEGIN_NETWORK_TABLE( CFFProjectileRail, DT_FFProjectileRail )
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_projectile_rail, CFFProjectileRail );
PRECACHE_WEAPON_REGISTER( ff_projectile_rail );

ConVar ffdev_railgun_maxangle( "ffdev_railgun_maxangle", "60", FCVAR_REPLICATED, "Maximum angle for a rail to bounce" );

#ifdef GAME_DLL
//=============================================================================
// CFFProjectileRail tables
//=============================================================================

BEGIN_DATADESC(CFFProjectileRail) 
	DEFINE_THINKFUNC( BubbleThink ), 
	DEFINE_ENTITYFUNC( RailTouch ), 
END_DATADESC() 
#endif

//=============================================================================
// CFFProjectileRaile implementation
//=============================================================================

#ifdef GAME_DLL
	//----------------------------------------------------------------------------
	// Purpose: Spawn a rail, set up model, size, etc
	//----------------------------------------------------------------------------
	void CFFProjectileRail::Spawn( void ) 
	{
		// Setup
//		SetModel(RAIL_MODEL);
		SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
		SetSize(-Vector(1, 1, 1), Vector(1, 1, 1));
		SetSolid(SOLID_BBOX);
		SetGravity(0.01f);

		// Oh really we're invisible
		AddEffects(EF_NODRAW);
		
		// Set the correct think & touch for the rail
		SetTouch(&CFFProjectileRail::RailTouch);		// |-- Mirv: Account for GCC strictness
		SetThink(&CFFProjectileRail::BubbleThink);	// |-- Mirv: Account for GCC strictness

		// Next think(ie. how bubbly it'll be) 
		SetNextThink(gpGlobals->curtime + 0.1f);
		
		// Make sure we're updated if we're underwater
		UpdateWaterState();

		// Start up the eye glow
		m_pMainGlow = CSprite::SpriteCreate(RAIL_GLOW, GetLocalOrigin(), false);

		if (m_pMainGlow != NULL) 
		{
			m_pMainGlow->FollowEntity(this);
			m_pMainGlow->SetTransparency(kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation);
			m_pMainGlow->SetScale(0.2f);
			m_pMainGlow->SetGlowProxySize(4.0f);
		}

		// Start up the eye trail
		m_pGlowTrail	= CSpriteTrail::SpriteTrailCreate(RAIL_TRAIL, GetLocalOrigin(), false);

		if (m_pGlowTrail != NULL) 
		{
			m_pGlowTrail->FollowEntity(this);
			m_pGlowTrail->SetTransparency(kRenderTransAdd, 0, 255, 0, 255, kRenderFxNone);
			m_pGlowTrail->SetStartWidth(8.0f);
			m_pGlowTrail->SetEndWidth(1.0f);
			m_pGlowTrail->SetLifeTime(0.5f);
		}

		m_iNumBounces = 0;

		BaseClass::Spawn();
	}

#endif

//----------------------------------------------------------------------------
// Purpose: Precache the rail model
//----------------------------------------------------------------------------
void CFFProjectileRail::Precache( void ) 
{
	PrecacheModel( RAIL_MODEL );

	PrecacheModel( RAIL_GLOW );
	PrecacheModel( RAIL_TRAIL );

	PrecacheScriptSound( "Rail.HitBody" );
	PrecacheScriptSound( "Rail.HitWorld" );

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Touch function for a rail
//----------------------------------------------------------------------------
void CFFProjectileRail::RailTouch( CBaseEntity *pOther ) 
{
	if( !pOther->IsSolid() || pOther->IsSolidFlagSet( FSOLID_VOLUME_CONTENTS ) || !g_pGameRules->ShouldCollide( GetCollisionGroup(), pOther->GetCollisionGroup() ) ) 
		return;

	// If the object we touch takes damage
	if( pOther->m_takedamage != DAMAGE_NO ) 
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

#ifdef GAME_DLL
		ClearMultiDamage();
		VectorNormalize( vecNormalizedVel );

		CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_flDamage, DMG_BULLET | DMG_NEVERGIB );
		CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
		dmgInfo.SetDamagePosition( tr.endpos );
		pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );

		ApplyMultiDamage();
#endif

		//Adrian: keep going through the glass.
		if( pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS )
			 return;

		SetAbsVelocity( Vector( 0, 0, 0 ) );

		// play body "thwack" sound
		EmitSound( "Rail.HitBody" );

		Vector vForward;
		AngleVectors( GetAbsAngles(), &vForward );
		VectorNormalize( vForward );

		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2 );

		if( tr2.fraction != 1.0f ) 
		{
			if( tr2.m_pEnt && ( tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE ) ) 
			{
				CEffectData	data;

				data.m_vOrigin = tr2.endpos;
				data.m_vNormal = vForward;

#ifdef GAME_DLL
				data.m_nEntIndex = tr2.m_pEnt->entindex();
#else
				data.m_hEntity = tr2.m_pEnt;
#endif
			
				DispatchEffect( "RailImpact", data );
			}
		}
		
		SetTouch( NULL );
		SetThink( NULL );

		Vector vecDir = GetAbsVelocity();

		// Spark & leave decal if we hit an entity which wasn't human(ie. player) 
		if( !pOther->IsPlayer() && UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER ) 
		{
			UTIL_ImpactTrace( &tr, DMG_BULLET );
			g_pEffects->Sparks( GetAbsOrigin() );
		}

		Remove();
	}
	// Object we hit doesn't take any damage
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

		// See if we struck the world
		// Bug #0000181: Railgun shot is absorbed by doors on ff_dev_ctf
		if( /*pOther->GetMoveType() == MOVETYPE_NONE &&*/ !( tr.surface.flags & SURF_SKY ) ) 
		{
			EmitSound( "Rail.HitWorld" );

			// if what we hit is static architecture, can stay around for a while.
			Vector vecDir = GetAbsVelocity();
			float speed = VectorNormalize( vecDir );

			Vector vForward;
			AngleVectors( GetAbsAngles(), &vForward );
			VectorNormalize( vForward );

			CEffectData	data;

			data.m_vOrigin = tr.endpos;
			data.m_vNormal = vForward;

#ifdef GAME_DLL
			data.m_nEntIndex = 0;
#else
			data.m_hEntity = GetClientWorldEntity();	// FIXME
#endif
		
			DispatchEffect( "RailImpact", data );			
			UTIL_ImpactTrace( &tr, DMG_BULLET );

			bool bCantBounce = false;

			// Check if its past the max # of bounces
			if( m_iNumBounces < m_iMaxBounces )
			{
				// Increment bounces
				++m_iNumBounces;

				// BOUNCE!
				m_flDamage *= 2;				

				vecDir = vForward * speed;
				Vector reflect = vecDir + ( -2 * tr.plane.normal * DotProduct( vecDir, tr.plane.normal ) );

				// Check if reflect is within some limits
				Vector vecReflectNorm = reflect;
				VectorNormalize( vecReflectNorm );

//#ifdef GAME_DLL
//				NDebugOverlay::Line( GetAbsOrigin(), GetAbsOrigin() + reflect, 255, 0, 0, false, 5.0f );
//				NDebugOverlay::Line( tr.endpos, tr.endpos + ( tr.plane.normal * 16.0f ), 0, 0, 255, false, 5.0f );
//
//				NDebugOverlay::Cross3D( GetAbsOrigin(), 32.0f, 0, 255, 0, false, 5.0f );
//#endif
				// Calculate angle. Since items are normalized its easy.
				float flAngle = RAD2DEG( acos( vecReflectNorm.Dot( tr.plane.normal ) ) );
//#ifdef GAME_DLL
//				Warning( "[Rail] Angle: %f\n", flAngle );
//#endif

				// If angle is too shallow, don't bounce
				if( flAngle <= ffdev_railgun_maxangle.GetFloat() )
					bCantBounce = true;

				// If we can bounce
				if( !bCantBounce )
				{
					// Shoot us back!
					SetAbsVelocity( reflect );
#ifdef GAME_DLL
					// Change the color
					if( m_iNumBounces == 1 ) 
					{
						m_pMainGlow->SetTransparency( kRenderGlow, 255, 0, 0, 255, kRenderFxNoDissipation );
						m_pGlowTrail->SetTransparency( kRenderTransAdd, 255, 0, 0, 255, kRenderFxNone );
					}
					else if( m_iNumBounces == 2 ) 
					{
						m_pMainGlow->SetTransparency( kRenderGlow, 0, 0, 255, 255, kRenderFxNoDissipation );
						m_pGlowTrail->SetTransparency( kRenderTransAdd, 0, 0, 255, 255, kRenderFxNone );
					}				
#endif
				}				
			}
			else
				bCantBounce = true;

			// If not allowed to bounce for whatever reason, delete
			if( bCantBounce )
			{
				SetTouch( NULL );
				SetThink( NULL );
				SetNextThink( gpGlobals->curtime );

#ifdef GAME_DLL
				m_pMainGlow->FadeAndDie( 2.0f );
#endif
				
				Remove();
			}

			// Shoot some sparks
			if( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER && ( speed > 500 ) )
				g_pEffects->Sparks( GetAbsOrigin() );
		}
		else
		{
			// Put a mark unless we've hit the sky
			if( ( tr.surface.flags & SURF_SKY ) == false ) 
				UTIL_ImpactTrace( &tr, DMG_BULLET );

			Remove();
		}
	}
}

//----------------------------------------------------------------------------
// Purpose: Make a trail of bubbles
//----------------------------------------------------------------------------
void CFFProjectileRail::BubbleThink( void ) 
{
	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	SetNextThink(gpGlobals->curtime + 0.1f);

	if (GetWaterLevel() == 0) 
		return;

#ifdef GAME_DLL
	UTIL_BubbleTrail( GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5 );
#endif
}

//----------------------------------------------------------------------------
// Purpose: Create a new rail
//----------------------------------------------------------------------------
CFFProjectileRail *CFFProjectileRail::CreateRail( const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed, float flChargeTime )
{
	CFFProjectileRail *pRail = ( CFFProjectileRail * )CreateEntityByName( "ff_projectile_rail" );

	UTIL_SetOrigin( pRail, vecOrigin );
	pRail->SetAbsAngles( angAngles );
	pRail->Spawn();
	pRail->SetOwnerEntity( pentOwner );
	pRail->m_iSourceClassname = ( pSource ? pSource->m_iClassname : NULL_STRING );
	pRail->m_iMaxBounces = ( flChargeTime >= 2.0f ) ? 2 : ( flChargeTime >= 1.0f ) ? 1 : 0;

	Vector vecForward;
	AngleVectors( angAngles, &vecForward );
	VectorNormalize( vecForward );

	// Set the speed and the initial transmitted velocity
	pRail->SetAbsVelocity( vecForward * iSpeed );

//#ifdef GAME_DLL
	//pRail->SetupInitialTransmittedVelocity(vecForward * iSpeed);
//#endif

	pRail->m_flDamage = iDamage;

#ifdef GAME_DLL
	/*
	CBasePlayer *pPlayer = ToBasePlayer(pentOwner);
	if(pPlayer && pPlayer->IsBot())
		Omnibot::Notify_PlayerShootProjectile(pPlayer, pRail->edict());
		*/
#endif

	return pRail;
}
