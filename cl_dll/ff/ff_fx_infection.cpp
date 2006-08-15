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
#include "materialsystem/imaterialvar.h"
#include "mathlib.h"

#include "ff_fx_infection.h"

#define INFECTION_EFFECT_MATERIAL "particle/particle_smokegrenade"

ConVar infection_particles	( "ffdev_particles_per_infection", "100", 0, "The number of particles in each infection." );
ConVar infection_scale		( "ffdev_infection_scale", "20.0", 0, "How big the particles in the infections are." );
ConVar infection_speed		( "ffdev_infection_speed", "0.2", 0, "Duration of the infections effect." );
ConVar infection_magnitude	( "ffdev_infection_magnitude", "1000.0", 0, "Speed the infections expand." );

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

		pParticle->m_Pos += pParticle->m_vVelocity * timeDelta * 0.5f;		
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

void FF_FX_InfectionEffect_Callback( const CEffectData &data )
{
	CSmartPtr< CInfectionEmitter > InfectionEffect = CInfectionEmitter::Create( "InfectionEffect" );

	for( int i = 0; i < 64; i++ )
	{
		InfectionParticle *p = InfectionEffect->AddInfectionParticle( data.m_vOrigin );
		if( p )
		{
			p->m_vOrigin = data.m_vOrigin;
			p->m_vVelocity = RandomVector( -60.0f, 60.0f );
			p->m_vVelocity.z *= 0.1f;
			p->m_Pos = p->m_vOrigin + RandomVector( -10.0f, 10.0f );
			p->m_Pos.z = p->m_vOrigin.z + RandomFloat( -15.0f, 15.0f );
		}
	}
}

DECLARE_CLIENT_EFFECT( "FF_InfectionEffect", FF_FX_InfectionEffect_Callback );
