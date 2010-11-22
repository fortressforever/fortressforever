/// =============== Fortress Forever ==============
/// ======== A modification for Half-Life 2 =======
///
/// @file ff_projectile_rocket.cpp
/// @author Gavin "Mirvin_Monkey" Bramhill
/// @date December 21, 2004
/// @brief The FF rocket projectile code.
///
/// REVISIONS
/// ---------
/// Dec 21, 2004 Mirv: First creation logged


#include "cbase.h"
#include "ff_projectile_rocket.h"

#define ROCKET_MODEL "models/projectiles/rocket/w_rocket.mdl"

#ifdef GAME_DLL
	#include "smoke_trail.h"
#else
	#define RocketTrail C_RocketTrail
	#include "c_smoke_trail.h"
	#include "tempentity.h"
	#include "iefx.h"

	// dlight scale
	extern ConVar cl_ffdlight_rocket;
#endif


//ConVar ffdev_rocketsize("ffdev_rocketsize", "2.0", FCVAR_REPLICATED );
#define FFDEV_ROCKETSIZE 2.0f //ffdev_rocketsize.GetFloat() //2.0f
//#define PREDICTED_ROCKETS

//=============================================================================
// CFFProjectileRocket tables
//=============================================================================

IMPLEMENT_NETWORKCLASS_ALIASED(FFProjectileRocket, DT_FFProjectileRocket)

BEGIN_NETWORK_TABLE(CFFProjectileRocket, DT_FFProjectileRocket)
#ifdef GAME_DLL
SendPropEHandle(SENDINFO(m_hRocketTrail)),
#else
RecvPropEHandle(RECVINFO(m_hRocketTrail)),
#endif
END_NETWORK_TABLE()

LINK_ENTITY_TO_CLASS(ff_projectile_rocket, CFFProjectileRocket);
PRECACHE_WEAPON_REGISTER(ff_projectile_rocket);

//=============================================================================
// CFFProjectileRocket implementation
//=============================================================================

#ifdef GAME_DLL
	// Bug #0000436: Need to truncate Rocket travel sound on impact.
	BEGIN_DATADESC( CFFProjectileRocket )
		// Function Pointers
		DEFINE_ENTITYFUNC( ExplodeTouch ),
	END_DATADESC()
#endif

#ifdef CLIENT_DLL

	//----------------------------------------------------------------------------
	// Purpose: Client constructor
	//----------------------------------------------------------------------------
	CFFProjectileRocket::CFFProjectileRocket()
	{
		// by default, no dynamic lights for projectiles
		m_pDLight = NULL;
	}

	//-----------------------------------------------------------------------------
	// Purpose: Remove the rocket trail
	//-----------------------------------------------------------------------------
	void CFFProjectileRocket::CleanUp()
	{
		if (m_hRocketTrail)
			m_hRocketTrail->m_bEmit = false;

		BaseClass::CleanUp();
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	bool CFFProjectileRocket::ShouldPredict()
	{
#ifdef PREDICTED_ROCKETS
		if (GetOwnerEntity() && GetOwnerEntity() == C_BasePlayer::GetLocalPlayer())
			return true;
#endif
		return BaseClass::ShouldPredict();
	}

	extern short	g_sModelIndexFireball;		// (in combatweapon.cpp) holds the index for the fireball 
	extern short	g_sModelIndexWExplosion;	// (in combatweapon.cpp) holds the index for the underwater explosion

	//-----------------------------------------------------------------------------
	// Purpose: Called when data changes on the server
	//-----------------------------------------------------------------------------
	void CFFProjectileRocket::OnDataChanged( DataUpdateType_t updateType )
	{
		// NOTE: We MUST call the base classes' implementation of this function
		BaseClass::OnDataChanged( updateType );

		// Setup our entity's particle system on creation
		if ( updateType == DATA_UPDATE_CREATED )
		{
			CreateDLight();

			// Call our ClientThink() function once every client frame
			SetNextClientThink( CLIENT_THINK_ALWAYS );
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: Client-side think function for the entity
	//-----------------------------------------------------------------------------
	void CFFProjectileRocket::ClientThink( void )
	{
		UpdateDLight();
	}

	//-----------------------------------------------------------------------------
	// Purpose: Client-side explosion stuff (since the rocket is predicted)
	//-----------------------------------------------------------------------------
	void CFFProjectileRocket::Explode(trace_t *pTrace, int bitsDamageType)
	{
		SetModelName( NULL_STRING );//invisible
		AddSolidFlags( FSOLID_NOT_SOLID );
		m_takedamage = DAMAGE_NO;

		// Pull out of the wall a bit
		if ( pTrace->fraction != 1.0 )
		{
			SetLocalOrigin( pTrace->endpos + (pTrace->plane.normal * 32.0f) );	// |-- Mirv: 32 units used in TFC
		}

		Vector vecAbsOrigin = GetAbsOrigin();
		int contents = UTIL_PointContents ( vecAbsOrigin );

		if ( pTrace->fraction != 1.0 )
		{
			Vector vecNormal = pTrace->plane.normal;
			surfacedata_t *pdata = physprops->GetSurfaceData( pTrace->surface.surfaceProps );	
			CPASFilter filter( vecAbsOrigin );

			te->Explosion( filter, -1.0, // don't apply cl_interp delay
				&vecAbsOrigin,
				!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
				/*m_DmgRadius * .03*/ m_flDamage / 128.0f, 
				25,
				TE_EXPLFLAG_NONE,
				m_DmgRadius,
				m_flDamage,
				&vecNormal,
				(char) pdata->game.material );
		}
		else
		{
			CPASFilter filter( vecAbsOrigin );
			te->Explosion( filter, -1.0, // don't apply cl_interp delay
				&vecAbsOrigin, 
				!( contents & MASK_WATER ) ? g_sModelIndexFireball : g_sModelIndexWExplosion,
				/*m_DmgRadius * .03*/ m_flDamage / 128.0f, 
				25,
				TE_EXPLFLAG_NONE,
				m_DmgRadius,
				m_flDamage );
		}

		UTIL_DecalTrace( pTrace, "Scorch" );
		EmitSound( "BaseGrenade.Explode" );

		SetThink(NULL);
		SetTouch(NULL);
	
		AddEffects( EF_NODRAW );
		SetAbsVelocity( vec3_origin );
		SetNextThink( gpGlobals->curtime );

		// As good a time as ever to try and clean up sound effects, etc.
		CleanUp();
	}

#endif

//----------------------------------------------------------------------------
// Purpose: Creata a trail of smoke for the rocket
//----------------------------------------------------------------------------
void CFFProjectileRocket::CreateSmokeTrail() 
{
#ifdef GAME_DLL
	// Smoke trail.
	if ((m_hRocketTrail = RocketTrail::CreateRocketTrail()) != NULL) 
	{
		m_hRocketTrail->m_Opacity = 0.2f;
		m_hRocketTrail->m_SpawnRate = 100;
		m_hRocketTrail->m_ParticleLifetime = 0.5f;
		m_hRocketTrail->m_StartColor.Init(0.65f, 0.65f , 0.65f);
		m_hRocketTrail->m_EndColor.Init(0.45f, 0.45f, 0.45f);
		m_hRocketTrail->m_StartSize = 6;
		m_hRocketTrail->m_EndSize = 10; // 24; // 32; Reduced a bit now
		m_hRocketTrail->m_SpawnRadius = 4;
		m_hRocketTrail->m_MinSpeed = 2;
		m_hRocketTrail->m_MaxSpeed = 16;
		
		m_hRocketTrail->SetLifetime(999);
		m_hRocketTrail->FollowEntity(this, "0");
	}
#endif
}

#ifdef CLIENT_DLL
	void CFFProjectileRocket::CreateDLight()
	{
		// dlight scale
		float flDLightScale = cl_ffdlight_rocket.GetFloat();
		if (flDLightScale > 0.0f)
		{
			m_pDLight = effects->CL_AllocDlight( 0 ); // 0 allows multiple dynamic lights at the same time
			if (m_pDLight) // I'm scared, daddy...of NULL pointers.
			{
				m_pDLight->origin = GetAbsOrigin();
				m_pDLight->radius = 144.0f * flDLightScale;
				m_pDLight->die = gpGlobals->curtime + 0.1;
				m_pDLight->decay = m_pDLight->radius / 0.1;
				m_pDLight->color.r = 255;
				m_pDLight->color.g = 192;
				m_pDLight->color.b = 64;
				m_pDLight->color.exponent = 4;
				m_pDLight->style = 6; // 0 through 12 (0 = normal, 1 = flicker, 5 = gentle pulse, 6 = other flicker);
			}
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: update the dynamic light
	//-----------------------------------------------------------------------------
	void CFFProjectileRocket::UpdateDLight()
	{
		if (m_pDLight) // I'm scared, daddy...of NULL pointers.
		{
			// keep the light attached and alive
			m_pDLight->origin = GetAbsOrigin();
			m_pDLight->radius = 144.0f * cl_ffdlight_rocket.GetFloat(); // dlight scale
			m_pDLight->die = gpGlobals->curtime + 0.1;
		}
	}

#endif

//----------------------------------------------------------------------------
// Purpose: Spawn a rocket, set up model, size, etc
//----------------------------------------------------------------------------
void CFFProjectileRocket::Spawn() 
{
	BaseClass::Spawn();

#if defined(CLIENT_DLL) && !defined(PREDICTED_ROCKETS)
	return;
#endif

	// Setup
	SetModel(ROCKET_MODEL);
	SetMoveType(MOVETYPE_FLY);
	SetSize(Vector(-(FFDEV_ROCKETSIZE), -(FFDEV_ROCKETSIZE), -(FFDEV_ROCKETSIZE)), Vector((FFDEV_ROCKETSIZE), (FFDEV_ROCKETSIZE), (FFDEV_ROCKETSIZE))); // smaller, cube bounding box so we rest on the ground
	SetSolid(SOLID_BBOX);	// So it will collide with physics props!
	SetSolidFlags(FSOLID_NOT_STANDABLE);

	// Hits everything but debris and interactive debris -GreenMushy
	SetCollisionGroup(COLLISION_GROUP_INTERACTIVE);

	// Set the correct think & touch for the nail
	SetTouch(&CFFProjectileRocket::ExplodeTouch); // No we're going to explode when we touch something
	SetThink(NULL);		// no thinking!

	// Next think
	SetNextThink(gpGlobals->curtime);

	// Creates the smoke trail
	CreateSmokeTrail();
}

//----------------------------------------------------------------------------
// Purpose: Precache the rocket model
//----------------------------------------------------------------------------
void CFFProjectileRocket::Precache() 
{
	PrecacheModel(ROCKET_MODEL);
	PrecacheScriptSound("rocket.fly");

	BaseClass::Precache();
}

//----------------------------------------------------------------------------
// Purpose: Create a new rocket
//----------------------------------------------------------------------------
CFFProjectileRocket * CFFProjectileRocket::CreateRocket(const CBaseEntity *pSource, const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner, const int iDamage, const int iDamageRadius, const int iSpeed) 
{
	CFFProjectileRocket *pRocket;
	
#ifdef PREDICTED_ROCKETS
	if (pentOwner->IsPlayer()) 
	{
		pRocket = (CFFProjectileRocket *) CREATE_PREDICTED_ENTITY("ff_projectile_rocket");
		pRocket->SetPlayerSimulated(ToBasePlayer(pentOwner));
	}
	else
#endif
	{
		pRocket = (CFFProjectileRocket *) CreateEntityByName("ff_projectile_rocket");
	}

	UTIL_SetOrigin(pRocket, vecOrigin);
	pRocket->SetAbsAngles(angAngles);
	pRocket->Spawn();
	pRocket->SetOwnerEntity(pentOwner);
	pRocket->m_iSourceClassname = (pSource ? pSource->m_iClassname : NULL_STRING);

	Vector vecForward;
	AngleVectors(angAngles, &vecForward);

	// Set the speed and the initial transmitted velocity
	pRocket->SetAbsVelocity(vecForward * iSpeed);

#ifdef GAME_DLL
	pRocket->SetupInitialTransmittedVelocity(vecForward * iSpeed);
#endif

	pRocket->m_flDamage = iDamage;
	pRocket->m_DmgRadius = iDamageRadius;

	//pRocket->EmitSound("rocket.fly");
	// this is being swapped over to the client -mirv

	return pRocket; 
}

