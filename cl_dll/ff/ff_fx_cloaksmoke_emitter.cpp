/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_cloaksmoke_emitter.cpp
/// @author Greg Stefanakis (GreenMushy)
/// @date May 27, 2011
/// @brief cloak smoke particle emitter
///
/// Implementation of the cloak smoke cloud emitter particle system
/// 
/// Revisions
/// ---------

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "ff_fx_cloaksmoke_emitter.h"
#include "ff_grenade_base.h"

//Randomizers for the direction a particle takes
ConVar ffdev_cloaksmoke_min_X( "ffdev_cloaksmoke_min_X", "-1.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Min rand X direction" );
ConVar ffdev_cloaksmoke_min_Y( "ffdev_cloaksmoke_min_Y", "-1.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Min rand Y direction" );
ConVar ffdev_cloaksmoke_min_Z( "ffdev_cloaksmoke_min_Z", "0.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Min rand Z direction" );

ConVar ffdev_cloaksmoke_max_X( "ffdev_cloaksmoke_max_X", "1.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Max rand X direction" );
ConVar ffdev_cloaksmoke_max_Y( "ffdev_cloaksmoke_max_Y", "1.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Max rand Y direction" );
ConVar ffdev_cloaksmoke_max_Z( "ffdev_cloaksmoke_max_Z", "0.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Max rand Z direction" );

//Starting location offsetting( so the particles start in different locations )
ConVar ffdev_cloaksmoke_min_loc_X( "ffdev_cloaksmoke_min_loc_X", "-32.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Min rand X start location" );
ConVar ffdev_cloaksmoke_min_loc_Y( "ffdev_cloaksmoke_min_loc_Y", "-32.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Min rand Y start location" );
ConVar ffdev_cloaksmoke_min_loc_Z( "ffdev_cloaksmoke_min_loc_Z", "-1.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Min rand Z start location" );

ConVar ffdev_cloaksmoke_max_loc_X( "ffdev_cloaksmoke_max_loc_X", "32.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Max rand X start location" );
ConVar ffdev_cloaksmoke_max_loc_Y( "ffdev_cloaksmoke_max_loc_Y", "32.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Max rand Y start location" );
ConVar ffdev_cloaksmoke_max_loc_Z( "ffdev_cloaksmoke_max_loc_Z", "1.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Max rand Z start location" );

ConVar ffdev_cloaksmoke_min_dist( "ffdev_cloaksmoke_min_dist", "16.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Min rand distance" );
ConVar ffdev_cloaksmoke_max_dist( "ffdev_cloaksmoke_max_dist", "16.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Max rand distance" );

ConVar ffdev_cloaksmoke_min_size( "ffdev_cloaksmoke_min_size", "16.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Min particle size" );
ConVar ffdev_cloaksmoke_max_size( "ffdev_cloaksmoke_max_size", "32.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Max particle size" );

ConVar ffdev_cloaksmoke_num_particles( "ffdev_cloaksmoke_num_particles", "32", FCVAR_REPLICATED | FCVAR_NOTIFY, "Number of particles that start alive" );
ConVar ffdev_cloaksmoke_dietime( "ffdev_cloaksmoke_dietime", "1.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "Particle Dietime" );

//Randomizers for direction
#define MIN_X ffdev_cloaksmoke_min_X.GetFloat()
#define MIN_Y ffdev_cloaksmoke_min_Y.GetFloat()
#define MIN_Z ffdev_cloaksmoke_min_Z.GetFloat()

#define MAX_X ffdev_cloaksmoke_max_X.GetFloat()
#define MAX_Y ffdev_cloaksmoke_max_Y.GetFloat()
#define MAX_Z ffdev_cloaksmoke_max_Z.GetFloat()

//Randomizers for start location
#define MIN_LOC_X ffdev_cloaksmoke_min_loc_X.GetFloat()
#define MIN_LOC_Y ffdev_cloaksmoke_min_loc_Y.GetFloat()
#define MIN_LOC_Z ffdev_cloaksmoke_min_loc_Z.GetFloat()

#define MAX_LOC_X ffdev_cloaksmoke_max_loc_X.GetFloat()
#define MAX_LOC_Y ffdev_cloaksmoke_max_loc_Y.GetFloat()
#define MAX_LOC_Z ffdev_cloaksmoke_max_loc_Z.GetFloat()

#define MIN_DIST ffdev_cloaksmoke_min_dist.GetFloat()
#define MAX_DIST ffdev_cloaksmoke_max_dist.GetFloat()

#define MIN_SIZE ffdev_cloaksmoke_min_size.GetFloat()
#define MAX_SIZE ffdev_cloaksmoke_max_size.GetFloat()

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CCloakSmokeCloud::m_hMaterial	= INVALID_MATERIAL_HANDLE;

//========================================================================
// material strings
//========================================================================
#define CLOAKSMOKE_PARTICLE_MATERIAL	"particle/particle_smokegrenade"

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheCloakSmokeCloud )
	CLIENTEFFECT_MATERIAL( CLOAKSMOKE_PARTICLE_MATERIAL )
CLIENTEFFECT_REGISTER_END()

//========================================================================
// CCloakSmokeCloud constructor
//========================================================================
CCloakSmokeCloud::CCloakSmokeCloud( const char *pDebugName ) : CParticleEffect( pDebugName )
{
	m_pDebugName = pDebugName;

	m_flNearClipMin	= 16.0f;
	m_flNearClipMax	= 64.0f;

	m_flNextParticle = 0;

	m_bInitiated = false;
}

//========================================================================
// CCloakSmokeCloud destructor
//========================================================================
CCloakSmokeCloud::~CCloakSmokeCloud()
{
}

//========================================================================
// CCloakSmokeCloud::Create
// ----------------------
// Purpose: Creates a new instance of a CCloakSmokeCloud object
//========================================================================
CSmartPtr<CCloakSmokeCloud> CCloakSmokeCloud::Create( const char *pDebugName )
{
	CCloakSmokeCloud *pRet = new CCloakSmokeCloud( pDebugName );
	pRet->SetDynamicallyAllocated( true );

	if(m_hMaterial == INVALID_MATERIAL_HANDLE)
		m_hMaterial = pRet->GetPMaterial(CLOAKSMOKE_PARTICLE_MATERIAL);
	return pRet;
}

//========================================================================
// AddCloakSmokeParticle
// -----------------
// Purpose: Adds a new cloaksmoke particle
//========================================================================
CloakSmokeParticle* CCloakSmokeCloud::AddCloakSmokeParticle( const Vector &vOrigin )
{
	CloakSmokeParticle *pRet = (CloakSmokeParticle*)AddParticle( sizeof( CloakSmokeParticle ), m_hMaterial, vOrigin );
	if ( pRet )
	{
		pRet->m_vOrigin = vOrigin;
		pRet->m_vFinalPos.Init();
		pRet->m_vVelocity.Init();
		pRet->m_flDieTime = ffdev_cloaksmoke_dietime.GetFloat();
		pRet->m_flLifetime = 0;
		pRet->m_uchColor[0] = 255;
		pRet->m_uchColor[1] = 255;
		pRet->m_uchColor[2] = 255;
		pRet->m_flAlpha = 0.8f;
		pRet->m_flSize = MIN_SIZE;
	}

	return pRet;
}

//========================================================================
// SimulateParticles
// ----------
// Purpose: Handles adjusting particle properties as time progresses as
//			well as removing dead particles
//========================================================================
void CCloakSmokeCloud::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	float timeDelta = pIterator->GetTimeDelta();

	CloakSmokeParticle *pParticle = (CloakSmokeParticle*)pIterator->GetFirst();
	while ( pParticle )
	{
		pParticle->m_flLifetime += timeDelta;

		//Goes from 0 up to 1
		float end = pParticle->m_flLifetime / pParticle->m_flDieTime;

		//Goes from 1 down to 0
		float start = 1.0f - end;

		Vector F(0.0f, 0.0f, 0.0f);

		ApplyDrag(&F, pParticle->m_vVelocity, 4.0f, 20.0f);

		pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
		pParticle->m_vVelocity += F * timeDelta;							// assume mass of 1
		pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;

		//Interpolates from start alpha to 0
		pParticle->m_flAlpha = 0.8f * start;

		//Interpolates the size from Min to Max
		pParticle->m_flSize = MIN_SIZE + (( MAX_SIZE - MIN_SIZE ) * end );

		if ( pParticle->m_flLifetime >= pParticle->m_flDieTime )
			pIterator->RemoveParticle( pParticle );

		pParticle = (CloakSmokeParticle*)pIterator->GetNext();
	}
}

void CCloakSmokeCloud::AddAttractor(Vector *F, Vector apos, Vector ppos, float scale)
{
	Vector dir = (apos - ppos);
	dir.NormalizeInPlace();
	float dist = (apos - ppos).Length();
	if(dist > 0.00001f)
	{
		*F += (scale / (dist)) * dir;
	}
}

void CCloakSmokeCloud::ApplyDrag(Vector *F, Vector vel, float scale, float targetvel)
{
	if(vel.IsLengthLessThan(targetvel))
		return;

	Vector dir = -vel;
	vel.NormalizeInPlace();

	float mag = vel.Length() * scale;
	*F += (dir * mag);
}


//========================================================================
// RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void CCloakSmokeCloud::RenderParticles( CParticleRenderIterator *pIterator )
{
	const CloakSmokeParticle *pParticle = (const CloakSmokeParticle *)pIterator->GetFirst();
	while ( pParticle )
	{
		//Render
		Vector	tPos;

		TransformParticle( ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos );
		float sortKey = (int) tPos.z;

		Vector vColor = Vector(pParticle->m_uchColor[0] / 255.0f,
			pParticle->m_uchColor[1] / 255.0f,
			pParticle->m_uchColor[2] / 255.0f);

		RenderParticle_ColorSize(
			pIterator->GetParticleDraw(),
			tPos,
			vColor,
			pParticle->m_flAlpha,
			pParticle->m_flSize
			);

		pParticle = (const CloakSmokeParticle *)pIterator->GetNext( sortKey );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add a new bunch of particles
//-----------------------------------------------------------------------------
void CCloakSmokeCloud::Update(float flTimeDelta)
{
	//Upon first time updating, add a bunch of particles
	if( m_bInitiated == false )
	{
		for( int i = 0; i < ffdev_cloaksmoke_num_particles.GetInt(); i++ )
		{
			CloakSmokeParticle *pParticle = NULL;
			Vector vecRandStartLoc = Vector( RandomFloat(MIN_LOC_X, MAX_LOC_X), 
											 RandomFloat(MIN_LOC_Y, MAX_LOC_Y), 
											 RandomFloat(MIN_LOC_Z, MAX_LOC_Z) );
			pParticle = AddCloakSmokeParticle( m_vecOrigin + vecRandStartLoc );

			if(!pParticle)
				continue;

			// Pick a random direction
			Vector vecDirection(RandomFloat(MIN_X, MAX_X), 
								RandomFloat(MIN_Y, MAX_Y), 
								RandomFloat(MIN_Z, MAX_Z) );
			vecDirection.NormalizeInPlace();

			// And a random distance
			Vector vecFinalPos = pParticle->m_vOrigin + vecDirection * RandomFloat( MIN_DIST, MAX_DIST );

			// Go as far as possible
			trace_t tr;
			UTIL_TraceLine(pParticle->m_vOrigin, vecFinalPos, MASK_SOLID, NULL, COLLISION_GROUP_DEBRIS, &tr);

			// Takes 5 seconds for a cloud to disperse
			pParticle->m_vVelocity = m_vecVelocity + (tr.endpos - m_vecOrigin) * 0.2f;

			// This is the position we're going to, even though we may not reach it
			pParticle->m_vFinalPos = tr.endpos;

		}

		m_bInitiated = true;
	}	

	// Don't create any more after this has died. Once the remaining ones have
	// been simulated this entity will automatically remove itself
	if (gpGlobals->curtime > m_flDieTime)
		return;

	if (gpGlobals->curtime < m_flNextParticle)
		return;

	m_flNextParticle = gpGlobals->curtime + 0.1f;

	CloakSmokeParticle *pParticle = NULL;

	Vector vecRandStartLoc = Vector( RandomFloat(MIN_LOC_X, MAX_LOC_X), 
									 RandomFloat(MIN_LOC_Y, MAX_LOC_Y), 
									 RandomFloat(MIN_LOC_Z, MAX_LOC_Z) );
	pParticle = AddCloakSmokeParticle( m_vecOrigin + vecRandStartLoc );

	if(!pParticle)
		return;

	// Pick a random direction
	Vector vecDirection(RandomFloat(MIN_X, MAX_X), 
						RandomFloat(MIN_Y, MAX_Y), 
						RandomFloat(MIN_Z, MAX_Z) );
	vecDirection.NormalizeInPlace();

	// And a random distance
	Vector vecFinalPos = pParticle->m_vOrigin + vecDirection * RandomFloat( MIN_DIST, MAX_DIST );

	// Go as far as possible
	trace_t tr;
	UTIL_TraceLine(pParticle->m_vOrigin, vecFinalPos, MASK_SOLID, NULL, COLLISION_GROUP_DEBRIS, &tr);

	// Takes 5 seconds for a cloud to disperse
	pParticle->m_vVelocity = m_vecVelocity + (tr.endpos - m_vecOrigin) * 0.2f;

	// This is the position we're going to, even though we may not reach it
	pParticle->m_vFinalPos = tr.endpos;

	//Tell the particle to not live too long after the effect's die time
	pParticle->m_flDieTime = m_flDieTime - gpGlobals->curtime + ffdev_cloaksmoke_dietime.GetFloat();
}