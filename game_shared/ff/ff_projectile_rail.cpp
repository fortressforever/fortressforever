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
#include "iefx.h"

class CRecvProxyData;
extern void RecvProxy_LocalVelocityX(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_LocalVelocityY(const CRecvProxyData *pData, void *pStruct, void *pOut);
extern void RecvProxy_LocalVelocityZ(const CRecvProxyData *pData, void *pStruct, void *pOut);

#define RAIL_MODEL "models/crossbow_bolt.mdl"

IMPLEMENT_NETWORKCLASS_ALIASED( FFProjectileRail, DT_FFProjectileRail )

BEGIN_NETWORK_TABLE( CFFProjectileRail, DT_FFProjectileRail )
#ifdef CLIENT_DLL
	RecvPropInt( RECVINFO(m_iNumBounces) ),
	RecvPropFloat		( RECVINFO(m_vecVelocity[0]), 0, RecvProxy_LocalVelocityX ),
	RecvPropFloat		( RECVINFO(m_vecVelocity[1]), 0, RecvProxy_LocalVelocityY ),
	RecvPropFloat		( RECVINFO(m_vecVelocity[2]), 0, RecvProxy_LocalVelocityZ ),
#else
	// Increased range for this because it goes at a mad speed
	SendPropExclude("DT_BaseGrenade", "m_vecVelocity[0]"),
	SendPropExclude("DT_BaseGrenade", "m_vecVelocity[1]"),
	SendPropExclude("DT_BaseGrenade", "m_vecVelocity[2]"),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecVelocity, 0), 16, SPROP_CHANGES_OFTEN|SPROP_ROUNDDOWN, -4096.0f, 4096.0f ),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecVelocity, 1), 16, SPROP_CHANGES_OFTEN|SPROP_ROUNDDOWN, -4096.0f, 4096.0f ),
	SendPropFloat( SENDINFO_VECTORELEM(m_vecVelocity, 2), 16, SPROP_CHANGES_OFTEN|SPROP_ROUNDDOWN, -4096.0f, 4096.0f ),

	SendPropInt( SENDINFO(m_iNumBounces), 8, SPROP_UNSIGNED ), // | SPROP_CHANGES_OFTEN ),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS( ff_projectile_rail, CFFProjectileRail );
PRECACHE_WEAPON_REGISTER( ff_projectile_rail );

extern ConVar ffdev_railgun_maxchargetime;

ConVar ffdev_rail_maxbounceangle( "ffdev_rail_maxbounceangle", "45", FCVAR_REPLICATED | FCVAR_CHEAT, "Maximum angle for a rail to bounce" );
ConVar ffdev_rail_bouncedamagefactor( "ffdev_rail_bouncedamagefactor", "1.4", FCVAR_REPLICATED | FCVAR_CHEAT, "Damage multiplier per bounce" );

ConVar ffdev_rail_explodedamage_min( "ffdev_rail_explodedamage_min", "20", FCVAR_REPLICATED | FCVAR_CHEAT, "Explosion damage caused from a half-charge shot." );
ConVar ffdev_rail_explodedamage_max( "ffdev_rail_explodedamage_max", "40", FCVAR_REPLICATED | FCVAR_CHEAT, "Explosion damage caused from a full-charge shot." );

#ifdef CLIENT_DLL

ConVar ffdev_rail_prate( "ffdev_rail_prate", "128", 0, "Amount of rail particles per second.", true, 0, true, 65536 );

// dlight scale
extern ConVar cl_ffdlight_rail;

#endif

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
#ifdef GAME_DLL
	m_vecSameOriginCheck = Vector(0,0,0);
	m_flSameOriginCheckTimer = 0.0f;
#else
	m_pDLight = NULL;
#endif

	m_iNumBounces = 0;
	m_iMaxBounces = 0;
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

//----------------------------------------------------------------------------
// Purpose: Create a new rail
//----------------------------------------------------------------------------
CFFProjectileRail *CFFProjectileRail::CreateRail( const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBasePlayer *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed, float flChargeTime )
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
	pRail->m_DmgRadius = iDamageRadius;

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
	SetTouch(&CFFProjectileRail::RailTouch);	// |-- Mirv: Account for GCC strictness
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
			data.m_nEntIndex = tr.m_pEnt->entindex();

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

					// FF TODO: bounce effects
					if( m_iNumBounces == 1 )
					{
						//DispatchEffect("RailBounce1", data);
					}
					else if ( m_iNumBounces == 2 )
					{
						//DispatchEffect("RailBounce2", data);
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
			// #0001434: Railgun beams get stuck in 3D sky -> Defrag
			// Changed the if statement below to include the inclusion of decals only.  
			// The actual railgun logic now runs regardless of whether the sky or an ordinary brush was hit.

			// Put a mark unless we've hit the sky
			if( ( tr.surface.flags & SURF_SKY ) == false ) 
			{
				UTIL_ImpactTrace( &tr, DMG_BULLET );
			}

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

//----------------------------------------------------------------------------
// Purpose: set it up to die
//----------------------------------------------------------------------------
void CFFProjectileRail::SetupEnd( Vector end ) 
{
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

// default, bounce1, bounce2
unsigned char g_uchRailColors[3][3] = { {64, 128, 192}, {32, 192, 160}, {0, 255, 128} };

//-----------------------------------------------------------------------------
// Purpose: Called when data changes on the server
//-----------------------------------------------------------------------------
void CFFProjectileRail::OnDataChanged( DataUpdateType_t updateType )
{
	// NOTE: We MUST call the base classes' implementation of this function
	BaseClass::OnDataChanged( updateType );

	// Setup our entity's particle system on creation
	if ( updateType == DATA_UPDATE_CREATED )
	{
		// make the crossbow bolt invisible (for some stupid reason, shit doesn't get interpolated properly without a model)
		FindOverrideMaterial("effects/cloak", TEXTURE_GROUP_CLIENT_EFFECTS);

		// dlight scale
		float fDLightScale = cl_ffdlight_rail.GetFloat();

		// create the rail light...maybe
		if (fDLightScale > 0.0f)
			m_pDLight = effects->CL_AllocDlight( 0 );
		else
			m_pDLight = NULL;

		// setup the dlight
		if (m_pDLight)
		{
			m_pDLight->origin = GetAbsOrigin();
			m_pDLight->radius = 64 * fDLightScale;
			m_pDLight->die = gpGlobals->curtime + 0.4;
			m_pDLight->decay = m_pDLight->radius / 0.4;
			m_pDLight->color.r = g_uchRailColors[m_iNumBounces][0];
			m_pDLight->color.g = g_uchRailColors[m_iNumBounces][1];
			m_pDLight->color.b = g_uchRailColors[m_iNumBounces][2];
			m_pDLight->color.exponent = 4;
			m_pDLight->style = 6; // 0 through 12 (0 = normal, 1 = flicker, 5 = gentle pulse, 6 = other flicker);
		}

		// Creat the emitter
		m_hEmitter = CSimpleEmitter::Create( "RailTrail" );

		// Obtain a reference handle to our particle's desired material
		if ( m_hEmitter.IsValid() )
			m_hMaterial = m_hEmitter->GetPMaterial( "effects/rail_glow" );

		// Spawn 128 particles per second
		m_tParticleTimer.Init( ffdev_rail_prate.GetFloat() );

		// Call our ClientThink() function once every client frame
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Client-side think function for the entity
//-----------------------------------------------------------------------------
void CFFProjectileRail::ClientThink( void )
{
	if ( !m_hEmitter )
		return;

	if (m_pDLight)
	{
		m_pDLight->origin = GetAbsOrigin();
		m_pDLight->radius = 64 * cl_ffdlight_rail.GetFloat(); // dlight scale
		m_pDLight->die = gpGlobals->curtime + 0.4;
		m_pDLight->decay = m_pDLight->radius / 0.4;
		m_pDLight->color.r = g_uchRailColors[m_iNumBounces][0];
		m_pDLight->color.g = g_uchRailColors[m_iNumBounces][1];
		m_pDLight->color.b = g_uchRailColors[m_iNumBounces][2];
	}

	SimpleParticle *pParticle;
	float curTime = gpGlobals->frametime;

	// Add as many particles as required this frame
	while ( m_tParticleTimer.NextEvent( curTime ) )
	{
		// Create the particle
		pParticle = m_hEmitter->AddSimpleParticle( m_hMaterial, GetAbsOrigin() );

		if ( pParticle == NULL )
			return;

		// Setup our size
		pParticle->m_uchStartSize = random->RandomFloat( 3, 4 );
		pParticle->m_uchEndSize = 0;

		// Setup our roll
		pParticle->m_flRoll = random->RandomFloat( 0, 2*M_PI );
		pParticle->m_flRollDelta = random->RandomFloat( -DEG2RAD( 180 ), DEG2RAD( 180 ) );

		// Set our color
		memcpy(pParticle->m_uchColor, g_uchRailColors[m_iNumBounces], sizeof(g_uchRailColors[0]) );

		// Setup our alpha values
		pParticle->m_uchStartAlpha = 255;
		pParticle->m_uchEndAlpha = 255;

		// Obtain a random direction
		Vector velocity = RandomVector( -1.0f, 1.0f );
		VectorNormalize( velocity );

		// Obtain a random speed
		float speed = random->RandomFloat( 0.5f, 5.0f );

		// Set our velocity
		pParticle->m_vecVelocity = velocity * speed;

		// Die in a short range of time
		pParticle->m_flDieTime = random->RandomFloat( 0.2f, 0.4f );
	}
}

//  ^  CLIENT_DLL  ^
#endif
