/// =============== Fortress Forever ===============
/// ======== A modification for Half-Life 2 ========
///
/// @file ff_fx_gascloud_emitter.cpp.cpp
/// @author Shawn Smith (L0ki)
/// @date May 8, 2005
/// @brief gas cloud emitter
///
/// Implementation of the gas cloud emitter particle system
/// 
/// Revisions
/// ---------
/// May 8, 2005	L0ki: Initial Creation
///
/// Jul 14, 2006 Mirv: Redone this so that it's no longer crap

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "ff_fx_gascloud_emitter.h"
#include "ff_grenade_base.h"

ConVar gas_dietime("ffdev_gas_dietime","5.0",0,"How long gas cloud particles live.");
ConVar gas_scale("ffdev_gas_scale","48.0",0,"How big gas particles are.");
ConVar gas_alpha("ffdev_gas_alpha","0.2",0,"Alpha of gas particles.");
ConVar gas_moveforce("ffdev_gas_moveforce","0.025",0,"Strength of gas movement attractor.");

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CGasCloud::m_hMaterial			= INVALID_MATERIAL_HANDLE;

//========================================================================
// material strings
//========================================================================
#define GAS_PARTICLE_MATERIAL		"particle/particle_smokegrenade"

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheGasCloud )
	CLIENTEFFECT_MATERIAL( GAS_PARTICLE_MATERIAL )
CLIENTEFFECT_REGISTER_END()

//========================================================================
// CGasCloud constructor
//========================================================================
CGasCloud::CGasCloud( const char *pDebugName ) : CParticleEffect( pDebugName )
{
	m_pDebugName = pDebugName;

	m_flNearClipMin	= 16.0f;
	m_flNearClipMax	= 64.0f;

	m_flNextParticle = 0;
}

//========================================================================
// CGasCloud destructor
//========================================================================
CGasCloud::~CGasCloud()
{
}

//========================================================================
// CGasCloud::Create
// ----------------------
// Purpose: Creates a new instance of a CGasCloud object
//========================================================================
CSmartPtr<CGasCloud> CGasCloud::Create( const char *pDebugName )
{
	CGasCloud *pRet = new CGasCloud( pDebugName );
	pRet->SetDynamicallyAllocated( true );
	if(m_hMaterial == INVALID_MATERIAL_HANDLE)
		m_hMaterial = pRet->GetPMaterial(GAS_PARTICLE_MATERIAL);
	return pRet;
}

//========================================================================
// AddNapalmParticle
// -----------------
// Purpose: Adds a new NapalmParticle to the system
//========================================================================
GasParticle* CGasCloud::AddGasParticle( const Vector &vOrigin )
{
	GasParticle *pRet = (GasParticle*)AddParticle( sizeof( GasParticle ), m_hMaterial, vOrigin );
	if ( pRet )
	{
		pRet->m_vOrigin = vOrigin;
		pRet->m_vFinalPos.Init();
		pRet->m_vVelocity.Init();
		pRet->m_flDieTime = 10.0f;
		pRet->m_flLifetime = 0;
		pRet->m_uchColor[0] = 32;
		pRet->m_uchColor[1] = 192;
		pRet->m_uchColor[2] = 32;
		pRet->m_flAlpha = 0.8f;
		pRet->m_flSize = 0.5f;
	}

	return pRet;
}

//========================================================================
// SimulateParticles
// ----------
// Purpose: Handles adjusting particle properties as time progresses as
//			well as removing dead particles
//========================================================================
void CGasCloud::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	float timeDelta = pIterator->GetTimeDelta();


	GasParticle *pParticle = (GasParticle*)pIterator->GetFirst();
	while ( pParticle )
	{
		pParticle->m_flLifetime += timeDelta;

		float end = pParticle->m_flLifetime / pParticle->m_flDieTime;
		float start = 1.0f - end;

		// Keep moving if we're not past out end time
		/*
		if (pParticle->m_flLifetime < pParticle->m_flEndPosTime)
			pParticle->m_Pos += pParticle->m_vVelocity * timeDelta;
		*/
		// ted - Now implements the same smoke-disturbance code as the conc particles
		//if (pParticle->m_flLifetime < pParticle->m_flEndPosTime)
		{
			Vector F(0.0f, 0.0f, 0.0f);

			C_BaseEntityIterator iterator;
			CBaseEntity *point = iterator.Next();
			while(point != NULL)
			{
				if((point->GetAbsOrigin() - pParticle->m_vOrigin).IsLengthLessThan(256.0f))
				{
					if(point->GetAbsVelocity().IsLengthGreaterThan(600.0f))
						AddAttractor(&F, point->GetAbsOrigin(), pParticle->m_Pos, gas_moveforce.GetFloat() * point->GetAbsVelocity().LengthSqr());
				}
				point = iterator.Next();
			}

			ApplyDrag(&F, pParticle->m_vVelocity, 4.0f, 20.0f);

			pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
			pParticle->m_vVelocity += F * timeDelta;							// assume mass of 1
			pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;
		}

		pParticle->m_flAlpha = 0.8f * start + 0.0f * end;
		pParticle->m_flSize = 1.0f * start + 96.0f * end;

		if ( pParticle->m_flLifetime >= pParticle->m_flDieTime )
			pIterator->RemoveParticle( pParticle );

		pParticle = (GasParticle*)pIterator->GetNext();
	}
}

void CGasCloud::AddAttractor(Vector *F, Vector apos, Vector ppos, float scale)
{
	Vector dir = (apos - ppos);
	dir.NormalizeInPlace();
	float dist = (apos - ppos).Length();
	if(dist > 0.00001f)
		*F += (scale / (dist/* * dist*/)) * dir;
}

void CGasCloud::ApplyDrag(Vector *F, Vector vel, float scale, float targetvel)
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
void CGasCloud::RenderParticles( CParticleRenderIterator *pIterator )
{
	const GasParticle *pParticle = (const GasParticle *)pIterator->GetFirst();
	while ( pParticle )
	{
		//Render
		Vector	tPos;

		TransformParticle( g_ParticleMgr.GetModelView(), pParticle->m_Pos, tPos );
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

		pParticle = (const GasParticle *)pIterator->GetNext( sortKey );
	}
}

//-----------------------------------------------------------------------------
// Purpose: Add a new bunch of particles
//-----------------------------------------------------------------------------
void CGasCloud::Update(float flTimeDelta)
{
	// Don't create any more after this has died. Once the remaining ones have
	// been simulated this entity will automatically remove itself
	if (gpGlobals->curtime > m_flDieTime)
		return;

	if (gpGlobals->curtime < m_flNextParticle)
		return;

	m_flNextParticle = gpGlobals->curtime + 0.1f;

	//float scale = gas_scale.GetFloat();
	GasParticle *pParticle = NULL;
	QAngle angle;
	Vector forward, right, up, velocity;

	pParticle = AddGasParticle(m_vecOrigin);

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