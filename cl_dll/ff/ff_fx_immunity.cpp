// =============== Fortress Forever ===============
// ======== A modification for Half-Life 2 ========
//
// @file cl_dll\ff\ff_fx_immunity.cpp
// @author Patrick O'Leary (Mulchman)
// @date 10/6/2006
// @brief Immunity particle effects
//
// particle manager for the immunity particles
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

#include "ff_fx_immunity.h"

//#define IMMUNITY_EFFECT_MATERIAL "particle/particle_smokegrenade"
#define IMMUNITY_EFFECT_MATERIAL "effects/yellowflare"

ConVar immunity_particles	( "ffdev_particles_per_immunity", "100", FCVAR_CHEAT, "The number of particles in each immunity." );
ConVar immunity_scale		( "ffdev_immunity_scale", "20.0", FCVAR_CHEAT, "How big the particles in the immunities are." );
ConVar immunity_speed		( "ffdev_immunity_speed", "0.2", FCVAR_CHEAT, "Duration of the immunity effect." );
ConVar immunity_magnitude	( "ffdev_immunity_magnitude", "1000.0", FCVAR_CHEAT, "Speed the immunity particles expand." );

//========================================================================
// Client effect precache table
//========================================================================
CLIENTEFFECT_REGISTER_BEGIN( PrecacheImmunityEmitter )
CLIENTEFFECT_MATERIAL( IMMUNITY_EFFECT_MATERIAL )
CLIENTEFFECT_REGISTER_END()

//========================================================================
// Static material handles
//========================================================================
PMaterialHandle CImmunityEmitter::m_hMaterial = INVALID_MATERIAL_HANDLE;

//========================================================================
// CImmunityEmitter constructor
//========================================================================
CImmunityEmitter::CImmunityEmitter( const char *pDebugName ) : CSimpleEmitter( pDebugName )
{
	m_pDebugName = pDebugName;
	m_flNextParticle = 0.0f;
}

//========================================================================
// CImmunityEmitter destructor
//========================================================================
CImmunityEmitter::~CImmunityEmitter( void )
{
}

//========================================================================
// CImmunityEmitter::Create
// ----------------------
// Purpose: Creates a new instance of a CImmunityEmitter object
//========================================================================
CSmartPtr< CImmunityEmitter > CImmunityEmitter::Create( const char *pDebugName )
{
	CImmunityEmitter *pRet = new CImmunityEmitter( pDebugName );

	if( !pRet )
		return NULL;

	pRet->SetDynamicallyAllocated();

	if( m_hMaterial == INVALID_MATERIAL_HANDLE )
		m_hMaterial = pRet->GetPMaterial( IMMUNITY_EFFECT_MATERIAL );

	return pRet;
}

//========================================================================
// CImmunityEmitter::RenderParticles
// ----------
// Purpose: Renders all the particles in the system
//========================================================================
void CImmunityEmitter::RenderParticles( CParticleRenderIterator *pIterator )
{
	const ImmunityParticle *pParticle = ( const ImmunityParticle * )pIterator->GetFirst();
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

		pParticle = ( const ImmunityParticle * )pIterator->GetNext( sortKey );
	}
}

//========================================================================
// CImmunityEmitter::SimulateParticles
// ----------
// Purpose: Simulates all the particles in the system
//========================================================================
void CImmunityEmitter::SimulateParticles( CParticleSimulateIterator *pIterator )
{
	float timeDelta = pIterator->GetTimeDelta();

	ImmunityParticle *pParticle = ( ImmunityParticle * )pIterator->GetFirst();
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

		pParticle = ( ImmunityParticle * )pIterator->GetNext();
	}
}

//========================================================================
// CRingEmitter::AddImmunityParticle
// ----------
// Purpose: Add a new particle to the system
//========================================================================
ImmunityParticle *CImmunityEmitter::AddImmunityParticle( const Vector& vecOrigin )
{
	ImmunityParticle *pRet = ( ImmunityParticle * )AddParticle( sizeof( ImmunityParticle ), m_hMaterial, vecOrigin );

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

void CImmunityEmitter::ApplyDrag( Vector *F, Vector vecVelocity, float flScale, float flTargetVel )
{
	if( vecVelocity.IsLengthLessThan( flTargetVel ) )
		return;

	Vector vecDir = -vecVelocity;
	vecVelocity.NormalizeInPlace();

	float flMag = vecVelocity.Length() * flScale;
	*F += ( vecDir * flMag );
}

void CImmunityEmitter::Update( float flTimeDelta )
{
	if( gpGlobals->curtime > m_flDieTime )
		return;

	if( gpGlobals->curtime < m_flNextParticle )
		return;

	m_flNextParticle = gpGlobals->curtime + 0.1f;

	// Add 5 particles
	for( int i = 0; i < 5; i++ )
	{
		ImmunityParticle *pParticle = AddImmunityParticle( m_vecOrigin );
		if( pParticle )
		{
			pParticle->m_vOrigin = m_vecOrigin;
			pParticle->m_vVelocity = m_vecVelocity;
			pParticle->m_Pos = pParticle->m_vOrigin + RandomVector( -16.0f, 16.0f );
			pParticle->m_Pos.z = pParticle->m_vOrigin.z + RandomFloat( -16.0f, 16.0f );
			pParticle->m_flRoll = random->RandomFloat( 0, 2 * M_PI );
			pParticle->m_flRollDelta = random->RandomFloat( -DEG2RAD( 180 ), DEG2RAD( 180 ) );
		}
	}
}
