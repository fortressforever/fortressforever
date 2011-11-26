// =============== Fortress Forever ===============
// ======== A modification for Half-Life 2 ========
//
// @file d:\ffsrc\cl_dll\ff_fx_infection.cpp
// @author Patrick O'Leary (Mulchman)
// @date 8/15/2006
// @brief infection particle effects
//
// particle manager for the infection particles
//
// Revisions
// ---------
// 8/15/2006, Mulchman
//		Initial version

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "c_te_effect_dispatch.h"
#include "cliententitylist.h"

#include "ff_fx_infection.h"

#define INFECTION_EFFECT_MATERIAL "particle/particle_smokegrenade"

ConVar ffdev_infection_startingparticles( "ffdev_infection_startingparticles", "7", FCVAR_CHEAT, "Number of particles to start out creating on the first Update()s." );

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheInfectionEmitter )
CLIENTEFFECT_MATERIAL( INFECTION_EFFECT_MATERIAL )
CLIENTEFFECT_REGISTER_END()

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CInfectionEmitter::m_hMaterial = INVALID_MATERIAL_HANDLE;

//========================================================================
// CInfectionEmitter constructor
//========================================================================
CInfectionEmitter::CInfectionEmitter( const char *pDebugName ) : CSimpleEmitter( pDebugName )
{
	m_pDebugName = pDebugName;
	m_flNextParticle = 0.0f;
}

//========================================================================
// CInfectionEmitter destructor
//========================================================================
CInfectionEmitter::~CInfectionEmitter( void )
{
}

//========================================================================
// CInfectionEmitter::Create
// ----------------------
// Purpose: Creates a new instance of a CInfectionEmitter object
//========================================================================
CSmartPtr< CInfectionEmitter > CInfectionEmitter::Create( const char *pDebugName )
{
	CInfectionEmitter *pRet = new CInfectionEmitter( pDebugName );

	if( !pRet )
		return NULL;

	pRet->SetDynamicallyAllocated();

	if( m_hMaterial == INVALID_MATERIAL_HANDLE )
		m_hMaterial = pRet->GetPMaterial( INFECTION_EFFECT_MATERIAL );

	return pRet;
}

//========================================================================
// CInfectionEmitter::RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void CInfectionEmitter::RenderParticles( CParticleRenderIterator *pIterator )
{
	const InfectionParticle *pParticle = ( const InfectionParticle * )pIterator->GetFirst();
	while( pParticle )
	{
		Vector	tPos;

		TransformParticle( ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos );
		float sortKey = ( int )tPos.z;

		Vector vColor = Vector( 
			pParticle->m_uchColor[ 0 ] / 255.0f,
			pParticle->m_uchColor[ 1 ] / 255.0f,
			pParticle->m_uchColor[ 2 ] / 255.0f );

		RenderParticle_ColorSize(
			pIterator->GetParticleDraw(),
			tPos,
			vColor,
			pParticle->m_flAlpha,
			pParticle->m_flSize	);

		pParticle = ( const InfectionParticle * )pIterator->GetNext( sortKey );
	}
}

//========================================================================
// CInfectionEmitter::SimulateParticles
// ----------
// Purpose: Simulates all the particles in the system
//========================================================================
void CInfectionEmitter::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	float timeDelta = pIterator->GetTimeDelta();

	InfectionParticle *pParticle = ( InfectionParticle * )pIterator->GetFirst();
	while( pParticle )
	{
		pParticle->m_flLifetime += timeDelta;

		float flEnd = pParticle->m_flLifetime / pParticle->m_flDieTime;
		float flBeg = 1.0f - flEnd;

		Vector F( 0.0f, 0.0f, 0.0f );

		ApplyDrag( &F, pParticle->m_vVelocity, 4.0f, 20.0f );

		pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.75f;
		pParticle->m_vVelocity += F * timeDelta;
		pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.75f;
		
		pParticle->m_flAlpha = 0.8f * flBeg + 0.0f * flEnd;

		if( pParticle->m_flLifetime >= pParticle->m_flDieTime )
			pIterator->RemoveParticle( pParticle );

		pParticle = ( InfectionParticle * )pIterator->GetNext();
	}
}

//========================================================================
// CRingEmitter::AddInfectionParticle
// ----------
// Purpose: Add a new particle to the system
//========================================================================
InfectionParticle *CInfectionEmitter::AddInfectionParticle( const Vector& vecOrigin )
{
	InfectionParticle *pRet = ( InfectionParticle * )AddParticle( sizeof( InfectionParticle ), m_hMaterial, vecOrigin );

	if( pRet )
	{
		pRet->m_vOrigin = vecOrigin;
		pRet->m_vFinalPos.Init();
		pRet->m_vVelocity.Init();
		pRet->m_flDieTime = 1.0f;
		pRet->m_flLifetime = 0.0f;
		pRet->m_uchColor[ 0 ] = 0;
		pRet->m_uchColor[ 1 ] = 255;
		pRet->m_uchColor[ 2 ] = 0;
		pRet->m_flAlpha = 0.8f;
		pRet->m_flSize = 1.0f;
	}

	return pRet;
}

void CInfectionEmitter::ApplyDrag( Vector *F, Vector vecVelocity, float flScale, float flTargetVel )
{
	if( vecVelocity.IsLengthLessThan( flTargetVel ) )
		return;

	Vector vecDir = -vecVelocity;
	vecVelocity.NormalizeInPlace();

	float flMag = vecVelocity.Length() * flScale;
	*F += ( vecDir * flMag );
}

void CInfectionEmitter::Update( float flTimeDelta )
{
	if( gpGlobals->curtime > m_flDieTime )
		return;

	if( gpGlobals->curtime < m_flNextParticle )
		return;

	m_flNextParticle = gpGlobals->curtime + 0.1f;

	// Add 5 particles
	for( int i = 0; i < m_iNumParticles; i++ )
	{
		InfectionParticle *pParticle = AddInfectionParticle( m_vecOrigin );
		if( pParticle )
		{
			/*
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
			*/

			pParticle->m_vOrigin = m_vecOrigin;
			pParticle->m_vVelocity = m_vecVelocity;
			pParticle->m_Pos = pParticle->m_vOrigin + RandomVector( -16.0f, 16.0f );
			pParticle->m_Pos.z = pParticle->m_vOrigin.z + RandomFloat( -16.0f, 16.0f );

			/*
			p->m_vOrigin = data.m_vOrigin;
			p->m_vVelocity = data.m_vStart;// + RandomVector( -10.0f, 10.0f );
			//p->m_vVelocity.z *= 0.1f;
			p->m_Pos = p->m_vOrigin + RandomVector( -10.0f, 10.0f );
			p->m_Pos.z = p->m_vOrigin.z + RandomFloat( -15.0f, 15.0f );
			*/
		}
	}
}

void FF_FX_InfectionEffect_Callback( const CEffectData &data )
{
	/*
	CSmartPtr< CInfectionEmitter > InfectionEffect = CInfectionEmitter::Create( "InfectionEffect" );

	for( int i = 0; i < 64; i++ )
	{
		InfectionParticle *p = InfectionEffect->AddInfectionParticle( data.m_vOrigin );
		if( p )
		{
			p->m_vOrigin = data.m_vOrigin;
			p->m_vVelocity = data.m_vStart;// + RandomVector( -10.0f, 10.0f );
			//p->m_vVelocity.z *= 0.1f;
			p->m_Pos = p->m_vOrigin + RandomVector( -10.0f, 10.0f );
			p->m_Pos.z = p->m_vOrigin.z + RandomFloat( -15.0f, 15.0f );
		}
	}
	*/
}

DECLARE_CLIENT_EFFECT( "FF_InfectionEffect", FF_FX_InfectionEffect_Callback );
