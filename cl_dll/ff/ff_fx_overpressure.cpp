// =============== Fortress Forever ===============
// ======== A modification for Half-Life 2 ========
//
// @file cl_dll\ff\ff_fx_overpressure.cpp
// @author Patrick O'Leary (Mulchman)
// @date 10/6/2006
// @brief Overpressure particle effects
//
// particle manager for the overpressure particles
//
// Revisions
// ---------
// 10/6/2006, Mulchman
//		Initial version

#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "particles_simple.h"
#include "c_te_effect_dispatch.h"
#include "cliententitylist.h"

#include "ff_fx_overpressure.h"

//#define OVERPRESSURE_EFFECT_MATERIAL "particle/particle_smokegrenade"
#define OVERPRESSURE_EFFECT_MATERIAL "effects/yellowflare"
#define RING_EFFECT_MATERIAL "sprites/lgtning.vmt"

ConVar overpressure_particles	( "ffdev_particles_per_overpressure", "750", FCVAR_CHEAT, "The number of particles in each overpressure." );
ConVar overpressure_scale		( "ffdev_overpressure_scale", "20.0", FCVAR_CHEAT, "How big the particles in the immunities are." );
ConVar overpressure_speed		( "ffdev_overpressure_speed", "1.0", FCVAR_CHEAT, "Duration of the overpressure effect." );
ConVar overpressure_particlespeed( "ffdev_overpressure_particlespeed", "200", FCVAR_CHEAT, "Velocity of the overpressure particles." );
ConVar overpressure_particlespeed_variability( "ffdev_overpressure_particlespeed_variability", "40", FCVAR_CHEAT, "Variability in the velocity of the overpressure particles." );
ConVar overpressure_particle_size( "ffdev_overpressure_particle_size", "2", FCVAR_CHEAT, "Velocity of the overpressure particles." );
ConVar overpressure_magnitude	( "ffdev_overpressure_magnitude", "1000.0", FCVAR_CHEAT, "Speed the overpressure particles expand." );

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheOverpressureEmitter )
	CLIENTEFFECT_MATERIAL( OVERPRESSURE_EFFECT_MATERIAL )
	CLIENTEFFECT_MATERIAL( RING_EFFECT_MATERIAL )
CLIENTEFFECT_REGISTER_END()

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle COverpressureEmitter::m_hMaterial = INVALID_MATERIAL_HANDLE;

//========================================================================
// COverpressureEmitter constructor
//========================================================================
COverpressureEmitter::COverpressureEmitter( const char *pDebugName ) : CSimpleEmitter( pDebugName )
{
	m_pDebugName = pDebugName;
	m_flNextParticle = 0.0f;
}

//========================================================================
// COverpressureEmitter destructor
//========================================================================
COverpressureEmitter::~COverpressureEmitter( void )
{
}

//========================================================================
// COverpressureEmitter::Create
// ----------------------
// Purpose: Creates a new instance of a COverpressureEmitter object
//========================================================================
CSmartPtr< COverpressureEmitter > COverpressureEmitter::Create( const char *pDebugName )
{
	COverpressureEmitter *pRet = new COverpressureEmitter( pDebugName );

	if( !pRet )
		return NULL;

	pRet->SetDynamicallyAllocated();

	if( m_hMaterial == INVALID_MATERIAL_HANDLE )
		m_hMaterial = pRet->GetPMaterial( OVERPRESSURE_EFFECT_MATERIAL );

	return pRet;
}

//========================================================================
// COverpressureEmitter::RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void COverpressureEmitter::RenderParticles( CParticleRenderIterator *pIterator )
{
	const OverpressureParticle *pParticle = ( const OverpressureParticle * )pIterator->GetFirst();
	while( pParticle )
	{
		Vector	tPos;

		TransformParticle( ParticleMgr()->GetModelView(), pParticle->m_Pos, tPos );
		float sortKey = ( int )tPos.z;

		Vector vColor = Vector( 
			pParticle->m_uchColor[ 0 ] / 255.0f,
			pParticle->m_uchColor[ 1 ] / 255.0f,
			pParticle->m_uchColor[ 2 ] / 255.0f );

		RenderParticle_ColorSizeAngle(
			pIterator->GetParticleDraw(),
			tPos,
			vColor,
			pParticle->m_flAlpha,
			pParticle->m_flSize,
			pParticle->m_flRoll );

		pParticle = ( const OverpressureParticle * )pIterator->GetNext( sortKey );
	}
}

//========================================================================
// COverpressureEmitter::SimulateParticles
// ----------
// Purpose: Simulates all the particles in the system
//========================================================================
void COverpressureEmitter::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	float timeDelta = pIterator->GetTimeDelta();

	OverpressureParticle *pParticle = ( OverpressureParticle * )pIterator->GetFirst();
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
		pParticle->m_flRoll += pParticle->m_flRollDelta * timeDelta;
		
		pParticle->m_flAlpha = 0.8f * flBeg + 0.0f * flEnd;

		if( pParticle->m_flLifetime >= pParticle->m_flDieTime )
			pIterator->RemoveParticle( pParticle );

		pParticle = ( OverpressureParticle * )pIterator->GetNext();
	}
}

//========================================================================
// CRingEmitter::AddOverpressureParticle
// ----------
// Purpose: Add a new particle to the system
//========================================================================
OverpressureParticle *COverpressureEmitter::AddOverpressureParticle( const Vector& vecOrigin )
{
	OverpressureParticle *pRet = ( OverpressureParticle * )AddParticle( sizeof( OverpressureParticle ), m_hMaterial, vecOrigin );

	if( pRet )
	{
		pRet->m_vOrigin = vecOrigin;
		pRet->m_vFinalPos.Init();
		pRet->m_vVelocity.Init();
		pRet->m_flDieTime = 1.0f;
		pRet->m_flLifetime = 0.0f;
		pRet->m_uchColor[ 0 ] = 255;
		pRet->m_uchColor[ 1 ] = 255;
		pRet->m_uchColor[ 2 ] = 255;
		pRet->m_flAlpha = 0.8f;
		pRet->m_flSize = 1.0f;
		pRet->m_flRoll = 0.0f;
		pRet->m_flRollDelta = 0.0f;
	}

	return pRet;
}

void COverpressureEmitter::ApplyDrag( Vector *F, Vector vecVelocity, float flScale, float flTargetVel )
{
	if( vecVelocity.IsLengthLessThan( flTargetVel ) )
		return;

	Vector vecDir = -vecVelocity;
	vecVelocity.NormalizeInPlace();

	float flMag = vecVelocity.Length() * flScale;
	*F += ( vecDir * flMag );
}

void FF_FX_OverpressureEffect_Callback( const CEffectData &data )
{
	CSmartPtr< COverpressureEmitter > overpressureEffect = COverpressureEmitter::Create( "OverpressureEffect" );

	float offset = 0.0f;

	// Add 5 particles
	for( int i = 0; i < overpressure_particles.GetInt(); i++ )
	{
		OverpressureParticle *pParticle = overpressureEffect->AddOverpressureParticle( data.m_vOrigin );
		if( pParticle )
		{
			pParticle->m_vOrigin = data.m_vOrigin;

			// get a random point on a unit sphere
			float ptZ = 2.0 * random->RandomFloat() - 1.0;
			float ptT = 2.0 * M_PI * random->RandomFloat();
			float ptW = sqrt( 1 - ptZ*ptZ );
			float ptX = ptW * cos( ptT );
			float ptY = ptW * sin( ptT );

			pParticle->m_vVelocity = Vector(ptX, ptY, ptZ) * (overpressure_particlespeed.GetFloat() + random->RandomInt(-overpressure_particlespeed_variability.GetInt(), overpressure_particlespeed_variability.GetInt()));
			pParticle->m_Pos = pParticle->m_vOrigin + RandomVector( -4.0f, 4.0f );
			pParticle->m_flRoll = random->RandomFloat( 0, 2 * M_PI );
			pParticle->m_flDieTime = overpressure_speed.GetFloat();
			pParticle->m_flSize = overpressure_particle_size.GetFloat();
			pParticle->m_flRollDelta = random->RandomFloat( -DEG2RAD( 180 ), DEG2RAD( 180 ) );
		}
	}
}

DECLARE_CLIENT_EFFECT( "FF_OverpressureEffect", FF_FX_OverpressureEffect_Callback );