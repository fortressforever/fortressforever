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

ConVar ffdev_cloaksmoke_dietime("ffdev_cloaksmoke_dietime","5.0", FCVAR_REPLICATED,"How long the particles live.");
ConVar ffdev_cloaksmoke_scale("ffdev_cloaksmoke_scale","48.0", FCVAR_REPLICATED,"How big the particles are.");
ConVar ffdev_cloaksmoke_alpha("ffdev_cloaksmoke_alpha","0.8", FCVAR_REPLICATED,"Alpha of the particles(in float form).");
ConVar ffdev_cloaksmoke_moveforce("ffdev_cloaksmoke_moveforce","0.025", FCVAR_REPLICATED,"Strength of the movement attractor.");
ConVar ffdev_cloaksmoke_red( "ffdev_cloaksmoke_red", "255", FCVAR_REPLICATED );
ConVar ffdev_cloaksmoke_green( "ffdev_cloaksmoke_green", "255", FCVAR_REPLICATED );
ConVar ffdev_cloaksmoke_blue( "ffdev_cloaksmoke_blue", "255", FCVAR_REPLICATED );

ConVar ffdev_cloaksmoke_fast_duration( "ffdev_cloaksmoke_fast_duration", "0.75", FCVAR_REPLICATED | FCVAR_NOTIFY, "shutup" );
ConVar ffdev_cloaksmoke_fast_mod( "ffdev_cloaksmoke_fast_mod", "6.0", FCVAR_REPLICATED | FCVAR_NOTIFY, "shutup" );
ConVar ffdev_cloaksmoke_fast_spawnrate( "ffdev_cloaksmoke_fast_spawnrate", "0.001f", FCVAR_REPLICATED | FCVAR_NOTIFY, "shutup" );
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

	m_flFastModeEndTime = gpGlobals->curtime + ffdev_cloaksmoke_fast_duration.GetFloat();
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
		pRet->m_flDieTime = ffdev_cloaksmoke_dietime.GetFloat();//10.0f;
		pRet->m_flLifetime = 0;
		pRet->m_uchColor[0] = ffdev_cloaksmoke_red.GetInt();
		pRet->m_uchColor[1] = ffdev_cloaksmoke_green.GetInt();
		pRet->m_uchColor[2] = ffdev_cloaksmoke_blue.GetInt();
		pRet->m_flAlpha = ffdev_cloaksmoke_alpha.GetFloat();//0.8f;
		pRet->m_flSize = ffdev_cloaksmoke_scale.GetFloat();//0.5f;
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

		float end = pParticle->m_flLifetime / pParticle->m_flDieTime;
		float start = 1.0f - end;

		Vector F(0.0f, 0.0f, 0.0f);

		ApplyDrag(&F, pParticle->m_vVelocity, 4.0f, 20.0f);

		//if( gpGlobals->curtime < m_flFastModeEndTime )
		if( pParticle->m_flLifetime < ffdev_cloaksmoke_fast_duration.GetFloat() )
		{
			pParticle->m_Pos += (pParticle->m_vVelocity * ffdev_cloaksmoke_fast_mod.GetFloat()) * timeDelta * 0.5f;
			pParticle->m_vVelocity += F * timeDelta;							// assume mass of 1
			pParticle->m_Pos += (pParticle->m_vVelocity * ffdev_cloaksmoke_fast_mod.GetFloat()) * timeDelta * 0.5f;
		}
		else
		{
			pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
			pParticle->m_vVelocity += F * timeDelta;							// assume mass of 1
			pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
		}

		pParticle->m_flAlpha = start * ffdev_cloaksmoke_alpha.GetFloat();
		pParticle->m_flSize = ffdev_cloaksmoke_scale.GetFloat();

		if ( pParticle->m_flLifetime >= pParticle->m_flDieTime )
		{
			pIterator->RemoveParticle( pParticle );
		}

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
	// Don't create any more after this has died. Once the remaining ones have
	// been simulated this entity will automatically remove itself
	if (gpGlobals->curtime > m_flDieTime)
		return;

	if (gpGlobals->curtime < m_flNextParticle)
		return;

	if( gpGlobals->curtime < m_flFastModeEndTime )
	{
		m_flNextParticle = gpGlobals->curtime + ffdev_cloaksmoke_fast_spawnrate.GetFloat();
	}
	else
	{
		m_flNextParticle = gpGlobals->curtime + 0.1f;
	}

	//float scale = gas_scale.GetFloat();
	CloakSmokeParticle *pParticle = NULL;
	QAngle angle;
	Vector forward, right, up, velocity;

	pParticle = AddCloakSmokeParticle(m_vecOrigin);

	if(!pParticle)
		return;

	// Pick a random direction
	Vector vecDirection(RandomFloat(-1.0, 1.0f), RandomFloat(-1.0, 1.0f), RandomFloat(0, 2.0f));
	vecDirection.NormalizeInPlace();

	// And a random distance
	Vector vecFinalPos = m_vecOrigin + vecDirection * RandomFloat(50.0f, 200.0f);

	// Go as far as possible
	trace_t tr;
	UTIL_TraceLine(m_vecOrigin, vecFinalPos, MASK_SOLID, NULL, COLLISION_GROUP_DEBRIS, &tr);

	// Takes 5 seconds for a cloud to disperse
	pParticle->m_vVelocity = m_vecVelocity + (tr.endpos - m_vecOrigin) * 0.2f;

	// This is the position we're going to, even though we may not reach it
	pParticle->m_vFinalPos = tr.endpos;
}