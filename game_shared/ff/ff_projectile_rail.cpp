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

IMPLEMENT_NETWORKCLASS_ALIASED( FFProjectileRail, DT_FFProjectileRail )

BEGIN_NETWORK_TABLE( CFFProjectileRail, DT_FFProjectileRail )
#ifdef CLIENT_DLL
	RecvPropBool( RECVINFO( m_bStart ) ),
	RecvPropVector( RECVINFO( m_vecStart )),
	RecvPropBool( RECVINFO( m_bEnd ) ),
	RecvPropVector( RECVINFO( m_vecEnd )),
	RecvPropBool( RECVINFO( m_bBounce1 ) ),
	RecvPropVector( RECVINFO( m_vecBounce1 )),
	RecvPropBool( RECVINFO( m_bBounce2 ) ),
	RecvPropVector( RECVINFO( m_vecBounce2 )),
#else
	SendPropBool( SENDINFO( m_bStart ) ),
	SendPropVector( SENDINFO( m_vecStart ), SPROP_COORD ),
	SendPropBool( SENDINFO( m_bEnd ) ),
	SendPropVector( SENDINFO( m_vecEnd ), SPROP_COORD ),
	SendPropBool( SENDINFO( m_bBounce1 ) ),
	SendPropVector( SENDINFO( m_vecBounce1 ), SPROP_COORD ),
	SendPropBool( SENDINFO( m_bBounce2 ) ),
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
	DEFINE_THINKFUNC( RailThink ), 
	DEFINE_THINKFUNC( DieThink ), 
	DEFINE_ENTITYFUNC( RailTouch ), 
END_DATADESC() 
#endif

//=============================================================================
// CFFProjectileRaile implementation
//=============================================================================

CFFProjectileRail::CFFProjectileRail()
{
	m_iNumBounces = 0;
	m_iMaxBounces = 0;

	m_bStart = false;
	m_bEnd = false;
	m_bBounce1 = false;
	m_bBounce2 = false;

#ifdef GAME_DLL
	m_vecSameOriginCheck = Vector(0,0,0);
	m_flSameOriginCheckTime = 0.0f;
#endif
}

//----------------------------------------------------------------------------
// Purpose: Precache the rail model
//----------------------------------------------------------------------------
void CFFProjectileRail::Precache( void ) 
{
	PrecacheScriptSound( "Rail.HitBody" );
	PrecacheScriptSound( "Rail.HitWorld" );

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Create a new rail
//----------------------------------------------------------------------------
CFFProjectileRail *CFFProjectileRail::CreateRail( const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iSpeed, float flChargeTime )
{
	CFFProjectileRail *pRail = ( CFFProjectileRail * )CreateEntityByName( "ff_projectile_rail" );

#ifdef GAME_DLL
	pRail->m_bStart = true;
	pRail->m_vecStart = vecOrigin;
	pRail->m_vecEnd = vecOrigin;
#endif

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
//		SetModel(RAIL_MODEL);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_CUSTOM);
	SetSize(-Vector(0.5, 0.5, 0.5), Vector(0.5, 0.5, 0.5));
	SetSolid(SOLID_BBOX);
	SetGravity(0.01f);

	// Oh really we're invisible
	AddEffects(EF_NODRAW);
	
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
	if( !pOther->IsSolid() || pOther->IsSolidFlagSet( FSOLID_VOLUME_CONTENTS ) || !g_pGameRules->ShouldCollide( GetCollisionGroup(), pOther->GetCollisionGroup() ) ) 
		return;

	// If the object we touch takes damage
	if( pOther->m_takedamage != DAMAGE_NO ) 
	{
		trace_t	tr, tr2;
		tr = BaseClass::GetTouchTrace();
		Vector	vecNormalizedVel = GetAbsVelocity();

		ClearMultiDamage();
		VectorNormalize( vecNormalizedVel );

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

		m_bEnd = true;
		m_vecEnd = GetAbsOrigin();

		SetTouch(NULL);
		SetThink(&CFFProjectileRail::DieThink);
		// give enough time for the client effects to catch up and die as well
		SetNextThink(gpGlobals->curtime + 1.5f);
	}
	// Object we hit doesn't take any damage
	else
	{
		trace_t	tr;
		tr = BaseClass::GetTouchTrace();

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

				m_bEnd = true;
				m_vecEnd = tr.endpos;

				SetTouch(NULL);
				SetThink(&CFFProjectileRail::DieThink);
				// give enough time for the client effects to catch up and die as well
				SetNextThink(gpGlobals->curtime + 1.5f);
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
			}

			m_bEnd = true;
			m_vecEnd = tr.endpos;

			SetTouch(NULL);
			SetThink(&CFFProjectileRail::DieThink);
			// give enough time for the client effects to catch up and die as well
			SetNextThink(gpGlobals->curtime + 1.5f);
		}
	}
}

//----------------------------------------------------------------------------
// Purpose: Make a trail of bubbles
//----------------------------------------------------------------------------
void CFFProjectileRail::RailThink( void ) 
{
	float flDisance = abs(Vector(GetAbsOrigin() - m_vecSameOriginCheck).Length());
	if (flDisance < 2)
	{
		m_flSameOriginCheckTime += gpGlobals->frametime;

		// Have we really been basically sitting still for 4 seconds?  Then, DIE!
		if (m_flSameOriginCheckTime > 4)
		{
			m_bEnd = true;

			SetTouch(NULL);
			SetThink(&CFFProjectileRail::DieThink);
			// give enough time for the client effects to catch up and die as well
			SetNextThink(gpGlobals->curtime + 1.5f);

			return;
		}
	}
	else
	{
		m_vecSameOriginCheck = GetAbsOrigin();
		m_flSameOriginCheckTime = 0.0f;
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

//----------------------------------------------------------------------------
// Purpose: die
//----------------------------------------------------------------------------
void CFFProjectileRail::DieThink( void ) 
{
	SetThink(NULL);
	Remove();
}

//  ^  GAME_DLL  ^
#else 
//  v  CLIENT_DLL  v

void CFFProjectileRail::OnDataChanged(DataUpdateType_t type) 
{
	BaseClass::OnDataChanged(type);
	if (type == DATA_UPDATE_CREATED)
	{
		m_flLastDataChange = gpGlobals->curtime;
		SetNextClientThink( CLIENT_THINK_ALWAYS );

		if (!m_pRailEffects)
		{
			m_pRailEffects = (CFFRailEffects*)CreateEntityByName("ff_rail_effects");
			m_pRailEffects->Spawn();
			SetAbsOrigin(m_vecStart);
			m_pRailEffects->SetAbsOrigin(GetAbsOrigin());
		}
	}else if ( type == DATA_UPDATE_DATATABLE_CHANGED )
	{
		m_flLastDataChange = gpGlobals->curtime;

		if (m_bStart && m_pRailEffects && !m_pRailEffects->m_bTimeToDie)
		{
			// count is at least 1 for the end position
			int iCount = 1;

			// count up for each possible position
			if (m_bStart)
				iCount++;
			if (m_bBounce1)
				iCount++;
			if (m_bBounce2)
				iCount++;

			// only purge and make a new array if we've "grown"
			if (iCount > m_pRailEffects->m_Keyframes.Count())
			{
				m_pRailEffects->m_Keyframes.Purge();

				// put the end keyframe at the head (0) position of the array
				RailKeyframe rkTemp = RailKeyframe(GetLocalOrigin(), RAIL_KEYFRAME_TYPE_END);
				m_pRailEffects->m_Keyframes.AddToHead(rkTemp);

				// insert start
				if (m_bStart)
				{
					rkTemp.pos = m_vecStart;
					rkTemp.type = RAIL_KEYFRAME_TYPE_START;
					m_pRailEffects->m_Keyframes.InsertAfter(0, rkTemp);
				}

				// insert bounce 1
				if (m_bBounce1)
				{
					rkTemp.pos = m_vecBounce1;
					rkTemp.type = RAIL_KEYFRAME_TYPE_BOUNCE1;
					m_pRailEffects->m_Keyframes.InsertAfter(0, rkTemp);
				}

				// insert bounce 2
				if (m_bBounce2)
				{
					rkTemp.pos = m_vecBounce2;
					rkTemp.type = RAIL_KEYFRAME_TYPE_BOUNCE2;
					m_pRailEffects->m_Keyframes.InsertAfter(0, rkTemp);
				}
			}
		}
	}
}

void CFFProjectileRail::ClientThink( void )
{
	if (m_pRailEffects)
	{
		if (m_bStart && !m_pRailEffects->m_bTimeToDie)
		{
			//float flDeltaTime = gpGlobals->curtime - m_flLastDataChange;
			//Vector vecOrigin = GetAbsOrigin();
			//VectorLerp( vecOrigin, m_vecEnd, flDeltaTime, vecOrigin );
			//SetAbsOrigin(vecOrigin);

			m_pRailEffects->SetAbsOrigin(GetAbsOrigin());

			if (m_pRailEffects->m_Keyframes.Count() > 0)
			{
				if (!m_bEnd)
					m_pRailEffects->m_Keyframes[0].pos = GetAbsOrigin();
				else
					m_pRailEffects->m_Keyframes[0].pos = m_vecEnd;
			}
		}

		// done moving, so kill it
		if (m_bEnd)
			m_pRailEffects->m_bTimeToDie = true;
	}
}

//  ^  CLIENT_DLL  ^
#endif
