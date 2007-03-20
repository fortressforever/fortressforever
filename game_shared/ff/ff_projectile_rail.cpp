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

#ifdef CLIENT_DLL
	#include "c_te_effect_dispatch.h"
	#include "c_world.h"
#else
	#include "te_effect_dispatch.h"
#endif

#define RAIL_MODEL "models/crossbow_bolt.mdl"

IMPLEMENT_NETWORKCLASS_ALIASED( FFProjectileRail, DT_FFProjectileRail )

BEGIN_NETWORK_TABLE( CFFProjectileRail, DT_FFProjectileRail )
#ifdef CLIENT_DLL
	RecvPropVector( RECVINFO( m_vecEnd )),
	RecvPropInt( RECVINFO( m_bBounce1 ) ),
	RecvPropVector( RECVINFO( m_vecBounce1 )),
	RecvPropInt( RECVINFO( m_bBounce2 ) ),
	RecvPropVector( RECVINFO( m_vecBounce2 )),
#else
	// m_vecEnd doesn't change often, but it needs to be a high priority
	SendPropVector( SENDINFO( m_vecEnd ), 32, SPROP_COORD | SPROP_CHANGES_OFTEN ),
	SendPropInt( SENDINFO(m_bBounce1), 1, SPROP_UNSIGNED ),
	SendPropVector( SENDINFO( m_vecBounce1 ), SPROP_COORD ),
	SendPropInt( SENDINFO(m_bBounce2), 1, SPROP_UNSIGNED ),
	SendPropVector( SENDINFO( m_vecBounce2 ), SPROP_COORD ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_projectile_rail, CFFProjectileRail );
PRECACHE_WEAPON_REGISTER( ff_projectile_rail );

extern ConVar ffdev_railgun_maxchargetime;

ConVar ffdev_rail_maxbounceangle( "ffdev_rail_maxbounceangle", "45", FCVAR_REPLICATED, "Maximum angle for a rail to bounce" );
ConVar ffdev_rail_bouncedamagefactor( "ffdev_rail_bouncedamagefactor", "1.4", FCVAR_REPLICATED, "Damage multiplier per bounce" );

ConVar ffdev_rail_explodedamage_min( "ffdev_rail_explodedamage_min", "20", FCVAR_REPLICATED, "Explosion damage caused from a half-charge shot." );
ConVar ffdev_rail_explodedamage_max( "ffdev_rail_explodedamage_max", "40", FCVAR_REPLICATED, "Explosion damage caused from a full-charge shot." );

#ifdef GAME_DLL
//=============================================================================
// CFFProjectileRail tables
//=============================================================================

BEGIN_DATADESC(CFFProjectileRail) 
	DEFINE_THINKFUNC( DieThink ),
	DEFINE_THINKFUNC( RailThink ),
	DEFINE_ENTITYFUNC( RailTouch ), 
END_DATADESC() 
#endif

//=============================================================================
// CFFProjectileRaile implementation
//=============================================================================

CFFProjectileRail::CFFProjectileRail()
{
#ifdef CLIENT_DLL
	m_pRailEffects = NULL;
	m_bShouldInit = true;
	m_vecStart = Vector(0,0,0);
#else
	m_vecSameOriginCheck = Vector(0,0,0);
	m_flSameOriginCheckTimer = 0.0f;
#endif

	m_iNumBounces = 0;
	m_iMaxBounces = 0;

	// networked variables...
	m_vecEnd = Vector(0,0,0);
	m_bBounce1 = false;
	m_vecBounce1 = Vector(0,0,0);
	m_bBounce2 = false;
	m_vecBounce2 = Vector(0,0,0);
}

//----------------------------------------------------------------------------
// Purpose: Precache the rail model
//----------------------------------------------------------------------------
void CFFProjectileRail::Precache( void ) 
{
	PrecacheModel(RAIL_MODEL);

	PrecacheScriptSound( "Rail.HitBody" );
	PrecacheScriptSound( "Rail.HitWorld" );

	BaseClass::Precache();
}

void CFFProjectileRail::UpdateOnRemove()
{
#ifdef CLIENT_DLL

	if (m_pRailEffects)
	{
		// Are you a God?
		// No.
		// Then...DIE!)
		m_pRailEffects->m_bTimeToDie = true;

		// the end gives good head (TEEEEEEEE HEEEEEEEE!!!!)
		if (m_pRailEffects->m_Keyframes.Count() > 0)
			m_pRailEffects->m_Keyframes[0].pos = m_vecEnd;
	}

#endif

	BaseClass::UpdateOnRemove();
}

//----------------------------------------------------------------------------
// Purpose: Create a new rail
//----------------------------------------------------------------------------
CFFProjectileRail *CFFProjectileRail::CreateRail( const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed, float flChargeTime )
{
	CFFProjectileRail *pRail = ( CFFProjectileRail * )CreateEntityByName( "ff_projectile_rail" );

	float flMaxChargeTime = ffdev_railgun_maxchargetime.GetFloat();

	UTIL_SetOrigin( pRail, vecOrigin );
	pRail->SetAbsAngles( angAngles );
	pRail->Spawn();
	pRail->SetOwnerEntity( pentOwner );
	pRail->m_iSourceClassname = ( pSource ? pSource->m_iClassname : NULL_STRING );
	pRail->m_iMaxBounces = (flChargeTime < flMaxChargeTime / 2) ? 0 : (flChargeTime < flMaxChargeTime) ? 1 : 2;

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

#ifdef GAME_DLL

//----------------------------------------------------------------------------
// Purpose: Spawn a rail, set up model, size, etc
//----------------------------------------------------------------------------
void CFFProjectileRail::Spawn( void ) 
{
	// Setup
	SetModel(RAIL_MODEL);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	SetSize(-Vector(0.5, 0.5, 0.5), Vector(0.5, 0.5, 0.5));
	SetSolid(SOLID_BBOX);
	SetGravity(0.01f);

	// Oh really we're invisible
	//AddEffects(EF_NODRAW);
	
	// Set the correct think & touch for the rail
	SetTouch(&CFFProjectileRail::RailTouch);		// |-- Mirv: Account for GCC strictness
	SetThink(&CFFProjectileRail::RailThink);	// |-- Mirv: Account for GCC strictness

	// Next think(ie. how bubbly it'll be) 
	SetNextThink(gpGlobals->curtime);
	
	// Make sure we're updated if we're underwater
	UpdateWaterState();

	m_iNumBounces = 0;

	BaseClass::Spawn();
}

//----------------------------------------------------------------------------
// Purpose: Touch function for a rail
//----------------------------------------------------------------------------
void CFFProjectileRail::RailTouch( CBaseEntity *pOther ) 
{
	// FF TODO: fix the problem with rails and other projectiles touching COLLISION_GROUP_TRIGGERONLY (apparently the elevator door in dustbowl is in this collision group...which is weird)
	if( !pOther->IsSolid() || pOther->IsSolidFlagSet( FSOLID_VOLUME_CONTENTS ) || !g_pGameRules->ShouldCollide( GetCollisionGroup(), pOther->GetCollisionGroup() ) ) 
		return;

	trace_t	tr;
	tr = BaseClass::GetTouchTrace();

	// If the object we touch takes damage
	if( pOther->m_takedamage != DAMAGE_NO ) 
	{
		Vector	vecNormalizedVel = GetAbsVelocity();
		VectorNormalize( vecNormalizedVel );

		ClearMultiDamage();

		CTakeDamageInfo	dmgInfo( this, GetOwnerEntity(), m_flDamage, DMG_BULLET | DMG_NEVERGIB );
		CalculateMeleeDamageForce( &dmgInfo, vecNormalizedVel, tr.endpos, 0.7f );
		dmgInfo.SetDamagePosition( tr.endpos );
		pOther->DispatchTraceAttack( dmgInfo, vecNormalizedVel, &tr );

		ApplyMultiDamage();

		//Adrian: keep going through the glass.
		if( pOther->GetCollisionGroup() == COLLISION_GROUP_BREAKABLE_GLASS )
			 return;

		SetAbsVelocity( Vector( 0, 0, 0 ) );

		// play body "thwack" sound
		EmitSound( "Rail.HitBody" );

		Vector vForward;
		AngleVectors( GetAbsAngles(), &vForward );
		VectorNormalize( vForward );

		trace_t	tr2;
		UTIL_TraceLine( GetAbsOrigin(), GetAbsOrigin() + vForward * 128, MASK_OPAQUE, pOther, COLLISION_GROUP_NONE, &tr2 );

		if( tr2.fraction != 1.0f ) 
		{
			if( tr2.m_pEnt && ( tr2.m_pEnt->GetMoveType() == MOVETYPE_NONE ) ) 
			{
				CEffectData	data;

				data.m_vOrigin = tr2.endpos;
				data.m_vNormal = vForward;
				data.m_nEntIndex = tr2.m_pEnt->entindex();
			
				DispatchEffect( "RailImpact", data );
			}
		}

		// Spark & leave decal if we hit an entity which wasn't human(ie. player) 
		if( !pOther->IsPlayer() && UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER ) 
		{
			UTIL_ImpactTrace( &tr, DMG_BULLET );
			g_pEffects->Sparks( GetAbsOrigin() );
		}

		// 0001267: Added explosion effect if charged
		// half charge
		if ( m_iMaxBounces == 1 )
		{
			// half charge explosion damage
			m_flDamage = ffdev_rail_explodedamage_min.GetFloat();

			// bounced, so multiply by specified bounce damage factor
			if (m_iNumBounces > 0)
				m_flDamage *= ffdev_rail_bouncedamagefactor.GetFloat();

			Detonate();
		}
		// full charge
		else if ( m_iMaxBounces == 2 )
		{
			// full charge explosion damage
			m_flDamage = ffdev_rail_explodedamage_max.GetFloat();

			// for each bounce, multiply by specified bounce damage factor
			for (int i = 0; i < m_iNumBounces; i++)
				m_flDamage *= ffdev_rail_bouncedamagefactor.GetFloat();

			Detonate();
		}
		SetupEnd(tr.endpos);
	}
	// Object we hit doesn't take any damage
	else
	{
		// See if we struck the world, make a mark, and then try to bounce
		// Bug #0000181: Railgun shot is absorbed by doors on ff_dev_ctf
		if( /*pOther->GetMoveType() == MOVETYPE_NONE &&*/ !( tr.surface.flags & SURF_SKY ) ) 
		{
			EmitSound( "Rail.HitWorld" );

			Vector vForward;
			AngleVectors( GetAbsAngles(), &vForward );
			VectorNormalize( vForward );

			CEffectData	data;

			data.m_vOrigin = tr.endpos;
			data.m_vNormal = vForward;
			data.m_nEntIndex = 0;

			DispatchEffect( "RailImpact", data );			
			UTIL_ImpactTrace( &tr, DMG_BULLET );

			bool bBounced = false;

			// bouncing is possible if num bounces is below max bounces
			if( m_iNumBounces < m_iMaxBounces )
			{
				Vector vecDir = GetAbsVelocity();
				float flSpeed = VectorNormalize(vecDir);
				float flDot = -DotProduct( tr.plane.normal, vecDir );
				float flAngle = RAD2DEG(flDot);

				// If angle is too shallow, don't bounce
				if( flAngle <= ffdev_rail_maxbounceangle.GetFloat() )
				{
					// we're bouncing!
					bBounced = true;

					// Increment bounces
					++m_iNumBounces;

					// BOUNCE BONUS DAMAGE!
					m_flDamage *= ffdev_rail_bouncedamagefactor.GetFloat();

					Vector vecReflect = 2 * tr.plane.normal * flDot + vecDir;
					Vector vecReflectNorm = vecReflect;
					VectorNormalize( vecReflectNorm );

					// Shoot us back!
					SetAbsVelocity( vecReflectNorm * flSpeed );

					// Spark & leave decal
					if ( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER )
					{
						UTIL_ImpactTrace( &tr, DMG_BULLET );
						g_pEffects->Sparks( GetAbsOrigin() );
					}

					if( m_iNumBounces == 1 )
					{
						m_bBounce1 = true;
						m_vecBounce1 = GetAbsOrigin();
					}
					else if ( m_iNumBounces == 2 )
					{
						m_bBounce2 = true;
						m_vecBounce2 = GetAbsOrigin();
					}
				}				
			}

			// we didn't bounce, so it's time to die
			if( !bBounced )
			{
				// 0001267: Added explosion effect if charged
				// only detonate anywhere if it's a fully charged rail
				if ( m_iMaxBounces == 2 )
				{
					// full charge explosion damage
					m_flDamage = ffdev_rail_explodedamage_max.GetFloat();

					// for each bounce, multiply by specified bounce damage factor
					for (int i = 0; i < m_iNumBounces; i++)
						m_flDamage *= ffdev_rail_bouncedamagefactor.GetFloat();

					Detonate();
				}
				SetupEnd(tr.endpos);
			}

			// Shoot some sparks
			if( UTIL_PointContents( GetAbsOrigin() ) != CONTENTS_WATER )
				g_pEffects->Sparks( GetAbsOrigin() );
		}
		else
		{
			// Put a mark unless we've hit the sky
			if( ( tr.surface.flags & SURF_SKY ) == false ) 
			{
				UTIL_ImpactTrace( &tr, DMG_BULLET );
				
				// 0001267: Added explosion effect if charged
				// only detonate anywhere if it's a fully charged rail
				if ( m_iMaxBounces == 2 )
				{
					// full charge explosion damage
					m_flDamage = ffdev_rail_explodedamage_max.GetFloat();

					// for each bounce, multiply by specified bounce damage factor
					for (int i = 0; i < m_iNumBounces; i++)
						m_flDamage *= ffdev_rail_bouncedamagefactor.GetFloat();

					Detonate();
				}
				SetupEnd(tr.endpos);
			}
		}
	}
}

//----------------------------------------------------------------------------
// Purpose: set it up to die
//----------------------------------------------------------------------------
void CFFProjectileRail::SetupEnd( Vector end ) 
{
	m_vecEnd = end;

	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_NONE);

	SetTouch(NULL);
	SetThink(&CFFProjectileRail::DieThink);

	// give just enough time for the client to interpolate to the end?
	SetNextThink(gpGlobals->curtime + 0.1f);
}

//----------------------------------------------------------------------------
// Purpose: DIE!
//----------------------------------------------------------------------------
void CFFProjectileRail::DieThink( void ) 
{
	SetThink(NULL);
	Remove();
	//UTIL_RemoveImmediate(this);
}

//----------------------------------------------------------------------------
// Purpose: Make a trail of bubbles
//----------------------------------------------------------------------------
void CFFProjectileRail::RailThink( void ) 
{
	float flDisance = abs(Vector(GetAbsOrigin() - m_vecSameOriginCheck).Length());
	if (flDisance < 2)
	{
		m_flSameOriginCheckTimer += gpGlobals->frametime;

		// FF TODO: need to fix the actual problem in the top return in the touch, 
		// which has something to do with COLLISION_GROUP_TRIGGERONLY...but I'm 
		// too tired to fix that shit right now, so it's "TODO" later.

		// Have we really been basically sitting still?  Then, DIE!
		if (m_flSameOriginCheckTimer > 3.0f)
		{
			// 0001267: Added explosion effect if charged
			// only detonate anywhere if it's a fully charged rail
			if ( m_iMaxBounces == 2 )
			{
				// full charge explosion damage
				m_flDamage = ffdev_rail_explodedamage_max.GetFloat();

				// for each bounce, multiply by specified bounce damage factor
				for (int i = 0; i < m_iNumBounces; i++)
					m_flDamage *= ffdev_rail_bouncedamagefactor.GetFloat();

				Detonate();
			}
			SetupEnd(GetAbsOrigin());
		}
	}
	else
	{
		m_vecSameOriginCheck = GetAbsOrigin();
		m_flSameOriginCheckTimer = 0.0f;
	}

	QAngle angNewAngles;

	VectorAngles(GetAbsVelocity(), angNewAngles);
	SetAbsAngles(angNewAngles);

	// always keep the client updated on where we are
	m_vecEnd = GetAbsOrigin();

	SetNextThink(gpGlobals->curtime);

	// make bubbles every tenth of a second
	if ( int(gpGlobals->curtime * 100) % 10 != 0)
		return;

	if (GetWaterLevel() == 0) 
		return;

	UTIL_BubbleTrail( GetAbsOrigin() - GetAbsVelocity() * 0.1f, GetAbsOrigin(), 5 );
}

//  ^  GAME_DLL  ^
#else 
//  v  CLIENT_DLL  v

void CFFProjectileRail::OnDataChanged(DataUpdateType_t type) 
{
	BaseClass::OnDataChanged(type);
	if (type == DATA_UPDATE_CREATED)
	{
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

void CFFProjectileRail::ClientThink( void )
{

	// make the rail effects if there are none yet
	if (m_bShouldInit)
	{
		m_pRailEffects = (CFFRailEffects*)CreateEntityByName("ff_rail_effects");

		// still NULL?  then return so there are no problems
		if (!m_pRailEffects)
			return;

		m_pRailEffects->Spawn();
		m_pRailEffects->SetAbsVelocity(GetAbsVelocity());

		// set these up as where we are when we create the effects
		m_vecStart = m_vecEnd = m_vecBounce1 = m_vecBounce2 = GetAbsOrigin();

		// should only doing this while using the crossbow bolt model...
		FindOverrideMaterial("effects/cloak", TEXTURE_GROUP_CLIENT_EFFECTS);

		// no need to init anymore
		m_bShouldInit = false;
	}

	if (!m_pRailEffects && !m_pRailEffects->m_bTimeToDie)
		return;

	// FF TODO? make the effect update its own origin based on the keyframes and the projectile velocity?
	m_pRailEffects->SetAbsOrigin(GetAbsOrigin());
	m_pRailEffects->SetAbsAngles(GetAbsAngles());

	// count up how many positions need to be worried with (starts at 2 because of start and current/end)
	int iCount = 2;

	// count up for each bounce
	if (m_bBounce1)
		iCount++;
	if (m_bBounce2)
		iCount++;

	// only purge and make a new array if we've "grown"
	if (iCount > m_pRailEffects->m_Keyframes.Count())
	{
		m_pRailEffects->m_Keyframes.Purge();

		// put the current/end at the head (0) position of the array
		m_pRailEffects->m_Keyframes.AddToHead(RailKeyframe(GetAbsOrigin(), RAIL_KEYFRAME_TYPE_END));

		// always insert start after head (end of list/array)
		m_pRailEffects->m_Keyframes.InsertAfter(0, RailKeyframe(m_vecStart, RAIL_KEYFRAME_TYPE_START));

		// insert bounce 1 after head (and before start)
		if (m_bBounce1)
			m_pRailEffects->m_Keyframes.InsertAfter(0, RailKeyframe(m_vecBounce1, RAIL_KEYFRAME_TYPE_BOUNCE1));

		// insert bounce 2 after head (and before bounce 1)
		if (m_bBounce2)
			m_pRailEffects->m_Keyframes.InsertAfter(0, RailKeyframe(m_vecBounce2, RAIL_KEYFRAME_TYPE_BOUNCE2));
	}
	// always update the head (0) with our current position
	else
		m_pRailEffects->m_Keyframes[0].pos = m_vecEnd = GetAbsOrigin();
}

//  ^  CLIENT_DLL  ^
#endif
